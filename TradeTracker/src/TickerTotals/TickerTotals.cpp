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
#include "CustomLabel/CustomLabel.h"
#include "CustomTextBox/CustomTextBox.h"
#include "CustomVScrollBar/CustomVScrollBar.h"
#include "Utilities/ListBoxData.h"
#include "Config/Config.h"
#include "MainWindow/MainWindow.h"
#include "ClosedTrades/ClosedTrades.h"

#include "TickerTotals.h"


CTickerPanel TickerPanel;


// ========================================================================================
// Get the HWND's of the the controls on the form.
// ========================================================================================
inline HWND CTickerPanel::TickersListBox() {
    return GetDlgItem(hWindow, IDC_TICKERTOTALS_LISTBOX);
}
inline HWND CTickerPanel::TickersHeader() {
    return GetDlgItem(hWindow, IDC_TICKERTOTALS_HEADER);
}
inline HWND CTickerPanel::SymbolLabel() {
    return GetDlgItem(hWindow, IDC_TICKERTOTALS_SYMBOL);
}
inline HWND CTickerPanel::VScrollBar() {
    return GetDlgItem(hWindow, IDC_TICKERTOTALS_CUSTOMVSCROLLBAR);
}


// ========================================================================================
// Populate the ListBox with the total earnings from each Ticker.
// ========================================================================================
void CTickerPanel::ShowTickerTotals() {
    // Prevent ListBox redrawing until all calculations are completed
    SendMessage(TickersListBox(), WM_SETREDRAW, false, 0);

    // Clear the current table
    ListBoxData_DestroyItemData(TickersListBox());

    // Calculate the amounts per Ticker Symbol
    std::unordered_map<std::wstring, double> mapTicker;
    mapTicker.clear();

    for (const auto& trade : trades) {
        if (trade->ticker_symbol == L"OPENBAL") continue;

        double close_amount = 0;

        for (const auto& trans : trade->transactions) {
            if (trans->underlying == Underlying::Shares ||
                trans->underlying == Underlying::Futures) {
                if (trans->legs.at(0)->action == Action::STC ||
                    trans->legs.at(0)->action == Action::BTC) {

                    int quantity = abs(trans->legs.at(0)->open_quantity);
                    double price = trans->price;

                    if (config.IsFuturesTicker(trade->ticker_symbol)) {
                        double multiplier = AfxValDouble(config.GetMultiplier(trade->ticker_symbol));
                        price *= multiplier;
                    }

                    double diff = 0;
                    if (trans->legs.at(0)->action == Action::STC) {
                        diff = (price + trans->share_average_cost);
                    }
                    if (trans->legs.at(0)->action == Action::BTC) {
                        diff = (trans->share_average_cost - price);
                    }

                    close_amount = quantity * diff;

                    double total = mapTicker[trade->ticker_symbol] + close_amount;
                    mapTicker[trade->ticker_symbol] = total;
                }
            }
        }

        if (!trade->is_open && trade->acb_total) {
            close_amount = trade->acb_total;
            double total = mapTicker[trade->ticker_symbol] + close_amount;
            mapTicker[trade->ticker_symbol] = total;
        }
    }

    struct VecData {
        std::wstring ticker;
        double amount;
    };

    // create a empty vector of VecData
    std::vector<VecData> vec;

    // copy from the map to the vector
    for (const auto& [key, value] : mapTicker) {
        VecData data{ key, value };
        vec.push_back(data);
    }

    // sort the vector largest to smallest based on amount (using a lamda)
    std::sort(vec.begin(), vec.end(),
        [](VecData a, VecData b) {return (a.amount > b.amount); });

    for (const auto& i : vec) {
        ListBoxData_OutputTickerTotals(TickersListBox(), i.ticker, i.amount);
    }

    ListBoxData_AddBlankLine(TickersListBox());

    // Calculate the actual column widths based on the size of the strings in
    // ListBoxData while respecting the minimum values as defined in nMinColWidth[].
    // This function is also called when receiving new price data from TWS because
    // that data may need the column width to be wider.
    ListBoxData_ResizeColumnWidths(TickersListBox(), TableType::ticker_totals);

    // Set the ListBox to the topline.
    ListBox_SetTopIndex(TickersListBox(), 0);

    // Redraw the ListBox to ensure that any recalculated columns are 
    // displayed correctly. Re-enable redraw.
    SendMessage(TickersListBox(), WM_SETREDRAW, true, 0);
    AfxRedrawWindow(TickersListBox());

    // Ensure that the Ticker panel is set
    MainWindow.SetRightPanel(hWindow);

    CustomVScrollBar_Recalculate(VScrollBar());
}


// ========================================================================================
// Header control subclass Window procedure
// ========================================================================================
LRESULT CALLBACK CTickerPanel::Header_SubclassProc(
    HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    switch (uMsg) {

    case WM_ERASEBKGND: {
        return true;
    }

    case WM_PAINT: {
        Header_OnPaint(hwnd);
        return 0;
    }

    case WM_DESTROY: {
        // REQUIRED: Remove control subclassing
        RemoveWindowSubclass(hwnd, (SUBCLASSPROC)Header_SubclassProc, uIdSubclass);
        break;
    }

    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}


// ========================================================================================
// Show the Closed Trades for the selected Ticker line
// ========================================================================================
void CTickerPanel::ShowTickerClosedTrades(HWND hListBox, int idx) {
    // Get the listbox pointer for the selected line.
    ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, idx);
    if (!ld) return;

    // Get the Ticker symbol
    std::wstring ticker_symbol = ld->col[1].text;

    // Show the trade history for the selected trade
    if (ticker_symbol.length()) {
        CustomTextBox_SetText(ClosedTrades.FilterPanel.TickerTextBox(), ticker_symbol);
        ClosedTrades.FilterPanel.ticker_symbol = ticker_symbol;
        ClosedTrades.SetShowTradeDetail(false);
        ClosedTrades.ShowClosedTrades();
    }
}


// ========================================================================================
// Listbox subclass Window procedure
// ========================================================================================
LRESULT CALLBACK CTickerPanel::ListBox_SubclassProc(
    HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    // Create static accumulation variable to collect the data from
    // a series of middle mouse wheel scrolls.
    static int accum_delta = 0;

    switch (uMsg) {

    case WM_MOUSEWHEEL: {
        // Accumulate delta until scroll one line (up +120, down -120). 
        // 120 is the Microsoft default delta
        int zdelta = GET_WHEEL_DELTA_WPARAM(wParam);
        int top_index = (int)SendMessage(hwnd, LB_GETTOPINDEX, 0, 0);
        accum_delta += zdelta;
        if (accum_delta >= 120) {     // scroll up 3 lines
            top_index -= 3;
            top_index = max(0, top_index);
            SendMessage(hwnd, LB_SETTOPINDEX, top_index, 0);
            accum_delta = 0;
        }
        else {
            if (accum_delta <= -120) {     // scroll down 3 lines
                top_index += +3;
                SendMessage(hwnd, LB_SETTOPINDEX, top_index, 0);
                accum_delta = 0;
            }
        }
        CustomVScrollBar_Recalculate(GetDlgItem(GetParent(hwnd), IDC_TICKERTOTALS_CUSTOMVSCROLLBAR));
        return 0;
    }
    
    case WM_LBUTTONDOWN: {
        int idx = Listbox_ItemFromPoint(hwnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        // The return value contains the index of the nearest item in the LOWORD. The HIWORD is zero 
        // if the specified point is in the client area of the list box, or one if it is outside the 
        // client area.
        if (HIWORD(idx) == 1) break;

        // Invoke the Closed Trades and show the selected Ticker transactions for the entire year-to-date.
        SendMessage(GetParent(hwnd), MSG_TICKERTOTALS_SHOWTICKERCLOSEDTRADES, idx, 0);
        break;
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
            SolidBrush back_brush(COLOR_GRAYDARK);
            graphics.FillRectangle(&back_brush, rc.left, rc.top, width, height);
        }

        ValidateRect(hwnd, &rc);
        return true;
    }

    case WM_DESTROY: {
        // Destroy all manually allocated ListBox display data that is held
        // in the LineData structures..
        ListBoxData_DestroyItemData(hwnd);

        // REQUIRED: Remove control subclassing
        RemoveWindowSubclass(hwnd, (SUBCLASSPROC)ListBox_SubclassProc, uIdSubclass);
        break;
    }

    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}


// ========================================================================================
// Process WM_MEASUREITEM message for window/dialog: TickerPanel
// ========================================================================================
void CTickerPanel::OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT* lpMeasureItem) {
    lpMeasureItem->itemHeight = AfxScaleY(TICKERTOTALS_LISTBOX_ROWHEIGHT);
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: TickerPanel
// ========================================================================================
bool CTickerPanel::OnEraseBkgnd(HWND hwnd, HDC hdc) {
    // Handle all of the painting in WM_PAINT
    return true;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: TickerPanel
// ========================================================================================
void CTickerPanel::OnPaint(HWND hwnd) {
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    Graphics graphics(hdc);

    // Create the background brush
    SolidBrush back_brush(COLOR_GRAYDARK);

    // Paint the background using brush.
    int width = (ps.rcPaint.right - ps.rcPaint.left);
    int height = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, width, height);

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Process WM_COMMAND message for window/dialog: TickerPanel
// ========================================================================================
void CTickerPanel::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {
    switch (codeNotify) {

    case LBN_SELCHANGE: {
        int nCurSel = ListBox_GetCurSel(hwndCtl);
        if (nCurSel == -1) break;
        // Invoke the Closed Trades and show the selected Ticker transactions for the entire year-to-date.
        ShowTickerClosedTrades(hwndCtl, nCurSel);
        break;
    }

    }
}


// ========================================================================================
// Process WM_SIZE message for window/dialog: TickerPanel
// ========================================================================================
void CTickerPanel::OnSize(HWND hwnd, UINT state, int cx, int cy) {
    int margin = AfxScaleY(TICKERTOTALS_MARGIN);

    HDWP hdwp = BeginDeferWindowPos(10);

    // Move and size the top label into place
    hdwp = DeferWindowPos(hdwp, SymbolLabel(), 0,
        0, 0, cx, margin, SWP_NOZORDER | SWP_SHOWWINDOW);

    // Do not call the calcVThumbRect() function during a scrollbar move. This WM_SIZE
    // gets triggered when the ListBox WM_DRAWITEM fires. If we do another calcVThumbRect()
    // calcualtion then the scrollbar will appear "jumpy" under the user's mouse cursor.
    bool show_scrollbar = false;
    CustomVScrollBar* pData = CustomVScrollBar_GetPointer(VScrollBar());
    if (pData) {
        if (pData->drag_active) {
            show_scrollbar = true;
        }
        else {
            show_scrollbar = pData->calcVThumbRect();
        }
    }
    int custom_scrollbar_width = show_scrollbar ? AfxScaleX(CUSTOMVSCROLLBAR_WIDTH) : 0;

    int top = margin;
    int left = 0;
    int width = cx;
    int height = AfxScaleY(TICKERTOTALS_LISTBOX_ROWHEIGHT);

    hdwp = DeferWindowPos(hdwp, TickersHeader(), 0, left, top, width, height, SWP_NOZORDER | SWP_SHOWWINDOW);
    top = top + height + AfxScaleX(1);

    width = cx - custom_scrollbar_width;
    height = cy - top;
    hdwp = DeferWindowPos(hdwp, TickersListBox(), 0, left, top, width, height, SWP_NOZORDER | SWP_SHOWWINDOW);

    left = left + width;   // right edge of ListBox
    width = custom_scrollbar_width;
    hdwp = DeferWindowPos(hdwp, VScrollBar(), 0, left, top, width, height,
        SWP_NOZORDER | (show_scrollbar ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));

    EndDeferWindowPos(hdwp);
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: TickerPanel
// ========================================================================================
bool CTickerPanel::OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct) {
    hWindow = hwnd;

    HWND hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TICKERTOTALS_SYMBOL, L"Ticker Totals",
        COLOR_WHITELIGHT, COLOR_BLACK);

    // Create an Ownerdraw listbox that we will use to custom paint ticker names.
    hCtl =
        TickerPanel.AddControl(Controls::ListBox, hwnd, IDC_TICKERTOTALS_LISTBOX, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWFIXED | LBS_NOTIFY,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)ListBox_SubclassProc,
            IDC_TICKERTOTALS_LISTBOX, NULL);
    ListBox_AddString(hCtl, NULL);

    // Create our custom vertical scrollbar and attach the ListBox to it.
    CreateCustomVScrollBar(hwnd, IDC_TICKERTOTALS_CUSTOMVSCROLLBAR, hCtl, Controls::ListBox);

    // Create Header control for our Ticker Totals output
    hCtl = TickerPanel.AddControl(Controls::Header, hwnd, IDC_TICKERTOTALS_HEADER,
        L"", 0, 0, 0, 0, -1, -1, NULL, (SUBCLASSPROC)Header_SubclassProc,
        IDC_TICKERTOTALS_HEADER, NULL);
    Header_InsertNewItem(hCtl, 0, AfxScaleX(nTickerTotalsMinColWidth[0]), L"", HDF_LEFT);
    Header_InsertNewItem(hCtl, 1, AfxScaleX(nTickerTotalsMinColWidth[1]), L"Ticker", HDF_LEFT);
    Header_InsertNewItem(hCtl, 2, AfxScaleX(nTickerTotalsMinColWidth[2]), L"Company Name", HDF_LEFT);
    Header_InsertNewItem(hCtl, 3, AfxScaleX(nTickerTotalsMinColWidth[3]), L"Amount", HDF_RIGHT);

    return true;
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CTickerPanel::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        HANDLE_MSG(m_hwnd, WM_CREATE, OnCreate);
        HANDLE_MSG(m_hwnd, WM_COMMAND, OnCommand);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, OnPaint);
        HANDLE_MSG(m_hwnd, WM_SIZE, OnSize);
        HANDLE_MSG(m_hwnd, WM_MEASUREITEM, OnMeasureItem);
        HANDLE_MSG(m_hwnd, WM_DRAWITEM, ListBoxData_OnDrawItem);

    case MSG_TICKERTOTALS_SHOWTICKERCLOSEDTRADES: {
        ShowTickerClosedTrades(TickersListBox(), (int)wParam);
        return 0;
    }

    }
    return DefWindowProc(m_hwnd, msg, wParam, lParam);
}

