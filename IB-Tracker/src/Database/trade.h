#pragma once

//  The hierarchy of the data is:
//      Trade
//          |-> Transaction
//                        |-> Legs

//  All open and closed Trades are stored in the global Trades vector. We differentiate
//  between them by iterating the vector and testing the isOpen() boolena method.
//

class Leg
{
public:
    int          origQuantity = 0;
    int          openQuantity = 0;
    std::wstring expiryDate   = L"";
    std::wstring strikePrice  = L"";
    std::wstring PutCall      = L"";
    std::wstring action       = L"";   // STO,BTO,STC,BTC
    std::wstring underlying   = L"";   // OPTIONS, STOCKS, FUTURES
    bool         isOpen();            // method to calc if leg quantity is not zero
};


class Transaction
{
public:
    std::wstring  underlying  = L"";    // OPTIONS,STOCKS,FUTURES,CURRENCY,COMMODITIES
    std::wstring  description = L"";    // Iron Condor, Strangle, Roll, Expired, Closed, Exercised, etc
    std::wstring  transDate   = L"";    // YYYY-MM-DD
    int           quantity    = 0;
    double        price       = 0;
    double        multiplier  = 0;
    double        fees        = 0;
    double        total       = 0;

    std::vector<std::shared_ptr<Leg>> legs;            // pointer list for all legs in the transaction
};


class Trade
{
public:

    bool          isOpen       = true;    // False if all legs are closed
    std::wstring  tickerSymbol = L"";
    std::wstring  tickerName   = L"";
    std::wstring  futureExpiry = L"";     // YYYYMM of Futures contract expiry
    int           category     = 0;       //  blue, purple, etc (0 to 4)
    double        ACB          = 0;

    double  tickerLastPrice = 0;
    double  tickerClosePrice = 0;

    std::vector<std::shared_ptr<Transaction>> transactions;     // pointer list for all transactions in the trade
    std::vector<std::shared_ptr<Leg>> openLegs;                 // sorted list of open legs for this trade

    void setTradeOpenStatus();
    void createOpenLegsVector();
};

// pointer list for all trades (initially loaded from database)
extern std::vector<std::shared_ptr<Trade>> trades;


