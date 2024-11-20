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

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <sstream>

#include "appstate.h"
#include "utilities.h"
#include "messagebox.h"


CDatabase::CDatabase() {
    dbFilename = GetDataFilesFolder() + "/tt-database.db";
    dbJournalNotes = GetDataFilesFolder() + "/tt-notes.txt";
}


std::string CDatabase::PutCallToString(const PutCall e) {
    if (e == PutCall::Put) return "P";
    if (e == PutCall::Call) return "C";
    return "";
}


PutCall CDatabase::StringToPutCall(const std::string& text) {
    if (text == "P") return PutCall::Put;
    if (text == "C") return PutCall::Call;
    if (text.length() == 0) return PutCall::Nothing;
    return PutCall::Nothing;
}


Underlying CDatabase::StringToUnderlying(const std::string& text) {
    static const std::unordered_map<std::string, Underlying> map = {
        {"0", Underlying::Options}, {"1", Underlying::Shares}, 
        {"2", Underlying::Futures}, {"3", Underlying::Dividend},
        {"4", Underlying::Other}
    };

    if (text.length() == 0) return Underlying::Nothing;

    // Search for an element in the map
    auto it = map.find(text);
    return (it != map.end()) ? it->second : Underlying::Nothing;
}


std::string CDatabase::UnderlyingToString(const Underlying e) {
    return std::to_string((int)e);
}


Action CDatabase::StringDescriptionToAction(const std::string& text) {
    static const std::unordered_map<std::string, Action> map = {
        {"STO", Action::STO}, {"BTO", Action::BTO}, 
        {"STC", Action::STC}, {"BTC", Action::BTC},
        {"", Action::Nothing}
    };

    if (text.length() == 0) return Action::Nothing;

    // Search for an element in the map
    auto it = map.find(text);
    return (it != map.end()) ? it->second : Action::Nothing;
}

Action CDatabase::StringToAction(const std::string& text) {
    static const std::unordered_map<std::string, Action> map = {
        {"0", Action::STO}, {"1", Action::BTO}, 
        {"2", Action::STC}, {"3", Action::BTC},
        {"", Action::Nothing}
    };

    if (text.length() == 0) return Action::Nothing;

    // Search for an element in the map
    auto it = map.find(text);
    return (it != map.end()) ? it->second : Action::Nothing;
}


std::string CDatabase::ActionToString(const Action e) {
    return std::to_string((int)e);
}

std::string CDatabase::ActionToStringDescription(const Action e) {
    switch (e) {
    case Action::STO: return "STO";
    case Action::BTO: return "BTO";
    case Action::STC: return "STC";
    case Action::BTC: return "BTC";
    case Action::Nothing : return "";
    default: return "";
    } 
}


bool CDatabase::SaveDatabase(AppState& state) {
    // Create and open a text file
    std::ofstream db(dbFilename);

    if (!db.is_open()) {
        CustomMessageBox(state, "Warning", "Could not save trades database.");
        return false;
    }

    std::ostringstream text;

    text << "// TRADE          T|isOpen|nextleg_id|TickerSymbol|TickerName|FutureExpiry|Category|TradeBP|Notes|3dteWarning|21dteWarning|TradePercentage\n"
         << "// TRANS          X|transDate|description|underlying|quantity|price|multiplier|fees|total|SharesAction\n"
         << "// LEG            L|leg_id|leg_back_pointer_id|original_quantity|open_quantity|expiry_date|strike_price|PutCall|action|underlying\n"
         << "// isOpen:        0:false, 1:true\n"
         << "// FutureExpiry:  YYYYMMDD (do not insert hyphens)\n"
         << "// Category:      0,1,2,3,4, etc (integer value)\n"
         << "// underlying:    0:OPTIONS, 1:SHARES, 2:FUTURES, 3:DIVIDEND, 4:OTHER\n"
         << "// action:        0:STO, 1:BTO, 2:STC, 3:BTC\n"
         << "// Dates are all in YYYYMMDD format with no embedded separators.\n";

    bool prev_trade_was_open = false;

    for (const auto& trade : trades) {
        // Insert a blank line if the trade is open in order to give some visual breathing room
        if (trade->is_open || prev_trade_was_open) text << "\n";
        prev_trade_was_open = trade->is_open;

        text << "T|"
             << std::string(trade->is_open ? "1|" : "0|")
             << trade->nextleg_id << "|"
             << trade->ticker_symbol << "|"
             << trade->ticker_name << "|"
             << AfxReplace(trade->future_expiry, "-", "") << "|"
             << trade->category << "|"
             << AfxDoubleToString(trade->trade_bp, 0)  << "|"
             << AfxReplace(trade->notes, "\n", "~~") << "|"
             << trade->warning_3_dte << "|"
             << trade->warning_21_dte << "|"
             << AfxDoubleToString(trade->trade_completed_percentage, 4)
             << "\n";

        static std::string p0 = "";
        static std::string p2 = "  ";
        static std::string p4 = "    ";

        for (const auto& trans : trade->transactions) {
            text << (trade->is_open ? p2 : p0) << "X|" 
                 << AfxReplace(trans->trans_date, "-", "") << "|"
                 << trans->description << "|"
                 << UnderlyingToString(trans->underlying) << "|"
                 << trans->quantity << "|"
                 << AfxDoubleToString(trans->price, 4) << "|"
                 << AfxDoubleToString(trans->multiplier, 4) << "|"
                 << AfxDoubleToString(trans->fees, 4) << "|"
                 << AfxDoubleToString(trans->total, 4) << "|"
                 << ActionToString(trans->share_action)
                 << "\n";

            for (const auto& leg : trans->legs) {
                 text << (trade->is_open ? p4 : p0) << "L|"
                      << leg->leg_id << "|"
                      << leg->leg_back_pointer_id << "|"
                      << leg->original_quantity << "|"
                      << leg->open_quantity << "|"
                      << AfxReplace(leg->expiry_date, "-", "") << "|"
                      << leg->strike_price << "|"
                      << PutCallToString(leg->put_call) << "|"
                      << ActionToString(leg->action) << "|"
                      << UnderlyingToString(leg->underlying)
                      << "\n";
            }
        }
    }

    db << text.str();
    db.close();

    return true;
}


inline static std::string try_catch_string(const std::vector<std::string>& st, const int idx) {
    if (idx >= st.size() || idx < 0) return "";
    return st.at(idx);
}


inline static int try_catch_int(const std::vector<std::string>& st, const int idx) {
    std::string text = try_catch_string(st, idx);
    return AfxValInteger(text);
}


inline static double try_catch_double(const std::vector<std::string>& st, const int idx) {
    std::string text = try_catch_string(st, idx);
    return AfxValDouble(text);
}


bool CDatabase::LoadDatabase(AppState& state) {
    trades.clear();
    trades.reserve(5000);         // reserve space for 5000 trades

    // If database file does not exist then simply exit because default values
    // will be used and then saved to disk.
    if (!AfxFileExists(dbFilename)) return true;

    std::ifstream db(dbFilename);
    if (!db) {
        CustomMessageBox(state, "Warning", "Could not load trades database.");
        return false;
    }

    std::shared_ptr<Trade> trade;
    std::shared_ptr<Transaction> trans;
    std::shared_ptr<Leg> leg; 

    std::string line;
    std::string text;
    std::string date_text;
    std::string expiry_date;
    
    std::vector<std::string> st;

    while (getline(db, line)) {
        // Trim leading and trailing white space
        line = AfxTrim(line);

        if (line.length() == 0) continue;

        // If this is a Comment line then simply iterate to next line.
        if (line.compare(1, 3, "// ") == 0) continue;

        // Tokenize the line into a vector based on the pipe delimiter
        st = AfxSplit(line, '|');

        if (st.empty()) continue;

        // Check for Trades, Trans, and Legs
        std::string action = try_catch_string(st, 0);

        if (action == "T") {  // TRADE
            trade = std::make_shared<Trade>();
            trade->is_open       = (try_catch_string(st, 1) == "0") ? false : true;
            trade->nextleg_id    = try_catch_int(st, 2);
            trade->ticker_symbol = try_catch_string(st, 3);
            trade->ticker_name   = try_catch_string(st, 4);
            date_text            = try_catch_string(st, 5);
            trade->future_expiry = AfxInsertDateHyphens(date_text);
            trade->category      = try_catch_int(st, 6);
            trade->trade_bp      = try_catch_double(st, 7);
            text                 = try_catch_string(st, 8);
            trade->notes         = AfxReplace(text, "~~", "\n");
            trade->warning_3_dte  = try_catch_int(st, 9);
            trade->warning_21_dte = try_catch_int(st, 10);
            trade->trade_completed_percentage = try_catch_double(st, 11);
            
            trades.emplace_back(trade);
            continue;
        }

        if (action == "X") {  // TRANSACTION
            trans = std::make_shared<Transaction>();
            date_text            = try_catch_string(st, 1);
            trans->trans_date    = AfxInsertDateHyphens(date_text);
            trans->description   = try_catch_string(st, 2);
            trans->underlying    = StringToUnderlying(try_catch_string(st, 3));
            trans->quantity      = try_catch_int(st, 4);
            trans->price         = try_catch_double(st, 5);
            trans->multiplier    = try_catch_double(st, 6);
            trans->fees          = try_catch_double(st, 7);
            trans->total         = try_catch_double(st, 8);
            trans->share_action  = StringToAction(try_catch_string(st, 9));
            if (trans->share_action == Action::Nothing) {
                trans->share_action = (trans->total < 0) ? Action::BTO: Action::STO;
            }

            if (trade) {
                // Determine earliest and latest dates for BP ROI calculation.
                if (AfxValDouble(date_text) < AfxValDouble(trade->bp_start_date)) trade->bp_start_date = date_text;
                if (AfxValDouble(date_text) > AfxValDouble(trade->bp_end_date)) trade->bp_end_date = date_text;
                if (AfxValDouble(date_text) > AfxValDouble(trade->oldest_trade_trans_date)) trade->oldest_trade_trans_date = date_text;
                trade->transactions.emplace_back(trans);
            }
            continue;
        }

        if (action == "L") {  // LEG
            leg = std::make_shared<Leg>();
            leg->leg_id              = try_catch_int(st, 1);
            leg->leg_back_pointer_id = try_catch_int(st, 2);
            leg->original_quantity   = try_catch_int(st, 3);
            leg->open_quantity       = try_catch_int(st, 4);
            expiry_date              = try_catch_string(st, 5);
            leg->expiry_date         = AfxInsertDateHyphens(expiry_date);
            leg->strike_price        = try_catch_string(st, 6);
            leg->put_call            = StringToPutCall(try_catch_string(st, 7)); 
            leg->action              = StringToAction(try_catch_string(st, 8));
            leg->underlying          = StringToUnderlying(try_catch_string(st, 9));

            if (trans) {
                leg->trans = trans;
                if (trade) {
                    // Determine latest date for BP ROI calculation.
                    if (AfxValDouble(expiry_date) > AfxValDouble(trade->bp_end_date)) trade->bp_end_date = expiry_date;
                }
                trans->legs.emplace_back(leg);
            }
            continue;
        }
    }

    db.close();


    // Now that the trades have been constructed, create the open position vector based
    // on a sorted list of open legs. We also calculate the ACB for the entire Trade
    // rather than physically storing that value in the database. This allows us to
    // manually edit individual Transactions externally and not have to go through
    // an error prone process of recalculating the ACB with the new change.
    for (auto& trade : trades) {
        if (trade->is_open) {
            trade->CreateOpenLegsVector();
        }
        else {
            // Trade is closed so set the BPendDate to be the oldest transaction in the Trade
            trade->bp_end_date = trade->oldest_trade_trans_date;
        }

        // Calculate the full trade ACB and also the Shares ACB depending on what costing
        // method has been chosen.
        trade->CalculateAdjustedCostBase(state);
    }

    return true;
}


