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

#include "Config/Config.h"
#include "CustomLabel/CustomLabel.h"
#include "Utilities/ListBoxData.h"

#include "CustomComboPopup.h"


HWND HWND_CUSTOMCOMBOPOPUP = NULL;

CCustomComboPopup CustomComboPopup;

// Control on parent window that new selected category will be stored in and displayed.
HWND hSortFilterUpdateParentCtl = NULL;
int selected_filter = 0;



// ========================================================================================
// Handle selecting an item in the listview. This will set the parent label window, update
// its CustomDataInt, and set its text label. Finally, it will close the popup dialog.
// ========================================================================================
void CustomComboPopup_DoSelected(HWND hListBox, int idx)
{
    std::wstring item_text = AfxGetListBoxText(hListBox, idx);
    int item_data = (int)ListBox_GetItemData(hListBox, idx);
    CustomLabel_SetUserDataInt(hSortFilterUpdateParentCtl, item_data);
    CustomLabel_SetText(hSortFilterUpdateParentCtl, item_text);
    SendMessage(GetParent(GetParent(hSortFilterUpdateParentCtl)), 
        MSG_CUSTOMCOMBO_ITEMCHANGED, item_data, 0);
    DestroyWindow(HWND_CUSTOMCOMBOPOPUP);
}


// ========================================================================================
// Listbox subclass Window procedure
// ========================================================================================
LRESULT CALLBACK CustomComboPopup_ListBox_SubclassProc(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    switch (uMsg)
    {

    case WM_MOUSEMOVE:
    {
        // Tracks the mouse movement and stores the hot state
        if (GetProp(hWnd, L"HOT") == 0) {
            TRACKMOUSEEVENT trackMouse{};
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

        CustomComboPopup_DoSelected(hWnd, idx);
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
        int item_height = (rcItem.bottom - rcItem.top);
        int items_count = ListBox_GetCount(hWnd);
        int top_index = (int)SendMessage(hWnd, LB_GETTOPINDEX, 0, 0);
        int visible_rows = 0;
        int items_per_page = 0;
        int bottom_index = 0;
        int nWidth = (rc.right - rc.left);
        int nHeight = (rc.bottom - rc.top);

        if (items_count > 0) {
            items_per_page = (nHeight) / item_height;
            bottom_index = (top_index + items_per_page);
            if (bottom_index >= items_count)
                bottom_index = items_count - 1;
            visible_rows = (bottom_index - top_index) + 1;
            rc.top = visible_rows * item_height;
        }

        if (rc.top < rc.bottom) {
            nHeight = (rc.bottom - rc.top);
            HDC hDC = (HDC)wParam;
            Graphics graphics(hDC);
            SolidBrush back_brush(COLOR_GRAYMEDIUM);
            graphics.FillRectangle(&back_brush, rc.left, rc.top, nWidth, nHeight);
        }

        ValidateRect(hWnd, &rc);
        return TRUE;
        break;

    }


    case WM_DESTROY:
        // REQUIRED: Remove control subclassing
        RemoveWindowSubclass(hWnd, CustomComboPopup_ListBox_SubclassProc, uIdSubclass);
        break;


    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);

}


// ========================================================================================
// Process WM_SIZE message for window/dialog: CustomComboPopup
// ========================================================================================
void CustomComboPopup_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    int margin = AfxScaleX(1);
    SetWindowPos(GetDlgItem(hwnd, IDC_CUSTOMCOMBOPOPUP_LISTBOX), 0,
        margin, margin, cx - (margin * 2), cy - (margin * 2), SWP_NOZORDER | SWP_SHOWWINDOW);
    return;
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: CustomComboPopup
// ========================================================================================
BOOL CustomComboPopup_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    // Handle all of the painting in WM_PAINT
    return TRUE;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: CustomComboPopup
// ========================================================================================
void CustomComboPopup_OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    Graphics graphics(hdc);

    // Create the background brush
    SolidBrush back_brush(COLOR_GRAYMEDIUM);

    // Paint the background using brush.
    int nWidth = (ps.rcPaint.right - ps.rcPaint.left);
    int nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Process WM_DRAWITEM message for window/dialog: CustomComboPopup
// ========================================================================================
void CustomComboPopup_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT* lpDrawItem)
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

        DWORD back_color = (bIsHot) ? COLOR_SELECTION : COLOR_GRAYMEDIUM;
        DWORD back_color_hot = COLOR_SELECTION;
        DWORD ntext_color = (bIsHot) ? COLOR_WHITELIGHT : COLOR_WHITEDARK;

        std::wstring font_name = AfxGetDefaultFont();
        FontFamily   fontFamily(font_name.c_str());
        REAL fontSize = 9;
        int fontStyle = FontStyleRegular;

        // Paint the full width background using brush 
        SolidBrush back_brush(back_color);
        graphics.FillRectangle(&back_brush, 0, 0, nWidth, nHeight);

        Font         font(&fontFamily, fontSize, fontStyle, Unit::UnitPoint);
        SolidBrush   text_brush(ntext_color);
        StringFormat stringF(StringFormatFlagsNoWrap);
        stringF.SetLineAlignment(StringAlignment::StringAlignmentCenter);

        std::wstring text;

        if (selected_filter == lpDrawItem->itemData) text = GLYPH_CHECKMARK;
        RectF rcText1((REAL)0, (REAL)0, (REAL)AfxScaleX(24), (REAL)nHeight);
        stringF.SetAlignment(StringAlignment::StringAlignmentCenter);
        graphics.DrawString(text.c_str(), -1, &font, rcText1, &stringF, &text_brush);

        text = AfxGetListBoxText(lpDrawItem->hwndItem, lpDrawItem->itemID);
        RectF rcText2((REAL)AfxScaleX(24), (REAL)0, (REAL)nWidth, (REAL)nHeight);
        stringF.SetAlignment(StringAlignment::StringAlignmentNear);
        graphics.DrawString(text.c_str(), -1, &font, rcText2, &stringF, &text_brush);

        BitBlt(lpDrawItem->hDC, lpDrawItem->rcItem.left,
            lpDrawItem->rcItem.top, nWidth, nHeight, memDC, 0, 0, SRCCOPY);


        // Cleanup
        RestoreDC(lpDrawItem->hDC, -1);
        if (hbit) DeleteObject(hbit);
        if (memDC) DeleteDC(memDC);
    }
}


// ========================================================================================
// Process WM_MEASUREITEM message for window/dialog: CustomComboPopup
// ========================================================================================
void CustomComboPopup_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT* lpMeasureItem)
{
    lpMeasureItem->itemHeight = AfxScaleY(CUSTOMCOMBOPOPUP_LISTBOX_ROWHEIGHT);
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: CustomComboPopup
// ========================================================================================
BOOL CustomComboPopup_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    HWND_CUSTOMCOMBOPOPUP = hwnd;

    HWND hCtl =
        CustomComboPopup.AddControl(Controls::ListBox, hwnd, IDC_CUSTOMCOMBOPOPUP_LISTBOX, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWFIXED | LBS_NOTIFY | LBS_HASSTRINGS,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)CustomComboPopup_ListBox_SubclassProc,
            IDC_CUSTOMCOMBOPOPUP_LISTBOX, NULL);

    return TRUE;
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CCustomComboPopup::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Prevent a recursive calling of the WM_NCACTIVATE message as DestroyWindow deactivates the window.
    static bool destroyed = false;

    switch (msg)
    {
        HANDLE_MSG(m_hwnd, WM_CREATE, CustomComboPopup_OnCreate);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, CustomComboPopup_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, CustomComboPopup_OnPaint);
        HANDLE_MSG(m_hwnd, WM_SIZE, CustomComboPopup_OnSize);
        HANDLE_MSG(m_hwnd, WM_MEASUREITEM, CustomComboPopup_OnMeasureItem);
        HANDLE_MSG(m_hwnd, WM_DRAWITEM, CustomComboPopup_OnDrawItem);


    case WM_DESTROY:
    {
        // Reset our destroyed variable for future use of the popup
        destroyed = false;
        HWND_CUSTOMCOMBOPOPUP = NULL;
        return 0;
    }
    break;


    case WM_NCACTIVATE:
    {
        // Detect that we have clicked outside the popup CategoryPopup and will now close it.
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
// Create SortFilter popup control and move it into position under the 
// specified incoming control.
// ========================================================================================
HWND CustomComboPopup_CreatePopup(HWND hParent, HWND hParentCtl, int NumItems)
{
    HWND hPopup = CustomComboPopup.Create(hParent, L"", 0, 0, 0, 0,
        WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);

    int combo_count = NumItems;

    int margin = AfxScaleX(1);
    RECT rc; GetWindowRect(hParentCtl, &rc);
    SetWindowPos(hPopup, HWND_TOP,
        rc.left - margin, rc.bottom,
        AfxScaleX(CUSTOMCOMBOPOPUP_WIDTH),
        AfxScaleY(CUSTOMCOMBOPOPUP_LISTBOX_ROWHEIGHT * (float)combo_count) + AfxScaleY(3),
        SWP_SHOWWINDOW);

    // Get the current selected category and apply it to the popup
    selected_filter = CustomLabel_GetUserDataInt(hParentCtl);

    // Set the module global hUpdateParentCtl after the above is created in order
    // to ensure the variable address is correct.
    hSortFilterUpdateParentCtl = hParentCtl;

    return hPopup;
}

