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

#if EDK2SIM_SUPPORT
#include <Edk2Sim.h>
#else
#define EDK2SIM_INIT_TIMER
#define EDK2SIM_READ_TIMER
#endif

#define CAL_TIME 100    // calibration time in ms

STATIC UINT64 gTscPerMs = 1;

/*
 * ReadTimer()
 */
UINT64 ReadTimer()
{
#if EDK2SIM_SUPPORT
    return EDK2SIM_READ_TIMER;
#else
    UINT64 rdx,rax;
    asm volatile ("rdtsc":"=d" (rdx), "=a" (rax));
    return (rdx<<32)+rax;
#endif
}

/*
 * InitTimer()
 */
VOID InitTimer(VOID)
{
#if EDK2SIM_SUPPORT
    gTscPerMs = EDK2SIM_INIT_TIMER;
#else
    // determine TSC interval
    UINT64 counter = ReadTimer();
    gBS->Stall(CAL_TIME * 1000); // us parameter
    gTscPerMs = (ReadTimer() - counter + CAL_TIME/2)/CAL_TIME;
//    Print(L"TSC/Sec: %ld\n", gTscPerMs);
#endif
}

/*
 * CalcMsTime()
 */
UINT64 CalcMsTime(UINT64 end, UINT64 start)
{
    return (end - start + gTscPerMs/2)/gTscPerMs;
}
