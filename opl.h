//
// RoboSOP
//
// Copyright (C) 2019 by RoboSoft.
//
// Made with FUSION-C in SDCC

#ifndef __OPL_H
#define __OPL_H

#define OPL4_REG   0xC4
#define OPL4_DATA  0xC5
#define OPL4_REG2  0xC6
#define OPL4_DATA2 0xC7

void FT_WriteOpl1(byte addr, byte value);
void FT_WriteOpl3(byte addr, byte value);

void FT_SetTimer2Count(byte value);
void FT_SetTimer2State(boolean state);

void FT_Set4OPMode(byte channel, byte value);
void FT_SetPercussionMode(boolean mode);

void FT_InitVolumeTable();
void FT_OPL_Reset();
void FT_ClearOPL();

void FT_NoteOn(byte channel, byte pitch);
void FT_NoteOff(byte channel);
void FT_SetFreq(byte voice, byte note, int pitch, byte keyOn);
void FT_SetVoiceVolume(byte chan, byte vol);
void FT_SendIns(int base_addr, byte* value, boolean mode);
void FT_SetVoiceTimbre(byte chan, byte* array);
void FT_SetStereoPAN(byte chan, byte value);
void FT_SetVoicePitch(byte chan, byte pitch);

#endif