/*

MIT License

Copyright(c) 2023-2025 Paul Squires

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

#include <memory>
#include <string>
#include <unordered_map>

#include "appstate.h"
#include "list_panel_data.h"
#include "questionbox.h"
#include "tws-client.h"
#include "active_trades_actions.h"
#include "utilities.h"

#if defined(_WIN32) // win32 and win64
#include <dwmapi.h>
#endif


// #include <iostream>


// Unfortunately these structures that have to
// be made global because the data needs to be updated in the TwsClient::tickPrice
// and TwsClient::updatePortfolio functions which are callbacks from the
// Interactive Brokers library (therefore I can't pass AppState into it).
// These maps are instantiated in tws-client.cpp
extern std::unordered_map<TickerId, TickerData> mapTickerData;
extern std::unordered_map<int, PortfolioData> mapPortfolioData;
extern double netliq_value;
extern double excessliq_value;
extern double maintenance_value;
extern bool is_connection_ready_for_data;
extern bool is_positions_ready_for_data;
extern bool positionEnd_fired;

enum class CurrentActivePanel {
    ActiveTrades,
    ClosedTrades,
    Transactions
};


void ReloadAppState(AppState& state) {
    // Prevent any current active connection from updating pointers
    // while the program is in the process of resetting everything.
    state.is_pause_market_data = true;

    // Save the active panel so that it can be reloaded after the database is reloaded.
    CurrentActivePanel current_active_panel;
    if (state.show_activetrades) current_active_panel = CurrentActivePanel::ActiveTrades;
    if (state.show_closedtrades) current_active_panel = CurrentActivePanel::ClosedTrades;
    if (state.show_transpanel) current_active_panel = CurrentActivePanel::Transactions;


    // Reset some state flags
    state.is_activetrades_data_loaded = false;
    state.is_closedtrades_data_loaded = false;
    state.is_transactions_data_loaded = false;
    state.is_tradehistory_data_loaded = false;
    state.is_journalnotes_data_loaded = false;
    state.is_transedit_data_loaded = false;

    state.show_activetrades = false;
    state.show_closedtrades = false;
    state.show_transpanel = false;
    state.show_transedit = false;
    state.show_tradehistory = false;
    state.show_journalnotes = false;
    state.show_reconciliation_popup = false;
    state.show_messagebox_popup = false;
    state.show_questionbox_popup = false;
    state.show_gettext_popup = false;
    state.show_assignment_popup = false;
    state.show_settingsdialog_popup = false;
    state.show_yearenddialog_popup = false;
    state.show_categoriesdialog_popup = false;
    state.show_importdialog_popup = false;
    state.show_tradedialog_popup = false;
    state.show_activetrades_rightclickmenu = false;
    state.show_journalfolders_rightclickmenu = false;
    state.show_journalnotes_rightclickmenu = false;

    // Attempt to apply the standard Windows dark theme to the non-client areas of the main form.
#if defined(_WIN32) // win32 and win64
    BOOL value = (state.config.color_theme == ColorThemeType::Dark) ? TRUE : FALSE;
    HWND hwnd = GetForegroundWindow();
    if (hwnd) {
        ::DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));
    }
#endif

    // Save the new data
    state.db.SaveDatabase(state);

    // Ensure that any previously requested Market Data is cancelled because the
    // ticker_id will have changed when the Trades are reloaded from the database.
    for (const auto& [key, value] : mapTickerData) {
        tws_CancelMarketData(state, key);
    }

    // Clear the ticker data map because the ticker id's will change when
    // the database is reloaded.
    mapTickerData.clear();
    state.ticker_id = 1;    // reset counter


    // Load the refeshed data
    state.db.LoadDatabase(state);

    // Ensure that the previously selected panel gets reloaded
    if (current_active_panel == CurrentActivePanel::ActiveTrades) state.show_activetrades = true;
    if (current_active_panel == CurrentActivePanel::ClosedTrades) state.show_closedtrades = true ;
    if (current_active_panel == CurrentActivePanel::Transactions) state.show_transpanel = true;

    state.db.is_previously_loaded = true;    // to allow reposition to previously selected row
    is_connection_ready_for_data = true;
    positionEnd_fired = false;
    is_positions_ready_for_data = false;

    state.stop_monitor_thread_requested = false;
    state.stop_ticker_update_thread_requested = false;

    // Allow price/market data to flow again.
    state.is_pause_market_data = false;

    // Set the color palatte (dark or light)
    state.initialize_imgui_state();

}


void UpdateTickerPortfolioLine(AppState& state, int index, int index_trade) {
    ImU32 theme_color = clrTextDarkWhite(state);

    std::string text;

    // Update the Trade's tickerLine with the new totals
    std::vector<CListPanelData>* vec = static_cast<std::vector<CListPanelData>*>(state.vecActiveTrades);
    CListPanelData* ld = &vec->at(index_trade);
    if (ld && ld->trade) {

        double value_aggregate{0};
        if (ld->trade->aggregate_shares) value_aggregate = ld->trade->aggregate_shares;
        if (ld->trade->aggregate_futures) value_aggregate = ld->trade->aggregate_futures;

        double multiplier = 1;
        if (state.config.IsFuturesTicker(ld->trade->ticker_symbol)) {
            multiplier = AfxValDouble(state.config.GetMultiplier(ld->trade->ticker_symbol));
        }

        double acb = (value_aggregate) ? ld->trade->acb_shares : ld->trade->acb_total;

        double trade_acb = acb;
        double shares_market_value = value_aggregate * ld->trade->ticker_last_price * multiplier;
        double total_cost = shares_market_value;

        for (const auto& leg : ld->trade->open_legs) {
            total_cost += leg->market_value;
        }

        theme_color = clrTextDarkWhite(state);

        text = AfxMoney(trade_acb, ld->trade->ticker_decimals, state);
        ld->SetTextData(COLUMN_TICKER_PORTFOLIO_1, text, theme_color);

        text = AfxMoney(total_cost, ld->trade->ticker_decimals, state);
        ld->SetTextData(COLUMN_TICKER_PORTFOLIO_2, text, theme_color);

        double difference = trade_acb + total_cost;
        theme_color = (difference < 0) ? clrRed(state) : clrGreen(state);
        text = AfxMoney(difference, 2, state);
        ld->SetTextData(COLUMN_TICKER_PORTFOLIO_3, text, theme_color);

        double percentage = (trade_acb == 0) ? 0 : difference / trade_acb * 100;
        if (difference < 0) {
            if (percentage >= 0) percentage *= -1;
        }
        else {
            if (percentage <= 0) percentage *= -1;
        }
        text = AfxMoney(percentage, 0, state) + " %";
        theme_color = (difference < 0) ? clrRed(state) : clrGreen(state);
        ld->SetTextData(COLUMN_TICKER_PORTFOLIO_4, text, theme_color);

        // Save the Trade's profit percentage complete so that it can be used for sorting
        // when the application is connected to TWS.
        ld->trade->trade_profit_percentage = percentage;
    }
}


void UpdateLegPortfolioLine(AppState& state, int index, CListPanelData* ld) {
    ImU32 theme_color = clrTextDarkWhite(state);

    std::string text;

    if (ld->line_type == LineType::shares ||
        ld->line_type == LineType::futures &&
        ld->trade) {

        // SHARES/FUTURES MARKET VALUE
        if (ld->trade->aggregate_shares || ld->trade->aggregate_futures) {
            double value_aggregate = 0;
            if (ld->trade->aggregate_shares) value_aggregate = ld->trade->aggregate_shares;
            if (ld->trade->aggregate_futures) value_aggregate = ld->trade->aggregate_futures;

            double shares_cost = ld->trade->acb_shares;
            if (shares_cost == 0) shares_cost = 1;
            text = AfxMoney(shares_cost, ld->trade->ticker_decimals, state);
            ld->SetTextData(COLUMN_TICKER_PORTFOLIO_1, text, theme_color);

            double multiplier = 1;
            if (state.config.IsFuturesTicker(ld->trade->ticker_symbol)) {
                multiplier = AfxValDouble(state.config.GetMultiplier(ld->trade->ticker_symbol));
            }

            double shares_market_value = value_aggregate * ld->trade->ticker_last_price * multiplier;
            text = AfxMoney(shares_market_value, ld->trade->ticker_decimals, state);
            ld->SetTextData(COLUMN_TICKER_PORTFOLIO_2, text, theme_color);

            // DELTA
            theme_color = clrTextDarkWhite(state);
            text = "1.00";   // shares are always 1 delta
            ld->SetTextData(COLUMN_OPTIONLEG_DELTA, text, theme_color);

            // UNREALIZED PNL
            double unrealized_pnl = (shares_cost + shares_market_value);
            theme_color = (unrealized_pnl < 0) ? clrRed(state) : clrGreen(state);
            text = AfxMoney(unrealized_pnl, ld->trade->ticker_decimals, state);
            ld->SetTextData(COLUMN_TICKER_PORTFOLIO_3, text, theme_color);    // Unrealized profit or loss

            // UNREALIZED PNL PERCENTAGE
            double percentage = (unrealized_pnl / shares_cost * 100);
            if (unrealized_pnl >= 0) {
                percentage = std::abs(percentage);
            }
            else {
                // percentage must also be negative
                if (percentage > 0) percentage *= -1;
            }
            theme_color = (percentage < 0) ? clrRed(state) : clrGreen(state);
            text = AfxMoney(percentage, 0, state) + " %";
            ld->SetTextData(COLUMN_TICKER_PORTFOLIO_4, text, theme_color);  // Percentage values for the previous two columns data
        }
    }

    if (ld->line_type == LineType::options_leg && ld->leg) {
        // Lookup the most recent Portfolio position data
        PortfolioData pd{};
        bool found = false;
        if (mapPortfolioData.count(ld->leg->contract_id)) {
            pd = mapPortfolioData.at(ld->leg->contract_id);
            found = true;
        }

        if (!found) return;


        // OPTION LEG DELTA
        theme_color = clrTextDarkWhite(state);
        text = "";
        // Lookup the most recent Option position Delta
        if (mapTickerData.count(ld->leg->ticker_id)) {
            text = AfxMoney(mapTickerData.at(ld->leg->ticker_id).delta, 2, state);
        }
        ld->SetTextData(COLUMN_OPTIONLEG_DELTA, text, theme_color);   // Option leg Delta

        // POSITION COST
        double position_cost = ld->leg->open_quantity * pd.average_cost;
        theme_color = clrTextDarkWhite(state);
        text = AfxMoney(position_cost, ld->trade->ticker_decimals, state);
        ld->SetTextData(COLUMN_TICKER_PORTFOLIO_1, text, theme_color);   // Book Value and average Price

        // MARKET VALUE
        theme_color = clrTextDarkWhite(state);
        double multiplier = AfxValDouble(state.config.GetMultiplier(ld->trade->ticker_symbol));
        double market_value = (pd.market_price * ld->leg->open_quantity * multiplier);
        ld->leg->market_value = market_value;
        text = AfxMoney(market_value, ld->trade->ticker_decimals, state);
        ld->SetTextData(COLUMN_TICKER_PORTFOLIO_2, text, theme_color);

        // UNREALIZED PNL
        double unrealized_pnl = (market_value - position_cost);
        ld->leg->unrealized_pnl = unrealized_pnl;
        theme_color = (unrealized_pnl < 0) ? clrRed(state) : clrGreen(state);
        text = AfxMoney(unrealized_pnl, ld->trade->ticker_decimals, state);
        ld->SetTextData(COLUMN_TICKER_PORTFOLIO_3, text, theme_color);    // Unrealized profit or loss

        // UNREALIZED PNL PERCENTAGE
        double percentage = (position_cost == 0) ? 0 : (unrealized_pnl / position_cost * 100);
        if (unrealized_pnl >= 0) {
            percentage = std::abs(percentage);
        }
        else {
            // percentage must also be negative
            if (percentage > 0) percentage *= -1;
        }
        theme_color = (percentage < 0) ? clrRed(state) : clrGreen(state);
        ld->leg->percentage = percentage;
        text = AfxMoney(percentage, 0, state) + " %";
        ld->SetTextData(COLUMN_TICKER_PORTFOLIO_4, text, theme_color);  // Percentage values for the previous two columns data

        ld->SetTextData(COLUMN_TICKER_PORTFOLIO_5, "", clrTextDarkWhite(state));
    }
}


void PerformITMcalculation(AppState& state, std::shared_ptr<Trade>& trade) {
    // Puts have aready been sorted before calls by strike price
    const ImU32 RED = clrRed(state);
    const ImU32 GREEN = clrGreen(state);
    int ITMCOLOR = 0;

    double strike_price = 0;

    // Iterate forward over the open legs to process the Puts but iterate in
    // reverse to process the Calls in order to properly ensure that the ITM
    // color is not reset if strike price is fully through a vertical spread.
    for (const auto& leg : trade->open_legs) {
        if (leg->underlying != Underlying::Options) continue;
        if (leg->put_call != PutCall::Put) continue;
        strike_price = AfxValDouble(leg->strike_price);
        if (trade->ticker_last_price < strike_price) {
            if (leg->open_quantity < 0) ITMCOLOR = RED;    // credit
            if (leg->open_quantity > 0) ITMCOLOR = GREEN;  // debit
        }
    }

    // Process the Calls in reverse
    for (int i = trade->open_legs.size() - 1; i >= 0; --i) {
        auto leg = trade->open_legs[i];
        if (leg->underlying != Underlying::Options) continue;
        if (leg->put_call != PutCall::Call) continue;
        strike_price = AfxValDouble(leg->strike_price);
        if (trade->ticker_last_price > strike_price) {
            if (leg->open_quantity < 0) ITMCOLOR = RED;    // credit
            if (leg->open_quantity > 0) ITMCOLOR = GREEN;  // debit
        }
    }

    trade->itm_text = (ITMCOLOR ? "ITM" : "");
    trade->itm_color = (ITMCOLOR ? ITMCOLOR : clrTextLightWhite(state));
}


void UpdateTickerPricesLine(AppState& state, int index, CListPanelData* ld) {
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
    double delta = 0;
    if (ld->trade->ticker_close_price != 0) {
        delta = (ld->trade->ticker_last_price - ld->trade->ticker_close_price);
    }

    std::string text;
    ImU32 theme_color = clrTextLightWhite(state);

    // Calculate if any of the option legs are ITM in a good (green) or bad (red) way.
    // We use a separate function call because scrapped data will need acces to the
    // ITM calculation also.
    PerformITMcalculation(state, ld->trade);

    ld->SetTextData(COLUMN_TICKER_ITM, ld->trade->itm_text, ld->trade->itm_color);  // ITM

    text = AfxMoney(delta, ld->trade->ticker_decimals, state);
    theme_color = (delta >= 0) ? clrGreen(state) : clrRed(state);
    ld->trade->ticker_column_1 = text;
    ld->trade->ticker_column_1_clr = theme_color;
    ld->SetTextData(COLUMN_TICKER_CHANGE, text, theme_color);  // price change

    text = AfxMoney(ld->trade->ticker_last_price, ld->trade->ticker_decimals, state);
    ld->trade->ticker_column_2 = text;
    ld->trade->ticker_column_1_clr = clrTextLightWhite(state);
    ld->SetTextData(COLUMN_TICKER_CURRENTPRICE, text, clrTextLightWhite(state));  // current price

    text = ((ld->trade->ticker_last_price == 0) ? "0.00" : AfxMoney((delta / ld->trade->ticker_last_price) * 100, 2, state)) + "%";
    if (delta > 0) text = "+" + text;
    theme_color = (delta >= 0) ? clrGreen(state) : clrRed(state);
    ld->trade->ticker_column_3 = text;
    ld->trade->ticker_column_3_clr = theme_color;
    ld->SetTextData(COLUMN_TICKER_PERCENTCHANGE, text, theme_color);  // price percentage change
}


// ========================================================================================
// Update the ActiveTrades list with the most up to date Market Price Data.
// Also updates leg Portfolio position data and option leg deltas.
// This function is called from the TickerUpdateFunction() thread.
// ========================================================================================
void UpdateTickerPrices(AppState& state) {
    if (state.is_pause_market_data) return;

    // Guard to prevent re-entry of update should the thread fire the update
    // prior to this function finishing.
    static std::atomic<bool> is_processing = false;

    std::vector<CListPanelData>* vec = static_cast<std::vector<CListPanelData>*>(state.vecActiveTrades);

    if (vec->size() == 0) return;
    if (is_processing) return;

    is_processing = true;

    int index_trade = 0;

    for (int index = 0; index < vec->size(); ++index) {
        CListPanelData* ld = &vec->at(index);
        if (ld == (void*)-1) continue;
        if (ld == nullptr) continue;

        if (ld->line_type == LineType::ticker_line) {
            index_trade = index;
            UpdateTickerPricesLine(state, index, ld);
        }

        UpdateLegPortfolioLine(state, index, ld);
        UpdateTickerPortfolioLine(state, index, index_trade);
    }

    is_processing = false;
}


// ========================================================================================
// Expire the selected legs. Basically, ask for confirmation via a messagebox and
// then take appropriate action.
// ========================================================================================
void ExpireSelectedLegs(AppState& state) {
    std::shared_ptr<Trade> trade = state.activetrades_selected_trade;

    std::shared_ptr<Transaction> trans = std::make_shared<Transaction>();

    trans->description = "Expiration";
    trans->underlying = Underlying::Options;
    trade->transactions.push_back(trans);

    for (auto& leg : state.activetrades_selected_legs) {

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

    // Save/Load the new data to the database
    ReloadAppState(state);
}


void AskExpireSelectedLegs(AppState& state) {
    // Do a check to ensure that there is actually legs selected to expire. It could
    // be that the user select SHARES or other non-options underlyings only.
    if (state.activetrades_selected_legs.size() == 0) {
        CustomQuestionBox(state, "Warning", "No valid option legs have been selected for expiration.", QuestionCallback::None, true);
        return;
    }

    CustomQuestionBox(state, "Confirm", "Are you sure you wish to EXPIRE the selected legs?", QuestionCallback::ExpireSelectedLegs);
}
