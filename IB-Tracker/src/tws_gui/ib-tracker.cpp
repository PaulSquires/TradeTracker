//
// Defines the entry point for the IB-TRACKER application.
//

#include "pch.h"

#include "ib-tracker.h"
#include "NavPanel.h"
#include "TradesPanel.h"
#include "HistoryPanel.h"
#include "tws-client.h"
#include "CWindow.h"
#include "Themes.h"
#include "database.h"



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




//' ========================================================================================
//' Position all child windows. Called manually and/or by WM_SIZE
//' ========================================================================================
LRESULT Main_PositionWindows(HWND hWnd)
{
    RECT rcClient;
    GetClientRect(hWnd, &rcClient);

    // Position the left hand side Navigation Panel
    HWND hWndNavPanel = GetDlgItem(hWnd, IDC_NAVPANEL);
    int nNavPanelWidth = AfxGetWindowWidth(hWndNavPanel);
    SetWindowPos(hWndNavPanel, 0,
        0, 0, nNavPanelWidth, rcClient.bottom,
        SWP_NOZORDER | SWP_SHOWWINDOW);


    // Position the right hand side History Panel
    HWND hWndHistoryPanel = GetDlgItem(hWnd, IDC_HISTORYPANEL);
    int nHistoryPanelWidth = AfxGetWindowWidth(hWndHistoryPanel);
    SetWindowPos(hWndHistoryPanel, 0,
        rcClient.right - nHistoryPanelWidth, 0, nHistoryPanelWidth, rcClient.bottom,
        SWP_NOZORDER | SWP_SHOWWINDOW);


    // Position the middle Trades Panel
    HWND hWndTradesPanel = GetDlgItem(hWnd, IDC_TRADESPANEL);
    int nTradesPanelWidth = (rcClient.right - nHistoryPanelWidth - nNavPanelWidth);
    SetWindowPos(hWndTradesPanel, 0,
        nNavPanelWidth, 0, nTradesPanelWidth, rcClient.bottom,
        SWP_NOZORDER | SWP_SHOWWINDOW);

    return 0;
}


//' ========================================================================================
//' Windows callback function.
//' ========================================================================================
LRESULT CALLBACK Main_WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {

    case WM_SIZE:
        // Position all of the child windows
        if (wParam != SIZE_MINIMIZED)
            Main_PositionWindows(hWnd);
    break;


    case WM_ERASEBKGND:
    {
        // Handle all of the painting in WM_PAINT
        // We do not need to paint the background because the full clint area will always
        // be covered by child windows (NavPanel, Trades Panel, History Panel)
        return TRUE;
        break;
    }


    case WM_PAINT:
    {
        PAINTSTRUCT ps;

        HDC hdc = BeginPaint(hWnd, &ps);

        EndPaint(hWnd, &ps);
        break;
    }


    case WM_DESTROY:
        // Disconnect from IBKR TWS and shut down monitoring thread.
        tws_disconnect();
        PostQuitMessage(0);
        break;


    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}


//' ========================================================================================
//' WinMain main application startup entry point.
//' ========================================================================================
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
    SetTheme(Themes::Light);


    // Set the Trader's name that will display in the Navigation panel.
    // LoadDatabase may override this setting if a setting is found 
    // in the database. Default to the name used to log into this computer.
    SetTraderName(AfxGetUserName());


    // Load all transactions and configuration information. 
    // If a Theme setting was found in the database then the Theme
    // will override the default one that we just specified prior
    // to calling this LoadDatabase function.
    LoadDatabase();


    // Create the main application window
    CWindow* pWindowMain = new CWindow();
    if (pWindowMain == nullptr) return FALSE;


    // Size the main window to encompass 65% of screen width
    // and 85% of screen height.
    float InitalMainWidth = AfxUnScaleX(AfxGetWorkAreaWidth() * 0.65f);
    float InitalMainHeight = AfxUnScaleY(AfxGetWorkAreaHeight() * 0.85f);


    HWND hWndMain = pWindowMain->Create(
        HWND_DESKTOP, 
        L"IB-Tracker", 
        Main_WindowProc, 
        CW_USEDEFAULT, CW_USEDEFAULT, 
        (INT)InitalMainWidth,
        (INT)InitalMainHeight
        );

    pWindowMain->SetBrush(GetStockBrush(NULL_BRUSH));


    // Set the top level main window that the ApplyActiveTheme function will use
    // to enumerate all children windows to apply any newly changed theme.
    SetThemeMainWindow(hWndMain);


    // Set the large and small application icons that will represent the application
    // in various area of Windows such as the title bar, task bar, and shortcuts.
    HANDLE hIconBig = LoadImage(pWindowMain->hInst(), MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON, 32, 32, LR_SHARED);
    SendMessage(hWndMain, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)hIconBig);

    HANDLE hIconSmall = LoadImage(pWindowMain->hInst(), MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON, 16, 16, LR_SHARED);
    SendMessage(hWndMain, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIconSmall);

    
    // Load the child windows
    CWindow* pWindowNavPanel = NavPanel_Show(hWndMain);
    CWindow* pWindowTradesPanel = TradesPanel_Show(hWndMain);
    CWindow* pWindowHistoryPanel = HistoryPanel_Show(hWndMain);


    // Center the main window within the desktop taking into account the actual work area.
    AfxCenterWindow(hWndMain, HWND_DESKTOP);

    
    // Call the main modal message pump and wait for it to end.
    pWindowMain->DoEvents(nCmdShow);


    // Shut down the GDI+ subsystem
    GdiplusShutdown(gdiplusToken);


    // delete our manually created memory and pointers for the various child panels.
    if (pWindowNavPanel) delete(pWindowNavPanel);
    if (pWindowTradesPanel) delete(pWindowTradesPanel);
    if (pWindowHistoryPanel) delete(pWindowHistoryPanel);
    if (pWindowMain) delete(pWindowMain);

    return 0;
}

