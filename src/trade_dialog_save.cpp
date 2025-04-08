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
#include "messagebox.h"
#include "active_trades_actions.h"
#include "trade_dialog.h"

extern bool is_connection_ready_for_data;



// ========================================================================================
// Remove pipe character from the string and return new copy. 
// ========================================================================================
std::string RemovePipeChar(const std::string& text) {
    std::string temp = text;
    temp.erase(remove(temp.begin(), temp.end(), '|'), temp.end());
    return temp;
}


Underlying SetTradeUnderlying(AppState& state) {
    if (state.trade_action == TradeAction::new_shares_trade)      return Underlying::Shares;
    if (state.trade_action == TradeAction::new_futures_trade)     return Underlying::Futures;
    if (state.trade_action == TradeAction::manage_shares)         return Underlying::Shares;
    if (state.trade_action == TradeAction::manage_futures)        return Underlying::Futures;
    if (state.trade_action == TradeAction::close_all_shares)      return Underlying::Shares;
    if (state.trade_action == TradeAction::close_all_futures)     return Underlying::Futures;
    if (state.trade_action == TradeAction::add_shares_to_trade)   return Underlying::Shares;
    if (state.trade_action == TradeAction::add_futures_to_trade)  return Underlying::Futures;
    if (state.trade_action == TradeAction::other_income_expense)  return Underlying::Other;
    if (state.trade_action == TradeAction::add_income_expense_to_trade) return Underlying::Other;
    if (state.trade_action == TradeAction::add_dividend_to_trade) return Underlying::Dividend;
    if (state.trade_dialog_transaction) return state.trade_dialog_transaction->underlying;
    return Underlying::Options;
}


// ========================================================================================
// Perform error checks on the Shares/Futures trade data prior to allowing the save 
// to the database.
// ========================================================================================
bool ValidateSharesFuturesTradeData(AppState& state, TradeDialogData& tdd, std::vector<TradeGridData>& tgd) {
    // Do an error check to ensure that the data about to be saved does not contain any missing data, etc.
    std::string error_message;
    std::string text;

    if (state.trade_action == TradeAction::manage_futures ||
        state.trade_action == TradeAction::add_futures_to_trade) {
        if (tdd.ticker_symbol.substr(0, 1) != "/") error_message += "- Invalid Futures Ticker Symbol.\n";
    }

    if (tdd.ticker_symbol.length() == 0) error_message += "- Missing Ticker Symbol.\n";
    if (tdd.ticker_name.length() == 0) error_message += "- Missing Company or Futures Name.\n";
    if (tdd.description.length() == 0) error_message += "- Missing Description.\n";
    if (tdd.quantity == 0) error_message += "- Missing Quantity.\n";

    if (error_message.length()) {
        CustomMessageBox(state, "Warning", error_message);
        return false;
    }

    return true;   // data is good, allow the save to continue
}


// ========================================================================================
// Create the Shares/Futures trade transaction data and save it to the database
// ========================================================================================
void CreateSharesFuturesTradeData(AppState& state, TradeDialogData& tdd, std::vector<TradeGridData>& tgd) {
    // PROCEED TO SAVE THE TRADE DATA
    std::shared_ptr<Trade> trade;

    if (IsNewSharesFuturesTradeAction(state.trade_action)) {
        trade = std::make_shared<Trade>();
        state.db.trades.push_back(trade);
    }
    else {
        trade = state.trade_dialog_trade;
    }

    trade->ticker_symbol = tdd.ticker_symbol;
    trade->ticker_name   = tdd.ticker_name;
    trade->future_expiry = tdd.futures_expiry_date;
    trade->category      = tdd.category;
    trade->trade_bp      = AfxValDouble(tdd.buying_power);
    trade->notes         = tdd.notes;

    std::shared_ptr<Transaction> trans = std::make_shared<Transaction>();
    trans->trans_date    = tdd.transaction_date;
    trans->description   = tdd.description;
    trans->underlying    = SetTradeUnderlying(state);
    trans->quantity      = (int)tdd.quantity; 
    trans->price         = tdd.price;
    trans->multiplier    = tdd.multiplier;
    trans->fees          = tdd.fees;
    trans->total         = tdd.total;
    trans->share_action  = tdd.share_action;
    if (tdd.dr_cr == "DR") { trans->total = trans->total * -1; }

    trade->transactions.push_back(trans);

    std::shared_ptr<Leg> leg = std::make_shared<Leg>();
    leg->underlying = trans->underlying;

    if (IsNewSharesFuturesTradeAction(state.trade_action) ||
        IsManageSharesFuturesTradeAction(state.trade_action) ||
        IsAddSharesFuturesToTradeAction(state.trade_action)) {
        if (trans->share_action == Action::BTO ||
            trans->share_action == Action::BTC) {
            leg->original_quantity = trans->quantity;
            leg->open_quantity = trans->quantity;
            leg->strike_price = std::to_string(trans->price);
            leg->action = trans->share_action;
        }
        if (trans->share_action == Action::STO ||
            trans->share_action == Action::STC) {
            leg->original_quantity = trans->quantity * -1;
            leg->open_quantity = trans->quantity * -1;
            leg->strike_price = std::to_string(trans->price);
            leg->action = trans->share_action;
        }
    }
    
    trans->legs.push_back(leg);
}



// ========================================================================================
// Perform error checks on the Other Income/Expense data prior to allowing the save 
// to the database.
// ========================================================================================
bool ValidateOtherIncomeExpenseData(AppState& state, TradeDialogData& tdd, std::vector<TradeGridData>& tgd) {
    // Do an error check to ensure that the data about to be saved does not contain
    // any missing data, etc.
    std::string error_message;
    std::string text;

    if (tdd.description.length() == 0) error_message += "- Missing Description.\n";
    if (tdd.quantity == 0) error_message += "- Missing Quantity.\n";
    if (tdd.price == 0) error_message += "- Amount can not be Zero.\n";

    if (error_message.length()) {
        CustomMessageBox(state, "Warning", error_message);
        return false;
    }

    return true;   // data is good, allow the save to continue
}



// ========================================================================================
// Create the Other Income/Expense transaction data and save it to the database
// ========================================================================================
void CreateOtherIncomeExpenseData(AppState& state, TradeDialogData& tdd, std::vector<TradeGridData>& tgd) {
    // PROCEED TO SAVE THE TRADE DATA
    std::shared_ptr<Trade> trade;

    if (IsOtherIncomeExpense(state.trade_action)) {
        trade = std::make_shared<Trade>();
        state.db.trades.push_back(trade);
        trade->ticker_symbol = "OTHER";
        trade->ticker_name = "Other Income/Expense";
        trade->is_open = false;
    }
    else {
        trade = state.trade_dialog_trade;
    }

    trade->category = CATEGORY_OTHER;
    trade->future_expiry = tdd.futures_expiry_date;
    trade->trade_bp = AfxValDouble(tdd.buying_power);
    trade->notes = tdd.notes;

    std::shared_ptr<Transaction> trans = std::make_shared<Transaction>();
    trans->trans_date = tdd.transaction_date;
    trans->description = tdd.description;
    trans->underlying = SetTradeUnderlying(state);
    trans->quantity = (int)tdd.quantity;
    trans->price = tdd.price;
    trans->multiplier = tdd.multiplier;
    trans->fees = tdd.fees;
    trans->total = tdd.total;
    if (tdd.dr_cr == "DR") { trans->total = trans->total * -1; }
    trade->transactions.push_back(trans);

    std::shared_ptr<Leg> leg = std::make_shared<Leg>();
    leg->underlying = trans->underlying;

    leg->original_quantity = trans->quantity;
    leg->open_quantity = 0;
    leg->strike_price = std::to_string(trans->total);
    leg->action = Action::BTO;

    trans->legs.push_back(leg);
}


// ========================================================================================
// Perform error checks on the Dividend trade data prior to allowing the save 
// to the database.
// ========================================================================================
bool ValidateDividendTradeData(AppState& state, TradeDialogData& tdd, std::vector<TradeGridData>& tgd) {
    // Do an error check to ensure that the data about to be saved does not contain
    // any missing data, etc.
    std::string error_message;
    std::string text;

    if (tdd.ticker_symbol.length() == 0) error_message += "- Missing Ticker Symbol.\n";
    if (tdd.ticker_name.length() == 0) error_message += "- Missing Company Name.\n";
    if (tdd.description.length() == 0) error_message += "- Missing Description.\n";
    if (tdd.quantity == 0) error_message += "- Missing Quantity.\n";
    if (tdd.price == 0) error_message += "- Dividend Amount is Zero.\n";

    if (error_message.length()) {
        CustomMessageBox(state, "Warning", error_message);
        return false;
    }

    return true;   // data is good, allow the save to continue
}


// ========================================================================================
// Create the Dividend trade transaction data and save it to the database
// ========================================================================================
void CreateDividendTradeData(AppState& state, TradeDialogData& tdd, std::vector<TradeGridData>& tgd) {
    // PROCEED TO SAVE THE TRADE DATA

    std::shared_ptr<Trade> trade = state.trade_dialog_trade;

    trade->ticker_symbol = tdd.ticker_symbol;
    trade->ticker_name = tdd.ticker_name;
    trade->future_expiry = tdd.futures_expiry_date;
    trade->category = tdd.category;
    trade->trade_bp = AfxValDouble(tdd.buying_power);

    std::shared_ptr<Transaction> trans = std::make_shared<Transaction>();
    trans->trans_date = tdd.transaction_date;
    trans->description = tdd.description;
    trans->underlying = SetTradeUnderlying(state);
    trans->quantity = (int)tdd.quantity;
    trans->price = tdd.price;
    trans->multiplier = tdd.multiplier;
    trans->fees = tdd.fees;
    trans->total = tdd.total;
    if (tdd.dr_cr == "DR") { trans->total = trans->total * -1; }
    trade->transactions.push_back(trans);

    std::shared_ptr<Leg> leg = std::make_shared<Leg>();
    leg->underlying = trans->underlying;

    leg->original_quantity = trans->quantity;
    leg->open_quantity = 0;
    leg->strike_price = std::to_string(trans->total);
    leg->action = Action::BTO;

    trans->legs.push_back(leg);
}


// ========================================================================================
// Perform error checks on the Options trade data prior to allowing the save to the database.
// ========================================================================================
bool ValidateOptionsTradeData(AppState& state, TradeDialogData& tdd, std::vector<TradeGridData>& tgd) {
    // Do an error check to ensure that the data about to be saved does not contain
    // any missing data, etc.
    std::string error_message;
    std::string text;

    if (tdd.ticker_name.length() == 0) error_message += "- Missing Ticker Symbol.\n";
    if (tdd.ticker_symbol.length() == 0) error_message += "- Missing Company or Futures Name.\n";
    if (tdd.description.length() == 0) error_message += "- Missing Description.\n";
    if (tdd.quantity == 0) error_message += "- Missing Quantity.\n";

    // In adition to validating each leg, we count the number of non blank legs. If all legs
    // are blank then there is nothing to save to add that to the error message.
    int num_blank_legs = 0;

    for (int row = 0; row < 4; ++row) {
        if (IsNewTradeAction(state.trade_action) || IsAddToTradeAction(state.trade_action)) {
            tgd.at(row).original_quantity = tgd.at(row).quantity;
        }

        // All strings must be zero length in order to skip it from being included in the transaction. 
        if (tgd.at(row).quantity.length() == 0 && 
            tgd.at(row).expiry_date.length() == 0 && 
            tgd.at(row).strike_price.length() == 0 && 
            tgd.at(row).put_call.length() == 0 && 
            tgd.at(row).action.length() == 0) {
            num_blank_legs++;
            continue;
        }

        // If any of the strings are zero length at this point then the row has incomplete data.
        bool incomplete = false;

        if (tgd.at(row).quantity.length() == 0) incomplete = true;
        if (tgd.at(row).expiry_date.length() == 0) incomplete = true;
        if (tgd.at(row).strike_price.length() == 0) incomplete = true;
        if (tgd.at(row).put_call.length() == 0) incomplete = true;
        if (tgd.at(row).action.length() == 0) incomplete = true;

        if (incomplete == true) {
            error_message += "- Leg #" + std::to_string(row + 1) + " has incomplete or missing data.\n";
        }
    }

    if (num_blank_legs == 4) {
        error_message += "- No Legs exist to be saved.\n";
    }

    if (state.trade_action == TradeAction::roll_leg) {
        int num_blank_legs = 0;
        for (int row = 4; row < 8; ++row) {
            if (tgd.at(row).quantity.length() == 0 && 
                tgd.at(row).expiry_date.length() == 0 && 
                tgd.at(row).strike_price.length() == 0 && 
                tgd.at(row).put_call.length() == 0 && 
                tgd.at(row).action.length() == 0) {
                num_blank_legs++;
                continue;
            }

            // If any of the strings are zero length at this point then the row has incomplete data.
            bool incomplete = false;

            if (tgd.at(row).quantity.length() == 0) incomplete = true;
            if (tgd.at(row).expiry_date.length() == 0) incomplete = true;
            if (tgd.at(row).strike_price.length() == 0) incomplete = true;
            if (tgd.at(row).put_call.length() == 0) incomplete = true;
            if (tgd.at(row).action.length() == 0) incomplete = true;

            if (incomplete == true) {
                error_message += "- Leg #" + std::to_string(row + 1) + " has incomplete or missing data.\n";
            }
        }
        if (num_blank_legs == 4) {
            error_message += "- No Roll Legs exist to be saved.\n";
        }
    }

    if (error_message.length()) {
        CustomMessageBox(state, "Warning", error_message);
        return false;
    }

    return true;   // data is good, allow the save to continue
}


// ========================================================================================
// Create the Options trade transaction data and save it to the database
// ========================================================================================
void CreateOptionsTradeData(AppState& state, TradeDialogData& tdd, std::vector<TradeGridData>& tgd) {
    // PROCEED TO SAVE THE TRADE DATA
    std::shared_ptr<Trade> trade;

    if (IsNewOptionsTradeAction(state.trade_action)) {
        trade = std::make_shared<Trade>();
        state.db.trades.push_back(trade);
    }
    else {
        trade = state.trade_dialog_trade;
    }

    trade->ticker_symbol = RemovePipeChar(tdd.ticker_symbol);
    trade->ticker_name   = RemovePipeChar(tdd.ticker_name);
    trade->future_expiry = tdd.futures_expiry_date;
    trade->category      = tdd.category;
    trade->trade_bp      = AfxValDouble(tdd.buying_power);
    trade->notes         = tdd.notes;
    trade->warning_3_dte = tdd.warning_3_dte;
    trade->warning_21_dte = tdd.warning_21_dte;

    std::shared_ptr<Transaction> trans = std::make_shared<Transaction>();
    trans->trans_date    = tdd.transaction_date;
    trans->description   = RemovePipeChar(tdd.description);
    trans->underlying    = SetTradeUnderlying(state);
    
    trans->quantity      = (int)tdd.quantity;
    trans->price         = tdd.price;
    trans->multiplier    = tdd.multiplier;
    trans->fees          = tdd.fees;
    trans->total         = tdd.total;
    if (tdd.dr_cr == "DR") { trans->total = trans->total * -1; }

    trade->transactions.push_back(trans);

    // Add the new transaction legs
    for (int row = 0; row < 4; ++row) {
        int intQuantity = AfxValInteger(tgd.at(row).original_quantity) * trans->quantity;
        if (intQuantity == 0) continue;
        
        std::shared_ptr<Leg> leg = std::make_shared<Leg>();

        trade->nextleg_id += 1;
        leg->leg_id       = trade->nextleg_id;
        leg->underlying   = trans->underlying;
        leg->expiry_date  = tgd.at(row).expiry_date;
        leg->strike_price = tgd.at(row).strike_price;
        leg->put_call     = state.db.StringToPutCall(tgd.at(row).put_call);
        leg->action       = state.db.StringDescriptionToAction(tgd.at(row).action);
        leg->trans        = trans;

        std::string expiry_date = AfxRemoveDateHyphens(leg->expiry_date);
        if (AfxValDouble(expiry_date) > AfxValDouble(trade->bp_end_date)) trade->bp_end_date = expiry_date;

        switch (state.trade_action) {
        case TradeAction::new_options_trade:
        case TradeAction::add_options_to_trade:
            leg->original_quantity = intQuantity;
            leg->open_quantity = intQuantity;
            break;

        case TradeAction::close_leg:
        case TradeAction::close_all_legs:
        case TradeAction::roll_leg:
            leg->original_quantity = AfxValInteger(tgd.at(row).quantity);
            leg->open_quantity = 0;
            // Update the original transaction being Closed quantities
            if (!state.activetrades_selected_legs.empty()) {
                state.activetrades_selected_legs.at(row)->open_quantity += AfxValInteger(tgd.at(row).quantity);
                leg->leg_back_pointer_id = state.activetrades_selected_legs.at(row)->leg_id;
            }
            break;
        case TradeAction::new_shares_trade:
        case TradeAction::new_futures_trade:
        case TradeAction::manage_shares:
        case TradeAction::manage_futures:
        case TradeAction::expire_leg:
        case TradeAction::close_all_shares:
        case TradeAction::close_all_futures:
        case TradeAction::assignment:
        case TradeAction::add_shares_to_trade:
        case TradeAction::add_dividend_to_trade:
        case TradeAction::add_income_expense_to_trade:
        case TradeAction::add_futures_to_trade:
        case TradeAction::other_income_expense:
        case TradeAction::edit_transaction:
        case TradeAction::no_action:
          break;
        }

        trans->legs.push_back(leg);

    }

    // Add legs for rolled portion of the transaction
    if (IsRollLegsTradeAction(state.trade_action)) {
        for (int row = 4; row < 8; ++row) {
            int intQuantity = AfxValInteger(tgd.at(row).quantity);
            if (intQuantity == 0) continue;

            std::shared_ptr<Leg> leg = std::make_shared<Leg>();

            trade->nextleg_id += 1;
            leg->leg_id       = trade->nextleg_id;
            leg->underlying   = Underlying::Options;
            leg->expiry_date  = tgd.at(row).expiry_date;
            leg->strike_price = tgd.at(row).strike_price;
            leg->put_call     = state.db.StringToPutCall(tgd.at(row).put_call);
            leg->action       = state.db.StringDescriptionToAction(tgd.at(row).action);
            leg->trans        = trans;

            std::string expiry_date = AfxRemoveDateHyphens(leg->expiry_date);
            if (AfxValDouble(expiry_date) > AfxValDouble(trade->bp_end_date)) trade->bp_end_date = expiry_date;

            leg->original_quantity = intQuantity;
            leg->open_quantity = intQuantity;

            trans->legs.push_back(leg);
        }

    }

    state.trade_dialog_trade = trade;
}


// ========================================================================================
// Perform error checks on the EDIT Options trade data prior to allowing the save to the database.
// ========================================================================================
bool ValidateEditTradeData(AppState& state, TradeDialogData& tdd, std::vector<TradeGridData>& tgd) {
    // Do an error check to ensure that the data about to be saved does not contain
    // any missing data, etc.
    std::string error_message;
    std::string text;

    // In adition to validating each leg, we count the number of non blank legs. If all legs
    // are blank then there is nothing to save to add that to the error message.
    int num_blank_legs = 0;

    if (tdd.description.length() == 0) error_message += "- Missing Description.\n";
    if (tdd.quantity == 0) error_message += "- Missing Quantity.\n";

    for (int row = 0; row < 4; ++row) {

        // All strings must be zero length in order to skip it from being included in the transaction. 
        if (tgd.at(row).original_quantity.length() == 0 && 
            tgd.at(row).expiry_date.length() == 0 && 
            tgd.at(row).strike_price.length() == 0 && 
            tgd.at(row).put_call.length() == 0 && 
            tgd.at(row).action.length() == 0) {
            num_blank_legs++;
            continue;
        }

        // If any of the strings are zero length at this point then the row has incomplete data.
        bool incomplete = false;

        if (tgd.at(row).original_quantity.length() == 0) incomplete = true;
        if (tgd.at(row).expiry_date.length() == 0) incomplete = true;
        if (tgd.at(row).strike_price.length() == 0) incomplete = true;
        if (tgd.at(row).put_call.length() == 0) incomplete = true;
        if (tgd.at(row).action.length() == 0) incomplete = true;

        if (incomplete == true) {
            error_message += "- Leg #" + std::to_string(row + 1) + " has incomplete or missing data.\n";
        }
    }

    if (num_blank_legs == 4) {
        error_message += "- No Legs exist to be saved.\n";
    }

    return true;   // data is good, allow the save to continue
}


// ========================================================================================
// Create the EDIT Options trade transaction data and save it to the database
// ========================================================================================
void CreateEditTradeData(AppState& state, TradeDialogData& tdd, std::vector<TradeGridData>& tgd) {
    // PROCEED TO SAVE THE TRADE DATA

    std::shared_ptr<Trade> trade = state.trans_edit_trade;
    std::shared_ptr<Transaction> trans = state.trans_edit_transaction;

    trade->future_expiry  = tdd.futures_expiry_date;
    trade->category       = tdd.category;
    trade->notes          = tdd.notes;
    trade->warning_3_dte  = tdd.warning_3_dte;
    trade->warning_21_dte = tdd.warning_21_dte;
    
    trans->trans_date  = tdd.transaction_date;
    trans->description = tdd.description;
    trans->quantity    = (int)tdd.quantity;
    trans->price       = tdd.price;
    trans->multiplier  = tdd.multiplier;
    trans->fees        = tdd.fees;
    trade->trade_bp    = AfxValDouble(tdd.buying_power);
    trans->total       = tdd.total;
    if (tdd.dr_cr == "DR") { trans->total = trans->total * -1; }
    trans->share_action = tdd.share_action;

    if (trans->underlying == Underlying::Shares ||
        trans->underlying == Underlying::Futures) {

        if (trans->share_action == Action::BTO ||
            trans->share_action == Action::BTC) {
            trans->legs.at(0)->original_quantity = trans->quantity;
            trans->legs.at(0)->open_quantity = trans->quantity;
            trans->legs.at(0)->strike_price = std::to_string(tdd.price);
            trans->legs.at(0)->action = tdd.share_action;
        }
        if (trans->share_action == Action::STO ||
            trans->share_action == Action::STC) {
            trans->legs.at(0)->original_quantity = trans->quantity * -1;
            trans->legs.at(0)->open_quantity = trans->quantity * -1;
            trans->legs.at(0)->strike_price = std::to_string(tdd.price);
            trans->legs.at(0)->action = tdd.share_action;
        }
    }
    else {
        std::vector<int> legsToDelete;

        // Cycle through both grids and add changes to the legs
        for (int row = 0; row < 8; ++row) {
            std::string leg_original_quantity = tgd.at(row).original_quantity;
            std::string leg_open_quantity     = tgd.at(row).quantity;
            std::string leg_expiry            = tgd.at(row).expiry_date;
            std::string leg_strike            = tgd.at(row).strike_price;
            PutCall leg_PutCall               = state.db.StringToPutCall(tgd.at(row).put_call);
            Action  leg_action                = state.db.StringDescriptionToAction(tgd.at(row).action);
            
            // Nothing new or changed to add? Just iterate to next line.
            if (leg_original_quantity.length() == 0) {
                // If row from original legs has been deleted then simply remove it from the 
                // legs vector.
                if (row < (int)trans->legs.size()) {
                    legsToDelete.push_back(row);
                }
                continue;
            }

            int int_original_quantity = AfxValInteger(leg_original_quantity);   // will GPF if empty leg_original_quantity string
            if (int_original_quantity == 0) continue;

            int int_open_quantity = AfxValInteger(leg_open_quantity);   // will GPF if empty leg_open_quantity string
            if (int_open_quantity == 0) continue;

            // Determine if backpointers exist and if yes then re-use that leg instead
            // of creating a new leg in order to preserve the integrity of the data.
            std::shared_ptr<Leg> leg = nullptr;
        
            // Reuse existing leg
            if (row < (int)trans->legs.size()) {
                leg = trans->legs.at(row);
            }
            else {
                leg = std::make_shared<Leg>();
                trade->nextleg_id += 1;
                leg->leg_id = trade->nextleg_id;
            }

            leg->original_quantity = int_original_quantity * trans->quantity;
            leg->open_quantity = int_open_quantity * trans->quantity;

            leg->underlying = trans->underlying;
            leg->expiry_date = leg_expiry;
            leg->strike_price = leg_strike;
            leg->put_call = leg_PutCall;
            leg->action = leg_action;

            std::string expiry_date = AfxRemoveDateHyphens(leg_expiry);
            if (AfxValDouble(expiry_date) > AfxValDouble(trade->bp_end_date)) trade->bp_end_date = expiry_date;

            if (row >= (int)trans->legs.size()) {
                trans->legs.push_back(leg);
            }
        }

        // Remove any rows from the original legs that were deleted in full
        for (auto idx : legsToDelete) {
            trans->legs.erase(trans->legs.begin() + idx);
        }
    }
}


void SaveTradeData(AppState& state, TradeDialogData& tdd) {
    state.trade_dialog_trade->SetTradeOpenStatus();
    ReloadAppState(state);
}

