//
// Defines the entry point for the IB-TRACKER application.
//

#include "pch.h"

#include "MainWindow.h"
//#include "tws-client.h"
//#include "..\NavPanel\NavPanel.h"
//#include "..\TradesPanel\TradesPanel.h"
//#include "..\HistoryPanel\HistoryPanel.h"
#include "..\Themes\Themes.h"
#include "..\Database\database.h"



// ========================================================================================
// Process WM_DESTROY message for window/dialog: MainWindow
// ========================================================================================
void MainWindow_OnDestroy(HWND hwnd)
{
    // Disconnect from IBKR TWS and shut down monitoring thread.
   // tws_disconnect();
    PostQuitMessage(0);
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: MainWindow
// ========================================================================================
BOOL MainWindow_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    // Handle all of the painting in WM_PAINT
    // We do not need to paint the background because the full client area will always
    // be covered by child windows (NavPanel, Trades Panel, History Panel)
    return TRUE;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: MainWindow
// ========================================================================================
void MainWindow_OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Process WM_SIZE message for window/dialog: MainWindow
// ========================================================================================
void MainWindow_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    // Position all of the child windows
    if (state == SIZE_MINIMIZED) return;

/*
    // Position the left hand side Navigation Panel
    HWND hWndNavPanel = GetDlgItem(hwnd, IDC_NAVPANEL);
    int nNavPanelWidth = AfxGetWindowWidth(hWndNavPanel);
    SetWindowPos(hWndNavPanel, 0,
        0, 0, nNavPanelWidth, cy,
        SWP_NOZORDER | SWP_SHOWWINDOW);


    // Position the right hand side History Panel
    HWND hWndHistoryPanel = GetDlgItem(hwnd, IDC_HISTORYPANEL);
    int nHistoryPanelWidth = AfxGetWindowWidth(hWndHistoryPanel);
    SetWindowPos(hWndHistoryPanel, 0,
        cx - nHistoryPanelWidth, 0, nHistoryPanelWidth, cy,
        SWP_NOZORDER | SWP_SHOWWINDOW);


    // Position the middle Trades Panel
    HWND hWndTradesPanel = GetDlgItem(hwnd, IDC_TRADESPANEL);
    int nTradesPanelWidth = (cx - nHistoryPanelWidth - nNavPanelWidth);
    SetWindowPos(hWndTradesPanel, 0,
        nNavPanelWidth, 0, nTradesPanelWidth, cy,
        SWP_NOZORDER | SWP_SHOWWINDOW);

        */
}




// ========================================================================================
// Process WM_CREATE message for window/dialog: MainWindow
// ========================================================================================
BOOL MainWindow_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    //HWND_NAVPANEL =
    //    m_pWindow->Create(hWndParent, L"", WndProc, 0, 0, NAVPANEL_WIDTH, 0,
    //        WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
    //        WS_EX_CONTROLPARENT | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);
    std::cout << "MainWindow WM_CREATE" << std::endl;
    return TRUE;
}



// ========================================================================================
// Windows callback function.
// ========================================================================================
//
// MainWindow  Window Procedure
//
LRESULT MainWindow::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(m_hwnd, WM_CREATE, MainWindow_OnCreate);
        HANDLE_MSG(m_hwnd, WM_DESTROY, MainWindow_OnDestroy);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, MainWindow_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, MainWindow_OnPaint);
        HANDLE_MSG(m_hwnd, WM_SIZE, MainWindow_OnSize);

    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}



