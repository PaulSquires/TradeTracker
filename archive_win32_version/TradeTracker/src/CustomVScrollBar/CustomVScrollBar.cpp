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
// CustomVScrollBar CONTROL
//

#include "pch.h"

#include "Utilities/AfxWin.h"
#include "CustomVScrollBar.h"


// ========================================================================================
// Calculate the RECT that holds the client coordinates of the scrollbar's vertical thumb
// Will return true if RECT is not empty. 
// ========================================================================================
bool CustomVScrollBar::calcVThumbRect() {
    if (ChildControlType == Controls::ListBox) {
        // calculate the vertical scrollbar in client coordinates
        SetRectEmpty(&rc);
        int top_index = (int)SendMessage(hChildCtl, LB_GETTOPINDEX, 0, 0);
    
        RECT rcListBox{};
        GetClientRect(hChildCtl, &rcListBox);
        child_control_height = (rcListBox.bottom - rcListBox.top);
        items_count = ListBox_GetCount(hChildCtl);

        int index = (items_count > 0) ? 1 : 0;
        item_height = ListBox_GetItemHeight(hChildCtl, index);

        // If no items exist then exit to avoid division by zero GPF's.
        if (items_count == 0) return false;

        items_per_page = (int)(std::round(child_control_height / (float)item_height));
        thumb_height = (int)(((float)items_per_page / (float)items_count) * (float)child_control_height);

        rc.left = rcListBox.left;
        rc.top = (int)(rcListBox.top + (((float)top_index / (float)items_count) * (float)child_control_height));
        rc.right = rcListBox.right;
        rc.bottom = (rc.top + thumb_height);

        // If the number of items in the listbox is less than what could display
        // on the screen then there is no need to show the scrollbar.
        return (items_count < items_per_page) ? false : true;
    }

    if (ChildControlType == Controls::MultilineTextBox) {
        SetRectEmpty(&rc);
        int top_index = (int)SendMessage(hChildCtl, EM_GETFIRSTVISIBLELINE, 0, 0);
        
        // Get the line height
        HFONT hFont = (HFONT)SendMessage(hChildCtl, WM_GETFONT, 0, 0);
        HDC hdc = GetDC(hChildCtl);
        auto const hOldFont = SelectFont(hdc, hFont);
        TEXTMETRIC tm;
        GetTextMetrics(hdc, &tm);
        item_height = tm.tmHeight;
        if (hOldFont) SelectFont(hdc, hOldFont);
        ReleaseDC(hChildCtl, hdc);

        RECT rcTextBox{};
        GetClientRect(hChildCtl, &rcTextBox);
        child_control_height = (rcTextBox.bottom - rcTextBox.top);
        items_count = Edit_GetLineCount(hChildCtl);  // value will never be less than 1.
        
        items_per_page = (int)(std::round(child_control_height / (float)item_height));
        thumb_height = (int)(((float)items_per_page / (float)items_count) * (float)child_control_height);

        rc.left = rcTextBox.left;
        rc.top = (int)(rcTextBox.top + (((float)top_index / (float)items_count) * (float)child_control_height));
        rc.right = rcTextBox.right;
        rc.bottom = (rc.top + thumb_height);

        // If the number of items in the textbox is less than what could display
        // on the screen then there is no need to show the scrollbar.
        return (items_count < items_per_page) ? false : true;
    }

    return false;
}


// ========================================================================================
// Scroll the correct number of lines up or down depending on the control type.
// ========================================================================================
void CustomVScrollBar_ScrollLines(HWND hScrollCtl, int num_lines) {
    CustomVScrollBar* pData = (CustomVScrollBar*)GetWindowLongPtr(hScrollCtl, 0);
    if (!pData) return;

    HWND hChildCtl = pData->hChildCtl;

    if (pData->ChildControlType == Controls::ListBox) {
        int top_index = (int)SendMessage(hChildCtl, LB_GETTOPINDEX, 0, 0);
        top_index = max(0, top_index + num_lines);
        SendMessage(hChildCtl, LB_SETTOPINDEX, top_index, 0);
    }

    if (pData->ChildControlType == Controls::MultilineTextBox) {
        SendMessage(hChildCtl, EM_LINESCROLL, 0, (LPARAM)num_lines);
    }
}


// ========================================================================================
// Windows message callback for the custom ScrollBar control.
// ========================================================================================
LRESULT CALLBACK CustomVScrollBarProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    CustomVScrollBar* pData = nullptr;

    // Create static accumulation variable to collect the data from
    // a series of middle mouse wheel scrolls.
    static int accum_delta = 0;

    if (uMsg != WM_CREATE) {
        pData = (CustomVScrollBar*)GetWindowLongPtr(hwnd, 0);
    }

    switch (uMsg) {

    case WM_MOUSEWHEEL: {
        // Accumulate delta until scroll one line (up +120, down -120). 
        // 120 is the Microsoft default delta
        int zdelta = GET_WHEEL_DELTA_WPARAM(wParam);
        accum_delta += zdelta;
        if (accum_delta >= 120) {     // scroll up 3 lines
            CustomVScrollBar_ScrollLines(hwnd, -3);
            accum_delta = 0;
        }
        else {
            if (accum_delta <= -120) {     // scroll down 3 lines
                CustomVScrollBar_ScrollLines(hwnd, 3);
                accum_delta = 0;
            }
        }
        CustomVScrollBar_Recalculate(hwnd);
        return 0;
    }

    case WM_LBUTTONDOWN: {
        if (!pData) break;
        POINT pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        pData->calcVThumbRect();

        if (PtInRect(&pData->rc, pt)) {
            pData->prev_pt = pt;
            pData->drag_active = true;
            SetCapture(hwnd);
        }
        else {
            // we have clicked on a PageUp or PageDn
            if (pt.y < pData->rc.top) {
                CustomVScrollBar_ScrollLines(hwnd, -pData->items_per_page);
                pData->calcVThumbRect();
                AfxRedrawWindow(pData->hwnd);
            }
            else {
                if (pt.y > pData->rc.bottom) {
                    CustomVScrollBar_ScrollLines(hwnd, pData->items_per_page);
                    pData->calcVThumbRect();
                    AfxRedrawWindow(pData->hwnd);
                }
            }

        }
        return 0;
    }

    case WM_MOUSEMOVE: {
        if (!pData) break;
        if (pData->drag_active) {
            POINT pt;
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            if (pt.y != pData->prev_pt.y) {
                int delta = (pt.y - pData->prev_pt.y);

                RECT rc; GetClientRect(hwnd, &rc);
                pData->rc.top = max(0, pData->rc.top + delta);
                pData->rc.top = min(pData->rc.top, rc.bottom - pData->thumb_height);
                pData->rc.bottom = pData->rc.top + pData->thumb_height;

                pData->prev_pt = pt;

                if (pData->ChildControlType == Controls::ListBox) {
                    int previous_top_index = (int)SendMessage(pData->hChildCtl, LB_GETTOPINDEX, 0, 0);
                    int top_index = (int)std::round(pData->rc.top / (float)rc.bottom * pData->items_count);
                    if (top_index != previous_top_index) {
                        SendMessage(pData->hChildCtl, LB_SETTOPINDEX, top_index, 0);
                    }
                }
                if (pData->ChildControlType == Controls::MultilineTextBox) {
                    int previous_top_index = (int)SendMessage(pData->hChildCtl, EM_GETFIRSTVISIBLELINE, 0, 0);
                    int top_index = (int)std::round(pData->rc.top / (float)rc.bottom * pData->items_count);
                    if (top_index != previous_top_index) {
                        SendMessage(pData->hChildCtl, EM_LINESCROLL, 0, (LPARAM)(top_index - previous_top_index));
                    }
                }

                AfxRedrawWindow(hwnd);
            }
        }
        return 0;
    }

    case WM_LBUTTONUP:
    {
        if (!pData) break;
        pData->drag_active = false;
        pData->prev_pt.x = 0;
        pData->prev_pt.y = 0;
        ReleaseCapture();
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

        SolidBrush back_brush(pData->scrollbar_back_color);
        graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, width, height);

        if (pData->items_count > pData->items_per_page) {
            back_brush.SetColor(pData->scrollbar_thumb_color);
            graphics.FillRectangle(&back_brush, pData->rc.left, pData->rc.top, width, pData->thumb_height);

            Pen pen(pData->scrollbar_line_color, 1);
            graphics.DrawLine(&pen, (INT)ps.rcPaint.left, (INT)ps.rcPaint.top, (INT)ps.rcPaint.left, (INT)ps.rcPaint.bottom);
        }

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
// Retrieve the stored data pointer from the custom ScrollBar
// ========================================================================================
CustomVScrollBar* CustomVScrollBar_GetPointer(HWND hCtrl) {
    CustomVScrollBar* pData = (CustomVScrollBar*)GetWindowLongPtr(hCtrl, 0);
    return pData;
}


// ========================================================================================
// Recalculate the ScrollBar thumb size and refresh display.
// ========================================================================================
void CustomVScrollBar_Recalculate(HWND hCtrl) {
    CustomVScrollBar* pData = CustomVScrollBar_GetPointer(hCtrl);
    if (pData) {
        pData->calcVThumbRect();
        AfxRedrawWindow(pData->hwnd);
    }
}


// ========================================================================================
// Create the vertical custom control.
// ========================================================================================
HWND CreateCustomVScrollBar(
    HWND hWndParent,
    LONG_PTR ctrl_id,
    HWND hWndChild,
    Controls ChildControlType
    )
{
    std::wstring class_name_text(L"CUSTOMVSCROLLBAR_CONTROL");

    WNDCLASSEX wcex{};

    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hWndParent, GWLP_HINSTANCE);

    if (GetClassInfoEx(hInst, class_name_text.c_str(), &wcex) == 0) {
        wcex.cbSize = sizeof(wcex);
        wcex.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = CustomVScrollBarProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = sizeof(HANDLE);    // make room to store a pointer to the class
        wcex.hInstance = hInst;
        wcex.hCursor = LoadCursor(NULL, (LPCWSTR)IDC_ARROW);
        wcex.hIcon = NULL;
        wcex.hIconSm = NULL;
        wcex.hbrBackground = (HBRUSH)WHITE_BRUSH;
        wcex.lpszMenuName = NULL;
        wcex.lpszClassName = class_name_text.c_str();
        if (RegisterClassEx(&wcex) == 0) return 0;
    }

    HWND hCtl =
        CreateWindowEx(0, class_name_text.c_str(), L"",
            WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
            0, 0, 0, 0,
            hWndParent, (HMENU)ctrl_id, hInst, (LPVOID)NULL);

    if (hCtl) {
        CustomVScrollBar* pData = new CustomVScrollBar;

        pData->hwnd = hCtl;
        pData->hParent = hWndParent;
        pData->hChildCtl = hWndChild;
        if (ChildControlType == Controls::MultilineTextBox) {
            pData->hChildCtl = GetDlgItem(hWndChild, 100);
        }
        pData->ctrl_id = (int)ctrl_id;
        pData->ChildControlType = ChildControlType;

        pData->calcVThumbRect();

        SetWindowLongPtr(hCtl, 0, (LONG_PTR)pData);
    }

    return hCtl;
}

