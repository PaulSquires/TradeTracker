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

#include "CustomMessageBox/CustomMessageBox.h"
#include "Utilities/AfxWin.h"
#include "Config/Config.h"

#include "trade.h"
#include "database.h"


// Pointer list for all trades (initially loaded from database)
// This variable is accessed via extern in other files that require it.
std::vector<std::shared_ptr<Trade>> trades;


bool CDatabase::Version4UpgradeDatabase() {
    std::wstring dbFilename_filename = config.GetDataFilesFolder() + dbFilename_new;

    // If version 4 filenames already exist then we would have already upgraded the
    // files previously,
    if (AfxFileExists(dbFilename_filename) || !AfxFileExists(dbFilename_old)) {
        dbFilename = dbFilename_filename;
        return false;
    }
    else {
        // Old files will be renamed after they are first loaded into memory.
        dbFilename = dbFilename_old;
        return true;
    }
}


bool CDatabase::Version4UpgradeJournalNotes() {
    std::wstring dbJournalNotes_filename = config.GetDataFilesFolder() + dbJournalNotes_new;

    // If version 4 filenames already exist then we would have already upgraded the
    // files previously,
    if (AfxFileExists(dbJournalNotes_filename) || !AfxFileExists(dbJournalNotes_old)) {
        dbJournalNotes = dbJournalNotes_filename;
        return false;
    }
    else {
        // Old files will be renamed after they are first loaded into memory.
        dbJournalNotes = dbJournalNotes_old;
        return true;
    }
}

bool CDatabase::Version4UpgradeTradePlan() {
    std::wstring dbTradePlan_filename = config.GetDataFilesFolder() + dbTradePlan_new;

    // If version 4 filenames already exist then we would have already upgraded the
    // files previously,
    if (AfxFileExists(dbTradePlan_filename) || !AfxFileExists(dbTradePlan_old)) {
        dbTradePlan = dbTradePlan_filename;
        return false;
    }
    else {
        // Old files will be renamed after they are first loaded into memory.
        dbTradePlan = dbTradePlan_old;
        return true;
    }
}


std::wstring CDatabase::PutCallToString(const PutCall e) {
    if (e == PutCall::Put) return L"P";
    if (e == PutCall::Call) return L"C";
    return L"";
}


PutCall CDatabase::StringToPutCall(const std::wstring& text) {
    if (text == L"P") return PutCall::Put;
    if (text == L"C") return PutCall::Call;
    if (text.length() == 0) return PutCall::Nothing;
    return PutCall::Nothing;
}


Underlying CDatabase::StringToUnderlying(const std::wstring& text) {
    static const std::unordered_map<std::wstring, Underlying> map = {
        {L"0", Underlying::Options}, {L"1", Underlying::Shares}, 
        {L"2", Underlying::Futures}, {L"3", Underlying::Dividend},
        {L"4", Underlying::Other}
    };

    if (text.length() == 0) return Underlying::Nothing;

    // Search for an element in the map
    auto it = map.find(text);
    return (it != map.end()) ? it->second : Underlying::Nothing;
}


std::wstring CDatabase::UnderlyingToString(const Underlying e) {
    return std::to_wstring((int)e);
}


Action CDatabase::StringDescriptionToAction(const std::wstring& text) {
    static const std::unordered_map<std::wstring, Action> map = {
        {L"STO", Action::STO}, {L"BTO", Action::BTO}, 
        {L"STC", Action::STC}, {L"BTC", Action::BTC},
        {L"", Action::Nothing}
    };

    if (text.length() == 0) return Action::Nothing;

    // Search for an element in the map
    auto it = map.find(text);
    return (it != map.end()) ? it->second : Action::Nothing;
}

Action CDatabase::StringToAction(const std::wstring& text) {
    static const std::unordered_map<std::wstring, Action> map = {
        {L"0", Action::STO}, {L"1", Action::BTO}, 
        {L"2", Action::STC}, {L"3", Action::BTC},
        {L"", Action::Nothing}
    };

    if (text.length() == 0) return Action::Nothing;

    // Search for an element in the map
    auto it = map.find(text);
    return (it != map.end()) ? it->second : Action::Nothing;
}


std::wstring CDatabase::ActionToString(const Action e) {
    return std::to_wstring((int)e);
}

std::wstring CDatabase::ActionToStringDescription(const Action e) {
    switch (e) {
    case Action::STO: return L"STO";
    case Action::BTO: return L"BTO";
    case Action::STC: return L"STC";
    case Action::BTC: return L"BTC";
    case Action::Nothing : return L"";
    default: return L"";
    } 
}


// ========================================================================================
// Get the JournalNotes text.
// ========================================================================================
std::wstring CDatabase::GetJournalNotesText() {
    bool upgrade_to_version4 = Version4UpgradeJournalNotes();

    static bool is_journal_notes_loaded = false;

    if (!is_journal_notes_loaded) {
        std::wifstream db;

        db.open(dbJournalNotes, std::ios::in);

        if (db.is_open()) {
            std::wostringstream ss;
            ss << db.rdbuf();
            journal_notes_text = ss.str();

            is_journal_notes_loaded = true;
        }
    }

    if (upgrade_to_version4) {
        dbJournalNotes = config.GetDataFilesFolder() + dbJournalNotes_new;
        SetJournalNotesText(journal_notes_text);
        // Delete the older version file
        DeleteFile(dbJournalNotes_old.c_str());
    }

    return journal_notes_text;
}


// ========================================================================================
// Set and save the JournalNotes text.
// ========================================================================================
void CDatabase::SetJournalNotesText(const std::wstring& text) {
    std::wofstream db;

    db.open(dbJournalNotes, std::ios::out | std::ios::trunc);

    if (!db.is_open()) {
        CustomMessageBox.Show(
            NULL,
            L"Could not save Journal Notes text to file",
            L"Warning",
            MB_ICONWARNING
        );
        return;
    }

    db << text;
    db.close();

    journal_notes_text = text;
}


// ========================================================================================
// Get the TradePlan text.
// ========================================================================================
std::wstring CDatabase::GetTradePlanText() {
    bool upgrade_to_version4 = Version4UpgradeTradePlan();

    static bool is_trade_plan_loaded = false;

    if (!is_trade_plan_loaded) {
        std::wifstream db;

        db.open(dbTradePlan, std::ios::in);

        if (db.is_open()) {
            std::wostringstream ss;
            ss << db.rdbuf();
            trade_plan_text = ss.str();

            is_trade_plan_loaded = true;
        }
    }

    if (upgrade_to_version4) {
        dbTradePlan = config.GetDataFilesFolder() + dbTradePlan_new;
        SetTradePlanText(trade_plan_text);
        // Delete the older version file
        DeleteFile(dbTradePlan_old.c_str());
    }

    return trade_plan_text;
}


// ========================================================================================
// Set and save the TradePlan text.
// ========================================================================================
void CDatabase::SetTradePlanText(const std::wstring& text) {
    std::wofstream db;

    db.open(dbTradePlan, std::ios::out | std::ios::trunc);

    if (!db.is_open()) {
        CustomMessageBox.Show(
            NULL,
            L"Could not save Trade Plan text to file",
            L"Warning",
            MB_ICONWARNING
        );
        return;
    }

    db << text;
    db.close();

    trade_plan_text = text;
}


bool CDatabase::SaveDatabase() {
    std::wofstream db;

    db.open(dbFilename, std::ios::out | std::ios::trunc);

    if (!db.is_open()) {
        CustomMessageBox.Show(
            NULL,
            L"Could not save trades database",
            L"Warning",
            MB_ICONWARNING
        );
        return false;
    }

    db  << "// TRADE          T|isOpen|nextleg_id|TickerSymbol|TickerName|FutureExpiry|Category|TradeBP|Notes\n"
        << "// TRANS          X|transDate|description|underlying|quantity|price|multiplier|fees|total|SharesLongShort\n"
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
        if (trade->is_open || prev_trade_was_open) db << "\n";
        prev_trade_was_open = trade->is_open;

        db << "T|"
            << std::wstring(trade->is_open ? L"1|" : L"0|")
            << trade->nextleg_id << "|"
            << trade->ticker_symbol << "|"
            << trade->ticker_name << "|"
            << AfxRemoveDateHyphens(trade->future_expiry) << "|"
            << trade->category << "|"
            << std::fixed << std::setprecision(0) << trade->trade_bp  << "|"
            << AfxReplace(trade->notes, L"\r\n", L"~~") 
            << "\n";

        static std::wstring p0 = L"";
        static std::wstring p2 = L"  ";
        static std::wstring p4 = L"    ";

        for (const auto& trans : trade->transactions) {
            db << (trade->is_open ? p2 : p0) << "X|" 
                << AfxRemoveDateHyphens(trans->trans_date) << "|"
                << trans->description << "|"
                << UnderlyingToString(trans->underlying) << "|"
                << trans->quantity << "|"
                << std::fixed << std::setprecision(4) << trans->price << "|"
                << std::fixed << std::setprecision(4) << trans->multiplier << "|"
                << std::fixed << std::setprecision(4) << trans->fees << "|"
                << std::fixed << std::setprecision(4) << trans->total << "|"
                << ActionToString(trans->share_action)
                << "\n";

            for (const auto& leg : trans->legs) {
                 db << (trade->is_open ? p4 : p0) << "L|"
                    << leg->leg_id << "|"
                    << leg->leg_back_pointer_id << "|"
                    << leg->original_quantity << "|"
                    << leg->open_quantity << "|"
                    << AfxRemoveDateHyphens(leg->expiry_date) << "|"
                    << leg->strike_price << "|"
                    << PutCallToString(leg->put_call) << "|"
                    << ActionToString(leg->action) << "|"
                    << UnderlyingToString(leg->underlying)
                    << "\n";
            }
        }
    }
    db.close();

    return true;
}


inline static std::wstring try_catch_wstring(const std::vector<std::wstring>& st, const int idx) {
    if (idx >= st.size() || idx < 0) return L"";
    return st.at(idx);
}


inline static int try_catch_int(const std::vector<std::wstring>& st, const int idx) {
    std::wstring text = try_catch_wstring(st, idx);
    return AfxValInteger(text);
}


inline static double try_catch_double(const std::vector<std::wstring>& st, const int idx) {
    std::wstring text = try_catch_wstring(st, idx);
    return AfxValDouble(text);
}


bool CDatabase::LoadDatabase() {
    bool upgrade_to_version4 = Version4UpgradeDatabase();

    trades.clear();
    trades.reserve(5000);         // reserve space for 5000 trades

    std::wifstream db;

    db.open(dbFilename, std::ios::in);

    if (!db.is_open()) 
        return false;

    std::wstring databaseVersion;

    std::shared_ptr<Trade> trade;
    std::shared_ptr<Transaction> trans;
    std::shared_ptr<Leg> leg; 

    std::wstring line;
    std::wstring text;
    std::wstring date_text;
    std::wstring expiry_date;
    
    std::vector<std::wstring> st;

    while (!db.eof()) {
        std::getline(db, line);

        if (line.length() == 0) continue;

        // If this is a Comment line then simply iterate to next line.
        if (line.compare(1, 3, L"// ") == 0) continue;

        // Trim leading white space
        line = AfxLTrim(line);

        // Tokenize the line into a vector based on the pipe delimiter
        st = AfxSplit(line, L'|');

        if (st.empty()) continue;

        // Check for Trades, Trans, and Legs

        if (try_catch_wstring(st, 0) == L"T") {
            trade = std::make_shared<Trade>();
            trade->is_open       = (try_catch_wstring(st, 1) == L"0") ? false : true;
            trade->nextleg_id    = try_catch_int(st, 2);
            trade->ticker_symbol = try_catch_wstring(st, 3);
            trade->ticker_name   = try_catch_wstring(st, 4);
            date_text            = try_catch_wstring(st, 5);
            trade->future_expiry = AfxInsertDateHyphens(date_text);
            trade->category      = try_catch_int(st, 6);
            trade->trade_bp      = try_catch_double(st, 7);
            text                 = try_catch_wstring(st, 8);
            trade->notes         = AfxReplace(text, L"~~", L"\r\n");
            
            trades.emplace_back(trade);
            continue;
        }

        if (try_catch_wstring(st, 0) == L"X") {
            trans = std::make_shared<Transaction>();
            date_text            = try_catch_wstring(st, 1);
            trans->trans_date    = AfxInsertDateHyphens(date_text);
            trans->description   = try_catch_wstring(st, 2);
            trans->underlying    = StringToUnderlying(try_catch_wstring(st, 3));
            trans->quantity      = try_catch_int(st, 4);
            trans->price         = try_catch_double(st, 5);
            trans->multiplier    = try_catch_double(st, 6);
            trans->fees          = try_catch_double(st, 7);
            trans->total         = try_catch_double(st, 8);
            trans->share_action  = StringToAction(try_catch_wstring(st, 9));
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

        if (try_catch_wstring(st, 0) == L"L") {
            leg = std::make_shared<Leg>();
            leg->leg_id              = try_catch_int(st, 1);
            leg->leg_back_pointer_id = try_catch_int(st, 2);
            leg->original_quantity   = try_catch_int(st, 3);
            leg->open_quantity       = try_catch_int(st, 4);
            expiry_date              = try_catch_wstring(st, 5);
            leg->expiry_date         = AfxInsertDateHyphens(expiry_date);
            leg->strike_price        = try_catch_wstring(st, 6);
            leg->put_call            = StringToPutCall(try_catch_wstring(st, 7)); 
            leg->action              = StringToAction(try_catch_wstring(st, 8));
            leg->underlying          = StringToUnderlying(try_catch_wstring(st, 9));
            
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
        trade->CalculateAdjustedCostBase();
    }

    if (upgrade_to_version4) {
        std::cout << "Upgrade database to version 4" << std::endl;
        dbFilename = config.GetDataFilesFolder() + dbFilename_new;
        SaveDatabase();
        // Delete the older version file
        DeleteFile(dbFilename_old.c_str());
        // Also load the TradePlan and JournalNotes so that they can be converted
        // now rather than waiting for the user to click on their tabs.
        GetJournalNotesText();
        GetTradePlanText();
    }

    return true;
}
