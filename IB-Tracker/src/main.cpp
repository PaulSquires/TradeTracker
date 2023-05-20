#include "pch.h"

#include "Utilities/AfxWin.h"
#include "Themes/Themes.h"
#include "Database/database.h"
#include "Config/Config.h"
#include "MainWindow/MainWindow.h"
#include "Utilities/UserMessages.h"


CMainWindow Main;


// Use the following pragma instead of havign to include a manifest with the application
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")



#ifndef ENABLECONSOLE
// Set to zero to disable the console window when the application runs.
// We would disable the console when distributing/deploying the final app.
#define ENABLECONSOLE 1
#endif


#if (ENABLECONSOLE >= 1)
void BindStdHandlesToConsole()
{
    // Redirect the CRT standard input, output, and error handles to the console
    std::ignore = freopen("CONIN$", "r", stdin);
    std::ignore = freopen("CONOUT$", "w", stderr);
    std::ignore = freopen("CONOUT$", "w", stdout);

    // Note that there is no CONERR$ file
    HANDLE hStdout = CreateFile(L"CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    HANDLE hStdin = CreateFile(L"CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    SetStdHandle(STD_OUTPUT_HANDLE, hStdout);
    SetStdHandle(STD_ERROR_HANDLE, hStdout);
    SetStdHandle(STD_INPUT_HANDLE, hStdin);

    //Clear the error state for each of the C++ standard stream objects. 
    std::wclog.clear();
    std::clog.clear();
    std::wcout.clear();
    std::cout.clear();
    std::wcerr.clear();
    std::cerr.clear();
    std::wcin.clear();
    std::cin.clear();
}
#endif



// ========================================================================================
// WinMain main application startup entry point.
// ========================================================================================
int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);


    // Ensure only one running instance
    HANDLE hMutexHandle = CreateMutex(NULL, TRUE, L"ibtracker.mutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        // Try to bring the existing instance to the foreground
        HWND existingApp = FindWindow(0, L"IB-Tracker");
        if (existingApp) {
            if (IsMinimized(existingApp)) ShowWindow(existingApp, SW_SHOWNORMAL);
            SetForegroundWindow(existingApp);
        }
        return 0;
    }


    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR           gdiplusToken;


    // Initialize GDI+.
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);



#if (ENABLECONSOLE >= 1)
    // Create console terminal for GUI application in order to print out debug messages
    AllocConsole();

    // Redirect stderr/stdout/stdin to new console
    BindStdHandlesToConsole();
#endif


    // Set the Theme to use for all windows and controls. Remember to call
    // the SetThemeMainWindow() function after the application's main 
    // window is created. LoadConfig may override this setting if a Theme
    // setting is found. 
    InitializeDarkThemeColors();


    // Set the Trader's name that will display in the Menu panel.
    // LoadConfig may override this setting if a setting is found.
    // Default to the name used to log into this computer.
    SetTraderName(AfxGetUserName());


    // Load the Config file. Settings found in the Config file will override
    // previously set values from InitializeDarkThemeColors, SetTraderName.
    LoadConfig();


    // Load all transactions and configuration information. 
    LoadDatabase();


    // Size the main window to encompass 75% of screen width
    // and 85% of screen height.
    int InitalMainWidth = AfxUnScaleX(AfxGetWorkAreaWidth() * 0.75f);
    int InitalMainHeight = AfxUnScaleY(AfxGetWorkAreaHeight() * 0.85f);

    // Impose a maximum size on the initial height/width in order
    // to ensure that the main window does not display exceptionally
    // large especially when run on very large monitors.
    // Target a minimum 720p screen resolution size (1280 x 720).
    if (InitalMainWidth > 1280) InitalMainWidth = 1280;
    if (InitalMainHeight > 720) InitalMainHeight = 720;


    HWND hWndMain = Main.Create(
                        HWND_DESKTOP,
                        L"IB-Tracker",
                        CW_USEDEFAULT, CW_USEDEFAULT,
                        InitalMainWidth,
                        InitalMainHeight);


    // Set the top level main window that the ApplyActiveTheme function will use
    // to enumerate all children windows to apply any newly changed theme.
    SetThemeMainWindow(hWndMain);


    // If we are using a dark theme then attempt to apply the standard Windows dark theme
    // to the non-client areas of the main form.
    BOOL value = GetIsThemeDark();
    ::DwmSetWindowAttribute(hWndMain, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));


    // Set the large and small application icons that will represent the application
    // in various area of Windows such as the title bar, task bar, and shortcuts.
    HANDLE hIconBig = LoadImage(Main.hInst(), MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON, 32, 32, LR_SHARED);
    SendMessage(hWndMain, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)hIconBig);

    HANDLE hIconSmall = LoadImage(Main.hInst(), MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON, 16, 16, LR_SHARED);
    SendMessage(hWndMain, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIconSmall);


    // Center the main window within the desktop taking into account the actual work area.
    AfxCenterWindow(hWndMain, HWND_DESKTOP);


    if (hWndMain == NULL) return 0;


    // Workaround for the Windows white flashing bug.
    // https://stackoverflow.com/questions/69715610/how-to-initialize-the-background-color-of-win32-app-to-something-other-than-whit
    BOOL cloak = TRUE;
    DwmSetWindowAttribute(hWndMain, DWMWA_CLOAK, &cloak, sizeof(cloak));

    // Show the window and update its client area
    ShowWindow(hWndMain, (nCmdShow == 0) ? SW_SHOW : nCmdShow);
    UpdateWindow(hWndMain);

    cloak = FALSE;
    DwmSetWindowAttribute(hWndMain, DWMWA_CLOAK, &cloak, sizeof(cloak));

    // Now that the child panels are created we can *post* a message to MainWindow to ask
    // to show any trades that already exist. We need to postmessage because we need for
    // the ListBox to be shown and sized, otherwise the CustomVScrollBar calculation will not
    // correctly determine it's thumb size due to zero height ListBox.
    PostMessage(hWndMain, MSG_STARTUP_SHOWTRADES, 0, 0);


    // Call the main modal message pump and wait for it to end.
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        // Processes accelerator keys for menu commands
        if (Main.hAccel() == NULL || (!TranslateAccelerator(hWndMain, Main.hAccel(), &msg))) {
#if (USEDLGMSG >= 1)
            // Determines whether a message is intended for the specified
            // dialog box and, if it is, processes the message.
            if (!IsDialogMessage(hWndMain, &msg)) {
                // Translates virtual-key messages into character messages.
                TranslateMessage(&msg);
                // Dispatches a message to a window procedure.
                DispatchMessage(&msg);
            }
#else
            // Translates virtual-key messages into character messages.
            TranslateMessage(&msg);
            // Dispatches a message to a window procedure.
            DispatchMessage(&msg);
#endif
        }
    }


    // Shut down the GDI+ subsystem
    GdiplusShutdown(gdiplusToken);


    // Release the mutex that prevents multiple application instances
    ReleaseMutex(hMutexHandle);
    CloseHandle(hMutexHandle);

    return 0;
}

