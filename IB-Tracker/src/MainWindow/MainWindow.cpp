//
// Defines the entry point for the IB-TRACKER application.
//

#include "pch.h"

#include "MainWindow.h"
#include "tws-client.h"
#include "..\MenuPanel\MenuPanel.h"
#include "..\HistoryPanel\HistoryPanel.h"
#include "..\TradesPanel\TradesPanel.h"
#include "..\Utilities\UserMessages.h"
#include "..\Config\Config.h"


HWND HWND_MAINWINDOW = NULL;

CMainWindowShadow Shadow;
CMenuPanel        MenuPanel;
CHistoryPanel     HistoryPanel;
CTradesPanel      TradesPanel;


extern void TradesPanel_ShowActiveTrades();

RECT rcSplitter{};
bool isDragging = false;    // If dragging our splitter
POINT prev_pt{};            // for tracking current splitter drag



// ========================================================================================
// Blur the MainWindow panels. This function is called prior and after
// a popup dialog is shown in order to reduce the underlying "visual noise" for that 
// popup window. For example, the Trade Management popup is easier to use and enter data
// into if the user is not distracted by the MainWindow menu panel, trades and history
// data displaying behind the popup.
// ========================================================================================
void MainWindow_BlurPanels(bool active)
{
    if (active) {
        RECT rc;       GetWindowRect(HWND_MAINWINDOW, &rc);
        RECT rcClient; GetClientRect(HWND_MAINWINDOW, &rcClient);
        
        int border_thickness = ((rc.right - rc.left) - rcClient.right) / 2;
        InflateRect(&rc, -border_thickness, -border_thickness);

        HWND hWndShadow = 
            Shadow.Create(HWND_MAINWINDOW, L"", 0, 0, 0, 0, 
                WS_POPUP | WS_DISABLED | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_LAYERED | WS_EX_NOACTIVATE);
        
        HBRUSH hbrBackground = GetSysColorBrush(COLOR_WINDOWTEXT);
        SetClassLongPtr(hWndShadow, GCLP_HBRBACKGROUND, (LONG_PTR)hbrBackground);

        // Make this window 50% alpha
        SetLayeredWindowAttributes(hWndShadow, 0, (255 * 50) / 100, LWA_ALPHA);

        SetWindowPos(hWndShadow, 0, 
            rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 
            SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
        
    }
    else {
        DestroyWindow(Shadow.WindowHandle());
    }

}


// ========================================================================================
// Automatically show any existing trades on program startup & connect to TWS.
// ========================================================================================
void MainWindow_StartupShowTrades()
{
    if (trades.size() != 0) {
        TradesPanel_ShowActiveTrades();

        if (GetStartupConnect()) {
            bool res = tws_connect();
            if (tws_isConnected()) {
                // Need to re-populate the Trades if successfully connected in order
                // to send the request market data for each ticker.
                TradesPanel_ShowActiveTrades();
            }
        }

    }
}


// ========================================================================================
// Process WM_DESTROY message for window/dialog: MainWindow
// ========================================================================================
void MainWindow_OnDestroy(HWND hwnd)
{
    // Disconnect from IBKR TWS and shut down monitoring thread.
    tws_disconnect();
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

    int nWidth = (ps.rcPaint.right - ps.rcPaint.left);
    int nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&backBrush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Process WM_SIZE message for window/dialog: MainWindow
// ========================================================================================
void MainWindow_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    // Position all of the child windows
    if (state == SIZE_MINIMIZED) return;
    

    int MARGIN = AfxScaleY(ACTIVE_TRADES_LISTBOX_ROWHEIGHT);
    int INNER_MARGIN = AfxScaleY(6);
    int SPLITTER_WIDTH = AfxScaleX(6);

    HDWP hdwp = BeginDeferWindowPos(3);

    // Position the left hand side Navigation Panel
    HWND hWndMenuPanel = MenuPanel.WindowHandle();
    int nMenuPanelWidth = AfxGetWindowWidth(hWndMenuPanel);
    hdwp = DeferWindowPos(hdwp, hWndMenuPanel, 0,
                0, 0, nMenuPanelWidth, cy,
                SWP_NOZORDER | SWP_SHOWWINDOW);


    // Position the right hand side History Panel
    HWND hWndHistoryPanel = HistoryPanel.WindowHandle();
    int nHistoryPanelWidth = AfxGetWindowWidth(hWndHistoryPanel);
    hdwp = DeferWindowPos(hdwp, hWndHistoryPanel, 0,
                cx - nHistoryPanelWidth - INNER_MARGIN, 0, nHistoryPanelWidth, cy - MARGIN,
                SWP_NOZORDER | SWP_SHOWWINDOW);

        
    // Position the middle Trades Panel
    HWND hWndTradesPanel = TradesPanel.WindowHandle();
    int nTradesPanelWidth = (cx - nHistoryPanelWidth - nMenuPanelWidth - SPLITTER_WIDTH - INNER_MARGIN);
    hdwp = DeferWindowPos(hdwp, hWndTradesPanel, 0,
                nMenuPanelWidth, 0, nTradesPanelWidth, cy - MARGIN,
                SWP_NOZORDER | SWP_SHOWWINDOW);

    EndDeferWindowPos(hdwp);

    // Calculate the area for the "splitter control"
    rcSplitter.left = nMenuPanelWidth + nTradesPanelWidth;
    rcSplitter.top = 0;
    rcSplitter.right = rcSplitter.left + SPLITTER_WIDTH;
    rcSplitter.bottom = cy;

}


// ========================================================================================
// Process WM_CREATE message for window/dialog: MainWindow
// ========================================================================================
BOOL MainWindow_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    HWND_MAINWINDOW = hwnd;

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
// Determine if mouse cursor is over our splitter control area.
// ========================================================================================
bool IsMouseOverSplitter(HWND hwnd)
{
    POINT pt; GetCursorPos(&pt);
    MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&pt, 1);

    if (PtInRect(&rcSplitter, pt)) {
        SetCursor(LoadCursor(NULL, (LPCWSTR)IDC_SIZEWE));
        return TRUE;
    }
    return FALSE;
}


// ========================================================================================
// Process WM_LBUTTONDOWN message for window/dialog: MainWindow
// ========================================================================================
void MainWindow_OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
    if (IsMouseOverSplitter(hwnd)) {
        isDragging = true;
        prev_pt.x = x;
        SetCapture(hwnd);
    }
}


void UpdateSplitterChildren(HWND hwnd, int xDelta)
{
    HWND hWndHistoryPanel = HistoryPanel.WindowHandle();
    int nHistoryPanelWidth = AfxGetWindowWidth(hWndHistoryPanel) + xDelta;
    int nHistoryPanelHeight = AfxGetWindowHeight(hWndHistoryPanel);
    SetWindowPos(hWndHistoryPanel, 0,
        0, 0, nHistoryPanelWidth, nHistoryPanelHeight, SWP_NOZORDER | SWP_NOMOVE);

    RECT rc; GetClientRect(hwnd, &rc);
    MainWindow_OnSize(hwnd, SIZE_RESTORED, rc.right, rc.bottom);
    AfxRedrawWindow(TradesPanel.WindowHandle());
    AfxRedrawWindow(HistoryPanel.WindowHandle());
    AfxRedrawWindow(hwnd);
}


// ========================================================================================
// Process WM_MOUSEMOVE message for window/dialog: MainWindow
// ========================================================================================
void MainWindow_OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
    if (isDragging) {
        // Calculate the change between mouse movements and resize
        // the child windows accordingly.
        int xDelta = prev_pt.x - x;

        // Only reposition after a cummulative greater than 3 pixel move
        // in order to reduce the amount of screen updating.
        if (abs(xDelta) > 3) {
            UpdateSplitterChildren(hwnd, xDelta);
            prev_pt.x = x;
        }

    }
}


// ========================================================================================
// Process WM_LBUTTONUP message for window/dialog: MainWindow
// ========================================================================================
void MainWindow_OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags)
{
    if (isDragging) {
        isDragging = false;
        int xDelta = prev_pt.x - x;
        UpdateSplitterChildren(hwnd, xDelta);
        ReleaseCapture();
    }
}


// ========================================================================================
// Process WM_SETCURSOR message for window/dialog: MainWindow
// ========================================================================================
BOOL MainWindow_OnSetCursor(HWND hwnd, HWND hwndCursor, UINT codeHitTest, UINT msg)
{
    LPWSTR curId = IDC_ARROW;

    // Determine if the mouse is over our "splitter control"
    if (IsMouseOverSplitter(hwnd)) {
        SetCursor(LoadCursor(NULL, (LPCWSTR)IDC_SIZEWE));
        return TRUE;
    }

    switch (codeHitTest)
    {
    case HTTOPRIGHT:
    case HTBOTTOMLEFT:
        curId = IDC_SIZENESW;
        break;

    case HTBOTTOMRIGHT:
    case HTTOPLEFT:
        curId = IDC_SIZENWSE;
        break;

    case HTLEFT:
    case HTRIGHT:
        curId = IDC_SIZEWE;
        break;

    case HTTOP:
    case HTBOTTOM:
        curId = IDC_SIZENS;
        break;

    default:
        curId = IDC_ARROW;
    }

    SetCursor(LoadCursor(NULL, (LPCWSTR)curId));
    return TRUE;
}


// ========================================================================================
// Windows callback function (MainWindow).
// ========================================================================================
LRESULT CMainWindow::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(m_hwnd, WM_CREATE, MainWindow_OnCreate);
        HANDLE_MSG(m_hwnd, WM_DESTROY, MainWindow_OnDestroy);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, MainWindow_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, MainWindow_OnPaint);
        HANDLE_MSG(m_hwnd, WM_SIZE, MainWindow_OnSize);
        HANDLE_MSG(m_hwnd, WM_MOUSEMOVE, MainWindow_OnMouseMove);
        HANDLE_MSG(m_hwnd, WM_LBUTTONDOWN, MainWindow_OnLButtonDown);
        HANDLE_MSG(m_hwnd, WM_LBUTTONUP, MainWindow_OnLButtonUp);
        HANDLE_MSG(m_hwnd, WM_SETCURSOR, MainWindow_OnSetCursor);

    case MSG_STARTUP_SHOWTRADES:
        MainWindow_StartupShowTrades();
        return 0;

    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}



// ========================================================================================
// Windows callback function (MainWindowShadow).
// ========================================================================================
LRESULT CMainWindowShadow::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(m_hwnd, msg, wParam, lParam);
}

