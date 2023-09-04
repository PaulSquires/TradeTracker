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

//
// Defines the entry point for the IB-TRACKER application.
//

#include "pch.h"

#include "MainWindow.h"
#include "tws-client.h"
#include "SideMenu/SideMenu.h"
#include "TradeHistory/TradeHistory.h"
#include "ActiveTrades/ActiveTrades.h"
#include "ClosedTrades/ClosedTrades.h"
#include "Transactions/TransPanel.h"
#include "Transactions/TransDetail.h"
#include "TickerTotals/TickerTotals.h"
#include "Category/Category.h"
#include "Category/CategoryDialog.h"
#include "CustomLabel/CustomLabel.h"

#include "Utilities/UserMessages.h"
#include "Config/Config.h"


HWND HWND_MAINWINDOW = NULL;
HWND HWND_LEFTPANEL = NULL;
HWND HWND_MIDDLEPANEL = NULL;
HWND HWND_RIGHTPANEL = NULL;

CMainWindowShadow Shadow;
CMainWindow Main;

RECT rcSplitter{};
bool isDragging = false;    // If dragging our splitter
POINT prev_pt{};            // for tracking current splitter drag


// ========================================================================================
// Set the HWND for the panel that will display on the right side of the MainWindow.
// Also place it into position and hide previous HWND of right panel.
// ========================================================================================
void MainWindow_SetRightPanel(HWND hPanel)
{
    // If the incoming panel is already set as the right panel then simply exit.
    if (hPanel == HWND_RIGHTPANEL) return;

    // Get the current position size of the current right panel.
    RECT rc; GetWindowRect(HWND_RIGHTPANEL, &rc);
    MapWindowPoints(HWND_DESKTOP, HWND_MAINWINDOW, (LPPOINT)&rc, 2);

    ShowWindow(HWND_RIGHTPANEL, SW_HIDE);

    HWND_RIGHTPANEL = hPanel;
    SetWindowPos(HWND_RIGHTPANEL, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
        SWP_NOZORDER | SWP_SHOWWINDOW);
}


// ========================================================================================
// Set the HWND for the panel that will display in the middle of the MainWindow.
// Also place it into position and hide previous HWND of middle panel.
// ========================================================================================
void MainWindow_SetMiddlePanel(HWND hPanel)
{
    // If the incoming panel is already set as the middle panel then simply exit.
    if (hPanel == HWND_MIDDLEPANEL) return;

    // Get the current position size of the current right panel.
    RECT rc; GetWindowRect(HWND_MIDDLEPANEL, &rc);
    MapWindowPoints(HWND_DESKTOP, HWND_MAINWINDOW, (LPPOINT)&rc, 2);

    ShowWindow(HWND_MIDDLEPANEL, SW_HIDE);

    HWND_MIDDLEPANEL = hPanel;
    SetWindowPos(HWND_MIDDLEPANEL, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
        SWP_NOZORDER | SWP_SHOWWINDOW);
}


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

        if (Shadow.WindowHandle() == NULL) {
            Shadow.Create(HWND_MAINWINDOW, L"", 0, 0, 0, 0,
                WS_POPUP | WS_DISABLED | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_LAYERED | WS_EX_NOACTIVATE);
        
            HBRUSH hbrBackground = GetSysColorBrush(COLOR_WINDOWTEXT);
            SetClassLongPtr(Shadow.WindowHandle(), GCLP_HBRBACKGROUND, (LONG_PTR)hbrBackground);

            // Make this window 50% alpha
            SetLayeredWindowAttributes(Shadow.WindowHandle(), 0, (255 * 50) / 100, LWA_ALPHA);
        }

        SetWindowPos(Shadow.WindowHandle(), 0,
            rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 
            SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
        
    }
    else {
        ShowWindow(Shadow.WindowHandle(), SW_HIDE);
    }

}


// ========================================================================================
// Automatically show any existing trades on program startup & connect to TWS.
// ========================================================================================
void MainWindow_StartupShowTrades()
{
    if (trades.size() != 0) {
        ActiveTrades_ShowActiveTrades();

        if (GetStartupConnect()) {
            SendMessage(SideMenu.WindowHandle(), MSG_tws_Connect_START, 0, 0);
            bool res = tws_Connect();
            if (tws_IsConnected()) {
                // Need to re-populate the Trades if successfully connected in order
                // to send the request market data for each ticker.
                ListBoxData_DestroyItemData(GetDlgItem(ActiveTrades.WindowHandle(), IDC_TRADES_LISTBOX));
                ActiveTrades_ShowActiveTrades();
            }
        }

    }
}


// ========================================================================================
// Process WM_CLOSE message for window/dialog: MainWindow
// ========================================================================================
void MainWindow_OnClose(HWND hwnd)
{
    // Disconnect from IBKR TWS and shut down monitoring thread.
    tws_Disconnect();

    // Destroy the popup shadow window should it exist.
    if (Shadow.WindowHandle()) DestroyWindow(Shadow.WindowHandle());

    // Save the Config file so that StartupWidth and StartupHeight will persist
    // for the time application is executed.
    SetStartupWidth(AfxGetWindowWidth(hwnd));
    SetStartupHeight(AfxGetWindowHeight(hwnd));
    SetStartupRightPanelWidth(AfxGetWindowWidth(HWND_RIGHTPANEL));

    SaveConfig();

    DestroyWindow(hwnd);
}


// ========================================================================================
// Process WM_DESTROY message for window/dialog: MainWindow
// ========================================================================================
void MainWindow_OnDestroy(HWND hwnd)
{
    // Quit the application
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

    // Create the background brush
    SolidBrush backBrush(COLOR_BLACK);

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
    
    int MARGIN = AfxScaleY(23);
    int INNER_MARGIN = AfxScaleY(6);
    int SPLITTER_WIDTH = AfxScaleX(6);

    HDWP hdwp = BeginDeferWindowPos(6);

    // Position the left hand side Navigation Panel
    int nLeftPanelWidth = AfxGetWindowWidth(HWND_LEFTPANEL);
    hdwp = DeferWindowPos(hdwp, HWND_LEFTPANEL, 0,
                0, 0, nLeftPanelWidth, cy, SWP_NOZORDER | SWP_SHOWWINDOW);


    // Position the right hand side History Panel
    int nRightPanelWidth = AfxGetWindowWidth(HWND_RIGHTPANEL);
    hdwp = DeferWindowPos(hdwp, HWND_RIGHTPANEL, 0,
                cx - nRightPanelWidth - INNER_MARGIN, 0, nRightPanelWidth, cy - MARGIN,
                SWP_NOZORDER | SWP_SHOWWINDOW);

        
    // Position the middle Trades Panel
    int nMiddlePanelWidth = (cx - nRightPanelWidth - nLeftPanelWidth - SPLITTER_WIDTH - INNER_MARGIN);
    hdwp = DeferWindowPos(hdwp, HWND_MIDDLEPANEL, 0,
                nLeftPanelWidth, 0, nMiddlePanelWidth, cy - MARGIN,
                SWP_NOZORDER | SWP_SHOWWINDOW);


    // Position the Warning label
    HWND hCtl = GetDlgItem(hwnd, IDC_MAINWINDOW_WARNING);
    int nTop = AfxScaleY(20);
    int nHeight = AfxScaleY(16);
    int nWidth = nMiddlePanelWidth; 
    DeferWindowPos(hdwp, hCtl, 0, nLeftPanelWidth, cy - nTop, nWidth, nHeight, SWP_NOZORDER | SWP_HIDEWINDOW);


    EndDeferWindowPos(hdwp);

    // Calculate the area for the "splitter control"
    rcSplitter.left = nLeftPanelWidth + nMiddlePanelWidth;
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

    HWND_LEFTPANEL = SideMenu.Create( hwnd, L"", 0, 0, SIDEMENU_WIDTH, 0,
        WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_CONTROLPARENT | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);

    HWND_RIGHTPANEL = TradeHistory.Create(hwnd, L"", 0, 0, GetStartupRightPanelWidth(), 0,
        WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_CONTROLPARENT | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);

    HWND_MIDDLEPANEL = ActiveTrades.Create(hwnd, L"", 0, 0, 0, 0,
        WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_CONTROLPARENT | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);
    
    ClosedTrades.Create(hwnd, L"", 0, 0, 0, 0,
        WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_CONTROLPARENT | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);

    TickerPanel.Create(hwnd, L"", 0, 0, 0, 0,
        WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_CONTROLPARENT | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);

    TransPanel.Create(hwnd, L"", 0, 0, 0, 0,
        WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_CONTROLPARENT | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);

    TransDetail.Create(hwnd, L"", 0, 0, 0, 0,
        WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_CONTROLPARENT | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);


    // Create a Warning label at bottom of the MainWindow to display warning messages.
    HWND hCtl = CustomLabel_SimpleLabel(hwnd, IDC_MAINWINDOW_WARNING, L"WARNING !!!", COLOR_YELLOW, COLOR_RED,
        CustomLabelAlignment::MiddleCenter, 0, 0, 0, 0);


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


// ========================================================================================
// Update the panel sizes based the completed splitter action.
// ========================================================================================
void UpdateSplitterChildren(HWND hwnd, int xDelta)
{
    int nRightPanelWidth = AfxGetWindowWidth(HWND_RIGHTPANEL) + xDelta;
    int nRightPanelHeight = AfxGetWindowHeight(HWND_RIGHTPANEL);
    SetWindowPos(HWND_RIGHTPANEL, 0,
        0, 0, nRightPanelWidth, nRightPanelHeight, SWP_NOZORDER | SWP_NOMOVE);

    RECT rc; GetClientRect(hwnd, &rc);
    MainWindow_OnSize(hwnd, SIZE_RESTORED, rc.right, rc.bottom);
    AfxRedrawWindow(HWND_MIDDLEPANEL);
    AfxRedrawWindow(HWND_RIGHTPANEL);
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
        HANDLE_MSG(m_hwnd, WM_CLOSE, MainWindow_OnClose);
        HANDLE_MSG(m_hwnd, WM_DESTROY, MainWindow_OnDestroy);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, MainWindow_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, MainWindow_OnPaint);
        HANDLE_MSG(m_hwnd, WM_SIZE, MainWindow_OnSize);
        HANDLE_MSG(m_hwnd, WM_MOUSEMOVE, MainWindow_OnMouseMove);
        HANDLE_MSG(m_hwnd, WM_LBUTTONDOWN, MainWindow_OnLButtonDown);
        HANDLE_MSG(m_hwnd, WM_LBUTTONUP, MainWindow_OnLButtonUp);
        HANDLE_MSG(m_hwnd, WM_SETCURSOR, MainWindow_OnSetCursor);


    case WM_SHOWWINDOW:
    {
        // Workaround for the Windows white flashing bug.
        // https://stackoverflow.com/questions/69715610/how-to-initialize-the-background-color-of-win32-app-to-something-other-than-whit

        SetWindowLongPtr(m_hwnd,
            GWL_EXSTYLE,
            GetWindowLongPtr(m_hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);

        if (!GetLayeredWindowAttributes(m_hwnd, NULL, NULL, NULL))
        {
            SetLayeredWindowAttributes(m_hwnd, 0, 0, LWA_ALPHA);
            DefWindowProc(m_hwnd, WM_ERASEBKGND, (WPARAM)GetDC(m_hwnd), lParam);
            SetLayeredWindowAttributes(m_hwnd, 0, 255, LWA_ALPHA);
            AnimateWindow(m_hwnd, 1, AW_ACTIVATE | AW_BLEND);
            return 0;
        }
        SetWindowLongPtr(m_hwnd,
            GWL_EXSTYLE,
            GetWindowLong(m_hwnd, GWL_EXSTYLE) & ~WS_EX_LAYERED);

        return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
    break;


    case MSG_STARTUP_SHOWTRADES:
    {
        MainWindow_StartupShowTrades();
        return 0;
    }

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

