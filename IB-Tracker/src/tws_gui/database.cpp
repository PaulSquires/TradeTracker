#include "pch.h"
#include "trade.h"
#include "CWindow.h"
#include "Themes.h"
#include "database.h"



const std::wstring dbFilename = AfxGetExePath() + L"\\trades.db";

const std::wstring idMagic = L"IB-TRACKER-DATABASE";
const std::wstring version = L"1.0.0";

// Pointer list for all trades (initially loaded from database)
// This variable is defined as extern in the trade.h header file
// so that it can be shared throughout the entire application.
std::vector<Trade*> trades;



std::wstring InsertDateHyphens(const std::wstring& dateString)
{
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


int UnderlyingToNumber(std::wstring& underlying)
{
    if (underlying == L"OPTIONS") return 0;
    if (underlying == L"SHARES") return 1;
    if (underlying == L"FUTURES") return 2;
    if (underlying == L"CURRENCY") return 3;
    if (underlying == L"COMMODITIES") return 4;
    return 0;
}


int ActionToNumber(std::wstring& action)
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

// Function to split the string to words in a vector
// separated by the delimiter
std::vector<std::wstring> split(std::wstring str, std::wstring delimiter)
{
    std::vector<std::wstring> v;
    if (!str.empty()) {
        int start = 0;
        do {
            // Find the index of occurrence
            int idx = str.find(delimiter, start);
            if (idx == std::wstring::npos) {
                break;
            }

            // If found add the substring till that
            // occurrence in the vector
            int length = idx - start;
            v.push_back(str.substr(start, length));
            start += (length + delimiter.size());
        } while (true);
        v.push_back(str.substr(start));
    }

    return v;
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
        << "THEME|" << GetThemeName() << "\n"
        << "TRADERNAME|" << GetTraderName() << "\n"
        << "// TRADE  T|isOpen|TickerSymbol|TickerName|FutureExpiry\n"
        << "// TRANS  X|underlying|description|transDate|quantity|price|multiplier|fees|total\n"
        << "// LEG    L|origQuantity|openQuantity|expiryDate|strikePrice|PutCall|action|underlying\n"
        << "// isOpen:        0:TRUE, 1:FALSE\n"
        << "// FutureExpiry:  YYYYMM (do not insert hyphens)\n"
        << "// underlying:    0:OPTIONS, 1:SHARES, 2:FUTURES, 3:CURRENCY, 4:COMMODITIES\n"
        << "// action:        0:STO, 1:BTO, 2:STC, 3:BTC\n";

    for (const auto trade : trades) {
        db << "T|"
            << std::wstring(trade->isOpen ? L"1|" : L"0|")
            << trade->tickerSymbol << "|"
            << trade->tickerName << "|"
            << trade->futureExpiry
            << "\n";

        for (const auto trans : trade->transactions) {
            db << "X|"
                << RemoveDateHyphens(trans->transDate) << "|"
                << trans->description << "|"
                << UnderlyingToNumber(trans->underlying) << "|"
                << trans->quantity << "|"
                << std::setprecision(4) << trans->price << "|"
                << trans->multiplier << "|"
                << std::setprecision(4) << trans->fees << "|"
                << std::setprecision(4) << trans->total
                << "\n";

            for (const auto leg : trans->legs) {
                db << "L|"
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



bool LoadDatabase()
{
    trades.reserve(5000);         // reserve space for 5000 trades

    std::wifstream db; 

    db.open(dbFilename, std::ios::in);

    if (!db.is_open()) {
        int msgboxID = MessageBox(
            NULL,
            (LPCWSTR)(L"Could not read trades database"),
            (LPCWSTR)L"Warning",
            MB_ICONWARNING
        );
        return false;
    }

    std::wstring databaseVersion;


    Trade* trade = nullptr;
    Transaction* trans = nullptr;
    Leg* leg = nullptr;

    bool isFirstline = true;

    std::wstring line;

    while (!db.eof()) {
        std::getline(db, line);

        if (line.length() == 0) continue;

        // If this is a Comment line then simply iterate to next line.
        if (line.compare(1, 3, L"// ") == 0) continue;

        // Tokenize the line into a vector based on the pipe delimiter
        std::vector<std::wstring> st = split(line, L"|");

        if (st.empty()) continue;

        // First line must be the database identifier and version
        if (isFirstline) {
            if (st.at(0) != idMagic) {
                int msgboxID = MessageBox(
                    NULL,
                    (LPCWSTR)(L"Invalid trades database"),
                    (LPCWSTR)L"Warning",
                    MB_ICONWARNING
                );
                db.close();
                return false;
            }
            databaseVersion = st.at(1);
            isFirstline = false;
            continue;
        }


        // Check for configuration identifiers
        if (st.at(0) == L"THEME") {
            SetThemeName(st.at(1));
            continue;
        }

        // Check for configuration identifiers
        if (st.at(0) == L"TRADERNAME") {
            std::wstring wszTraderName = st.at(1);
            if (wszTraderName.length() == 0)
                wszTraderName = AfxGetUserName();
            SetTraderName(wszTraderName);
            continue;
        }


        // Check for Trades, Transactions, and Legs

        if (st.at(0) == L"T") {
            trade = new Trade();
            trade->isOpen = (st.at(1) == L"0" ? false : true);
            trade->tickerSymbol = st.at(2);
            trade->tickerName = st.at(3);
            trade->futureExpiry = st.at(4);
            trades.push_back(trade);
            continue;
        }

        if (st.at(0) == L"X") {
            trans = new Transaction();
            trans->transDate = InsertDateHyphens(st.at(1));
            trans->description = st.at(2);
            trans->underlying = NumberToUnderlying(stoi(st.at(3)));
            trans->quantity = stoi(st.at(4));
            trans->price = stod(st.at(5));
            trans->multiplier = stod(st.at(6));
            trans->fees = stod(st.at(7));
            trans->total = stod(st.at(8));
            if (trade != nullptr)
                trade->transactions.push_back(trans);
            continue;
        }

        if (st.at(0) == L"L") {
            leg = new Leg();
            leg->origQuantity = stoi(st.at(1));
            leg->openQuantity = stoi(st.at(2));
            leg->expiryDate = InsertDateHyphens(st.at(3));
            leg->strikePrice = st.at(4);
            leg->PutCall = st.at(5);
            leg->action = NumberToAction(stoi(st.at(6)));
            leg->underlying = NumberToUnderlying(stoi(st.at(7)));
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
