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

typedef enum {
    PIXEL_TEST=0,
    LINE_TEST,
    HLINE_TEST,
    VLINE_TEST,
    RECTANGLE_TEST,
    CIRCLE_TEST,
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


EFI_STATUS RunGraphicTest(UINT32 Mode, GRAPHIC_TEST_TYPE TestType, UINT32 Duration, UINT32 Iterations, BOOLEAN ClipTest);
VOID PrintTestResults(VOID);


#endif // GRAPHICS_TEST_H
