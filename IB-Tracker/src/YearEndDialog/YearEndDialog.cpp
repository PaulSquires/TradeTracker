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

#include "YearEndDialog.h"
#include "Database/trade.h"
#include "Database/database.h"
#include "MainWindow/MainWindow.h"
#include "CustomLabel/CustomLabel.h"
#include "CustomTextBox/CustomTextBox.h"
#include "ClosedTrades/ClosedTrades.h"
#include "Utilities/UserMessages.h"


HWND HWND_YEARENDDIALOG = NULL;

CYearEndDialog YearEndDialog;



// ========================================================================================
// Process the Year End closing procedure
// ========================================================================================
bool YearEndDialog_Process(HWND hwnd)
{
    HWND hTextDate = GetDlgItem(hwnd, IDC_YEARENDDIALOG_TXTDATE);

    std::wstring wszYear = AfxGetWindowText(hTextDate);

    std::wstring text = L"Remove closed Trades on or before December 31, " + wszYear 
        + L"? \n\nDo not continue if you have not made a database backup.";
    
    if (MessageBox(hwnd, text.c_str(), L"Remove Closed Trades", MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_DEFBUTTON2) != IDYES) {
        return false;
    }

    SetCursor(LoadCursor(0, IDC_WAIT));

    std::wstring wszCloseDate = wszYear + L"-12-31";

    // Iterate the trades and remove all closed trades before the close date.
    auto iter = trades.begin();
    while (iter != trades.end())
    {
        if (!(*iter)->is_open) {

            // Iterate to find the latest closed date
            std::wstring wszTradeCloseDate = L"0000-00-00";
            for (auto& trans : (*iter)->transactions) {
                if (trans->trans_date > wszTradeCloseDate) {
                    wszTradeCloseDate = trans->trans_date;
                }
            }

            if (wszTradeCloseDate <= wszCloseDate) {
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


    SaveDatabase();
    LoadDatabase();
    
    ClosedTrades_ShowClosedTrades();

    SetCursor(LoadCursor(0, IDC_ARROW));

    return true;
}




// ========================================================================================
// Process WM_CLOSE message for window/dialog: YearEndDialog
// ========================================================================================
void YearEndDialog_OnClose(HWND hwnd)
{
    MainWindow_BlurPanels(false);
    EnableWindow(HWND_MAINWINDOW, TRUE);
    DestroyWindow(hwnd);
}


// ========================================================================================
// Process WM_DESTROY message for window/dialog: YearEndDialog
// ========================================================================================
void YearEndDialog_OnDestroy(HWND hwnd)
{
    PostQuitMessage(0);
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: YearEndDialog
// ========================================================================================
BOOL YearEndDialog_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    HWND_YEARENDDIALOG = hwnd;

    HWND hCtl = NULL;
    
    std::wstring font_name = L"Segoe UI";
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
        CustomLabelAlignment::middle_center, 580, 396, 80, 23);
    CustomLabel_SetFont(hCtl, font_name, font_size, true);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);

    return TRUE;
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: YearEndDialog
// ========================================================================================
BOOL YearEndDialog_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    // Handle all of the painting in WM_PAINT
    return TRUE;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: YearEndDialog
// ========================================================================================
void YearEndDialog_OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    SaveDC(hdc);

    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP hbit = CreateCompatibleBitmap(hdc, ps.rcPaint.right, ps.rcPaint.bottom);
    SelectBitmap(memDC, hbit);

    Graphics graphics(memDC);
    int nWidth = (ps.rcPaint.right - ps.rcPaint.left);
    int nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);

    // Create the background brush
    SolidBrush back_brush(COLOR_GRAYDARK);
    graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

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
LRESULT CYearEndDialog::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(m_hwnd, WM_CREATE, YearEndDialog_OnCreate);
        HANDLE_MSG(m_hwnd, WM_DESTROY, YearEndDialog_OnDestroy);
        HANDLE_MSG(m_hwnd, WM_CLOSE, YearEndDialog_OnClose);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, YearEndDialog_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, YearEndDialog_OnPaint);


    case WM_SHOWWINDOW:
    {
        // Workaround for the Windows 11 (The cloaking solution seems to work only
        // on Windows 10 whereas this WM_SHOWWINDOW workaround seems to only work
        // on Windows 11).
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
    return 0;


    case WM_KEYDOWN:
    {
        // We are handling the TAB naviagation ourselves.
        if (wParam == VK_TAB) {
            HWND hFocus = GetFocus();
            HWND hNextCtrl = NULL;
            if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
                hNextCtrl = GetNextDlgTabItem(m_hwnd, hFocus, TRUE);
            }
            else {
                hNextCtrl = GetNextDlgTabItem(m_hwnd, hFocus, FALSE);
            }
            SetFocus(hNextCtrl);
            return TRUE;
        }
    }
    return 0;


    case MSG_DATEPICKER_DATECHANGED:
    {
    }
    return 0;


    case MSG_CUSTOMLABEL_CLICK:
    {
        HWND hCtl = (HWND)lParam;
        int CtrlId = (int)wParam;

        if (hCtl == NULL) return 0;

        if (CtrlId == IDC_YEARENDDIALOG_PROCESS) {
            if (YearEndDialog_Process(m_hwnd) == true) {
                dialog_return_code = DIALOG_RETURN_OK;
                SendMessage(m_hwnd, WM_CLOSE, 0, 0);
            }
        }

        return 0;
    }

    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}


// ========================================================================================
// Create and show the YearEnd Close modal dialog.
// ========================================================================================
int YearEndDialog_Show()
{
    int nWidth = 715;
    int nHeight = 500;

    HWND hwnd = YearEndDialog.Create(HWND_MAINWINDOW, L"Year End Close", 0, 0, nWidth, nHeight,
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_CONTROLPARENT | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);

    // Attempt to apply the standard Windows dark theme to the non-client areas of the main form.
    BOOL value = true;
    ::DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));

    HBRUSH hbrBackground = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)hbrBackground);

    HANDLE hIconSmall = LoadImage(YearEndDialog.hInst(), MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON, 16, 16, LR_SHARED);
    SendMessage(hwnd, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIconSmall);


    // Blur the underlying MainWindow panels in order to reduce "visual noise" while
    // our Trade Management popup is active. The MainWindow panels are shown again
    // during our call to YearEndDialog_OnClose() prior to enabling the MainWindow
    // and this popup closing.
    MainWindow_BlurPanels(true);

    AfxCenterWindow(hwnd, HWND_MAINWINDOW);

    EnableWindow(HWND_MAINWINDOW, FALSE);

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

