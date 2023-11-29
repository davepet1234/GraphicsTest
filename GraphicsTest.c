/*
 * File:    GraphicsTest.c
 * 
 * Author:  David Petrovic
 * 
 * Description:
 * 
 * Tests to exercise the graphics primitives
 */
 
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h> //###
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/IoLib.h>
#include "GraphicsLib/Graphics.h"
#include "GraphicsTest.h"
#include "Timer.h"
#include "Rand.h"
#include "Font.h"

#define DbgPrint(Level, sFormat, ...)


#define MIN_CIRCLE_RADIUS   5
#define MIN_CIRCLE_DIAMETER (2 * MIN_CIRCLE_RADIUS)
#define CLIP_FACTOR         8


// local variables
typedef struct {
    BOOLEAN Run;    // true if test has been run
    UINT32 Count;   // number of iterations
    UINT32 Time;    // time taken
} TEST_RUN_DATA;

typedef struct {
    UINTN Mode;     // graphics mode used
    UINT32 HorRes;  // horizontial resolution
    UINT32 VerRes;  // vertical resolution
    TEST_RUN_DATA Data[NUM_TESTS];
} TEST_RESULTS;

TEST_RESULTS TestResults = {0};


// local functions
STATIC VOID RunTest(GRAPHIC_TEST_TYPE TestType, UINT32 Duration, UINT32 Iterations);
STATIC CHAR16 *GetTestDesc(GRAPHIC_TEST_TYPE type);
STATIC VOID RunRandPixelTest(UINT32 Duration, UINT32 Iterations);
STATIC VOID RunRandLineTest(UINT32 Duration, UINT32 Iterations);
STATIC VOID RunRandHLineTest(UINT32 Duration, UINT32 Iterations);
STATIC VOID RunRandVLineTest(UINT32 Duration, UINT32 Iterations);
STATIC VOID RunRandRectangleTest(UINT32 Duration, UINT32 Iterations, BOOLEAN Filled);
STATIC VOID RunRandCircleTest(UINT32 Duration, UINT32 Iterations, BOOLEAN Filled);
STATIC VOID RunRandTextTest(UINT32 Duration, UINT32 Iterations, BOOLEAN SetBackground);
STATIC VOID RunClearScreenTest(UINT32 Duration, UINT32 Iterations);
STATIC VOID RunBouncingBallTest(UINT32 Duration, UINT32 Iterations);
//STATIC VOID WaitKeyPress(VOID);


/*
 * RunGraphicTest()
 */
EFI_STATUS RunGraphicTest(UINT32 Mode, GRAPHIC_TEST_TYPE TestType, UINT32 Duration, UINT32 Iterations, BOOLEAN ClipTest)
{
    EFI_STATUS Status;

    if (!Duration && !Iterations) {
        DbgPrint(DL_WARN, "%s() zero duration and iterations", __func__);
        return EFI_INVALID_PARAMETER;
    }
    InitTimer();
    Status = InitGraphics();
    if (EFI_ERROR(Status)) {
        Print(L"ERROR: Failed to initialise graphics (%r)\n", Status);
        return Status;
    }
    if (Mode != CURRENT_MODE) {
        Status = SetGraphicsMode(Mode);
        if (EFI_ERROR(Status)) {
            Print(L"ERROR: Failed to set graphics mode %u (%r)\n", Mode, Status);
            return Status;
        }
    }
    UINTN CurrMode;
    GetGraphicsMode(&CurrMode);
    TestResults.Mode = CurrMode;
    TestResults.HorRes = GetFBHorRes();
    TestResults.VerRes = GetFBVerRes();
    ClearScreen(0);
    if (ClipTest) {
        INT32 HorOff = GetFBHorRes() / CLIP_FACTOR;
        INT32 VerOff = GetFBVerRes() / CLIP_FACTOR;
        SetClipping(HorOff, VerOff, GetFBHorRes() - HorOff - 1, GetFBVerRes() - VerOff - 1);
    }    
    if (TestType == ALL_TESTS) {
        for (UINTN i=0; i<NUM_TESTS; i++) {
            RunTest(i, Duration, Iterations);
        }         
    } else {
        RunTest(TestType, Duration, Iterations);
    }
    RestoreConsole();

    return EFI_SUCCESS;
}

/*
 * RunTest()
 */
STATIC VOID RunTest(GRAPHIC_TEST_TYPE TestType, UINT32 Duration, UINT32 Iterations)
{
    ClearScreen(BLACK);
    Srand(1);
    switch(TestType) {            
    case PIXEL_TEST:
        RunRandPixelTest(Duration, Iterations);
        break;
    case LINE_TEST:
        RunRandLineTest(Duration, Iterations);
        break;            
    case HLINE_TEST:
        RunRandHLineTest(Duration, Iterations);
        break;            
    case VLINE_TEST:
        RunRandVLineTest(Duration, Iterations);
        break;            
    case RECTANGLE_TEST:
        RunRandRectangleTest(Duration, Iterations, FALSE);
        break;            
    case FILL_RECTANGLE_TEST:
        RunRandRectangleTest(Duration, Iterations, TRUE);
        break;            
    case CIRCLE_TEST:
        RunRandCircleTest(Duration, Iterations, FALSE);
        break;
    case FILL_CIRCLE_TEST:
        RunRandCircleTest(Duration, Iterations, TRUE);
        break;
    case TEXT1_TEST:
        RunRandTextTest(Duration, Iterations, TRUE);
        break;
    case TEXT2_TEST: // transparent text background
        RunRandTextTest(Duration, Iterations, FALSE);
        break;
    case CLEAR_SCREEN_TEST:
        RunClearScreenTest(Duration, Iterations);
        break;
    case BOUNCING_BALL_TEST:
        RunBouncingBallTest(Duration, Iterations);
        break;
    default:
        DbgPrint(DL_ERROR, "Invalid graphics test (%u)\n", TestType);
        break;
    }
}

/*
 * PrintTestResults()
 */
VOID PrintTestResults(VOID)
{
    Print(L"Mode %u - %ux%u\n", TestResults.Mode, TestResults.HorRes, TestResults.VerRes);
    Print(L"Test        Iterations  Time\n");
    for (UINTN i=0; i<NUM_TESTS; i++) {
        if (TestResults.Data[i].Run) {
            Print(L"%-10s : %9u %5u\n", GetTestDesc(i), TestResults.Data[i].Count, TestResults.Data[i].Time);
        }
    }
}

/*
 * GetTestDesc()
 */
STATIC CHAR16 *GetTestDesc(GRAPHIC_TEST_TYPE type)
{
    switch (type) {
    case PIXEL_TEST:
        return L"Pixel";
    case LINE_TEST:
        return L"Line";
    case HLINE_TEST:
        return L"HorLine";
    case VLINE_TEST:
        return L"VerLine";
    case RECTANGLE_TEST:
        return L"Rect";
    case CIRCLE_TEST:
        return L"Circle";
    case FILL_RECTANGLE_TEST:
        return L"FillRect";
    case FILL_CIRCLE_TEST:
        return L"FillCircle";
    case TEXT1_TEST:
        return L"Text";
    case TEXT2_TEST:
        return L"Text2";
    case CLEAR_SCREEN_TEST:
        return L"ClearScn";
    case BOUNCING_BALL_TEST:
        return L"BounBall";
    default:
        break;
    }
    return L"Unknown";
}

/*
 * RunRandPixelTest()
 */
STATIC VOID RunRandPixelTest(UINT32 Duration, UINT32 Iterations)
{
    UINT32 DisplayWidth = GetFBHorRes();
    UINT32 DisplayHeight = GetFBVerRes();
    UINT32 Count = 0;
    UINT64 StartTime = ReadTimer();
    UINT64 EndTime = StartTime;
    while (TRUE) {
        UINT32 colour = Rand() % 0x1000000;

        INT32 x0 = Rand() % DisplayWidth;
        INT32 y0 = Rand() % DisplayHeight;
        PutPixel(x0, y0, colour);

        Count++;
        EndTime = ReadTimer();
        if (Duration && CalcMsTime(EndTime, StartTime) >= Duration) break;
        if (Iterations && (Count >= Iterations)) break;
    }
    TEST_RUN_DATA *Results = &TestResults.Data[PIXEL_TEST];
    Results->Run = TRUE;
    Results->Count = Count;
    Results->Time = CalcMsTime(EndTime, StartTime);
}

/*
 * RunRandLineTest()
 */
STATIC VOID RunRandLineTest(UINT32 Duration, UINT32 Iterations)
{
    INT32 DisplayWidth = GetFBHorRes();
    INT32 DisplayHeight = GetFBVerRes();
    UINT32 Count = 0;
    UINT64 StartTime = ReadTimer();
    UINT64 EndTime = StartTime;
    while (TRUE) {
        UINT32 colour = Rand() % 0x1000000;

        INT32 x0 = Rand() % DisplayWidth;
        INT32 y0 = Rand() % DisplayHeight;
        INT32 x1 = Rand() % DisplayWidth;
        INT32 y1 = Rand() % DisplayHeight;
        DrawLine(x0, y0, x1, y1, colour);

        Count++;
        EndTime = ReadTimer();
        if (Duration && CalcMsTime(EndTime, StartTime) >= Duration) break;
        if (Iterations && (Count >= Iterations)) break;
    }
    TEST_RUN_DATA *Results = &TestResults.Data[LINE_TEST];
    Results->Run = TRUE;
    Results->Count = Count;
    Results->Time = CalcMsTime(EndTime, StartTime);
}

/*
 * RunRandHLineTest()
 */
STATIC VOID RunRandHLineTest(UINT32 Duration, UINT32 Iterations)
{
    INT32 DisplayWidth = GetFBHorRes();
    INT32 DisplayHeight = GetFBVerRes();
    UINT32 Count = 0;
    UINT64 StartTime = ReadTimer();
    UINT64 EndTime = StartTime;
    while (TRUE) {
        UINT32 colour = Rand() % 0x1000000;
#if 1
        INT32 x0 = Rand() % DisplayWidth;
        INT32 x1 = Rand() % DisplayWidth;
        INT32 y0 = Rand() % DisplayHeight;
        UINT32 w = ABS(x0-x1);
        DrawHLine((x0 > x1) ? x1 : x0, y0, w, colour);
#else
        INT32 x0 = Rand() % (DisplayWidth/2);
        INT32 y0 = Rand() % DisplayHeight;
        INT32 width = Rand() % (DisplayWidth - x0);
        DrawHLine(x0, y0, width, colour);
#endif
        Count++;
        EndTime = ReadTimer();
        if (Duration && CalcMsTime(EndTime, StartTime) >= Duration) break;
        if (Iterations && (Count >= Iterations)) break;
    }
    TEST_RUN_DATA *Results = &TestResults.Data[HLINE_TEST];
    Results->Run = TRUE;
    Results->Count = Count;
    Results->Time = CalcMsTime(EndTime, StartTime);
}

/*
 * RunRandVLineTest()
 */
STATIC VOID RunRandVLineTest(UINT32 Duration, UINT32 Iterations)
{
    INT32 DisplayWidth = GetFBHorRes();
    INT32 DisplayHeight = GetFBVerRes();
    UINT32 Count = 0;
    UINT64 StartTime = ReadTimer();
    UINT64 EndTime = StartTime;
    while (TRUE) {
        UINT32 colour = Rand() % 0x1000000;
#if 1
        INT32 x0 = Rand() % DisplayWidth;
        INT32 y0 = Rand() % DisplayHeight;
        INT32 y1 = Rand() % DisplayHeight;
        UINT32 h = ABS(y0-y1);
        DrawVLine(x0, (y0 > y1) ? y1 : y0, h, colour);
#else        
        INT32 x0 = Rand() % DisplayWidth;
        INT32 y0 = Rand() % (DisplayHeight/2);
        INT32 height = Rand() % (DisplayHeight - y0);
        DrawVLine(x0, y0, height, colour);
#endif
        Count++;
        EndTime = ReadTimer();
        if (Duration && CalcMsTime(EndTime, StartTime) >= Duration) break;
        if (Iterations && (Count >= Iterations)) break;
    }
    TEST_RUN_DATA *Results = &TestResults.Data[VLINE_TEST];
    Results->Run = TRUE;
    Results->Count = Count;
    Results->Time = CalcMsTime(EndTime, StartTime);
}

/*
 * RunRandRectangleTest()
 */
STATIC VOID RunRandRectangleTest(UINT32 Duration, UINT32 Iterations, BOOLEAN Filled)
{
    INT32 DisplayWidth = GetFBHorRes();
    INT32 DisplayHeight = GetFBVerRes();
    UINT32 Count = 0;
    UINT64 StartTime = ReadTimer();
    UINT64 EndTime = StartTime;
    while (TRUE) {
        UINT32 colour = Rand() % 0x1000000;

        INT32 x0 = Rand() % DisplayWidth;
        INT32 y0 = Rand() % DisplayHeight;
        INT32 x1 = Rand() % DisplayWidth;
        INT32 y1 = Rand() % DisplayHeight;
        if (Filled) {
            DrawFillRectangle(x0, y0, x1, y1, colour);
        } else {
            DrawRectangle(x0, y0, x1, y1, colour);
        }

        Count++;
        EndTime = ReadTimer();
        if (Duration && CalcMsTime(EndTime, StartTime) >= Duration) break;
        if (Iterations && (Count >= Iterations)) break;
    }
    TEST_RUN_DATA *Results = Filled ? &TestResults.Data[FILL_RECTANGLE_TEST] : &TestResults.Data[RECTANGLE_TEST];
    Results->Run = TRUE;
    Results->Count = Count;
    Results->Time = CalcMsTime(EndTime, StartTime);
}

/*
 * RunRandCircleTest()
 */
STATIC VOID RunRandCircleTest(UINT32 Duration, UINT32 Iterations, BOOLEAN Filled)
{
    INT32 DisplayWidth = GetFBHorRes();
    INT32 DisplayHeight = GetFBVerRes();
    UINT32 Count = 0;
    UINT64 StartTime = ReadTimer();
    UINT64 EndTime = StartTime;
    while (TRUE) {
        UINT32 colour = Rand() % 0x1000000;
#if 0
        INT32 x0 = Rand() % DisplayWidth;
        INT32 y0 = Rand() % DisplayHeight;
        INT32 dist1 = x0 > DisplayWidth-x0 ? DisplayWidth-x0 : x0;
        INT32 dist2 = y0 > DisplayHeight-y0 ? DisplayHeight-y0 : y0;
        INT32 r = Rand() % (dist1 > dist2 ? dist2 : dist1);
        if (Filled) {
            DrawFillCircle(x0, y0, r, colour);
        } else {
            DrawCircle(x0, y0, r, colour);
        }
#else
        UINT32 xc = MIN_CIRCLE_RADIUS + Rand() % (DisplayWidth - MIN_CIRCLE_DIAMETER);
        UINT32 yc = MIN_CIRCLE_RADIUS + Rand() % (DisplayHeight - MIN_CIRCLE_DIAMETER);
        UINT32 dist1 = xc < yc ? xc : yc;
        UINT32 dist2 = DisplayWidth - xc; 
        dist1 = dist1 < dist2 ? dist1 : dist2;
        dist2 = DisplayHeight - yc;
        dist1 = dist1 < dist2 ? dist1 : dist2;
        UINT32 radius;
        if (dist1 !=  0) {
            radius = Rand() % dist1;
            if (Filled) {
                DrawFillCircle(xc, yc, radius, colour);
            } else {
                DrawCircle(xc, yc, radius, colour);
            }
        } else {
            DbgPrint(DL_ERROR, "%u: %3u %3u %3u\n", Count, xc, yc, dist1);
        }
#endif

        Count++;
        EndTime = ReadTimer();
        if (Duration && CalcMsTime(EndTime, StartTime) >= Duration) break;
        if (Iterations && (Count >= Iterations)) break;
    }
    TEST_RUN_DATA *Results = Filled ? &TestResults.Data[FILL_CIRCLE_TEST] : &TestResults.Data[CIRCLE_TEST];
    Results->Run = TRUE;
    Results->Count = Count;
    Results->Time = CalcMsTime(EndTime, StartTime);
}

/*
 * RunRandTextTest()
 */
STATIC VOID RunRandTextTest(UINT32 Duration, UINT32 Iterations, BOOLEAN SetBackground)
{
    FONT font = FONT10x20;
    INT32 DisplayWidth = GetFBHorRes();
    INT32 DisplayHeight = GetFBVerRes();
    UINT16 Message[] = L"The quick brown fox jumps over the lazy dog.";
    INT32 width = StrLen(Message) * GetFontWidth(font);
    INT32 height = GetFontHeight(font);
    UINT32 Count = 0;
    UINT64 StartTime = ReadTimer();
    UINT64 EndTime = StartTime;
    while (TRUE) {
        UINT32 colour = Rand() % 0x1000000;

        INT32 x = Rand() % (DisplayWidth - width);
        INT32 y = Rand() % (DisplayHeight - height);
        GPutString(x, y, Message, colour, ~colour, SetBackground, font);

        Count++;
        EndTime = ReadTimer();
        if (Duration && CalcMsTime(EndTime, StartTime) >= Duration) break;
        if (Iterations && (Count >= Iterations)) break;
    }
    TEST_RUN_DATA *Results = SetBackground ? &TestResults.Data[TEXT1_TEST] : &TestResults.Data[TEXT2_TEST];
    Results->Run = TRUE;
    Results->Count = Count;
    Results->Time = CalcMsTime(EndTime, StartTime);
}

/*
 * RunClearScreenTest()
 */
STATIC VOID RunClearScreenTest(UINT32 Duration, UINT32 Iterations)
{
    UINT32 Count = 0;
    UINT64 StartTime = ReadTimer();
    UINT64 EndTime = StartTime;
    while (TRUE) {
        UINT32 colour = Rand() % 0x1000000;

        if (Clipped()) {
            ClearClipWindow(colour);
        } else {
            ClearScreen(colour);
        }

        Count++;
        EndTime = ReadTimer();
        if (Duration && CalcMsTime(EndTime, StartTime) >= Duration) break;
        if (Iterations && (Count >= Iterations)) break;
    }
    TEST_RUN_DATA *Results = &TestResults.Data[CLEAR_SCREEN_TEST];
    Results->Run = TRUE;
    Results->Count = Count;
    Results->Time = CalcMsTime(EndTime, StartTime);
}

/*
 * RunBouncingBallTest()
 */
STATIC VOID RunBouncingBallTest(UINT32 Duration, UINT32 Iterations)
{
    EFI_STATUS Status;

    if (!Duration && !Iterations) {
        DbgPrint(DL_WARN, "%a() - Zero Duration and Iterations!", __func__);
        goto error_exit;
    }
    RENDER_BUFFER RenBuf;
    INT32 Radius = 100;
    INT32 PixSize = (2*Radius + 1) + 2;
    Status = CreateRenderBuffer(&RenBuf, PixSize, PixSize);
    if (EFI_ERROR(Status)) goto error_exit;
    Status = SetRenderBuffer(&RenBuf);
    if (EFI_ERROR(Status))  goto error_exit;
    for (INT32 i=Radius; i>=0; i--) {
        // colour from [55 -> 255] i.e. range of 200
        DrawFillCircle(Radius+1, Radius+1, i, RGB_COLOUR(0, 255-((200*i + Radius/2)/Radius), 0));
    }            
    INT32 x = Radius+1;   // centre of circle
    INT32 y = Radius+1;
    INT32 dx = 1;
    INT32 dy = 1;
    UINT32 Count = 0;
    UINT64 StartTime = ReadTimer();
    UINT64 EndTime = StartTime;
    while (TRUE) {
        //gBS->Stall(1000);
        Status = DisplayRenderBuffer(&RenBuf, x-Radius, y-Radius);
        if (EFI_ERROR(Status)) break;
        x += dx;
        y += dy;
        if (x-Radius <= 0) dx = 1;
        if (y-Radius <= 0) dy = 1; 
        if (x+Radius >= GetFBHorRes()-1) dx = -1;
        if (y+Radius >= GetFBVerRes()-1) dy = -1;
        Count++;
        EndTime = ReadTimer();
        if (Duration && CalcMsTime(EndTime, StartTime) >= Duration) {
            break;
        }
        if (Iterations && (Count >= Iterations)) {
            break;
        }
    }
    TEST_RUN_DATA *Results = &TestResults.Data[BOUNCING_BALL_TEST];
    Results->Run = TRUE;
    Results->Count = Count;
    Results->Time = CalcMsTime(EndTime, StartTime);

error_exit:    
    DestroyRenderBuffer(&RenBuf);
    SetScreenRender();
}

#if 0
/*
 * WaitKeyPress()
 */
STATIC VOID WaitKeyPress(VOID)
{
    EFI_INPUT_KEY key;
    UINTN index;
    // flush keyboard
    while (!EFI_ERROR(gST->ConIn->ReadKeyStroke(gST->ConIn, &key)));
    // wait for key
    if (!EFI_ERROR(gST->BootServices->WaitForEvent(1, &gST->ConIn->WaitForKey, &index))) {
        // read key pressed
        while (!EFI_ERROR(gST->ConIn->ReadKeyStroke(gST->ConIn, &key)));
    }
}
#endif 
