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
#include "Utilities/AfxWin.h"
#include "Utilities/ListBoxData.h"
#include "Utilities/Colors.h"
#include "CustomLabel/CustomLabel.h"
#include "Utilities/UserMessages.h"
#include "MainWindow/MainWindow.h"
#include "Config/Config.h"

#include "CustomCombo.h"
#include "CustomComboPopup.h"


// ========================================================================================
// Get the index of the selected item in the group.
// ========================================================================================
int CustomComboControl_GetSelectedIndex(HWND hwnd) {
    return CustomLabel_GetUserDataInt(GetDlgItem(hwnd, IDC_CUSTOMCOMBOCONTROL_COMBOBOX));
}


// ========================================================================================
// Select the incoming index in the group.
// ========================================================================================
void CustomComboControl_SetSelectedIndex(HWND hwnd, int index) {
    CustomLabel_SetUserDataInt(GetDlgItem(hwnd, IDC_CUSTOMCOMBOCONTROL_COMBOBOX), index);
}


// ========================================================================================
// Create child controls for the Window.
// ========================================================================================
void CustomComboControl_OnCreate(HWND hwnd) {
    HWND hCtl = NULL;

    int left = 0;

    std::wstring font_name = AfxGetDefaultFont();
    int font_size = 8;

    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_CUSTOMCOMBOCONTROL_COMBOBOX, L"",
        COLOR_WHITEDARK, COLOR_GRAYMEDIUM, COLOR_GRAYMEDIUM, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_left, left, 0, CUSTOMCOMBOCONTROL_COMBOBOX_WIDTH, CUSTOMCOMBOCONTROL_HEIGHT);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);
    CustomLabel_SetMousePointer(hCtl, CustomLabelPointer::hand, CustomLabelPointer::hand);

    left += CUSTOMCOMBOCONTROL_COMBOBOX_WIDTH;
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_CUSTOMCOMBOCONTROL_COMMAND, GLYPH_DROPDOWN,
        COLOR_WHITEDARK, COLOR_GRAYMEDIUM, COLOR_GRAYLIGHT, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_center, left, 0, CUSTOMCOMBOCONTROL_COMMAND_WIDTH, CUSTOMCOMBOCONTROL_HEIGHT);
    CustomLabel_SetFont(hCtl, font_name, font_size, true);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);
}


// ========================================================================================
// Windows message callback for the custom SortFilter control.
// ========================================================================================
LRESULT CALLBACK CustomComboControlProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    CustomComboControl* pData = nullptr;

    if (uMsg != WM_CREATE) {
        pData = (CustomComboControl*)GetWindowLongPtr(hwnd, 0);
    }

    switch (uMsg) {

    case WM_KEYDOWN: {
        // Parent to handle the TAB navigation key to move amongst constrols.
        if (wParam == VK_TAB) {
            if (SendMessage(GetParent(hwnd), uMsg, wParam, lParam))
                return 0;
        }
    }

    case MSG_CUSTOMLABEL_CLICK: {
        HWND hCtl = (HWND)lParam;
        int ctrl_id = (int)wParam;

        if (hCtl == NULL) return 0;

        if (ctrl_id == IDC_CUSTOMCOMBOCONTROL_COMBOBOX || ctrl_id == IDC_CUSTOMCOMBOCONTROL_COMMAND) {
            // Clicked on the combobox dropdown or label itself
            HWND hParent = CustomComboPopup_CreatePopup(hwnd, GetDlgItem(hwnd, IDC_CUSTOMCOMBOCONTROL_COMBOBOX), pData->item_count);
            HWND hListBox = GetDlgItem(hParent, IDC_CUSTOMCOMBOPOPUP_LISTBOX);
            for (auto& item : pData->items) {
                int idx = ListBox_AddString(hListBox, item.item_text.c_str());
                ListBox_SetItemData(hListBox, idx, item.item_data);
            }
        }

        return 0;
    }

    case WM_ERASEBKGND: {
        return true;
    }

    case WM_PAINT: {
        if (!pData) break;
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        SaveDC(hdc);

        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP hbit = CreateCompatibleBitmap(hdc, ps.rcPaint.right, ps.rcPaint.bottom);
        SelectBitmap(memDC, hbit);

        Graphics graphics(memDC);
        int width = (ps.rcPaint.right - ps.rcPaint.left);
        int height = (ps.rcPaint.bottom - ps.rcPaint.top);

        Color back_color(pData->back_color);
        SolidBrush back_brush(back_color);
        graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, width, height);

        // Copy the entire memory bitmap to the main display
        BitBlt(hdc, 0, 0, ps.rcPaint.right, ps.rcPaint.bottom, memDC, 0, 0, SRCCOPY);

        // Restore the original state of the DC
        RestoreDC(hdc, -1);

        // Cleanup
        DeleteObject(hbit);
        DeleteDC(memDC);

        EndPaint(hwnd, &ps);

        return 0;
    }

    case WM_NCDESTROY: {
        if (pData) delete(pData);
        break;
    }

    }
   return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


// ========================================================================================
// Retrieve the stored data pointer from the control
// ========================================================================================
CustomComboControl* CustomComboControl_GetOptions(HWND hCtrl) {
    CustomComboControl* pData = (CustomComboControl*)GetWindowLongPtr(hCtrl, 0);
    return pData;
}


// ========================================================================================
// Store the data pointer into the control
// ========================================================================================
int CustomComboControl_SetOptions(HWND hCtrl, CustomComboControl* pData) {
    if (!pData) return 0;

    if (pData->tooltip_text.length()) {
        if (pData->hToolTip) {
            AfxSetTooltipText(pData->hToolTip, hCtrl, pData->tooltip_text);
        }
    }

    SetWindowLongPtr(hCtrl, 0, (LONG_PTR)pData);
    AfxRedrawWindow(hCtrl);

    return 0;
}


// ========================================================================================
// Create the SortFilter control.
// ========================================================================================
HWND CreateCustomComboControl(HWND hWndParent, int ctrl_id, int left, int top, int item_count) {
    std::wstring class_name_text(L"CUSTOMCOMBOCONTROL_CONTROL");

    WNDCLASSEX wcex{};

    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hWndParent, GWLP_HINSTANCE);

    if (GetClassInfoEx(hInst, class_name_text.c_str(), &wcex) == 0) {
        wcex.cbSize = sizeof(wcex);
        wcex.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = CustomComboControlProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = sizeof(HANDLE);    // make room to store a pointer to the class
        wcex.hInstance = hInst;
        wcex.hCursor = LoadCursor(NULL, (LPCWSTR)IDC_ARROW);
        wcex.hIcon = NULL;
        wcex.hIconSm = NULL;
        wcex.hbrBackground = (HBRUSH)HOLLOW_BRUSH;
        wcex.lpszMenuName = NULL;
        wcex.lpszClassName = class_name_text.c_str();
        if (RegisterClassEx(&wcex) == 0) return 0;
    }

    float rx = AfxScaleRatioX();
    float ry = AfxScaleRatioY();

    int width = AfxScaleX(CUSTOMCOMBOCONTROL_COMBOBOX_WIDTH + CUSTOMCOMBOCONTROL_COMMAND_WIDTH);
    int height = AfxScaleY(CUSTOMCOMBOCONTROL_HEIGHT);
    left = (int)(left * rx);
    top = (int)(top * ry);

    HWND hCtl =
        CreateWindowEx(WS_EX_CONTROLPARENT, class_name_text.c_str(), L"",
            WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
            left, top, width, height,
            hWndParent, (HMENU)(UINT_PTR)ctrl_id, hInst, (LPVOID)NULL);

    if (hCtl) {
        CustomComboControl* pData = new CustomComboControl;
        pData->hwnd = hCtl;
        pData->hParent = hWndParent;
        pData->ctrl_id = ctrl_id;
        pData->back_color = COLOR_BLACK;
        pData->item_count = item_count;

        CustomComboControl_SetOptions(hCtl, pData);
        CustomComboControl_OnCreate(hCtl);
    }

    return hCtl;
}


