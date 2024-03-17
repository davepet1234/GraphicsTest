/*
 * File:    GraphicTest.h
 * 
 * Author:  David Petrovic
 *
 * Description:
 * 
 * Tests to exercise the graphics primitives
 */

#ifndef GRAPHICS_TEST_H
#define GRAPHICS_TEST_H

#include <Uefi.h>

#define CURRENT_MODE 0xFFFF

// test type
typedef enum {
    PIXEL_TEST=0,
    LINE_TEST,
    HLINE_TEST,
    VLINE_TEST,
    TRIANGLE_TEST,
    RECTANGLE_TEST,
    CIRCLE_TEST,
    FILL_TRIANGLE_TEST,
    FILL_RECTANGLE_TEST,
    FILL_CIRCLE_TEST,
    TEXT1_TEST,
    TEXT2_TEST,
    CLEAR_SCREEN_TEST,
    BOUNCING_BALL_TEST,
    NUM_TESTS,          // number of tests defined
    ALL_TESTS,
    NO_TEST
} GRAPHIC_TEST_TYPE;

// test result data
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

EFI_STATUS RunGraphicTest(UINT32 Mode, GRAPHIC_TEST_TYPE TestType, UINT32 Duration, UINT32 Iterations, BOOLEAN ClipTest, BOOLEAN Pause, TEST_RESULTS *TestResults);
CHAR16 *GetTestDesc(GRAPHIC_TEST_TYPE type);


#endif // GRAPHICS_TEST_H
