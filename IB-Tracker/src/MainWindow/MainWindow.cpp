//
// Defines the entry point for the IB-TRACKER application.
//

#include "pch.h"

#include "MainWindow.h"
//#include "tws-client.h"
#include "..\MenuPanel\MenuPanel.h"
#include "..\HistoryPanel\HistoryPanel.h"
#include "..\TradesPanel\TradesPanel.h"
#include "..\Themes\Themes.h"
#include "..\Database\database.h"


CMenuPanel      MenuPanel;
CHistoryPanel   HistoryPanel;
CTradesPanel    TradesPanel;



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

    Graphics graphics(hdc);

    DWORD nBackColor = GetThemeColor(ThemeElement::MenuPanelBack);

    // Create the background brush
    SolidBrush backBrush(nBackColor);

    // Paint the background (only the bottom "statusbar" area) using brush.
    int margin = AfxScaleY(LISTBOX_ROWHEIGHT);
    int nTop = ps.rcPaint.bottom - margin;
    int nWidth = (ps.rcPaint.right - ps.rcPaint.left);
    int nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&backBrush, ps.rcPaint.left, nTop, nWidth, nHeight);

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Process WM_SIZE message for window/dialog: MainWindow
// ========================================================================================
void MainWindow_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    // Position all of the child windows
    if (state == SIZE_MINIMIZED) return;

    int margin = AfxScaleY(LISTBOX_ROWHEIGHT);

    // Position the left hand side Navigation Panel
    HWND hWndMenuPanel = MenuPanel.WindowHandle();
    int nMenuPanelWidth = AfxGetWindowWidth(hWndMenuPanel);
    SetWindowPos(hWndMenuPanel, 0,
        0, 0, nMenuPanelWidth, cy,
        SWP_NOZORDER | SWP_SHOWWINDOW);


    // Position the right hand side History Panel
    HWND hWndHistoryPanel = HistoryPanel.WindowHandle();
    int nHistoryPanelWidth = AfxGetWindowWidth(hWndHistoryPanel);
    SetWindowPos(hWndHistoryPanel, 0,
        cx - nHistoryPanelWidth, 0, nHistoryPanelWidth, cy - margin,
        SWP_NOZORDER | SWP_SHOWWINDOW);


    // Position the middle Trades Panel
    HWND hWndTradesPanel = TradesPanel.WindowHandle();
    int nTradesPanelWidth = (cx - nHistoryPanelWidth - nMenuPanelWidth);
    SetWindowPos(hWndTradesPanel, 0,
        nMenuPanelWidth, 0, nTradesPanelWidth, cy - margin,
        SWP_NOZORDER | SWP_SHOWWINDOW);

}




// ========================================================================================
// Process WM_CREATE message for window/dialog: MainWindow
// ========================================================================================
BOOL MainWindow_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    MenuPanel.Create( hwnd, L"", 0, 0, MENUPANEL_WIDTH, 0,
    WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
    WS_EX_CONTROLPARENT | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);

    HistoryPanel.Create(hwnd, L"", 0, 0, HISTORYPANEL_WIDTH, 0,
        WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_CONTROLPARENT | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);

    TradesPanel.Create(hwnd, L"", 0, 0, 0, 0,
        WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_CONTROLPARENT | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);
    
    return TRUE;
}



// ========================================================================================
// Windows callback function.
// ========================================================================================
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



