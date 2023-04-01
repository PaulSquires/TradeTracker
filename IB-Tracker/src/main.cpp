#include "pch.h"

#include "Utilities/AfxWin.h"
#include "Themes/Themes.h"
#include "Database/database.h"

#include "MainWindow/MainWindow.h"


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
    // window is created. LoadDatabase may override this setting if a Theme
    // setting is found in the database.
    SetTheme(Themes::Dark);


    // Set the Trader's name that will display in the Navigation panel.
    // LoadDatabase may override this setting if a setting is found 
    // in the database. Default to the name used to log into this computer.
    SetTraderName(AfxGetUserName());


    // Load all transactions and configuration information. 
    // If a Theme setting was found in the database then the Theme
    // will override the default one that we just specified prior
    // to calling this LoadDatabase function.
    LoadDatabase();


    // Size the main window to encompass 65% of screen width
    // and 85% of screen height.
    int InitalMainWidth = AfxUnScaleX(AfxGetWorkAreaWidth() * 0.65f);
    int InitalMainHeight = AfxUnScaleY(AfxGetWorkAreaHeight() * 0.85f);


    MainWindow Main;

    HWND hWndMain = Main.Create(
                        HWND_DESKTOP,
                        L"IB-Tracker",
                        CW_USEDEFAULT, CW_USEDEFAULT,
                        InitalMainWidth,
                        InitalMainHeight);


    // Set the top level main window that the ApplyActiveTheme function will use
    // to enumerate all children windows to apply any newly changed theme.
    SetThemeMainWindow(hWndMain);


    // Set the large and small application icons that will represent the application
    // in various area of Windows such as the title bar, task bar, and shortcuts.
    HANDLE hIconBig = LoadImage(Main.hInst(), MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON, 32, 32, LR_SHARED);
    SendMessage(hWndMain, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)hIconBig);

    HANDLE hIconSmall = LoadImage(Main.hInst(), MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON, 16, 16, LR_SHARED);
    SendMessage(hWndMain, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIconSmall);


    // Center the main window within the desktop taking into account the actual work area.
    AfxCenterWindow(hWndMain, HWND_DESKTOP);


    // Call the main modal message pump and wait for it to end.
    if (hWndMain == NULL) return 0;

    // Show the window and update its client area
    ShowWindow(hWndMain, (nCmdShow == 0) ? SW_SHOW : nCmdShow);
    UpdateWindow(hWndMain);


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

    return 0;
}

