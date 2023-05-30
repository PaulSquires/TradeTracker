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

#include "..\CustomLabel\CustomLabel.h"
#include "TransDateFilter.h"


HWND HWND_TRANSDATEFILTER = NULL;

CTransDateFilter TransDateFilter;

std::wstring wszSelectedDateFilter;

// Control on parent window that new selected date will be stored in and displayed.
// That control must be a CustomLabel because we store the full ISO date in that
// control's UserData string.
HWND hDateUpdateParentCtl = NULL;
TransDateFilterReturnType TheUpdateDateReturnType = TransDateFilterReturnType::ISODate;



// ========================================================================================
// Process WM_SIZE message for window/dialog: TransDateFilter
// ========================================================================================
void TransDateFilter_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    SetWindowPos(GetDlgItem(hwnd, IDC_TRANSDATEFILTER_LISTBOX), 0, 0, 0, cx, cy, SWP_NOZORDER | SWP_SHOWWINDOW);
    return;
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: TransDateFilter
// ========================================================================================
BOOL TransDateFilter_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    // Handle all of the painting in WM_PAINT
    return TRUE;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: TransDateFilter
// ========================================================================================
void TransDateFilter_OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    Graphics graphics(hdc);

    //DWORD nBackColor = GetThemeColor(ThemeElement::GrayDark);
    DWORD nBackColor = GetThemeColor(ThemeElement::Black);

    // Create the background brush
    SolidBrush backBrush(nBackColor);

    // Paint the background using brush.
    int nWidth = (ps.rcPaint.right - ps.rcPaint.left);
    int nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&backBrush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

    EndPaint(hwnd, &ps);
}



// ========================================================================================
// Process WM_CREATE message for window/dialog: TransDateFilter
// ========================================================================================
BOOL TransDateFilter_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    HWND_TRANSDATEFILTER = hwnd;

    HWND hCtl =
        TransDateFilter.AddControl(Controls::ListBox, hwnd, IDC_TRANSDATEFILTER_LISTBOX, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWFIXED | LBS_NOTIFY,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR);
    ListBox_AddString(hCtl, NULL);

    return TRUE;
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CTransDateFilter::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Prevent a recursive calling of the WM_NCACTIVATE message as DestroyWindow deactivates the window.
    static bool destroyed = false;

    switch (msg)
    {
        HANDLE_MSG(m_hwnd, WM_CREATE, TransDateFilter_OnCreate);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, TransDateFilter_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, TransDateFilter_OnPaint);
        HANDLE_MSG(m_hwnd, WM_SIZE, TransDateFilter_OnSize);


    case WM_DESTROY:
    {
        // Reset our destroyed variable for future use of the DatePicker
        destroyed = false;
        return 0;
    }
    break;


    case WM_NCACTIVATE:
    {
        // Detect that we have clicked outside the popup TransDateFilter picker and will now close it.
        if (wParam == false) {
            // Set our static flag to prevent recursion
            if (destroyed == false) {
                destroyed = true;
                DestroyWindow(m_hwnd);
            }
            return TRUE;
        }
        return 0;
    }
    break;


    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}


// ========================================================================================
// Create TransDateFilter picker control and move it into position under the specified incoming control.
// ========================================================================================
HWND TransDateFilter_CreatePicker(
    HWND hParent, HWND hParentCtl, std::wstring wszDate, TransDateFilterReturnType DateReturnType)
{
    if (wszDate.length() == 0)
        wszDate = AfxCurrentDate();

    wszSelectedDateFilter = wszDate;


    TransDateFilter.Create(hParent, L"", 0, 0, 0, 0,
        WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);


    RECT rc; GetWindowRect(hParentCtl, &rc);
    SetWindowPos(TransDateFilter.WindowHandle(), HWND_TOP,
        rc.left, rc.bottom,
        AfxScaleX(TRANSDATEFILTER_WIDTH),
        AfxScaleY(TRANSDATEFILTER_HEIGHT),
        SWP_SHOWWINDOW);


    // Set the module global hUpdateParentCtl after the above is created in
    // to ensure the variable address is correct.
    hDateUpdateParentCtl = hParentCtl;
    TheUpdateDateReturnType = DateReturnType;

    return TransDateFilter.WindowHandle();
}

