//
// RoboSOP 1.0
//
// Copyright (C) 2019 by RoboSoft.
//
// Made with FUSION-C in SDCC

//
// Load SOP instruments
//
void FT_LoadSOPInstruments(int fH)
{
    byte i = 0;

    printf("Number of instruments: %i\n\r", Header.nInsts);
    Instruments = (SOP_INST*)INSTRUMENTS_BASE;
    for(i = 0; i < Header.nInsts; i++)
    {
        Read(fH, &Instruments[i], 1+8+19);  // instType + shortName + longName
        switch(Instruments[i].instType)
        {
            case INSTRUMENT_TYPE_UNUSED:
                break;
            case INSTRUMENT_TYPE_MELODY_4OP:
                Read(fH, &Instruments[i].instData, INSTRUMENT_DATA_SIZE_4OP);
                break;
            default:
                Read(fH, &Instruments[i].instData, INSTRUMENT_DATA_SIZE_2OP);
        }
    }

}

//
// Load SOP track data
//
void FT_LoadSOPTrackData(int fH)
{
    int i = 0;
    int n = 0;

    unsigned long dataSize = 0;
    unsigned int  readSize = 0;
    unsigned int  pageLeft = PAGE_SIZE;

    byte* readAddress = (byte *)SOP_EVNT_BASE;
    byte currentSegment = 1;
    Put_PN(PAGE_2, segmentList[currentSegment].allocatedSegmentNumber);

#ifdef __DEBUG
    printf("nTracks: %i [%i]\n\r", Header.nTracks, sizeof(SOP_TRK));
#endif

    for(i = 0; i < Header.nTracks+1; i++)
    {
#ifdef __DEBUG        
        printf("%i: ", i);
#endif

        Tracks[i].startSegment = currentSegment;
        Tracks[i].startData = readAddress;

        Read(fH, &Tracks[i].numEvents, sizeof(Tracks[i].numEvents));
        Read(fH, &dataSize, sizeof(dataSize));

        while(dataSize > 0)
        {
            readSize = (dataSize < pageLeft) ? dataSize : pageLeft;
#ifdef __DEBUG        
            printf("(%x)", readSize);
#endif

            n = Read(fH, readAddress, readSize);

            dataSize -= n;
            readAddress += n;
            pageLeft -= n;
#ifdef __DEBUG        
            printf(" (%x, %x) ", readSize, pageLeft);
#endif

            if(pageLeft == 0)
            {
                if(currentSegment < segmentsFound)
                {
                    currentSegment++;
                    Put_PN(PAGE_2, segmentList[currentSegment].allocatedSegmentNumber);
                    pageLeft = PAGE_SIZE;
                    readAddress = (byte *)SOP_EVNT_BASE;
                }
                else
                {
                    Exit(_NORAM);
                }
            }

        }

#ifdef __DEBUG        
        printf("\n\r");
#endif
    }
}

//
// Load a SOP file
//
void FT_LoadSOPFile(char* file_name)
{
    int fH = 0;
    byte i = 0;
    
    printf("Loading: %s\n\r", file_name);
    fH = Open(file_name, O_RDONLY);
    if(fH == -1)
    {
        Exit(__NOFIL);
    }

    Read(fH, &Header, sizeof(Header));
    if(strncmp(Header.signature,"sopepos", 7) != 0)
    {
        printf("Not a SOP file\n\r");
        Close(fH);
        Exit(__INTER);
    }
    if((Header.majorVersion != '\0') || (Header.minorVersion != '\1'))
    {
        printf("This program is designed for SOP format version 0.1\n\r");
        Close(fH);
        Exit(__INTER);
    }

    if(Header.basicTempo == 0) Header.basicTempo = SOP_DEF_TEMPO;
    if(tickBeat) Header.tickBeat = tickBeat;

    printf("\nSong title: %s\n\r", Header.title);
    printf("Tempo: %i BPM (Ticks per beat: %i, beatMeasure: %i)\n\r", Header.basicTempo, Header.tickBeat, Header.beatMeasure);

    chanMode = (byte*)MMalloc(Header.nTracks);
    Read(fH, chanMode, Header.nTracks);

    FT_LoadSOPInstruments(fH);
    FT_LoadSOPTrackData(fH);

    Close(fH);
}
