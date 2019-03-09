//
// RoboSOP
//
// Copyright (C) 2019 by RoboSoft.
//
// Made with FUSION-C in SDCC

#ifndef __ROBOSOP_H
#define __ROBOSOP_H

//#define __DEBUG         // Enable for debug output

#define VOLUME_TABLE_BASE 0x8000
#define VOLUME_TABLE_SIZE 0x2000

#define INSTRUMENTS_BASE  (VOLUME_TABLE_BASE + VOLUME_TABLE_SIZE)
#define SOP_EVNT_BASE     0x8000

#define REGISTER_BUFFER      0x5000
#define REGISTER_BUFFER_SIZE 8

#define PAGE_SIZE        0x4000
#define PAGE_0           0
#define PAGE_1           1   
#define PAGE_2           2
#define PAGE_3           3

#define VOLUME_SEGMENT     0
#define INSTRUMENT_SEGMENT 0

#define MAX_NR_SEGMENTS 256

#define SOP_DEF_TEMPO 120

#define SOP_MAX_TRACK   24
#define SOP_MAX_VOL     127

#define SOP_CHAN_NONE   0
#define SOP_CHAN_4OP    1
#define SOP_CHAN_2OP    2

#define INSTRUMENT_TYPE_MELODY_4OP 0
#define INSTRUMENT_TYPE_MELODY_2OP 1
#define INSTRUMENT_TYPE_BASS_DRUM  6
#define INSTRUMENT_TYPE_SNARE_DRUM 7
#define INSTRUMENT_TYPE_TOM_TOM    8
#define INSTRUMENT_TYPE_CYMBAL     9
#define INSTRUMENT_TYPE_HI_HAT     10
#define INSTRUMENT_TYPE_UNUSED     12

#define INSTRUMENT_DATA_SIZE_2OP  11
#define INSTRUMENT_DATA_SIZE_4OP  22

#define SOP_EVNT_SPEC   1   /* Special event */
#define SOP_EVNT_NOTE   2   /* Note event */
#define SOP_EVNT_TEMPO  3   /* Tempo event */
#define SOP_EVNT_VOL    4   /* Volume event */
#define SOP_EVNT_PITCH  5   /* Pitch event */
#define SOP_EVNT_INST   6   /* Instrument event */
#define SOP_EVNT_PAN    7   /* Panning event */
#define SOP_EVNT_MVOL   8   /* Master volume event */

typedef struct
{
    char signature[7];
    byte majorVersion;
    byte minorVersion;
    byte padding_1;
    char fileName[13];
    char title[31];
    byte percussive;
    byte padding_2;
    byte tickBeat;
    byte padding_3;
    byte beatMeasure;
    byte basicTempo;
    char comment[13];
    byte nTracks;
    byte nInsts;
    byte padding_4;
} SOP_HEADER;

typedef struct
{
    byte instType;
    char shortName[8];
    char longName[19];
    byte instData[22];
} SOP_INST;

typedef struct
{
    uint numEvents;
    uint curEvent;

    byte  startSegment;
    byte* startData;
    byte  currentSegment;
    byte* currentData;

    uint ticks;
    uint dur;
    uint counter;
} SOP_TRK;

typedef struct 
{
    byte allocatedSegmentNumber;
    byte slotAddressOfMapper;   
} SEGMENT_INFO;

typedef void (*fnPtr)(void);

extern SEGMENT_INFO segmentList[MAX_NR_SEGMENTS];
extern int segmentsFound;
extern int tickBeat;

extern SOP_HEADER Header;
extern SOP_INST*  Instruments;
extern SOP_TRK    Tracks[SOP_MAX_TRACK+1];

extern byte* chanMode;

extern byte volume[SOP_MAX_TRACK];
extern byte lastvol[SOP_MAX_TRACK];
extern byte masterVolume;

extern boolean songContinue;
extern boolean JIFFY;

#endif