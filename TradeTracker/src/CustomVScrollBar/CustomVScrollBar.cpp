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
// CustomVScrollBar CONTROL
//

#include "pch.h"

#include "Utilities/AfxWin.h"
#include "CustomVScrollBar.h"


// ========================================================================================
// Calculate the RECT that holds the client coordinates of the scrollbar's vertical thumb
// Will return TRUE if RECT is not empty. 
// ========================================================================================
bool CustomVScrollBar::calcVThumbRect()
{
    // calculate the vertical scrollbar in client coordinates
    SetRectEmpty(&rc);
    int top_index = (int)SendMessage(hListBox, LB_GETTOPINDEX, 0, 0);
    
    RECT rcListBox{};
    GetClientRect(hListBox, &rcListBox);
    listbox_height = (rcListBox.bottom - rcListBox.top);
    items_count = ListBox_GetCount(hListBox);

    int index = (items_count > 0) ? 1 : 0;
    item_height = ListBox_GetItemHeight(hListBox, index);

    // If no items exist then exit to avoid division by zero GPF's.
    if (items_count == 0) return FALSE;

    items_per_page = (int)(std::round(listbox_height / (float)item_height));
    thumb_height = (int)(((float)items_per_page / (float)items_count) * (float)listbox_height);

    rc.left = rcListBox.left;
    rc.top = (int)(rcListBox.top + (((float)top_index / (float)items_count) * (float)listbox_height));
    rc.right = rcListBox.right;
    rc.bottom = (rc.top + thumb_height);

    // If the number of items in the listbox is less than what could display
    // on the screen then there is no need to show the scrollbar.
    return (items_count < items_per_page) ? FALSE : TRUE;
}


// ========================================================================================
// Windows message callback for the custom ScrollBar control.
// ========================================================================================
LRESULT CALLBACK CustomVScrollBarProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CustomVScrollBar* pData = nullptr;

    // Create static accumulation variable to collect the data from
    // a series of middle mouse wheel scrolls.
    static int accumDelta = 0;


    if (uMsg != WM_CREATE) {
        pData = (CustomVScrollBar*)GetWindowLongPtr(hWnd, 0);
    }

    switch (uMsg)
    {

    case WM_MOUSEWHEEL:
    {
        // Accumulate delta until scroll one line (up +120, down -120). 
        // 120 is the Microsoft default delta
        int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        int top_index = (int)SendMessage(pData->hListBox, LB_GETTOPINDEX, 0, 0);
        accumDelta += zDelta;
        if (accumDelta >= 120) {     // scroll up 3 lines
            top_index -= 3;
            top_index = max(0, top_index);
            SendMessage(pData->hListBox, LB_SETTOPINDEX, top_index, 0);
            accumDelta = 0;
        }
        else {
            if (accumDelta <= -120) {     // scroll down 3 lines
                top_index += +3;
                SendMessage(pData->hListBox, LB_SETTOPINDEX, top_index, 0);
                accumDelta = 0;
            }
        }
        CustomVScrollBar_Recalculate(hWnd);
        return 0;
        break;
    }


    case WM_LBUTTONDOWN:
        {
            if (pData == nullptr) break;
            POINT pt;
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            pData->calcVThumbRect();

            if (PtInRect(&pData->rc, pt)) {
                pData->prev_pt = pt;
                pData->drag_active = true;
                SetCapture(hWnd);
            }
            else {
                // we have clicked on a PageUp or PageDn
                int top_index = (int)SendMessage(pData->hListBox, LB_GETTOPINDEX, 0, 0);
                if (pt.y < pData->rc.top) {
                    top_index = max(top_index - pData->items_per_page, 0);
                    SendMessage(pData->hListBox, LB_SETTOPINDEX, top_index, 0);
                    pData->calcVThumbRect();
                    AfxRedrawWindow(pData->hwnd);
                }
                else {
                    if (pt.y > pData->rc.bottom) {
                        int nMaxTopIndex = pData->items_count - pData->items_per_page;
                        top_index = min(top_index + pData->items_per_page, nMaxTopIndex);
                        SendMessage(pData->hListBox, LB_SETTOPINDEX, top_index, 0);
                        pData->calcVThumbRect();
                        AfxRedrawWindow(pData->hwnd);
                    }
                }

            }
            return 0;
            break;
        }


        case WM_MOUSEMOVE:
        {
            if (pData == nullptr) break;
            if (pData->drag_active) {
                POINT pt;
                pt.x = GET_X_LPARAM(lParam);
                pt.y = GET_Y_LPARAM(lParam);
                if (pt.y != pData->prev_pt.y) {
                    int delta = (pt.y - pData->prev_pt.y);

                    RECT rc; GetClientRect(hWnd, &rc);
                    pData->rc.top = max(0, pData->rc.top + delta);
                    pData->rc.top = min(pData->rc.top, rc.bottom - pData->thumb_height);
                    pData->rc.bottom = pData->rc.top + pData->thumb_height;

                    pData->prev_pt = pt;

                    int previous_topline = (int)SendMessage(pData->hListBox, LB_GETTOPINDEX, 0, 0);
                    int top_line = (int)std::round(pData->rc.top / (float)rc.bottom * pData->items_count);
                    if (top_line != previous_topline)
                        SendMessage(pData->hListBox, LB_SETTOPINDEX, (WPARAM)top_line, 0);

                    AfxRedrawWindow(hWnd);
                }
            }
            return 0;
            break;
        }


        case WM_LBUTTONUP:
        {
            if (pData == nullptr) break;
            pData->drag_active = false;
            pData->prev_pt.x = 0;
            pData->prev_pt.y = 0;
            ReleaseCapture();
            return 0;
            break;
        }

        case WM_ERASEBKGND:
            return TRUE;
            break;


        case WM_PAINT:
        {
            if (pData == nullptr) break;
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            SaveDC(hdc);

            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP hbit = CreateCompatibleBitmap(hdc, ps.rcPaint.right, ps.rcPaint.bottom);
            SelectBitmap(memDC, hbit);

            Graphics graphics(memDC);
            int nWidth = (ps.rcPaint.right - ps.rcPaint.left);
            int nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);

            SolidBrush back_brush(pData->scrollbar_back_color);
            graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

            if (pData->items_count > pData->items_per_page) {
                back_brush.SetColor(pData->scrollbar_thumb_color);
                graphics.FillRectangle(&back_brush, pData->rc.left, pData->rc.top, nWidth, pData->thumb_height);

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

            EndPaint(hWnd, &ps);

            return 0;
            break;
        }


    case WM_NCDESTROY:
        if (pData) delete(pData);
        break;


    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}


// ========================================================================================
// Retrieve the stored data pointer from the custom ScrollBar
// ========================================================================================
CustomVScrollBar* CustomVScrollBar_GetPointer(HWND hCtrl)
{
    CustomVScrollBar* pData = (CustomVScrollBar*)GetWindowLongPtr(hCtrl, 0);
    return pData;
}


// ========================================================================================
// Recalculate the ScrollBar thumb size and refresh display.
// ========================================================================================
void CustomVScrollBar_Recalculate(HWND hCtrl)
{
    CustomVScrollBar* pData = CustomVScrollBar_GetPointer(hCtrl);
    if (pData != nullptr) {
        pData->calcVThumbRect();
        AfxRedrawWindow(pData->hwnd);
    }
}


// ========================================================================================
// Create the vertical custom control.
// ========================================================================================
HWND CreateCustomVScrollBar(
    HWND hWndParent,
    LONG_PTR CtrlId,
    HWND hListBox
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
            hWndParent, (HMENU)CtrlId, hInst, (LPVOID)NULL);

    if (hCtl) {
        CustomVScrollBar* pData = new CustomVScrollBar;

        pData->hwnd = hCtl;
        pData->hParent = hWndParent;
        pData->hListBox = hListBox;
        pData->CtrlId = (int)CtrlId;
        pData->calcVThumbRect();

        SetWindowLongPtr(hCtl, 0, (LONG_PTR)pData);
    }

    return hCtl;
}

