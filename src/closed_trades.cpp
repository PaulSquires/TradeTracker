#include "imgui.h"

#include <algorithm>
#include <map>

#include "appstate.h"
#include "list_panel.h"
#include "list_panel_data.h"
#include "tab_panel.h"
#include "utilities.h"
#include "filter_panel.h"
#include "trade_history.h"
#include "closed_trades.h"


void SetFirstLineClosedTrades(AppState& state, std::vector<CListPanelData>& vec) {
    // Reset the selected trade whose Trade History will be shown for the first
    // selectable line in the list.
    state.closedtrades_selected_trade = nullptr;

    // ensure all other lines are not selected
    for (auto& ld : vec) {
        ld.is_selected = false;
    }

    // If the database was reloaded then attempt to reposition to the row that
    // selected prior tot he reload.
    if (state.db.is_previously_loaded && (state.closedtrades_current_row_index != -1)) {
        for (int i = 0; i < vec.size(); ++i) {
            if (i == state.closedtrades_current_row_index) {
                vec.at(i).is_selected = true;
                state.closedtrades_selected_trade = vec.at(i).trade;
                state.selected_tabpanelitem = TabPanelItem::ClosedTrades;
                SelectTabPanelItem(state);
                break;
            }
        }
    }

    // Finally, if still no selection then use the first selectable line in the grid.
    if (!state.closedtrades_selected_trade) {
        for (auto& ld : vec) {
            if (!state.closedtrades_selected_trade && ld.line_type == LineType::ticker_line) {
                ld.is_selected = true;
                state.closedtrades_selected_trade = ld.trade;
                state.selected_tabpanelitem = TabPanelItem::ClosedTrades;
                SelectTabPanelItem(state);
                break;
            }
        }
    }
}


void LoadClosedTradesData(AppState& state, std::vector<CListPanelData>& vec) {
    std::string start_date = state.filterpanel_start_date;
    std::string end_date = state.filterpanel_end_date;
    std::string ticker = state.filterpanel_ticker_symbol;
    int selected_category = state.filterpanel_selected_category;

    if (selected_category == CATEGORY_END + 1) selected_category = CATEGORY_OTHER;
    if (selected_category == CATEGORY_END + 2) selected_category = CATEGORY_ALL;

    struct ClosedData {
        std::shared_ptr<Trade> trade;
        std::shared_ptr<Transaction> trans;
        std::string description;
        std::string closed_date;
        double close_amount = 0;
    };

    std::vector<ClosedData> vectorClosed;
    vectorClosed.reserve(1000);         // reserve space for 1000 closed trades

    // Look at all the closed trades as well as open trades that have shares/futures

    for (auto& trade : state.db.trades) {

        if (ticker.length() && ticker != trade->ticker_symbol) continue;

        if (selected_category != CATEGORY_ALL) {
            if (trade->category != selected_category) continue;
        }

        // Iterate to find the latest closed date
        std::string latest_closed_date;
        for (auto& trans : trade->transactions) {
            if (trans->trans_date > latest_closed_date) {
                latest_closed_date = trans->trans_date;
            }
        } 

        if (latest_closed_date > end_date) continue;

        // If this Trade has Shares/Futures transactions show the costing
        bool exclude_acb_non_shares = true;

        for (const auto& share : trade->shares_history) {
            if (share.leg_action == Action::STC || share.leg_action == Action::BTC) {

                ClosedData data;
                data.trade = trade;
                //data.trans = trans;
                data.closed_date = share.trans->trans_date;

                int quantity = std::abs(share.open_quantity);
                double price = share.trans->price;

                if (state.config.IsFuturesTicker(trade->ticker_symbol)) {
                    double multiplier = AfxValDouble(state.config.GetMultiplier(trade->ticker_symbol));
                    price *= multiplier;
                }

                std::string diff_describe;
                double diff = 0;
                if (share.leg_action == Action::STC) {
                    diff = (price + share.average_cost);
                    diff_describe = AfxMoney(price, 2, state) + "-" + AfxMoney(std::abs(share.average_cost), 2, state);
                }
                if (share.leg_action == Action::BTC) {
                    diff = (share.average_cost - price);
                    diff_describe = AfxMoney(std::abs(share.average_cost), 2, state) + "-" + AfxMoney(price, 2, state);
                }

                data.close_amount = quantity * diff;

                std::string describe = (share.trans->underlying == Underlying::Shares) ? " shares @ $" : " futures @ $";
                data.description = std::to_string(quantity) + describe + AfxMoney(diff, 2, state) +
                    " (" + diff_describe + ")";

                vectorClosed.push_back(data);

                // If this closed Trade had shares/futures and average cost with options included then
                // we set the flag here to bypass outputting the acb_non_shares amount because that amount
                // would have been factored into the average cost of the shares.
                exclude_acb_non_shares = state.config.exclude_nonstock_costs;
            }

        }

        if (trade->is_open == false && exclude_acb_non_shares == true) {
            if (trade->acb_non_shares) {
                ClosedData data;
                data.trade = trade;
                data.trans = nullptr;
                data.closed_date = latest_closed_date;
                data.close_amount = trade->acb_non_shares;
                data.description = trade->ticker_name;
                if (trade->ticker_symbol == "OTHER") data.description = trade->transactions[0]->description;
                if (state.config.IsFuturesTicker(trade->ticker_symbol)) data.description += " (" + AfxFormatFuturesDate(trade->future_expiry) + ")";
                vectorClosed.push_back(data);
            }
        }
    }

    // Destroy any existing ListPanel line data
    vec.clear();
    vec.reserve(128);

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

    std::string today_date = AfxCurrentDate();
    int today_year = AfxGetYear(today_date);
    std::string week_start_date = AfxDateAddDays(today_date, -AfxLocalDayOfWeek());
    std::string week_end_date = AfxDateAddDays(week_start_date, 6);
    std::string current_date = "";

    for (const auto& ClosedData : vectorClosed) {
        current_month = AfxGetMonth(ClosedData.closed_date);
        current_year = AfxGetYear(ClosedData.closed_date);

        if (ClosedData.closed_date < start_date && current_year != today_year) continue;

        if (ClosedData.closed_date < start_date &&
            current_year == today_year) {
            if (ClosedData.close_amount >= 0) ++year_win;
            if (ClosedData.close_amount < 0) ++year_loss;
            YTD += ClosedData.close_amount;
            continue;
        }

        if (subtotal_month != current_month && subtotal_month != 0) {
            ListPanelData_OutputClosedMonthSubtotal(state, vec, current_date, subtotal_amount, current_month_win, current_month_loss);
            current_date = ClosedData.closed_date;
            subtotal_amount = 0;
            current_month_win = 0;
            current_month_loss = 0;
        }

        current_date = ClosedData.closed_date;
        subtotal_month = current_month;
        if (ClosedData.closed_date >= start_date) {
            subtotal_amount += ClosedData.close_amount;
        }
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
        ListPanelData_OutputClosedPosition(state, vec, ClosedData.trade,
            ClosedData.closed_date, ClosedData.trade->ticker_symbol, ClosedData.description, ClosedData.close_amount);
     }
    if (subtotal_amount != 0) {
        ListPanelData_OutputClosedMonthSubtotal(state, vec, current_date, subtotal_amount, current_month_win, current_month_loss);
    }

    ListPanelData_OutputClosedDayTotal(state, vec, daily_amount, day_win, day_loss);
    ListPanelData_OutputClosedWeekTotal(state, vec, weekly_amount, week_win, week_loss);
    ListPanelData_OutputClosedMonthTotal(state, vec, monthly_amount, month_win, month_loss);
    ListPanelData_OutputClosedYearTotal(state, vec, today_year, YTD, year_win, year_loss);

    // If no closed trades exist then add at least one line
    if (vec.size() == 0) {
        ListPanelData_AddBlankLine(state, vec);
    }

    state.is_closedtrades_data_loaded = true;
}


void ShowClosedTrades(AppState& state) {
    if (!state.show_closedtrades) return;

    static std::vector<CListPanelData> vec;
    static std::vector<CListPanelData> vecHeader;

    static CListPanel lp;

    ImGui::BeginChild("ClosedTrades", ImVec2(state.left_panel_width, state.top_panel_height));

    // Must call ShowFilterPanel here prior to initializing the closed trades data because
    // ShowFilterPanel makes a call to SetFilterPanelStartEndDates which sets the dates
    // that the initialization code needs in order to properly filter the trades.
    ShowFilterPanel(state);

    if (!state.is_closedtrades_data_loaded) {
        lp.table_id = TableType::closed_trades;
        lp.is_left_panel = true;
        lp.table_flags = ImGuiTableFlags_ScrollY;
        lp.column_count = 7;
        lp.vec = &vec;
        lp.vecHeader = nullptr;
        lp.header_backcolor = 0;
        lp.header_height = 0;
        lp.row_height = 12;
        lp.min_col_widths = nClosedMinColWidth;
        LoadClosedTradesData(state, vec);

        // Default to display the Trade History for the first entry in the list.
        SetFirstLineClosedTrades(state, vec);

        state.tradehistory_trade = state.closedtrades_selected_trade;
        SetTradeHistoryTrade(state, state.closedtrades_selected_trade);
}

    ImGui::BeginGroup();
    lp.panel_width = ImGui::GetContentRegionAvail().x;
    lp.panel_height = ImGui::GetContentRegionAvail().y;
    DrawListPanel(state, lp);
    ImGui::EndGroup();

    ImGui::EndChild();
}

