//
// RoboSOP
//
// Copyright (C) 2019 by RoboSoft.
//
// Made with FUSION-C in SDCC

#include "fusion-c/header/msx_fusion.h"
#include "fusion-c/header/newTypes.h"
#include "RoboSOP.h"
#include "interrupt.h"
#include "opl.h"

#define RG1SAV 0xF3E0

boolean JIFFY;
byte oldInterruptHandler[3];

void FT_InterruptHandler(void) 
{
    byte status;

    __asm
    push af
    push bc
    push de
    push hl
    push ix
    push iy
    __endasm;

    status = InPort(OPL4_REG) & 0x80;
    if(status)
    {
        FT_WriteOpl1(4, 0x80);
        JIFFY = true;
    }

    __asm
    pop iy
    pop ix
    pop hl
    pop de
    pop bc
    pop af
    ei
    __endasm; 
}

//
// Program hook
//
void FT_SetInterruptHandler(fnPtr handler)
{
    unsigned int address = (unsigned int)handler;
    byte* mem = (byte *)0x0038;

    oldInterruptHandler[0] = Peek(0x38);
    oldInterruptHandler[1] = Peek(0x39);
    oldInterruptHandler[2] = Peek(0x3A);

    mem[0] = 0xC3; //JP
    mem[1] = (byte)(address & 0x00ff);
    mem[2] = (byte)((address & 0xff00) >> 8);

    OutPort(0x99, Peek(RG1SAV) & 0xDF);
    OutPort(0x99, 1 + 128);
}

void FT_RestoreInterruptHandler()
{
    byte* mem = (byte *)0x0038;

    mem[0] = oldInterruptHandler[0];
    mem[1] = oldInterruptHandler[1];
    mem[2] = oldInterruptHandler[2];

    OutPort(0x99, Peek(RG1SAV));
    OutPort(0x99, 1 + 128);
}
