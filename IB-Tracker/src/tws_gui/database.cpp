#include "framework.h"
#include "trade.h"
#include "database.h"


const std::wstring dbFilename = L"trades.db";
const std::wstring idMagic = L"QTOPTIONSTRACKER-DATABASE";
const std::wstring version = L"1.0.0";

bool SaveDatabase()
{
    std::ofstream db(dbFilename);

    if (!db.is_open()) {
        int msgboxID = MessageBox(
            NULL,
            (LPCWSTR)(L"Could not save database " + dbFilename),
            (LPCWSTR)L"Warning",
            MB_ICONWARNING
        );
        return false;
    }

    db << idMagic << "|" << version << "\n"
        << "// TRADE  T|isOpen|TickerSymbol|TickerName|FutureExpiry\n"
        << "// TRANS  X|underlying|description|transDate|quantity|price|multiplier|fees|total\n"
        << "// LEG    L|origQuantity|openQuantity|expiryDate|strikePrice|PutCall|action|underlying\n"
        << "// isOpen:        0:TRUE, 1:FALSE\n"
        << "// FutureExpiry:  YYYYMM (do not insert hyphens)\n"
        << "// underlying:    0:OPTIONS, 1:SHARES, 2:FUTURES, 3:CURRENCY, 4:COMMODITIES\n"
        << "// action:        0:STO, 1:BTO, 2:STC, 3:BTC\n";

    for (const auto trade : trades) {
        out << "T|"
            << std::wstring(trade->isOpen ? "1|" : "0|")
            << trade->tickerSymbol << "|"
            << trade->tickerName << "|"
            << trade->futureExpiry
            << "\n";

        for (const auto trans : trade->transactions) {
            out << "X|"
                << RemoveDateHyphens(trans->transDate) << "|"
                << trans->description << "|"
                << UnderlyingToNumber(trans->underlying) << "|"
                << trans->quantity << "|"
                << std::wstring::number(trans->price, 'f', 4).toDouble() << "|"
                << trans->multiplier << "|"
                << std::wstring::number(trans->fees, 'f', 4).toDouble() << "|"
                << std::wstring::number(trans->total, 'f', 4).toDouble()
                << "\n";

            for (const auto leg : trans->legs) {
                out << "L|"
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


std::wstring InsertDateHyphens( const std::wstring& dateString)
{
    std::wstring newDate = dateString;
    // YYYYMMDD
    // 01234567

    newDate.insert(4, "-");
    // YYYY-MMDD
    // 012345678

    newDate.insert(7, "-");
    // YYYY-MM-DD
    // 0123456789

    return newDate;
}


std::wstring RemoveDateHyphens( const std::wstring& dateString)
{
    std::wstring newDate = dateString;
    newDate.remove("-");
    return newDate;
}


int UnderlyingToNumber( std::wstring& underlying)
{
    if (underlying == L"OPTIONS") return 0;
    if (underlying == L"SHARES") return 1;
    if (underlying == L"FUTURES") return 2;
    if (underlying == L"CURRENCY") return 3;
    if (underlying == L"COMMODITIES") return 4;
    return 0;
}


int ActionToNumber( std::wstring& action)
{
    if (action == L"STO") return 0;
    if (action == L"BTO") return 1;
    if (action == L"STC") return 2;
    if (action == L"BTC") return 3;
    return 0;
}


std::wstring NumberToUnderlying( int number)
{
    if (number == 0) return L"OPTIONS";
    if (number == 1) return L"SHARES";
    if (number == 2) return L"FUTURES";
    if (number == 3) return L"CURRENCY";
    if (number == 4) return L"COMMODITIES";
    return L"OPTIONS";
}


std::wstring NumberToAction( int number)
{
    if (number == 0) return L"STO";
    if (number == 1) return L"BTO";
    if (number == 2) return L"STC";
    if (number == 3) return L"BTC";
    return L"STO";
}


bool LoadDatabase()
{
    trades.reserve(5000);         // reserve space for 5000 trades

    QFile db(dbFilename);

    if (!db.open(QIODevice::ReadOnly | QIODevice::Text)) {
        int msgboxID = MessageBox(
            NULL,
            (LPCWSTR)(L"Could not read database " + dbFilename),
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

    QTextStream in(&db);
    while (!in.atEnd()) {
        std::wstring line = in.readLine();
        if (line.length() == 0) continue;

        // If this is a Comment line then simply iterate to next line.
        if (line.left(3) == "// ") continue;

        std::wstringList st = line.split("|");
        if (st.isEmpty()) continue;

        // First line must be the database identifier and version
        if (isFirstline) {
            if (st.at(0) != idMagic) {
                int msgboxID = MessageBox(
                    NULL,
                    (LPCWSTR)(L"Invalid database " + dbFilename),
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
            trans->underlying = NumberToUnderlying(st.at(3).toInt());
            trans->quantity = st.at(4).toDouble();
            trans->price = st.at(5).toDouble();
            trans->multiplier = st.at(6).toDouble();
            trans->fees = st.at(7).toDouble();
            trans->total = st.at(8).toDouble();
            trade->transactions.push_back(trans);
            continue;
        }

        if (st.at(0) == L"L") {
            leg = new Leg();
            leg->origQuantity = st.at(1).toInt();
            leg->openQuantity = st.at(2).toInt();
            leg->expiryDate = InsertDateHyphens(st.at(3));
            leg->strikePrice = st.at(4);
            leg->PutCall = st.at(5);
            leg->action = NumberToAction(st.at(6).toInt());
            leg->underlying = NumberToUnderlying(st.at(7).toInt());
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
