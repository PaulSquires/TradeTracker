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
#include "MainWindow/tws-client.h"
#include "CustomLabel/CustomLabel.h"
#include "Utilities/ListBoxData.h"
#include "SideMenu/SideMenu.h"
#include "MainWindow/MainWindow.h"
#include "CustomVScrollBar/CustomVScrollBar.h"
#include "TradeDialog/TradeDialog.h"
#include "TradeHistory/TradeHistory.h"
#include "Database/trade.h"
#include "Category/Category.h"
#include "Config/Config.h"
#include "ActiveTrades.h"


HWND HWND_ACTIVETRADES = NULL;

CActiveTrades ActiveTrades;


//
// Perform the ITM calculation and update the Trade pointer with the values.
//
void PerformITMcalculation(std::shared_ptr<Trade>& trade)
{

    bool is_itm_red = false;
    bool is_itm_green = false;
    bool is_long_spread = false;

    for (const auto& leg : trade->open_legs) {
        if (leg->underlying == L"OPTIONS") {
            if (leg->PutCall == L"P") {
                if (trade->ticker_last_price < AfxValDouble(leg->strike_price)) {
                    if (leg->open_quantity < 0) {
                        is_itm_red = (is_long_spread == true) ? false : true;
                    }
                    if (leg->open_quantity > 0) {
                        is_itm_green = true; is_itm_red = false; is_long_spread = true;
                    }
                }
            }
            else if (leg->PutCall == L"C") {
                if (trade->ticker_last_price > AfxValDouble(leg->strike_price)) {
                    if (leg->open_quantity < 0) {
                        is_itm_red = (is_long_spread == true) ? false : true;
                    }
                    if (leg->open_quantity > 0) {
                        is_itm_green = true; is_itm_red = false; is_long_spread = true;
                    }
                }
            }
        }
    }

    std::wstring text = L"";

    DWORD theme_color = COLOR_WHITELIGHT;
    if (is_itm_red) {
        text = L"ITM";
        theme_color = COLOR_RED;
    }
    if (is_itm_green) {
        text = L"ITM";
        theme_color = COLOR_GREEN;
    }

    trade->itm_text = text;
    trade->itm_color = theme_color;

}


//
// Update the ActiveTrades list with the most up to dat Market Price Data.
// This function is called from the TickerUpdateFunction() thread.
//
void ActiveTrades_UpdateTickerPrices()
{
    HWND hListBox = GetDlgItem(HWND_ACTIVETRADES, IDC_TRADES_LISTBOX);

    if (!IsWindow(hListBox)) return;

    int item_count = ListBox_GetCount(hListBox);
    if (item_count == 0) return;

    // These columns in the table are updated in real time when connected
    // to TWS. The LineData pointer is updated via a call to SetColumnData
    // and the correct ListBox line is invalidated/redrawn in order to force
    // display of the new price data. 

    for (int index = 0; index < item_count; index++) {

        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, index);
        if (ld == (void*)-1) continue;
        if (ld == nullptr) continue;
        if (ld->tickerId == -1) continue;
        if (ld->line_type != LineType::ticker_line) continue;

        // Lookup the most recent Market Price data
        TickerData td{};
        if (mapTickerData.count(ld->tickerId)) {
            td = mapTickerData.at(ld->tickerId);
        }

        // If the price has not changed since last update then skip
        if (ld->trade->ticker_last_price == td.last_price &&
            ld->trade->ticker_close_price == td.close_price) {
            continue;
        }

        ld->trade->ticker_last_price = td.last_price;
        ld->trade->ticker_close_price = td.close_price;

        // Calculate the price change
        double delta = 0;
        if (ld->trade->ticker_close_price != 0) {
            delta = (ld->trade->ticker_last_price - ld->trade->ticker_close_price);
        }

        std::wstring text = L"";
        DWORD theme_color = COLOR_WHITELIGHT;

        // Calculate if any of the option legs are ITM in a good (green) or bad (red) way.
        // We use a separate function call because scrapped data will need acces to the 
        // ITM calculation also.
        PerformITMcalculation(ld->trade);

        ld->SetTextData(COLUMN_TICKER_ITM, ld->trade->itm_text, ld->trade->itm_color);  // ITM


        text = AfxMoney(delta, true, ld->trade->ticker_decimals);
        ld->trade->ticker_change_text = text;
        theme_color = (delta >= 0) ? COLOR_GREEN : COLOR_RED;
        ld->trade->ticker_change_color = theme_color;
        ld->SetTextData(COLUMN_TICKER_CHANGE, text, theme_color);  // price change

        text = AfxMoney(ld->trade->ticker_last_price, false, ld->trade->ticker_decimals);
        ld->trade->ticker_last_price_text = text;
        ld->SetTextData(COLUMN_TICKER_CURRENTPRICE, text, COLOR_WHITELIGHT);  // current price

        text = (delta >= 0 ? L"+" : L"") + AfxMoney((delta / ld->trade->ticker_last_price) * 100, true) + L"%";
        ld->trade->ticker_percent_change_text = text;
        theme_color = (delta >= 0) ? COLOR_GREEN : COLOR_RED;
        ld->trade->ticker_percent_change_color = theme_color;
        ld->SetTextData(COLUMN_TICKER_PERCENTCHANGE, text, theme_color);  // price percentage change


        RECT rc{};
        ListBox_GetItemRect(hListBox, index, &rc);
        InvalidateRect(hListBox, &rc, TRUE);
        UpdateWindow(hListBox);

    }  // for

    // Do calculation to ensure column widths are wide enough to accommodate the new
    // price data that has just arrived.
    if (ListBoxData_ResizeColumnWidths(hListBox, TableType::active_trades, -1) == true) {
        AfxRedrawWindow(hListBox);
    }
}


// ========================================================================================
// Returns True/False if incoming Trade action is consider a "New" Options type of action.
// ========================================================================================
bool IsNewOptionsTradeAction(TradeAction action)
{
    switch (action)
    {
    case TradeAction::new_options_trade:
    case TradeAction::new_iron_condor:
    case TradeAction::new_short_LT112:
    case TradeAction::new_short_strangle:
    case TradeAction::new_short_put:
    case TradeAction::new_short_call:
        return true;
    default:
        return false;
    }
}


// ========================================================================================
// Returns True/False if incoming Trade action is consider a "New" Shares/Futures 
// type of action.
// ========================================================================================
bool IsNewSharesTradeAction(TradeAction action)
{
    switch (action)
    {
    case TradeAction::new_shares_trade:
    case TradeAction::new_futures_trade:
        return true;
    default:
        return false;
    }
}


// ========================================================================================
// Central function that actually selects and displays the incoming ListBox index item.
// ========================================================================================
void ActiveTrades_ShowListBoxItem(int index)
{
    HWND hListBox = GetDlgItem(HWND_ACTIVETRADES, IDC_TRADES_LISTBOX);
    HWND hCustomVScrollBar = GetDlgItem(HWND_ACTIVETRADES, IDC_TRADES_CUSTOMVSCROLLBAR);

    // Ensure that the ActiveTrades menu item is selected
    SideMenu_SelectMenuItem(HWND_SIDEMENU, IDC_SIDEMENU_ACTIVETRADES);

    //  update the scrollbar position if necessary
    CustomVScrollBar_Recalculate(hCustomVScrollBar);

    // Get the current line to determine if a valid Trade pointer exists so that we
    // can show the trade history.
    if (index > -1) {
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, index);
        if (ld != nullptr)
            TradeHistory_ShowTradesHistoryTable(ld->trade);
    }

    SetFocus(hListBox);
}


// ========================================================================================
// Populate the Trades ListBox with the current active/open trades
// ========================================================================================
void ActiveTrades_ShowActiveTrades(const bool bForceReload)
{
    HWND hListBox = GetDlgItem(HWND_ACTIVETRADES, IDC_TRADES_LISTBOX);
    HWND hCustomVScrollBar = GetDlgItem(HWND_ACTIVETRADES, IDC_TRADES_CUSTOMVSCROLLBAR);
    HWND hLabel = GetDlgItem(HWND_ACTIVETRADES, IDC_TRADES_LABEL);

    static bool positions_requested = false;
    static int tickerId = 100;
    int curSel = 0;

    // Select the correct menu panel item
    SideMenu_SelectMenuItem(HWND_SIDEMENU, IDC_SIDEMENU_ACTIVETRADES);

    CustomLabel_SetText(hLabel, L"Active Trades");

    // Ensure that the Trades panel is set
    MainWindow_SetMiddlePanel(HWND_ACTIVETRADES);


    // Determine if we need to initialize the listbox
    if (bForceReload == true && trades.size() != 0) {

        // Prevent ListBox redrawing until all calculations are completed
        SendMessage(hListBox, WM_SETREDRAW, FALSE, 0);

        // In case of newly added/deleted data ensure data is sorted.
        // Sort based on Category and then TickerSymbol
        std::sort(trades.begin(), trades.end(),
            [](const auto trade1, const auto trade2) {
                {
                    if (trade1->category < trade2->category) return true;
                    if (trade2->category < trade1->category) return false;

                    // a=b for primary condition, go to secondary
                    if (trade1->ticker_symbol < trade2->ticker_symbol) return true;
                    if (trade2->ticker_symbol < trade1->ticker_symbol) return false;

                    return false;
                } 
            });


        // Destroy any existing ListBox line data (clear the LineData pointers)
        ListBoxData_DestroyItemData(GetDlgItem(HWND_ACTIVETRADES, IDC_TRADES_LISTBOX));


        // Create the new ListBox line data and initiate the new market data.
        int category_header = -1;
        for (auto& trade : trades) {
            // We are displaying only open trades 
            if (trade->is_open) {
                // Set the decimals for this tickerSymbol. Most will be 2 but futures can have a lot more.
                trade->ticker_decimals = GetTickerDecimals(trade->ticker_symbol);

                if (trade->category != category_header) {
                    ListBoxData_AddCategoryHeader(hListBox, trade);
                    category_header = trade->category;
                }

                // Setup the line, assign the tickerId.
                TickerId id = (trade->tickerId == -1) ? tickerId++ : trade->tickerId;

                ListBoxData_OpenPosition(hListBox, trade, id);
                trade->tickerId = id;
            }
        }


        // Start getting market data (ticker price data) for each active ticker.
        // If the ticker has already requested market data then it will skip requesting
        // it again for a duplicate time.
        tws_RequestMarketUpdates();


        // When requestPositions completes, it sends a notification to the Active Trades
        // window that it is now okay to request the Portfolio Updates. We make those
        // portfolio update calls there rather than here.
        if (tws_IsConnected() == true && positions_requested == false) {
            tws_RequestPositions();
            positions_requested = true;
        }


        // Calculate the actual column widths based on the size of the strings in
        // ListBoxData while respecting the minimum values as defined in nMinColWidth[].
        // This function is also called when receiving new price data from TWS because
        // that data may need the column width to be wider.
        ListBoxData_ResizeColumnWidths(hListBox, TableType::active_trades, -1);


        // Redraw the ListBox to ensure that any recalculated columns are 
        // displayed correctly. Re-enable redraw.
        SendMessage(hListBox, WM_SETREDRAW, TRUE, 0);
        AfxRedrawWindow(hListBox);

    }


    // If no Trades exist then add simple message to user to add Trade
    if (trades.size() == 0) {
        ListBox_ResetContent(hListBox);
        ListBoxData_NoTradesExistMessage(hListBox);
        ListBoxData_ResizeColumnWidths(hListBox, TableType::active_trades, -1);
        AfxRedrawWindow(hListBox);
        auto t = std::make_shared<Trade>();
        t = nullptr;
        TradeHistory_ShowTradesHistoryTable(t);
    }


    CustomVScrollBar_Recalculate(hCustomVScrollBar);

    curSel = ListBox_GetCurSel(hListBox);
    if (curSel <= 1) curSel = 1;


    // If trades exist then select the first trade so that its history will show
    if (ListBox_GetCount(hListBox) == 0) {
        ListBoxData_AddBlankLine(hListBox);
        curSel = 0;
    }

    if (trades.size() != 0) {
        ListBox_SetSel(hListBox, true, curSel);
        ActiveTrades_ShowListBoxItem(curSel);
    }

    RECT rc; GetClientRect(HWND_ACTIVETRADES, &rc);
    ActiveTrades_OnSize(HWND_ACTIVETRADES, 0, rc.right, rc.bottom);

    SetFocus(hListBox);
}


// ========================================================================================
// Select a line in the Listbox and deselect any other lines that do not match this
// new line's Trade pointer.
// ========================================================================================
bool ActiveTrades_SelectListBoxItem(HWND hListBox, int idx)
{
    // Get the trade pointer for the newly selected line.
    std::shared_ptr<Trade> trade;
    ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, idx);
    if (ld == nullptr) return false;

    trade = ld->trade;
    if (trade == nullptr) {
        ListBox_SetSel(hListBox, false, idx);
        return false;
    }

    
    // Show the trade history for the selected trade
    ActiveTrades_ShowListBoxItem(idx);


    // Check to see if other lines in the listbox are selected. We will deselect those other
    // lines if they do not belong to the current trade being selected.
    // If the line being clicked on is the main header line for the trade (LineType::ticker_line)
    // then we deselected everything else including legs of this same trade. We do this
    // because there are different popup menu actions applicable to the main header line
    // and/or the individual trade legs.
    LineType current_selected_line_type = ld->line_type;

    int nCount = ListBox_GetSelCount(hListBox);

    if (nCount) {
        int* selItems = new int[nCount]();
        SendMessage(hListBox, LB_GETSELITEMS, (WPARAM)nCount, (LPARAM)selItems);

        for (int i = 0; i < nCount; ++i)
        {
            ld = (ListBoxData*)ListBox_GetItemData(hListBox, selItems[i]);
            if (ld != nullptr) {
                if (ld->trade != trade) {
                    ListBox_SetSel(hListBox, false, selItems[i]);
                }
                else {
                    // If selecting items within the same Trade then do not allow
                    // mixing selections of the header line and individual legs.
                    if (current_selected_line_type != ld->line_type) {
                        ListBox_SetSel(hListBox, false, selItems[i]);
                    }
                }
            }
        }

        delete[] selItems;
    }

    AfxRedrawWindow(hListBox);

    return true;
}


// ========================================================================================
// Expire the selected legs. Basically, ask for confirmation via a messagebox and 
// then take appropriate action.
// ========================================================================================
void ActiveTrades_ExpireSelectedLegs(auto trade)
{
    // Do a check to ensure that there is actually legs selected to expire. It could
    // be that the user select SHARES or other non-options underlyings only.
    if (tdd.legs.size() == 0) {
        MessageBox(
            HWND_SIDEMENU,
            (LPCWSTR)(L"No valid option legs have been selected for expiration."),
            (LPCWSTR)L"Warning",
            MB_ICONWARNING | MB_OK);
        return;
    }
        
    int res = MessageBox(
        HWND_SIDEMENU,
        (LPCWSTR)(L"Are you sure you wish to EXPIRE the selected legs?"),
        (LPCWSTR)L"Confirm",
        MB_ICONWARNING | MB_YESNOCANCEL | MB_DEFBUTTON2);

    if (res != IDYES) return;

    std::shared_ptr<Transaction> trans = std::make_shared<Transaction>();

    trans->description = L"Expiration";
    trans->underlying = L"OPTIONS";
    trade->transactions.push_back(trans);

    for (auto leg : tdd.legs) {

        // Save this transaction's leg quantities
        std::shared_ptr<Leg> newleg = std::make_shared<Leg>();

        trade->nextleg_id += 1;
        newleg->leg_id = trade->nextleg_id;

        newleg->underlying = trans->underlying;

        trans->trans_date = AfxCurrentDate();
        newleg->original_quantity = leg->open_quantity * -1;
        newleg->open_quantity = 0;
        newleg->leg_back_pointer_id = leg->leg_id;
        leg->open_quantity = 0;

        if (leg->action == L"STO") newleg->action = L"BTC";
        if (leg->action == L"BTO") newleg->action = L"STC"; 

        newleg->expiry_date = leg->expiry_date;
        newleg->strike_price = leg->strike_price;
        newleg->PutCall = leg->PutCall;
        trans->legs.push_back(newleg);
    }

    // Set the open status of the entire trade based on the new modified legs
    trade->SetTradeOpenStatus();

    // Rebuild the openLegs position vector
    trade->CreateOpenLegsVector();

    // Save the new data to the database
    SaveDatabase();

    // Reload the trade list
    ActiveTrades_ShowActiveTrades(true);

}


// ========================================================================================
// Create Transaction for Shares or Futures that have been called away.
// ========================================================================================
void ActiveTrades_CalledAwayAssignment(
    auto trade, auto leg, int aggregate_shares, int aggregate_futures)
{
    std::shared_ptr<Transaction> trans;
    std::shared_ptr<Leg> newleg;

    bool isShares = (IsFuturesTicker(trade->ticker_symbol)) ? false : true;

    int quantity_assigned = 0;
    int leg_quantity = 0;
    double multiplier = 1;

    std::wstring msg = L"Continue with OPTION ASSIGNMENT?\n\n";

    if (isShares == true) {
        quantity_assigned = min(abs(leg->open_quantity * 100), abs(aggregate_shares));
        leg_quantity = quantity_assigned / 100;
        msg += std::to_wstring(quantity_assigned) + L" shares called away at $" + leg->strike_price + L" per share.";
        multiplier = 1;
    }
    else {
        quantity_assigned = min(abs(leg->open_quantity), abs(aggregate_futures));
        leg_quantity = quantity_assigned;
        msg += std::to_wstring(quantity_assigned) + L" futures called away at $" + leg->strike_price + L" per future.";
        multiplier = trade->multiplier;
    }
    leg_quantity = (leg->open_quantity < 0) ? leg_quantity * -1 : leg_quantity;

    int res = MessageBox(
        HWND_SIDEMENU,
        (LPCWSTR)(msg.c_str()),
        (LPCWSTR)L"Confirm",
        MB_ICONWARNING | MB_YESNOCANCEL | MB_DEFBUTTON2);

    if (res != IDYES) return;


    // Close the Option. Save this transaction's leg quantities
    trans = std::make_shared<Transaction>();
    trans->trans_date = AfxCurrentDate();
    trans->description = L"Called away";
    trans->underlying = L"OPTIONS";
    trade->transactions.push_back(trans);

    newleg = std::make_shared<Leg>();
    trade->nextleg_id += 1;
    newleg->leg_id = trade->nextleg_id;
    newleg->underlying = trans->underlying;
    newleg->original_quantity = leg_quantity * -1;
    newleg->open_quantity = 0;
    newleg->leg_back_pointer_id = leg->leg_id;
    leg->open_quantity = leg->open_quantity - leg_quantity;

    if (leg->action == L"STO") newleg->action = L"BTC";
    if (leg->action == L"BTO") newleg->action = L"STC";

    newleg->expiry_date = leg->expiry_date;
    newleg->strike_price = leg->strike_price;
    newleg->PutCall = leg->PutCall;
    trans->legs.push_back(newleg);


    // Remove the SHARES/FUTURES that have been called away.
    trans = std::make_shared<Transaction>();
    trans->trans_date = AfxCurrentDate();
    trans->description = L"Called away";
    trans->underlying = (isShares == true) ? L"SHARES" : L"FUTURES";
    trans->quantity = quantity_assigned;
    trans->price = AfxValDouble(leg->strike_price);
    trans->multiplier = multiplier;
    trans->fees = 0;
    trade->transactions.push_back(trans);

    newleg = std::make_shared<Leg>();
    trade->nextleg_id += 1;
    newleg->leg_id = trade->nextleg_id;
    newleg->underlying = trans->underlying;
    newleg->strike_price = leg->strike_price;

    if (leg->PutCall == L"P") {
        newleg->action = L"BTC";
        newleg->original_quantity = quantity_assigned;
        newleg->open_quantity = quantity_assigned;
        trans->total = trans->quantity * multiplier * trans->price * -1;  // DR
        trade->acb = trade->acb + trans->total;
    }
    else {
        newleg->action = L"STC";
        newleg->original_quantity = quantity_assigned * -1;
        newleg->open_quantity = quantity_assigned * -1;
        trans->total = trans->quantity * multiplier * trans->price;  // CR
        trade->acb = trade->acb + trans->total;
    }

    trans->legs.push_back(newleg);

    // Set the open status of the entire trade based on the new modified legs
    trade->SetTradeOpenStatus();

    // Rebuild the openLegs position vector
    trade->CreateOpenLegsVector();

    // Save the new data to the database
    SaveDatabase();

    // Reload the trade list
    ActiveTrades_ShowActiveTrades(true);

}


// ========================================================================================
// Create Transaction for option assignment for the selected leg.
// ========================================================================================
void ActiveTrades_Assignment(auto trade, auto leg)
{
    std::shared_ptr<Transaction> trans;
    std::shared_ptr<Leg> newleg;

    bool isShares = (trade->ticker_symbol.substr(0,1) == L"/") ? false : true;

    int quantity_assigned = 0;
    double multiplier = 1;

    std::wstring long_short_text = (leg->PutCall == L"P") ? L"LONG " : L"SHORT ";
    std::wstring msg = L"Continue with OPTION ASSIGNMENT?\n\n";
        
    if (isShares == true) {
        quantity_assigned = abs(leg->open_quantity * 100);
        multiplier = 1;
        msg += long_short_text + std::to_wstring(quantity_assigned) + L" shares at $" + leg->strike_price + L" per share.";
    }
    else {
        quantity_assigned = abs(leg->open_quantity);
        multiplier = trade->multiplier;
        msg += long_short_text + std::to_wstring(quantity_assigned) + L" futures at $" + leg->strike_price + L" per future.";
    }

    int res = MessageBox(
        HWND_SIDEMENU,
        (LPCWSTR)(msg.c_str()),
        (LPCWSTR)L"Confirm",
        MB_ICONWARNING | MB_YESNOCANCEL | MB_DEFBUTTON2);

    if (res != IDYES) return;


    // Close the Option. Save this transaction's leg quantities
    trans = std::make_shared<Transaction>();
    trans->trans_date = leg->expiry_date;  // AfxCurrentDate();
    trans->description = L"Assignment";
    trans->underlying = L"OPTIONS";
    trade->transactions.push_back(trans);

    newleg = std::make_shared<Leg>();
    trade->nextleg_id += 1;
    newleg->leg_id = trade->nextleg_id;
    newleg->underlying = trans->underlying;
    newleg->original_quantity = leg->open_quantity * -1;
    newleg->open_quantity = 0;
    newleg->leg_back_pointer_id = leg->leg_id;
    leg->open_quantity = 0;

    if (leg->action == L"STO") newleg->action = L"BTC";
    if (leg->action == L"BTO") newleg->action = L"STC";

    newleg->expiry_date = leg->expiry_date;
    newleg->strike_price = leg->strike_price;
    newleg->PutCall = leg->PutCall;
    trans->legs.push_back(newleg);


    // Make the SHARES/FUTURES that have been assigned.
    trans = std::make_shared<Transaction>();
    trans->trans_date = leg->expiry_date;  // AfxCurrentDate();
    trans->description = L"Assignment";
    trans->underlying = (isShares == true) ? L"SHARES" : L"FUTURES";
    trans->quantity = quantity_assigned;
    trans->price = AfxValDouble(leg->strike_price);
    trans->multiplier = multiplier;
    trans->fees = 0;
    trade->transactions.push_back(trans);

    newleg = std::make_shared<Leg>();
    trade->nextleg_id += 1;
    newleg->leg_id = trade->nextleg_id;
    newleg->underlying = trans->underlying;
    newleg->strike_price = leg->strike_price;

    if (leg->PutCall == L"P") {
        newleg->action = L"BTO";
        newleg->original_quantity = quantity_assigned;
        newleg->open_quantity = quantity_assigned;
        trans->total = trans->quantity * multiplier * trans->price * -1;  // DR
        trade->acb = trade->acb + trans->total;
    }
    else {
        newleg->action = L"STO";
        newleg->original_quantity = quantity_assigned * -1;
        newleg->open_quantity = quantity_assigned * -1;
        trans->total = trans->quantity * multiplier * trans->price;  // CR
        trade->acb = trade->acb + trans->total;
    }

    trans->legs.push_back(newleg);

    // Set the open status of the entire trade based on the new modified legs
    trade->SetTradeOpenStatus();

    // Rebuild the openLegs position vector
    trade->CreateOpenLegsVector();

    // Save the new data to the database
    SaveDatabase();

    // Reload the trade list
    ActiveTrades_ShowActiveTrades(true);
}


// ========================================================================================
// Create Transaction for option assignment for the selected leg.
// ========================================================================================
void ActiveTrades_OptionAssignment(auto trade)
{
    // Do a check to ensure that there is actually a leg selected for assignment. 
    if (tdd.legs.size() == 0) {
        MessageBox(
            HWND_SIDEMENU,
            (LPCWSTR)(L"No valid option leg has been selected for assignment."),
            (LPCWSTR)L"Warning",
            MB_ICONWARNING | MB_OK);
        return;
    }

    // Check to see if Shares/Futures exist that could be "called away". If yes, then process
    // that type of transaction rather than adding a new assigned to transaction.
    int aggregate_shares = 0;
    int aggregate_futures = 0;

    for (const auto& trans : trade->transactions) {
        for (const auto& leg : trans->legs) {
            if (leg->underlying == L"SHARES") {
                aggregate_shares += leg->open_quantity;
            }
            else if (leg->underlying == L"FUTURES") {
                aggregate_futures += leg->open_quantity;
            }
        }
    }


    auto leg = tdd.legs.at(0);

    // Are LONG SHARES or LONG FUTURES being called away
    if ((aggregate_shares > 0 || aggregate_futures > 0) && leg->PutCall == L"C") {
        ActiveTrades_CalledAwayAssignment(trade, leg, aggregate_shares, aggregate_futures);
        return;
    }
    // Are SHORT SHARES or SHORT FUTURES being called away
    if ((aggregate_shares < 0 || aggregate_futures < 0) && leg->PutCall == L"P") {
        ActiveTrades_CalledAwayAssignment(trade, leg, aggregate_shares, aggregate_futures);
        return;
    }

    // Otherwise, Option is being assigned Shares or Futures
    ActiveTrades_Assignment(trade, leg);

}


// ========================================================================================
// Populate vector that holds all selected lines/legs. This will be passed to the 
// TradeDialog in order to perform actions on the legs.
// ========================================================================================
void ActiveTrades_PopulateLegsEditVector(HWND hListBox)
{
    tdd.legs.clear();

    int nCount = ListBox_GetSelCount(hListBox);
    tdd.legs.reserve(nCount);

    if (nCount) {
        int* selItems = new int[nCount]();
        SendMessage(hListBox, LB_GETSELITEMS, (WPARAM)nCount, (LPARAM)selItems);

        for (int i = 0; i < nCount; i++)
        {
            ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, selItems[i]);
            if (ld != nullptr) {
                // Only allow Option legs to be pushed to legEdit
                if (ld->leg != nullptr) {
                    tdd.legs.push_back(ld->leg);
                }
            }
        }

        delete[] selItems;
    }
}



// ========================================================================================
// Handle the right-click popup menu on the ListBox's selected lines.
// ========================================================================================
void ActiveTrades_RightClickMenu(HWND hListBox, int idx)
{
    HMENU hMenu = CreatePopupMenu();

    std::wstring text;
    std::wstring plural_text;
    int nCount = ListBox_GetSelCount(hListBox);

    std::shared_ptr<Trade> trade = nullptr;

    bool IsTickerLine = false;

    // Clear the tdd module global trade variable
    tdd.ResetDefaults();

    int nCurSel = ListBox_GetCurSel(hListBox);
    ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, nCurSel);
    if (ld->trade == nullptr) return;

    trade = ld->trade;
    tdd.trade = ld->trade;
    tdd.trans = ld->trans;
    tdd.shares_aggregate_edit = ld->aggregate_shares;

    if (nCount == 1) {
        // Is this the Trade header line
        if (ld != nullptr) {
            if (ld->line_type == LineType::ticker_line) {
                IsTickerLine = true;
            }
        }
    }
    else {
        plural_text = L"s";
    }


    if (ld->line_type == LineType::options_leg) {
        text = L"Roll Leg" + plural_text;
        InsertMenu(hMenu, 0, MF_BYCOMMAND | MF_STRING | MF_ENABLED, (int)TradeAction::roll_leg, text.c_str());

        text = L"Close Leg" + plural_text;
        InsertMenu(hMenu, 0, MF_BYCOMMAND | MF_STRING | MF_ENABLED, (int)TradeAction::close_leg, text.c_str());

        text = L"Expire Leg" + plural_text;
        InsertMenu(hMenu, 0, MF_BYCOMMAND | MF_STRING | MF_ENABLED, (int)TradeAction::expire_leg, text.c_str());

        if (nCount == 1) {
            InsertMenu(hMenu, 0, MF_BYCOMMAND | MF_SEPARATOR | MF_ENABLED, (int)TradeAction::no_action + 1, L"");
            InsertMenu(hMenu, 0, MF_BYCOMMAND | MF_STRING | MF_ENABLED, (int)TradeAction::assignment, L"Option Assignment");
        }
        InsertMenu(hMenu, 0, MF_BYCOMMAND | MF_SEPARATOR | MF_ENABLED, (int)TradeAction::no_action + 2, L"");
    }


    if (ld->line_type == LineType::shares) {
        text = L"Manage Shares";
        InsertMenu(hMenu, 0, MF_BYCOMMAND | MF_STRING | MF_ENABLED, (int)TradeAction::manage_shares, text.c_str());
        InsertMenu(hMenu, 0, MF_BYCOMMAND | MF_SEPARATOR | MF_ENABLED, (int)TradeAction::no_action + 2, L"");
    }

    if (ld->line_type == LineType::futures) {
        text = L"Manage Futures";
        InsertMenu(hMenu, 0, MF_BYCOMMAND | MF_STRING | MF_ENABLED, (int)TradeAction::manage_futures, text.c_str());
        InsertMenu(hMenu, 0, MF_BYCOMMAND | MF_SEPARATOR | MF_ENABLED, (int)TradeAction::no_action + 2, L"");
    }

    InsertMenu(hMenu, 0, MF_BYCOMMAND | MF_STRING | MF_ENABLED, (int)TradeAction::add_options_to_trade, L"Add Options to Trade");
    InsertMenu(hMenu, 0, MF_BYCOMMAND | MF_STRING | MF_ENABLED, (int)TradeAction::add_put_to_trade, L"Add Put to Trade");
    InsertMenu(hMenu, 0, MF_BYCOMMAND | MF_STRING | MF_ENABLED, (int)TradeAction::add_call_to_trade, L"Add Call to Trade");
    InsertMenu(hMenu, 0, MF_BYCOMMAND | MF_SEPARATOR | MF_ENABLED, (int)TradeAction::no_action + 3, L"");

    if (IsFuturesTicker(trade->ticker_symbol)) {
        InsertMenu(hMenu, 0, MF_BYCOMMAND | MF_STRING | MF_ENABLED, (int)TradeAction::add_futures_to_trade, L"Add Futures to Trade");
    } else {
        InsertMenu(hMenu, 0, MF_BYCOMMAND | MF_STRING | MF_ENABLED, (int)TradeAction::add_shares_to_trade, L"Add Shares to Trade");
    }

    POINT pt; GetCursorPos(&pt);
    TradeAction selected =
        (TradeAction) TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RETURNCMD, pt.x, pt.y, 0, hListBox, NULL);



    switch (selected)
    {
    case TradeAction::manage_shares:
    case TradeAction::manage_futures:
    case TradeAction::add_shares_to_trade:
    case TradeAction::add_futures_to_trade:
    case TradeAction::add_options_to_trade:
    case TradeAction::add_put_to_trade:
    case TradeAction::add_call_to_trade:
        TradeDialog_Show(selected);
        break;

    case TradeAction::roll_leg:
    case TradeAction::close_leg:
        ActiveTrades_PopulateLegsEditVector(hListBox);
        TradeDialog_Show(selected);
        break;

    case TradeAction::expire_leg:
        ActiveTrades_PopulateLegsEditVector(hListBox);
        ActiveTrades_ExpireSelectedLegs(trade);
        break;

    case TradeAction::assignment:
        ActiveTrades_PopulateLegsEditVector(hListBox);
        ActiveTrades_OptionAssignment(trade);
        break;
    }

}


// ========================================================================================
// Listbox subclass Window procedure
// ========================================================================================
LRESULT CALLBACK ActiveTrades_ListBox_SubclassProc(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    // Create static accumulation variable to collect the data from
    // a series of middle mouse wheel scrolls.
    static int accumDelta = 0;

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
        HWND hCustomVScrollBar = GetDlgItem(HWND_ACTIVETRADES, IDC_TRADES_CUSTOMVSCROLLBAR);
        CustomVScrollBar_Recalculate(hCustomVScrollBar);
        return 0;
        break;
    }


    case WM_RBUTTONDOWN:
    {
        int idx = Listbox_ItemFromPoint(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        
        // The return value contains the index of the nearest item in the LOWORD. The HIWORD is zero 
        // if the specified point is in the client area of the list box, or one if it is outside the 
        // client area.
        if (HIWORD(idx) == 1) break;
            
        // Return to not select the line (eg. if a blank line was clicked on)
        if (ActiveTrades_SelectListBoxItem(hWnd, idx) == false) {
            return 0;
        }
        ListBox_SetSel(hWnd, true, idx);

        ActiveTrades_RightClickMenu(hWnd, idx);
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
        ActiveTrades_SelectListBoxItem(hWnd, idx);
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
            Color back_color(COLOR_GRAYDARK);
            SolidBrush back_brush(back_color);
            graphics.FillRectangle(&back_brush, rc.left, rc.top, nWidth, nHeight);
        }

        ValidateRect(hWnd, &rc);
        return TRUE;
        break;

    }


    case WM_DESTROY:
        // Destroy all manually allocated ListBox display data that is held
        // in the LineData structures..
        ListBoxData_DestroyItemData(hWnd);

        // REQUIRED: Remove control subclassing
        RemoveWindowSubclass(hWnd, ActiveTrades_ListBox_SubclassProc, uIdSubclass);
        break;


    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);

}



// ========================================================================================
// Process WM_MEASUREITEM message for window/dialog: ActiveTrades
// ========================================================================================
void ActiveTrades_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT* lpMeasureItem)
{
    lpMeasureItem->itemHeight = AfxScaleY(ACTIVETRADES_LISTBOX_ROWHEIGHT);
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: ActiveTrades
// ========================================================================================
BOOL ActiveTrades_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    // Handle all of the painting in WM_PAINT
    return TRUE;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: ActiveTrades
// ========================================================================================
void ActiveTrades_OnPaint(HWND hwnd)
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
// Show/Hide the Net and Excess liquidity labels and values.
// ========================================================================================
void ActiveTrades_ShowHideLiquidityLabels(HWND hwnd)
{
    int nShow = (tws_IsConnected()) ? SW_SHOW : SW_HIDE;

    ShowWindow(GetDlgItem(hwnd, IDC_TRADES_NETLIQUIDATION), nShow);
    ShowWindow(GetDlgItem(hwnd, IDC_TRADES_NETLIQUIDATION_VALUE), nShow);
    ShowWindow(GetDlgItem(hwnd, IDC_TRADES_EXCESSLIQUIDITY), nShow);
    ShowWindow(GetDlgItem(hwnd, IDC_TRADES_EXCESSLIQUIDITY_VALUE), nShow);
}
    
    
 // ========================================================================================
// Process WM_SIZE message for window/dialog: ActiveTrades
// ========================================================================================
void ActiveTrades_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    HWND hHeader = GetDlgItem(hwnd, IDC_TRADES_HEADER);
    HWND hListBox = GetDlgItem(hwnd, IDC_TRADES_LISTBOX);
    HWND hCustomVScrollBar = GetDlgItem(hwnd, IDC_TRADES_CUSTOMVSCROLLBAR);
        
    int margin = AfxScaleY(ACTIVETRADES_MARGIN);

    // If no entries exist for the ListBox then don't show any child controls
    int show_flag = (ListBox_GetCount(hListBox) <= 1) ? SWP_HIDEWINDOW : SWP_SHOWWINDOW;

    HDWP hdwp = BeginDeferWindowPos(10);

    // Move and size the top labels into place
    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_TRADES_LABEL), 0, 0, 0, AfxScaleX(200), margin, SWP_NOZORDER | SWP_SHOWWINDOW);
    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_TRADES_NETLIQUIDATION),        0, AfxScaleX(380), 0, AfxScaleX(60), margin, SWP_NOZORDER);
    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_TRADES_NETLIQUIDATION_VALUE),  0, AfxScaleX(440), 0, AfxScaleX(50), margin, SWP_NOZORDER);
    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_TRADES_EXCESSLIQUIDITY),       0, AfxScaleX(490), 0, AfxScaleX(65), margin, SWP_NOZORDER);
    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_TRADES_EXCESSLIQUIDITY_VALUE), 0, AfxScaleX(555), 0, AfxScaleX(50), margin, SWP_NOZORDER);

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
    int nTop = margin;
    int nHeight = cy - nTop;
    int nWidth = cx - custom_scrollbar_width;
    hdwp = DeferWindowPos(hdwp, hListBox, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | show_flag);
    
    nLeft = nLeft + nWidth;   // right edge of ListBox
    nWidth = custom_scrollbar_width;
    int show_scrollbar = (bshow_scrollbar ? SWP_SHOWWINDOW : SWP_HIDEWINDOW);
    if (show_flag == SWP_HIDEWINDOW) show_scrollbar = SWP_HIDEWINDOW;
    hdwp = DeferWindowPos(hdwp, hCustomVScrollBar, 0, nLeft, nTop, nWidth, nHeight,
        SWP_NOZORDER | show_flag);

    EndDeferWindowPos(hdwp);

    ActiveTrades_ShowHideLiquidityLabels(hwnd);

}


// ========================================================================================
// Process WM_CREATE message for window/dialog: ActiveTrades
// ========================================================================================
BOOL ActiveTrades_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    HWND_ACTIVETRADES = hwnd;
        
    HWND hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TRADES_LABEL, L"Active Trades", 
        COLOR_WHITELIGHT, COLOR_BLACK);

    hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TRADES_NETLIQUIDATION, L"Net Liq:",
        COLOR_WHITEDARK, COLOR_BLACK);

    hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TRADES_NETLIQUIDATION_VALUE, L"",
        COLOR_WHITELIGHT, COLOR_BLACK);

    hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TRADES_EXCESSLIQUIDITY, L"Excess Liq:",
        COLOR_WHITEDARK, COLOR_BLACK);

    hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TRADES_EXCESSLIQUIDITY_VALUE, L"",
        COLOR_WHITELIGHT, COLOR_BLACK);

    // Create an Ownerdraw fixed row sized listbox that we will use to custom
    // paint our various open trades.
    hCtl =
        ActiveTrades.AddControl(Controls::ListBox, hwnd, IDC_TRADES_LISTBOX, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_EXTENDEDSEL | 
            LBS_OWNERDRAWFIXED | LBS_NOTIFY | LBS_WANTKEYBOARDINPUT,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)ActiveTrades_ListBox_SubclassProc,
            IDC_TRADES_LISTBOX, NULL);
    ListBox_AddString(hCtl, NULL);

    // Create our custom vertical scrollbar and attach the ListBox to it.
    CreateCustomVScrollBar(hwnd, IDC_TRADES_CUSTOMVSCROLLBAR, hCtl);

    return TRUE;
}


// ========================================================================================
// Process WM_COMMAND message for window/dialog: ActiveTrades
// ========================================================================================
void ActiveTrades_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (codeNotify)
    {

    case LBN_SELCHANGE:
    {
        int nCurSel = ListBox_GetCurSel(hwndCtl);
        ActiveTrades_SelectListBoxItem(hwndCtl, nCurSel);
        break;
    }

    }
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CActiveTrades::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    static bool portfolio_requested = false;
    
    switch (msg)
    {
        HANDLE_MSG(m_hwnd, WM_CREATE, ActiveTrades_OnCreate);
        HANDLE_MSG(m_hwnd, WM_COMMAND, ActiveTrades_OnCommand);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, ActiveTrades_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, ActiveTrades_OnPaint);
        HANDLE_MSG(m_hwnd, WM_SIZE, ActiveTrades_OnSize);
        HANDLE_MSG(m_hwnd, WM_MEASUREITEM, ActiveTrades_OnMeasureItem);
        HANDLE_MSG(m_hwnd, WM_DRAWITEM, ListBoxData_OnDrawItem);

    
    case MSG_POSITIONS_READY:
        // Request Positions has completed and has sent this notification so 
        // we can now start requesting the portfolio updates real time data.
        if (!portfolio_requested) {
            client.RequestPortfolioUpdates();
            portfolio_requested = true;
        }
        return 0;


    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}

