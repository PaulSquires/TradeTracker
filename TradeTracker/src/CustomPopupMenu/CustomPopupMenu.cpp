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

#include "MainWindow/MainWindow.h"
#include "Utilities/UserMessages.h"
#include "Utilities/ListBoxData.h"
#include "CustomPopupMenu.h"


HWND HWND_CUSTOMPOPUPMENU = NULL;   // needed for the global hook

CCustomPopupMenu CustomPopupMenu;

HHOOK hCustomPopupMouseHook = nullptr;


// ========================================================================================
// Listbox subclass Window procedure
// ========================================================================================
LRESULT CALLBACK CustomPopupMenu_ListBox_SubclassProc(
    HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    switch (uMsg) {

    case WM_MOUSEMOVE: {
        // Tracks the mouse movement and stores the hot state
        if (GetProp(hwnd, L"HOT") == 0) {
            TRACKMOUSEEVENT trackMouse{};
            trackMouse.cbSize = sizeof(trackMouse);
            trackMouse.dwFlags = TME_LEAVE;
            trackMouse.hwndTrack = hwnd;
            trackMouse.dwHoverTime = 1;
            TrackMouseEvent(&trackMouse);
            SetProp(hwnd, L"HOT", (HANDLE)true);
        }
        AfxRedrawWindow(hwnd);
        return 0;
    }

    case WM_MOUSELEAVE: {
        RemoveProp(hwnd, L"HOT");
        AfxRedrawWindow(hwnd);
        return 0;
    }

    case WM_LBUTTONDOWN: {
        int idx = Listbox_ItemFromPoint(hwnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        // The return value contains the index of the nearest item in the LOWORD. The HIWORD is zero 
        // if the specified point is in the client area of the list box, or one if it is outside the 
        // client area.
        if (HIWORD(idx) == 1) break;

        SendMessage(GetParent(hwnd), MSG_CUSTOMPOPUPMENU_DOSELECTED, idx, 0);

        return 0;
    }

    case WM_ERASEBKGND: {
        // If the number of lines in the listbox maybe less than the number per page then 
        // calculate from last item to bottom of listbox, otherwise calculate based on
        // the mod of the lineheight to listbox height so we can color the partial line
        // that won't be displayed at the bottom of the list.
        RECT rc; GetClientRect(hwnd, &rc);


        RECT rcItem{};
        SendMessage(hwnd, LB_GETITEMRECT, 0, (LPARAM)&rcItem);
        int item_height = (rcItem.bottom - rcItem.top);
        int items_count = ListBox_GetCount(hwnd);
        int top_index = (int)SendMessage(hwnd, LB_GETTOPINDEX, 0, 0);
        int visible_rows = 0;
        int items_per_page = 0;
        int bottom_index = 0;
        int width = (rc.right - rc.left);
        int height = (rc.bottom - rc.top);

        if (items_count > 0) {
            items_per_page = (height) / item_height;
            bottom_index = (top_index + items_per_page);
            if (bottom_index >= items_count)
                bottom_index = items_count - 1;
            visible_rows = (bottom_index - top_index) + 1;
            rc.top = visible_rows * item_height;
        }

        if (rc.top < rc.bottom) {
            height = (rc.bottom - rc.top);
            HDC hDC = (HDC)wParam;
            Graphics graphics(hDC);
            SolidBrush back_brush(COLOR_GRAYLIGHT);
            graphics.FillRectangle(&back_brush, rc.left, rc.top, width, height);
        }

        ValidateRect(hwnd, &rc);

        return true;
    }

    case WM_DESTROY: {
        // REQUIRED: Remove control subclassing
        RemoveWindowSubclass(hwnd, CustomPopupMenu_ListBox_SubclassProc, uIdSubclass);
        break;
    }

    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}


// ========================================================================================
// Process WM_SIZE message for window/dialog: CCustomPopupMenu
// ========================================================================================
void CCustomPopupMenu::OnSize(HWND hwnd, UINT state, int cx, int cy) {
    int vmargin = AfxScaleY(1);
    int hmargin = AfxScaleY(1);
    SetWindowPos(GetDlgItem(hwnd, IDC_CUSTOMPOPUPMENU_LISTBOX), 0, 
        hmargin, vmargin, cx - (hmargin * 2), cy - (vmargin * 2), SWP_NOZORDER | SWP_SHOWWINDOW);
    return;
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: CCustomPopupMenu
// ========================================================================================
bool CCustomPopupMenu::OnEraseBkgnd(HWND hwnd, HDC hdc) {
    // Handle all of the painting in WM_PAINT
    return true;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: CCustomPopupMenu
// ========================================================================================
void CCustomPopupMenu::OnPaint(HWND hwnd) {
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    Graphics graphics(hdc);

    // Create the background brush
    SolidBrush back_brush(COLOR_GRAYLIGHT);

    // Paint the background using brush.
    int width = (ps.rcPaint.right - ps.rcPaint.left);
    int height = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, width, height);

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Process WM_DRAWITEM message for window/dialog: CCustomPopupMenu
// ========================================================================================
void CCustomPopupMenu::OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT* lpDrawItem) {

    if (lpDrawItem->itemID == -1) return;

    if (lpDrawItem->itemAction == ODA_DRAWENTIRE ||
        lpDrawItem->itemAction == ODA_SELECT) {

        int width = (lpDrawItem->rcItem.right - lpDrawItem->rcItem.left);
        int height = (lpDrawItem->rcItem.bottom - lpDrawItem->rcItem.top);

        bool is_hot = false;

        SaveDC(lpDrawItem->hDC);

        HDC memDC = NULL;         // Double buffering
        HBITMAP hbit = NULL;      // Double buffering

        memDC = CreateCompatibleDC(lpDrawItem->hDC);
        hbit = CreateCompatibleBitmap(lpDrawItem->hDC, width, height);
        if (hbit) SelectObject(memDC, hbit);

        if ((lpDrawItem->itemAction | ODA_SELECT) &&
            (lpDrawItem->itemState & ODS_SELECTED)) {
            is_hot = true;
        }

        // Determine if we are in a Hot mouseover state
        POINT pt; GetCursorPos(&pt);
        MapWindowPoints(HWND_DESKTOP, lpDrawItem->hwndItem, &pt, 1);
        if (PtInRect(&lpDrawItem->rcItem, pt)) is_hot = true;

        std::wstring text = items.at(lpDrawItem->itemID).text;
        std::wstring glyph;

        if (items.at(lpDrawItem->itemID).is_separator) is_hot = false;

        Graphics graphics(memDC);
        graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

        DWORD back_color = (is_hot) ? COLOR_GRAYDARK: COLOR_GRAYMEDIUM;
        DWORD ntext_color = (is_hot) ? COLOR_WHITELIGHT : COLOR_WHITELIGHT; 

        std::wstring font_name = AfxGetDefaultFont();
        FontFamily   fontFamily(font_name.c_str());
        REAL font_size = 9;
        int font_style = FontStyleRegular;

        // Paint the full width background using brush 
        SolidBrush back_brush(back_color);
        graphics.FillRectangle(&back_brush, 0, 0, width, height);

        Font         font(&fontFamily, font_size, font_style, Unit::UnitPoint);
        SolidBrush   text_brush(ntext_color);
        StringFormat stringF(StringFormatFlagsNoWrap);
        stringF.SetLineAlignment(StringAlignment::StringAlignmentCenter);

        if (items.at(lpDrawItem->itemID).is_separator) {
            // Draw the horizontal line
            int margin = AfxScaleX(12);
            ARGB clrPen = COLOR_GRAYLIGHT;
            Pen pen(clrPen, 1);
            graphics.DrawLine(&pen, margin, height / 2, width - margin, height / 2);
        }
        else {
            if (selected_item == items.at(lpDrawItem->itemID).id) glyph = GLYPH_CHECKMARK;
            RectF rcText1((REAL)0, (REAL)0, (REAL)glyph_width, (REAL)height);
            stringF.SetAlignment(StringAlignment::StringAlignmentCenter);
            graphics.DrawString(glyph.c_str(), -1, &font, rcText1, &stringF, &text_brush);

            RectF rcText2((REAL)glyph_width, (REAL)0, (REAL)width, (REAL)height);
            stringF.SetAlignment(StringAlignment::StringAlignmentNear);
            graphics.DrawString(text.c_str(), -1, &font, rcText2, &stringF, &text_brush);
        }

        BitBlt(lpDrawItem->hDC, lpDrawItem->rcItem.left,
            lpDrawItem->rcItem.top, width, height, memDC, 0, 0, SRCCOPY);

        // Cleanup
        RestoreDC(lpDrawItem->hDC, -1);
        if (hbit) DeleteObject(hbit);
        if (memDC) DeleteDC(memDC);
    }
}


// ========================================================================================
// Process WM_MEASUREITEM message for window/dialog: CCustomPopupMenu
// ========================================================================================
void CCustomPopupMenu::OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT* lpMeasureItem) {
    lpMeasureItem->itemHeight = 
        (items.at(lpMeasureItem->itemID).is_separator) ? row_height_separator : row_height;
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: CCustomPopupMenu
// ========================================================================================
bool CCustomPopupMenu::OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    hWindow = hwnd;
    HWND_CUSTOMPOPUPMENU = hwnd;

    HWND hCtl =
        AddControl(Controls::ListBox, hwnd, IDC_CUSTOMPOPUPMENU_LISTBOX, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWVARIABLE | LBS_NOTIFY,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)CustomPopupMenu_ListBox_SubclassProc,
            IDC_CUSTOMPOPUPMENU_LISTBOX, NULL);

    row_count = 0;
    sep_count = 0;

    for (auto& item : items ) {
        if (item.is_separator) {
            item.text = L"-";
            item.id = -1;
            sep_count++;
        }
        else {
            row_count++;
        }
        int idx = ListBox_AddString(hCtl, L"");
    }

    return true;
}


// ========================================================================================
// Global mouse hook.
// ========================================================================================
LRESULT CALLBACK CustomPopupMenuHook(int Code, WPARAM wParam, LPARAM lParam) {
    // messages are defined in a linear way the first being WM_LBUTTONUP up to WM_MBUTTONDBLCLK
    // this subset does not include WM_MOUSEMOVE, WM_MOUSEWHEEL and a few others
    // (Don't handle WM_LBUTTONUP here because the mouse is most likely outside the menu popup
    // at the point this hook is called).
    if (wParam == WM_LBUTTONDOWN) {
        if (HWND_CUSTOMPOPUPMENU) {
            POINT pt;       GetCursorPos(&pt);
            RECT rcWindow;  GetWindowRect(HWND_CUSTOMPOPUPMENU, &rcWindow);

            // if the mouse action is outside the menu, hide it. the window procedure will also unset this hook 
            if (!PtInRect(&rcWindow, pt)) {
                SendMessage(HWND_CUSTOMPOPUPMENU, MSG_CUSTOMPOPUPMENU_CANCEL, 0, 0);
            }
        }
    }

    return CallNextHookEx(NULL, Code, wParam, lParam);
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CCustomPopupMenu::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        HANDLE_MSG(m_hwnd, WM_CREATE, OnCreate);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, OnPaint);
        HANDLE_MSG(m_hwnd, WM_SIZE, OnSize);
        HANDLE_MSG(m_hwnd, WM_MEASUREITEM, OnMeasureItem);
        HANDLE_MSG(m_hwnd, WM_DRAWITEM, OnDrawItem);


    case MSG_CUSTOMPOPUPMENU_DOSELECTED: {
        int cursel = (int)wParam;
        if (!items.at(cursel).is_separator) {
            selected_item = items.at(cursel).id;
            DestroyWindow(hWindow);
        }
        return 0;
    }

    case MSG_CUSTOMPOPUPMENU_CANCEL: {
        selected_item = -1;
        DestroyWindow(hWindow);
        return 0;
    }

    case WM_DESTROY: {
        // unhook and remove our global mouse hook
        UnhookWindowsHookEx(hCustomPopupMouseHook);
        hCustomPopupMouseHook = nullptr;

        // Reset our destroyed variable for future use of the popup
        HWND_CUSTOMPOPUPMENU = NULL;
        items.clear();

        PostQuitMessage(0);
        return 0;
    }

    case WM_MOUSEACTIVATE: {
        return MA_NOACTIVATE;
    }

    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}


// ========================================================================================
// Create popup control 
// ========================================================================================
int CCustomPopupMenu::Show(HWND hwndParent, std::vector<CCustomPopupMenuItem> popup_items, 
    int initial_selected_item, int left, int top, int width_override) {
    // If a popup menu is already active and showing then destroy it
    // before showing the new popup menu.
    if (IsWindow(hWindow)) {
        selected_item = -1;
        DestroyWindow(hWindow);
    }

    hParent = hwndParent;
    items = popup_items;
    selected_item = initial_selected_item;

    Create(hParent, L"", 0, 0, 0, 0,
        WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR |
        WS_EX_NOACTIVATE);
    
    HBRUSH hbrBackground = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
    SetClassLongPtr(hWindow, GCLP_HBRBACKGROUND, (LONG_PTR)hbrBackground);

    int text_length = 0;
    SIZEL size{};

    if (width_override != -1) {
        row_width = width_override;
    }
    else {
        HDC hdc = GetDC(hWindow);
        SelectFont(hdc, m_hFont);
        for (const auto& item : items) {
            GetTextExtentPoint(hdc, item.text.c_str(), (int)item.text.length(), &size);
            text_length = max(text_length, size.cx);
        }
        // Add some trailing right margin
        row_width = glyph_width + text_length + AfxScaleX(32);
        ReleaseDC(hWindow, hdc);
    }

    int height = (row_count * row_height) + (sep_count * row_height_separator);

    SetWindowPos(hWindow, 0, left, top, row_width, height,
        SWP_NOZORDER | SWP_SHOWWINDOW | SWP_NOACTIVATE);

    // Set our hook and store the handle in the global variable
    hCustomPopupMouseHook = SetWindowsHookEx(WH_MOUSE, CustomPopupMenuHook, 0, GetCurrentThreadId());

    // Call modal message pump and wait for it to end.
    MSG msg{};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!IsWindow(MainWindow.hWindow)) break;

        if (msg.message == WM_KEYUP) {
            if (msg.wParam == VK_ESCAPE) {
                selected_item = -1;
                DestroyWindow(hWindow);
            }
            if (msg.wParam == VK_RETURN || msg.wParam == VK_SPACE) {
                int cursel = ListBox_GetCurSel(GetDlgItem(hWindow, IDC_CUSTOMPOPUPMENU_LISTBOX));
                if (!items.at(cursel).is_separator) {
                    selected_item = items.at(cursel).id;
                    DestroyWindow(hWindow);
                }
            }
        }

        // Determines whether a message is intended for the specified
        // dialog box and, if it is, processes the message.
        if (!IsDialogMessage(hWindow, &msg)) {
            // Translates virtual-key messages into character messages.
            TranslateMessage(&msg);
            // Dispatches a message to a window procedure.
            DispatchMessage(&msg);
        }
    }

    return selected_item;
}

