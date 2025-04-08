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

#include <string>

#include "appstate.h"
#include "utilities.h"
#include "list_panel_data.h"

//#include <iostream>


// ========================================================================================
// Create the display data for a Category Header line
// ========================================================================================
void ListPanelData_AddCategoryHeader(AppState& state, std::vector<CListPanelData>& vec, const std::shared_ptr<Trade>& trade, int num_trades_category) {
    CListPanelData ld;

    std::string plural_trades = (num_trades_category == 1 ? " trade)" : " trades)");
    std::string text = AfxUpper(state.config.GetCategoryDescription(trade->category)) +
        " (" + std::to_string(num_trades_category) + plural_trades;

    ld.SetData(0, nullptr, -1, text, StringAlignment::left, clrBackDarkGray(state), clrBlue(state), 9, true);
    ld.line_type = LineType::category_header;

    vec.push_back(ld);
}


// ========================================================================================
// Add a simple No Trades Exist message
// ========================================================================================
void ListBoxData_NoTradesExistMessage(AppState& state, std::vector<CListPanelData>& vec) {
    CListPanelData ld;
    std::string text = "No locally created Trades exist.";
    ld.line_type = LineType::category_header;
    ld.SetData(0, nullptr, -1, text, StringAlignment::left, clrBackDarkGray(state), clrBlue(state), 9, true);
    vec.push_back(ld);

    text = "Connect to TWS in order to start the Trades importing process.";
    ld.line_type = LineType::category_header;
    ld.SetData(0, nullptr, -1, text, StringAlignment::left, clrBackDarkGray(state), clrBlue(state), 9, true);
    vec.push_back(ld);
}


// ========================================================================================
// Create the display data for BP, Days,ROI (for a Trade History trade).
// ========================================================================================
void ListPanelData_TradeROI(AppState& state, std::vector<CListPanelData>& vec, const std::shared_ptr<Trade>& trade, TickerId ticker_id) {
    CListPanelData ld;

    ld.line_type = LineType::history_roi;

    int font9 = 9;
    std::string text;

    std::string start_date = AfxInsertDateHyphens(trade->bp_start_date);
    std::string end_date = AfxInsertDateHyphens(trade->bp_end_date);

    // Buying Power
    text = AfxMoney(trade->trade_bp, 0, state);
    ld.SetData(2, trade, ticker_id, text, StringAlignment::right, clrBackDarkGray(state),
        clrTextLightWhite(state), font9, false);
    text = "BP";
    ld.SetData(3, trade, ticker_id, text, StringAlignment::left, clrBackDarkGray(state),
        clrTextDarkWhite(state), font9, false);
    
    // Days In Trade
    int days_in_trade = AfxDaysBetween(start_date, (trade->is_open ? AfxCurrentDate() : end_date));
    text = AfxMoney(days_in_trade, 0, state);
    ld.SetData(4, trade, ticker_id, text, StringAlignment::right, clrBackDarkGray(state),
        clrTextLightWhite(state), font9, false);
    text = "DIT";
    ld.SetData(5, trade, ticker_id, text, StringAlignment::left, clrBackDarkGray(state),
        clrTextDarkWhite(state), font9, false);

    vec.push_back(ld);

    // Totals Days for Trade
    int days_total = AfxDaysBetween(start_date, end_date);
    text = AfxMoney(days_total, 0, state);
    ld.SetData(2, trade, ticker_id, text, StringAlignment::right, clrBackDarkGray(state),
        clrTextLightWhite(state), font9, false);
    text = "Days";
    ld.SetData(3, trade, ticker_id, text, StringAlignment::left, clrBackDarkGray(state),
        clrTextDarkWhite(state), font9, false);
    
    // ROI% per 30 days  
    text = AfxMoney(0, 1, state) + "%";
    if (days_total == 0) days_total = 1;
    if (trade->trade_bp != 0 && days_total != 0) {
        text = AfxMoney((trade->acb_total / trade->trade_bp * 100 / days_total * 30), 1, state) + "%";
    }
    ld.SetData(4, trade, ticker_id, text, StringAlignment::right, clrBackDarkGray(state),
        clrTextLightWhite(state), font9, false);
    text = "30d";
    ld.SetData(5, trade, ticker_id, text, StringAlignment::left, clrBackDarkGray(state),
        clrTextDarkWhite(state), font9, false);

    vec.push_back(ld);

    ListPanelData_AddBlankLine(state, vec);
}

    
// ========================================================================================
// Create the display data for an Open Position that displays in Trades & History tables.
// ========================================================================================
void ListPanelData_OpenPosition(
    AppState& state, std::vector<CListPanelData>& vec, const std::shared_ptr<Trade>& trade, const bool is_history) {
    
    TickerId ticker_id = -1;

    CListPanelData ld;

    int font8 = 9;   // debugging see how 9pt looks on all O/S
    int font9 = 9;
    ImU32 clr = clrTextLightWhite(state);
    std::string text;

    bool has_shares = (trade->aggregate_shares || trade->aggregate_futures) ? true : false;
    double acb = trade->total_share_profit + trade->acb_total; 

    if (is_history) {
        ticker_id = -1;
        ld.line_type = LineType::nonselectable;

        ld.SetData(0, trade, ticker_id, GLYPH_TREEOPEN, StringAlignment::center, clrBackDarkGray(state),
            clrTextDarkWhite(state), font8, false);

        text = (trade->is_open ? "Open Pos" : "Closed Pos");
        ld.SetData(1, trade, ticker_id, text, StringAlignment::center, clrBackDarkGray(state),
            clrOrange(state), font9, false);   // orange

        text = AfxMoney(acb, 2, state);
        clr = (acb >= 0) ? clrGreen(state) : clrRed(state);
        ld.SetData(7, trade, ticker_id, text, StringAlignment::center, clrBackDarkGray(state),
            clr, font9, false);
    }
    else {
        ld.line_type = LineType::ticker_line;
        ticker_id = (trade->ticker_id == -1) ? ++state.ticker_id : trade->ticker_id;
        
        ld.SetData(0, trade, ticker_id, GLYPH_CIRCLE, StringAlignment::center, clrBackDarkGray(state),
            clrTextDarkWhite(state), font9, false);
        ld.SetData(1, trade, ticker_id, trade->ticker_symbol, StringAlignment::left, clrBackDarkGray(state),
            clrTextLightWhite(state), font9, true);

        text = "";

        text = (trade->itm_text.length()) ? trade->itm_text : "";
        clr = (trade->itm_color != clrTextLightWhite(state)) ? trade->itm_color : clrTextLightWhite(state);
        ld.SetData(COLUMN_TICKER_ITM, trade, ticker_id, text, StringAlignment::center, clrBackDarkGray(state),
            clr, font9, false);   // ITM
        ld.SetData(3, trade, ticker_id, "", StringAlignment::left, clrBackDarkGray(state),
            clrTextLightWhite(state), font9, false);
        ld.SetData(4, trade, ticker_id, "", StringAlignment::left, clrBackDarkGray(state),
            clrTextLightWhite(state), font9, false);

        text = trade->ticker_column_1;
        clr = trade->ticker_column_1_clr;
        ld.SetData(COLUMN_TICKER_CHANGE, trade, ticker_id, text, StringAlignment::right, clrBackDarkGray(state),
            clr, font9, false);   // price change

        text = trade->ticker_column_2;
        clr = trade->ticker_column_2_clr;
        ld.SetData(COLUMN_TICKER_CURRENTPRICE, trade, ticker_id, text, StringAlignment::center, clrBackDarkGray(state),
            clr, font9, true);   // current price

        text = trade->ticker_column_3;
        clr = trade->ticker_column_3_clr;
        ld.SetData(COLUMN_TICKER_PERCENTCHANGE, trade, ticker_id, text, StringAlignment::left, clrBackDarkGray(state),
            clr, font9, false);   // price percentage change

        text = "";
        clr = clrTextDarkWhite(state);
        ld.SetData(COLUMN_OPTIONLEG_DELTA, trade, ticker_id, text, StringAlignment::right, clrBackDarkGray(state),
            clrTextDarkWhite(state), font9, false);   

        ld.SetData(COLUMN_TICKER_PORTFOLIO_1, trade, ticker_id, text, StringAlignment::right, clrBackDarkGray(state),
            clrTextDarkWhite(state), font9, false);   

        ld.SetData(COLUMN_TICKER_PORTFOLIO_2, trade, ticker_id, text, StringAlignment::right, clrBackDarkGray(state),
            clrTextDarkWhite(state), font9, false);   

        ld.SetData(COLUMN_TICKER_PORTFOLIO_3, trade, ticker_id, text, StringAlignment::right, clrBackDarkGray(state),
            clrTextDarkWhite(state), font9, false);   

        ld.SetData(COLUMN_TICKER_PORTFOLIO_4, trade, ticker_id, text, StringAlignment::right, clrBackDarkGray(state),
            clrTextDarkWhite(state), font9, false);
        
        ld.SetData(COLUMN_TICKER_PORTFOLIO_5, trade, ticker_id, text, StringAlignment::right, clrBackDarkGray(state),
            clrTextDarkWhite(state), font9, false);
    }
    vec.push_back(ld);


    // If the Trade is closed then there is nothing to output as an "open position".
    if (!trade->is_open) return;

    std::string dot = GLYPH_CIRCLE;
    ImU32 dte_color = clrMagenta(state);

    // // *** SHARES/FUTURES ***
    if (has_shares) {
        CListPanelData ld;
        ticker_id = -1;

        std::string text_shares;
        std::string text_aggregate;
        double value_aggregate = 0;

        if (trade->aggregate_futures) {
            ld.line_type = LineType::futures;
            text_shares = "FUTURES: " + AfxFormatFuturesDate(trade->future_expiry);
            text_aggregate = std::to_string(trade->aggregate_futures);
            value_aggregate = trade->aggregate_futures;
        }

        if (trade->aggregate_shares) {
            ld.line_type = LineType::shares;
            text_shares = "SHARES";
            text_aggregate = std::to_string(trade->aggregate_shares);
            value_aggregate = trade->aggregate_shares;
        }

        if (is_history) ld.line_type = LineType::nonselectable;

        ld.SetData(0, trade, ticker_id, "", StringAlignment::left, clrBackDarkGray(state),
            clrTextLightWhite(state), font9, false);

        int col = 1;
        if (!is_history) {
            ld.SetData(col, trade, ticker_id, dot, StringAlignment::right, clrBackDarkGray(state),
                clrMagenta(state), font9, false);
            col++;
        }
        col++;

        ld.SetData(col, trade, ticker_id, text_shares, StringAlignment::left, clrBackMediumGray(state),
            clrTextDarkWhite(state), font9, false);
        col++;

        ld.SetData(col, trade, ticker_id, "", StringAlignment::right, clrBackMediumGray(state),
            clrTextDarkWhite(state), font9, false);
        col++;

        ld.SetData(col, trade, ticker_id, "", StringAlignment::right, clrBackMediumGray(state),
            clrTextDarkWhite(state), font9, false);
        col++;

        if (is_history) {
            text = "";
            ld.SetData(col, trade, ticker_id, "", StringAlignment::right, clrBackMediumGray(state),
                clrTextDarkWhite(state), font9, false);
            col++;
        }

        text = AfxMoney(std::abs(trade->acb_shares / value_aggregate), 2, state);
        ld.SetData(col, trade, ticker_id, text, StringAlignment::center, clrBackMediumGray(state),
            clrTextDarkWhite(state), font9, false);
        col++;

        text = text_aggregate;
        ld.aggregate_shares = text;
        ld.SetData(col, trade, ticker_id, text, StringAlignment::center, clrBackMediumGray(state),
            clrTextDarkWhite(state), font9, false);
        col++;

        text = "";
        ld.SetData(col, trade, ticker_id, text, StringAlignment::right, clrBackDarkGray(state),
            clrBackMediumGray(state), font9, false);
        col++;

        text = "";
        ld.SetData(col, trade, ticker_id, text, StringAlignment::right, clrBackDarkGray(state),
            clrTextDarkWhite(state), font9, false);
        col++;

        text = "";
        ld.SetData(col, trade, ticker_id, text, StringAlignment::right, clrBackDarkGray(state),
            clrTextDarkWhite(state), font9, false);
        col++;

        text = "";
        ld.SetData(col, trade, ticker_id, text, StringAlignment::right, clrBackDarkGray(state),
            clrTextDarkWhite(state), font9, false);
        col++;

        text = "";
        ld.SetData(col, trade, ticker_id, text, StringAlignment::right, clrBackDarkGray(state),
            clrTextDarkWhite(state), font9, false);

        vec.push_back(ld);
    }


    // *** OPTION LEGS ***
    for (auto& leg : trade->open_legs) {
        if (leg->underlying == Underlying::Options) {
            CListPanelData ld;
            ticker_id = (leg->ticker_id == -1) ? ++state.ticker_id : leg->ticker_id;

            ld.leg = leg;
            ld.line_type = LineType::options_leg;
            if (is_history) ld.line_type = LineType::nonselectable;

            std::string current_date = AfxCurrentDate();
            std::string expiry_date = leg->expiry_date;
            std::string short_date = AfxShortDate(expiry_date);
            std::string dte_text;

            
            // Check the Trade option set to show warnings based on the DTE.
            // 3 DTE and/or 21 DTE

            int dte = AfxDaysBetween(current_date, expiry_date);
            dte_text = std::to_string(dte) + "d";

            dte_color = clrMagenta(state);
            if (dte >= 19 && dte <= 23 && trade->warning_21_dte) {
                dte_color = clrOrange(state);
            }
            if (dte <= 3 && trade->warning_3_dte) {
                dte_color = clrYellow(state);
            }

            // If the expiry year is greater than current year + 1 then add
            // the year to the display string. Useful for LEAP options.
            if (AfxGetYear(expiry_date) > AfxGetYear(current_date) + 1) {
                short_date.append("/");
                short_date.append(std::to_string(AfxGetYear(expiry_date)));
            }

            int col = 1;

            if (!is_history) {
                ld.SetData(col, trade, ticker_id, dot, StringAlignment::right, clrBackDarkGray(state),
                    dte_color, font9, false);
                col++;

                ld.SetData(col, trade, ticker_id, "", StringAlignment::left, clrBackDarkGray(state),
                    clrTextLightWhite(state), font9, false);  // empty column
            }
            col++;

            ld.SetData(col, trade, ticker_id, std::to_string(leg->open_quantity), StringAlignment::right,
                clrBackMediumGray(state), clrTextLightWhite(state), font9, false);  // position quantity
            col++;

            ld.SetData(col, trade, ticker_id, short_date, StringAlignment::center, 
                clrBackLightGray(state), clrTextLightWhite(state), font9, false);   // expiry date
            col++;

            ld.SetData(col, trade, ticker_id, dte_text, StringAlignment::center, clrBackMediumGray(state),
                clrTextDarkWhite(state), font9, false);   // DTE
            col++;

            ld.SetData(col, trade, ticker_id, leg->strike_price, StringAlignment::center, 
                clrBackLightGray(state), clrTextLightWhite(state), font9, false);   // strike price
            col++;

            ld.SetData(col, trade, ticker_id, state.db.PutCallToString(leg->put_call), StringAlignment::left, 
                clrBackMediumGray(state), clrTextDarkWhite(state), font9, false);   // PutCall
            col++;

            if (!is_history) {
                text = "";
                ld.SetData(COLUMN_OPTIONLEG_DELTA, trade, ticker_id, text, StringAlignment::right, clrBackDarkGray(state),
                    clrTextDarkWhite(state), font9, false);

                text = "";
                ld.SetData(COLUMN_TICKER_PORTFOLIO_1, trade, ticker_id, text, StringAlignment::right, clrBackDarkGray(state),
                    clrTextDarkWhite(state), font9, false);
                col++;

                ld.SetData(COLUMN_TICKER_PORTFOLIO_2, trade, ticker_id, text, StringAlignment::right, clrBackDarkGray(state),
                    clrTextDarkWhite(state), font9, false);   
                col++;

                ld.SetData(COLUMN_TICKER_PORTFOLIO_3, trade, ticker_id, text, StringAlignment::right, clrBackDarkGray(state),
                    clrTextDarkWhite(state), font9, false);   
                col++;

                ld.SetData(COLUMN_TICKER_PORTFOLIO_4, trade, ticker_id, text, StringAlignment::right, clrBackDarkGray(state),
                    clrTextDarkWhite(state), font9, false);
                col++;

                text = "";
                ld.SetData(COLUMN_TICKER_PORTFOLIO_5, trade, ticker_id, text, StringAlignment::right, clrBackDarkGray(state),
                    clrTextDarkWhite(state), font9, false);
            }

            vec.push_back(ld);
        }
    }

    // // *** BLANK SEPARATION LINE ***
    ListPanelData_AddBlankLine(state, vec);
}


// ========================================================================================
// Create the display data for a blank line
// ========================================================================================
void ListPanelData_AddBlankLine(AppState& state, std::vector<CListPanelData>& vec) {
    // *** BLANK SEPARATION LINE AT END OF LIST ***
    CListPanelData ld;
    ld.line_type = LineType::none;
    vec.push_back(ld);
}


// ========================================================================================
// Create the display data a History Header line.
// ========================================================================================
void ListPanelData_HistoryHeader(AppState& state, std::vector<CListPanelData>& vec, const std::shared_ptr<Trade>& trade, 
    const std::shared_ptr<Transaction>& trans, const std::shared_ptr<Transaction>& trans_orig) {
    // Display the transaction description, date, and total prior to showing the detail lines
    CListPanelData ld;
    ld.line_type = LineType::history_selectable;

    std::string text;
    int font9 = 9;

    ImU32 clr = clrTextDarkWhite(state);
        
    TickerId ticker_id = -1;

    ld.line_type = LineType::transaction_header;
    ld.trans = trans_orig;

    ld.SetData(0, trade, ticker_id, GLYPH_TREEOPEN, StringAlignment::center, clrBackDarkGray(state),
        clrTextDarkWhite(state), font9, false);

    text = trans->description;
    ld.SetData(1, trade, ticker_id, text, StringAlignment::left, clrBackDarkGray(state),
        clrOrange(state), font9, false);   // orange

    text = trans->trans_date;
    ld.SetData(2, trade, ticker_id, text, StringAlignment::left, clrBackDarkGray(state),
        clrTextLightWhite(state), font9, false);

    if (trans->underlying == Underlying::Shares ||
        trans->underlying == Underlying::Futures) {

        if (trans->legs.at(0)->action == Action::STC ||
            trans->legs.at(0)->action == Action::BTC) {

            double multiplier = 1;
            if (trans->underlying == Underlying::Futures) {
                multiplier = AfxValDouble(state.config.GetMultiplier(trade->ticker_symbol));
            }

            double quantity = trans->quantity;
            double total = trans->total + trans->fees;  
            double cost = (quantity * std::abs(trans->share_average_cost));
            double fees = trans->fees * -1;
            double diff = (total + cost);

            text = "";

            clr = (total >= 0) ? clrGreen(state) : clrRed(state);
            text = AfxMoney(total, 2, state);
            ld.SetData(4, trade, ticker_id, text, StringAlignment::center, clrBackDarkGray(state),
                clr, font9, false);

            clr = (cost >= 0) ? clrGreen(state) : clrRed(state);
            text = AfxMoney(cost, 2, state);
            ld.SetData(5, trade, ticker_id, text, StringAlignment::center, clrBackDarkGray(state),
                clr, font9, false);

            clr = (diff >= 0) ? clrGreen(state) : clrRed(state);
            text = AfxMoney(diff, 2, state);
            ld.SetData(6, trade, ticker_id, text, StringAlignment::center, clrBackDarkGray(state),
                clr, font9, false);

            clr = (total >= 0) ? clrGreen(state) : clrRed(state);
            text = AfxMoney(total, 2, state);
            ld.SetData(7, trade, ticker_id, text, StringAlignment::center, clrBackDarkGray(state),
                clr, font9, false);
        }

        if (trans->legs.at(0)->action == Action::BTO ||
            trans->legs.at(0)->action == Action::STO) {
            text = AfxMoney(trans->total, 2, state);
            clr = (trans->total >= 0) ? clrGreen(state) : clrRed(state);
            ld.SetData(7, trade, ticker_id, text, StringAlignment::center, clrBackDarkGray(state),
                clr, font9, false);
        }
    }
    else {
        text = AfxMoney(trans->total, 2, state);
        clr = (trans->total >= 0) ? clrGreen(state) : clrRed(state);
        ld.SetData(7, trade, ticker_id, text, StringAlignment::center, clrBackDarkGray(state),
                clr, font9, false);
    }

    ld.SetData(COLUMN_EDIT_ICON, trade, ticker_id, ICON_MD_EDIT , StringAlignment::center,
        clrBackDarkGray(state), clrTextDarkWhite(state), font9, false);

    vec.push_back(ld);
}


// ========================================================================================
// Create the display data for a History SHARES/FUTURES leg.
// ========================================================================================
void ListPanelData_HistorySharesLeg(AppState& state, std::vector<CListPanelData>& vec, const std::shared_ptr<Trade>& trade, 
    const std::shared_ptr<Transaction>& trans, const std::shared_ptr<Leg>& leg)
{
    CListPanelData ld;
    ld.line_type = LineType::nonselectable;

    TickerId ticker_id = -1;
    int font9 = 9;

    std::string text;

    if (trans->legs.at(0)->action == Action::STC ||
        trans->legs.at(0)->action == Action::BTC) {

        text = state.db.ActionToStringDescription(trans->share_action);
        ld.SetData(2, trade, ticker_id, text, StringAlignment::left, 
            clrBackMediumGray(state), clrTextDarkWhite(state), font9, false);

        text = AfxMoney(leg->open_quantity, 0, state);
        ld.SetData(3, trade, ticker_id, text, StringAlignment::right, 
            clrBackMediumGray(state), clrTextDarkWhite(state), font9, false);

        text = AfxMoney(trans->price, trade->ticker_decimals,state);
        ld.SetData(4, trade, ticker_id, text, StringAlignment::right,
            clrBackMediumGray(state), clrTextDarkWhite(state), font9, false);

        double multiplier = 1;
        double average_cost = trans->share_average_cost;
        if (leg->underlying == Underlying::Futures) {
            multiplier = AfxValDouble(state.config.GetMultiplier(trade->ticker_symbol));
            average_cost /= multiplier;
        }

        text = AfxMoney(average_cost, trade->ticker_decimals, state);
        ld.SetData(5, trade, ticker_id, text, StringAlignment::center, 
            clrBackMediumGray(state), clrTextDarkWhite(state), font9, false);

        double diff = 0; 
        if (trans->legs.at(0)->action == Action::STC) {
            diff = (trans->price + average_cost);
        }
        if (trans->legs.at(0)->action == Action::BTC) {
            diff = (average_cost - trans->price);
        }
        text = AfxMoney(diff, trade->ticker_decimals, state);
        ld.SetData(6, trade, ticker_id, text, StringAlignment::center, 
            clrBackMediumGray(state), clrTextDarkWhite(state), font9, false);

        double total = (abs(leg->open_quantity) * (diff * multiplier));
        text = AfxMoney(total, trade->ticker_decimals, state);
        ld.SetData(7, trade, ticker_id, text, StringAlignment::center, 
            clrBackMediumGray(state), clrTextDarkWhite(state), font9, false);
    }

    if (trans->legs.at(0)->action == Action::BTO ||
        trans->legs.at(0)->action == Action::STO) {

        text = state.db.ActionToStringDescription(trans->share_action);
        ld.SetData(2, trade, ticker_id, text, StringAlignment::left, 
            clrBackMediumGray(state), clrTextDarkWhite(state), font9, false);

        ld.SetData(3, trade, ticker_id, "", StringAlignment::right, 
            clrBackMediumGray(state), clrTextDarkWhite(state), font9, false);

        ld.SetData(4, trade, ticker_id, "", StringAlignment::right, 
            clrBackMediumGray(state), clrTextDarkWhite(state), font9, false);

        ld.SetData(5, trade, ticker_id, "", StringAlignment::right, 
            clrBackMediumGray(state), clrTextDarkWhite(state), font9, false);

        text = AfxMoney(leg->open_quantity, 0, state);
        ld.SetData(6, trade, ticker_id, text, StringAlignment::center, 
            clrBackMediumGray(state), clrTextDarkWhite(state), font9, false);

        double cost = trans->price + (trans->fees / leg->open_quantity);
        text = AfxMoney(cost, trade->ticker_decimals, state);
        ld.SetData(7, trade, ticker_id, text, StringAlignment::center, 
            clrBackMediumGray(state), clrTextDarkWhite(state), font9, false);
    }

    vec.push_back(ld);
}


// ========================================================================================
// Create the display data for a History Dividend.
// ========================================================================================
void ListPanelData_HistoryDividendLeg(AppState& state, std::vector<CListPanelData>& vec, const std::shared_ptr<Trade>& trade, 
    const std::shared_ptr<Transaction>& trans, const std::shared_ptr<Leg>& leg)
{
    CListPanelData ld;
    ld.line_type = LineType::nonselectable;

    TickerId ticker_id = -1;
    int font9 = 9;

    std::string text = "DIVIDEND";
    ld.SetData(2, trade, ticker_id, text, StringAlignment::left, 
            clrBackMediumGray(state), clrTextDarkWhite(state), font9, false);

    for (int i = 3; i < 8; i++) {
        ld.SetData(i, trade, ticker_id, "", StringAlignment::left, 
            clrBackMediumGray(state), clrTextDarkWhite(state), font9, false);
    }
    
    text = AfxMoney(trans->price, 2,state);
    ld.SetData(7, trade, ticker_id, text, StringAlignment::center, 
            clrBackMediumGray(state), clrTextDarkWhite(state), font9, false);

    vec.push_back(ld);
}


// ========================================================================================
// Create the display data for a Other Income (for a Trade).
// ========================================================================================
void ListPanelData_OtherIncomeLeg(AppState& state, std::vector<CListPanelData>& vec, const std::shared_ptr<Trade>& trade, 
    const std::shared_ptr<Transaction>& trans, const std::shared_ptr<Leg>& leg)
{
    CListPanelData ld;
    ld.line_type = LineType::nonselectable;

    TickerId ticker_id = -1;
    int font9 = 9;

    std::string text = "OTHER";
    ld.SetData(2, trade, ticker_id, text, StringAlignment::left, 
            clrBackMediumGray(state), clrTextDarkWhite(state), font9, false);

    for (int i = 3; i < 8; i++) {
        ld.SetData(i, trade, ticker_id, "", StringAlignment::left, 
            clrBackMediumGray(state), clrTextDarkWhite(state), font9, false);
    }
    
    text = AfxMoney(trans->price, 2, state);
    ld.SetData(7, trade, ticker_id, text, StringAlignment::center, 
            clrBackMediumGray(state), clrTextDarkWhite(state), font9, false);

    vec.push_back(ld);
}


// ========================================================================================
// Create the display data for a History OPTIONS leg.
// ========================================================================================
void ListPanelData_HistoryOptionsLeg(AppState& state, std::vector<CListPanelData>& vec, const std::shared_ptr<Trade>& trade, 
    const std::shared_ptr<Transaction>& trans, const std::shared_ptr<Leg>& leg)
{
    CListPanelData ld;
    ld.line_type = LineType::nonselectable;

    TickerId ticker_id = -1;
    int font9 = 9;

    std::string text = std::to_string(leg->original_quantity);
    ld.SetData(2, trade, ticker_id, text, StringAlignment::right, 
            clrBackMediumGray(state), clrTextLightWhite(state), font9, false);

    int dte = AfxDaysBetween(trans->trans_date, leg->expiry_date);
    std::string days = std::to_string(dte) + "d";
    std::string short_date = AfxShortDate(leg->expiry_date);

    // If the expiry year is greater than current year + 1 then add
    // the year to the display string. Useful for LEAP options.
    if (AfxGetYear(leg->expiry_date) > AfxGetYear(trans->trans_date) + 1) {
        short_date.append("/");
        short_date.append(std::to_string(AfxGetYear(leg->expiry_date)));
    }

    ld.SetData(3, trade, ticker_id, short_date, StringAlignment::center, 
        clrBackLightGray(state), clrTextLightWhite(state), font9, false);

    ld.SetData(4, trade, ticker_id, days, StringAlignment::center,
        clrBackMediumGray(state), clrTextDarkWhite(state), font9, false);

    ld.SetData(5, trade, ticker_id, leg->strike_price, StringAlignment::center,
        clrBackLightGray(state), clrTextLightWhite(state), font9, false);

    ld.SetData(6, trade, ticker_id, state.db.PutCallToString(leg->put_call), StringAlignment::left,
        clrBackMediumGray(state), clrTextDarkWhite(state), font9, false);

    ImU32 clr = clrRed(state);
    if (leg->action == Action::BTO || leg->action == Action::BTC) clr = clrGreen(state);

    ld.SetData(7, trade, ticker_id, state.db.ActionToStringDescription(leg->action), StringAlignment::center,
        clrBackLightGray(state), clr, font9, false);

    vec.push_back(ld);
}


// ========================================================================================
// Create the display data line for closed position yearly total.
// ========================================================================================
void ListPanelData_OutputClosedYearTotal(AppState& state, std::vector<CListPanelData>& vec, int year, double subtotal, int year_win, int year_loss) {
    CListPanelData ld;
    ld.line_type = LineType::nonselectable;
    
    TickerId ticker_id = -1;
    int font9 = 9;

    ImU32 clr = (subtotal >= 0) ? clrGreen(state) : clrRed(state);
    
    if (year == 0) year = AfxGetYear(AfxCurrentDate());

    std::string text = std::to_string(year) + " TOTAL";
    ld.SetData(3, nullptr, ticker_id, text, StringAlignment::right, clrBackDarkGray(state), clr, font9, false);

    ld.SetData(4, nullptr, ticker_id, AfxMoney(subtotal, 2, state), StringAlignment::right, 
        clrBackDarkGray(state), clr, font9, false);

    // col 5 is a "spacer" column

    clr = (year_win >= year_loss) ? clrGreen(state) : clrRed(state);
    text = std::to_string(year_win) + "W " + std::to_string(year_loss)
        + "L  " + AfxMoney((double)year_win / (MAX(year_win + year_loss,1)) * 100, 0, state) + "%";
    ld.SetData(6, nullptr, ticker_id, text, StringAlignment::left, clrBackDarkGray(state), clr, font9, false);

    vec.insert(vec.begin(), ld);
}


// ========================================================================================
// Create the display data line for total closed monthly total.
// ========================================================================================
void ListPanelData_OutputClosedMonthTotal(AppState& state, std::vector<CListPanelData>& vec, double monthly_total, int month_win, int month_loss) {
    CListPanelData ld;
    ld.line_type = LineType::nonselectable;

    TickerId ticker_id = -1;
    int font9 = 9;

    ImU32 clr = (monthly_total >= 0) ? clrGreen(state) : clrRed(state);
    
    std::string text = "THIS MONTH";
    ld.SetData(3, nullptr, ticker_id, text, StringAlignment::right, clrBackDarkGray(state), clr, font9, false);

    ld.SetData(4, nullptr, ticker_id, AfxMoney(monthly_total, 2, state), StringAlignment::right,
        clrBackDarkGray(state), clr, font9, false);

    // col 5 is a "spacer" column

    clr = (month_win >= month_loss) ? clrGreen(state) : clrRed(state);
    text = std::to_string(month_win) + "W " + std::to_string(month_loss)
        + "L  " + AfxMoney((double)month_win / (MAX(month_win + month_loss,1)) * 100, 0, state) + "%";
    ld.SetData(6, nullptr, ticker_id, text, StringAlignment::left, clrBackDarkGray(state), clr, font9, false);

    vec.insert(vec.begin(), ld);
}


// ========================================================================================
// Create the display data line for total closed weekly total.
// ========================================================================================
void ListPanelData_OutputClosedWeekTotal(AppState& state, std::vector<CListPanelData>& vec, double weekly_total, int week_win, int week_loss) {
    CListPanelData ld;
    ld.line_type = LineType::nonselectable;

    TickerId ticker_id = -1;
    int font9 = 9;

    ImU32 clr = (weekly_total >= 0) ? clrGreen(state) : clrRed(state);
    
    std::string text = "THIS WEEK";
    ld.SetData(3, nullptr, ticker_id, text, StringAlignment::right, 
        clrBackDarkGray(state), clr, font9, false);

    ld.SetData(4, nullptr, ticker_id, AfxMoney(weekly_total, 2, state), StringAlignment::right, 
        clrBackDarkGray(state), clr, font9, false);

    // col 5 is a "spacer" column

    clr = (week_win >= week_loss) ? clrGreen(state) : clrRed(state);
    text = std::to_string(week_win) + "W " + std::to_string(week_loss)
        + "L  " + AfxMoney((double)week_win / (MAX(week_win + week_loss,1)) * 100, 0, state) + "%";
    ld.SetData(6, nullptr, ticker_id, text, StringAlignment::left, clrBackDarkGray(state), clr, font9, false);

    vec.insert(vec.begin(), ld);
}


// ========================================================================================
// Create the display data line for total closed daily total.
// ========================================================================================
void ListPanelData_OutputClosedDayTotal(AppState& state, std::vector<CListPanelData>& vec, double daily_total, int day_win, int day_loss) {
    CListPanelData ld;
    ld.line_type = LineType::nonselectable;

    TickerId ticker_id = -1;
    int font9 = 9;

    ImU32 clr = (daily_total >= 0) ? clrGreen(state) : clrRed(state);
    
    std::string text = "TODAY";
    ld.SetData(3, nullptr, ticker_id, text, StringAlignment::right, 
        clrBackDarkGray(state), clr, font9, false);

    ld.SetData(4, nullptr, ticker_id, AfxMoney(daily_total, 2, state), StringAlignment::right, 
        clrBackDarkGray(state), clr, font9, false);

    // col 5 is a "spacer" column

    clr = (day_win >= day_loss) ? clrGreen(state) : clrRed(state);
    text = std::to_string(day_win) + "W " + std::to_string(day_loss)
        + "L  " + AfxMoney((double)day_win / (MAX(day_win + day_loss,1)) * 100, 0, state) + "%";
    ld.SetData(6, nullptr, ticker_id, text, StringAlignment::left,
        clrBackDarkGray(state), clr, font9, false);

    vec.insert(vec.begin(), ld);

    // *** BLANK SEPARATION LINE AFTER THE DAY TOTAL ***
    CListPanelData ld_blank;
    ld_blank.line_type = LineType::none;
    vec.insert(vec.begin()+1, ld_blank);
}


// ========================================================================================
// Create the display data line for a closed position month subtotal.
// ========================================================================================
void ListPanelData_OutputClosedMonthSubtotal(
    AppState& state, std::vector<CListPanelData>& vec, const std::string& closed_date, double subtotal, int month_win, int month_loss)
{
    CListPanelData ld;
    ld.line_type = LineType::nonselectable;

    TickerId ticker_id = -1;
    int font9 = 9;

    ImU32 clr = (subtotal >= 0) ? clrGreen(state) : clrRed(state);
    
    std::string text = AfxUpper(AfxGetLongMonthName(closed_date)) + " " + std::to_string(AfxGetYear(closed_date));
    ld.SetData(3, nullptr, ticker_id, text, StringAlignment::right, 
        clrBackDarkGray(state), clr, font9, false);

    ld.SetData(4, nullptr, ticker_id, AfxMoney(subtotal, 2, state), StringAlignment::right, 
        clrBackDarkGray(state), clr, font9, false);

    // col 5 is a "spacer" column

    clr = (month_win >= month_loss) ? clrGreen(state) : clrRed(state);
    text = std::to_string(month_win) + "W " + std::to_string(month_loss)
        + "L  " + AfxMoney((double)month_win / (MAX(month_win + month_loss,1)) * 100, 0, state) + "%";
    ld.SetData(6, nullptr, ticker_id, text, StringAlignment::left, 
        clrBackDarkGray(state), clr, font9, false);

    vec.push_back(ld);
    ListPanelData_AddBlankLine(state, vec);
}


// ========================================================================================
// Create the display data line for a closed position.
// ========================================================================================
void ListPanelData_OutputClosedPosition(AppState& state, std::vector<CListPanelData>& vec, const std::shared_ptr<Trade>& trade, 
    const std::string& closed_date, const std::string& ticker_symbol, const std::string& description, double closed_amount) {

    CListPanelData ld;
    ld.line_type = LineType::ticker_line;

    TickerId ticker_id = -1;
    int font9 = 9;

    ld.SetData(0, trade, ticker_id, "", StringAlignment::left, 
        clrBackDarkGray(state), clrOrange(state), font9, false);

    ld.SetData(1, trade, ticker_id, closed_date, StringAlignment::left, 
        clrBackDarkGray(state), clrOrange(state), font9, false);

    ld.SetData(2, trade, ticker_id, ticker_symbol, StringAlignment::left, 
        clrBackDarkGray(state), clrTextLightWhite(state), font9, false);

    ld.SetData(3, trade, ticker_id, description, StringAlignment::left, 
        clrBackDarkGray(state), clrTextLightWhite(state), font9, false);

    ImU32 clr = (closed_amount >= 0) ? clrGreen(state) : clrRed(state);
    ld.SetData(4, trade, ticker_id, AfxMoney(closed_amount, 2, state), StringAlignment::right, 
        clrBackDarkGray(state), clr, font9, false);

    // col 5 is a "spacer" column

    ld.SetData(6, trade, ticker_id, state.config.GetCategoryDescription(trade->category), StringAlignment::left,
        clrBackDarkGray(state), clrTextDarkWhite(state), font9, false);

    vec.push_back(ld);
}


// ========================================================================================
// Create the display data line for Transaction running total.
// ========================================================================================
void ListPanelData_OutputTransactionRunningTotal(AppState& state, std::vector<CListPanelData>& vec, 
    double running_gross_total, double running_fees_total, double running_net_total)
{
    CListPanelData ld;
    ld.line_type = LineType::nonselectable;

    TickerId ticker_id = -1;
    int font9 = 9;

    ImU32 clr = (running_net_total >= 0) ? clrGreen(state) : clrRed(state);
    if (running_fees_total >= 0) running_fees_total = running_fees_total * -1;

    std::string text = "TOTAL";
    ld.SetData(3, nullptr, ticker_id, text, StringAlignment::right,
        clrBackDarkGray(state), clr, font9, false);

    ld.SetData(5, nullptr, ticker_id, AfxMoney(running_gross_total, 2, state), StringAlignment::right, 
        clrBackDarkGray(state), clr, font9, false);

    ld.SetData(6, nullptr, ticker_id, AfxMoney(running_fees_total, 2, state), StringAlignment::right, 
        clrBackDarkGray(state), clr, font9, false);

    ld.SetData(7, nullptr, ticker_id, AfxMoney(running_net_total, 2, state), StringAlignment::right, 
        clrBackDarkGray(state), clr, font9, false);

    vec.push_back(ld);

    // *** BLANK SEPARATION LINE AFTER THE TOTAL ***
    ListPanelData_AddBlankLine(state, vec);
}


// ========================================================================================
// Create the display data line for a Transaction.
// ========================================================================================
void ListPanelData_OutputTransaction(
    AppState& state, std::vector<CListPanelData>& vec, const std::shared_ptr<Trade>& trade, const std::shared_ptr<Transaction>& trans)
{
    CListPanelData ld;
    ld.line_type = LineType::ticker_line;

    TickerId ticker_id = -1;
    int font9 = 9;

    ld.trans = trans;

    ld.SetData(0, trade, ticker_id, "", StringAlignment::left, 
        clrBackDarkGray(state), clrOrange(state), font9, false);

    ld.SetData(1, trade, ticker_id, trans->trans_date, StringAlignment::left, 
        clrBackDarkGray(state), clrTextLightWhite(state), font9, false);

    ld.SetData(2, trade, ticker_id, trade->ticker_symbol, StringAlignment::left,
        clrBackDarkGray(state), clrOrange(state), font9, false);

    ld.SetData(3, trade, ticker_id, trans->description, StringAlignment::left, 
        clrBackDarkGray(state), clrTextLightWhite(state), font9, false);

    ld.SetData(4, trade, ticker_id, AfxMoney(trans->quantity, 2, state), StringAlignment::right,
        clrBackDarkGray(state), clrTextLightWhite(state), font9, false);

    ld.SetData(5, trade, ticker_id, AfxMoney(trans->price, state.config.GetTickerDecimals(trade->ticker_symbol), state), 
        StringAlignment::right, clrBackDarkGray(state), clrTextLightWhite(state), font9, false);

    ld.SetData(6, trade, ticker_id, AfxMoney(trans->fees, 2, state), StringAlignment::right, 
        clrBackDarkGray(state), clrTextLightWhite(state), font9, false);

    ImU32 clr = (trans->total >= 0) ? clrGreen(state) : clrRed(state);
    ld.SetData(7, trade, ticker_id, AfxMoney(trans->total, 2, state), StringAlignment::right, 
        clrBackDarkGray(state), clr, font9, false);

    // col 8 is a "spacer" column

    ld.SetData(9, trade, ticker_id, state.config.GetCategoryDescription(trade->category), StringAlignment::left, 
        clrBackDarkGray(state), clrTextDarkWhite(state), font9, false);

    vec.push_back(ld);
}


