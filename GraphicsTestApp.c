/**

 GraphicsTest.c

**/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/ShellCEntryLib.h>
#include "CmdLineLib/CmdLine.h"
#include "GraphicsLib/Graphics.h"
#include "Timer.h"
#include "GraphicsTest.h"


// CmdLine: Enum definition for test types
ENUMSTR_START(GraphicTestEnumStrs)
ENUMSTR_ENTRY(ALL_TESTS,            L"all")
ENUMSTR_ENTRY(PIXEL_TEST,           L"pixel")
ENUMSTR_ENTRY(LINE_TEST,            L"line")
ENUMSTR_ENTRY(HLINE_TEST,           L"hline")
ENUMSTR_ENTRY(VLINE_TEST,           L"vline")
ENUMSTR_ENTRY(RECTANGLE_TEST,       L"rect")
ENUMSTR_ENTRY(CIRCLE_TEST,          L"circle")
ENUMSTR_ENTRY(FILL_RECTANGLE_TEST,  L"fillrect")
ENUMSTR_ENTRY(FILL_CIRCLE_TEST,     L"fillcircle")
ENUMSTR_ENTRY(TEXT1_TEST,           L"text")
ENUMSTR_ENTRY(TEXT2_TEST,           L"text2")
ENUMSTR_ENTRY(CLEAR_SCREEN_TEST,    L"clear")
ENUMSTR_ENTRY(BOUNCING_BALL_TEST,   L"ball")
ENUMSTR_END

// CmdLine: Variables
STATIC GRAPHIC_TEST_TYPE GraphicTest = ALL_TESTS;
STATIC BOOLEAN ClipEnable =  FALSE;
STATIC UINTN TimeParam = 2000;   // 2 second
STATIC UINTN NumParam = 0;
STATIC BOOLEAN GopInfo = FALSE;
STATIC UINT32 Mode = CURRENT_MODE;
STATIC BOOLEAN ProgVersion =  FALSE;

// CmdLine: Main program help
CHAR16 ProgHelpStr[]    = L"Graphics test";

// CmdLine: Switch table
SWTABLE_START(SwitchTable)
SWTABLE_OPT_ENUM(   L"-r",  L"-run",        &GraphicTest, GraphicTestEnumStrs,  L"[opt]run graphics test")
SWTABLE_OPT_FLAG(   L"-c",  L"-clip",       &ClipEnable,                        L"enable clipping during graphics test")
SWTABLE_OPT_DEC(    L"-t",  L"-time",       &TimeParam,                         L"[time]time parameter (ms)")
SWTABLE_OPT_DEC(    L"-n",  L"-number",     &NumParam,                          L"[num]number parameter")
SWTABLE_OPT_DEC32(  L"-m",  L"-mode",       &Mode,                              L"[num]set graphics mode")
SWTABLE_OPT_FLAG(   L"-i",  L"-info",       &GopInfo,                           L"graphics info")
SWTABLE_OPT_FLAG(   NULL,   L"-version",    &ProgVersion,                       L"program version")
SWTABLE_END


STATIC EFI_STATUS DisplayGopInfo();


INTN
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
    SHELL_STATUS ShellStatus = SHELL_SUCCESS;

    ShellStatus = ParseCmdLine(NULL, 0, SwitchTable, ProgHelpStr, NO_BREAK, NULL);
    if (ShellStatus != SHELL_SUCCESS) {
        return ShellStatus;
    }
    if (ProgVersion) {
        Print(L"Built: " __DATE__ " " __TIME__ "\n\n");
    } else if (GopInfo) {
        DisplayGopInfo();
    } else if (GraphicTest != NO_TEST) {
        EFI_STATUS Status = RunGraphicTest(Mode, GraphicTest, TimeParam, NumParam, ClipEnable);
        if (EFI_ERROR(Status)) {
            // Error should of already been displayed
            goto Error_exit;
        }
        PrintTestResults();
    }

Error_exit:
    return ShellStatus;
}


STATIC EFI_STATUS DisplayGopInfo()
{
    EFI_STATUS Status;

    Status = InitGraphics();
    if (EFI_ERROR(Status)) {
        Print(L"ERROR: Failed to initialise graphics (%r)\n", Status);
        goto Error_exit;
    }
    // List available graphics modes
    Print(L"Available graphics modes:\n");
    for (UINTN i=0; i<NumGraphicsModes(); i++) {
        UINT32 HorRes, VerRes;
        if (!EFI_ERROR(QueryGraphicsMode(i, &HorRes, &VerRes))) {
            Print(L"%2u: %ux%u\n", i , HorRes, VerRes);
        }
    }
    UINTN Mode;
    Status = GetGraphicsMode(&Mode);
    if (EFI_ERROR(Status)) {
        Print(L"ERROR: Failed to read current graphics mode (%r)\n", Status);
        goto Error_exit;
    }
    Print(L"Current Mode %d - %ux%u\n", Mode, GetFBHorRes(), GetFBVerRes());

Error_exit:
    return Status;
}