//
// RoboSOP
//
// Copyright (C) 2019 by RoboSoft.
//
// Made with FUSION-C in SDCC

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "fusion-c/header/msx_fusion.h"
#include "fusion-c/header/newTypes.h"
#include "fusion-c/header/rammapper.h"
#include "RoboSOP.h"
#include "opl.h"
#include "memory.h"
#include "file.h"
#include "interrupt.h"

boolean songContinue;

#define PPI_REG_A 0xA8
#define PPI_REG_B 0xA9
#define PPI_REG_C 0xAA

//
// Set the overall tempo
//
void FT_SetTempo(byte tempo)
{
    int interval;
    int curTempo = tempo;
    float timer; 

    interval = (tempo * Header.tickBeat) / 60;
    timer = interval / 0.32;

    FT_SetTimer2Count(255 - (byte)timer);

#ifdef __DEBUG        
    printf("Tempo: %i, delay: %i, N2:%i\n\r", tempo, interval, 255-(int)timer);
#endif
}

//
// Reset all song data
//
void FT_ResetSong()
{
    byte i = 0;
    FT_OPL_Reset();
    FT_SetTempo(Header.basicTempo);

    for(i = 0; i < Header.nTracks+1; i++)
    {
        Tracks[i].currentSegment = Tracks[i].startSegment;
        Tracks[i].currentData = Tracks[i].startData;

        Tracks[i].curEvent = 0;
        Tracks[i].dur = 0;
        Tracks[i].counter = 0;

#ifdef __DEBUG
        printf("%i: ", i);
#endif
        Put_PN(PAGE_2, segmentList[Tracks[i].currentSegment].allocatedSegmentNumber);
        Tracks[i].ticks = FT_GetTrackData(i);
        Tracks[i].ticks |= FT_GetTrackData(i) << 8;

#ifdef __DEBUG 
        printf("Ticks: %i\n\r", Tracks[i].ticks);
#endif

        if(chanMode[i] == SOP_CHAN_4OP)
        {
            FT_Set4OPMode(i, 1);
        }
    }

    for(i = 0; i < SOP_MAX_TRACK; i++)
    {
        volume[i] = 0;
        lastvol[i] = 0;
    }
    masterVolume = SOP_MAX_VOL;

    FT_SetPercussionMode(Header.percussive);
}

//
// FT_ExecuteCommand
//
void FT_ExecuteCommand(byte track)
{
    byte i, value = 0;

    byte event = FT_GetTrackData(track);

    switch(event)
    {
         case SOP_EVNT_SPEC:
            break;
         case SOP_EVNT_NOTE:
            value = FT_GetTrackData(track);
            Tracks[track].dur = FT_GetTrackData(track) + (FT_GetTrackData(track) << 8);
            FT_NoteOn(track, value);
            break;
         case SOP_EVNT_VOL:
            value = FT_GetTrackData(track);          
            lastvol[track] = value;
            value = value * masterVolume / SOP_MAX_VOL;
            if(value != volume[track])
            {
                FT_SetVoiceVolume(track, value);
                volume[track] = value;
            }
            break;
         case SOP_EVNT_PITCH:
            value = FT_GetTrackData(track);
            FT_SetVoicePitch(track, value);
            break;
         case SOP_EVNT_INST:
            value = FT_GetTrackData(track);
            FT_SetVoiceTimbre(track, Instruments[value].instData);
            break;
         case SOP_EVNT_PAN:
            value = FT_GetTrackData(track);
            FT_SetStereoPAN(track, value);
            break;
         case SOP_EVNT_TEMPO:
            value = FT_GetTrackData(track);
            FT_SetTempo(value);
            break;
         case SOP_EVNT_MVOL:
            value = FT_GetTrackData(track);
            masterVolume = value;
            for(i = 0; i < Header.nTracks; i++)
            {
                value = lastvol[i] * masterVolume / SOP_MAX_VOL;
                if(value != volume[i])
                {
                    FT_SetVoiceVolume(i, value);
                    volume[i] = value;
                }
            }
            break;
    }

#ifdef __DEBUG
    printf("%i %i [%i]\n\r", event, value, Tracks[track].curEvent-1);
#endif    

    Tracks[track].curEvent++;
}

// Update all tracks
//
boolean FT_Update()
{
    byte i;
    boolean songEnd = true;

    for(i = 0; i < Header.nTracks + 1; i++)
    {
#ifdef __DEBUG
        printf("\n\r%i:\n\r", i);
#endif

        if(Tracks[i].dur)
        {
            songEnd = false;
            if(!--Tracks[i].dur) FT_NoteOff(i);
        }

        if(Tracks[i].curEvent >= Tracks[i].numEvents) continue;
        songEnd = false;

        if(!Tracks[i].counter)
        {
            if(Tracks[i].curEvent == 0 && Tracks[i].ticks)
            {
                Tracks[i].ticks++;
            }
        }

        if(++Tracks[i].counter >= Tracks[i].ticks)
        {
            Tracks[i].counter = 0;
            Put_PN(PAGE_2, segmentList[Tracks[i].currentSegment].allocatedSegmentNumber);
            while(Tracks[i].curEvent < Tracks[i].numEvents)
            {
                FT_ExecuteCommand(i);
                if(Tracks[i].curEvent >= Tracks[i].numEvents) break;
                else
                {
                    Tracks[i].ticks = FT_GetTrackData(i);
                    Tracks[i].ticks |= FT_GetTrackData(i) << 8;
                }
                if(Tracks[i].ticks) break;
            }
        }
    }

    return !songEnd;
}

//
// Check for ESC key pressed
//
boolean CheckEsc()
{
    OutPort(PPI_REG_C, (InPort(PPI_REG_C) & 0xF0) | 0x07);

    return (InPort(PPI_REG_B) & 0x04) ? false : true;
}

//
// Main
//
void main(char *argv[], int argc) 
{
    int i = 0;
    byte j = 0;
 
    printf("RoboSOP v0.5\n\r");
    printf("Copyright (C) 2019 RoboSoft Inc.\n\r");
    printf("\nPlays SOP files from the Note OPL3 Music Sequencer\n\r");

    if(argc < 1)
    {
        printf("Usage: RoboSOP <filename>\n\r");
        Exit(0);
    }

    tickBeat = 0;
    if(argc > 1)
    {
        if(!strcmp("/T", argv[1]))
        {
            tickBeat = atoi(argv[2]);
        }
    }

    InitRamMapperInfo(4);

    FT_AllocateSegments();
    FT_InitVolumeTable();
    FT_LoadSOPFile(argv[0]);

    FT_ResetSong();
    printf("\n\rPlaying... ESC to stop\n\r");

    DisableInterupt();
    FT_SetInterruptHandler(FT_InterruptHandler);
    FT_SetTimer2State(true);

    songContinue = true;
    JIFFY = false;

    while(!CheckEsc() && songContinue)
    {
        songContinue = FT_Update();

        EnableInterupt();
        while(!JIFFY);
        DisableInterupt();
        JIFFY = false;
    }

    DisableInterupt();
    FT_WriteOpl1(4,0x80);
    FT_SetTimer2State(false);
    FT_RestoreInterruptHandler();
    FT_ClearOPL();
    EnableInterupt();

    KillKeyBuffer();
    Exit(0);
}