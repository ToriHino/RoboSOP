//
// RoboSOP 1.0
//
// Copyright (C) 2019 by RoboSoft.
//
// Made with FUSION-C in SDCC

//
// Allocate memory segments
//
void FT_AllocateSegments()
{
    int i = 0;

    MAPPERINFOBLOCK* pMapperInfoBlock = _GetRamMapperBaseTable();
    SEGMENTSTATUS* pSegmentStatus = 0;

    segmentList[0].allocatedSegmentNumber = Get_PN(PAGE_2);
    segmentList[0].slotAddressOfMapper = pMapperInfoBlock->slot;

    for(i = 1; i < MAX_NR_SEGMENTS; i++)
    {
        pSegmentStatus = AllocateSegment(0, 0x0);

        if(pSegmentStatus->carryFlag) break;
        
        segmentList[i].allocatedSegmentNumber = pSegmentStatus->allocatedSegmentNumber;
        segmentList[i].slotAddressOfMapper    = pSegmentStatus->slotAddressOfMapper;
    }
    segmentsFound = i;
#ifdef __DEBUG           
    printf("\n\rSegments found: %i\n\r", segmentsFound);
#endif
}

//
// FT_GetTrackData
//
byte FT_GetTrackData(byte track)
{   
    byte data = *(Tracks[track].currentData++);

#ifdef __DEBUG
    printf("%x=%i|", Tracks[track].currentData, data);
#endif

    if(Tracks[track].currentData == (byte *)(SOP_EVNT_BASE + PAGE_SIZE))
    {
        Tracks[track].currentSegment++;
        Put_PN(PAGE_2, segmentList[Tracks[track].currentSegment].allocatedSegmentNumber);
        Tracks[track].currentData = (byte *)SOP_EVNT_BASE;
    }

#ifdef __DEBUG
    printf("|");
#endif

    return data;
}

