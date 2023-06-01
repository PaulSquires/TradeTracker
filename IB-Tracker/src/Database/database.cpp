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

#include "..\Utilities\AfxWin.h"

#include "trade.h"
#include "database.h"



const std::wstring dbFilename = AfxGetExePath() + L"\\IB-Tracker-database.db";

const std::wstring idMagic = L"IB-TRACKER-DATABASE";
const std::wstring version = L"1.0.0";

// Pointer list for all trades (initially loaded from database)
// This variable is accessed via extern in other files that require it.
std::vector<std::shared_ptr<Trade>> trades;



std::wstring InsertDateHyphens(const std::wstring& dateString)
{
    if (dateString.length() != 8) return L"";

    std::wstring newDate = dateString;
    // YYYYMMDD
    // 01234567

    newDate.insert(4, L"-");
    // YYYY-MMDD
    // 012345678

    newDate.insert(7, L"-");
    // YYYY-MM-DD
    // 0123456789

    return newDate;
}


std::wstring RemoveDateHyphens(const std::wstring& dateString)
{
    std::wstring newDate = dateString;
    newDate.erase(remove(newDate.begin(), newDate.end(), L'-'), newDate.end()); 
    return newDate;
}


int UnderlyingToNumber(const std::wstring& underlying)
{
    if (underlying == L"OPTIONS") return 0;
    if (underlying == L"SHARES") return 1;
    if (underlying == L"FUTURES") return 2;
    if (underlying == L"CURRENCY") return 3;
    if (underlying == L"COMMODITIES") return 4;
    return 0;
}


int ActionToNumber(const std::wstring& action)
{
    if (action == L"STO") return 0;
    if (action == L"BTO") return 1;
    if (action == L"STC") return 2;
    if (action == L"BTC") return 3;
    return 0;
}


std::wstring NumberToUnderlying(int number)
{
    if (number == 0) return L"OPTIONS";
    if (number == 1) return L"SHARES";
    if (number == 2) return L"FUTURES";
    if (number == 3) return L"CURRENCY";
    if (number == 4) return L"COMMODITIES";
    return L"OPTIONS";
}


std::wstring NumberToAction(int number)
{
    if (number == 0) return L"STO";
    if (number == 1) return L"BTO";
    if (number == 2) return L"STC";
    if (number == 3) return L"BTC";
    return L"STO";
}


bool SaveDatabase()
{
    std::wofstream db;

    db.open(dbFilename, std::ios::out | std::ios::trunc);

    if (!db.is_open()) {
        int msgboxID = MessageBox(
            NULL,
            (LPCWSTR)(L"Could not save trades database"),
            (LPCWSTR)L"Warning",
            MB_ICONWARNING
        );
        return false;
    }

    db << idMagic << "|" << version << "\n"
        << "// TRADE          T|isOpen|nextLegID|TickerSymbol|TickerName|FutureExpiry|Category\n"
        << "// TRANS          X|transDate|description|underlying|quantity|price|multiplier|fees|total\n"
        << "// LEG            L|legID|legBackPointerID|origQuantity|openQuantity|expiryDate|strikePrice|PutCall|action|underlying\n"
        << "// isOpen:        0:FALSE, 1:TRUE\n"
        << "// FutureExpiry:  YYYYMMDD (do not insert hyphens)\n"
        << "// Category:      0,1,2,3,4 (integer value)\n"
        << "// underlying:    0:OPTIONS, 1:SHARES, 2:FUTURES, 3:CURRENCY, 4:COMMODITIES\n"
        << "// action:        0:STO, 1:BTO, 2:STC, 3:BTC\n"
        << "// Dates are all in YYYYMMDD format with no embedded separators.\n";

    for (const auto trade : trades) {
        db << "T|"
            << std::wstring(trade->isOpen ? L"1|" : L"0|")
            << trade->nextLegID << "|"
            << trade->tickerSymbol << "|"
            << trade->tickerName << "|"
            << RemoveDateHyphens(trade->futureExpiry) << "|"
            << trade->category
            << "\n";

        for (const auto trans : trade->transactions) {
            db << "X|"
                << RemoveDateHyphens(trans->transDate) << "|"
                << trans->description << "|"
                << UnderlyingToNumber(trans->underlying) << "|"
                << trans->quantity << "|"
                << std::fixed << std::setprecision(4) << trans->price << "|"
                << std::fixed << std::setprecision(4) << trans->multiplier << "|"
                << std::fixed << std::setprecision(4) << trans->fees << "|"
                << std::fixed << std::setprecision(4) << trans->total
                << "\n";

            for (const auto leg : trans->legs) {
                db << "L|"
                    << leg->legID << "|"
                    << leg->legBackPointerID << "|"
                    << leg->origQuantity << "|"
                    << leg->openQuantity << "|"
                    << RemoveDateHyphens(leg->expiryDate) << "|"
                    << leg->strikePrice << "|"
                    << leg->PutCall << "|"
                    << ActionToNumber(leg->action) << "|"
                    << UnderlyingToNumber(leg->underlying)
                    << "\n";
            }
        }
    }
    db.close();

    return true;
}


int try_catch_int(std::vector<std::wstring>& st, int idx) {
    try {
        return stoi(st.at(idx));
    }
    catch (...) {
        return 0;
    }
}

double try_catch_double(std::vector<std::wstring>& st, int idx) {
    try {
        return stod(st.at(idx));
    }
    catch (...) {
        return 0;
    }
}

std::wstring try_catch_wstring(std::vector<std::wstring>& st, int idx) {
    try {
        return st.at(idx);
    }
    catch (...) {
        return L"";
    }
}


bool LoadDatabase()
{
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

    bool isFirstline = true;

    std::wstring line;

    while (!db.eof()) {
        std::getline(db, line);

        if (line.length() == 0) continue;

        // If this is a Comment line then simply iterate to next line.
        if (line.compare(1, 3, L"// ") == 0) continue;

        // Tokenize the line into a vector based on the pipe delimiter
        std::vector<std::wstring> st = AfxSplit(line, L"|");

        if (st.empty()) continue;

        // First line must be the database identifier and version
        if (isFirstline) {
            if (try_catch_wstring(st, 0) != idMagic) {
                int msgboxID = MessageBox(
                    NULL,
                    (LPCWSTR)(L"Invalid trades database"),
                    (LPCWSTR)L"Warning",
                    MB_ICONWARNING
                );
                db.close();
                return false;
            }
            databaseVersion = try_catch_wstring(st, 1);
            isFirstline = false;
            continue;
        }


        // Check for Trades, Transactions, and Legs

        if (try_catch_wstring(st, 0) == L"T") {
            trade = std::make_shared<Trade>();
            trade->isOpen = (try_catch_wstring(st, 1) == L"0" ? false : true);
            trade->nextLegID = try_catch_int(st, 2);
            trade->tickerSymbol = try_catch_wstring(st, 3);
            trade->tickerName = try_catch_wstring(st, 4);
            trade->futureExpiry = InsertDateHyphens(try_catch_wstring(st, 5));
            trade->category = try_catch_int(st, 6);
            trades.push_back(trade);
            continue;
        }

        if (try_catch_wstring(st, 0) == L"X") {
            trans = std::make_shared<Transaction>();
            trans->transDate = InsertDateHyphens(try_catch_wstring(st, 1));
            trans->description = try_catch_wstring(st, 2);
            trans->underlying = NumberToUnderlying(try_catch_int(st, 3));
            trans->quantity = try_catch_int(st, 4);
            trans->price = try_catch_double(st, 5);
            trans->multiplier = try_catch_double(st, 6);
            trans->fees = try_catch_double(st, 7);
            trans->total = try_catch_double(st, 8);
            if (trade != nullptr)
                trade->transactions.push_back(trans);
            continue;
        }

        if (try_catch_wstring(st, 0) == L"L") {
            leg = std::make_shared<Leg>();
            leg->legID = try_catch_int(st, 1);
            leg->legBackPointerID = try_catch_int(st, 2);
            leg->origQuantity = try_catch_int(st, 3);
            leg->openQuantity = try_catch_int(st, 4);
            leg->expiryDate = InsertDateHyphens(try_catch_wstring(st, 5));
            leg->strikePrice = try_catch_wstring(st, 6);
            leg->PutCall = try_catch_wstring(st, 7); 
            leg->action = NumberToAction(try_catch_int(st, 8));
            leg->underlying = NumberToUnderlying(try_catch_int(st, 9));
            if (trans != nullptr)
                trans->legs.push_back(leg);
            continue;
        }
    
    }

    // Now that the trades have been constructed, create the open position vector based
    // on a sorted list of open legs. We also calculate the ACB for the entire Trade
    // rather than physically storing that value in the database. This allows us to
    // manually edit individual Transactions externally and not have to go through
    // an error prone process of recalculating the ACB with the new change.
    for (auto trade : trades) {
        if (trade->isOpen) trade->createOpenLegsVector();

        trade->ACB = 0;
        for (const auto trans : trade->transactions) {
            trade->ACB = trade->ACB + trans->total;
        }
    }

    db.close();

    return true;
}
