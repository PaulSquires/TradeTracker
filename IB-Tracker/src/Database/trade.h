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

#pragma once

//  The hierarchy of the data is:
//      Trade
//          |-> Transaction
//                        |-> Legs

//  All open and closed Trades are stored in the global Trades vector. We differentiate
//  between them by iterating the vector and testing the isOpen() boolean method.
//

#include "Utilities/Colors.h"


class Transaction;   // forward declare


class Leg
{
public:
    int          contractId = 0;          // Contract ID received from IBKR 
    int          legID = 0;               // Unique id for this leg within the Trade (see Trade nextLegID) 
    int          legBackPointerID = 0;    // If transaction is CLOSE, EXPIRE, ROLL this points back to leg where quantity modified
    int          origQuantity = 0;
    int          openQuantity = 0;
    std::wstring expiryDate   = L"";
    std::wstring strikePrice  = L"";
    std::wstring PutCall      = L"";
    std::wstring action       = L"";      // STO,BTO,STC,BTC
    std::wstring underlying   = L"";      // OPTIONS, STOCKS, FUTURES
    bool         isOpen();                // method to calc if leg quantity is not zero
    std::shared_ptr<Transaction> trans = nullptr;   // back pointer to transaction that this leg belongs to
    double averagePrice = 0;              // real time data receive via updatePortfolio
    double marketPrice = 0;               // real time data receive via updatePortfolio
    double percentage = 0;                // real time data receive via updatePortfolio
    double unrealizedPNL = 0;             // real time data receive via updatePortfolio
    // The following are string representations of the updatePortfolio values. We save them here
    // so that the Activetrades lists gets visually updated immediately after a new Trade or close trade.
    // There is a delay from the time portfolio values are cancelled and when the new request data arrives
    // therefore the user will always see the most recent data until the new data arrives.
    std::wstring wszAveragePrice = L"";
    std::wstring wszMarketPrice = L"";
    std::wstring wszPercentage = L"";
    std::wstring wszUnrealizedPNL = L"";
    DWORD clrUnrealizedPNL = COLOR_WHITEDARK;
};


class Transaction
{
public:
    std::wstring  underlying  = L"";      // OPTIONS,STOCKS,FUTURES,CURRENCY,COMMODITIES
    std::wstring  description = L"";      // Iron Condor, Strangle, Roll, Expired, Closed, Exercised, etc
    std::wstring  transDate   = L"";      // YYYY-MM-DD
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
    std::wstring  notes        = L"";     
    int           category     = 0;       // blue, purple, etc (0 to 4)
    double        ACB          = 0;
    double        TradeBP      = 0;       // Buying Power for the entire trade 
    int           nextLegID    = 0;       // Incrementing counter that gets unique ID for legs being generated in TransDetail.    
    double        multiplier   = 0;       // Retrieved from Transaction and needed for updatePortfolio real time calculations

    double  tickerLastPrice = 0;
    double  tickerClosePrice = 0;
    int     tickerDecimals = 2;           // upated via data from Config. 

    // The following are string representations of the marketdata and updatePortfolio values. We save them here
    // so that the Activetrades lists gets visually updated immediately after a new Trade or close trade.
    // There is a delay from the time portfolio values are cancelled and when the new request data arrives
    // therefore the user will always see the most recent data until the new data arrives.
    std::wstring wszITM = L"";
    std::wstring wszTickerChange = L"";
    std::wstring wszTickerLastPrice = L"0.00";
    std::wstring wszTickerPercentChange = L"";
    std::wstring wszPercentage = L"";
    std::wstring wszUnrealizedPNL = L"";
    DWORD clrITM = COLOR_WHITELIGHT;
    DWORD clrTickerChange = COLOR_WHITELIGHT;
    DWORD clrTickerPercentChange = COLOR_WHITELIGHT;
    DWORD clrUnrealizedPNL = COLOR_WHITEDARK;


    // Dates used to calculate ROI on TradeBP.
    std::wstring  BPstartDate = L"99999999";           // YYYYMMDD  First transaction date
    std::wstring  BPendDate = L"00000000";             // YYYYMMDD  Last trans expiry date or trade close date if earlier) 
    std::wstring  OldestTradeTransDate = L"00000000";  // If Trade is closed then this trans will be the BPendDate


    std::vector<std::shared_ptr<Transaction>> Transactions;     // pointer list for all transactions in the trade
    std::vector<std::shared_ptr<Leg>> openLegs;                 // sorted list of open legs for this trade

    void setTradeOpenStatus();
    void createOpenLegsVector();
};


// pointer list for all trades (initially loaded from database)
extern std::vector<std::shared_ptr<Trade>> trades;


