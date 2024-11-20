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
#include "Utilities/ListBoxData.h"
#include "CustomLabel/CustomLabel.h"
#include "CustomPopupMenu/CustomPopupMenu.h"
#include "Utilities/UserMessages.h"
#include "ClosedTrades/ClosedTrades.h"
#include "Transactions/TransPanel.h"

#include "MainWindow/MainWindow.h"
#include "Config/Config.h"

#include "Category.h"
#include "CategoryDialog.h"



// ========================================================================================
// Get the index of the selected Category in the group.
// ========================================================================================
int CategoryControl_GetSelectedIndex(HWND hwnd) {
    HWND hComboBox = GetDlgItem(hwnd, IDC_CATEGORYCONTROL_COMBOBOX);
    return CustomLabel_GetUserDataInt(hComboBox);
}


// ========================================================================================
// Select the incoming index in the Category group.
// ========================================================================================
void CategoryControl_SetSelectedIndex(HWND hwnd, int index) {
    HWND hComboBox = GetDlgItem(hwnd, IDC_CATEGORYCONTROL_COMBOBOX);
    CustomLabel_SetUserDataInt(hComboBox, index);
    CustomLabel_SetText(hComboBox, config.GetCategoryDescription(index));
}


// ========================================================================================
// Create child controls for the Window.
// ========================================================================================
void CategoryControl_OnCreate(HWND hwnd) {
    HWND hCtl = NULL;

    int left = 0;

    std::wstring font_name = AfxGetDefaultFont();
    int font_size = 8;

    int category = CategoryControl_Getallow_all_categories(hwnd) ? CATEGORY_ALL : CATEGORY_START;
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_CATEGORYCONTROL_COMBOBOX, config.GetCategoryDescription(category),
        COLOR_WHITEDARK, COLOR_GRAYMEDIUM, COLOR_GRAYMEDIUM, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_left, left, 0, CATEGORYCONTROL_COMBOBOX_WIDTH, CATEGORYCONTROL_HEIGHT);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);
    CustomLabel_SetMousePointer(hCtl, CustomLabelPointer::hand, CustomLabelPointer::hand);
    CustomLabel_SetUserDataInt(hCtl, category);

    left += CATEGORYCONTROL_COMBOBOX_WIDTH;
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_CATEGORYCONTROL_COMMAND, GLYPH_DROPDOWN,
        COLOR_WHITEDARK, COLOR_GRAYMEDIUM, COLOR_GRAYLIGHT, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_center, left, 0, CATEGORYCONTROL_COMMAND_WIDTH, CATEGORYCONTROL_HEIGHT);
    CustomLabel_SetFont(hCtl, font_name, font_size, true);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);

    left += CATEGORYCONTROL_COMMAND_WIDTH + CATEGORYCONTROL_HMARGIN;

    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_CATEGORYCONTROL_SETUP, GLYPH_SETUP,
        COLOR_WHITEDARK, COLOR_GRAYMEDIUM, COLOR_GRAYLIGHT, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_center, left, 0, CATEGORYCONTROL_SETUP_WIDTH, CATEGORYCONTROL_HEIGHT);
    CustomLabel_SetToolTip(hCtl, L"Configure Categories");
}


// ========================================================================================
// Windows message callback for the custom Category control.
// ========================================================================================
LRESULT CALLBACK CategoryControl_CategoryControlProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    CategoryControl* pData = nullptr;

    if (uMsg != WM_CREATE) {
        pData = (CategoryControl*)GetWindowLongPtr(hwnd, 0);
    }

    switch (uMsg) {

    case WM_KEYDOWN: {
        // Parent to handle the TAB navigation key to move amongst constrols.
        if (wParam == VK_TAB) {
            if (SendMessage(GetParent(hwnd), uMsg, wParam, lParam))
                return 0;
        }
        break;
    }

    case MSG_CUSTOMLABEL_CLICK: {
        HWND hCtl = (HWND)lParam;
        int ctrl_id = (int)wParam;

        if (!hCtl) return 0;

        if (ctrl_id == IDC_CATEGORYCONTROL_COMBOBOX || ctrl_id == IDC_CATEGORYCONTROL_COMMAND) {
            // Clicked on the combobox dropdown or label itself
            std::vector<CCustomPopupMenuItem> items;

            for (int i = CATEGORY_START; i <= CATEGORY_END; ++i) {
                items.push_back({ config.GetCategoryDescription(i), i, false });
            }

            if (CategoryControl_Getallow_all_categories(hwnd)) {
                items.insert(items.begin(), { config.GetCategoryDescription(CATEGORY_OTHER), CATEGORY_OTHER, false });
                items.insert(items.begin(), { config.GetCategoryDescription(CATEGORY_ALL), CATEGORY_ALL, false });
            }

            // Position the popup menu immediately under the control that was clicked on
            HWND hComboBox = GetDlgItem(hwnd, IDC_CATEGORYCONTROL_COMBOBOX);
            int top_offset = AfxScaleY(1);
            RECT rc{}; GetWindowRect(hComboBox, &rc);
            POINT pt{ rc.left, rc.bottom + top_offset };
            int initial_selected_category = CategoryControl_GetSelectedIndex(hwnd);
            int selected = CustomPopupMenu.Show(hComboBox, items, initial_selected_category, pt.x, pt.y);

            if (selected != -1 &&  selected != initial_selected_category) {
                CategoryControl_SetSelectedIndex(hwnd, selected);
                SendMessage(GetParent(hwnd), MSG_CATEGORY_CATEGORYCHANGED, 0, 0);
            }
        }

        if (ctrl_id == IDC_CATEGORYCONTROL_SETUP) {
            HWND hWndParent = GetParent(hwnd);
            if (hWndParent == ClosedTrades.FilterPanel.hWindow) hWndParent = MainWindow.hWindow;
            if (hWndParent == TransPanel.FilterPanel.hWindow) hWndParent = MainWindow.hWindow;
            if (CategoryDialog_Show(hWndParent) == DIALOG_RETURN_OK) {
                int selected_category = CategoryControl_GetSelectedIndex(hwnd);   // Update the label in case text has changed
                CategoryControl_SetSelectedIndex(hwnd, selected_category);
                SendMessage(GetParent(hwnd), MSG_CATEGORY_CATEGORYCHANGED, 0, 0);
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
CategoryControl* CategoryControl_GetOptions(HWND hCtrl) {
    CategoryControl* pData = (CategoryControl*)GetWindowLongPtr(hCtrl, 0);
    return pData;
}


// ========================================================================================
// Store the data pointer into the control
// ========================================================================================
int CategoryControl_SetOptions(HWND hCtrl, CategoryControl* pData) {
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
// Return if control will allow the "All Categories" menu option.
// ========================================================================================
bool CategoryControl_Getallow_all_categories(HWND hwnd) {
    CategoryControl* pData = CategoryControl_GetOptions(hwnd);
    if (pData) {
        return pData->allow_all_categories;
    }
    return false;
}


// ========================================================================================
// Create the Category control.
// ========================================================================================
HWND CreateCategoryControl(HWND hWndParent,
    int ctrl_id, int left, int top, int selected_index, bool allow_all_categories)
{
    std::wstring class_name_text(L"CATEGORYCONTROL_CONTROL");

    WNDCLASSEX wcex{};

    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hWndParent, GWLP_HINSTANCE);

    if (GetClassInfoEx(hInst, class_name_text.c_str(), &wcex) == 0) {
        wcex.cbSize = sizeof(wcex);
        wcex.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = CategoryControl_CategoryControlProc;
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

    int width = CATEGORYCONTROL_COMBOBOX_WIDTH + CATEGORYCONTROL_COMMAND_WIDTH + 
                    CATEGORYCONTROL_HMARGIN + CATEGORYCONTROL_SETUP_WIDTH;
    int height = CATEGORYCONTROL_HEIGHT;

    HWND hCtl =
        CreateWindowEx(WS_EX_CONTROLPARENT, class_name_text.c_str(), L"",
            WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
            (int)(left * rx), (int)(top * ry), 
            (int)(width * rx), (int)(height * ry),   
            hWndParent, (HMENU)(UINT_PTR)ctrl_id, hInst, (LPVOID)NULL);

    if (hCtl) {
        CategoryControl* pData = new CategoryControl;
        pData->hwnd = hCtl;
        pData->hParent = hWndParent;
        pData->ctrl_id = ctrl_id;
        pData->back_color = (hWndParent == ClosedTrades.hWindow) ? COLOR_BLACK :COLOR_GRAYDARK;
        pData->allow_all_categories = allow_all_categories;

        CategoryControl_SetOptions(hCtl, pData);
        CategoryControl_OnCreate(hCtl);
        CategoryControl_SetSelectedIndex(hCtl, selected_index);
    }

    return hCtl;
}

