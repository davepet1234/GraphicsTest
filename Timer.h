/*
 * File:    Timer.h
 * 
 * Author:  David Petrovic
 *
 * Description:
 * 
 * Timer using processor Time Stamp Counter (TSC)
 */

#ifndef TIMER_H
#define TIMER_H

#include <Uefi.h>

VOID InitTimer(VOID);
UINT64 ReadTimer(VOID);
UINT64 CalcMsTime(UINT64 end, UINT64 start);

#endif // TIMER_H
