//
// RoboSOP
//
// Copyright (C) 2019 by RoboSoft.
//
// Made with FUSION-C in SDCC

#ifndef __INTERRUPT_H
#define __INTERRUPT_H

void FT_InterruptHandler(void);

void FT_SetInterruptHandler(fnPtr handler);
void FT_RestoreInterruptHandler();

#endif