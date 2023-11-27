/*
 * File:    Timer.c
 * 
 * Author:  David Petrovic
 * 
 * Description:
 * 
 * Timer using processor Time Stamp Counter (TSC)
 */

#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include "Timer.h"

#define CAL_TIME 100    // calibration time in ms

STATIC UINT64 gTscPerMs = 1;

/*
 * ReadTimer()
 */
UINT64 ReadTimer()
{
    UINT64 rdx,rax;
    asm volatile ("rdtsc":"=d" (rdx), "=a" (rax));
    return (rdx<<32)+rax;
}

/*
 * InitTimer()
 */
VOID InitTimer(VOID)
{
    // determine TSC interval
    UINT64 counter = ReadTimer();
    gBS->Stall(CAL_TIME * 1000); // us parameter
    gTscPerMs = (ReadTimer() - counter + CAL_TIME/2)/CAL_TIME;
//    Print(L"TSC/Sec: %ld\n", gTscPerMs);
}

/*
 * CalcMsTime()
 */
UINT64 CalcMsTime(UINT64 end, UINT64 start)
{
    return (end - start + gTscPerMs/2)/gTscPerMs;
}
