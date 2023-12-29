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
#include "ActiveTrades/ActiveTrades.h"
#include "TradeDialog/TradeDialog.h"
#include "Utilities/UserMessages.h"
#include "Config/Config.h"
#include "CustomLabel/CustomLabel.h"
#include "CustomTextBox/CustomTextBox.h"

#include "Category.h"
#include "CategoryDialog.h"


HWND HWND_CATEGORYDIALOG = NULL;

CCategoryDialog CategoryDialog;



// ========================================================================================
// Process WM_CLOSE message for window/dialog: CategoryDialog
// ========================================================================================
void CategoryDialog_OnClose(HWND hwnd) {
    EnableWindow(GetParent(hwnd), true);
    DestroyWindow(hwnd);
}


// ========================================================================================
// Process WM_DESTROY message for window/dialog: CategoryDialog
// ========================================================================================
void CategoryDialog_OnDestroy(HWND hwnd) {
    PostQuitMessage(0);
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: CategoryDialog
// ========================================================================================
bool CategoryDialog_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct) {
    HWND_CATEGORYDIALOG = hwnd;

    DWORD light_text_color = COLOR_WHITELIGHT;
    DWORD dark_back_color = COLOR_GRAYMEDIUM;
    DWORD dark_text_color = COLOR_WHITEDARK;

    DWORD text_color = COLOR_WHITELIGHT;
    DWORD text_color_dim = COLOR_WHITEDARK;
    DWORD back_color = COLOR_GRAYDARK;

    int horiz_text_margin = 0;
    int vert_text_margin = 3;

    HWND hCtl = NULL;

    CustomLabel_SimpleLabel(hwnd, -1, L"Category", text_color_dim, back_color,
        CustomLabelAlignment::middle_left, 40, 20, 68, 22);
    CustomLabel_SimpleLabel(hwnd, -1, L"Description", text_color_dim, back_color,
        CustomLabelAlignment::middle_left, 108, 20, 115, 22);

    int top = 45;
    int ctl_id = IDC_CATEGORYCONTROL_FIRST;

    for (int i = 0; i < 8; ++i) {
        CustomLabel_SimpleLabel(hwnd, -1, std::to_wstring(i), text_color, back_color,
            CustomLabelAlignment::middle_left, 60, top, 20, 23);
        hCtl = CreateCustomTextBox(hwnd, ctl_id, false, ES_LEFT, L"", 110, top, 300, 23);
        CustomTextBox_SetMargins(hCtl, horiz_text_margin, vert_text_margin);
        CustomTextBox_SetColors(hCtl, light_text_color, dark_back_color);
        AfxSetWindowText(hCtl, config.GetCategoryDescription(i));
        top += 25;
        ctl_id++;
    }

    CustomLabel_SimpleLabel(hwnd, -1, L"Category", text_color_dim, back_color,
        CustomLabelAlignment::middle_left, 420, 20, 68, 22);
    CustomLabel_SimpleLabel(hwnd, -1, L"Description", text_color_dim, back_color,
        CustomLabelAlignment::middle_left, 498, 20, 115, 22);

    top = 45;
    for (int i = 8; i < 16; ++i) {
        CustomLabel_SimpleLabel(hwnd, -1, std::to_wstring(i), text_color, back_color,
            CustomLabelAlignment::middle_left, 450, top, 20, 23);
        hCtl = CreateCustomTextBox(hwnd, ctl_id, false, ES_LEFT, L"", 500, top, 300, 23);
        CustomTextBox_SetMargins(hCtl, horiz_text_margin, vert_text_margin);
        CustomTextBox_SetColors(hCtl, light_text_color, dark_back_color);
        AfxSetWindowText(hCtl, config.GetCategoryDescription(i));
        top += 25;
        ctl_id++;
    }

    // SAVE button
    std::wstring font_name = AfxGetDefaultFont();
    int font_size = 9;

    // SAVE button
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_CATEGORYDIALOG_SAVE, L"SAVE",
        COLOR_BLACK, COLOR_GREEN, COLOR_GREEN, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_center, 720, 390, 80, 23);
    CustomLabel_SetFont(hCtl, font_name, font_size, true);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);

    // CANCEL button
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_CATEGORYDIALOG_CANCEL, L"Cancel",
        COLOR_BLACK, COLOR_RED, COLOR_RED, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_center, 720, 423, 80, 23);
    CustomLabel_SetFont(hCtl, font_name, font_size, true);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);

    return true;
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: CategoryDialog
// ========================================================================================
bool CategoryDialog_OnEraseBkgnd(HWND hwnd, HDC hdc) {
    // Handle all of the painting in WM_PAINT
    return true;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: CategoryDialog
// ========================================================================================
void CategoryDialog_OnPaint(HWND hwnd) {
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
LRESULT CCategoryDialog::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        HANDLE_MSG(m_hwnd, WM_CREATE, CategoryDialog_OnCreate);
        HANDLE_MSG(m_hwnd, WM_DESTROY, CategoryDialog_OnDestroy);
        HANDLE_MSG(m_hwnd, WM_CLOSE, CategoryDialog_OnClose);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, CategoryDialog_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, CategoryDialog_OnPaint);

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
        return 0;
    }

    case MSG_CUSTOMLABEL_CLICK: {
        HWND hCtl = (HWND)lParam;
        int ctrl_id = (int)wParam;

        if (hCtl == NULL) return 0;

        if (ctrl_id == IDC_CATEGORYDIALOG_CANCEL) {
            dialog_return_code = DIALOG_RETURN_CANCEL;
            SendMessage(m_hwnd, WM_CLOSE, 0, 0);
        }

        if (ctrl_id == IDC_CATEGORYDIALOG_SAVE) {
            int cID = IDC_CATEGORYCONTROL_FIRST;
            for (int i = 0; i < 16; ++i) {
                config.SetCategoryDescription(i, CustomTextBox_GetText(GetDlgItem(m_hwnd, cID)));
                cID++;
            }
            config.SaveConfig();
            dialog_return_code = DIALOG_RETURN_OK;
            if (MainWindow.hLeftPanel == ActiveTrades.hWindow) {
                ActiveTrades.ShowActiveTrades();
            }
            SendMessage(m_hwnd, WM_CLOSE, 0, 0);
        }

        return 0;
    }

    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}


// ========================================================================================
// Create and show the Category modal dialog.
// ========================================================================================
int CategoryDialog_Show(HWND hWndParent) {
    int width = 840;
    int height = 500;

    HWND hwnd = CategoryDialog.Create(hWndParent, L"Category Management", 0, 0, width, height,
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_CONTROLPARENT | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);

    // Attempt to apply the standard Windows dark theme to the non-client areas of the main form.
    BOOL value = TRUE;
    ::DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));

    HBRUSH hbrBackground = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)hbrBackground);

    HANDLE hIconSmall = LoadImage(CategoryDialog.hInst(), MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON, 16, 16, LR_SHARED);
    SendMessage(hwnd, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIconSmall);

    AfxCenterWindow(hwnd, hWndParent);

    EnableWindow(hWndParent, false);

    // Fix Windows 10 white flashing
    BOOL cloak = TRUE;
    DwmSetWindowAttribute(hwnd, DWMWA_CLOAK, &cloak, sizeof(cloak));

    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);

    cloak = FALSE;
    DwmSetWindowAttribute(hwnd, DWMWA_CLOAK, &cloak, sizeof(cloak));

    SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

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

