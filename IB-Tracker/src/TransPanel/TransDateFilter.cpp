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

// Control on parent window that new selected date will be stored in and displayed.
// That control must be a CustomLabel because we store the full ISO date in that
// control's UserData string.
HWND hDateUpdateParentCtl = NULL;
TransDateFilterType SelectedFilterType = TransDateFilterType::Today;



// ========================================================================================
// Return string based on TransDateFilterType
// ========================================================================================
std::wstring TransDateFilter_GetString(int idx)
{
    switch ((TransDateFilterType)idx)
    {
    case TransDateFilterType::Today: return L"Today";
    case TransDateFilterType::Yesterday: return L"Yesterday";
    case TransDateFilterType::Days7: return L"7 days";
    case TransDateFilterType::Days14: return L"14 days";
    case TransDateFilterType::Days30: return L"30 days";
    case TransDateFilterType::Days60: return L"60 days";
    case TransDateFilterType::Days120: return L"120 days";
    case TransDateFilterType::YearToDate: return L"Year to Date";
    case TransDateFilterType::Custom: return L"Custom";
    default: return L"";
    }
}


// ========================================================================================
// Handle selecting an item in the listview. This will set the parent label window, update
// its CustomDataInt, and set its text label. Finally, it will close the popup dialog.
// ========================================================================================
void TransDateFilter_DoSelected(int idx)
{
    CustomLabel_SetUserDataInt(hDateUpdateParentCtl, idx);
    CustomLabel_SetText(hDateUpdateParentCtl, TransDateFilter_GetString(idx).c_str());
    PostMessage(GetParent(hDateUpdateParentCtl), MSG_DATEPICKER_DATECHANGED,
        GetDlgCtrlID(hDateUpdateParentCtl), (LPARAM)hDateUpdateParentCtl);
    DestroyWindow(HWND_TRANSDATEFILTER);
}


// ========================================================================================
// Listbox subclass Window procedure
// ========================================================================================
LRESULT CALLBACK TransDateFilter_ListBox_SubclassProc(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    switch (uMsg)
    {

    case WM_MOUSEMOVE:
    {
        // Tracks the mouse movement and stores the hot state
        if (GetProp(hWnd, L"HOT") == 0) {
            TRACKMOUSEEVENT trackMouse;
            trackMouse.cbSize = sizeof(trackMouse);
            trackMouse.dwFlags = TME_LEAVE;
            trackMouse.hwndTrack = hWnd;
            trackMouse.dwHoverTime = 1;
            TrackMouseEvent(&trackMouse);
            SetProp(hWnd, L"HOT", (HANDLE)TRUE);
        }
        AfxRedrawWindow(hWnd);
        return 0;
    }
    break;


    case WM_MOUSELEAVE:
    {
        RemoveProp(hWnd, L"HOT");
        AfxRedrawWindow(hWnd);
        return 0;
    }
    break;


    case WM_LBUTTONDOWN:
    {
        int idx = Listbox_ItemFromPoint(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        // The return value contains the index of the nearest item in the LOWORD. The HIWORD is zero 
        // if the specified point is in the client area of the list box, or one if it is outside the 
        // client area.
        if (HIWORD(idx) == 1) break;

        TransDateFilter_DoSelected(idx);
        return 0;
    }
    break;


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
    int margin = AfxScaleX(1);
    SetWindowPos(GetDlgItem(hwnd, IDC_TRANSDATEFILTER_LISTBOX), 0, 
        margin, margin, cx-(margin*2), cy-(margin*2), SWP_NOZORDER | SWP_SHOWWINDOW);
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

        // Determine if we are in a Hot mouseover state
        POINT pt; GetCursorPos(&pt);
        MapWindowPoints(HWND_DESKTOP, lpDrawItem->hwndItem, &pt, 1);
        if (PtInRect(&lpDrawItem->rcItem, pt)) bIsHot = true;

        Graphics graphics(memDC);
        graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

        DWORD nBackColor = (bIsHot)
            ? GetThemeColor(ThemeElement::Selection)
            : GetThemeColor(ThemeElement::GrayMedium);
        DWORD nBackColorHot = GetThemeColor(ThemeElement::Selection);
        DWORD nTextColor = (bIsHot)
            ? GetThemeColor(ThemeElement::WhiteLight)
            : GetThemeColor(ThemeElement::WhiteDark);

        std::wstring wszFontName = AfxGetDefaultFont();
        FontFamily   fontFamily(wszFontName.c_str());
        REAL fontSize = 9;
        int fontStyle = FontStyleRegular;

        // Paint the full width background using brush 
        SolidBrush backBrush(nBackColor);
        graphics.FillRectangle(&backBrush, 0, 0, nWidth, nHeight);

        Font         font(&fontFamily, fontSize, fontStyle, Unit::UnitPoint);
        SolidBrush   textBrush(nTextColor);
        StringFormat stringF(StringFormatFlagsNoWrap);
        stringF.SetLineAlignment(StringAlignment::StringAlignmentCenter);

        std::wstring wszText;
        
        if ((int)SelectedFilterType == lpDrawItem->itemID) wszText = L"\u2713";
        RectF rcText1((REAL)0, (REAL)0, (REAL)AfxScaleX(24), (REAL)nHeight);
        stringF.SetAlignment(StringAlignment::StringAlignmentCenter);
        graphics.DrawString(wszText.c_str(), -1, &font, rcText1, &stringF, &textBrush);

        wszText = AfxGetListBoxText(lpDrawItem->hwndItem, lpDrawItem->itemID);
        RectF rcText2((REAL)AfxScaleX(24), (REAL)0, (REAL)nWidth, (REAL)nHeight);
        stringF.SetAlignment(StringAlignment::StringAlignmentNear);
        graphics.DrawString(wszText.c_str(), -1, &font, rcText2, &stringF, &textBrush);

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

    for (int i = (int)TransDateFilterType::Today; i <= (int)TransDateFilterType::Custom; ++i) {
        int idx = ListBox_AddString(hCtl, TransDateFilter_GetString(i).c_str());
        ListBox_SetItemData(hCtl, idx, i);
    }

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
HWND TransDateFilter_CreatePicker(HWND hParent, HWND hParentCtl)
{
    TransDateFilter.Create(hParent, L"", 0, 0, 0, 0,
        WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);

    int margin = AfxScaleX(1);
    RECT rc; GetWindowRect(hParentCtl, &rc);
    SetWindowPos(TransDateFilter.WindowHandle(), HWND_TOP,
        rc.left-margin, rc.bottom,
        AfxScaleX(TRANSDATEFILTER_WIDTH) + (margin * 2),
        AfxScaleY(TRANSDATEFILTER_LISTBOX_ROWHEIGHT * 9),
        SWP_SHOWWINDOW);

    // Get the current selected filter and apply it to the popup
    SelectedFilterType = (TransDateFilterType)CustomLabel_GetUserDataInt(hParentCtl);


    // Set the module global hUpdateParentCtl after the above is created in
    // to ensure the variable address is correct.
    hDateUpdateParentCtl = hParentCtl;

    return TransDateFilter.WindowHandle();
}

