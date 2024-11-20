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

#ifndef TRADEDIALOG_H
#define TRADEDIALOG_H

#include "appstate.h"


struct TradeDialogData {
    std::string trade_dialog_banner;   // OPTIONS, EDIT, etc
    std::string trade_grid1_label;
    std::string trade_grid2_label;
    std::string ticker_symbol;
    std::string ticker_name;
    std::string futures_expiry_date;
    std::string transaction_date;
    std::string description;
    std::string short_long = "SHORT";
    std::string put_call = "PUT";
    bool is_futures_ticker = false;
    int category = 0;
    double quantity = 0.0f;
    double price = 0.0f;
    double multiplier = 0.0f;
    double fees = 0.0f;
    double total = 0.0f;
    std::string dr_cr = "CR";
    std::string buying_power;
    std::string notes;
    bool warning_3_dte = true;
    bool warning_21_dte = true;
    int grid_count = 1;
    Action share_action = Action::Nothing;
};

enum class Strategy {
    Vertical = 0,
    Strangle,
    Straddle,
    Option,
    IronCondor,
    Butterfly,
    RatioSpread,
    LT112
};

struct TradeGridData {
    std::string original_quantity;
    std::string quantity;
    std::string expiry_date;
    std::string dte;
    std::string strike_price;
    std::string put_call;
    std::string action;
};

double LookupTickerMultiplier(AppState& state, std::string& ticker_symbol);
Underlying SetTradeUnderlying(AppState& state);
bool IsNewTradeAction(TradeAction action);
bool IsRollLegsTradeAction(TradeAction action);
bool IsEditTradeAction(TradeAction action);
bool IsNewOptionsTradeAction(TradeAction action);
bool IsNewSharesTradeAction(TradeAction action);
bool IsNewFuturesTradeAction(TradeAction action);
bool IsNewSharesFuturesTradeAction(TradeAction action);
bool IsAddSharesToTradeAction(TradeAction action);
bool IsAddFuturesToTradeAction(TradeAction action);
bool IsAddSharesFuturesToTradeAction(TradeAction action);
bool IsAddOptionsToTradeAction(TradeAction action);
bool IsAddToTradeAction(TradeAction action);
bool IsManageSharesTradeAction(TradeAction action);
bool IsManageFuturesTradeAction(TradeAction action);
bool IsManageSharesFuturesTradeAction(TradeAction action);
bool IsOtherIncomeExpense(TradeAction action);
bool IsAddOtherIncomeExpenseToTrade(TradeAction action);
bool IsAddDividendToTrade(TradeAction action);
bool IsCloseAllSharesFutures(TradeAction action);
bool IsCloseTrade(TradeAction action);
bool IsCloseLegTradeAction(TradeAction action);
void ShowTradeDialogPopup(AppState& state);

bool CalendarComboBox(AppState& state, std::string id_combo, 
            std::string& iso_date, std::string& display_date, const ImGuiComboFlags flags, const float x_position, const float width);
void CategoriesComboBox(AppState& state, std::string id_combo, const float x_position, const float width);

#endif  // TRADEDIALOG_H 
