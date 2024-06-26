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

#include "pch.h"

#include "YearEndDialog.h"
#include "Database/trade.h"
#include "Database/database.h"
#include "MainWindow/MainWindow.h"
#include "CustomLabel/CustomLabel.h"
#include "CustomTextBox/CustomTextBox.h"
#include "CustomMessageBox/CustomMessageBox.h"
#include "ClosedTrades/ClosedTrades.h"
#include "Utilities/UserMessages.h"


HWND HWND_YEARENDDIALOG = NULL;

CYearEndDialog YearEndDialog;



// ========================================================================================
// Process the Year End closing procedure
// ========================================================================================
bool YearEndDialog_Process(HWND hwnd) {
    HWND hTextDate = GetDlgItem(hwnd, IDC_YEARENDDIALOG_TXTDATE);

    std::wstring year_text = CustomTextBox_GetText(hTextDate);

    std::wstring text = L"Remove closed Trades on or before December 31, " + year_text 
        + L"? \n\nDo not continue if you have not made a database backup.";
    
    if (CustomMessageBox.Show(hwnd, text, L"Remove Closed Trades", MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_DEFBUTTON2) != IDYES) {
        return false;
    }

    SetCursor(LoadCursor(0, IDC_WAIT));

    std::wstring wszCloseDate = year_text + L"-12-31";

    // Iterate the trades and remove all closed trades before the close date.
    auto iter = trades.begin();
    while (iter != trades.end())
    {
        if (!(*iter)->is_open) {

            // Iterate to find the latest closed date
            std::wstring trade_close_date = L"0000-00-00";
            for (auto& trans : (*iter)->transactions) {
                if (trans->trans_date > trade_close_date) {
                    trade_close_date = trans->trans_date;
                }
            }

            if (trade_close_date <= wszCloseDate) {
                iter = trades.erase(iter);
            } else {
                iter++;
            }
        }
        else
        {
            iter++;
        }
    }

    db.SaveDatabase();
    
    ClosedTrades.ShowClosedTrades();

    SetCursor(LoadCursor(0, IDC_ARROW));

    return true;
}




// ========================================================================================
// Process WM_CLOSE message for window/dialog: YearEndDialog
// ========================================================================================
void YearEndDialog_OnClose(HWND hwnd) {
    MainWindow.BlurPanels(false);
    EnableWindow(YearEndDialog.hParent, true);
    DestroyWindow(hwnd);
}


// ========================================================================================
// Process WM_DESTROY message for window/dialog: YearEndDialog
// ========================================================================================
void YearEndDialog_OnDestroy(HWND hwnd) {
    PostQuitMessage(0);
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: YearEndDialog
// ========================================================================================
bool YearEndDialog_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct) {
    HWND_YEARENDDIALOG = hwnd;

    HWND hCtl = NULL;
    
    std::wstring font_name = AfxGetDefaultFont();
    int font_size = 9;

    std::wstring text;
    int cx = 715;  // AfxGetWindowWidth(hwnd);

    text = L"*** IMPORTANT: BACKUP YOUR DATABASE PRIOR TO RUNNING THIS PROCEDURE ***";
    hCtl = CustomLabel_SimpleLabel(hwnd, -1, text, COLOR_RED, COLOR_GRAYDARK,
        CustomLabelAlignment::middle_center, 0, 40, cx, 23);
    CustomLabel_SetFont(hCtl, font_name, font_size, true);

    text = L"All closed trades dated before December 31 of the selected year will be removed from the database.";
    hCtl = CustomLabel_SimpleLabel(hwnd, -1, text, COLOR_WHITELIGHT, COLOR_GRAYDARK,
        CustomLabelAlignment::middle_center, 0, 80, cx, 23);

    text = L"YEAR:";
    hCtl = CustomLabel_SimpleLabel(hwnd, -1, text, COLOR_WHITELIGHT, COLOR_GRAYDARK,
        CustomLabelAlignment::middle_left, 290, 110, 45, 23);

    text = std::to_wstring(AfxGetYear(AfxCurrentDate()) - 1);
    hCtl = CreateCustomTextBox(hwnd, IDC_YEARENDDIALOG_TXTDATE, false, ES_CENTER, text, 335, 110, 50, 23);
    CustomTextBox_SetMargins(hCtl, 0, 3);
    CustomTextBox_SetColors(hCtl, COLOR_WHITELIGHT, COLOR_GRAYMEDIUM);
    CustomTextBox_SetNumericAttributes(hCtl, 0, CustomTextBoxNegative::disallow, CustomTextBoxFormatting::disallow);

    // PROCESS button
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_YEARENDDIALOG_PROCESS, L"PROCESS",
        COLOR_BLACK, COLOR_GREEN, COLOR_GREEN, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_center, 580, 390, 80, 23);
    CustomLabel_SetFont(hCtl, font_name, font_size, true);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);

    // CANCEL button
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_YEARENDDIALOG_CANCEL, L"Cancel",
        COLOR_BLACK, COLOR_RED, COLOR_RED, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_center, 580, 423, 80, 23);
    CustomLabel_SetFont(hCtl, font_name, font_size, true);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);

    return true;
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: YearEndDialog
// ========================================================================================
bool YearEndDialog_OnEraseBkgnd(HWND hwnd, HDC hdc) {
    // Handle all of the painting in WM_PAINT
    return true;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: YearEndDialog
// ========================================================================================
void YearEndDialog_OnPaint(HWND hwnd) {
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    SaveDC(hdc);

    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP hbit = CreateCompatibleBitmap(hdc, ps.rcPaint.right, ps.rcPaint.bottom);
    SelectBitmap(memDC, hbit);

    Graphics graphics(memDC);
    int width = (ps.rcPaint.right - ps.rcPaint.left);
    int height = (ps.rcPaint.bottom - ps.rcPaint.top);

    // Create the background brush
    SolidBrush back_brush(COLOR_GRAYDARK);
    graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, width, height);

    // Copy the entire memory bitmap to the main display
    BitBlt(hdc, 0, 0, ps.rcPaint.right, ps.rcPaint.bottom, memDC, 0, 0, SRCCOPY);

    // Restore the original state of the DC
    RestoreDC(hdc, -1);

    // Cleanup
    DeleteObject(hbit);
    DeleteDC(memDC);

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CYearEndDialog::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        HANDLE_MSG(m_hwnd, WM_CREATE, YearEndDialog_OnCreate);
        HANDLE_MSG(m_hwnd, WM_DESTROY, YearEndDialog_OnDestroy);
        HANDLE_MSG(m_hwnd, WM_CLOSE, YearEndDialog_OnClose);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, YearEndDialog_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, YearEndDialog_OnPaint);

    case WM_SHOWWINDOW: {
        // Workaround for the Windows 11 (The cloaking solution seems to work only
        // on Windows 10 whereas this WM_SHOWWINDOW workaround seems to only work
        // on Windows 11).
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

    case WM_KEYDOWN: {
        // We are handling the TAB naviagation ourselves.
        if (wParam == VK_TAB) {
            HWND hFocus = GetFocus();
            HWND hNextCtrl = NULL;
            if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
                hNextCtrl = GetNextDlgTabItem(m_hwnd, hFocus, true);
            }
            else {
                hNextCtrl = GetNextDlgTabItem(m_hwnd, hFocus, false);
            }
            SetFocus(hNextCtrl);
            return true;
        }
        break;
    }

    case MSG_CUSTOMLABEL_CLICK: {
        HWND hCtl = (HWND)lParam;
        int ctrl_id = (int)wParam;

        if (!hCtl) return 0;

        if (ctrl_id == IDC_YEARENDDIALOG_PROCESS) {
            if (YearEndDialog_Process(m_hwnd) == true) {
                dialog_return_code = DIALOG_RETURN_OK;
                SendMessage(m_hwnd, WM_CLOSE, 0, 0);
            }
        }

        if (ctrl_id == IDC_YEARENDDIALOG_CANCEL) {
            dialog_return_code = DIALOG_RETURN_CANCEL;
            SendMessage(m_hwnd, WM_CLOSE, 0, 0);
        }

        return 0;
    }

    }

    return DefWindowProc(m_hwnd, msg, wParam, lParam);
}


// ========================================================================================
// Create and show the YearEnd Close modal dialog.
// ========================================================================================
int YearEndDialog_Show(HWND hwndParent) {
    int width = 715;
    int height = 500;
    
    YearEndDialog.hParent = hwndParent;

    HWND hwnd = YearEndDialog.Create(hwndParent, L"Year End Close", 0, 0, width, height,
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_CONTROLPARENT | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);

    // Attempt to apply the standard Windows dark theme to the non-client areas of the main form.
    BOOL value = TRUE;
    ::DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));

    HBRUSH hbrBackground = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)hbrBackground);

    HANDLE hIconSmall = LoadImage(YearEndDialog.hInst(), MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON, 16, 16, LR_SHARED);
    SendMessage(hwnd, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIconSmall);


    // Blur the underlying MainWindow panels in order to reduce "visual noise" while
    // our Trade Management popup is active. The MainWindow panels are shown again
    // during our call to YearEndDialog_OnClose() prior to enabling the MainWindow
    // and this popup closing.
    MainWindow.BlurPanels(true);

    AfxCenterWindowMonitorAware(hwnd, MainWindow.hWindow);

    EnableWindow(hwndParent, false);

    // Fix Windows 10 white flashing
    BOOL cloak = TRUE;
    DwmSetWindowAttribute(hwnd, DWMWA_CLOAK, &cloak, sizeof(cloak));

    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);

    cloak = FALSE;
    DwmSetWindowAttribute(hwnd, DWMWA_CLOAK, &cloak, sizeof(cloak));

    SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    SetFocus(GetDlgItem(hwnd, IDC_YEARENDDIALOG_TXTDATE));

    dialog_return_code = DIALOG_RETURN_CANCEL;

    // Call modal message pump and wait for it to end.
    MSG msg{};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (msg.message == WM_KEYUP && msg.wParam == VK_ESCAPE) {
            SendMessage(hwnd, WM_CLOSE, 0, 0);
        }

        // We handle VK_TAB processing ourselves rather than using IsDialogMessage
        // Translates virtual-key messages into character messages.
        TranslateMessage(&msg);
 
        // Dispatches a message to a window procedure.
        DispatchMessage(&msg);
    }

    return dialog_return_code;
}

