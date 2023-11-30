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
#include "CustomLabel/CustomLabel.h"
#include "Utilities/UserMessages.h"
#include "Config/Config.h"
#include "CustomVScrollBar/CustomVScrollBar.h"
#include "MainWindow/tws-client.h"
#include "MainWindow/MainWindow.h"
#include "ActiveTrades/ActiveTrades.h"
#include "ClosedTrades/ClosedTrades.h"
#include "TradeDialog/TradeDialog.h"
#include "Transactions/TransPanel.h"
#include "TickerTotals/TickerTotals.h"
#include "JournalNotes/JournalNotes.h"
#include "TradePlan/TradePlan.h"
#include "Utilities/ListBoxData.h"
#include "SideMenu.h"

CSideMenu SideMenu;

HWND HWND_SIDEMENU = NULL;


// ========================================================================================
// Select the SideMenu listbox row based on itemData (CtrlID)
// ========================================================================================
void SideMenu_SelectMenuItem(HWND hwnd, int CtrlId)
{
    HWND hListBox = GetDlgItem(hwnd, IDC_SIDEMENU_LISTBOX);

    int nCurSel = -1;
    int item_count = ListBox_GetCount(hListBox);
    
    for (int i = 0; i < item_count; i++) {
        if (ListBox_GetItemData(hListBox, i) == CtrlId) {
            nCurSel = i; break;
        }
    }

    ListBox_SetCurSel(hListBox, nCurSel);
    AfxRedrawWindow(hListBox);
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: SideMenu
// ========================================================================================
BOOL SideMenu_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    // Handle all of the painting in WM_PAINT
    return TRUE;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: SideMenu
// ========================================================================================
void SideMenu_OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    Graphics graphics(hdc);

    // Create the background brush
    Color back_color(COLOR_BLACK);
    SolidBrush back_brush(back_color);

    // Paint the background using brush.
    int nWidth = (ps.rcPaint.right - ps.rcPaint.left);
    int nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Process WM_MEASUREITEM message for window/dialog: SideMenu
// ========================================================================================
void SideMenu_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT* lpMeasureItem)
{
    HWND hListBox = GetDlgItem(hwnd, IDC_SIDEMENU_LISTBOX);
    lpMeasureItem->itemHeight = ListBox_GetItemHeight(hListBox, lpMeasureItem->itemID);
}


// ========================================================================================
// Process WM_DRAWITEM message for window/dialog: SideMenu
// ========================================================================================
void SideMenu_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT* lpDrawItem)
{
    if (lpDrawItem->itemID == -1) return;

    if (lpDrawItem->itemAction == ODA_DRAWENTIRE ||
        lpDrawItem->itemAction == ODA_SELECT) {

        int nWidth = (lpDrawItem->rcItem.right - lpDrawItem->rcItem.left);
        int nHeight = (lpDrawItem->rcItem.bottom - lpDrawItem->rcItem.top);

        SaveDC(lpDrawItem->hDC);

        HDC memDC = NULL;         // Double buffering
        HBITMAP hbit = NULL;      // Double buffering

        memDC = CreateCompatibleDC(lpDrawItem->hDC);
        hbit = CreateCompatibleBitmap(lpDrawItem->hDC, nWidth, nHeight);
        if (hbit) SelectObject(memDC, hbit);

        Graphics graphics(memDC);
        graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

        RECT rc{};
        bool is_hot = false;
        POINT pt{}; GetCursorPos(&pt);
        MapWindowPoints(HWND_DESKTOP, lpDrawItem->hwndItem, (LPPOINT)&pt, 1);
        if (PtInRect(&lpDrawItem->rcItem, pt)) is_hot = true;

        std::wstring font_name = AfxGetDefaultFont();
        FontFamily   fontFamily(font_name.c_str());
        REAL fontSize = 10;
        int fontStyle = FontStyleRegular;

        StringAlignment HAlignment = StringAlignmentCenter;
        StringAlignment VAlignment = StringAlignmentCenter;

        // Draw text string or separator

        if (lpDrawItem->itemData == IDC_SIDEMENU_SEPARATOR) {
            int nLeft = AfxScaleX(10);
            int nTop = (int)(nHeight * 0.5f);
            int nRight = lpDrawItem->rcItem.right - AfxScaleX(10);
            int nBottom = nTop ;

            ARGB clrPen = COLOR_SEPARATOR;
            Pen pen(clrPen, 2);
            
            // Paint the full width background using brush 
            SolidBrush back_brush(COLOR_BLACK);
            graphics.FillRectangle(&back_brush, 0, 0, nWidth, nHeight);
            
            // Draw the horizontal line centered taking margins into account
            graphics.DrawLine(&pen, (REAL)nLeft, (REAL)nTop, (REAL)nRight, (REAL)nBottom);
        }
        else {
            // Set some defaults in case there is no valid ListBox line number
            std::wstring text = AfxGetListBoxText(lpDrawItem->hwndItem, lpDrawItem->itemID);

            DWORD back_color = COLOR_BLACK;
            DWORD text_color = COLOR_WHITELIGHT;
            bool is_selected = false;
            if ((lpDrawItem->itemAction | ODA_SELECT) &&
                (lpDrawItem->itemState & ODS_SELECTED)) {
                back_color = COLOR_SELECTION;
                is_selected = true;
            }

            if (lpDrawItem->itemData == IDC_SIDEMENU_CONNECTTWS) {
                back_color = COLOR_BLACK;
                if (tws_IsConnected()) text_color = COLOR_GREEN;
            }

            if (is_hot) {
                back_color = COLOR_SELECTION;
                text_color = COLOR_WHITELIGHT;
            }

            Font         font(&fontFamily, fontSize, fontStyle, Unit::UnitPoint);
            SolidBrush   text_brush(text_color);
            StringFormat stringF(StringFormatFlagsNoWrap);
            stringF.SetAlignment(HAlignment);
            stringF.SetLineAlignment(VAlignment);

            // Paint the full width background using brush 
            SolidBrush back_brush(back_color);
            graphics.FillRectangle(&back_brush, 0, 0, nWidth, nHeight);

            RectF rcText((REAL)0, (REAL)0, (REAL)nWidth, (REAL)nHeight);
            graphics.DrawString(text.c_str(), -1, &font, rcText, &stringF, &text_brush);


            if ((lpDrawItem->itemData == IDC_SIDEMENU_ACTIVETRADES ||
                lpDrawItem->itemData == IDC_SIDEMENU_CLOSEDTRADES ||
                lpDrawItem->itemData == IDC_SIDEMENU_TRANSACTIONS) &&
                is_selected == true) {

                // Create the background brush
                SolidBrush back_brush(COLOR_MENUNOTCH);
                // Need to center the notch vertically
                REAL notch_half_height = AfxScaleY(16) * 0.5f;
                REAL nTop = (nHeight * 0.5f) - notch_half_height;
                PointF point1((REAL)lpDrawItem->rcItem.right, nTop);
                PointF point2((REAL)lpDrawItem->rcItem.right - AfxScaleX(8), nTop + notch_half_height);
                PointF point3((REAL)lpDrawItem->rcItem.right, nTop + (notch_half_height * 2));
                PointF points[3] = { point1, point2, point3 };
                PointF* pPoints = points;
                graphics.FillPolygon(&back_brush, pPoints, 3);
            }

        }

        BitBlt(lpDrawItem->hDC, lpDrawItem->rcItem.left,
            lpDrawItem->rcItem.top, nWidth, nHeight, memDC, 0, 0, SRCCOPY);


        // Cleanup
        RestoreDC(lpDrawItem->hDC, -1);
        if (hbit) DeleteObject(hbit);
        if (memDC) DeleteDC(memDC);
    }
}


// ========================================================================================
// Listbox subclass Window procedure
// ========================================================================================
LRESULT CALLBACK SideMenu_ListBox_SubclassProc(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    // Create static accumulation variable to collect the data from
    // a series of middle mouse wheel scrolls.
    static int accumDelta = 0;

    // Keep track of last index we were over so that we only issue a 
    // repaint if the cursor has moved off of the line
    static long nLastIdx = -1;

    switch (uMsg)
    {

    case WM_MOUSEWHEEL:
    {
        // Accumulate delta until scroll one line (up +120, down -120). 
        // 120 is the Microsoft default delta
        int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        int top_index = (int)SendMessage(hWnd, LB_GETTOPINDEX, 0, 0);
        accumDelta += zDelta;
        if (accumDelta >= 120) {     // scroll up 3 lines
            top_index -= 3;
            top_index = max(0, top_index);
            SendMessage(hWnd, LB_SETTOPINDEX, top_index, 0);
            accumDelta = 0;
        }
        else {
            if (accumDelta <= -120) {     // scroll down 3 lines
                top_index += +3;
                SendMessage(hWnd, LB_SETTOPINDEX, top_index, 0);
                accumDelta = 0;
            }
        }
        HWND hCustomVScrollBar = GetDlgItem(HWND_SIDEMENU, IDC_SIDEMENU_CUSTOMVSCROLLBAR);
        CustomVScrollBar_Recalculate(hCustomVScrollBar);
        return 0;
        break;
    }

    case WM_MOUSEMOVE:
    {
        // Track that we are over the control in order to catch the 
        // eventual WM_MOUSEHOVER and WM_MOUSELEAVE events
        TRACKMOUSEEVENT tme{};
        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        tme.dwFlags = TME_HOVER or TME_LEAVE;
        tme.hwndTrack = hWnd;
        TrackMouseEvent(&tme);

        // Get the item rect that the mouse is over and only invalidate
        // that instead of the entire listbox
        RECT rc{};
        long idx = Listbox_ItemFromPoint(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        // The return value contains the index of the nearest item in the LOWORD. The HIWORD is zero 
        // if the specified point is in the client area of the list box, or one if it is outside the 
        // client area.
        if (HIWORD(idx) == 1) break;
        if (idx != nLastIdx) {
            ListBox_GetItemRect(hWnd, idx, &rc);
            InvalidateRect(hWnd, &rc, true);
            ListBox_GetItemRect(hWnd, nLastIdx, &rc);
            InvalidateRect(hWnd, &rc, true);
            nLastIdx = idx;
        }
    }
    break;


    case WM_MOUSELEAVE:
    {
        nLastIdx = -1;
        AfxRedrawWindow(hWnd);
    }
    break;


    case WM_LBUTTONDOWN:
    {
        int idx = Listbox_ItemFromPoint(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        // The return value contains the index of the nearest item in the LOWORD. The HIWORD is zero 
        // if the specified point is in the client area of the list box, or one if it is outside the 
        // client area.
        if (HIWORD(idx) == 1) break;

        int itemData = (int)ListBox_GetItemData(hWnd, idx);

        // We don't want to "select" connect listbox row or any of the separators.
        if (itemData == IDC_SIDEMENU_CONNECTTWS && tws_IsConnected()) return true;
        if (itemData == IDC_SIDEMENU_SEPARATOR) return true;
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
            Color back_color(COLOR_BLACK);
            SolidBrush back_brush(back_color);
            graphics.FillRectangle(&back_brush, rc.left, rc.top, nWidth, nHeight);
        }

        ValidateRect(hWnd, &rc);
        return TRUE;
        break;

    }


    case WM_DESTROY:
        // REQUIRED: Remove control subclassing
        RemoveWindowSubclass(hWnd, SideMenu_ListBox_SubclassProc, uIdSubclass);
        break;


    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);

}


// ========================================================================================
// Process WM_SIZE message for window/dialog: SideMenu
// ========================================================================================
void SideMenu_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    HWND hListBox = GetDlgItem(hwnd, IDC_SIDEMENU_LISTBOX);
    HWND hCustomVScrollBar = GetDlgItem(hwnd, IDC_SIDEMENU_CUSTOMVSCROLLBAR);

    // Do not call the calcVThumbRect() function during a scrollbar move. This WM_SIZE
    // gets triggered when the ListBox WM_DRAWITEM fires. If we do another calcVThumbRect()
    // calculation then the scrollbar will appear "jumpy" under the user's mouse cursor.
    bool bshow_scrollbar = false;
    CustomVScrollBar* pData = CustomVScrollBar_GetPointer(hCustomVScrollBar);
    if (pData != nullptr) {
        if (pData->drag_active) {
            bshow_scrollbar = true;
        }
        else {
            bshow_scrollbar = pData->calcVThumbRect();
        }
    }
    int custom_scrollbar_width = bshow_scrollbar ? AfxScaleX(CUSTOMVSCROLLBAR_WIDTH) : 0;

    int nLeft = 0;
    int nTop = AfxScaleY(150);
    int nHeight = cy - nTop;
    int nWidth = cx - custom_scrollbar_width;
    SetWindowPos(hListBox, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nLeft = nLeft + nWidth;   // right edge of ListBox
    nWidth = custom_scrollbar_width;
    int show_scrollbar = (bshow_scrollbar ? SWP_SHOWWINDOW : SWP_HIDEWINDOW);
    SetWindowPos(hCustomVScrollBar, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | show_scrollbar);

    AfxRedrawWindow(hListBox);
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: SideMenu
// ========================================================================================
BOOL SideMenu_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    HWND_SIDEMENU = hwnd;

    int nTop = 0;
    int nLeft = 0;
    int nItemHeight = 28;

    HWND hCtl;

    CustomLabel* pData = nullptr;

    // HEADER CONTROLS
    nLeft = (SIDEMENU_WIDTH - 68) / 2;
    hCtl = CreateCustomLabel(
        hwnd,
        IDC_SIDEMENU_LOGO,
        CustomLabelType::image_only,
        nLeft, 20, 68, 68);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->hot_test_enable = false;
        pData->back_color = COLOR_BLACK;
        pData->image_width = 68;
        pData->image_height = 68;
        pData->pImage = LoadImageFromResource(pData->hInst, MAKEINTRESOURCE(IDB_LOGO), L"PNG");
        pData->pImageHot = LoadImageFromResource(pData->hInst, MAKEINTRESOURCE(IDB_LOGO), L"PNG");
        CustomLabel_SetOptions(hCtl, pData);
    }

    nTop = 100;

    hCtl = CreateCustomLabel(
        hwnd,
        IDC_SIDEMENU_APPNAME,
        CustomLabelType::text_only,
        0, nTop, SIDEMENU_WIDTH, 18);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->hot_test_enable = false;
        pData->back_color = COLOR_BLACK;
        pData->text_color = COLOR_WHITELIGHT;
        pData->font_size = 10;
        pData->text_alignment = CustomLabelAlignment::middle_center;
        pData->text = L"IB-Tracker";
        pData->text_hot = pData->text;
        CustomLabel_SetOptions(hCtl, pData);
    }


    nTop += 18;
    hCtl = CreateCustomLabel(
        hwnd,
        IDC_SIDEMENU_APPVERSION,
        CustomLabelType::text_only,
        0, nTop, SIDEMENU_WIDTH, 18);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->hot_test_enable = false;
        pData->back_color = COLOR_BLACK;
        pData->text_color = COLOR_WHITEMEDIUM;
        pData->font_size = 9;
        pData->text_alignment = CustomLabelAlignment::middle_center;
        pData->text = L"v" + version;
        pData->text_hot = pData->text;
        CustomLabel_SetOptions(hCtl, pData);
    }

    // Create an Ownerdraw variable row sized listbox that we will use to custom
    // paint our various side menu options.
    hCtl =
        SideMenu.AddControl(Controls::ListBox, hwnd, IDC_SIDEMENU_LISTBOX, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_HASSTRINGS |
            LBS_OWNERDRAWVARIABLE | LBS_NOTIFY | LBS_WANTKEYBOARDINPUT,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)SideMenu_ListBox_SubclassProc,
            IDC_SIDEMENU_LISTBOX, NULL);

    // Create our custom vertical scrollbar and attach the ListBox to it.
    CreateCustomVScrollBar(hwnd, IDC_SIDEMENU_CUSTOMVSCROLLBAR, hCtl);

    int row_height = AfxScaleY(SIDEMENU_LISTBOX_ROWHEIGHT);
    int row_height_half = (int)(row_height * 0.67f);

    AfxAddListBoxData(hCtl, L"-", IDC_SIDEMENU_SEPARATOR, row_height_half);
    AfxAddListBoxData(hCtl, L"Active Trades", IDC_SIDEMENU_ACTIVETRADES, row_height);
    AfxAddListBoxData(hCtl, L"Closed Trades", IDC_SIDEMENU_CLOSEDTRADES, row_height);
    AfxAddListBoxData(hCtl, L"-", IDC_SIDEMENU_SEPARATOR, row_height_half);
    AfxAddListBoxData(hCtl, L"Shares Trade", IDC_SIDEMENU_NEWSHARESTRADE, row_height);
    AfxAddListBoxData(hCtl, L"Futures Trade", IDC_SIDEMENU_NEWFUTURESTRADE, row_height);
    AfxAddListBoxData(hCtl, L"-", IDC_SIDEMENU_SEPARATOR, row_height_half);
    AfxAddListBoxData(hCtl, L"Custom Options", IDC_SIDEMENU_NEWOPTIONSTRADE, row_height);
    AfxAddListBoxData(hCtl, L"Iron Condor", IDC_SIDEMENU_NEWIRONCONDOR, row_height);
    AfxAddListBoxData(hCtl, L"Short Strangle", IDC_SIDEMENU_NEWSHORTSTRANGLE, row_height);
    AfxAddListBoxData(hCtl, L"Short Put", IDC_SIDEMENU_NEWSHORTPUT, row_height);
    AfxAddListBoxData(hCtl, L"Short Call", IDC_SIDEMENU_NEWSHORTCALL, row_height);
    AfxAddListBoxData(hCtl, L"Short LT112", IDC_SIDEMENU_NEWSHORTLT112, row_height);
    AfxAddListBoxData(hCtl, L"-", IDC_SIDEMENU_SEPARATOR, row_height_half);
    AfxAddListBoxData(hCtl, L"Other Income", IDC_SIDEMENU_OTHERINCOME, row_height);
    AfxAddListBoxData(hCtl, L"Ticker Totals", IDC_SIDEMENU_TICKERTOTALS, row_height);
    AfxAddListBoxData(hCtl, L"Transactions", IDC_SIDEMENU_TRANSACTIONS, row_height);
    AfxAddListBoxData(hCtl, L"Journal Notes", IDC_SIDEMENU_JOURNALNOTES, row_height);
    AfxAddListBoxData(hCtl, L"Trade Plan", IDC_SIDEMENU_TRADEPLAN, row_height);
    AfxAddListBoxData(hCtl, L"-", IDC_SIDEMENU_SEPARATOR, row_height_half);
    AfxAddListBoxData(hCtl, L"Reconcile", IDC_SIDEMENU_RECONCILE, row_height);
    AfxAddListBoxData(hCtl, L"-", IDC_SIDEMENU_SEPARATOR, row_height_half);
    AfxAddListBoxData(hCtl, L"Connect to TWS", IDC_SIDEMENU_CONNECTTWS, row_height);

    return TRUE;
}


// ========================================================================================
// Select the specified selected menu item 
// ========================================================================================
void SideMenu_ExecuteMenuItem(const int itemData)
{
    HWND hListBox = GetDlgItem(HWND_SIDEMENU, IDC_SIDEMENU_LISTBOX);
    int current_selection = 0;

    if (HWND_MIDDLEPANEL == HWND_ACTIVETRADES) current_selection = IDC_SIDEMENU_ACTIVETRADES;
    if (HWND_MIDDLEPANEL == HWND_CLOSEDTRADES) current_selection = IDC_SIDEMENU_CLOSEDTRADES;
    if (HWND_MIDDLEPANEL == HWND_TRANSPANEL) current_selection = IDC_SIDEMENU_TRANSACTIONS;
    
    switch (itemData) {

    case IDC_SIDEMENU_NEWOPTIONSTRADE:
    {
        TradeDialog_Show(TradeAction::new_options_trade);
        SideMenu_SelectMenuItem(HWND_SIDEMENU, current_selection);
    }
    break;

    case IDC_SIDEMENU_NEWSHARESTRADE:
    {
        TradeDialog_Show(TradeAction::new_shares_trade);
        SideMenu_SelectMenuItem(HWND_SIDEMENU, current_selection);
    }
    break;

    case IDC_SIDEMENU_NEWFUTURESTRADE:
    {
        TradeDialog_Show(TradeAction::new_futures_trade);
        SideMenu_SelectMenuItem(HWND_SIDEMENU, current_selection);
    }
    break;

    case IDC_SIDEMENU_NEWIRONCONDOR:
    {
        TradeDialog_Show(TradeAction::new_iron_condor);
        SideMenu_SelectMenuItem(HWND_SIDEMENU, current_selection);
    }
    break;

    case IDC_SIDEMENU_NEWSHORTLT112:
    {
        TradeDialog_Show(TradeAction::new_short_LT112);
        SideMenu_SelectMenuItem(HWND_SIDEMENU, current_selection);
    }
    break;

    case IDC_SIDEMENU_NEWSHORTSTRANGLE:
    {
        TradeDialog_Show(TradeAction::new_short_strangle);
        SideMenu_SelectMenuItem(HWND_SIDEMENU, current_selection);
    }
    break;

    case IDC_SIDEMENU_NEWSHORTPUT:
    {
        TradeDialog_Show(TradeAction::new_short_put);
        SideMenu_SelectMenuItem(HWND_SIDEMENU, current_selection);
    }
    break;

    case IDC_SIDEMENU_NEWSHORTCALL:
    {
        TradeDialog_Show(TradeAction::new_short_call);
        SideMenu_SelectMenuItem(HWND_SIDEMENU, current_selection);
    }
    break;

    case IDC_SIDEMENU_OTHERINCOME:
    {
        TradeDialog_Show(TradeAction::other_income_expense);
        SideMenu_SelectMenuItem(HWND_SIDEMENU, current_selection);
    }
    break;

    case IDC_SIDEMENU_CONNECTTWS:
    {
        // If already connected then don't try to connect again
        if (!tws_IsConnected()) {
            // Prevent multiple clicks of the connect button by waiting until
            // the first click is finished.
            static bool processing_connect_click = false;
            if (processing_connect_click) break;
            processing_connect_click = true;
            bool res = tws_Connect();
            processing_connect_click = false;
        }
        SideMenu_SelectMenuItem(HWND_SIDEMENU, current_selection);

        break;
    }

    case IDC_SIDEMENU_ACTIVETRADES:
    {
        ActiveTrades_ShowActiveTrades(false);
        break;
    }

    case IDC_SIDEMENU_CLOSEDTRADES:
    {
        ClosedTrades_ShowClosedTrades();
        break;
    }

    case IDC_SIDEMENU_TICKERTOTALS:
    {
        TickerPanel_ShowTickerTotals();
        break;
    }

    case IDC_SIDEMENU_TRANSACTIONS:
    {
        TransPanel_ShowTransactions();
        break;
    }

    case IDC_SIDEMENU_RECONCILE:
    {
        tws_PerformReconciliation();
        SideMenu_SelectMenuItem(HWND_SIDEMENU, current_selection);
        break;
    }

    case IDC_SIDEMENU_JOURNALNOTES:
    {
        JournalNotes_ShowJournalNotes();
        break;
    }

    case IDC_SIDEMENU_TRADEPLAN:
    {
        TradePlan_ShowTradePlan();
        break;
    }

    }  // switch itemData

}


// ========================================================================================
// Process WM_COMMAND message for window/dialog: SideMenu
// ========================================================================================
void SideMenu_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (codeNotify)
    {
        case LBN_SELCHANGE:
        {
            int nCurSel = ListBox_GetCurSel(hwndCtl);
            int itemData = (int)ListBox_GetItemData(hwndCtl, nCurSel);
            SideMenu_ExecuteMenuItem(itemData);
        }
    }

}


// ========================================================================================
// SideMenu Window procedure
// ========================================================================================
LRESULT CSideMenu::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(m_hwnd, WM_CREATE, SideMenu_OnCreate);
        HANDLE_MSG(m_hwnd, WM_COMMAND, SideMenu_OnCommand);
        HANDLE_MSG(m_hwnd, WM_SIZE, SideMenu_OnSize);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, SideMenu_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, SideMenu_OnPaint);
        HANDLE_MSG(m_hwnd, WM_MEASUREITEM, SideMenu_OnMeasureItem);
        HANDLE_MSG(m_hwnd, WM_DRAWITEM, SideMenu_OnDrawItem);


        case MSG_TWS_CONNECT_START:
        {
            SetCursor(LoadCursor(0, IDC_WAIT));
            CustomLabel* pData = nullptr;
            pData = CustomLabel_GetOptions(GetDlgItem(m_hwnd, IDC_SIDEMENU_CONNECTTWS));
            if (pData) {
                pData->text = L"Connecting to TWS";
                AfxRedrawWindow(GetDlgItem(m_hwnd, IDC_SIDEMENU_CONNECTTWS));
            }
            return 0;
            break;
        }


        case MSG_TWS_CONNECT_SUCCESS:
        {
            SetCursor(LoadCursor(0, IDC_ARROW));
            CustomLabel* pData = nullptr;
            pData = CustomLabel_GetOptions(GetDlgItem(m_hwnd, IDC_SIDEMENU_CONNECTTWS));
            if (pData) {
                pData->text = L"TWS Connected";
                pData->text_color = COLOR_GREEN;
                pData->text_color_hot = COLOR_GREEN;
                AfxRedrawWindow(GetDlgItem(m_hwnd, IDC_SIDEMENU_CONNECTTWS));
            }
            return 0;
            break;
        }


        case MSG_TWS_CONNECT_WAIT_RECONNECTION:
        {
            SetCursor(LoadCursor(0, IDC_ARROW));
            CustomLabel* pData = nullptr;
            pData = CustomLabel_GetOptions(GetDlgItem(m_hwnd, IDC_SIDEMENU_CONNECTTWS));
            if (pData) {
                pData->text = L"Reconnect Wait";
                pData->text_color = COLOR_RED;
                pData->text_color_hot = COLOR_RED;
                AfxRedrawWindow(GetDlgItem(m_hwnd, IDC_SIDEMENU_CONNECTTWS));
            }
            return 0;
            break;
        }


        case MSG_TWS_WARNING_EXCEPTION:
        {
            SetCursor(LoadCursor(0, IDC_ARROW));
            CustomLabel_SetText(GetDlgItem(HWND_MAINWINDOW, IDC_MAINWINDOW_WARNING),
                L"Monitoring thread exception! Please restart application.");
            ShowWindow(GetDlgItem(HWND_MAINWINDOW, IDC_MAINWINDOW_WARNING), SW_SHOWNORMAL);
            return 0;
            break;
        }


        case MSG_TWS_CONNECT_FAILURE:
        case MSG_TWS_CONNECT_DISCONNECT:
        {
            SetCursor(LoadCursor(0, IDC_ARROW));
            CustomLabel* pData = nullptr;
            pData = CustomLabel_GetOptions(GetDlgItem(m_hwnd, IDC_SIDEMENU_CONNECTTWS));
            if (pData) {
                pData->text = L"Connect to TWS";
                pData->text_color = COLOR_WHITELIGHT;
                pData->text_color_hot = COLOR_WHITELIGHT;
                AfxRedrawWindow(GetDlgItem(m_hwnd, IDC_SIDEMENU_CONNECTTWS));
            }
            tws_EndMonitorThread();
            return 0;
            break;
        }


        default: return DefWindowProc(m_hwnd, msg, wParam, lParam);

    }   // end of switch statement

    return 0;

}
