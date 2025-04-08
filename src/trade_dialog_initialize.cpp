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

#include "appstate.h"
#include "utilities.h"

#include "trade_dialog.h"
#include <string>

// #include <iostream>


void CreateTradeActionOptionType(AppState& state, TradeDialogData& tdd) {
    
    const bool is_short = (tdd.short_long == "SHORT") ? true : false;
    const bool is_long  = (tdd.short_long == "LONG")  ? true : false;
    const bool is_put   = (tdd.put_call   == "PUT")   ? true : false;
    const bool is_call  = (tdd.put_call   == "CALL")  ? true : false;

    if (state.strategy_current_item == (int)Strategy::Vertical && is_short && is_put) {
        state.trade_action_option_type = TradeActionOptionType::short_put_vertical;
        return;
    }

    if (state.strategy_current_item == (int)Strategy::Vertical && is_long && is_put) {
        state.trade_action_option_type = TradeActionOptionType::long_put_vertical;
        return;
    }

    if (state.strategy_current_item == (int)Strategy::Vertical && is_short && is_call) {
        state.trade_action_option_type = TradeActionOptionType::short_call_vertical;
        return;
    }

    if (state.strategy_current_item == (int)Strategy::Vertical && is_long && is_call) {
        state.trade_action_option_type = TradeActionOptionType::long_call_vertical;
        return;
    }

    if (state.strategy_current_item == (int)Strategy::Strangle && is_short) {
        state.trade_action_option_type = TradeActionOptionType::short_strangle;
        return;
    }

    if (state.strategy_current_item == (int)Strategy::Strangle && is_long) {
        state.trade_action_option_type = TradeActionOptionType::long_strangle;
        return;
    }

    if (state.strategy_current_item == (int)Strategy::Straddle && is_short) {
        state.trade_action_option_type = TradeActionOptionType::short_straddle;
        return;
    }

    if (state.strategy_current_item == (int)Strategy::Straddle && is_long) {
        state.trade_action_option_type = TradeActionOptionType::long_straddle;
        return;
    }

    if (state.strategy_current_item == (int)Strategy::Option && is_short && is_put) {
        state.trade_action_option_type = TradeActionOptionType::short_put;
        return;
    }

    if (state.strategy_current_item == (int)Strategy::Option && is_long && is_put) {
        state.trade_action_option_type = TradeActionOptionType::long_put;
        return;
    }

    if (state.strategy_current_item == (int)Strategy::Option && is_short && is_call) {
        state.trade_action_option_type = TradeActionOptionType::short_call;
        return;
    }

    if (state.strategy_current_item == (int)Strategy::Option && is_long && is_call) {
        state.trade_action_option_type = TradeActionOptionType::long_call;
        return;
    }

    if (state.strategy_current_item == (int)Strategy::IronCondor && is_short) {
        state.trade_action_option_type = TradeActionOptionType::short_iron_condor;
        return;
    }

    if (state.strategy_current_item == (int)Strategy::IronCondor && is_long) {
        state.trade_action_option_type = TradeActionOptionType::long_iron_condor;
        return;
    }

    if (state.strategy_current_item == (int)Strategy::Butterfly && is_short && is_put) {
        state.trade_action_option_type = TradeActionOptionType::short_put_butterfly;
        return;
    }

    if (state.strategy_current_item == (int)Strategy::Butterfly && is_long && is_put) {
        state.trade_action_option_type = TradeActionOptionType::long_put_butterfly;
        return;
    }

    if (state.strategy_current_item == (int)Strategy::Butterfly && is_short && is_call) {
        state.trade_action_option_type = TradeActionOptionType::short_call_butterfly;
        return;
    }

    if (state.strategy_current_item == (int)Strategy::Butterfly && is_long && is_call) {
        state.trade_action_option_type = TradeActionOptionType::long_call_butterfly;
        return;
    }

    if (state.strategy_current_item == (int)Strategy::RatioSpread && is_short && is_put) {
        state.trade_action_option_type = TradeActionOptionType::short_put_ratio;
        return;
    }

    if (state.strategy_current_item == (int)Strategy::RatioSpread && is_long && is_put) {
        state.trade_action_option_type = TradeActionOptionType::long_put_ratio;
        return;
    }

    if (state.strategy_current_item == (int)Strategy::RatioSpread && is_short && is_call) {
        state.trade_action_option_type = TradeActionOptionType::short_call_ratio;
        return;
    }

    if (state.strategy_current_item == (int)Strategy::RatioSpread && is_long && is_call) {
        state.trade_action_option_type = TradeActionOptionType::long_call_ratio;
        return;
    }

    if (state.strategy_current_item == (int)Strategy::LT112 && is_short && is_put) {
        state.trade_action_option_type = TradeActionOptionType::short_put_LT112;
        return;
    }

    if (state.strategy_current_item == (int)Strategy::LT112 && is_long && is_put) {
        state.trade_action_option_type = TradeActionOptionType::long_put_LT112;
        return;
    }

    if (state.strategy_current_item == (int)Strategy::LT112 && is_short && is_call) {
        state.trade_action_option_type = TradeActionOptionType::short_call_LT112;
        return;
    }

    if (state.strategy_current_item == (int)Strategy::LT112 && is_long && is_call) {
        state.trade_action_option_type = TradeActionOptionType::long_call_LT112;
        return;
    }
 
}


void InitializeTradeGridDefaults(AppState& state, TradeDialogData& tdd, std::vector<TradeGridData>& tgd) {
    std::string current_date = AfxCurrentDate();

    if (state.trade_action_option_type == TradeActionOptionType::none) {    // Custom Options Trade
        tdd.short_long = ""; tdd.put_call = ""; tdd.description = "";
        tgd.at(0).quantity = ""; tgd.at(0).expiry_date = current_date; tgd.at(0).put_call = "P"; tgd.at(0).action = "STO";
        return;
    }

    if (state.trade_action_option_type == TradeActionOptionType::short_put_vertical) {
        state.strategy_current_item = (int)Strategy::Vertical;
        tdd.short_long = "SHORT"; tdd.put_call = "PUT"; tdd.description = "Vertical";
        tgd.at(0).quantity = "-1"; tgd.at(0).expiry_date = current_date; tgd.at(0).put_call = "P"; tgd.at(0).action = "STO";
        tgd.at(1).quantity =  "1"; tgd.at(1).expiry_date = current_date; tgd.at(1).put_call = "P"; tgd.at(1).action = "BTO";
        return;
    }

    if (state.trade_action_option_type == TradeActionOptionType::long_put_vertical) {
        state.strategy_current_item = (int)Strategy::Vertical;
        tdd.short_long = "LONG"; tdd.put_call = "PUT"; tdd.description = "Vertical";
        tgd.at(0).quantity =  "1"; tgd.at(0).expiry_date = current_date; tgd.at(0).put_call = "P"; tgd.at(0).action = "BTO";
        tgd.at(1).quantity = "-1"; tgd.at(1).expiry_date = current_date; tgd.at(1).put_call = "P"; tgd.at(1).action = "STO";
        return;
    }

    if (state.trade_action_option_type == TradeActionOptionType::short_call_vertical) {
        state.strategy_current_item = (int)Strategy::Vertical;
        tdd.short_long = "SHORT"; tdd.put_call = "CALL"; tdd.description = "Vertical";
        tgd.at(0).quantity = "-1"; tgd.at(0).expiry_date = current_date; tgd.at(0).put_call = "C"; tgd.at(0).action = "STO";
        tgd.at(1).quantity =  "1"; tgd.at(1).expiry_date = current_date; tgd.at(1).put_call = "C"; tgd.at(1).action = "BTO";
        return;
    }

    if (state.trade_action_option_type == TradeActionOptionType::long_call_vertical) {
        state.strategy_current_item = (int)Strategy::Vertical;
        tdd.short_long = "LONG"; tdd.put_call = "CALL"; tdd.description = "Vertical";
        tgd.at(0).quantity =  "1"; tgd.at(0).expiry_date = current_date; tgd.at(0).put_call = "C"; tgd.at(0).action = "BTO";
        tgd.at(1).quantity = "-1"; tgd.at(1).expiry_date = current_date; tgd.at(1).put_call = "C"; tgd.at(1).action = "STO";
        return;
    }

    if (state.trade_action_option_type == TradeActionOptionType::short_strangle) {
        state.strategy_current_item = (int)Strategy::Strangle;
        tdd.short_long = "SHORT"; tdd.put_call = ""; tdd.description = "Strangle";
        tgd.at(0).quantity = "-1"; tgd.at(0).expiry_date = current_date; tgd.at(0).put_call = "P"; tgd.at(0).action = "STO";
        tgd.at(1).quantity = "-1"; tgd.at(1).expiry_date = current_date; tgd.at(1).put_call = "C"; tgd.at(1).action = "STO";
        return;
    }

    if (state.trade_action_option_type == TradeActionOptionType::long_strangle) {
        state.strategy_current_item = (int)Strategy::Strangle;
        tdd.short_long = "LONG"; tdd.put_call = ""; tdd.description = "Strangle";
        tgd.at(0).quantity = "1"; tgd.at(0).expiry_date = current_date; tgd.at(0).put_call = "P"; tgd.at(0).action = "BTO";
        tgd.at(1).quantity = "1"; tgd.at(1).expiry_date = current_date; tgd.at(1).put_call = "C"; tgd.at(1).action = "BTO";
        return;
    }

    if (state.trade_action_option_type == TradeActionOptionType::short_straddle) {
        state.strategy_current_item = (int)Strategy::Straddle;
        tdd.short_long = "SHORT"; tdd.put_call = ""; tdd.description = "Straddle";
        tgd.at(0).quantity = "-1"; tgd.at(0).expiry_date = current_date; tgd.at(0).put_call = "P"; tgd.at(0).action = "STO";
        tgd.at(1).quantity = "-1"; tgd.at(1).expiry_date = current_date; tgd.at(1).put_call = "C"; tgd.at(1).action = "STO";
        return;
    }

    if (state.trade_action_option_type == TradeActionOptionType::long_straddle) {
        state.strategy_current_item = (int)Strategy::Straddle;
        tdd.short_long = "LONG"; tdd.put_call = ""; tdd.description = "Straddle";
        tgd.at(0).quantity = "1"; tgd.at(0).expiry_date = current_date; tgd.at(0).put_call = "P"; tgd.at(0).action = "BTO";
        tgd.at(1).quantity = "1"; tgd.at(1).expiry_date = current_date; tgd.at(1).put_call = "C"; tgd.at(1).action = "BTO";
        return;
    }

    if (state.trade_action_option_type == TradeActionOptionType::short_put) {
        state.strategy_current_item = (int)Strategy::Option;
        tdd.short_long = "SHORT"; tdd.put_call = "PUT"; tdd.description = "Put Option";
        tgd.at(0).quantity = "-1"; tgd.at(0).expiry_date = current_date; tgd.at(0).put_call = "P"; tgd.at(0).action = "STO";
        return;
    }

    if (state.trade_action_option_type == TradeActionOptionType::long_put) {
        state.strategy_current_item = (int)Strategy::Option;
        tdd.short_long = "LONG"; tdd.put_call = "PUT"; tdd.description = "Put Option";
        tgd.at(0).quantity = "1"; tgd.at(0).expiry_date = current_date; tgd.at(0).put_call = "P"; tgd.at(0).action = "BTO";
        return;
    }

    if (state.trade_action_option_type == TradeActionOptionType::short_call) {
        state.strategy_current_item = (int)Strategy::Option;
        tdd.short_long = "SHORT"; tdd.put_call = "CALL"; tdd.description = "Call Option";
        tgd.at(0).quantity = "-1"; tgd.at(0).expiry_date = current_date; tgd.at(0).put_call = "C"; tgd.at(0).action = "STO";
        return;
    }

    if (state.trade_action_option_type == TradeActionOptionType::long_call) {
        state.strategy_current_item = (int)Strategy::Option;
        tdd.short_long = "LONG"; tdd.put_call = "CALL"; tdd.description = "Call Option";
        tgd.at(0).quantity = "1"; tgd.at(0).expiry_date = current_date; tgd.at(0).put_call = "C"; tgd.at(0).action = "BTO";
        return;
    }

    if (state.trade_action_option_type == TradeActionOptionType::short_iron_condor) {
        state.strategy_current_item = (int)Strategy::IronCondor;
        tdd.short_long = "SHORT"; tdd.put_call = ""; tdd.description = "Iron Condor";
        tgd.at(0).quantity =  "1"; tgd.at(0).expiry_date = current_date; tgd.at(0).put_call = "P"; tgd.at(0).action = "BTO";
        tgd.at(1).quantity = "-1"; tgd.at(1).expiry_date = current_date; tgd.at(1).put_call = "P"; tgd.at(1).action = "STO";
        tgd.at(2).quantity = "-1"; tgd.at(2).expiry_date = current_date; tgd.at(2).put_call = "C"; tgd.at(2).action = "STO";
        tgd.at(3).quantity =  "1"; tgd.at(3).expiry_date = current_date; tgd.at(3).put_call = "C"; tgd.at(3).action = "BTO";
        return;
    }

    if (state.trade_action_option_type == TradeActionOptionType::long_iron_condor) {
        state.strategy_current_item = (int)Strategy::IronCondor;
        tdd.short_long = "LONG"; tdd.put_call = ""; tdd.description = "Iron Condor";
        tgd.at(0).quantity = "-1"; tgd.at(0).expiry_date = current_date; tgd.at(0).put_call = "P"; tgd.at(0).action = "STO";
        tgd.at(1).quantity =  "1"; tgd.at(1).expiry_date = current_date; tgd.at(1).put_call = "P"; tgd.at(1).action = "BTO";
        tgd.at(2).quantity =  "1"; tgd.at(2).expiry_date = current_date; tgd.at(2).put_call = "C"; tgd.at(2).action = "BTO";
        tgd.at(3).quantity = "-1"; tgd.at(3).expiry_date = current_date; tgd.at(3).put_call = "C"; tgd.at(3).action = "STO";
        return;
    }

    if (state.trade_action_option_type == TradeActionOptionType::short_put_butterfly) {
        state.strategy_current_item = (int)Strategy::Butterfly;
        tdd.short_long = "SHORT"; tdd.put_call = "PUT"; tdd.description = "Butterfly";
        tgd.at(0).quantity = "-1"; tgd.at(0).expiry_date = current_date; tgd.at(0).put_call = "P"; tgd.at(0).action = "STO";
        tgd.at(1).quantity =  "2"; tgd.at(1).expiry_date = current_date; tgd.at(1).put_call = "P"; tgd.at(1).action = "BTO";
        tgd.at(2).quantity = "-1"; tgd.at(2).expiry_date = current_date; tgd.at(2).put_call = "P"; tgd.at(2).action = "STO";
        return;
    }

    if (state.trade_action_option_type == TradeActionOptionType::long_put_butterfly) {
        state.strategy_current_item = (int)Strategy::Butterfly;
        tdd.short_long = "LONG"; tdd.put_call = "PUT"; tdd.description = "Butterfly";
        tgd.at(0).quantity =  "1"; tgd.at(0).expiry_date = current_date; tgd.at(0).put_call = "P"; tgd.at(0).action = "BTO";
        tgd.at(1).quantity = "-2"; tgd.at(1).expiry_date = current_date; tgd.at(1).put_call = "P"; tgd.at(1).action = "STO";
        tgd.at(2).quantity =  "1"; tgd.at(2).expiry_date = current_date; tgd.at(2).put_call = "P"; tgd.at(2).action = "BTO";
        return;
    }

    if (state.trade_action_option_type == TradeActionOptionType::short_call_butterfly) {
        state.strategy_current_item = (int)Strategy::Butterfly;
        tdd.short_long = "SHORT"; tdd.put_call = "CALL"; tdd.description = "Butterfly";
        tgd.at(0).quantity = "-1"; tgd.at(0).expiry_date = current_date; tgd.at(0).put_call = "C"; tgd.at(0).action = "STO";
        tgd.at(1).quantity =  "2"; tgd.at(1).expiry_date = current_date; tgd.at(1).put_call = "C"; tgd.at(1).action = "BTO";
        tgd.at(2).quantity = "-1"; tgd.at(2).expiry_date = current_date; tgd.at(2).put_call = "C"; tgd.at(2).action = "STO";
        return;
    }

    if (state.trade_action_option_type == TradeActionOptionType::long_call_butterfly) {
        state.strategy_current_item = (int)Strategy::Butterfly;
        tdd.short_long = "LONG"; tdd.put_call = "CALL"; tdd.description = "Butterfly";
        tgd.at(0).quantity =  "1"; tgd.at(0).expiry_date = current_date; tgd.at(0).put_call = "C"; tgd.at(0).action = "BTO";
        tgd.at(1).quantity = "-2"; tgd.at(1).expiry_date = current_date; tgd.at(1).put_call = "C"; tgd.at(1).action = "STO";
        tgd.at(2).quantity =  "1"; tgd.at(2).expiry_date = current_date; tgd.at(2).put_call = "C"; tgd.at(2).action = "BTO";
        return;
    }

    if (state.trade_action_option_type == TradeActionOptionType::short_put_ratio) {
        state.strategy_current_item = (int)Strategy::RatioSpread;
        tdd.short_long = "SHORT"; tdd.put_call = "PUT"; tdd.description = "Ratio";
        tgd.at(0).quantity = "-1"; tgd.at(0).expiry_date = current_date; tgd.at(0).put_call = "P"; tgd.at(0).action = "STO";
        tgd.at(1).quantity =  "2"; tgd.at(1).expiry_date = current_date; tgd.at(1).put_call = "P"; tgd.at(1).action = "BTO";
        return;
    }

    if (state.trade_action_option_type == TradeActionOptionType::long_put_ratio) {
        state.strategy_current_item = (int)Strategy::RatioSpread;
        tdd.short_long = "LONG"; tdd.put_call = "PUT"; tdd.description = "Ratio";
        tgd.at(0).quantity =  "1"; tgd.at(0).expiry_date = current_date; tgd.at(0).put_call = "P"; tgd.at(0).action = "BTO";
        tgd.at(1).quantity = "-2"; tgd.at(1).expiry_date = current_date; tgd.at(1).put_call = "P"; tgd.at(1).action = "STO";
        return;
    }

    if (state.trade_action_option_type == TradeActionOptionType::short_call_ratio) {
        state.strategy_current_item = (int)Strategy::RatioSpread;
        tdd.short_long = "SHORT"; tdd.put_call = "CALL"; tdd.description = "Ratio";
        tgd.at(0).quantity = "-1"; tgd.at(0).expiry_date = current_date; tgd.at(0).put_call = "C"; tgd.at(0).action = "STO";
        tgd.at(1).quantity =  "2"; tgd.at(1).expiry_date = current_date; tgd.at(1).put_call = "C"; tgd.at(1).action = "BTO";
        return;
    }

    if (state.trade_action_option_type == TradeActionOptionType::long_call_ratio) {
        state.strategy_current_item = (int)Strategy::RatioSpread;
        tdd.short_long = "LONG"; tdd.put_call = "CALL"; tdd.description = "Ratio";
        tgd.at(0).quantity =  "1"; tgd.at(0).expiry_date = current_date; tgd.at(0).put_call = "C"; tgd.at(0).action = "BTO";
        tgd.at(1).quantity = "-2"; tgd.at(1).expiry_date = current_date; tgd.at(1).put_call = "C"; tgd.at(1).action = "STO";
        return;
    }

    if (state.trade_action_option_type == TradeActionOptionType::short_put_LT112) {
        state.strategy_current_item = (int)Strategy::LT112;
        tdd.short_long = "SHORT"; tdd.put_call = "PUT"; tdd.description = "LT112";
        tgd.at(0).quantity =  "1"; tgd.at(0).expiry_date = current_date; tgd.at(0).put_call = "P"; tgd.at(0).action = "BTO";
        tgd.at(1).quantity = "-1"; tgd.at(1).expiry_date = current_date; tgd.at(1).put_call = "P"; tgd.at(1).action = "STO";
        tgd.at(2).quantity = "-2"; tgd.at(2).expiry_date = current_date; tgd.at(2).put_call = "P"; tgd.at(2).action = "STO";
        return;
    }

    if (state.trade_action_option_type == TradeActionOptionType::long_put_LT112) {
        state.strategy_current_item = (int)Strategy::LT112;
        tdd.short_long = "LONG"; tdd.put_call = "PUT"; tdd.description = "LT112";
        tgd.at(0).quantity = "-1"; tgd.at(0).expiry_date = current_date; tgd.at(0).put_call = "P"; tgd.at(0).action = "STO";
        tgd.at(1).quantity =  "1"; tgd.at(1).expiry_date = current_date; tgd.at(1).put_call = "P"; tgd.at(1).action = "BTO";
        tgd.at(2).quantity =  "2"; tgd.at(2).expiry_date = current_date; tgd.at(2).put_call = "P"; tgd.at(2).action = "BTO";
        return;
    }

    if (state.trade_action_option_type == TradeActionOptionType::short_call_LT112) {
        state.strategy_current_item = (int)Strategy::LT112;
        tdd.short_long = "SHORT"; tdd.put_call = "CALL"; tdd.description = "LT112";
        tgd.at(0).quantity =  "1"; tgd.at(0).expiry_date = current_date; tgd.at(0).put_call = "C"; tgd.at(0).action = "BTO";
        tgd.at(1).quantity = "-1"; tgd.at(1).expiry_date = current_date; tgd.at(1).put_call = "C"; tgd.at(1).action = "STO";
        tgd.at(2).quantity = "-2"; tgd.at(2).expiry_date = current_date; tgd.at(2).put_call = "C"; tgd.at(2).action = "STO";
        return;
    }

    if (state.trade_action_option_type == TradeActionOptionType::long_call_LT112) {
        state.strategy_current_item = (int)Strategy::LT112;
        tdd.short_long = "LONG"; tdd.put_call = "CALL"; tdd.description = "LT112";
        tgd.at(0).quantity = "-1"; tgd.at(0).expiry_date = current_date; tgd.at(0).put_call = "C"; tgd.at(0).action = "STO";
        tgd.at(1).quantity =  "1"; tgd.at(1).expiry_date = current_date; tgd.at(1).put_call = "C"; tgd.at(1).action = "BTO";
        tgd.at(2).quantity =  "2"; tgd.at(2).expiry_date = current_date; tgd.at(2).put_call = "C"; tgd.at(2).action = "BTO";
        return;
    }
}


void InitializeCloseLegsDefaults(AppState& state, TradeDialogData& tdd, std::vector<TradeGridData>& tgd) {
    std::shared_ptr<Trade> trade = state.trade_dialog_trade;

    // If this is close_trade / close_all_legs then we need to populate the selected legs vector
    // because this would not have been done during the right click menu selection.
    if (IsCloseTrade(state.trade_action)) {
        for (auto& leg : trade->open_legs) {
            state.activetrades_selected_legs.push_back(leg);
        }
    }

    // Selected legs would have been loaded to state.activetrades_selected_legs
    // Filter the vector so that only OPTIONS legs remain.
    std::erase_if(state.activetrades_selected_legs, 
        [](std::shared_ptr<Leg> leg) { 
            return leg->underlying != Underlying::Options; 
        });


    tdd.ticker_symbol       = trade->ticker_symbol;
    tdd.ticker_name         = trade->ticker_name;
    tdd.futures_expiry_date = trade->future_expiry;
    tdd.transaction_date    = AfxCurrentDate();
    tdd.description         = "Close";
    tdd.is_futures_ticker   = state.config.IsFuturesTicker(tdd.ticker_symbol);
    tdd.category            = trade->category;
    state.category_current_item = trade->category;
    tdd.quantity            = 1;
    tdd.multiplier          = LookupTickerMultiplier(state, tdd.ticker_symbol);
    tdd.dr_cr               = "DR";
    tdd.buying_power        = std::to_string((int)trade->trade_bp);
    tdd.notes               = trade->notes;
    tdd.warning_3_dte       = trade->warning_3_dte;
    tdd.warning_21_dte      = trade->warning_21_dte;
    tdd.grid_count          = 1;

    // Display the legs being closed and set each to the needed inverse action.
    int i = 0;
    for (const auto& leg : state.activetrades_selected_legs) {
        TradeGridData legdata{};

        std::string action;
        if (leg->action == Action::STO) action = "BTC";
        if (leg->action == Action::BTO) action = "STC";

        legdata.original_quantity = std::to_string(leg->original_quantity);
        legdata.quantity          = std::to_string(leg->open_quantity * -1);
        legdata.expiry_date       = leg->expiry_date;
        legdata.strike_price      = leg->strike_price;
        legdata.put_call          = state.db.PutCallToString(leg->put_call);
        legdata.action            = action;

        // tgd vector is already initialized with 8 rows so we just need to
        // insert them at the correct location rather than push_back append.
        tgd.at(i) = legdata;
        i++;
    }
}


void InitializeRollLegsDefaults(AppState& state, TradeDialogData& tdd, std::vector<TradeGridData>& tgd) {
    // Selected legs would have been loaded to state.activetrades_selected_legs
    // Filter the vector so that only OPTIONS legs remain.
    std::erase_if(state.activetrades_selected_legs, 
        [](std::shared_ptr<Leg> leg) { 
            return leg->underlying != Underlying::Options; 
        });

    std::shared_ptr<Trade> trade = state.trade_dialog_trade;

    tdd.ticker_symbol       = trade->ticker_symbol;
    tdd.ticker_name         = trade->ticker_name;
    tdd.futures_expiry_date = trade->future_expiry;
    tdd.transaction_date    = AfxCurrentDate();
    tdd.description         = "Roll";
    tdd.is_futures_ticker   = state.config.IsFuturesTicker(tdd.ticker_symbol);
    tdd.category            = trade->category;
    state.category_current_item = trade->category;
    tdd.quantity            = 1;
    tdd.multiplier          = LookupTickerMultiplier(state, tdd.ticker_symbol);
    tdd.dr_cr               = "CR";
    tdd.buying_power        = std::to_string((int)trade->trade_bp);
    tdd.notes               = trade->notes;
    tdd.warning_3_dte       = trade->warning_3_dte;
    tdd.warning_21_dte      = trade->warning_21_dte;
    tdd.grid_count          = 2;

    // Display the legs being closed and set each to the needed inverse action.
    int i = 0;   // we will only roll up to 4 legs
    for (const auto& leg : state.activetrades_selected_legs) {
        TradeGridData legdata{};

        std::string action;
        if (leg->action == Action::STO) action = "BTC";
        if (leg->action == Action::BTO) action = "STC";

        legdata.original_quantity = std::to_string(leg->original_quantity);
        legdata.quantity          = std::to_string(leg->open_quantity * -1);
        legdata.expiry_date       = leg->expiry_date;
        legdata.strike_price      = leg->strike_price;
        legdata.put_call          = state.db.PutCallToString(leg->put_call);
        legdata.action            = action;

        // tgd vector is already initialized with 8 rows so we just need to
        // insert them at the correct location rather than push_back append.
        tgd.at(i) = legdata;

        // Add some default information for the new Roll transaction
        legdata.quantity      = std::to_string(leg->open_quantity);
        legdata.expiry_date   = AfxDateAddDays(legdata.expiry_date, 7);
        if (action == "BTC") legdata.action = "STO";
        if (action == "STC") legdata.action = "BTO";
        tgd.at(i + 4) = legdata;

        i++;
    }
}


void InitializeEditTradeDefaults(AppState& state, TradeDialogData& tdd, std::vector<TradeGridData>& tgd) {
    if (!state.trans_edit_trade) return;
    if (!state.trans_edit_transaction) return;

    std::shared_ptr<Trade> trade = state.trans_edit_trade;
    std::shared_ptr<Transaction> trans = state.trans_edit_transaction;

    tdd.ticker_symbol       = trade->ticker_symbol;
    tdd.ticker_name         = trade->ticker_name;
    tdd.futures_expiry_date = trade->future_expiry;
    tdd.transaction_date    = trans->trans_date;
    tdd.description         = trans->description;
    tdd.short_long          = "";   // strategy. Not used in Edit.
    tdd.put_call            = "";   // strategy. Not used in Edit.
    tdd.is_futures_ticker   = state.config.IsFuturesTicker(tdd.ticker_symbol);
    tdd.category            = trade->category;
    state.category_current_item = trade->category;
    tdd.quantity            = trans->quantity;
    tdd.price               = trans->price;
    tdd.multiplier          = trans->multiplier;
    tdd.fees                = trans->fees;
    tdd.total               = trans->total;
    tdd.dr_cr               = (trans->total < 0) ? "DR" : "CR";
    tdd.buying_power        = std::to_string((int)trade->trade_bp);
    tdd.notes               = trade->notes;
    tdd.warning_3_dte       = trade->warning_3_dte;
    tdd.warning_21_dte      = trade->warning_21_dte;
    tdd.grid_count          = 1;

    int i = 0;
    for (const auto& leg : trans->legs) {
        TradeGridData legdata{};

        legdata.original_quantity = std::to_string(leg->original_quantity / MAX(trans->quantity,1));
        legdata.quantity          = std::to_string(leg->open_quantity / MAX(trans->quantity,1));
        legdata.expiry_date       = leg->expiry_date;
        legdata.strike_price      = leg->strike_price;
        legdata.put_call          = state.db.PutCallToString(leg->put_call);
        legdata.action            = state.db.ActionToStringDescription(leg->action);

        // tgd vector is already initialized with 8 rows so we just need to
        // insert them at the correct location rather than push_back append.
        tgd.at(i) = legdata;
        i++;
    }
}


void InitializeManageSharesDefaults(AppState& state, TradeDialogData& tdd, std::vector<TradeGridData>& tgd) {
    // Handles New and Manage existing positions
    tdd.transaction_date    = AfxCurrentDate();
    tdd.description         = "Shares";
    tdd.multiplier          = 1;
    tdd.grid_count          = 0;
    tdd.is_futures_ticker   = false;
    tdd.futures_expiry_date = "";
    tdd.trade_grid1_label = "Shares Action";
    tdd.trade_grid2_label = "";

    if (IsNewSharesTradeAction(state.trade_action)) {
        tdd.quantity          = 1;
        tdd.category          = 1;
        state.category_current_item = tdd.category;
        tdd.buying_power      = "";
        tdd.notes             = "";
        tdd.dr_cr             = "DR";
        tdd.share_action      = Action::BTO;
    }
    if (IsManageSharesTradeAction(state.trade_action) ||
        IsAddSharesToTradeAction(state.trade_action)) {
        std::shared_ptr<Trade> trade = state.trade_dialog_trade;
        tdd.ticker_symbol     = trade->ticker_symbol;
        tdd.ticker_name       = trade->ticker_name;
        tdd.quantity          = (double)(state.activetrades_rightclickmenu_shares_count);
        tdd.category          = trade->category;
        state.category_current_item = trade->category;
        tdd.buying_power      = std::to_string((int)trade->trade_bp);
        tdd.notes             = trade->notes;
        tdd.dr_cr           = "CR";
        tdd.share_action    = Action::STC;
        if (IsAddSharesToTradeAction(state.trade_action)) {
            tdd.dr_cr       = "DR";
            tdd.share_action = Action::BTO;
        }
    }
}


void InitializeManageFuturesDefaults(AppState& state, TradeDialogData& tdd, std::vector<TradeGridData>& tgd) {
    // Handles New and Manage existing positions
    tdd.transaction_date    = AfxCurrentDate();
    tdd.description         = "Futures";
    tdd.multiplier          = LookupTickerMultiplier(state, tdd.ticker_symbol);
    tdd.dr_cr               = "DR";
    tdd.grid_count          = 0;
    tdd.is_futures_ticker   = true;
    tdd.trade_grid1_label = "Futures Action";
    tdd.trade_grid2_label = "";

    if (IsNewFuturesTradeAction(state.trade_action)) {
        tdd.quantity        = 1;
        tdd.category        = 1;
        state.category_current_item = tdd.category;
        tdd.buying_power    = "";
        tdd.notes           = "";
        tdd.dr_cr           = "DR";
        tdd.share_action    = Action::BTO;
    }
    if (IsManageFuturesTradeAction(state.trade_action) ||
        IsAddFuturesToTradeAction(state.trade_action)) {
        std::shared_ptr<Trade> trade = state.trade_dialog_trade;
        tdd.ticker_symbol   = trade->ticker_symbol;
        tdd.ticker_name     = trade->ticker_name;
        tdd.category        = trade->category;
        tdd.quantity        = (double)(state.activetrades_rightclickmenu_futures_count);
        state.category_current_item = trade->category;
        tdd.futures_expiry_date = trade->future_expiry;
        tdd.buying_power    = std::to_string((int)trade->trade_bp);
        tdd.notes           = trade->notes;
        tdd.dr_cr           = "CR";
        tdd.share_action    = Action::STC;
        if (IsAddFuturesToTradeAction(state.trade_action)) {
            tdd.dr_cr       = "DR";
            tdd.share_action = Action::BTO;
        }
    }
}


void InitializeOtherIncomeExpense(AppState& state, TradeDialogData& tdd, std::vector<TradeGridData>& tgd) {
    tdd.transaction_date    = AfxCurrentDate();
    tdd.description         = "";
    tdd.multiplier          = 1;
    tdd.quantity            = 1;
    tdd.dr_cr               = "CR";
    tdd.grid_count          = 0;
    tdd.is_futures_ticker   = false;
    tdd.trade_grid1_label   = "";
    tdd.trade_grid2_label   = "";

    if (IsOtherIncomeExpense(state.trade_action)) {
        tdd.category        = CATEGORY_OTHER;
        state.category_current_item = tdd.category;
        tdd.buying_power    = "";
        tdd.notes           = "";
    }
    if (IsAddOtherIncomeExpenseToTrade(state.trade_action)) {
        std::shared_ptr<Trade> trade = state.trade_dialog_trade;
        tdd.ticker_symbol   = trade->ticker_symbol;
        tdd.ticker_name     = trade->ticker_name;
        tdd.category        = trade->category;
        state.category_current_item = trade->category;
        tdd.notes           = trade->notes;
    }
}


void InitializeAddDividendToTrade(AppState& state, TradeDialogData& tdd, std::vector<TradeGridData>& tgd) {
    tdd.transaction_date    = AfxCurrentDate();
    tdd.description         = "Dividend";
    tdd.multiplier          = 1;
    tdd.quantity            = 1;
    tdd.dr_cr               = "CR";
    tdd.grid_count          = 0;
    tdd.is_futures_ticker   = false;
    tdd.trade_grid1_label   = "";
    tdd.trade_grid2_label   = "";

    std::shared_ptr<Trade> trade = state.trade_dialog_trade;
    tdd.ticker_symbol   = trade->ticker_symbol;
    tdd.ticker_name     = trade->ticker_name;
    tdd.category        = trade->category;
    state.category_current_item = trade->category;
    tdd.notes           = trade->notes;
}


void InitializeTradeDialogDefaults(AppState& state, TradeDialogData& tdd, std::vector<TradeGridData>& tgd) {
    // Reset the tgd vector
    tgd.clear();
    std::vector<TradeGridData>td_blank(8);
    tgd.swap(td_blank);

    // Reset the data variable
    TradeDialogData data_blank{};
    tdd = data_blank;

    // Default that the TradeDialog is the ActiveTraces current selected trade. This
    // is modified to the Edit trade pointer if this is an edit transaction.
    state.trade_dialog_trade = state.activetrades_selected_trade;

    std::string current_date = AfxCurrentDate();
    tdd.transaction_date = current_date;


    if (IsNewOptionsTradeAction(state.trade_action)) {
        tdd.trade_dialog_banner = "OPTIONS";
        tdd.trade_grid1_label = "New Transaction";
        tdd.trade_grid2_label = "";
        tdd.futures_expiry_date = tdd.transaction_date;
        InitializeTradeGridDefaults(state, tdd, tgd);
        return;
    }

    if (IsAddOptionsToTradeAction(state.trade_action)) {
        tdd.trade_dialog_banner = "ADD OPTIONS";
        tdd.trade_grid1_label = "New Transaction";
        tdd.trade_grid2_label = "";
        tdd.ticker_symbol = state.trade_dialog_trade->ticker_symbol;
        tdd.ticker_name = state.trade_dialog_trade->ticker_name;
        tdd.category = state.trade_dialog_trade->category;
        tdd.futures_expiry_date = state.trade_dialog_trade->future_expiry;
        state.category_current_item = state.trade_dialog_trade->category;
        tdd.multiplier = LookupTickerMultiplier(state, tdd.ticker_symbol);
        tdd.buying_power = std::to_string(state.trade_dialog_trade->trade_bp);
        tdd.notes = state.trade_dialog_trade->notes;
        InitializeTradeGridDefaults(state, tdd, tgd);
        return;
    }

    if (IsCloseTrade(state.trade_action)) {
        state.trade_dialog_trade = state.activetrades_selected_trade;
        if (state.activetrades_rightclickmenu_options_exist) {
            state.trade_action = TradeAction::close_all_legs;
            InitializeCloseLegsDefaults(state, tdd, tgd);
            tdd.trade_dialog_banner = "CLOSE OPTIONS";
            tdd.trade_grid1_label = "Close Transaction";
        } else if (state.activetrades_rightclickmenu_shares_exist) {
            state.trade_action = TradeAction::manage_shares;
            InitializeManageSharesDefaults(state, tdd, tgd);
            tdd.trade_dialog_banner = "CLOSE SHARES";
        } else if (state.activetrades_rightclickmenu_futures_exist) {
            state.trade_action = TradeAction::manage_futures;
            InitializeManageFuturesDefaults(state, tdd, tgd);
            tdd.trade_dialog_banner = "CLOSE FUTURES";
        }
        return;
    }

    if (IsCloseLegTradeAction(state.trade_action)) {
        state.trade_dialog_trade = state.activetrades_selected_trade;
        InitializeCloseLegsDefaults(state, tdd, tgd);
        tdd.trade_dialog_banner = "CLOSE";
        tdd.trade_grid1_label = "Close Transaction";
        return;
    }

    if (IsRollLegsTradeAction(state.trade_action)) {
        state.trade_dialog_trade = state.activetrades_selected_trade;
        InitializeRollLegsDefaults(state, tdd, tgd);
        tdd.trade_dialog_banner = "ROLL OPTIONS";
        tdd.trade_grid1_label = "Close Transaction";
        tdd.trade_grid2_label = "Roll Transaction";
        return;
    }

    if (IsEditTradeAction(state.trade_action)) {
        // Also make sure to set the dialog's trade pointer to the same as the edit 
        // trade pointer because it is referenced when the data is saved.
        state.trade_dialog_trade = state.trans_edit_trade;
        InitializeEditTradeDefaults(state, tdd, tgd);
        tdd.trade_dialog_banner = "EDIT";
        tdd.trade_grid1_label = "Transaction Data";
        return;
    }

    if (IsNewSharesTradeAction(state.trade_action) ||
        IsManageSharesTradeAction(state.trade_action)) {
        state.trade_dialog_trade = state.activetrades_selected_trade;
        InitializeManageSharesDefaults(state, tdd, tgd);
        tdd.trade_dialog_banner = "SHARES";
        tdd.trade_grid1_label = "New Transaction";
        return;
    }

    if (IsAddSharesToTradeAction(state.trade_action)) {
        state.trade_dialog_trade = state.activetrades_selected_trade;
        InitializeManageSharesDefaults(state, tdd, tgd);
        tdd.trade_dialog_banner = "ADD SHARES";
        tdd.trade_grid1_label = "New Transaction";
        return;
    }

    if (IsNewFuturesTradeAction(state.trade_action) ||
        IsManageFuturesTradeAction(state.trade_action)) {
        state.trade_dialog_trade = state.activetrades_selected_trade;
        InitializeManageFuturesDefaults(state, tdd, tgd);
        tdd.trade_dialog_banner = "FUTURES";
        tdd.trade_grid1_label = "New Transaction";
        return;
    }

    if (IsAddFuturesToTradeAction(state.trade_action)) {
        state.trade_dialog_trade = state.activetrades_selected_trade;
        InitializeManageFuturesDefaults(state, tdd, tgd);
        tdd.trade_dialog_banner = "ADD FUTURES";
        tdd.trade_grid1_label = "New Transaction";
        return;
    }

    if (IsOtherIncomeExpense(state.trade_action) ||
        IsAddOtherIncomeExpenseToTrade(state.trade_action)) {
        state.trade_dialog_trade = state.activetrades_selected_trade;
        InitializeOtherIncomeExpense(state, tdd, tgd);
        tdd.trade_dialog_banner = "OTHER INCOME/EXPENSE";
        return;
    }

    if (IsAddDividendToTrade(state.trade_action)) {
        state.trade_dialog_trade = state.activetrades_selected_trade;
        InitializeAddDividendToTrade(state, tdd, tgd);
        tdd.trade_dialog_banner = "ADD DIVIDEND";
        return;
    }
}

