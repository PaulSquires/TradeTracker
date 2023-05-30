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
// Listbox subclass Window procedure
// ========================================================================================
LRESULT CALLBACK TransDateFilter_ListBox_SubclassProc(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    switch (uMsg)
    {

    case WM_ERASEBKGND:
    {
        // If the number of lines in the listbox maybe less than the number per page then 
        // calculate from last item to bottom of listbox, otherwise calculate based on
        // the mod of the lineheight to listbox height so we can color the partial line
        // that won't be displayed at the bottom of the list.
        RECT rc; GetClientRect(hWnd, &rc);

        RECT rcItem{};
        SendMessage(hWnd, LB_GETITEMRECT, 0, (LPARAM)&rcItem);
        int itemHeight = (rcItem.bottom - rcItem.top);
        int NumItems = ListBox_GetCount(hWnd);
        int nTopIndex = SendMessage(hWnd, LB_GETTOPINDEX, 0, 0);
        int visible_rows = 0;
        int ItemsPerPage = 0;
        int bottom_index = 0;
        int nWidth = (rc.right - rc.left);
        int nHeight = (rc.bottom - rc.top);

        if (NumItems > 0) {
            ItemsPerPage = (nHeight) / itemHeight;
            bottom_index = (nTopIndex + ItemsPerPage);
            if (bottom_index >= NumItems)
                bottom_index = NumItems - 1;
            visible_rows = (bottom_index - nTopIndex) + 1;
            rc.top = visible_rows * itemHeight;
        }

        if (rc.top < rc.bottom) {
            nHeight = (rc.bottom - rc.top);
            HDC hDC = (HDC)wParam;
            Graphics graphics(hDC);
            SolidBrush backBrush(GetThemeColor(ThemeElement::GrayDark));
            graphics.FillRectangle(&backBrush, rc.left, rc.top, nWidth, nHeight);
        }

        ValidateRect(hWnd, &rc);
        return TRUE;
        break;

    }


    case WM_DESTROY:
        // REQUIRED: Remove control subclassing
        RemoveWindowSubclass(hWnd, TransDateFilter_ListBox_SubclassProc, uIdSubclass);
        break;


    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);

}


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
// Process WM_DRAWITEM message for window/dialog: TransDateFilter
// ========================================================================================
void TransDateFilter_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT* lpDrawItem)
{
    if (lpDrawItem->itemID == -1) return;

    if (lpDrawItem->itemAction == ODA_DRAWENTIRE ||
        lpDrawItem->itemAction == ODA_SELECT) {

        int nWidth = (lpDrawItem->rcItem.right - lpDrawItem->rcItem.left);
        int nHeight = (lpDrawItem->rcItem.bottom - lpDrawItem->rcItem.top);

        bool bIsHot = false;

        SaveDC(lpDrawItem->hDC);

        HDC memDC = NULL;         // Double buffering
        HBITMAP hbit = NULL;      // Double buffering

        memDC = CreateCompatibleDC(lpDrawItem->hDC);
        hbit = CreateCompatibleBitmap(lpDrawItem->hDC, nWidth, nHeight);
        if (hbit) SelectObject(memDC, hbit);

        if ((lpDrawItem->itemAction | ODA_SELECT) &&
            (lpDrawItem->itemState & ODS_SELECTED)) {
            bIsHot = true;
        }

        Graphics graphics(memDC);
        graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);


        // Set some defaults in case there is no valid ListBox line number
        std::wstring wszText = AfxGetListBoxText(lpDrawItem->hwndItem, lpDrawItem->itemID);

        DWORD nBackColor = (bIsHot)
            ? GetThemeColor(ThemeElement::Selection)
            : GetThemeColor(ThemeElement::GrayDark);
        DWORD nBackColorHot = GetThemeColor(ThemeElement::Selection);
        DWORD nTextColor = GetThemeColor(ThemeElement::WhiteLight);

        std::wstring wszFontName = AfxGetDefaultFont();
        FontFamily   fontFamily(wszFontName.c_str());
        REAL fontSize = 10;
        int fontStyle = FontStyleRegular;

        StringAlignment HAlignment = StringAlignmentNear;
        StringAlignment VAlignment = StringAlignmentCenter;

        // Paint the full width background using brush 
        SolidBrush backBrush(nBackColor);
        graphics.FillRectangle(&backBrush, 0, 0, nWidth, nHeight);

        Font         font(&fontFamily, fontSize, fontStyle, Unit::UnitPoint);
        SolidBrush   textBrush(nTextColor);
        StringFormat stringF(StringFormatFlagsNoWrap);
        stringF.SetAlignment(HAlignment);
        stringF.SetLineAlignment(VAlignment);

        RectF rcText((REAL)0, (REAL)0, (REAL)nWidth, (REAL)nHeight);
        graphics.DrawString(wszText.c_str(), -1, &font, rcText, &stringF, &textBrush);


        BitBlt(lpDrawItem->hDC, lpDrawItem->rcItem.left,
            lpDrawItem->rcItem.top, nWidth, nHeight, memDC, 0, 0, SRCCOPY);


        // Cleanup
        RestoreDC(lpDrawItem->hDC, -1);
        if (hbit) DeleteObject(hbit);
        if (memDC) DeleteDC(memDC);
    }
}


// ========================================================================================
// Process WM_MEASUREITEM message for window/dialog: TransDateFilter
// ========================================================================================
void TransDateFilter_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT* lpMeasureItem)
{
    lpMeasureItem->itemHeight = AfxScaleY(TRANSDATEFILTER_LISTBOX_ROWHEIGHT);
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
            LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWFIXED | LBS_NOTIFY | LBS_HASSTRINGS,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)TransDateFilter_ListBox_SubclassProc,
            IDC_TRANSDATEFILTER_LISTBOX, NULL);

    int idx = ListBox_AddString(hCtl, L"Today");
    ListBox_SetItemData(hCtl, idx, TransDateFilterType::Today);
    idx = ListBox_AddString(hCtl, L"Yesterday");
    ListBox_SetItemData(hCtl, idx, TransDateFilterType::Yesterday);
    idx = ListBox_AddString(hCtl, L"7 days");
    ListBox_SetItemData(hCtl, idx, TransDateFilterType::Days7);
    idx = ListBox_AddString(hCtl, L"14 days");
    ListBox_SetItemData(hCtl, idx, TransDateFilterType::Days14);
    idx = ListBox_AddString(hCtl, L"30 days");
    ListBox_SetItemData(hCtl, idx, TransDateFilterType::Days30);
    idx = ListBox_AddString(hCtl, L"60 days");
    ListBox_SetItemData(hCtl, idx, TransDateFilterType::Days60);
    idx = ListBox_AddString(hCtl, L"120 days");
    ListBox_SetItemData(hCtl, idx, TransDateFilterType::Days120);
    idx = ListBox_AddString(hCtl, L"Year to Date");
    ListBox_SetItemData(hCtl, idx, TransDateFilterType::YearToDate);
    idx = ListBox_AddString(hCtl, L"Custom");
    ListBox_SetItemData(hCtl, idx, TransDateFilterType::Custom);

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
        HANDLE_MSG(m_hwnd, WM_MEASUREITEM, TransDateFilter_OnMeasureItem);
        HANDLE_MSG(m_hwnd, WM_DRAWITEM, TransDateFilter_OnDrawItem);


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

