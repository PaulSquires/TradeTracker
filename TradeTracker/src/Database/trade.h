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

#pragma once

//  The hierarchy of the data is:
//      Trade
//          |-> Transaction
//                        |-> Legs

//  All open and closed Trades are stored in the global Trades vector. We differentiate
//  between them by iterating the vector and testing the isOpen() boolean method.
//

#include "Utilities/Colors.h"


typedef long TickerId;

class Transaction;   // forward declare

enum class PutCall {
    Put,
    Call,
    Count,     // used in StrategyButton when toggling state
    Nothing     // always put last for historical reasons saving int to database
};

enum class Underlying {
    Options,
    Shares,
    Futures,
    Dividend,
    Other,
    Nothing     // always put last for historical reasons saving int to database
};

enum class Action {
    STO,
    BTO,
    STC,
    BTC,
    Nothing     // always put last for historical reasons saving int to database
};


class Leg {
public:
    bool         isOpen();                 // method to calc if leg quantity is not zero
    int          contract_id = 0;          // Contract ID received from IBKR 
    int          leg_id = 0;               // Unique id for this leg within the Trade (see Trade nextleg_id) 
    int          leg_back_pointer_id = 0;  // If transaction is CLOSE, EXPIRE, ROLL this points back to leg where quantity modified
    int          original_quantity = 0;
    int          open_quantity = 0;
    std::wstring expiry_date   = L"";
    std::wstring strike_price  = L"";
    PutCall      put_call      = PutCall::Nothing;
    Action       action        = Action::Nothing;      // STO,BTO,STC,BTC
    Underlying   underlying    = Underlying::Nothing;      // OPTIONS, STOCKS, FUTURES, DIVIDEND, OTHER
    std::shared_ptr<Transaction> trans = nullptr;   // back pointer to transaction that this leg belongs to

    double calculated_leg_cost = 0;        // refer to CalculateLegCosting(). Alternative for position_cost.

    double position_cost = 0;              // ACB as calculated CalculateLegCosting()
    double market_value = 0;               // real time data receive via updatePortfolio
    double percentage = 0;                 // real time data receive via updatePortfolio
    double unrealized_pnl = 0;             // real time data receive via updatePortfolio

    double position_cost_tws = 0;          // position cost received from incoming IB/TWS data
    double position_cost_ratio = 0;        // position cost ratio to overall trade acb
    int    listbox_index = -1;             // index of the ownerdraw listbox
};


class Transaction {
public:
    Underlying    underlying  = Underlying::Nothing;      // OPTIONS,STOCKS,FUTURES,DIVIDEND
    std::wstring  description = L"";      // Iron Condor, Strangle, Roll, Expired, Closed, Exercised, etc
    std::wstring  trans_date  = L"";      // YYYY-MM-DD
    int           quantity    = 0;
    double        price       = 0;
    double        multiplier  = 0;
    double        fees        = 0;
    double        total       = 0;

    std::vector<std::shared_ptr<Leg>> legs;            // pointer list for all legs in the transaction
};


class Trade {
public:
    bool          is_open       = true;    // false if all legs are closed
    bool          ticker_data_requested = false;   // ticker data already requested
    TickerId      ticker_id     = -1;
    std::wstring  ticker_symbol = L"";
    std::wstring  ticker_name   = L"";
    std::wstring  future_expiry = L"";     // YYYYMM of Futures contract expiry
    std::wstring  notes         = L"";     
    int           category      = 0;       // Category number
    int           nextleg_id    = 0;       // Incrementing counter that gets unique ID for legs being generated in TransDetail.    

    int           aggregate_shares = 0;    // Calculated from all transactions roll-up
    int           aggregate_futures = 0;   // Calculated from all transactions roll-up
    double        acb           = 0;       // adjusted cost base of entire trade
    double        shares_acb    = 0;       // adjusted cost base of shares/futures (may include/exclude costs like dividends)
    double        trade_bp      = 0;       // Buying Power for the entire trade 
    double        multiplier    = 0;       // Retrieved from Transaction and needed for updatePortfolio real time calculations

    double        ticker_last_price  = 0;
    double        ticker_close_price = 0;
    int           ticker_decimals    = 2;  // upated via data from Config. 

    // The earliest DTE from the Legs of the Trade are calculated in the SetTradeOpenStatus() function
    // and is used when displaying ActiveTrades with the ExpiryDate filter set.
    int earliest_legs_DTE = 9999999;

    // The following are string representations of the marketdata and updatePortfolio values. We save them here
    // so that the Active Trades lists gets visually updated immediately after a new Trade or close trade.
    // The ticker data would not be updated until a new price occurs so we simply display the most recent price
    // rather than blank text.
    std::wstring itm_text = L"";
    DWORD itm_color = COLOR_WHITELIGHT;

    std::wstring ticker_column_1;
    std::wstring ticker_column_2;
    std::wstring ticker_column_3;
    
    DWORD ticker_column_1_clr{};
    DWORD ticker_column_2_clr{};
    DWORD ticker_column_3_clr{};

    // Dates used to calculate ROI on TradeBP.
    std::wstring  bp_start_date = L"99999999";            // YYYYMMDD  First transaction date
    std::wstring  bp_end_date = L"00000000";              // YYYYMMDD  Last trans expiry date or trade close date if earlier) 
    std::wstring  oldest_trade_trans_date = L"00000000";  // If Trade is closed then this trans will be the BPendDate

    std::vector<std::shared_ptr<Transaction>> transactions;     // pointer list for all transactions in the trade
    std::vector<std::shared_ptr<Leg>> open_legs;                 // sorted list of open legs for this trade

    void SetTradeOpenStatus();
    void CalculateAdjustedCostBase();
    void CreateOpenLegsVector();
    void CalculateLegCosting();
};


// pointer list for all trades (initially loaded from database)
extern std::vector<std::shared_ptr<Trade>> trades;


