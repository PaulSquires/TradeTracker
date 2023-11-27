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

#include "Utilities/CWindowBase.h"
#include "Utilities/UserMessages.h"
#include "Database/trade.h"

typedef long TickerId;


class CActiveTrades : public CWindowBase<CActiveTrades>
{
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

};


extern CActiveTrades ActiveTrades;
extern HWND HWND_ACTIVETRADES;

constexpr int IDC_TRADES_LISTBOX = 100;
constexpr int IDC_TRADES_LABEL = 101;
constexpr int IDC_TRADES_CUSTOMVSCROLLBAR = 102;
constexpr int IDC_TRADES_HEADER = 103;
constexpr int IDC_TRADES_NETLIQUIDATION = 104;
constexpr int IDC_TRADES_NETLIQUIDATION_VALUE = 105;
constexpr int IDC_TRADES_EXCESSLIQUIDITY = 106;
constexpr int IDC_TRADES_EXCESSLIQUIDITY_VALUE = 107;

constexpr int ACTIVETRADES_LISTBOX_ROWHEIGHT = 24;
constexpr int ACTIVETRADES_MARGIN = 24;


// These columns in the table are updated in real time when connected
// to TWS. The LineData pointer is updated via a call to SetColumnData
// and the correct ListBox line is invalidated/redrawn in order to force
// display of the new price data. Refer to TwsClient::tickPrice in the
// tws-client.cpp file to see this in action.
constexpr int COLUMN_TICKER_ITM             = 2;    // ITM (In the Money)
constexpr int COLUMN_TICKER_CHANGE          = 5;    // price change
constexpr int COLUMN_TICKER_CURRENTPRICE    = 6;    // current price
constexpr int COLUMN_TICKER_PERCENTCHANGE   = 7;    // price percentage change

// These columns in the table are updated in real time when connected
// to TWS. The LineData pointer is updated via a call to SetColumnData
// and the correct ListBox line is invalidated/redrawn in order to force
// display of the new price data. Refer to TwsClient::updatePortfolio in the
// tws-client.cpp file to see this in action.
constexpr int COLUMN_TICKER_COST            = 8;    // Book Value and average Price
constexpr int COLUMN_TICKER_MARKETVALUE     = 9;    // Market Value
constexpr int COLUMN_TICKER_UPNL            = 10;   // Unrealized profit or loss
constexpr int COLUMN_TICKER_PERCENTCOMPLETE = 11;   // Percentage values for the previous two columns data


bool IsNewOptionsTradeAction(TradeAction action);
bool IsNewSharesTradeAction(TradeAction action);
void ActiveTrades_ShowActiveTrades(const bool bForceReload);
void ActiveTrades_OnSize(HWND hwnd, UINT state, int cx, int cy);
int ActiveTrades_ShowHideLiquidityLabels(HWND hwnd);
void ActiveTrades_UpdateTickerPrices();
void PerformITMcalculation(std::shared_ptr<Trade>& trade);
