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
#include "Database/database.h"


class CTradeHistory : public CWindowBase<CTradeHistory>
{
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

};

extern CTradeHistory TradeHistory;
extern HWND HWND_TRADEHISTORY;

constexpr int IDC_HISTORY_LISTBOX = 110;
constexpr int IDC_HISTORY_SYMBOL = 111;
constexpr int IDC_HISTORY_CUSTOMVSCROLLBAR = 112;
constexpr int IDC_HISTORY_SEPARATOR = 113;
constexpr int IDC_HISTORY_LISTBOX_TRANSDETAILS = 117;
constexpr int IDC_HISTORY_LBLNOTES = 118;
constexpr int IDC_HISTORY_TXTNOTES = 119;

constexpr int HISTORY_LISTBOX_ROWHEIGHT = 20;
constexpr int TICKER_TOTALS_LISTBOX_ROWHEIGHT = 16;
constexpr int TRADEHISTORY_MARGIN = 24;

void TradeHistory_ShowTradesHistoryTable(std::shared_ptr<Trade>& trade);

