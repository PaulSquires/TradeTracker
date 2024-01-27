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
#include "Utilities/ListBoxData.h"
#include "Category/Category.h"
#include "MainWindow/MainWindow.h"
#include "CustomVScrollBar/CustomVScrollBar.h"
#include "Transactions/TransDetail.h"
#include "TradeHistory/TradeHistory.h"
#include "ActiveTrades/ActiveTrades.h"
#include "TickerTotals/TickerTotals.h"
#include "TabPanel/TabPanel.h"
#include "Database/trade.h"
#include "Config/Config.h"
#include "ClosedTrades.h"


CClosedTrades ClosedTrades;

bool show_trade_detail = true;


// ========================================================================================
// Get the HWND's of the the controls on the form.
// ========================================================================================
inline HWND CClosedTrades::TradesListBox() {
    return GetDlgItem(hWindow, IDC_CLOSEDTRADES_LISTBOX);
}
inline HWND CClosedTrades::VScrollBar() {
    return GetDlgItem(hWindow, IDC_CLOSEDTRADES_CUSTOMVSCROLLBAR);
}
inline HWND CClosedTrades::TradesHeader() {
    return GetDlgItem(hWindow, IDC_CLOSEDTRADES_HEADER);
}


// ========================================================================================
// Set flag to enable/disable showing the Trade Detail when line in the listbox
// is clicked on. This is useful when TickerTotals is shown and we do want the 
// TickerTotals list to remain shown.
// ========================================================================================
void CClosedTrades::SetShowTradeDetail(bool enable) {
    show_trade_detail = enable;
}


// ========================================================================================
// Central function that actually selects and displays the incoming ListBox index item.
// ========================================================================================
void CClosedTrades::ShowListBoxItem(int index) {
    if (!show_trade_detail && (MainWindow.hRightPanel == TickerPanel.hWindow)) return;

    ListBox_SetCurSel(TradesListBox(), index);

    // Ensure that the ClosedTrades menu item is selected
    TabPanel_SelectPanelItem(HWND_TABPANEL, IDC_TABPANEL_CLOSEDTRADES);

    //  update the scrollbar position if necessary
    CustomVScrollBar_Recalculate(VScrollBar());

    // Get the current line to determine if a valid Trade pointer exists so that we
    // can show the trade history.
    if (index > -1) {
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(TradesListBox(), index);
        if (ld != nullptr)
            TradeHistory_ShowTradesHistoryTable(ld->trade);
    }

    SetFocus(TradesListBox());
}


// ========================================================================================
// Populate the ListBox with the closed trades
// ========================================================================================
void CClosedTrades::ShowClosedTrades() {

    // Select the correct menu panel item
    TabPanel_SelectPanelItem(HWND_TABPANEL, IDC_TABPANEL_CLOSEDTRADES);

    // Ensure that the Closed panel is set
    MainWindow.SetLeftPanel(hWindow);

    // Prevent ListBox redrawing until all calculations are completed.
    SendMessage(TradesListBox(), WM_SETREDRAW, false, 0);

    std::wstring start_date = FilterPanel.filter_start_date;
    std::wstring end_date = FilterPanel.filter_end_date;
    std::wstring ticker = FilterPanel.ticker_symbol;
    int selected_category = FilterPanel.selected_category;
    
    struct ClosedData {
        std::shared_ptr<Trade> trade;
        std::shared_ptr<Transaction> trans;
        std::wstring description;
        std::wstring closed_date;
        double close_amount = 0;

    };

    std::vector<ClosedData> vectorClosed;
    vectorClosed.reserve(1000);         // reserve space for 1000 closed trades
    
    // Look at all the closed trades as well as open trades that have shares/futures

    for (auto& trade : trades) {

        if (ticker.length() > 0) {
            if (ticker != trade->ticker_symbol) continue;
        }

        if (selected_category != CATEGORY_ALL) {
            if (trade->category != selected_category) continue;
        }

        // Iterate to find the latest closed date
        std::wstring latest_closed_date;
        for (auto& trans : trade->transactions) {
            if (trans->trans_date > latest_closed_date) {
                latest_closed_date = trans->trans_date;
            }
        }

        if (latest_closed_date < start_date || latest_closed_date > end_date) continue;

        // If this Trade has Shares/Futures transactions showing the costing would have been created
        // during the CalculateAdjustedCostBase function. 
        for (const auto& share : trade->shares_history) {
            if (share.leg_action == Action::STC || share.leg_action == Action::BTC) {

                ClosedData data;
                data.trade = trade;
                //data.trans = trans;
                data.closed_date = share.trans->trans_date;

                int quantity = abs(share.open_quantity);
                double price = share.trans->price;

                if (config.IsFuturesTicker(trade->ticker_symbol)) {
                    double multiplier = AfxValDouble(config.GetMultiplier(trade->ticker_symbol));
                    price *= multiplier;
                }

                std::wstring diff_describe;
                double diff = 0;
                if (share.leg_action == Action::STC) {
                    diff = (price + share.average_cost);
                    diff_describe = AfxMoney(price, true, 2) + L"-" + AfxMoney(abs(share.average_cost), true, 2);
                }
                if (share.leg_action == Action::BTC) {
                    diff = (share.average_cost - price);
                    diff_describe = AfxMoney(abs(share.average_cost), true, 2) + L"-" + AfxMoney(price, true, 2);
                }

                data.close_amount = quantity * diff;

                std::wstring describe = (share.trans->underlying == Underlying::Shares) ? L" shares @ $" : L" futures @ $";
                data.description = std::to_wstring(quantity) + describe + AfxMoney(diff, true, 2) +
                    L" (" + diff_describe + L")";

                vectorClosed.push_back(data);
            }

        }

        if (!trade->is_open && trade->acb_non_shares) {
            ClosedData data;
            data.trade = trade;
            data.trans = nullptr;
            data.closed_date = latest_closed_date;
            data.close_amount = trade->acb_non_shares;
            data.description = trade->ticker_name;
            if (config.IsFuturesTicker(trade->ticker_symbol)) data.description += L" (" + AfxFormatFuturesDate(trade->future_expiry) + L")";
            vectorClosed.push_back(data);
        }
    }

    // Destroy any existing ListBox line data
    ListBoxData_DestroyItemData(TradesListBox());

    // Sort the closed vector based on trade closed date
    std::sort(vectorClosed.begin(), vectorClosed.end(),
        [](const ClosedData data1, const ClosedData data2) {
            return (data1.closed_date > data2.closed_date) ? true : false;
        });

    // Output the closed trades and subtotals based on month when the month changes.
    double subtotal_amount = 0;
    double YTD = 0;
    double monthly_amount = 0;
    double weekly_amount = 0;
    double daily_amount = 0;
    
    int subtotal_month = 0;
    
    int current_month = 0;
    int current_year = 0;
    int current_month_win = 0;
    int current_month_loss = 0;

    int month_win = 0;
    int month_loss = 0;

    int week_win = 0;
    int week_loss = 0;

    int day_win = 0;
    int day_loss = 0;

    int year_win = 0;
    int year_loss = 0;

    std::wstring today_date = AfxCurrentDate();
    int today_year = AfxGetYear(today_date);
    std::wstring week_start_date = AfxDateAddDays(today_date, -AfxLocalDayOfWeek());
    std::wstring week_end_date = AfxDateAddDays(week_start_date, 6);
    std::wstring current_date = L"";

    for (const auto& ClosedData : vectorClosed) {
        current_month = AfxGetMonth(ClosedData.closed_date);
        current_year = AfxGetYear(ClosedData.closed_date);
        if (subtotal_month != current_month && subtotal_month != 0) {
            ListBoxData_OutputClosedMonthSubtotal(TradesListBox(), current_date, subtotal_amount, current_month_win, current_month_loss);
            current_date = ClosedData.closed_date;
            subtotal_amount = 0;
            current_month_win = 0;
            current_month_loss = 0;
        }
        current_date = ClosedData.closed_date;
        subtotal_month = current_month;
        subtotal_amount += ClosedData.close_amount;
        if (ClosedData.close_amount >= 0) ++current_month_win;
        if (ClosedData.close_amount < 0) ++current_month_loss;

        if (current_month == AfxGetMonth(today_date) &&
            current_year == AfxGetYear(today_date)) {
            monthly_amount += ClosedData.close_amount;
            if (ClosedData.close_amount >= 0) ++month_win;
            if (ClosedData.close_amount < 0) ++month_loss;
        }

        if (current_date >= week_start_date && current_date <= week_end_date) {
            weekly_amount += ClosedData.close_amount;
            if (ClosedData.close_amount >= 0) ++week_win;
            if (ClosedData.close_amount < 0) ++week_loss;
        }

        if (current_date == today_date) {
            daily_amount += ClosedData.close_amount;
            if (ClosedData.close_amount >= 0) ++day_win;
            if (ClosedData.close_amount < 0) ++day_loss;
        }

        if (today_year == AfxGetYear(current_date)) {
            if (ClosedData.close_amount >= 0) ++year_win;
            if (ClosedData.close_amount < 0) ++year_loss;
            YTD += ClosedData.close_amount;
        }
        ListBoxData_OutputClosedPosition(TradesListBox(), ClosedData.trade, 
            ClosedData.closed_date, ClosedData.trade->ticker_symbol, ClosedData.description, ClosedData.close_amount);
    }
    if (subtotal_amount != 0) {
        ListBoxData_OutputClosedMonthSubtotal(TradesListBox(), current_date, subtotal_amount, current_month_win, current_month_loss);
    }

    ListBoxData_OutputClosedYearTotal(TradesListBox(), today_year, YTD, year_win, year_loss);
    ListBoxData_OutputClosedMonthTotal(TradesListBox(), monthly_amount, month_win, month_loss);
    ListBoxData_OutputClosedWeekTotal(TradesListBox(), weekly_amount, week_win, week_loss);
    ListBoxData_OutputClosedDayTotal(TradesListBox(), daily_amount, day_win, day_loss);


    // Calculate the actual column widths based on the size of the strings in
    // ListBoxData while respecting the minimum values as defined in nMinColWidth[].
    // This function is also called when receiving new price data from TWS because
    // that data may need the column width to be wider.
    ListBoxData_ResizeColumnWidths(TradesListBox(), TableType::closed_trades);

    // If no closed trades exist then add at least one line
    if (ListBox_GetCount(TradesListBox()) == 0) {
        ListBoxData_AddBlankLine(TradesListBox());
    }

    CustomVScrollBar_Recalculate(VScrollBar());

    // Select row past the DAY total line if possible
    int curSel = min(ListBox_GetCount(TradesListBox())-1, 5);
    ShowListBoxItem(curSel);  

    // Redraw the ListBox to ensure that any recalculated columns are 
    // displayed correctly. Re-enable redraw.
    SendMessage(TradesListBox(), WM_SETREDRAW, true, 0);
    AfxRedrawWindow(TradesListBox());
}


// ========================================================================================
// Header control subclass Window procedure
// ========================================================================================
LRESULT CALLBACK CClosedTrades::Header_SubclassProc(
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
        RemoveWindowSubclass(hwnd, Header_SubclassProc, uIdSubclass);
        break;
    }

    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}


// ========================================================================================
// Listbox subclass Window procedure
// ========================================================================================
LRESULT CALLBACK CClosedTrades::ListBox_SubclassProc(
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
        HWND hCustomVScrollBar = GetDlgItem(GetParent(hwnd), IDC_CLOSEDTRADES_CUSTOMVSCROLLBAR);
        CustomVScrollBar_Recalculate(hCustomVScrollBar);
        return 0;
    }

    case WM_LBUTTONDOWN: {
        int idx = Listbox_ItemFromPoint(hwnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        // The return value contains the index of the nearest item in the LOWORD. The HIWORD is zero 
        // if the specified point is in the client area of the list box, or one if it is outside the 
        // client area.
        if (HIWORD(idx) == 1) break;

        SendMessage(GetParent(hwnd), MSG_CLOSEDTRADES_SETSHOWTRADEDETAIL, true, 0);
        SendMessage(GetParent(hwnd), MSG_CLOSEDTRADES_SHOWLISTBOXITEM, idx, 0);
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
        RemoveWindowSubclass(hwnd, ListBox_SubclassProc, uIdSubclass);
        break;
    }

    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}


// ========================================================================================
// Process WM_MEASUREITEM message for window/dialog: ClosedTrades
// ========================================================================================
void CClosedTrades::OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT* lpMeasureItem) {
    lpMeasureItem->itemHeight = AfxScaleY(CLOSED_TRADES_LISTBOX_ROWHEIGHT);
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: ClosedTrades
// ========================================================================================
bool CClosedTrades::OnEraseBkgnd(HWND hwnd, HDC hdc) {
    // Handle all of the painting in WM_PAINT
    return true;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: ClosedTrades
// ========================================================================================
void CClosedTrades::OnPaint(HWND hwnd) {
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    Graphics graphics(hdc);

    Color back_color = COLOR_BLACK;

    // Create the background brush
    SolidBrush back_brush(back_color);

    // Paint the background using brush.
    int width = (ps.rcPaint.right - ps.rcPaint.left);
    int height = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, width, height);

    // Paint the area to the left of the ListBox in order to give the illusion
    // of a margin before the ListBox data is displyed.
    ps.rcPaint.top += AfxScaleY(CLOSEDTRADES_MARGIN); 

    // Set the background brush
    back_color.SetValue(COLOR_GRAYDARK);
    back_brush.SetColor(back_color);
    width = (ps.rcPaint.right - ps.rcPaint.left);
    height = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, width, height);

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Process WM_SIZE message for window/dialog: ClosedTrades
// ========================================================================================
void CClosedTrades::OnSize(HWND hwnd, UINT state, int cx, int cy) {

    // Do not call the calcVThumbRect() function during a scrollbar move. This WM_SIZE
    // gets triggered when the ListBox WM_DRAWITEM fires. If we do another calcVThumbRect()
    // calcualtion then the scrollbar will appear "jumpy" under the user's mouse cursor.
    bool bshow_scrollbar = false;
    CustomVScrollBar* pData = CustomVScrollBar_GetPointer(VScrollBar());
    if (pData) {
        if (pData->drag_active) {
            bshow_scrollbar = true;
        }
        else {
            bshow_scrollbar = pData->calcVThumbRect();
        }
    }
    int custom_scrollbar_width = bshow_scrollbar ? AfxScaleX(CUSTOMVSCROLLBAR_WIDTH) : 0;

    int margin = AfxScaleY(CLOSEDTRADES_MARGIN);
    int left = AfxScaleX(APP_LEFTMARGIN_WIDTH);
    int top = 0;
    int width = cx;
    int height = 0;

    HDWP hdwp = BeginDeferWindowPos(10);

    hdwp = DeferWindowPos(hdwp, FilterPanel.hWindow, 0, left, top, width, FilterPanel.fixed_height, SWP_NOZORDER | SWP_SHOWWINDOW);

    left = AfxScaleX(APP_LEFTMARGIN_WIDTH);
    top = margin;
    width = cx - left;
    height = AfxScaleY(CLOSED_TRADES_LISTBOX_ROWHEIGHT);
    hdwp = DeferWindowPos(hdwp, TradesHeader(), 0, left, top, width, height, SWP_NOZORDER | SWP_SHOWWINDOW);
    top = top + height + AfxScaleY(1);

    width = cx - left - custom_scrollbar_width;
    height = cy - top;
    hdwp = DeferWindowPos(hdwp, TradesListBox(), 0, left, top, width, height, SWP_NOZORDER | SWP_SHOWWINDOW);

    left = left + width;   // right edge of ListBox
    width = custom_scrollbar_width;
    hdwp = DeferWindowPos(hdwp, VScrollBar(), 0, left, top, width, height,
        SWP_NOZORDER | (bshow_scrollbar ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));

    EndDeferWindowPos(hdwp);
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: ClosedTrades
// ========================================================================================
bool CClosedTrades::OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct) {
    hWindow = hwnd;

    // Add the top Filter Panel
    HWND hCtl = FilterPanel.CreateFilterPanel(hwnd);

    hCtl = ClosedTrades.AddControl(Controls::Header, hwnd, IDC_CLOSEDTRADES_HEADER, L"",
        0, 0, 0, 0, -1, -1, NULL, (SUBCLASSPROC)Header_SubclassProc,
        IDC_CLOSEDTRADES_HEADER, NULL);
    Header_InsertNewItem(hCtl, 0, AfxScaleX(nClosedMinColWidth[0]), L"", HDF_CENTER);
    Header_InsertNewItem(hCtl, 1, AfxScaleX(nClosedMinColWidth[1]), L"Date", HDF_LEFT);
    Header_InsertNewItem(hCtl, 2, AfxScaleX(nClosedMinColWidth[2]), L"Ticker", HDF_LEFT);
    Header_InsertNewItem(hCtl, 3, AfxScaleX(nClosedMinColWidth[3]), L"Description", HDF_LEFT);
    Header_InsertNewItem(hCtl, 4, AfxScaleX(nClosedMinColWidth[4]), L"Amount", HDF_RIGHT);
    // Must turn off Window Theming for the control in order to correctly apply colors
    SetWindowTheme(hCtl, L"", L"");

    // Create an Ownerdraw fixed row sized listbox that we will use to custom
    // paint our various closed trades.
    hCtl =
        ClosedTrades.AddControl(Controls::ListBox, hwnd, IDC_CLOSEDTRADES_LISTBOX, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWFIXED | LBS_NOTIFY,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)ListBox_SubclassProc,
            IDC_CLOSEDTRADES_LISTBOX, NULL);
    ListBox_AddString(hCtl, NULL);

    // Create our custom vertical scrollbar and attach the ListBox to it.
    CreateCustomVScrollBar(hwnd, IDC_CLOSEDTRADES_CUSTOMVSCROLLBAR, hCtl, Controls::ListBox);

    return true;
}


// ========================================================================================
// Process WM_COMMAND message for window/dialog: ClosedTrades
// ========================================================================================
void CClosedTrades::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {
    switch (codeNotify) {

    case LBN_SELCHANGE: {
        int selected = ListBox_GetCurSel(hwndCtl);
        if (selected == -1) break;
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hwndCtl, selected);
        if (ld) {
            // Show the trade history for the selected trade
            SetShowTradeDetail(true);
            ShowListBoxItem(selected);
        }
        break;
    }

    }
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CClosedTrades::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        HANDLE_MSG(m_hwnd, WM_CREATE, OnCreate);
        HANDLE_MSG(m_hwnd, WM_COMMAND, OnCommand);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, OnPaint);
        HANDLE_MSG(m_hwnd, WM_SIZE, OnSize);
        HANDLE_MSG(m_hwnd, WM_MEASUREITEM, OnMeasureItem);
        HANDLE_MSG(m_hwnd, WM_DRAWITEM, ListBoxData_OnDrawItem);

    case MSG_DATEPICKER_DATECHANGED: {
        // Received from FilterPanel to indicate that we need to refresh our ClosedTrades grid.
        ShowClosedTrades();
        return 0;
    }

    case MSG_CATEGORY_CATEGORYCHANGED: {
        // Categories have change so update the Closed Trades list.
        SetShowTradeDetail(true);
        ShowClosedTrades();

        // Also need to update Active Trades list because Category headers have changed.
        AfxRedrawWindow(GetDlgItem(ActiveTrades.hWindow, IDC_ACTIVETRADES_LISTBOX));
        return 0;
    }

    case MSG_CLOSEDTRADES_SETSHOWTRADEDETAIL: {
        SetShowTradeDetail((bool)wParam);
        return 0;
    }

    case MSG_CLOSEDTRADES_SHOWLISTBOXITEM: {
        ShowListBoxItem((int)wParam);
        return 0;
    } 

    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}

