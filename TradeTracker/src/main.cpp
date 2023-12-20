/*

MIT License

Copyright(c) 2023 Paul Squires

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include "pch.h"

#include "Utilities/AfxWin.h"
#include "Database/database.h"
#include "Config/Config.h"
#include "MainWindow/MainWindow.h"
#include "ActiveTrades/ActiveTrades.h"
#include "Utilities/UpdateCheck.h"
#include <wchar.h>



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

    std::wstring wszCaption = L"TradeTracker: " + version;

    // Ensure only one running instance based on the EXE folder. We use the EXE folder
    // because we want to allow multiple instances of the program IF they are run from
    // differing folders, thereby using different databases.
    // Must remove any invalid backslash characters (\). 
    std::wstring wszMutex = AfxGetExePath();
    wszMutex.erase(remove(wszMutex.begin(), wszMutex.end(), L'\\'), wszMutex.end());

    HANDLE hMutexHandle = CreateMutex(NULL, TRUE, wszMutex.c_str());
    if (hMutexHandle != nullptr && GetLastError() == ERROR_ALREADY_EXISTS)
    {
        // Try to bring the existing instance to the foreground
        HWND existingApp = FindWindow(0, wszCaption.c_str());
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


    // Only create the console if command line argument -console is passed.
    if (wcscmp(lpCmdLine, L"-console") == 0) {
        // Create console terminal for GUI application in order to print out debug messages
        AllocConsole();
        // Redirect stderr/stdout/stdin to new console
        BindStdHandlesToConsole();
    }


    // Load the Configuration file. 
    LoadConfig();


    // Load all transactions. 
    LoadDatabase();


    HWND hWndMain = Main.Create(
                        HWND_DESKTOP,
                        wszCaption,
                        CW_USEDEFAULT, CW_USEDEFAULT,
                        GetStartupWidth(),
                        GetStartupHeight());

    // Initialize COM
    HRESULT hResult = CoInitialize(NULL);


    // Attempt to apply the standard Windows dark theme to the non-client areas of the main form.
    BOOL value = true;
    ::DwmSetWindowAttribute(hWndMain, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));


    // Set the large and small application icons that will represent the application
    // in various area of Windows such as the title bar, task bar, and shortcuts.
    HANDLE hIconBig = LoadImage(Main.hInst(), MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON, 32, 32, LR_SHARED);
    SendMessage(hWndMain, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)hIconBig);

    HANDLE hIconSmall = LoadImage(Main.hInst(), MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON, 16, 16, LR_SHARED);
    SendMessage(hWndMain, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIconSmall);


    if (hWndMain == NULL) return 0;

    // Center the main window within the desktop taking into account the actual work area.
    AfxCenterWindow(hWndMain, HWND_DESKTOP);


    // Show the window and update its client area
    ShowWindow(hWndMain, (nCmdShow == 0) ? SW_SHOW : nCmdShow);
    UpdateWindow(hWndMain);


    // Show the current list of active trades
    ActiveTrades_ShowActiveTrades();


    // Display message if Paper Trading is enabled.
    DisplayPaperTradingWarning();


    // Start thread to check for internet connection and determine if an updated
    // version of the program is available.
    DisplayUpdateAvailableMessage();



    // Call the main modal message pump and wait for it to end.
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {            
        if (msg.message == WM_KEYUP && msg.wParam == VK_ESCAPE) {
            MainWindow_CloseComboPopups();
        }

        // Processes accelerator keys for menu commands
        if (Main.hAccel() == NULL || (!TranslateAccelerator(hWndMain, Main.hAccel(), &msg))) {
            // Translates virtual-key messages into character messages.
            TranslateMessage(&msg);
            // Dispatches a message to a window procedure.
            DispatchMessage(&msg);
        }
    }


    // Shut down the GDI+ subsystem
    GdiplusShutdown(gdiplusToken);


    // Release the mutex that prevents multiple application instances
    if (hMutexHandle != nullptr) {
        ReleaseMutex(hMutexHandle);
        CloseHandle(hMutexHandle);
    }


    // Uninitialize COM
    CoUninitialize();

    return 0;
}

