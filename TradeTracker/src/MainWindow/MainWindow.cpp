/*

MIT License

Copyright(c) 2023-2024 Paul Squires

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
// Defines the entry point for the TradeTracker application.
//

#include "pch.h"

#include "tws-client.h"
#include "TradeHistory/TradeHistory.h"
#include "ActiveTrades/ActiveTrades.h"
#include "ClosedTrades/ClosedTrades.h"
#include "Transactions/TransPanel.h"
#include "Transactions/TransDetail.h"
#include "TickerTotals/TickerTotals.h"
#include "JournalNotes/JournalNotes.h"
#include "TradePlan/TradePlan.h"
#include "TabPanel/TabPanel.h"
#include "CustomLabel/CustomLabel.h"
#include "CustomPopupMenu/CustomPopupMenu.h"
#include "Utilities/UpdateCheck.h"
#include "Utilities/UserMessages.h"
#include "Config/Config.h"

#include "MainWindow.h"


CMainWindowShadow Shadow;
CMainWindow MainWindow;

RECT rcSplitter{};
bool is_dragging = false;    // If dragging our splitter
POINT prev_pt{};            // for tracking current splitter drag


// ========================================================================================
// Display Paper Trading warning message if port is enabled. 
// Normal Trading 7496;   7497 is paper trading account.
// ========================================================================================
void CMainWindow::DisplayPaperTradingWarning() {
    CustomLabel_SetText(GetDlgItem(MainWindow.hWindow, IDC_MAINWINDOW_WARNING),
        L"*** USING PAPER TRADING ACCOUNT ***");
    ShowWindow(GetDlgItem(MainWindow.hWindow, IDC_MAINWINDOW_WARNING), SW_SHOWNORMAL);
}


// ========================================================================================
// Check the server to determine if an update is available to be downloaded.
// ========================================================================================
void CMainWindow::DisplayUpdateAvailableMessage() {
    if (!config.IsUpdateCheckActive()) return;
    PerformUpdateCheck();
}


// ========================================================================================
// Set the HWND for the panel that will display on the right side of the MainWindow.
// Also place it into position and hide previous HWND of right panel.
// ========================================================================================
void CMainWindow::SetRightPanel(HWND hPanel) {
    // If the incoming panel is already set as the right panel then simply exit.
    if (hPanel == hRightPanel) return;

    // Get the current position size of the current right panel.
    RECT rc; GetWindowRect(hRightPanel, &rc);
    MapWindowPoints(HWND_DESKTOP, hWindow, (LPPOINT)&rc, 2);

    ShowWindow(hRightPanel, SW_HIDE);

    hRightPanel = hPanel;
    SetWindowPos(hRightPanel, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
        SWP_NOZORDER | SWP_SHOWWINDOW);
}


// ========================================================================================
// Get the HWND for the panel that is currently displayed in the left of the MainWindow.
// ========================================================================================
HWND CMainWindow::GetLeftPanel() {
    return hLeftPanel;
}

    
// ========================================================================================
// Set the HWND for the panel that will display in the left of the MainWindow.
// Also place it into position and hide previous HWND of left panel.
// ========================================================================================
void CMainWindow::SetLeftPanel(HWND hPanel) {
    // If the incoming panel is already set as the middle panel then simply exit.
    if (hPanel == hLeftPanel) return;

    // Get the current position size of the current left panel.
    RECT rc; GetWindowRect(hLeftPanel, &rc);
    MapWindowPoints(HWND_DESKTOP, hWindow, (LPPOINT)&rc, 2);

    ShowWindow(hLeftPanel, SW_HIDE);

    hLeftPanel = hPanel;
    SetWindowPos(hLeftPanel, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
        SWP_NOZORDER | SWP_SHOWWINDOW);
}


// ========================================================================================
// Blur the MainWindow panels. This function is called prior and after
// a popup dialog is shown in order to reduce the underlying "visual noise" for that 
// popup window. For example, the Trade Management popup is easier to use and enter data
// into if the user is not distracted by the MainWindow menu panel, trades and history
// data displaying behind the popup.
// ========================================================================================
void CMainWindow::BlurPanels(bool active) {
    if (isWineActive()) return;

    if (active) {
        RECT rc;       GetWindowRect(hWindow, &rc);
        RECT rcClient; GetClientRect(hWindow, &rcClient);
        
        int border_thickness = ((rc.right - rc.left) - rcClient.right) / 2;
        InflateRect(&rc, -border_thickness, -border_thickness);

        if (Shadow.WindowHandle() == NULL) {
            Shadow.Create(hWindow, L"", 0, 0, 0, 0,
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
// Process WM_CLOSE message for window/dialog: MainWindow
// ========================================================================================
void CMainWindow::OnClose(HWND hwnd) {
    // Disconnect from IBKR TWS and shut down monitoring thread.
    tws_Disconnect();

    // Destroy the popup shadow window should it exist.
    if (Shadow.WindowHandle()) DestroyWindow(Shadow.WindowHandle());

    // Save the Config file so that startup_width and startup_height will persist
    // for the time application is executed.
    config.SetStartupWidth(AfxGetWindowWidth(hwnd));
    config.SetStartupHeight(AfxGetWindowHeight(hwnd));
    config.SetStartupRightPanelWidth(AfxGetWindowWidth(hRightPanel));

    config.SaveConfig();

    DestroyWindow(hwnd);
}


// ========================================================================================
// Process WM_DESTROY message for window/dialog: MainWindow
// ========================================================================================
void CMainWindow::OnDestroy(HWND hwnd) {
    // Quit the application
    PostQuitMessage(0);
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: MainWindow
// ========================================================================================
bool CMainWindow::OnEraseBkgnd(HWND hwnd, HDC hdc) {
    // Handle all of the painting in WM_PAINT
    // We do not need to paint the background because the full client area will always
    // be covered by child windows (NavPanel, Trades Panel, History Panel)
    return true;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: MainWindow
// ========================================================================================
void CMainWindow::OnPaint(HWND hwnd) {
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    Graphics graphics(hdc);

    // Create the background brush
    SolidBrush back_brush(COLOR_BLACK);

    int width = (ps.rcPaint.right - ps.rcPaint.left);
    int height = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, width, height);

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Process WM_SIZE message for window/dialog: MainWindow
// ========================================================================================
void CMainWindow::OnSize(HWND hwnd, UINT state, int cx, int cy) {
    // Position all of the child windows
    if (state == SIZE_MINIMIZED) return;
    
    int MARGIN = AfxScaleY(TABPANEL_HEIGHT);
    int INNER_MARGIN = AfxScaleY(6);
    int SPLITTER_WIDTH = AfxScaleX(6);

    HDWP hdwp = BeginDeferWindowPos(6);

    // Position the right hand side Panel
    int right_panel_width = AfxGetWindowWidth(hRightPanel);
    int right_panel_left = cx - right_panel_width - INNER_MARGIN;
    hdwp = DeferWindowPos(hdwp, hRightPanel, 0,
                right_panel_left, 0, right_panel_width, cy - MARGIN,
                SWP_NOZORDER | SWP_SHOWWINDOW);

        
    // Position the left hand side Panel
    int left_panel_width = (cx - right_panel_width - SPLITTER_WIDTH - INNER_MARGIN);
    hdwp = DeferWindowPos(hdwp, hLeftPanel, 0,
                0, 0, left_panel_width, cy - MARGIN,
                SWP_NOZORDER | SWP_SHOWWINDOW);


    // Position the Warning label
    HWND hCtl = GetDlgItem(hwnd, IDC_MAINWINDOW_WARNING);
    int top = AfxScaleY(20);
    int height = AfxScaleY(16);
    int width = left_panel_width; 
    DeferWindowPos(hdwp, hCtl, 0, 0, cy - top, width, height, SWP_NOZORDER);

    // Move the bottom TabPanel into place
    height = AfxScaleY(TABPANEL_HEIGHT);
    top = height;
    width = left_panel_width; 
    DeferWindowPos(hdwp, HWND_TABPANEL, 0, 0, cy - top, width, height, SWP_NOZORDER | SWP_SHOWWINDOW);

    // Position the Update Available label
    hCtl = GetDlgItem(hwnd, IDC_MAINWINDOW_UPDATEAVAILABLE);
    width = right_panel_width; 
    DeferWindowPos(hdwp, hCtl, 0, cx - right_panel_width, cy - top, width - SPLITTER_WIDTH, height, SWP_NOZORDER);

    EndDeferWindowPos(hdwp);

    // Calculate the area for the "splitter control"
    rcSplitter.left = left_panel_width;
    rcSplitter.top = 0;
    rcSplitter.right = rcSplitter.left + SPLITTER_WIDTH;
    rcSplitter.bottom = cy;
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: MainWindow
// ========================================================================================
bool CMainWindow::OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct) {
    hWindow = hwnd;

    hRightPanel = TradeHistory.Create(hwnd, L"", 0, 0, config.GetStartupRightPanelWidth(), 0,
        WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_CONTROLPARENT | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);

    hLeftPanel = ActiveTrades.Create(hwnd, L"", 0, 0, 0, 0,
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

    JournalNotes.Create(hwnd, L"", 0, 0, 0, 0,
        WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_CONTROLPARENT | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);

    TradePlan.Create(hwnd, L"", 0, 0, 0, 0,
        WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_CONTROLPARENT | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);

    TabPanel.Create(hwnd, L"", 0, 0, 0, 0,
        WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_CONTROLPARENT | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);

    // Create a Warning label at bottom of the MainWindow to display warning messages.
    HWND hCtl = CustomLabel_SimpleLabel(hwnd, IDC_MAINWINDOW_WARNING, L"", COLOR_YELLOW, COLOR_RED,
        CustomLabelAlignment::middle_center, 0, 0, 0, 0);
    ShowWindow(hCtl, SW_HIDE);

    // Create an Update Available label at bottom of the MainWindow to display new version available message.
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_MAINWINDOW_UPDATEAVAILABLE, L"",
        COLOR_YELLOW, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_YELLOW,
        CustomLabelAlignment::middle_right, 0, 0, 0, 0);
    CustomLabel_SetMousePointer(hCtl, CustomLabelPointer::hand, CustomLabelPointer::hand);
    ShowWindow(hCtl, SW_HIDE);

    return true;
}


// ========================================================================================
// Determine if mouse cursor is over our splitter control area.
// ========================================================================================
bool CMainWindow::IsMouseOverSplitter(HWND hwnd) {
    POINT pt; GetCursorPos(&pt);
    MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&pt, 1);

    if (PtInRect(&rcSplitter, pt)) {
        SetCursor(LoadCursor(NULL, (LPCWSTR)IDC_SIZEWE));
        return true;
    }
    return false;
}


// ========================================================================================
// Process WM_LBUTTONDOWN message for window/dialog: MainWindow
// ========================================================================================
void CMainWindow::OnLButtonDown(HWND hwnd, bool fDoubleClick, int x, int y, UINT keyFlags) {
    if (IsMouseOverSplitter(hwnd)) {
        is_dragging = true;
        prev_pt.x = x;
        SetCapture(hwnd);
    }
}


// ========================================================================================
// Update the panel sizes based the completed splitter action.
// ========================================================================================
void CMainWindow::UpdateSplitterChildren(HWND hwnd, int xdelta) {
    int right_panel_width = AfxGetWindowWidth(hRightPanel) + xdelta;
    int right_panel_height = AfxGetWindowHeight(hRightPanel);
    SetWindowPos(hRightPanel, 0,
        0, 0, right_panel_width, right_panel_height, SWP_NOZORDER | SWP_NOMOVE);

    RECT rc; GetClientRect(hwnd, &rc);
    OnSize(hwnd, SIZE_RESTORED, rc.right, rc.bottom);
    AfxRedrawWindow(hLeftPanel);
    AfxRedrawWindow(hRightPanel);
    AfxRedrawWindow(hwnd);
}


// ========================================================================================
// Process WM_MOUSEMOVE message for window/dialog: MainWindow
// ========================================================================================
void CMainWindow::OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags) {
    if (is_dragging) {
        // Calculate the change between mouse movements and resize
        // the child windows accordingly.
        int xdelta = prev_pt.x - x;

        // Only reposition after a cummulative greater than 3 pixel move
        // in order to reduce the amount of screen updating.
        if (abs(xdelta) > 3) {
            UpdateSplitterChildren(hwnd, xdelta);
            prev_pt.x = x;
        }
    }
}


// ========================================================================================
// Process WM_LBUTTONUP message for window/dialog: MainWindow
// ========================================================================================
void CMainWindow::OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags) {
    if (is_dragging) {
        is_dragging = false;
        int xdelta = prev_pt.x - x;
        UpdateSplitterChildren(hwnd, xdelta);
        ReleaseCapture();
    }
}


// ========================================================================================
// Process WM_SETCURSOR message for window/dialog: MainWindow
// ========================================================================================
bool CMainWindow::OnSetCursor(HWND hwnd, HWND hwndCursor, UINT codeHitTest, UINT msg) {
    LPWSTR curId = IDC_ARROW;

    // Determine if the mouse is over our "splitter control"
    if (IsMouseOverSplitter(hwnd)) {
        SetCursor(LoadCursor(NULL, (LPCWSTR)IDC_SIZEWE));
        return true;
    }

    switch (codeHitTest) {
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
    return true;
}


// ========================================================================================
// Windows callback function (MainWindow).
// ========================================================================================
LRESULT CMainWindow::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        HANDLE_MSG(m_hwnd, WM_CREATE, OnCreate);
        HANDLE_MSG(m_hwnd, WM_CLOSE, OnClose);
        HANDLE_MSG(m_hwnd, WM_DESTROY, OnDestroy);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, OnPaint);
        HANDLE_MSG(m_hwnd, WM_SIZE, OnSize);
        HANDLE_MSG(m_hwnd, WM_MOUSEMOVE, OnMouseMove);
        HANDLE_MSG(m_hwnd, WM_LBUTTONDOWN, OnLButtonDown);
        HANDLE_MSG(m_hwnd, WM_LBUTTONUP, OnLButtonUp);
        HANDLE_MSG(m_hwnd, WM_SETCURSOR, OnSetCursor);

    case WM_ACTIVATEAPP: {
        // Application is losing focus so destroy any visible popup
        // combobox popups.
        if (wParam == false) {
            PostMessage(m_hwnd, WM_USER, NULL, NULL);
            if (IsWindowVisible(HWND_CUSTOMPOPUPMENU)) {
                DestroyWindow(HWND_CUSTOMPOPUPMENU);
            }
        }
        return 0;
    }

    case WM_SHOWWINDOW: {
        // Workaround for the Windows white flashing bug.
        // https://stackoverflow.com/questions/69715610/how-to-initialize-the-background-color-of-win32-app-to-something-other-than-whit

        SetWindowLongPtr(m_hwnd,
            GWL_EXSTYLE,
            GetWindowLongPtr(m_hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);

        if (!GetLayeredWindowAttributes(m_hwnd, NULL, NULL, NULL)) {
            HDC hdc = GetDC(m_hwnd);
            SetLayeredWindowAttributes(m_hwnd, 0, 0, LWA_ALPHA);
            DefWindowProc(m_hwnd, WM_ERASEBKGND, (WPARAM)hdc, lParam);
            SetLayeredWindowAttributes(m_hwnd, 0, 255, LWA_ALPHA);
            AnimateWindow(m_hwnd, 1, AW_ACTIVATE | AW_BLEND);
            ReleaseDC(m_hwnd, hdc);
            return 0;
        }
        SetWindowLongPtr(m_hwnd,
            GWL_EXSTYLE,
            GetWindowLong(m_hwnd, GWL_EXSTYLE) & ~WS_EX_LAYERED);

        return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }

    case MSG_CUSTOMLABEL_CLICK: {
        HWND hCtl = (HWND)lParam;
        int ctrl_id = (int)wParam;

        if (hCtl == NULL) return 0;

        if (ctrl_id == IDC_MAINWINDOW_UPDATEAVAILABLE) {
            ShowReleasesWebPage();
        }
        return 0;
    }

    }
    
    return DefWindowProc(m_hwnd, msg, wParam, lParam);
}


// ========================================================================================
// Windows callback function (MainWindowShadow).
// ========================================================================================
LRESULT CMainWindowShadow::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(m_hwnd, msg, wParam, lParam);
}

