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

enum class SortOrder {
    Category,
    Expiration,
    TickerSymbol
};


class CActiveTrades : public CWindowBase<CActiveTrades>
{
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    SortOrder sort_order = SortOrder::Category;
};


extern CActiveTrades ActiveTrades;
extern HWND HWND_ACTIVETRADES;

constexpr int IDC_TRADES_LISTBOX = 100;
constexpr int IDC_TRADES_CUSTOMVSCROLLBAR = 102;
constexpr int IDC_TRADES_LBLSORTFILTER = 104;
constexpr int IDC_TRADES_SORTFILTER = 105;
constexpr int IDC_TRADES_LBLNEWTRADE = 106;
constexpr int IDC_TRADES_NEWTRADE = 107;
constexpr int IDC_TRADES_NETLIQUIDATION = 108;
constexpr int IDC_TRADES_NETLIQUIDATION_VALUE = 109;
constexpr int IDC_TRADES_EXCESSLIQUIDITY = 110;
constexpr int IDC_TRADES_EXCESSLIQUIDITY_VALUE = 111;

constexpr int ACTIVETRADES_LISTBOX_ROWHEIGHT = 24;
constexpr int ACTIVETRADES_MARGIN = 80;

constexpr int IDC_SORTFILTER_FIRST = 140;
constexpr int IDC_SORTFILTER_CATEGORY = 140;
constexpr int IDC_SORTFILTER_EXPIRATION = 141;
constexpr int IDC_SORTFILTER_TICKERSYMBOL = 142;
constexpr int IDC_SORTFILTER_LAST = 142;

constexpr int IDC_NEWTRADE_FIRST = 150;
constexpr int IDC_NEWTRADE_CUSTOMOPTIONS = 150;
constexpr int IDC_NEWTRADE_IRONCONDOR = 151;
constexpr int IDC_NEWTRADE_SHORTSTRANGLE = 152;
constexpr int IDC_NEWTRADE_SHORTPUT = 153;
constexpr int IDC_NEWTRADE_SHORTPUTVERTICAL = 154;
constexpr int IDC_NEWTRADE_SHORTCALL = 155;
constexpr int IDC_NEWTRADE_SHORTCALLVERTICAL = 156;
constexpr int IDC_NEWTRADE_SHORTPUTLT112 = 157;
constexpr int IDC_NEWTRADE_SHARESTRADE = 158;
constexpr int IDC_NEWTRADE_FUTURESTRADE = 159;
constexpr int IDC_NEWTRADE_OTHERINCOME = 160;
constexpr int IDC_NEWTRADE_LAST = 160;


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
constexpr int COLUMN_TICKER_PORTFOLIO_1 = 8; 
constexpr int COLUMN_TICKER_PORTFOLIO_2 = 9; 
constexpr int COLUMN_TICKER_PORTFOLIO_3 = 10;
constexpr int COLUMN_TICKER_PORTFOLIO_4 = 11;
constexpr int COLUMN_TICKER_PORTFOLIO_5 = 12;


bool IsNewOptionsTradeAction(TradeAction action);
bool IsNewSharesTradeAction(TradeAction action);
void ActiveTrades_ShowActiveTrades();
void ActiveTrades_OnSize(HWND hwnd, UINT state, int cx, int cy);
void ActiveTrades_UpdateTickerPrices();
void PerformITMcalculation(std::shared_ptr<Trade>& trade);
std::wstring GetSortFilterDescription(const int index);
std::wstring GetNewTradeDescription(const int index);