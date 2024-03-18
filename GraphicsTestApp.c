/**

 GraphicsTest.c

**/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/ShellCEntryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include "CmdLineLib/CmdLine.h"
#include "GraphicsLib/Graphics.h"
#include "Timer.h"
#include "Rand.h"
#include "GraphicsTest.h"

// CmdLine: Enum definition for test types
ENUMSTR_START(GraphicTestEnumStrs)
ENUMSTR_ENTRY(ALL_TESTS,            L"all")
ENUMSTR_ENTRY(PIXEL_TEST,           L"pixel")
ENUMSTR_ENTRY(LINE_TEST,            L"line")
ENUMSTR_ENTRY(HLINE_TEST,           L"hline")
ENUMSTR_ENTRY(VLINE_TEST,           L"vline")
ENUMSTR_ENTRY(TRIANGLE_TEST,        L"triangle")
ENUMSTR_ENTRY(RECTANGLE_TEST,       L"rectangle")
ENUMSTR_ENTRY(CIRCLE_TEST,          L"circle")
ENUMSTR_ENTRY(FILL_TRIANGLE_TEST,   L"ftriangle")
ENUMSTR_ENTRY(FILL_RECTANGLE_TEST,  L"frectangle")
ENUMSTR_ENTRY(FILL_CIRCLE_TEST,     L"fcircle")
ENUMSTR_ENTRY(TEXT1_TEST,           L"text")
ENUMSTR_ENTRY(TEXT2_TEST,           L"text2")
ENUMSTR_ENTRY(CLEAR_SCREEN_TEST,    L"clear")
ENUMSTR_ENTRY(BOUNCING_BALL_TEST,   L"ball")
ENUMSTR_END

// CmdLine: Variables
#define MAX_FILENAME_LEN 256
STATIC GRAPHIC_TEST_TYPE GraphicTest = ALL_TESTS;
STATIC BOOLEAN ClipEnable =  FALSE;
STATIC UINTN TimeParam = 2000;   // 2 second
STATIC UINTN NumParam = 0;
STATIC BOOLEAN GopInfo = FALSE;
STATIC UINT32 Mode = CURRENT_MODE;
STATIC BOOLEAN ProgVersion =  FALSE;
STATIC BOOLEAN AllModes = FALSE;
STATIC CHAR16 Filename[MAX_FILENAME_LEN];
STATIC BOOLEAN Pause = FALSE;
STATIC BOOLEAN DevFlag = FALSE;

// CmdLine: Main program help
CHAR16 ProgHelpStr[]    = L"Graphics test";

// CmdLine: Switch table
SWTABLE_START(SwitchTable)
SWTABLE_OPT_ENUM(   L"-r",  L"-run",        &GraphicTest, GraphicTestEnumStrs,  L"[opt]run graphics test")
SWTABLE_OPT_FLAG(   L"-c",  L"-clip",       &ClipEnable,                        L"enable clipping during graphics test")
SWTABLE_OPT_DEC(    L"-t",  L"-time",       &TimeParam,                         L"[time]time parameter (ms)")
SWTABLE_OPT_DEC(    L"-n",  L"-number",     &NumParam,                          L"[num]number parameter")
SWTABLE_OPT_DEC32(  L"-m",  L"-mode",       &Mode,                              L"[num]set graphics mode (0...n)")
SWTABLE_OPT_FLAG(   L"-a",  L"-allmodes",   &AllModes,                          L"run for all available graphics modes")
SWTABLE_OPT_FLAG(   L"-p",  L"-pause",      &Pause,                             L"pause after each test")
SWTABLE_OPT_FLAG(   L"-i",  L"-info",       &GopInfo,                           L"graphics info")
SWTABLE_OPT_STR(    L"-f",  L" -file",      Filename, MAX_FILENAME_LEN,         L"[filename]Write results to file")
SWTABLE_OPT_FLAG(   NULL,   L"-version",    &ProgVersion,                       L"program version")
SWTABLE_OPT_FLAG(   NULL,   L"-dev",        &DevFlag,                           L"development")
SWTABLE_END


STATIC EFI_STATUS DisplayGopInfo(VOID);
STATIC EFI_STATUS CheckFile(CHAR16 *Filename);
STATIC EFI_STATUS OutputTestResults(IN EFI_TIME *StartTime, IN EFI_TIME *EndTime, IN BOOLEAN ClipEnabled, IN TEST_RESULTS *Results, IN UINTN NumResults, IN CHAR16 *Filename);
STATIC EFI_STATUS EFIAPI OutputString(IN SHELL_FILE_HANDLE FileHandle, IN CONST CHAR16 *FormatString, ...);
STATIC VOID DevCode();

/*
 * ShellAppMain() - Main entry point for shell application
 */
INTN
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
    SHELL_STATUS ShellStatus = SHELL_SUCCESS;
    EFI_STATUS Status = EFI_SUCCESS;
    TEST_RESULTS *TestResults = NULL;
    UINT32 *ModeList = NULL;

    Filename[0] = '\0';

    // Parse command line options
    ShellStatus = ParseCmdLine(NULL, 0, SwitchTable, ProgHelpStr, NO_BREAK, NULL);
    if (ShellStatus != SHELL_SUCCESS) {
        return ShellStatus;
    }

    if (ProgVersion) {
        Print(L"Built: " __DATE__ " " __TIME__ "\n");
        goto App_exit;
    }

    // Initialise graphics lib so we can determine the number of modes
    Status = InitGraphics();
    if (EFI_ERROR(Status)) {
        Print(L"ERROR: Failed to initialise graphics (%r)\n", Status);
        goto App_exit;
    }
    UINT32 NumModes = NumGraphicsModes();
    if (EFI_ERROR(Status)) {
        Print(L"ERROR: Number of graphic modes is zero\n");
        goto App_exit;
    }
    
    if (GopInfo) {
        DisplayGopInfo();
        goto App_exit;
    }

    // Check we can create file before running tests
    Status = CheckFile(Filename);
    if (EFI_ERROR(Status)) {
        goto App_exit;
    }

    // Development
    if (DevFlag) {
        GraphicTest = NO_TEST;
        DevCode();
    }

    // Run tests
    if (GraphicTest != NO_TEST) {
        EFI_TIME StartTime;
        EFI_TIME EndTime;
        TestResults = (TEST_RESULTS *)AllocatePool((AllModes ? NumModes : 1) * sizeof(TEST_RESULTS));
        if (!TestResults) {
            Status = EFI_OUT_OF_RESOURCES;
            Print(L"ERROR: Failed to allocate memory for test results\n");
            goto App_exit;
        }
        gST->RuntimeServices->GetTime(&StartTime, (EFI_TIME_CAPABILITIES*)NULL);
        if (AllModes) {
            // All graphic modes
            ModeList = (UINT32 *)AllocatePool(NumModes * sizeof(UINT32));
            for (UINTN i = 0; i < NumModes; i++) {  // initialise mode list
                ModeList[i] = i;
            }
            // Sort modes by resolution - bubble sort
            for (UINTN i = 0; i < NumModes - 1; i++) {
                for (UINTN j = 0; j < NumModes - i - 1; j++) {
                    UINT32 HorRes1, VerRes1;
                    Status = QueryGraphicsMode(ModeList[j], &HorRes1, &VerRes1);
                    if (EFI_ERROR(Status)) {
                        goto App_exit;
                    }
                    UINT32 HorRes2, VerRes2;
                    Status = QueryGraphicsMode(ModeList[j + 1], &HorRes2, &VerRes2);
                    if (EFI_ERROR(Status)) {
                        goto App_exit;
                    }
                    if ((UINTN)HorRes1 * (UINTN)VerRes1 > (UINTN)HorRes2 * (UINTN)VerRes2) {
                        UINT32 temp = ModeList[j];
                        ModeList[j] = ModeList[j + 1];
                        ModeList[j + 1] = temp;
                    }
                }
            }
            // Run test over all modes
            for (UINTN i = 0; i < NumModes; i++) {
                Status = RunGraphicTest(ModeList[i], GraphicTest, TimeParam, NumParam, ClipEnable, Pause, &TestResults[i]);
                if (EFI_ERROR(Status)) {
                    goto App_exit;
                }
            }
        } else {
            // Current graphic mode
            Status = RunGraphicTest(Mode, GraphicTest, TimeParam, NumParam, ClipEnable, Pause, &TestResults[0]);
            if (EFI_ERROR(Status)) {
                goto App_exit;
            }
        }
        gST->RuntimeServices->GetTime(&EndTime, (EFI_TIME_CAPABILITIES*)NULL);

        // Results to console
        OutputTestResults(&StartTime, &EndTime, ClipEnable, TestResults, AllModes ? NumModes : 1, NULL);
        // Results to file if specified
        if (Filename[0]) {
            Status = OutputTestResults(&StartTime, &EndTime, ClipEnable, TestResults, AllModes ? NumModes : 1, Filename);
            if (EFI_ERROR(Status)) {
                goto App_exit;
            }
        }
    }

App_exit:
    SHELL_FREE_NON_NULL(TestResults);
    SHELL_FREE_NON_NULL(ModeList);

    if (EFI_ERROR(Status)) {
        Print(L"ERROR: Application returned - %r\n", Status);
    }
    return ShellStatus;
}

/*
 * DisplayGopInfo() - Display supported graphics modes
 */
STATIC EFI_STATUS DisplayGopInfo(VOID)
{
    EFI_STATUS Status;

    // List all available graphics modes
    Print(L"Available graphics modes:\n");
    for (UINTN i=0; i<NumGraphicsModes(); i++) {
        UINT32 HorRes, VerRes;
        if (!EFI_ERROR(QueryGraphicsMode(i, &HorRes, &VerRes))) {
            Print(L"%2u: %ux%u\n", i , HorRes, VerRes);
        }
    }
    // Current graphics mode
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


/*
 * CheckFile() - Check we can create file before running tests
 */
STATIC EFI_STATUS CheckFile(CHAR16 *Filename)
{
    EFI_STATUS Status = EFI_SUCCESS;

    if (Filename == NULL || !Filename[0]) {
        // no file specified so just exit
        goto Error_exit;
    }
    // remove existing file if it exists
    Status = ShellFileExists(Filename);
    if (Status == EFI_SUCCESS) {
        Status = ShellDeleteFileByName(Filename);
        if (EFI_ERROR(Status)) {
            Print(L"ERROR: Failed to delete existing file '%s' (%r)\n", Filename, Status);
            goto Error_exit;
        }
    } else if (Status != EFI_NOT_FOUND) {
        Print(L"ERROR: Failed checking file '%s' (%r)\n", Filename, Status);
        goto Error_exit;
    }
    // check we can create file
    SHELL_FILE_HANDLE FileHandle;
    Status = ShellOpenFileByName(Filename, &FileHandle, EFI_FILE_MODE_CREATE | EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);
    if (EFI_ERROR(Status)) {
        Print(L"ERROR: Failed to create file '%s' (%r)\n", Filename, Status);
        goto Error_exit;
    }
    // close file
    ShellCloseFile(&FileHandle); 

Error_exit:
    return Status;
}

/*
 * OutputTestResults() - Output results to file or console
 */
STATIC EFI_STATUS OutputTestResults(IN EFI_TIME *StartTime, IN EFI_TIME *EndTime, IN BOOLEAN ClipEnabled, IN TEST_RESULTS *Results, IN UINTN NumResults, IN CHAR16 *Filename)
{
    EFI_STATUS Status = EFI_SUCCESS;
    SHELL_FILE_HANDLE FileHandle = NULL;

    if (Filename && Filename[0]) {
        Status = ShellOpenFileByName(Filename, &FileHandle, EFI_FILE_MODE_CREATE | EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);
        if (EFI_ERROR(Status)) {
            Print(L"ERROR: Failed to create file '%s' (%r)\n", Filename, Status);
            goto Error_exit;
        }
    }

    Status = OutputString(FileHandle, L"Start: %04u/%02u/%02u %02u:%02u:%02u\n", StartTime->Year, StartTime->Month, StartTime->Day, StartTime->Hour, StartTime->Minute, StartTime->Second);
    if (EFI_ERROR(Status)) goto Error_exit;
    Status = OutputString(FileHandle, L"End  : %04u/%02u/%02u %02u:%02u:%02u\n", EndTime->Year, EndTime->Month, EndTime->Day, EndTime->Hour, EndTime->Minute, EndTime->Second);
    if (EFI_ERROR(Status)) goto Error_exit;
    Status = OutputString(FileHandle, L"Clipped: %s\n\n", ClipEnabled ? L"Yes":L"No");
    if (EFI_ERROR(Status)) goto Error_exit;

    for (UINT32 m = 0; m < NumResults; m++) {
        Status = OutputString(FileHandle, L"%ux%u - Mode %u\n", Results[m].HorRes, Results[m].VerRes, Results[m].Mode);
        if (EFI_ERROR(Status)) goto Error_exit;
        Status = OutputString(FileHandle, L"Test           Iterations  Time\n");
        if (EFI_ERROR(Status)) goto Error_exit;
        for (UINTN i=0; i<NUM_TESTS; i++) {
            if (Results[m].Data[i].Run) {
                Status = OutputString(FileHandle, L"%-13s : %9u %5u\n", GetTestDesc(i), Results[m].Data[i].Count, Results[m].Data[i].Time);
                if (EFI_ERROR(Status)) goto Error_exit;
            }
        }
        Status = OutputString(FileHandle, L"\n");
        if (EFI_ERROR(Status)) goto Error_exit;
    }

Error_exit:
    if (FileHandle) {
        ShellCloseFile(&FileHandle); 
    }
    return Status;
}

/*
 * OutputString() - Ouput string to file or console
 */
#define BUFFLEN 1024

STATIC EFI_STATUS EFIAPI OutputString(IN SHELL_FILE_HANDLE FileHandle, IN CONST CHAR16 *FormatString, ...)
{
    EFI_STATUS Status = EFI_SUCCESS;
    VA_LIST  Marker;
    CHAR8 buffer[BUFFLEN];
 
    VA_START (Marker, FormatString);
    UINTN buffSize = AsciiVSPrintUnicodeFormat (buffer, BUFFLEN, FormatString, Marker);
    VA_END (Marker);
    if (FileHandle) {
        Status = ShellWriteFile (FileHandle, &buffSize, (VOID *)buffer);
        if (EFI_ERROR(Status)) {
            Print(L"ERROR: Failed to write to file '%s' (%r)\n", Filename, Status);
        }
    } else {
        AsciiPrint(buffer);
    }
    return Status;
}

/*
 * DevCode() - Code in development
 */

STATIC VOID DevCode()
{
    Print(L"Development\n");

    InitGraphics();
    DrawFillTriangle(0, GetVerRes()/2, GetHorRes()/2, 0, GetHorRes()-1, GetVerRes()-1, RED); 
    SetClipping(200, 200, GetHorRes()-200, GetVerRes()-200);
    DrawFillTriangle(0, GetVerRes()/2, GetHorRes()/2, 0, GetHorRes()-1, GetVerRes()-1, GREEN); 
}
