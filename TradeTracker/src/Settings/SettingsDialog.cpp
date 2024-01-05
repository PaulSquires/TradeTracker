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

#include "Utilities/CWindowBase.h"
#include "MainWindow/MainWindow.h"
#include "Utilities/UserMessages.h"
#include "Config/Config.h"
#include "CustomLabel/CustomLabel.h"
#include "CustomTextBox/CustomTextBox.h"
#include "CustomMessageBox/CustomMessageBox.h"
#include "YearEndDialog/YearEndDialog.h"

#include "SettingsDialog.h"


HWND HWND_SETTINGSDIALOG = NULL;

extern int dialog_return_code;

CSettingsDialog SettingsDialog;



// ========================================================================================
// Process WM_CLOSE message for window/dialog: SettingsDialog
// ========================================================================================
void SettingsDialog_OnClose(HWND hwnd) {
    EnableWindow(GetParent(hwnd), true);
    DestroyWindow(hwnd);
}


// ========================================================================================
// Process WM_DESTROY message for window/dialog: SettingsDialog
// ========================================================================================
void SettingsDialog_OnDestroy(HWND hwnd) {
    PostQuitMessage(0);
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: SettingsDialog
// ========================================================================================
bool SettingsDialog_OnEraseBkgnd(HWND hwnd, HDC hdc) {
    // Handle all of the painting in WM_PAINT
    return true;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: SettingsDialog
// ========================================================================================
void SettingsDialog_OnPaint(HWND hwnd) {
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
// Process WM_CREATE message for window/dialog: SettingsDialog
// ========================================================================================
bool SettingsDialog_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct) {
    HWND_SETTINGSDIALOG = hwnd;

    DWORD text_color = COLOR_WHITELIGHT;
    DWORD text_color_dim = COLOR_WHITEDARK;
    DWORD back_color = COLOR_GRAYDARK;
    DWORD check_color = COLOR_WHITELIGHT;
    DWORD check_back_color = COLOR_GRAYDARK;
    DWORD border_focus_color = COLOR_GRAYLIGHT;

    std::wstring font_name = AfxGetDefaultFont();
    int font_size = 9;

    // Save the original config values that require a restart. These are checked when SAVE
    // is pressed to see if the original value was changed.
    SettingsDialog.orig_number_format_type = config.GetNumberFormatType();
    SettingsDialog.orig_costing_method = config.GetCostingMethod();

    HWND hCtl = NULL;

    int left = 75;
    int top = 40;
    int width = 400;
    int height = 23;
    int vert_spacing = 8;
    int left_indent = 30;

    hCtl = CustomLabel_SimpleCheckBox(hwnd, IDC_SETTINGSDIALOG_UPDATECHECK, L"Check for newer available TradeTracker versions",
        text_color, back_color, check_color, check_back_color, border_focus_color, left, top, width, height);
    CustomLabel_SetCheckState(hCtl, config.GetAllowUpdateCheck());

    top += height;
    hCtl = CustomLabel_SimpleCheckBox(hwnd, IDC_SETTINGSDIALOG_PORTFOLIOVALUE, L"Display Net and Excess portfolio liquidity amounts",
        text_color, back_color, check_color, check_back_color, border_focus_color, left, top, width, height);
    CustomLabel_SetCheckState(hCtl, config.GetAllowPortfolioDisplay());

    top += (height + vert_spacing);
    CustomLabel_SimpleLabel(hwnd, IDC_SETTINGSDIALOG_NUMBERFORMAT, L"Number format:",
        text_color, back_color, CustomLabelAlignment::middle_left, left, top, width, height);

    top += height;
    hCtl = CustomLabel_SimpleCheckBox(hwnd, IDC_SETTINGSDIALOG_NUMBERFORMAT_USA, L"American (US):  1,234.00",
        text_color, back_color, check_color, check_back_color, border_focus_color, left + left_indent, top, width, height);
    if (config.GetNumberFormatType() == NumberFormatType::American) CustomLabel_SetCheckState(hCtl, true);

    top += height;
    hCtl = CustomLabel_SimpleCheckBox(hwnd, IDC_SETTINGSDIALOG_NUMBERFORMAT_EU, L"European (EU):  1.234,00",
        text_color, back_color, check_color, check_back_color, border_focus_color, left + left_indent, top, width, height);
    if (config.GetNumberFormatType() == NumberFormatType::European) CustomLabel_SetCheckState(hCtl, true);

    top += (height + vert_spacing);
    CustomLabel_SimpleLabel(hwnd, IDC_SETTINGSDIALOG_COSTBASIS, L"Stock costing method:",
        text_color, back_color, CustomLabelAlignment::middle_left, left, top, width, height);

    top += height;
    hCtl = CustomLabel_SimpleCheckBox(hwnd, IDC_SETTINGSDIALOG_COSTBASIS_AVERAGE, L"Average Cost Basis",
        text_color, back_color, check_color, check_back_color, border_focus_color, left + left_indent, top, width, height);
    if (config.GetCostingMethod() == CostingMethod::AverageCost) CustomLabel_SetCheckState(hCtl, true);

    top += height;
    hCtl = CustomLabel_SimpleCheckBox(hwnd, IDC_SETTINGSDIALOG_COSTBASIS_FIFO, L"First-In, First-Out (FIFO)",
        text_color, back_color, check_color, check_back_color, border_focus_color, left + left_indent, top, width, height);
    if (config.GetCostingMethod() == CostingMethod::fifo) CustomLabel_SetCheckState(hCtl, true);


    // YEAR END CLOSE
    top += (height * 2);
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_SETTINGSDIALOG_CMDYEAREND, L"Year End Procedure",
        COLOR_BLACK, COLOR_RED, COLOR_RED, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_center, left, top, 150, height);
    CustomLabel_SetFont(hCtl, font_name, font_size, true);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);

    left = 440;
    width = 80;

    // SAVE button
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_SETTINGSDIALOG_SAVE, L"SAVE",
        COLOR_BLACK, COLOR_GREEN, COLOR_GREEN, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_center, left, top, width, height);
    CustomLabel_SetFont(hCtl, font_name, font_size, true);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);

    // CANCEL button
    top += height + 10;
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_SETTINGSDIALOG_CANCEL, L"Cancel",
        COLOR_BLACK, COLOR_RED, COLOR_RED, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_center, left, top, width, height);
    CustomLabel_SetFont(hCtl, font_name, font_size, true);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);

    return true;
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CSettingsDialog::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        HANDLE_MSG(m_hwnd, WM_CREATE, SettingsDialog_OnCreate);
        HANDLE_MSG(m_hwnd, WM_DESTROY, SettingsDialog_OnDestroy);
        HANDLE_MSG(m_hwnd, WM_CLOSE, SettingsDialog_OnClose);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, SettingsDialog_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, SettingsDialog_OnPaint);

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
        if (wParam == VK_TAB || wParam == VK_UP || wParam == VK_DOWN) {
            HWND hFocus = GetFocus();
            HWND hNextCtrl = NULL;
            if (((GetAsyncKeyState(VK_SHIFT) & 0x8000) && wParam == VK_TAB) || wParam == VK_UP) {
                hNextCtrl = GetNextDlgTabItem(m_hwnd, hFocus, true);
            } 
            else if (wParam == VK_TAB || wParam == VK_DOWN) {
                hNextCtrl = GetNextDlgTabItem(m_hwnd, hFocus, false);
            }
            SetFocus(hNextCtrl);
            return true;
        }
        return 0;
    }

    case MSG_CUSTOMLABEL_CLICK: {
        HWND hCtl = (HWND)lParam;
        int ctrl_id = (int)wParam;
        bool check_state = false;

        if (hCtl == NULL) return 0;


        if (ctrl_id == IDC_SETTINGSDIALOG_UPDATECHECK) {
            check_state = !CustomLabel_GetCheckState(hCtl);
            CustomLabel_SetCheckState(hCtl, check_state);
            SetFocus(hCtl);
        }

        if (ctrl_id == IDC_SETTINGSDIALOG_PORTFOLIOVALUE) {
            check_state = !CustomLabel_GetCheckState(hCtl);
            CustomLabel_SetCheckState(hCtl, check_state);
            SetFocus(hCtl);
        }

        if (ctrl_id == IDC_SETTINGSDIALOG_NUMBERFORMAT_USA) {
            check_state = CustomLabel_GetCheckState(hCtl);
            CustomLabel_SetCheckState(hCtl, !check_state);
            if (!check_state) {
                CustomLabel_SetCheckState(GetDlgItem(m_hwnd, IDC_SETTINGSDIALOG_NUMBERFORMAT_EU), false);
            }
            SetFocus(hCtl);
        }

        if (ctrl_id == IDC_SETTINGSDIALOG_NUMBERFORMAT_EU) {
            check_state = CustomLabel_GetCheckState(hCtl);
            CustomLabel_SetCheckState(hCtl, !check_state);
            if (!check_state) {
                CustomLabel_SetCheckState(GetDlgItem(m_hwnd, IDC_SETTINGSDIALOG_NUMBERFORMAT_USA), false);
            }
            SetFocus(hCtl);
        }

        if (ctrl_id == IDC_SETTINGSDIALOG_COSTBASIS_AVERAGE) {
            check_state = CustomLabel_GetCheckState(hCtl);
            CustomLabel_SetCheckState(hCtl, !check_state);
            if (!check_state) {
                CustomLabel_SetCheckState(GetDlgItem(m_hwnd, IDC_SETTINGSDIALOG_COSTBASIS_FIFO), false);
            }
            SetFocus(hCtl);
        }

        if (ctrl_id == IDC_SETTINGSDIALOG_COSTBASIS_FIFO) {
            check_state = CustomLabel_GetCheckState(hCtl);
            CustomLabel_SetCheckState(hCtl, !check_state);
            if (!check_state) {
                CustomLabel_SetCheckState(GetDlgItem(m_hwnd, IDC_SETTINGSDIALOG_COSTBASIS_AVERAGE), false);
            }
            SetFocus(hCtl);
        }

        if (ctrl_id == IDC_SETTINGSDIALOG_CMDYEAREND) {
            HWND hFocus = GetFocus();
            YearEndDialog_Show(m_hwnd);
            SetFocus(hFocus);
        }

        if (ctrl_id == IDC_SETTINGSDIALOG_CANCEL) {
            dialog_return_code = DIALOG_RETURN_CANCEL;
            SendMessage(m_hwnd, WM_CLOSE, 0, 0);
        }

        if (ctrl_id == IDC_SETTINGSDIALOG_SAVE) {

            bool bool_value = CustomLabel_GetCheckState(GetDlgItem(m_hwnd, IDC_SETTINGSDIALOG_UPDATECHECK));
            config.SetAllowUpdateCheck(bool_value);

            bool_value = CustomLabel_GetCheckState(GetDlgItem(m_hwnd, IDC_SETTINGSDIALOG_PORTFOLIOVALUE));
            config.SetAllowPortfolioDisplay(bool_value);

            bool_value = CustomLabel_GetCheckState(GetDlgItem(m_hwnd, IDC_SETTINGSDIALOG_NUMBERFORMAT_USA));
            config.SetNumberFormatType((bool_value) ? NumberFormatType::American : NumberFormatType::European);

            bool_value = CustomLabel_GetCheckState(GetDlgItem(m_hwnd, IDC_SETTINGSDIALOG_COSTBASIS_FIFO));
            config.SetCostingMethod((bool_value) ? CostingMethod::fifo : CostingMethod::AverageCost);

            config.SaveConfig();


            bool restart_required = false;
            if (orig_number_format_type != config.GetNumberFormatType()) restart_required = true;
            if (orig_costing_method != config.GetCostingMethod()) restart_required = true;

            if (restart_required) {
                CustomMessageBox.Show(
                    MainWindow.hWindow,
                    L"A setting has changed that requires a restart to take effect.\nPlease close and restart the application.\n",
                    L"Settings",
                    MB_ICONWARNING | MB_OK);
            }

            dialog_return_code = DIALOG_RETURN_OK;
            SendMessage(m_hwnd, WM_CLOSE, 0, 0);
            
        }

        return 0;
    }

    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}


// ========================================================================================
// Create and show the Settings modal dialog.
// ========================================================================================
int SettingsDialog_Show(HWND hWndParent) {
    int width = 570;
    int height = 380;

    HWND hwnd = SettingsDialog.Create(hWndParent, L"Settings", 0, 0, width, height,
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_CONTROLPARENT | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);

    // Attempt to apply the standard Windows dark theme to the non-client areas of the main form.
    BOOL value = TRUE;
    ::DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));

    HBRUSH hbrBackground = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)hbrBackground);

    HANDLE hIconSmall = LoadImage(SettingsDialog.hInst(), MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON, 16, 16, LR_SHARED);
    SendMessage(hwnd, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIconSmall);

    AfxCenterWindowMonitorAware(hwnd, hWndParent);

    EnableWindow(hWndParent, false);

    // Fix Windows 10 white flashing
    BOOL cloak = TRUE;
    DwmSetWindowAttribute(hwnd, DWMWA_CLOAK, &cloak, sizeof(cloak));

    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);

    cloak = FALSE;
    DwmSetWindowAttribute(hwnd, DWMWA_CLOAK, &cloak, sizeof(cloak));

    SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    SetFocus(GetDlgItem(hwnd, IDC_SETTINGSDIALOG_SAVE));

    dialog_return_code = DIALOG_RETURN_CANCEL;

    // Call modal message pump and wait for it to end.
    MSG msg{};
    while (GetMessage(&msg, NULL, 0, 0)) {
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

