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
#include "MainWindow/tws-client.h"
#include "CustomLabel/CustomLabel.h"
#include "CustomTextBox/CustomTextBox.h"
#include "CustomMessageBox/CustomMessageBox.h"
#include "MainWindow/MainWindow.h"

#include "Assignment.h"


CAssignment Assignment;

extern int dialog_return_code;
extern std::wstring quantity_set_from_assignment_modal;

std::wstring max_quantity_allowed_text;


// ========================================================================================
// Get the HWND's of the the controls on the form.
// ========================================================================================
inline HWND CAssignment::QuantityTextBox() {
    return GetDlgItem(hWindow, IDC_ASSIGNMENT_TXTQUANTITY);
}
inline HWND CAssignment::Text1Label() {
    return GetDlgItem(hWindow, IDC_ASSIGNMENT_LBLTEXT1);
}
inline HWND CAssignment::Text2Label() {
    return GetDlgItem(hWindow, IDC_ASSIGNMENT_LBLTEXT2);
}
inline HWND CAssignment::OkayButton() {
    return GetDlgItem(hWindow, IDC_ASSIGNMENT_OK);
}
inline HWND CAssignment::CancelButton() {
    return GetDlgItem(hWindow, IDC_ASSIGNMENT_CANCEL);
}


// ========================================================================================
// Process WM_CLOSE message for window/dialog: Assignment
// ========================================================================================
void CAssignment::OnClose(HWND hwnd) {

    if (dialog_return_code == DIALOG_RETURN_OK) {
        std::wstring quantity_text = CustomTextBox_GetText(QuantityTextBox());
        int quantity_assigned = AfxValInteger(quantity_text);
        int max_quantity_allowed = AfxValInteger(max_quantity_allowed_text);

        if (quantity_text.length() == 0 || quantity_assigned <= 0 || quantity_assigned > max_quantity_allowed) {
            CustomMessageBox.Show(hwnd, L"Invalid quantity amount.", L"Error", MB_ICONWARNING | MB_OK);
            CustomTextBox_SetText(QuantityTextBox(), max_quantity_allowed_text);
            SetFocus(QuantityTextBox());
            dialog_return_code = DIALOG_RETURN_CANCEL;
            return;
        }

        quantity_set_from_assignment_modal = quantity_text;
    }
    else {
        dialog_return_code = DIALOG_RETURN_CANCEL;
    }

    MainWindow.BlurPanels(false);
    EnableWindow(MainWindow.hWindow, true);
    DestroyWindow(hwnd);
}


// ========================================================================================
// Process WM_DESTROY message for window/dialog: Assignment
// ========================================================================================
void CAssignment::OnDestroy(HWND hwnd) {
    PostQuitMessage(0);
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: Assignment
// ========================================================================================
bool CAssignment::OnEraseBkgnd(HWND hwnd, HDC hdc) {
    // Handle all of the painting in WM_PAINT
    return true;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: Assignment
// ========================================================================================
void CAssignment::OnPaint(HWND hwnd) {
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
// Process WM_CREATE message for window/dialog: Assignment
// ========================================================================================
bool CAssignment::OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct) {
    hWindow = hwnd;
    return true;
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CAssignment::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        HANDLE_MSG(m_hwnd, WM_CREATE, OnCreate);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, OnPaint);
        HANDLE_MSG(m_hwnd, WM_DESTROY, OnDestroy);
        HANDLE_MSG(m_hwnd, WM_CLOSE, OnClose);
    
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

        if (ctrl_id == IDC_ASSIGNMENT_OK) {
            dialog_return_code = DIALOG_RETURN_OK;
            SendMessage(m_hwnd, WM_CLOSE, 0, 0);
        }

        if (ctrl_id == IDC_ASSIGNMENT_CANCEL) {
            dialog_return_code = DIALOG_RETURN_CANCEL;
            SendMessage(m_hwnd, WM_CLOSE, 0, 0);
        }

        return 0;
    }

    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}


// ========================================================================================
// Create and show the Assignment modal dialog.
// ========================================================================================
int CAssignment::ShowModal(const std::wstring& long_short_text, 
    const std::wstring& quantity_assigned_text, const std::wstring& strike_price_text)
{
    int width = 350;
    int height = 200;

    HWND hwnd = Assignment.Create(MainWindow.hWindow, L"Assignment", 0, 0, width, height,
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_CONTROLPARENT | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);

    // Attempt to apply the standard Windows dark theme to the non-client areas of the main form.
    BOOL value = TRUE;
    ::DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));

    HBRUSH hbrBackground = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)hbrBackground);

    HANDLE hIconSmall = LoadImage(Assignment.hInst(), MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON, 16, 16, LR_SHARED);
    SendMessage(hwnd, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIconSmall);

    // Blur the underlying MainWindow panels in order to reduce "visual noise" while
    // our Trade Management popup is active. The MainWindow panels are shown again
    // during our call to TradeDialog_OnClose() prior to enabling the MainWindow
    // and this popup closing.
    MainWindow.BlurPanels(true);

    AfxCenterWindow(hwnd, MainWindow.hWindow);

    EnableWindow(MainWindow.hWindow, false);

    // Fix Windows 10 white flashing
    BOOL cloak = TRUE;
    DwmSetWindowAttribute(hwnd, DWMWA_CLOAK, &cloak, sizeof(cloak));

    max_quantity_allowed_text = quantity_assigned_text;

    std::wstring font_name = AfxGetDefaultFont();
    int font_size = 9;
    int horiz_text_margin = 0;
    int vert_text_margin = 3;

    HWND hCtl = NULL;

    int left = 30;
    int top = 20;
    width = 300;
    height = 23;

    CustomLabel_SimpleLabel(hwnd, -1, L"Continue with OPTION ASSIGNMENT?", COLOR_WHITELIGHT, COLOR_GRAYDARK,
        CustomLabelAlignment::middle_left, left, top, width, height);

    top += height;
    width = (long_short_text.length()) ? 50 : 0;
    CustomLabel_SimpleLabel(hwnd, IDC_ASSIGNMENT_LBLTEXT1, long_short_text, COLOR_WHITELIGHT, COLOR_GRAYDARK,
        CustomLabelAlignment::middle_left, left, top, width, height);

    left += width;
    width = 50;
    hCtl = CreateCustomTextBox(hwnd, IDC_ASSIGNMENT_TXTQUANTITY, false, ES_RIGHT, quantity_assigned_text, left, top, width, height);
    CustomTextBox_SetMargins(hCtl, horiz_text_margin, vert_text_margin);
    CustomTextBox_SetColors(hCtl, COLOR_WHITELIGHT, COLOR_GRAYMEDIUM);
    CustomTextBox_SetNumericAttributes(hCtl, 0, CustomTextBoxNegative::disallow, CustomTextBoxFormatting::allow);

    left += width;
    width = 300;
    CustomLabel_SimpleLabel(hwnd, IDC_ASSIGNMENT_LBLTEXT2, strike_price_text, COLOR_WHITELIGHT, COLOR_GRAYDARK,
        CustomLabelAlignment::middle_left, left, top, width, height);

    // SAVE button
    top += height * 2;
    left = 240;
    width = 80;
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_ASSIGNMENT_OK, L"OK",
        COLOR_BLACK, COLOR_GREEN, COLOR_GREEN, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_center, left, top, width, height);
    CustomLabel_SetFont(hCtl, font_name, font_size, true);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);

    // CANCEL button
    top += height + 10;
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_ASSIGNMENT_CANCEL, L"Cancel",
        COLOR_BLACK, COLOR_RED, COLOR_RED, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_center, left, top, width, height);
    CustomLabel_SetFont(hCtl, font_name, font_size, true);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);

    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);

    cloak = FALSE;
    DwmSetWindowAttribute(hwnd, DWMWA_CLOAK, &cloak, sizeof(cloak));

    SetFocus(GetDlgItem(hwnd, IDC_ASSIGNMENT_TXTQUANTITY));

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

