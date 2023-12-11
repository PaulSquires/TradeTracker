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
#include "CategoryPopup.h"


HWND HWND_CATEGORYPOPUP = NULL;

CCategoryPopup CategoryPopup;

// Control on parent window that new selected category will be stored in and displayed.
HWND hCategoryUpdateParentCtl = NULL;
int selected_category = 0;
HHOOK hCategoryPopupMouseHook = nullptr;



// ========================================================================================
// Handle selecting an item in the listview. This will set the parent label window, update
// its CustomDataInt, and set its text label. Finally, it will close the popup dialog.
// ========================================================================================
void CategoryPopup_DoSelected(HWND hListBox, int idx)
{
    int item_data = (int)ListBox_GetItemData(hListBox, idx);
    if (item_data != selected_category) {
        CustomLabel_SetUserDataInt(hCategoryUpdateParentCtl, item_data);
        CustomLabel_SetText(hCategoryUpdateParentCtl, GetCategoryDescription(item_data));
        SendMessage(GetParent(GetParent(hCategoryUpdateParentCtl)), MSG_CATEGORY_CATEGORYCHANGED, 0, 0);
    }
    DestroyWindow(HWND_CATEGORYPOPUP);
}


// ========================================================================================
// Listbox subclass Window procedure
// ========================================================================================
LRESULT CALLBACK CategoryPopup_ListBox_SubclassProc(
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

        CategoryPopup_DoSelected(hWnd, idx);
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
            SolidBrush back_brush(COLOR_GRAYDARK);
            graphics.FillRectangle(&back_brush, rc.left, rc.top, nWidth, nHeight);
        }

        ValidateRect(hWnd, &rc);
        return TRUE;
        break;

    }


    case WM_DESTROY:
        // REQUIRED: Remove control subclassing
        RemoveWindowSubclass(hWnd, CategoryPopup_ListBox_SubclassProc, uIdSubclass);
        break;


    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);

}


// ========================================================================================
// Process WM_SIZE message for window/dialog: CategoryPopup
// ========================================================================================
void CategoryPopup_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    int margin = AfxScaleX(1);
    SetWindowPos(GetDlgItem(hwnd, IDC_CATEGORYPOPUP_LISTBOX), 0,
        margin, margin, cx - (margin * 2), cy - (margin * 2), SWP_NOZORDER | SWP_SHOWWINDOW);
    return;
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: CategoryPopup
// ========================================================================================
BOOL CategoryPopup_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    // Handle all of the painting in WM_PAINT
    return TRUE;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: CategoryPopup
// ========================================================================================
void CategoryPopup_OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    Graphics graphics(hdc);

    // Create the background brush
    SolidBrush back_brush(COLOR_BLACK);

    // Paint the background using brush.
    int nWidth = (ps.rcPaint.right - ps.rcPaint.left);
    int nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Process WM_DRAWITEM message for window/dialog: CategoryPopup
// ========================================================================================
void CategoryPopup_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT* lpDrawItem)
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

        if (selected_category == lpDrawItem->itemData) text = GLYPH_CHECKMARK;
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
// Process WM_MEASUREITEM message for window/dialog: CategoryPopup
// ========================================================================================
void CategoryPopup_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT* lpMeasureItem)
{
    lpMeasureItem->itemHeight = AfxScaleY(CATEGORYPOPUP_LISTBOX_ROWHEIGHT);
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: CategoryPopup
// ========================================================================================
BOOL CategoryPopup_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    HWND_CATEGORYPOPUP = hwnd;

    HWND hCtl =
        CategoryPopup.AddControl(Controls::ListBox, hwnd, IDC_CATEGORYPOPUP_LISTBOX, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWFIXED | LBS_NOTIFY | LBS_HASSTRINGS,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)CategoryPopup_ListBox_SubclassProc,
            IDC_CATEGORYPOPUP_LISTBOX, NULL);

    for (int i = CATEGORY_START; i <= CATEGORY_END; ++i) {
        int idx = ListBox_AddString(hCtl, GetCategoryDescription(i).c_str());
        ListBox_SetItemData(hCtl, idx, i);
    }

    return TRUE;
}


// ========================================================================================
// Global mouse hook.
// ========================================================================================
LRESULT CALLBACK CategoryPopupHook(int Code, WPARAM wParam, LPARAM lParam)
{
    // messages are defined in a linear way the first being WM_LBUTTONUP up to WM_MBUTTONDBLCLK
    // this subset does not include WM_MOUSEMOVE, WM_MOUSEWHEEL and a few others
    // (Don't handle WM_LBUTTONUP here because the mouse is most likely outside the menu popup
    // at the point this hook is called).
    if (wParam == WM_LBUTTONDOWN)
    {
        if (HWND_CATEGORYPOPUP)
        {
            POINT pt;       GetCursorPos(&pt);
            RECT rcWindow;  GetWindowRect(HWND_CATEGORYPOPUP, &rcWindow);

            // if the mouse action is outside the menu, hide it. the window procedure will also unset this hook 
            if (!PtInRect(&rcWindow, pt)) {
                DestroyWindow(HWND_CATEGORYPOPUP);
            }
        }
    }

    return CallNextHookEx(NULL, Code, wParam, lParam);
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CCategoryPopup::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(m_hwnd, WM_CREATE, CategoryPopup_OnCreate);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, CategoryPopup_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, CategoryPopup_OnPaint);
        HANDLE_MSG(m_hwnd, WM_SIZE, CategoryPopup_OnSize);
        HANDLE_MSG(m_hwnd, WM_MEASUREITEM, CategoryPopup_OnMeasureItem);
        HANDLE_MSG(m_hwnd, WM_DRAWITEM, CategoryPopup_OnDrawItem);


    case WM_DESTROY:
    {
        // unhook and remove our global mouse hook
        UnhookWindowsHookEx(hCategoryPopupMouseHook);
        hCategoryPopupMouseHook = nullptr;

        // Reset our destroyed variable for future use of the popup
        HWND_CATEGORYPOPUP = NULL;
        return 0;
    }
    break;


    case WM_MOUSEACTIVATE:
    {
        return MA_NOACTIVATE;
    }
    break;


    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}


// ========================================================================================
// Create category popup control and move it into position under the 
// specified incoming control.
// ========================================================================================
HWND CategoryPopup_CreatePopup(HWND hParent, HWND hParentCtl)
{
    HWND hPopup = CategoryPopup.Create(hParent, L"", 0, 0, 0, 0,
        WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR |
        WS_EX_NOACTIVATE);
    
    int categories_count = CATEGORY_END + 1;
    if (CategoryControl_Getallow_all_categories(hParent)) ++categories_count;
    
    int margin = AfxScaleX(1);
    RECT rc; GetWindowRect(hParentCtl, &rc);
    SetWindowPos(hPopup, 0,
        rc.left - margin, rc.bottom,
        AfxScaleX(CATEGORYPOPUP_WIDTH),
        AfxScaleY(CATEGORYPOPUP_LISTBOX_ROWHEIGHT * (float)categories_count),
        SWP_NOZORDER | SWP_SHOWWINDOW | SWP_NOACTIVATE);

    if (CategoryControl_Getallow_all_categories(hParent)) {
        HWND hListBox = GetDlgItem(hPopup, IDC_CATEGORYPOPUP_LISTBOX);
        int idx = ListBox_InsertString(hListBox, 0, GetCategoryDescription(CATEGORY_OTHER).c_str());
        ListBox_SetItemData(hListBox, idx, CATEGORY_OTHER);
        idx = ListBox_InsertString(hListBox, 0, GetCategoryDescription(CATEGORY_ALL).c_str());
        ListBox_SetItemData(hListBox, idx, CATEGORY_ALL);
    }

    // Get the current selected category and apply it to the popup
    selected_category = CustomLabel_GetUserDataInt(hParentCtl);


    // Set the module global hUpdateParentCtl after the above is created in order
    // to ensure the variable address is correct.
    hCategoryUpdateParentCtl = hParentCtl;


    // Set our hook and store the handle in the global variable
    hCategoryPopupMouseHook = SetWindowsHookEx(WH_MOUSE, CategoryPopupHook, 0, GetCurrentThreadId());

    return hPopup;
}

