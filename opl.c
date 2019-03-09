//
// RoboSOP
//
// Copyright (C) 2019 by RoboSoft.
//
// Made with FUSION-C in SDCC

#include "fusion-c/header/msx_fusion.h"
#include "fusion-c/header/newTypes.h"
#include "fusion-c/header/rammapper.h"
#include "RoboSOP.h"
#include "opl.h"

#define OPL4_TIMER1_COUNT 0x02
#define OPL4_TIMER2_COUNT 0x03

/* percussive voice numbers: */
#define BD      6
#define SD      7
#define TOM     8
#define CYMB    9
#define HIHAT  10

#define MAX_VOLUME          0x7f
#define LOG2_VOLUME         7       /* log2( MAX_VOLUME) */

#define MID_C               60      /* MIDI standard mid C */

#define SOP_TOM_PITCH       36      /* best frequency, in range of 0 to 95 */
#define TOM_TO_SD           7       /* 7 half-tones between voice 7 & 8 */
#define SOP_SD_PITCH        (SOP_TOM_PITCH + TOM_TO_SD)

#define NB_NOTES            96      /* 8 octave of 12 notes */
#define OCTAVE              12      /* half-tone by octave */
#define NB_STEP_PITCH       32      /* 32 steps between two half-tones */
#define LOG_NB_STEP_PITCH   5       /* LOG2( NB_STEP_PITCH ) */
#define NB_TABLE_DEMI_TON   OCTAVE
#define TABLE_SIZE          (NB_STEP_PITCH * NB_TABLE_DEMI_TON)

#define maxVoices       20
#define YMB_SIZE        80

byte volume[SOP_MAX_TRACK];
byte lastvol[SOP_MAX_TRACK];
byte masterVolume;
byte* chanMode;

const int fNumTbl[TABLE_SIZE]={
    0x0159, 0x015A, 0x015A, 0x015B, 0x015C, 0x015C, 0x015D, 0x015D, 0x015E, 0x015F, 0x015F, 0x0160,
    0x0161, 0x0161, 0x0162, 0x0162, 0x0163, 0x0164, 0x0164, 0x0165, 0x0166, 0x0166, 0x0167, 0x0168,
    0x0168, 0x0169, 0x016A, 0x016A, 0x016B, 0x016C, 0x016C, 0x016D, 0x016E, 0x016E, 0x016F, 0x016F,
    0x0170, 0x0171, 0x0171, 0x0172, 0x0173, 0x0174, 0x0174, 0x0175, 0x0176, 0x0176, 0x0177, 0x0178,
    0x0178, 0x0179, 0x017A, 0x017A, 0x017B, 0x017C, 0x017C, 0x017D, 0x017E, 0x017E, 0x017F, 0x0180,
    0x0180, 0x0181, 0x0182, 0x0183, 0x0183, 0x0184, 0x0185, 0x0185, 0x0186, 0x0187, 0x0187, 0x0188,
    0x0189, 0x018A, 0x018A, 0x018B, 0x018C, 0x018C, 0x018D, 0x018E, 0x018F, 0x018F, 0x0190, 0x0191,
    0x0191, 0x0192, 0x0193, 0x0194, 0x0194, 0x0195, 0x0196, 0x0197, 0x0197, 0x0198, 0x0199, 0x019A,
    0x019A, 0x019B, 0x019C, 0x019D, 0x019D, 0x019E, 0x019F, 0x019F, 0x01A0, 0x01A1, 0x01A2, 0x01A3,
    0x01A3, 0x01A4, 0x01A5, 0x01A6, 0x01A6, 0x01A7, 0x01A8, 0x01A9, 0x01A9, 0x01AA, 0x01AB, 0x01AC,
    0x01AC, 0x01AD, 0x01AE, 0x01AF, 0x01B0, 0x01B0, 0x01B1, 0x01B2, 0x01B3, 0x01B3, 0x01B4, 0x01B5,
    0x01B6, 0x01B7, 0x01B7, 0x01B8, 0x01B9, 0x01BA, 0x01BB, 0x01BB, 0x01BC, 0x01BD, 0x01BE, 0x01BF,
    0x01BF, 0x01C0, 0x01C1, 0x01C2, 0x01C3, 0x01C3, 0x01C4, 0x01C5, 0x01C6, 0x01C7, 0x01C8, 0x01C8,
    0x01C9, 0x01CA, 0x01CB, 0x01CC, 0x01CD, 0x01CD, 0x01CE, 0x01CF, 0x01D0, 0x01D1, 0x01D2, 0x01D2,
    0x01D3, 0x01D4, 0x01D5, 0x01D6, 0x01D7, 0x01D7, 0x01D8, 0x01D9, 0x01DA, 0x01DB, 0x01DC, 0x01DD,
    0x01DD, 0x01DE, 0x01DF, 0x01E0, 0x01E1, 0x01E2, 0x01E3, 0x01E4, 0x01E4, 0x01E5, 0x01E6, 0x01E7,
    0x01E8, 0x01E9, 0x01EA, 0x01EB, 0x01EB, 0x01EC, 0x01ED, 0x01EE, 0x01EF, 0x01F0, 0x01F1, 0x01F2,
    0x01F3, 0x01F3, 0x01F4, 0x01F5, 0x01F6, 0x01F7, 0x01F8, 0x01F9, 0x01FA, 0x01FB, 0x01FC, 0x01FD,
    0x01FE, 0x01FE, 0x01FF, 0x0200, 0x0201, 0x0202, 0x0203, 0x0204, 0x0205, 0x0206, 0x0207, 0x0208,
    0x0209, 0x020A, 0x020B, 0x020B, 0x020C, 0x020D, 0x020E, 0x020F, 0x0210, 0x0211, 0x0212, 0x0213,
    0x0214, 0x0215, 0x0216, 0x0217, 0x0218, 0x0219, 0x021A, 0x021B, 0x021C, 0x021D, 0x021E, 0x021F,
    0x0220, 0x0221, 0x0222, 0x0223, 0x0224, 0x0225, 0x0226, 0x0227, 0x0228, 0x0229, 0x022A, 0x022B,
    0x022C, 0x022D, 0x022E, 0x022F, 0x0230, 0x0231, 0x0232, 0x0233, 0x0234, 0x0235, 0x0236, 0x0237,
    0x0238, 0x0239, 0x023A, 0x023B, 0x023C, 0x023D, 0x023E, 0x023F, 0x0240, 0x0241, 0x0242, 0x0243,
    0x0244, 0x0245, 0x0246, 0x0247, 0x0248, 0x0249, 0x024B, 0x024C, 0x024D, 0x024E, 0x024F, 0x0250,
    0x0251, 0x0252, 0x0253, 0x0254, 0x0255, 0x0256, 0x0257, 0x0258, 0x025A, 0x025B, 0x025C, 0x025D,
    0x025E, 0x025F, 0x0260, 0x0261, 0x0262, 0x0263, 0x0265, 0x0266, 0x0267, 0x0268, 0x0269, 0x026A,
    0x026B, 0x026C, 0x026D, 0x026F, 0x0270, 0x0271, 0x0272, 0x0273, 0x0274, 0x0275, 0x0276, 0x0278,
    0x0279, 0x027A, 0x027B, 0x027C, 0x027D, 0x027E, 0x0280, 0x0281, 0x0282, 0x0283, 0x0284, 0x0285,
    0x0287, 0x0288, 0x0289, 0x028A, 0x028B, 0x028C, 0x028E, 0x028F, 0x0290, 0x0291, 0x0292, 0x0294,
    0x0295, 0x0296, 0x0297, 0x0298, 0x029A, 0x029B, 0x029C, 0x029D, 0x029E, 0x02A0, 0x02A1, 0x02A2,
    0x02A3, 0x02A4, 0x02A6, 0x02A7, 0x02A8, 0x02A9, 0x02AB, 0x02AC, 0x02AD, 0x02AE, 0x02B0, 0x02B1
};

const byte SlotX[maxVoices * 2]={
    0, 1, 2, 8, 9, 10, 16, 17, 18,  0,  0, 0, 1, 2, 8, 9, 10, 16, 17, 18,
    0, 1, 2, 8, 9, 10, 16, 20, 18, 21, 17, 0, 1, 2, 8, 9, 10, 16, 17, 18
};

const byte VolReg[11 * 2]={
    0x43, 0x44, 0x45, 0x4B, 0x4C, 0x4D, 0x53, 0x54, 0x55, 0, 0,
    0x43, 0x44, 0x45, 0x4B, 0x4C, 0x4D, 0x53, 0x54, 0x52, 0x55, 0x51
};

const byte MOD12[OCTAVE * 11]={
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
};

const byte DIV12[OCTAVE * 8]={
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
};

boolean percussion; /* percussion mode parameter */

byte* VolumeTable; /* pre-calculated Volume Table */

char voiceNote[20];    /* pitch of last note-on of each voice */
char voiceKeyOn[20];   /* state of keyOn bit of each voice */
byte vPitchBend[20];   /* current pitch bend of each voice */
byte Ksl[20];          /* KSL value for Slot 1 */
byte Ksl2[20];         /* KSL value for Slot 2 */
byte Ksl2V[20];        /* Parallel connection? */
byte VoiceVolume[20], OP_MASK;
byte ymbuf[ 2 * YMB_SIZE ];
byte OP4[20];
byte Stereo[22];

//
// Output to OPL port 1
// 

void FT_WriteOpl1(byte addr, byte value)
{
    byte busy = 0;
    if(addr >= 0xB0) ymbuf[addr - 0xB0] = value;

#ifdef __DEBUG
    printf("%x: %x ", OPL4_REG, addr);
#endif
    do
    {
        busy = InPort(OPL4_REG);
    } while(busy & 1);
    OutPort(OPL4_REG, addr);
    do 
    {
        busy = InPort(OPL4_REG);
    } while(busy & 1);
#ifdef __DEBUG
    printf("%x: %x| ", OPL4_DATA, value);
#endif    
    OutPort(OPL4_DATA, value);   
}

//
// Output to OPL port 3
// 

void FT_WriteOpl3(byte addr, byte value)
{
    byte busy = 0;
    if(addr >= 0xB0) ymbuf[YMB_SIZE - 0xB0 + addr] = value;

#ifdef __DEBUG
    printf("%x: %x ", OPL4_REG2, addr);
#endif
    do    
    {
        busy = InPort(OPL4_REG);
    } while(busy & 1);
    OutPort(OPL4_REG2, addr);
    do 
    {
        busy = InPort(OPL4_REG);
    } while(busy & 1);
#ifdef __DEBUG
    printf("%x: %x| ", OPL4_DATA, value);
#endif    
    OutPort(OPL4_DATA, value);
}

//
// Set timer T2 count
//
void FT_SetTimer2Count(byte value)
{
    FT_WriteOpl1(OPL4_TIMER2_COUNT, value);
}

//
// Set timer T2 state
//
void FT_SetTimer2State(boolean state)
{
    FT_WriteOpl1(4, (state) ? 0x02 : 0x00);
}

//
// Set channel to 4OP mode
//
void FT_Set4OPMode(byte channel, byte value)
{
    if(SlotX[channel + 20] <= 2)
    {
        OP4[channel] = value;

        if(value)
        {
            if(channel > 10) 
                OP_MASK |= (0x01 << (channel - 11 + 3));
            else 
                OP_MASK |= (0x01 << channel);
        }
        else
        {
            if(channel > 10) 
                OP_MASK &= (0xFF - (0x01 << (channel - 11 + 3)));
            else 
                OP_MASK &= (0xFF -(0x01 << channel));
        }

        FT_WriteOpl3(0x04, OP_MASK);
    }
}

//
// Set frequency
//
void FT_SetFreq(byte voice, byte note, int pitch, byte keyOn)
{
    int temp, fN, divFactor, fNIndex;

    temp = (int)((pitch - 100) / 3.125) + ((note - 12) << LOG_NB_STEP_PITCH);

    if (temp < 0)
        temp = 0;
    else {
        if (temp >= ((NB_NOTES << LOG_NB_STEP_PITCH) - 1))
            temp = (NB_NOTES << LOG_NB_STEP_PITCH) - 1;
    }

    fNIndex = (MOD12[(temp >> 5)] << 5) + (temp & (NB_STEP_PITCH - 1));

    fN = fNumTbl[fNIndex];

    divFactor = DIV12[(temp >> 5)];

    if (voice <= 10)
        FT_WriteOpl1(0xA0 + voice, (fN & 0xFF));
    else
        FT_WriteOpl3(0xA0 + voice - 11, (fN & 0xFF));

    fN = (((fN >> 8) & 0x03) | (divFactor << 2) | keyOn) & 0xFF;

    if (voice <= 10)
        FT_WriteOpl1(0xB0 + voice, fN);
    else
        FT_WriteOpl3(0xB0 + voice - 11, fN);
}

//
// Set percussion mode
//
void FT_SetPercussionMode(boolean mode)
{
    if (mode) 
    {
        /* set the frequency for the last 4 percussion voices */
        voiceNote[TOM] = SOP_TOM_PITCH;
        vPitchBend[TOM] = 100;
        FT_SetFreq(TOM, voiceNote[TOM], vPitchBend[TOM], 0);

        voiceNote[SD] = SOP_SD_PITCH;
        vPitchBend[SD] = 100;
        FT_SetFreq(SD, voiceNote[SD], vPitchBend[SD], 0);
    }

    percussion = mode;

    FT_WriteOpl1(0xBD, (percussion ? 0x20 : 0));
}

void FT_InitVolumeTable()
{
    byte i,j;
    
    VolumeTable = (byte *)VOLUME_TABLE_BASE;
    for(i = 0; i < 64; i++)
    {
        for(j = 0; j < 128; j++)
        {
            VolumeTable[i * 128 + j] = (i * j + (MAX_VOLUME +1) /2) >> LOG2_VOLUME;
        }
    }
}

//
//
//
void FT_ClearOPL()
{
    byte i;
    
    for(i = 20; i <= 0xf5; i++)
    {
        FT_WriteOpl1(i, 0);
        FT_WriteOpl3(i, 0);
    }
}

//
// Reset OPL
//
void FT_OPL_Reset()
{
    byte i;

    for(i = 0; i < YMB_SIZE; i++)
    {
        ymbuf[i] = 0;
        ymbuf[i + YMB_SIZE] = 0;
    }

    for (i = 0; i < 20; i++) 
    {
        vPitchBend[i] = 100;
        voiceKeyOn[i] = 0;
        voiceNote[i] = MID_C;
        VoiceVolume[i] = 0;
        Ksl[i] = 0;
        Ksl2[i] = 0;
        Ksl2V[i] = 0;
        OP4[i] = 0;
        Stereo[i] = 0x30;
    }

    OP_MASK = 0;


    FT_WriteOpl3(5, 3); /* YMF-262M Mode */
    FT_WriteOpl3(4, 0);

    FT_WriteOpl1(4, 0x60); /* Mask T1 & T2 */

    FT_SetPercussionMode(false); /* melodic mode */

    FT_WriteOpl1(8, 0);
    FT_WriteOpl1(1, 0x20);

    FT_ClearOPL();
}

//
// Activate note
//
void FT_NoteOn(byte channel, byte pitch)
{
    if(percussion && channel >= BD && channel <= HIHAT)
    {
        if(channel == BD)
        {
            voiceNote[BD] = pitch;
            FT_SetFreq(BD, voiceNote[BD], vPitchBend[BD], 0);
        }
        else
        {
            if(channel == TOM && (unsigned)voiceNote[TOM] != pitch)
            {
                voiceNote[SD] = (voiceNote[TOM] = pitch) + TOM_TO_SD;
                FT_SetFreq(TOM, voiceNote[TOM], 100, 0);
                FT_SetFreq(SD, voiceNote[SD], 100, 0);
            }
        }
        FT_WriteOpl1(0xBD, ymbuf[0x0D] | (0x10 >> (channel - BD)));
    }
    else
    {
        voiceNote[channel] = pitch;
        voiceKeyOn[channel] = 0x20;

        FT_SetFreq(channel, pitch, vPitchBend[channel], 0x20);
    }
}

//
// Switch off active note
//
void FT_NoteOff(byte channel)
{
    voiceKeyOn[channel] = 0;

    if(percussion && channel >= BD && channel <= HIHAT)
        FT_WriteOpl1(0xBD, ymbuf[0x0D] & (0xFF - (0x10 >> (channel -BD))));
    else
    {
        if(channel < HIHAT)
            FT_WriteOpl1(0xB0 + channel, ymbuf[channel] & 0xDF);
        else
            FT_WriteOpl3(0xB0 - 11 + channel, ymbuf[channel - 11 + YMB_SIZE] & 0xDF);
    }
}

//
// Set voice volume
//
void FT_SetVoiceVolume(byte chan, byte vol)
{    

    int volume;
    unsigned char KSL_value;

    if (chan > 2 && OP4[chan - 3])
        return;

    Put_PN(PAGE_2, segmentList[VOLUME_SEGMENT].allocatedSegmentNumber);

    if (vol > MAX_VOLUME)
        vol = MAX_VOLUME;

    VoiceVolume[chan] = vol;

    if (Ksl2V[chan]) {
        volume = 63 - VolumeTable[((63 - ((KSL_value = Ksl2[chan]) & 0x3F)) << 7) + vol];

        if (chan >= 11)
            FT_WriteOpl3(VolReg[chan - 11] - 3, KSL_value & 0xC0 | volume);
        else
            FT_WriteOpl1((percussion ? VolReg[chan + 11] : VolReg[chan]) - 3, KSL_value & 0xC0 | volume);

        if (OP4[chan]) {
            chan += 3;
            volume = 63 - VolumeTable[((63 - ((KSL_value = Ksl[chan]) & 0x3F)) << 7) + vol];

            if (chan >= 11)
                FT_WriteOpl3(VolReg[chan - 11], KSL_value & 0xC0 | volume);
            else
                FT_WriteOpl1(VolReg[chan], KSL_value & 0xC0 | volume);

            if (Ksl2V[chan]) {
                volume = 63 - VolumeTable[((63 - ((KSL_value = Ksl2[chan]) & 0x3F)) << 7) + vol];

                if (chan >= 11)
                    FT_WriteOpl3(VolReg[chan - 11] - 3, KSL_value & 0xC0 | volume);
                else
                    FT_WriteOpl1(VolReg[chan] - 3, KSL_value & 0xC0 | volume);
            }
        }
        else {
            volume = 63 - VolumeTable[((63 - ((KSL_value = Ksl[chan]) & 0x3F)) << 7) + vol];

            if (chan >= 11)
                FT_WriteOpl3(VolReg[chan - 11], KSL_value & 0xC0 | volume);
            else
                FT_WriteOpl1((percussion ? VolReg[chan + 11] : VolReg[chan]), KSL_value & 0xC0 | volume);
        }
    }
    else {
        if (OP4[chan]) {
            volume = 63 - VolumeTable[((63 - ((KSL_value = Ksl[chan + 3]) & 0x3F)) << 7) + vol];

            if (chan >= 11)
                FT_WriteOpl3(VolReg[chan + 3 - 11], KSL_value & 0xC0 | volume);
            else
                FT_WriteOpl1(VolReg[chan + 3], KSL_value & 0xC0 | volume);

            if (Ksl2V[chan + 3]) {
                volume = 63 - VolumeTable[((63 - ((KSL_value = Ksl[chan]) & 0x3F)) << 7) + vol];

                if (chan >= 11)
                    FT_WriteOpl3(VolReg[chan - 11], KSL_value & 0xC0 | volume);
                else
                    FT_WriteOpl1(VolReg[chan], KSL_value & 0xC0 | volume);
            }
        }
        else {
            volume = 63 - VolumeTable[((63 - ((KSL_value = Ksl[chan]) & 0x3F)) << 7) + vol];

            if (chan >= 11)
                FT_WriteOpl3(VolReg[chan - 11], KSL_value & 0xC0 | volume);
            else
                FT_WriteOpl1((percussion ? VolReg[chan + 11] : VolReg[chan]), KSL_value & 0xC0 | volume);
        }
    }   

    Put_PN(PAGE_2, segmentList[Tracks[chan].currentSegment].allocatedSegmentNumber);
}

//
// Send instrument data
//
void FT_SendIns(int base_addr, byte* value, boolean mode)
{
    byte i;

    for(i = 0; i < 4; i++)
    {
        if(!mode) 
            FT_WriteOpl1(base_addr, *value++);
        else
            FT_WriteOpl3(base_addr, *value++);
        base_addr += 0x20;
    }
    base_addr += 0x40;
    if(!mode) 
        FT_WriteOpl1(base_addr, (*value) & 0x07);
    else
        FT_WriteOpl3(base_addr, (*value) & 0x07);
}

//
// Set voice timbre
//
void FT_SetVoiceTimbre(byte chan, byte* array)
{
    int i;
    int Slot_Number, KSL_value;

    if (chan > 2 && OP4[chan - 3])
        return;

    Put_PN(PAGE_2, segmentList[INSTRUMENT_SEGMENT].allocatedSegmentNumber);

    if (!percussion)
        Slot_Number = SlotX[chan];
    else
        Slot_Number = SlotX[chan + 20];

    Ksl2V[chan] = ((KSL_value = (array[5] & 0x0F)) & 0x01);

    if (chan > 10) 
    {
        i = chan + 0xC0 - 11;

        FT_WriteOpl3(i, 0);

        FT_SendIns(0x20 + Slot_Number, array, true);
        FT_SendIns(0x23 + Slot_Number, &array[6], true);

        if (OP4[chan]) {
            FT_WriteOpl3(i + 3, 0);

            FT_SendIns(0x28 + Slot_Number, &((unsigned char*)array)[11], true);
            FT_SendIns(0x2B + Slot_Number, &((unsigned char*)array)[17], true);

            Ksl[chan + 3] = *(array + 18);
            Ksl2[chan + 3] = *(array + 12);
            Ksl2V[chan + 3] = *(array + 16) & 1;

            FT_WriteOpl3(i + 3, (*(array + 16) & 0x0F) | Stereo[chan]);
        }

        Ksl[chan] = *(array + 7);
        Ksl2[chan] = *(array + 1);
        Ksl2V[chan] = *(array + 5) & 1;

        FT_SetVoiceVolume(chan, VoiceVolume[chan]);
        FT_WriteOpl3(i, KSL_value | Stereo[chan]);
    }
    else {
        if (chan > 8)
            i = 0xC0 + 17 - chan;
        else
            i = chan + 0xC0;

        FT_WriteOpl1(i, 0);

        FT_SendIns(0x20 + Slot_Number, array, false);

        if (percussion && chan > BD)
        {
            Ksl[chan] = *(array + 1);
            Ksl2V[chan] = 0;
        }
        else
        {
            FT_SendIns(0x23 + Slot_Number, &array[6], false);

            Ksl[chan] = *(array + 7);
            Ksl2[chan] = *(array + 1);
            Ksl2V[chan] = *(array + 5) & 1;
        }

        if (OP4[chan]) 
        {
            FT_WriteOpl1(i + 3, 0);

            FT_SendIns(0x28 + Slot_Number, &((unsigned char*)array)[11], false);
            FT_SendIns(0x2B + Slot_Number, &((unsigned char*)array)[17], false);

            Ksl[chan + 3] = *(array + 18);
            Ksl2[chan + 3] = *(array + 12);
            Ksl2V[chan + 3] = *(array + 16) & 1;

            FT_WriteOpl1(i + 3, *(array + 16) & 0x0F | Stereo[chan]);
        }

        FT_SetVoiceVolume(chan, VoiceVolume[chan]);
        FT_WriteOpl1(i, KSL_value | Stereo[chan]);
    }

    Put_PN(PAGE_2, segmentList[Tracks[chan].currentSegment].allocatedSegmentNumber);
}

//
// Set stereo panning
//
void FT_SetStereoPAN(byte chan, byte value)
{
    byte PAN[] = { 0xA0, 0x30, 0x50 };
    boolean output3 = false;
    byte addr = 0;
    byte data;

    Stereo[chan] = (byte)(value = PAN[value]);

    if(chan < 9) addr = chan;
    else
    {
        if(chan < 11) addr = 17 - chan;
        else
        {
            addr = chan - 11;
            output3 = true;
        }
    }

    value |= ((chan >= 11) ? ymbuf[YMB_SIZE + 0x10 + addr] & 0x0F : ymbuf[addr + 0x10] & 0x0F);

    if(OP4[chan])
    {
        data = (value & 0xF0) | ((chan >= 11) ? ymbuf[YMB_SIZE + 0x13 + addr] : ymbuf[addr + 0x13]) & 0x0F;
        (output3) ? FT_WriteOpl3(addr + 0xC3, data) : FT_WriteOpl1(addr + 0xC3, data);
    }
    (output3) ? FT_WriteOpl3(addr + 0xC0, value) : FT_WriteOpl1(addr + 0xC0, value);
}

//
// Set voice pitch
//
void FT_SetVoicePitch(byte chan, byte pitch)
{
    vPitchBend[chan] = pitch;

    if(!percussion)
    {
        FT_SetFreq(chan, voiceNote[chan], pitch, voiceKeyOn[chan]);
    }
    else
    {
        if(chan <= BD || chan > HIHAT)
        {
            FT_SetFreq(chan, voiceNote[chan], pitch, voiceKeyOn[chan]);
        }
    }
}