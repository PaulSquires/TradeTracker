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
#include "MainWindow/tws-client.h"
#include "CustomLabel/CustomLabel.h"
#include "CustomMessageBox/CustomMessageBox.h"
#include "CustomVScrollBar/CustomVScrollBar.h"
#include "CustomPopupMenu/CustomPopupMenu.h"
#include "Utilities/ListBoxData.h"
#include "MainWindow/MainWindow.h"
#include "TradeDialog/TradeDialog.h"
#include "TradeHistory/TradeHistory.h"
#include "TabPanel/TabPanel.h"
#include "Database/trade.h"
#include "Database/database.h"
#include "Category/Category.h"
#include "Config/Config.h"

#include "Assignment.h"
#include "ActiveTrades.h"


CActiveTrades ActiveTrades;

std::wstring quantity_set_from_assignment_modal;


// ========================================================================================
// Get the HWND's of the the controls on the form.
// ========================================================================================
inline HWND CActiveTrades::TradesListBox() {
    return GetDlgItem(hWindow, IDC_ACTIVETRADES_LISTBOX);
}
inline HWND CActiveTrades::PaperTradingLabel() {
    return GetDlgItem(hWindow, IDC_ACTIVETRADES_PAPERWARNING);
}
inline HWND CActiveTrades::SortFilterLabel() {
    return GetDlgItem(hWindow, IDC_ACTIVETRADES_LBLSORTFILTER);
}
inline HWND CActiveTrades::SortFilterCombo() {
    return GetDlgItem(hWindow, IDC_ACTIVETRADES_SORTFILTER);
}
inline HWND CActiveTrades::SortFilterButton() {
    return GetDlgItem(hWindow, IDC_ACTIVETRADES_CMDSORTFILTER);
}
inline HWND CActiveTrades::VScrollBar() {
    return GetDlgItem(hWindow, IDC_ACTIVETRADES_CUSTOMVSCROLLBAR);
}
inline HWND CActiveTrades::NewTradeLabel() {
    return GetDlgItem(hWindow, IDC_ACTIVETRADES_LBLNEWTRADE);
}
inline HWND CActiveTrades::NewTradeCombo() {
    return GetDlgItem(hWindow, IDC_ACTIVETRADES_NEWTRADE);
}
inline HWND CActiveTrades::NewTradeButton() {
    return GetDlgItem(hWindow, IDC_ACTIVETRADES_CMDNEWTRADE);
}
inline HWND CActiveTrades::NetLiquidationLabel() {
    return GetDlgItem(hWindow, IDC_ACTIVETRADES_NETLIQUIDATION);
}
inline HWND CActiveTrades::NetLiquidationValueLabel() {
    return GetDlgItem(hWindow, IDC_ACTIVETRADES_NETLIQUIDATION_VALUE);
}
inline HWND CActiveTrades::ExcessLiquidityLabel() {
    return GetDlgItem(hWindow, IDC_ACTIVETRADES_EXCESSLIQUIDITY);
}
inline HWND CActiveTrades::ExcessLiquidityValueLabel() {
    return GetDlgItem(hWindow, IDC_ACTIVETRADES_EXCESSLIQUIDITY_VALUE);
}
inline HWND CActiveTrades::MaintenanceLabel() {
    return GetDlgItem(hWindow, IDC_ACTIVETRADES_MAINTENANCE);
}
inline HWND CActiveTrades::MaintenanceValueLabel() {
    return GetDlgItem(hWindow, IDC_ACTIVETRADES_MAINTENANCE_VALUE);
}


// ========================================================================================
// Perform the ITM calculation and update the Trade pointer with the values.
// ========================================================================================
void CActiveTrades::PerformITMcalculation(std::shared_ptr<Trade>& trade) {
    bool is_itm_red = false;
    bool is_itm_green = false;
    bool is_long_spread = false;

    for (const auto& leg : trade->open_legs) {
        if (leg->underlying == Underlying::Options) {
            if (leg->put_call == PutCall::Put) {
                if (trade->ticker_last_price < AfxValDouble(leg->strike_price)) {
                    if (leg->open_quantity < 0) {
                        is_itm_red = (is_long_spread == true) ? false : true;
                    }
                    if (leg->open_quantity > 0) {
                        is_itm_green = true; is_itm_red = false; is_long_spread = true;
                    }
                }
            }
            else if (leg->put_call == PutCall::Call) {
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

    std::wstring text;

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


// ========================================================================================
// ActiveTrades_UpdateTickerPricesLine()
// ========================================================================================
void CActiveTrades::UpdateTickerPricesLine(int index, ListBoxData* ld) {

    // Lookup the most recent Market Price data
    TickerData td{};
    if (mapTickerData.count(ld->ticker_id)) {
        td = mapTickerData.at(ld->ticker_id);
    }

    ld->trade->ticker_last_price = td.last_price;
    ld->trade->ticker_close_price = td.close_price;
    if (ld->trade->ticker_last_price == 0) {
        if (td.close_price != 0) ld->trade->ticker_last_price = td.close_price;
    }

    // Calculate the price change
    double delta{0};
    if (ld->trade->ticker_close_price != 0) {
        delta = (ld->trade->ticker_last_price - ld->trade->ticker_close_price);
    }

    std::wstring text;
    DWORD theme_color = COLOR_WHITELIGHT;

    // Calculate if any of the option legs are ITM in a good (green) or bad (red) way.
    // We use a separate function call because scrapped data will need acces to the 
    // ITM calculation also.
    PerformITMcalculation(ld->trade);

    ld->SetTextData(COLUMN_TICKER_ITM, ld->trade->itm_text, ld->trade->itm_color);  // ITM

    text = AfxMoney(delta, true, ld->trade->ticker_decimals);
    theme_color = (delta >= 0) ? COLOR_GREEN : COLOR_RED;
    ld->trade->ticker_column_1 = text;
    ld->trade->ticker_column_1_clr = theme_color;
    ld->SetTextData(COLUMN_TICKER_CHANGE, text, theme_color);  // price change

    text = AfxMoney(ld->trade->ticker_last_price, true, ld->trade->ticker_decimals);
    ld->trade->ticker_column_2 = text;
    ld->trade->ticker_column_1_clr = COLOR_WHITELIGHT;
    ld->SetTextData(COLUMN_TICKER_CURRENTPRICE, text, COLOR_WHITELIGHT);  // current price

    text = AfxMoney((delta / ld->trade->ticker_last_price) * 100, true) + L"%";
    if (delta > 0) text = L"+" + text;
    theme_color = (delta >= 0) ? COLOR_GREEN : COLOR_RED;
    ld->trade->ticker_column_3 = text;
    ld->trade->ticker_column_3_clr = theme_color;
    ld->SetTextData(COLUMN_TICKER_PERCENTCHANGE, text, theme_color);  // price percentage change

    RECT rc{};
    ListBox_GetItemRect(TradesListBox(), index, &rc);
    InvalidateRect(TradesListBox(), &rc, true);
    UpdateWindow(TradesListBox());
}


// ========================================================================================
// ActiveTrades_UpdateTickerPortfolioLine()
// ========================================================================================
void CActiveTrades::UpdateTickerPortfolioLine(int index, int index_trade, ListBoxData* ld) {

    DWORD theme_color = COLOR_WHITEDARK;

    std::wstring text;

    // Update the Trade's tickerLine with the new totals
    ld = (ListBoxData*)ListBox_GetItemData(TradesListBox(), index_trade);
    if (ld && ld->trade) {

        double value_aggregate{0};
        if (ld->trade->aggregate_shares) value_aggregate = ld->trade->aggregate_shares;
        if (ld->trade->aggregate_futures) value_aggregate = ld->trade->aggregate_futures;

        double trade_acb = ld->trade->acb * -1;
        double shares_market_value = value_aggregate * ld->trade->ticker_last_price;
        double total_cost = shares_market_value;

        for (const auto& leg : ld->trade->open_legs) {
            total_cost += leg->market_value;
        }

        theme_color = COLOR_WHITEDARK;

        text = AfxMoney(trade_acb, true, ld->trade->ticker_decimals);
        ld->SetTextData(COLUMN_TICKER_PORTFOLIO_1, text, theme_color);

        text = AfxMoney(total_cost, true, ld->trade->ticker_decimals);
        ld->SetTextData(COLUMN_TICKER_PORTFOLIO_2, text, theme_color);

        double difference = total_cost - trade_acb;
        theme_color = (difference < 0) ? COLOR_RED : COLOR_GREEN;
        text = AfxMoney(difference, true, 2);
        ld->SetTextData(COLUMN_TICKER_PORTFOLIO_3, text, theme_color);    

        double percentage = difference / trade_acb * 100;
        if (difference < 0) {
            if (percentage >= 0) percentage *= -1;
        }
        else {
            if (percentage <= 0) percentage *= -1;
        } 
        text = AfxMoney(percentage, true, 0) + L" %";
        theme_color = (difference < 0) ? COLOR_RED : COLOR_GREEN;
        ld->SetTextData(COLUMN_TICKER_PORTFOLIO_4, text, theme_color);  

        RECT rc{};
        ListBox_GetItemRect(TradesListBox(), index_trade, &rc);
        InvalidateRect(TradesListBox(), &rc, true);
        UpdateWindow(TradesListBox());
    }
}


// ========================================================================================
// ActiveTrades_UpdateLegPortfolioLine()
// ========================================================================================
void CActiveTrades::UpdateLegPortfolioLine(int index, ListBoxData* ld) {

    DWORD theme_color = COLOR_WHITEDARK;

    std::wstring text;

    if (ld->line_type == LineType::shares ||
        ld->line_type == LineType::futures &&
        ld->trade) {

        // SHARES/FUTURES MARKET VALUE
        if (ld->trade->aggregate_shares || ld->trade->aggregate_futures) {
            double value_aggregate = 0;
            if (ld->trade->aggregate_shares) value_aggregate = ld->trade->aggregate_shares;
            if (ld->trade->aggregate_futures) value_aggregate = ld->trade->aggregate_futures;

            double shares_cost = ld->trade->acb;
            if (shares_cost == 0) shares_cost = 1;
            text = AfxMoney(shares_cost, true, ld->trade->ticker_decimals);
            ld->SetTextData(COLUMN_TICKER_PORTFOLIO_1, text, theme_color);

            double shares_market_value = value_aggregate * ld->trade->ticker_last_price;
            text = AfxMoney(shares_market_value, true, ld->trade->ticker_decimals);
            ld->SetTextData(COLUMN_TICKER_PORTFOLIO_2, text, theme_color);

            // UNREALIZED PNL
            double unrealized_pnl = (shares_cost + shares_market_value);
            theme_color = (unrealized_pnl < 0) ? COLOR_RED : COLOR_GREEN;
            text = AfxMoney(unrealized_pnl, true, ld->trade->ticker_decimals);
            ld->SetTextData(COLUMN_TICKER_PORTFOLIO_3, text, theme_color);    // Unrealized profit or loss

            // UNREALIZED PNL PERCENTAGE
            double percentage = (unrealized_pnl / shares_cost * 100);
            if (unrealized_pnl >= 0) {
                percentage = abs(percentage);
            }
            else {
                // percentage must also be negative
                if (percentage > 0) percentage *= -1;
            }
            theme_color = (percentage < 0) ? COLOR_RED : COLOR_GREEN;
            text = AfxMoney(percentage, true, 0) + L" %";
            ld->SetTextData(COLUMN_TICKER_PORTFOLIO_4, text, theme_color);  // Percentage values for the previous two columns data
        }
    }

    if (ld->line_type == LineType::options_leg && ld->leg) {

        ld->leg->listbox_index = index;

        // Lookup the most recent Portfolio position data
        PortfolioData pd{};
        bool found = false;
        if (mapPortfolioData.count(ld->leg->contract_id)) {
            pd = mapPortfolioData.at(ld->leg->contract_id);
            found = true;
        }

        // CALCULATE THE RATIO BASED LEG COST
        // Use the incoming IB data costs for the legs in order to create a ratio for each leg
        // to multiple against the total ACB for the trade.
        double position_cost_all_legs = 0;
        double trade_acb = ld->trade->acb * -1;
        if (found) {
            ld->leg->position_cost_tws = pd.average_cost * ld->leg->open_quantity;
            for (const auto& leg : ld->trade->open_legs) {
                // Accummulate the costs of all the open legs
                position_cost_all_legs += leg->position_cost_tws;
            }
        }

        // POSITION COST BASIS
        //double position_cost = ld->leg->calculated_leg_cost;
        
        // Update all of the open legs in the Trade based on the newly acquired leg cost data
        for (const auto& leg : ld->trade->open_legs) {

            index = leg->listbox_index;
            ListBoxData* ldleg = (ListBoxData*)ListBox_GetItemData(TradesListBox(), index);

            if (ldleg == (void*)-1) continue;
            if (ldleg == nullptr) continue;
            
            //std::cout << leg->listbox_index << "  " << ldleg->leg->contract_id << std::endl;

            found = false;
            if (mapPortfolioData.count(ldleg->leg->contract_id)) {
                pd = mapPortfolioData.at(ldleg->leg->contract_id);
                found = true;
            }

            ldleg->leg->position_cost_ratio = (ldleg->leg->position_cost_tws / position_cost_all_legs);

            double position_cost = 0;
            position_cost = (ldleg->leg->position_cost_ratio * trade_acb);

            theme_color = COLOR_WHITEDARK;
            text = AfxMoney(position_cost, true, ldleg->trade->ticker_decimals);
            if (!found) text = L"";
            ldleg->SetTextData(COLUMN_TICKER_PORTFOLIO_1, text, theme_color);   // Book Value and average Price

            // MARKET VALUE
            theme_color = COLOR_WHITEDARK;
            double multiplier = AfxValDouble(config.GetMultiplier(ldleg->trade->ticker_symbol));
            double market_value = (pd.market_price * ldleg->leg->open_quantity * multiplier);
            ldleg->leg->market_value = market_value;
            text = AfxMoney(market_value, true, ldleg->trade->ticker_decimals);
            if (!found) text = L"";
            ldleg->SetTextData(COLUMN_TICKER_PORTFOLIO_2, text, theme_color);

            // UNREALIZED PNL
            double unrealized_pnl = (market_value - position_cost);
            ldleg->leg->unrealized_pnl = unrealized_pnl;
            theme_color = (unrealized_pnl < 0) ? COLOR_RED : COLOR_GREEN;
            text = AfxMoney(unrealized_pnl, true, ldleg->trade->ticker_decimals);
            if (!found) text = L"";
            ldleg->SetTextData(COLUMN_TICKER_PORTFOLIO_3, text, theme_color);    // Unrealized profit or loss

            // UNREALIZED PNL PERCENTAGE
            double percentage = (unrealized_pnl / position_cost * 100);
            if (unrealized_pnl >= 0) {
                percentage = abs(percentage);
            }
            else {
                // percentage must also be negative
                if (percentage > 0) percentage *= -1;
            }
            theme_color = (percentage < 0) ? COLOR_RED : COLOR_GREEN;
            ldleg->leg->percentage = percentage;
            text = AfxMoney(percentage, true, 0) + L" %";
            if (!found) text = L"";
            ldleg->SetTextData(COLUMN_TICKER_PORTFOLIO_4, text, theme_color);  // Percentage values for the previous two columns data

            ldleg->SetTextData(COLUMN_TICKER_PORTFOLIO_5, L"", COLOR_WHITEDARK); 

            RECT rc{};
            ListBox_GetItemRect(TradesListBox(), index, &rc);
            InvalidateRect(TradesListBox(), &rc, true);
        }
        UpdateWindow(TradesListBox());



        //double position_cost = 0;
        //position_cost = (ld->leg->position_cost_ratio * trade_acb);

        //text = AfxMoney(position_cost, true, ld->trade->ticker_decimals);
        //if (!found) text = L"";
        //ld->SetTextData(COLUMN_TICKER_PORTFOLIO_1, text, theme_color);   // Book Value and average Price

        //// MARKET VALUE
        //double multiplier = AfxValDouble(config.GetMultiplier(ld->trade->ticker_symbol));
        //double market_value = (pd.market_price * ld->leg->open_quantity * multiplier);
        //ld->leg->market_value = market_value;
        //text = AfxMoney(market_value, true, ld->trade->ticker_decimals);
        //if (!found) text = L"";
        //ld->SetTextData(COLUMN_TICKER_PORTFOLIO_2, text, theme_color);
    
        //// UNREALIZED PNL
        //double unrealized_pnl = (market_value - position_cost);
        //ld->leg->unrealized_pnl = unrealized_pnl;
        //theme_color = (unrealized_pnl < 0) ? COLOR_RED : COLOR_GREEN;
        //text = AfxMoney(unrealized_pnl, true, ld->trade->ticker_decimals);
        //if (!found) text = L"";
        //ld->SetTextData(COLUMN_TICKER_PORTFOLIO_3, text, theme_color);    // Unrealized profit or loss

        //// UNREALIZED PNL PERCENTAGE
        //double percentage = (unrealized_pnl / position_cost * 100);
        //if (unrealized_pnl >= 0) {
        //    percentage = abs(percentage);
        //}
        //else {
        //    // percentage must also be negative
        //    if (percentage > 0) percentage *= -1;
        //}
        //theme_color = (percentage < 0) ? COLOR_RED : COLOR_GREEN;
        //ld->leg->percentage = percentage;
        //text = AfxMoney(percentage, true, 0) + L" %";
        //if (!found) text = L"";
        //ld->SetTextData(COLUMN_TICKER_PORTFOLIO_4, text, theme_color);  // Percentage values for the previous two columns data
        //
        //ld->SetTextData(COLUMN_TICKER_PORTFOLIO_5, L"", COLOR_WHITEDARK);  
    }

    //RECT rc{};
    //ListBox_GetItemRect(TradesListBox(), index, &rc);
    //InvalidateRect(TradesListBox(), &rc, true);
    //UpdateWindow(TradesListBox());
}


// ========================================================================================
// Update the ActiveTrades list with the most up to dat Market Price Data.
// Also updates leg Portfolio position data.
// This function is called from the TickerUpdateFunction() thread.
// ========================================================================================
void CActiveTrades::UpdateTickerPrices() {
    // Guard to prevent re-entry of update should the thread fire the update
    // prior to this function finishing.
    static std::atomic<bool> is_processing = false;

    if (is_processing) return;

    is_processing = true;

    if (!IsWindow(TradesListBox())) return;

    int item_count = ListBox_GetCount(TradesListBox());
    if (item_count == 0) return;

    int index_trade{0};

    for (int index = 0; index < item_count; ++index) {
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(TradesListBox(), index);
        if (ld == (void*)-1) continue;
        if (ld == nullptr) continue;

        if (ld->line_type == LineType::ticker_line) {
            index_trade = index;
            UpdateTickerPricesLine(index, ld);
        }

        UpdateLegPortfolioLine(index, ld);
        UpdateTickerPortfolioLine(index, index_trade, ld);
    }

    // Do calculation to ensure column widths are wide enough to accommodate the new
    // price data that has just arrived.
    if (ListBoxData_ResizeColumnWidths(TradesListBox(), TableType::active_trades) == true) {
        AfxRedrawWindow(TradesListBox());
    }

    is_processing = false;
}


// ========================================================================================
// Returns true/false if incoming Trade action is considered a "New" Options type of action.
// ========================================================================================
bool CActiveTrades::IsNewOptionsTradeAction(TradeAction action) {
    switch (action) {
    case TradeAction::new_options_trade:
    case TradeAction::new_iron_condor:
    case TradeAction::new_short_put_LT112:
    case TradeAction::new_short_strangle:
    case TradeAction::new_short_put:
    case TradeAction::new_short_put_vertical:
    case TradeAction::new_short_call:
    case TradeAction::new_short_call_vertical:
        return true;
    default:
        return false;
    }
}


// ========================================================================================
// Returns true/false if incoming Trade action is considered an "Add Options To" action.
// ========================================================================================
bool CActiveTrades::IsAddOptionToTradeAction(TradeAction action) {
    switch (action) {
    case TradeAction::add_options_to_trade:
    case TradeAction::add_put_to_trade:
    case TradeAction::add_call_to_trade:
        return true;
    default:
        return false;
    }
}


// ========================================================================================
// Returns true/false if incoming Trade action is considered an "Add Shares To" action.
// ========================================================================================
bool CActiveTrades::IsAddSharesToTradeAction(TradeAction action) {
    switch (action) {
    case TradeAction::add_shares_to_trade:
    case TradeAction::add_futures_to_trade:
        return true;
    default:
        return false;
    }
}


// ========================================================================================
// Returns true/false if incoming Trade action is considered a "Manage Shares" action.
// ========================================================================================
bool CActiveTrades::IsManageSharesTradeAction(TradeAction action) {
    switch (action) {
    case TradeAction::manage_shares:
    case TradeAction::manage_futures:
        return true;
    default:
        return false;
    }
}


// ========================================================================================
// Returns true/false if incoming Trade action is consider a "New" Shares/Futures 
// type of action.
// ========================================================================================
bool CActiveTrades::IsNewSharesTradeAction(TradeAction action) {
    switch (action) {
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
void CActiveTrades::ShowListBoxItem(int index) {

    // Select the correct menu panel item
    TabPanel_SelectPanelItem(HWND_TABPANEL, IDC_TABPANEL_ACTIVETRADES);

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
// Populate the Trades ListBox with the current active/open trades
// ========================================================================================
void CActiveTrades::ShowActiveTrades() {

    static bool positions_requested = false;
    static int ticker_id = 100;
    int current_sel{0};

    // Select the correct menu panel item
    TabPanel_SelectPanelItem(HWND_TABPANEL, IDC_TABPANEL_ACTIVETRADES);

    // Ensure that the Trades panel is set
    MainWindow.SetLeftPanel(hWindow);

    // Determine if we need to initialize the listbox
    if (trades.size()) {

        // Prevent ListBox redrawing until all calculations are completed
        SendMessage(TradesListBox(), WM_SETREDRAW, false, 0);

        // In case of newly added/deleted data ensure data is sorted.
        
        if (filter_type == ActiveTradesFilterType::Category) {
            // Sort based on Category and then TickerSymbol
            std::sort(trades.begin(), trades.end(),
                [](const auto& trade1, const auto& trade2) {
                    {
                        if (trade1->category < trade2->category) return true;
                        if (trade2->category < trade1->category) return false;

                        // a=b for primary condition, go to secondary
                        if (trade1->ticker_symbol < trade2->ticker_symbol) return true;
                        if (trade2->ticker_symbol < trade1->ticker_symbol) return false;

                        return false;
                    } 
                });
        }

        if (filter_type == ActiveTradesFilterType::TickerSymbol) {
            // Sort based on TickerSymbol and Expiration
            std::sort(trades.begin(), trades.end(),
                [](const auto& trade1, const auto& trade2) {
                    {
                        if (trade1->ticker_symbol < trade2->ticker_symbol) return true;
                        if (trade2->ticker_symbol < trade1->ticker_symbol) return false;

                        // a=b for primary condition, go to secondary
                        if (trade1->earliest_legs_DTE < trade2->earliest_legs_DTE) return true;
                        if (trade2->earliest_legs_DTE < trade1->earliest_legs_DTE) return false;

                        return false;
                    } 
                });
        }

        if (filter_type == ActiveTradesFilterType::Expiration) {
            // Sort based on Expiration and TickerSymbol
            std::sort(trades.begin(), trades.end(),
                [](const auto& trade1, const auto& trade2) {
                    {
                        if (trade1->earliest_legs_DTE < trade2->earliest_legs_DTE) return true;
                        if (trade2->earliest_legs_DTE < trade1->earliest_legs_DTE) return false;

                        // a=b for primary condition, go to secondary
                        if (trade1->ticker_symbol < trade2->ticker_symbol) return true;
                        if (trade2->ticker_symbol < trade1->ticker_symbol) return false;

                        return false;
                    } 
                });
        }

        // Destroy any existing ListBox line data (clear the LineData pointers)
        ListBoxData_DestroyItemData(TradesListBox());

        // Create the new ListBox line data and initiate the new market data.
        int category_header = -1;
        for (auto& trade : trades) {
            // We are displaying only open trades 
            if (trade->is_open) {
                // Set the decimals for this tickerSymbol. Most will be 2 but futures can have a lot more.
                trade->ticker_decimals = config.GetTickerDecimals(trade->ticker_symbol);

                if (filter_type == ActiveTradesFilterType::Category) {
                    if (trade->category != category_header) {

                        // Count the number of open trades in this category
                        auto num_trades_category{ std::ranges::count_if(trades, 
                            [trade](auto t) {return (t->is_open && (t->category == trade->category)) ? true : false; })
                        };

                        ListBoxData_AddCategoryHeader(TradesListBox(), trade, (int)num_trades_category);
                        category_header = trade->category;
                    }
                }

                // Setup the line, assign the ticker_id.
                TickerId id = (trade->ticker_id == -1) ? ticker_id++ : trade->ticker_id;

                ListBoxData_OpenPosition(TradesListBox(), trade, id);
                trade->ticker_id = id;
            }
        }

        // Start getting market data (ticker price data) for each active ticker.
        // If the ticker has already requested market data then it will skip requesting
        // it again for a duplicate time.
        tws_RequestMarketUpdates();

        // When requestPositions completes, it sends a notification to the Active Trades
        // window that it is now okay to request the Portfolio Updates. We make those
        // portfolio update calls there rather than here.
        if (tws_IsConnected() && !positions_requested) {
            tws_RequestPositions();
            positions_requested = true;
        }

        // Calculate the actual column widths based on the size of the strings in
        // ListBoxData while respecting the minimum values as defined in nMinColWidth[].
        // This function is also called when receiving new price data from TWS because
        // that data may need the column width to be wider.
        ListBoxData_ResizeColumnWidths(TradesListBox(), TableType::active_trades);

        // Redraw the ListBox to ensure that any recalculated columns are 
        // displayed correctly. Re-enable redraw.
        SendMessage(TradesListBox(), WM_SETREDRAW, true, 0);
        AfxRedrawWindow(TradesListBox());
    }

    // If no Trades exist then add simple message to user to add Trade
    if (trades.size() == 0) {
        ListBox_ResetContent(TradesListBox());
        ListBoxData_NoTradesExistMessage(TradesListBox());
        ListBoxData_ResizeColumnWidths(TradesListBox(), TableType::active_trades);
        AfxRedrawWindow(TradesListBox());
        auto t = std::make_shared<Trade>();
        t = nullptr;
        TradeHistory_ShowTradesHistoryTable(t);
    }

    CustomVScrollBar_Recalculate(VScrollBar());

    current_sel = ListBox_GetCurSel(TradesListBox());
    if (current_sel <= 1) {
        current_sel = (filter_type == ActiveTradesFilterType::Category) ? 1 : 0;
    }

    // If trades exist then select the first trade so that its history will show
    if (ListBox_GetCount(TradesListBox()) == 0) {
        ListBoxData_AddBlankLine(TradesListBox());
        current_sel = 0;
    }

    if (trades.size()) {
        ListBox_SetTopIndex(TradesListBox(), 0);
        ListBox_SetSel(TradesListBox(), true, current_sel);
        ShowListBoxItem(current_sel);
    }

    RECT rc{};
    GetClientRect(hWindow, &rc);
    OnSize(hWindow, 0, rc.right, rc.bottom);

    SetFocus(TradesListBox());
}


// ========================================================================================
// Select a line in the Listbox and deselect any other lines that do not match this
// new line's Trade pointer.
// ========================================================================================
bool CActiveTrades::SelectListBoxItem(HWND hListBox, int idx) {
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
    ShowListBoxItem(idx);

    // Check to see if other lines in the listbox are selected. We will deselect those other
    // lines if they do not belong to the current trade being selected.
    // If the line being clicked on is the main header line for the trade (LineType::ticker_line)
    // then we deselected everything else including legs of this same trade. We do this
    // because there are different popup menu actions applicable to the main header line
    // and/or the individual trade legs.
    LineType current_selected_line_type = ld->line_type;

    int nCount = ListBox_GetSelCount(hListBox);

    if (nCount) {
        int* selected_items = new int[nCount]();
        SendMessage(hListBox, LB_GETSELITEMS, (WPARAM)nCount, (LPARAM)selected_items);

        for (int i = 0; i < nCount; ++i)
        {
            ld = (ListBoxData*)ListBox_GetItemData(hListBox, selected_items[i]);
            if (ld != nullptr) {
                if (ld->trade != trade) {
                    ListBox_SetSel(hListBox, false, selected_items[i]);
                }
                else {
                    // If selecting items within the same Trade then do not allow
                    // mixing selections of the header line and individual legs.
                    if (current_selected_line_type != ld->line_type) {
                        ListBox_SetSel(hListBox, false, selected_items[i]);
                    }
                }
            }
        }

        delete[] selected_items;
    }

    AfxRedrawWindow(hListBox);

    return true;
}


// ========================================================================================
// Expire the selected legs. Basically, ask for confirmation via a messagebox and 
// then take appropriate action.
// ========================================================================================
void CActiveTrades::ExpireSelectedLegs(auto trade) {
    // Do a check to ensure that there is actually legs selected to expire. It could
    // be that the user select SHARES or other non-options underlyings only.

    if (tdd.legs.size() == 0) {
        CustomMessageBox.Show(
            MainWindow.hWindow,
            L"No valid option legs have been selected for expiration.",
            L"Warning",
            MB_ICONWARNING | MB_OK);
        return;
    }
        
    int res = CustomMessageBox.Show(
        MainWindow.hWindow,
        L"Are you sure you wish to EXPIRE the selected legs?",
        L"Confirm",
        MB_ICONWARNING | MB_YESNOCANCEL | MB_DEFBUTTON2);

    if (res != IDYES) return;


    std::shared_ptr<Transaction> trans = std::make_shared<Transaction>();

    trans->description = L"Expiration";
    trans->underlying = Underlying::Options;
    trade->transactions.push_back(trans);

    for (const auto& leg : tdd.legs) {

        // Save this transaction's leg quantities
        std::shared_ptr<Leg> newleg = std::make_shared<Leg>();

        trade->nextleg_id += 1;
        newleg->leg_id = trade->nextleg_id;

        newleg->underlying = trans->underlying;

        trans->trans_date = leg->expiry_date;
        trans->quantity = 1;   // must have something > 0 otherwise "Quantity error" if saving an Edit
        newleg->original_quantity = leg->open_quantity * -1;
        newleg->open_quantity = 0;
        newleg->leg_back_pointer_id = leg->leg_id;
        leg->open_quantity = 0;

        if (leg->action == Action::STO) newleg->action = Action::BTC;
        if (leg->action == Action::BTO) newleg->action = Action::STC; 

        newleg->expiry_date = leg->expiry_date;
        newleg->strike_price = leg->strike_price;
        newleg->put_call = leg->put_call;
        trans->legs.push_back(newleg);
    }

    // Set the open status of the entire trade based on the new modified legs
    trade->SetTradeOpenStatus();

    // Rebuild the openLegs position vector
    trade->CreateOpenLegsVector();

    // Save the new data to the database
    db.SaveDatabase();

    // Reload the trade list
    ShowActiveTrades();
}


// ========================================================================================
// Create Transaction for Shares or Futures that have been called away.
// ========================================================================================
void CActiveTrades::CalledAwayAssignment(
    auto trade, auto leg, int aggregate_shares, int aggregate_futures)
{
    std::shared_ptr<Transaction> trans;
    std::shared_ptr<Leg> newleg;

    bool is_shares = (config.IsFuturesTicker(trade->ticker_symbol)) ? false : true;

    int quantity_assigned{0};
    std::wstring quantity_assigned_text;
    std::wstring strike_price_text;
    std::wstring long_short_text;

    int leg_quantity{0};
    double multiplier = 1;

    std::wstring msg = L"Continue with OPTION ASSIGNMENT?\n\n";

    if (is_shares) {
        quantity_assigned = min(abs(leg->open_quantity * 100), abs(aggregate_shares));
        leg_quantity = quantity_assigned / 100;
        quantity_assigned_text = std::to_wstring(quantity_assigned);
        strike_price_text = L" shares called away at $" + leg->strike_price + L" per share.";
        multiplier = 1;
    }
    else {
        quantity_assigned = min(abs(leg->open_quantity), abs(aggregate_futures));
        leg_quantity = quantity_assigned;
        quantity_assigned_text = std::to_wstring(quantity_assigned);
        strike_price_text = L" futures called away at $" + leg->strike_price + L" per future.";
        multiplier = trade->multiplier;
    }
    leg_quantity = (leg->open_quantity < 0) ? leg_quantity * -1 : leg_quantity;

    if (Assignment.ShowModal(
        long_short_text,
        quantity_assigned_text,
        strike_price_text) == DIALOG_RETURN_CANCEL) {
        return;
    }

    // Retreive the Quantity amount that was set by the Assignment modal when OK was pressed.
    quantity_assigned = AfxValInteger(quantity_set_from_assignment_modal);

    // Close the Option. Save this transaction's leg quantities
    trans = std::make_shared<Transaction>();
    trans->trans_date = AfxCurrentDate();
    trans->description = L"Called away";
    trans->underlying = Underlying::Options;
    trade->transactions.push_back(trans);

    newleg = std::make_shared<Leg>();
    trade->nextleg_id += 1;
    newleg->leg_id = trade->nextleg_id;
    newleg->underlying = trans->underlying;
    newleg->open_quantity = 0;

    if (is_shares) {
        newleg->original_quantity = (quantity_assigned / 100);
        leg->open_quantity = (leg->open_quantity + (quantity_assigned / 100));;
    }
    else {
        newleg->original_quantity = quantity_assigned;
        leg->open_quantity = (leg->open_quantity + quantity_assigned);;
    }
    newleg->leg_back_pointer_id = leg->leg_id;

    if (leg->action == Action::STO) newleg->action = Action::BTC;
    if (leg->action == Action::BTO) newleg->action = Action::STC;

    newleg->expiry_date = AfxCurrentDate();
    newleg->strike_price = leg->strike_price;
    newleg->put_call = leg->put_call;
    trans->legs.push_back(newleg);


    // Remove the SHARES/FUTURES that have been called away.
    trans = std::make_shared<Transaction>();
    trans->trans_date = AfxCurrentDate();
    trans->description = L"Called away";
    trans->underlying = (is_shares) ? Underlying::Shares : Underlying::Futures;
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

    if (leg->put_call == PutCall::Put) {
        newleg->action = Action::BTC;
        newleg->original_quantity = quantity_assigned;
        newleg->open_quantity = quantity_assigned;
        trans->total = trans->quantity * multiplier * trans->price * -1;  // DR
        trade->acb = trade->acb + trans->total;
    }
    else {
        newleg->action = Action::STC;
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
    db.SaveDatabase();

    // Reload the trade list
    ShowActiveTrades();
}


// ========================================================================================
// Create Transaction for option assignment for the selected leg.
// ========================================================================================
void CActiveTrades::CreateAssignment(auto trade, auto leg) {
    std::shared_ptr<Transaction> trans;
    std::shared_ptr<Leg> newleg;

    bool is_shares = (trade->ticker_symbol.substr(0,1) == L"/") ? false : true;

    int quantity_assigned{0};
    std::wstring quantity_assigned_text;
    std::wstring strike_price_text;
    double multiplier = 1;

    std::wstring long_short_text = (leg->put_call == PutCall::Put) ? L"LONG " : L"SHORT ";
        
    if (is_shares) {
        quantity_assigned = abs(leg->open_quantity * 100);
        quantity_assigned_text = std::to_wstring(quantity_assigned);
        strike_price_text = L" shares at $" + leg->strike_price + L" per share.";
        multiplier = 1;
    }
    else {
        quantity_assigned = abs(leg->open_quantity);
        quantity_assigned_text = std::to_wstring(quantity_assigned);
        strike_price_text = L" futures at $" + leg->strike_price + L" per future.";
        multiplier = trade->multiplier;
    }

    if (Assignment.ShowModal(
        long_short_text, 
        quantity_assigned_text, 
        strike_price_text) == DIALOG_RETURN_CANCEL) {
        return;
    }

    // Retreive the Quantity amount that was set by the Assignment modal when OK was pressed.
    quantity_assigned = AfxValInteger(quantity_set_from_assignment_modal);

    // Close the Option. Save this transaction's leg quantities
    trans = std::make_shared<Transaction>();
    trans->trans_date = leg->expiry_date;
    trans->quantity = 1;   // must have something > 0 otherwise "Quantity error" if saving an Edit
    trans->description = L"Assignment";
    trans->underlying = Underlying::Options;
    trade->transactions.push_back(trans);

    newleg = std::make_shared<Leg>();
    trade->nextleg_id += 1;
    newleg->leg_id = trade->nextleg_id;
    newleg->underlying = trans->underlying;

    newleg->open_quantity = 0;

    if (is_shares) {
        newleg->original_quantity = (quantity_assigned / 100);
        leg->open_quantity = (leg->open_quantity + (quantity_assigned / 100));;
    }
    else {
        newleg->original_quantity = quantity_assigned;
        leg->open_quantity = (leg->open_quantity + quantity_assigned);;
    }
    newleg->leg_back_pointer_id = leg->leg_id;

    if (leg->action == Action::STO) newleg->action = Action::BTC;
    if (leg->action == Action::BTO) newleg->action = Action::STC;

    newleg->expiry_date = leg->expiry_date; 
    newleg->strike_price = leg->strike_price;
    newleg->put_call = leg->put_call;
    trans->legs.push_back(newleg);

    // Make the SHARES/FUTURES that have been assigned.
    trans = std::make_shared<Transaction>();
    trans->trans_date = leg->expiry_date;
    trans->description = L"Assignment";
    trans->underlying = (is_shares) ? Underlying::Shares : Underlying::Futures;
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

    if (leg->put_call == PutCall::Put) {
        newleg->action = Action::BTO;
        newleg->original_quantity = quantity_assigned;
        newleg->open_quantity = quantity_assigned;
        trans->total = trans->quantity * multiplier * trans->price * -1;  // DR
        trade->acb = trade->acb + trans->total;
    }
    else {
        newleg->action = Action::STO;
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
    db.SaveDatabase();

    // Reload the trade list
    ShowActiveTrades();
}


// ========================================================================================
// Create Transaction for option assignment for the selected leg.
// ========================================================================================
void CActiveTrades::OptionAssignment(auto trade) {
    // Do a check to ensure that there is actually a leg selected for assignment. 
    if (tdd.legs.size() == 0) {
        CustomMessageBox.Show(
            MainWindow.hWindow,
            L"No valid option leg has been selected for assignment.",
            L"Warning",
            MB_ICONWARNING | MB_OK);
        return;
    }

    // Check to see if Shares/Futures exist that could be "called away". If yes, then process
    // that type of transaction rather than adding a new assigned to transaction.
    int aggregate_shares{0};
    int aggregate_futures{0};

    for (const auto& trans : trade->transactions) {
        for (const auto& leg : trans->legs) {
            if (leg->underlying == Underlying::Shares) {
                aggregate_shares += leg->open_quantity;
            }
            else if (leg->underlying == Underlying::Futures) {
                aggregate_futures += leg->open_quantity;
            }
        }
    }

    auto leg = tdd.legs.at(0);

    // Are LONG SHARES or LONG FUTURES being called away
    if ((aggregate_shares > 0 || aggregate_futures > 0) && leg->put_call == PutCall::Call) {
        CalledAwayAssignment(trade, leg, aggregate_shares, aggregate_futures);
        return;
    }
    // Are SHORT SHARES or SHORT FUTURES being called away
    if ((aggregate_shares < 0 || aggregate_futures < 0) && leg->put_call == PutCall::Put) {
        CalledAwayAssignment(trade, leg, aggregate_shares, aggregate_futures);
        return;
    }

    // Otherwise, Option is being assigned Shares or Futures
    CreateAssignment(trade, leg);
}


// ========================================================================================
// Populate vector that holds all selected lines/legs. This will be passed to the 
// TradeDialog in order to perform actions on the legs.
// ========================================================================================
void CActiveTrades::PopulateLegsEditVector(HWND hListBox) {
    tdd.legs.clear();

    int num_selected = ListBox_GetSelCount(hListBox);
    tdd.legs.reserve(num_selected);

    if (num_selected) {
        int* sel_items = new int[num_selected]();
        SendMessage(hListBox, LB_GETSELITEMS, (WPARAM)num_selected, (LPARAM)sel_items);

        for (int i = 0; i < num_selected; i++) {
            ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, sel_items[i]);
            if (ld) {
                // Only allow Option legs to be pushed to legEdit
                if (ld->leg != nullptr) {
                    tdd.legs.push_back(ld->leg);
                }
            }
        }

        delete[] sel_items;
    }
}


// ========================================================================================
// Handle the right-click popup menu on the ListBox's selected lines.
// ========================================================================================
void CActiveTrades::RightClickMenu(HWND hListBox, int idx) {
    std::wstring text;
    std::wstring plural_text;
    int sel_count = ListBox_GetSelCount(hListBox);

    std::shared_ptr<Trade> trade = nullptr;

    bool is_tickerLine = false;

    // Clear the tdd module global trade variable
    tdd.ResetDefaults();

    ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, idx);
    if (ld->trade == nullptr) return;

    trade = ld->trade;
    tdd.trade = ld->trade;
    tdd.trans = ld->trans;
    tdd.shares_aggregate_edit = ld->aggregate_shares;

    if (sel_count == 1) {
        // Is this the Trade header line
        if (ld != nullptr) {
            if (ld->line_type == LineType::ticker_line) {
                is_tickerLine = true;
            }
        }
    }
    else {
        plural_text = L"s";
    }


    std::vector<CCustomPopupMenuItem> items;

    if (ld->line_type == LineType::options_leg) {
        text = L"Roll Leg" + plural_text;
        items.push_back({ text, (int)TradeAction::roll_leg, false });

        text = L"Close Leg" + plural_text;
        items.push_back({ text, (int)TradeAction::close_leg, false });

        text = L"Expire Leg" + plural_text;
        items.push_back({ text, (int)TradeAction::expire_leg, false });

        if (sel_count == 1) {
            items.push_back({ L"", (int)TradeAction::no_action , true});
            items.push_back({ L"Option Assignment", (int)TradeAction::assignment, false });
        }
        items.push_back({ L"", (int)TradeAction::no_action, true });
    }

    if (ld->line_type == LineType::shares) {
        text = L"Manage Shares";
        items.push_back({ text, (int)TradeAction::manage_shares, false });
        items.push_back({ L"", (int)TradeAction::no_action, true });
    }

    if (ld->line_type == LineType::futures) {
        text = L"Manage Futures";
        items.push_back({ text, (int)TradeAction::manage_futures, false });
        items.push_back({ L"", (int)TradeAction::no_action, true });
    }

    items.push_back({ L"Add Options to Trade", (int)TradeAction::add_options_to_trade, false });
    items.push_back({ L"Add Put to Trade", (int)TradeAction::add_put_to_trade, false });
    items.push_back({ L"Add Call to Trade", (int)TradeAction::add_call_to_trade, false });
    items.push_back({ L"", (int)TradeAction::no_action, true });

    if (config.IsFuturesTicker(trade->ticker_symbol)) {
        items.push_back({ L"Add Futures to Trade", (int)TradeAction::add_futures_to_trade, false });
    }
    else {
        items.push_back({ L"Add Shares to Trade", (int)TradeAction::add_shares_to_trade, false });
        items.push_back({ L"Add Dividend to Trade", (int)TradeAction::add_dividend_to_trade, false });
    }


    POINT pt; GetCursorPos(&pt);
    TradeAction selected = (TradeAction)CustomPopupMenu.Show(hListBox, items, -1, pt.x, pt.y);

    switch (selected) {
    case TradeAction::manage_shares:
    case TradeAction::manage_futures:
    case TradeAction::add_shares_to_trade:
    case TradeAction::add_dividend_to_trade:
    case TradeAction::add_futures_to_trade:
    case TradeAction::add_options_to_trade:
    case TradeAction::add_put_to_trade:
    case TradeAction::add_call_to_trade:
        TradeDialog_Show(selected);
        break;

    case TradeAction::roll_leg:
    case TradeAction::close_leg:
        PopulateLegsEditVector(hListBox);
        TradeDialog_Show(selected);
        break;

    case TradeAction::expire_leg:
        PopulateLegsEditVector(hListBox);
        ExpireSelectedLegs(trade);
        break;

    case TradeAction::assignment:
        PopulateLegsEditVector(hListBox);
        OptionAssignment(trade);
        break;
    }
}


// ========================================================================================
// Listbox subclass Window procedure
// ========================================================================================
LRESULT CALLBACK CActiveTrades::ListBox_SubclassProc(
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
        HWND hCustomVScrollBar = GetDlgItem(GetParent(hwnd), IDC_ACTIVETRADES_CUSTOMVSCROLLBAR);
        int zdelta = GET_WHEEL_DELTA_WPARAM(wParam);
        //int top_index = (int)SendMessage(hwnd, LB_GETTOPINDEX, 0, 0);
        accum_delta += zdelta;
        if (accum_delta >= 120) {     // scroll up 3 lines
            CustomVScrollBar_ScrollLines(hCustomVScrollBar, -3);
            accum_delta = 0;
        }
        else {
            if (accum_delta <= -120) {     // scroll down 3 lines
                CustomVScrollBar_ScrollLines(hCustomVScrollBar, 3);
                accum_delta = 0;
            }
        }
        CustomVScrollBar_Recalculate(hCustomVScrollBar);
        return 0;
    }

    case WM_RBUTTONDOWN: {
        int idx = Listbox_ItemFromPoint(hwnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        
        // The return value contains the index of the nearest item in the LOWORD. The HIWORD is zero 
        // if the specified point is in the client area of the list box, or one if it is outside the 
        // client area.
        if (HIWORD(idx) == 1) break;
            
        // Return to not select the line (eg. if a blank line was clicked on)
        if (SendMessage(GetParent(hwnd), MSG_ACTIVETRADES_SELECTLISTBOXITEM, idx, 0) == false) {
            return 0;
        }
        ListBox_SetSel(hwnd, true, idx);

        
        SendMessage(GetParent(hwnd), MSG_ACTIVETRADES_RIGHTCLICKMENU, idx, 0);
        return 0;
    }

    case WM_LBUTTONDOWN: {
        int idx = Listbox_ItemFromPoint(hwnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        // The return value contains the index of the nearest item in the LOWORD. The HIWORD is zero 
        // if the specified point is in the client area of the list box, or one if it is outside the 
        // client area.
        if (HIWORD(idx) == 1) break;
        SendMessage(GetParent(hwnd), MSG_ACTIVETRADES_SELECTLISTBOXITEM, idx, 0);
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
            Color back_color(COLOR_GRAYDARK);
            SolidBrush back_brush(back_color);
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
// Process WM_MEASUREITEM message for window/dialog: ActiveTrades
// ========================================================================================
void CActiveTrades::OnMeasureItem( HWND hwnd, MEASUREITEMSTRUCT* lpMeasureItem) {
    lpMeasureItem->itemHeight = AfxScaleY(ACTIVETRADES_LISTBOX_ROWHEIGHT);
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: ActiveTrades
// ========================================================================================
bool CActiveTrades::OnEraseBkgnd( HWND hwnd,  HDC hdc) {
    // Handle all of the painting in WM_PAINT
    return true;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: ActiveTrades
// ========================================================================================
void CActiveTrades::OnPaint(HWND hwnd) {
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    Graphics graphics(hdc);

    // Create the background brush
    Color back_color(COLOR_BLACK);
    SolidBrush back_brush(back_color);

    // Paint the background using brush.
    int width = (ps.rcPaint.right - ps.rcPaint.left);
    int height = (ps.rcPaint.bottom - ps.rcPaint.top);
    
    graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, width, height);

    // Paint the area to the left of the ListBox in order to give the illusion
    // of a margin before the ListBox data is displyed.
    ps.rcPaint.top += AfxScaleY(ACTIVETRADES_MARGIN);

    // Set the background brush
    back_color.SetValue(COLOR_GRAYDARK);
    back_brush.SetColor(back_color);
    width = (ps.rcPaint.right - ps.rcPaint.left);
    height = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, width, height);

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Show/Hide the Net and Excess liquidity labels and values.
// ========================================================================================
int CActiveTrades::ShowHideLiquidityLabels() {
    int nShow = (config.GetAllowPortfolioDisplay()) ? SW_SHOW : SW_HIDE;
    if (!tws_IsConnected()) nShow = SW_HIDE;

    ShowWindow(NetLiquidationLabel(), nShow);
    ShowWindow(NetLiquidationValueLabel(), nShow);
    ShowWindow(ExcessLiquidityLabel(), nShow);
    ShowWindow(ExcessLiquidityValueLabel(), nShow);
    ShowWindow(MaintenanceLabel(), nShow);
    ShowWindow(MaintenanceValueLabel(), nShow);

    return nShow;
}

    
 // ========================================================================================
// Process WM_SIZE message for window/dialog: ActiveTrades
// ========================================================================================
void CActiveTrades::OnSize(HWND hwnd,  UINT state, int cx, int cy) {
        
    int margin = AfxScaleY(ACTIVETRADES_MARGIN);

    // If no entries exist for the ListBox then don't show any child controls
    int show_flag = (ListBox_GetCount(TradesListBox()) <= 1) ? SWP_HIDEWINDOW : SWP_SHOWWINDOW;


    // Do not call the calcVThumbRect() function during a scrollbar move. This WM_SIZE
    // gets triggered when the ListBox WM_DRAWITEM fires. If we do another calcVThumbRect()
    // calculation then the scrollbar will appear "jumpy" under the user's mouse cursor.
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
    int custom_scrollbar_width = (show_scrollbar) ? AfxScaleX(CUSTOMVSCROLLBAR_WIDTH) : 0;

    int left = AfxScaleY(APP_LEFTMARGIN_WIDTH);
    int top = 0;
    int width = AfxScaleX(120);
    int height = AfxScaleY(16);

    HDWP hdwp = BeginDeferWindowPos(12);

    // Position the Warning label
    DeferWindowPos(hdwp, PaperTradingLabel(), 0, 0, top, cx, height, SWP_NOZORDER);

    height = AfxScaleY(23);
    top = height;
    hdwp = DeferWindowPos(hdwp, SortFilterLabel(), 0, left, top, width, height, SWP_NOZORDER | SWP_SHOWWINDOW);

    top += height;
    width = AfxScaleX(200);
    hdwp = DeferWindowPos(hdwp, SortFilterCombo(), 0, left, top, width, height, SWP_NOZORDER | SWP_SHOWWINDOW);
    left += width;
    width = AfxScaleX(23);
    hdwp = DeferWindowPos(hdwp, SortFilterButton(), 0, left, top, width, height, SWP_NOZORDER | SWP_SHOWWINDOW);

    top = height;
    height = AfxScaleY(16);
    left = AfxScaleX(280);
    width = AfxScaleX(80);
    hdwp = DeferWindowPos(hdwp, NetLiquidationLabel(), 0, left, top, width, height, SWP_NOZORDER);

    left = AfxScaleX(360);
    width = AfxScaleX(60);
    hdwp = DeferWindowPos(hdwp, NetLiquidationValueLabel(), 0, left, top, width, height, SWP_NOZORDER);

    top = top + AfxScaleY(17);
    left = AfxScaleX(280);
    width = AfxScaleX(80);
    hdwp = DeferWindowPos(hdwp, ExcessLiquidityLabel(), 0, left, top, width, height, SWP_NOZORDER);

    left = AfxScaleX(360);
    width = AfxScaleX(60);
    hdwp = DeferWindowPos(hdwp, ExcessLiquidityValueLabel(), 0, left, top, width, height, SWP_NOZORDER);

    top = top + AfxScaleY(17);
    height = AfxScaleY(16);
    left = AfxScaleX(280);
    width = AfxScaleX(80);
    hdwp = DeferWindowPos(hdwp, MaintenanceLabel(), 0, left, top, width, height, SWP_NOZORDER);

    left = AfxScaleX(360);
    width = AfxScaleX(60);
    hdwp = DeferWindowPos(hdwp, MaintenanceValueLabel(), 0, left, top, width, height, SWP_NOZORDER);
    

    height = AfxScaleY(23);
    top = height;
    left = AfxScaleY(470);
    width = AfxScaleX(120);
    hdwp = DeferWindowPos(hdwp, NewTradeLabel(), 0, left, top, width, height, SWP_NOZORDER | SWP_SHOWWINDOW);

    top += height;
    width = AfxScaleX(200);
    hdwp = DeferWindowPos(hdwp, NewTradeCombo(), 0, left, top, width, height, SWP_NOZORDER | SWP_SHOWWINDOW);
    left += width;
    width = AfxScaleX(23);
    hdwp = DeferWindowPos(hdwp, NewTradeButton(), 0, left, top, width, height, SWP_NOZORDER | SWP_SHOWWINDOW);

    top = margin;
    left = AfxScaleY(APP_LEFTMARGIN_WIDTH);
    height = cy - top;
    width = cx - left - custom_scrollbar_width;
    hdwp = DeferWindowPos(hdwp, TradesListBox(), 0, left, top, width, height, SWP_NOZORDER | show_flag);
    
    left += width;   // right edge of ListBox
    width = custom_scrollbar_width;
    hdwp = DeferWindowPos(hdwp, VScrollBar(), 0, left, top, width, height, SWP_NOZORDER | show_flag);

    EndDeferWindowPos(hdwp);

    ShowHideLiquidityLabels();
}


// ========================================================================================
// Get the text for the specified SortFilter index.
// ========================================================================================
std::wstring CActiveTrades::GetSortFilterDescription(int idx) {
    switch ((ActiveTradesFilterType)idx) {
    case ActiveTradesFilterType::Category: return L"By Category";
    case ActiveTradesFilterType::Expiration: return L"By Days to Expiration (DTE)";
    case ActiveTradesFilterType::TickerSymbol: return L"By Ticker Symbol and DTE";
    default: return L"";
    }
}

// ========================================================================================
// Get the text for the specified NewTrade index.
// ========================================================================================
std::wstring CActiveTrades::GetNewTradeDescription(int idx) {
    switch ((NewTradeType)idx) {
    case NewTradeType::Custom: return L"Custom Options Trade";
    case NewTradeType::IronCondor: return L"Iron Condor";
    case NewTradeType::ShortStrangle: return L"Short Strangle";
    case NewTradeType::ShortPut: return L"Short Put";
    case NewTradeType::ShortPutVertical: return L"Short Put Vertical";
    case NewTradeType::ShortCall: return L"Short Call";
    case NewTradeType::ShortCallVertical: return L"Short Call Vertical";
    case NewTradeType::ShortPut112: return L"Short Put 112";
    case NewTradeType::SharesTrade: return L"Shares Trade";
    case NewTradeType::FuturesTrade: return L"Futures Trade";
    case NewTradeType::OtherIncomeExpense: return L"Other Income/Expense";
    default: return L"";
    }
}


// ========================================================================================
// Display Paper Trading warning message if port is enabled. 
// Normal Trading 7496;   7497 is paper trading account.
// ========================================================================================
void CActiveTrades::DisplayPaperTradingWarning() {
    HWND hCtrl = GetDlgItem(ActiveTrades.hWindow, IDC_ACTIVETRADES_PAPERWARNING);
    CustomLabel_SetText(hCtrl, L"*** USING PAPER TRADING ACCOUNT ***");
    ShowWindow(hCtrl, SW_SHOWNORMAL);
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: ActiveTrades
// ========================================================================================
bool CActiveTrades::OnCreate(HWND hwnd,  LPCREATESTRUCT lpCreateStruct) {
    hWindow = hwnd;

    std::wstring font_name = AfxGetDefaultFont();
    int font_size = 8;

    // Create a Warning label at top of the window to display paper trading warning message.
    HWND hCtl = CustomLabel_SimpleLabel(hwnd, IDC_ACTIVETRADES_PAPERWARNING, L"", COLOR_YELLOW, COLOR_RED,
        CustomLabelAlignment::middle_center, 0, 0, 0, 0);
    ShowWindow(hCtl, SW_HIDE);

    // SORTFILTER SELECTOR
    CustomLabel_SimpleLabel(hwnd, IDC_ACTIVETRADES_LBLSORTFILTER, L"Sort Filter",
        COLOR_WHITEDARK, COLOR_BLACK);

    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_ACTIVETRADES_SORTFILTER, GetSortFilterDescription((int)ActiveTradesFilterType::Category),
        COLOR_WHITEDARK, COLOR_GRAYMEDIUM, COLOR_GRAYMEDIUM, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_left, 0, 0, 0, 0);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);
    CustomLabel_SetMousePointer(hCtl, CustomLabelPointer::hand, CustomLabelPointer::hand);
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_ACTIVETRADES_CMDSORTFILTER, GLYPH_DROPDOWN,
        COLOR_WHITEDARK, COLOR_GRAYMEDIUM, COLOR_GRAYLIGHT, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_center, 0, 0, 0, 0);
    CustomLabel_SetFont(hCtl, font_name, font_size, true);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);


    // PORTFOLIO LIQUIDATION VALUES
    CustomLabel_SimpleLabel(hwnd, IDC_ACTIVETRADES_NETLIQUIDATION, L"Net Liq:", COLOR_WHITEDARK, COLOR_BLACK);
    CustomLabel_SimpleLabel(hwnd, IDC_ACTIVETRADES_NETLIQUIDATION_VALUE, L"", COLOR_WHITELIGHT, COLOR_BLACK);
    CustomLabel_SimpleLabel(hwnd, IDC_ACTIVETRADES_EXCESSLIQUIDITY, L"Excess Liq:", COLOR_WHITEDARK, COLOR_BLACK);
    CustomLabel_SimpleLabel(hwnd, IDC_ACTIVETRADES_EXCESSLIQUIDITY_VALUE, L"", COLOR_WHITELIGHT, COLOR_BLACK);
    CustomLabel_SimpleLabel(hwnd, IDC_ACTIVETRADES_MAINTENANCE, L"Maintenance:", COLOR_WHITEDARK, COLOR_BLACK);
    CustomLabel_SimpleLabel(hwnd, IDC_ACTIVETRADES_MAINTENANCE_VALUE, L"", COLOR_WHITELIGHT, COLOR_BLACK);


    // NEW TRADE SELECTOR
    CustomLabel_SimpleLabel(hwnd, IDC_ACTIVETRADES_LBLNEWTRADE, L"New Trade",
        COLOR_WHITEDARK, COLOR_BLACK);
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_ACTIVETRADES_NEWTRADE, GetNewTradeDescription((int)NewTradeType::Custom),
        COLOR_WHITEDARK, COLOR_GRAYMEDIUM, COLOR_GRAYMEDIUM, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_left, 0, 0, 0, 0);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);
    CustomLabel_SetMousePointer(hCtl, CustomLabelPointer::hand, CustomLabelPointer::hand);
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_ACTIVETRADES_CMDNEWTRADE, GLYPH_DROPDOWN,
        COLOR_WHITEDARK, COLOR_GRAYMEDIUM, COLOR_GRAYLIGHT, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_center, 0, 0, 0, 0);
    CustomLabel_SetFont(hCtl, font_name, font_size, true);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);


    // Create an Ownerdraw fixed row sized listbox that we will use to custom
    // paint our various open trades.
    hCtl =
        ActiveTrades.AddControl(Controls::ListBox, hwnd, IDC_ACTIVETRADES_LISTBOX, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_EXTENDEDSEL | 
            LBS_OWNERDRAWFIXED | LBS_NOTIFY | LBS_WANTKEYBOARDINPUT,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)ListBox_SubclassProc,
            IDC_ACTIVETRADES_LISTBOX, NULL);
    ListBox_AddString(hCtl, NULL);
        
    // Create our custom vertical scrollbar and attach the ListBox to it.
    CreateCustomVScrollBar(hwnd, IDC_ACTIVETRADES_CUSTOMVSCROLLBAR, hCtl, Controls::ListBox);

    return true;
}


// ========================================================================================
// Process WM_COMMAND message for window/dialog: ActiveTrades
// ========================================================================================
void CActiveTrades::OnCommand( HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {
    switch (codeNotify) {

    case LBN_SELCHANGE: {
        if (id == IDC_ACTIVETRADES_LISTBOX) {
            int nCurSel = ListBox_GetCurSel(hwndCtl);
            SelectListBoxItem(hwndCtl, nCurSel);
        }
        break;
    }

    }
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CActiveTrades::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    static bool portfolio_requested = false;
    
    switch (msg) {
        HANDLE_MSG(m_hwnd, WM_CREATE, OnCreate);
        HANDLE_MSG(m_hwnd, WM_COMMAND, OnCommand);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, OnPaint);
        HANDLE_MSG(m_hwnd, WM_SIZE, OnSize);
        HANDLE_MSG(m_hwnd, WM_MEASUREITEM, OnMeasureItem);
        HANDLE_MSG(m_hwnd, WM_DRAWITEM, ListBoxData_OnDrawItem);
            
    case MSG_POSITIONS_READY: {
        // Request Positions has completed and has sent this notification so 
        // we can now start requesting the portfolio updates real time data.
        if (!portfolio_requested) {
            client.RequestPortfolioUpdates();
            portfolio_requested = true;
        }
        return 0;
    }

    case MSG_ACTIVETRADES_SELECTLISTBOXITEM: {
        return SelectListBoxItem(TradesListBox(), (int)wParam);
    }

    case MSG_ACTIVETRADES_RIGHTCLICKMENU: {
        RightClickMenu(TradesListBox(), (int)wParam);
        return 0;
    }

    case MSG_CUSTOMLABEL_CLICK: {
        HWND hCtl = (HWND)lParam;
        int ctrl_id = (int)wParam;

        if (!hCtl) return 0;

        if (ctrl_id == IDC_ACTIVETRADES_CMDSORTFILTER || ctrl_id == IDC_ACTIVETRADES_SORTFILTER) {
            std::vector<CCustomPopupMenuItem> items;
            for (int i = (int)ActiveTradesFilterType::Category; i <= (int)ActiveTradesFilterType::TickerSymbol; ++i) {
                items.push_back({ GetSortFilterDescription(i), i, false });
            }

            // Position the popup menu immediately under the control that was clicked on
            HWND hCombo = SortFilterCombo();
            int top_offset = AfxScaleY(1);
            RECT rc{}; GetWindowRect(hCombo, &rc);
            POINT pt{ rc.left, rc.bottom + top_offset };
            int selected = CustomPopupMenu.Show(hWindow, items, (int)filter_type, pt.x, pt.y, AfxScaleX(223));

            if (selected != -1 && selected != (int)filter_type) {
                filter_type = (ActiveTradesFilterType)selected;
                CustomLabel_SetText(hCombo, GetSortFilterDescription(selected));
                ShowActiveTrades();
            }
        }

        if (ctrl_id == IDC_ACTIVETRADES_CMDNEWTRADE || ctrl_id == IDC_ACTIVETRADES_NEWTRADE) {
            std::vector<CCustomPopupMenuItem> items;
            int i = 0;
            i = (int)NewTradeType::Custom;             items.push_back({ GetNewTradeDescription(i), i, false });
            i = (int)NewTradeType::no_action;          items.push_back({ L"", i, true });
            i = (int)NewTradeType::IronCondor;         items.push_back({ GetNewTradeDescription(i), i, false });
            i = (int)NewTradeType::ShortStrangle;      items.push_back({ GetNewTradeDescription(i), i, false });
            i = (int)NewTradeType::ShortPut;           items.push_back({ GetNewTradeDescription(i), i, false });
            i = (int)NewTradeType::ShortPutVertical;   items.push_back({ GetNewTradeDescription(i), i, false });
            i = (int)NewTradeType::ShortCall;          items.push_back({ GetNewTradeDescription(i), i, false });
            i = (int)NewTradeType::ShortCallVertical;  items.push_back({ GetNewTradeDescription(i), i, false });
            i = (int)NewTradeType::ShortPut112;        items.push_back({ GetNewTradeDescription(i), i, false });
            i = (int)NewTradeType::no_action;          items.push_back({ L"", i, true });
            i = (int)NewTradeType::SharesTrade;        items.push_back({ GetNewTradeDescription(i), i, false });
            i = (int)NewTradeType::FuturesTrade;       items.push_back({ GetNewTradeDescription(i), i, false });
            i = (int)NewTradeType::no_action;          items.push_back({ L"", i, true });
            i = (int)NewTradeType::OtherIncomeExpense; items.push_back({ GetNewTradeDescription(i), i, false });


            // Position the popup menu immediately under the control that was clicked on
            HWND hCombo = NewTradeCombo();
            int top_offset = AfxScaleY(1);
            RECT rc{}; GetWindowRect(hCombo, &rc);
            POINT pt{ rc.left, rc.bottom + top_offset };
            int selected = CustomPopupMenu.Show(hWindow, items, (int)new_trade_type, pt.x, pt.y, AfxScaleX(223));

            if (selected != -1) {
                new_trade_type = (NewTradeType)selected;
                CustomLabel_SetText(hCombo, GetNewTradeDescription(selected));

                switch (new_trade_type) {
                case NewTradeType::Custom:
                    TradeDialog_Show(TradeAction::new_options_trade);
                    break;
                case NewTradeType::IronCondor:
                    TradeDialog_Show(TradeAction::new_iron_condor);
                    break;
                case NewTradeType::ShortStrangle:
                    TradeDialog_Show(TradeAction::new_short_strangle);
                    break;
                case NewTradeType::ShortPut:
                    TradeDialog_Show(TradeAction::new_short_put);
                    break;
                case NewTradeType::ShortCall:
                    TradeDialog_Show(TradeAction::new_short_call);
                    break;
                case NewTradeType::ShortPutVertical:
                    TradeDialog_Show(TradeAction::new_short_put_vertical);
                    break;
                case NewTradeType::ShortCallVertical:
                    TradeDialog_Show(TradeAction::new_short_call_vertical);
                    break;
                case NewTradeType::ShortPut112:
                    TradeDialog_Show(TradeAction::new_short_put_LT112);
                    break;
                case NewTradeType::SharesTrade:
                    TradeDialog_Show(TradeAction::new_shares_trade);
                    break;
                case NewTradeType::FuturesTrade:
                    TradeDialog_Show(TradeAction::new_futures_trade);
                    break;
                case NewTradeType::OtherIncomeExpense:
                    TradeDialog_Show(TradeAction::other_income_expense);
                    break;
                }
            }
        }
        return 0;
    }

    }

    return DefWindowProc(m_hwnd, msg, wParam, lParam);
}

