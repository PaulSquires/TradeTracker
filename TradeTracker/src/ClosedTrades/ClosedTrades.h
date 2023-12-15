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


class CClosedTrades : public CWindowBase<CClosedTrades>
{
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

};

extern CClosedTrades ClosedTrades;
extern HWND HWND_CLOSEDTRADES;

constexpr int IDC_CLOSED_LISTBOX = 100;
constexpr int IDC_CLOSED_CUSTOMVSCROLLBAR = 102;
constexpr int IDC_CLOSED_HEADER = 103;
constexpr int IDC_CLOSED_LBLCATEGORYFILTER = 104;
constexpr int IDC_CLOSED_CATEGORY = 105;
constexpr int IDC_CLOSED_CMDYEAREND = 106;
constexpr int IDC_CLOSED_LBLTICKERFILTER = 107;
constexpr int IDC_CLOSED_TXTTICKER = 108;
constexpr int IDC_CLOSED_CMDTICKERGO = 109;

constexpr int CLOSED_TRADES_LISTBOX_ROWHEIGHT = 18;
constexpr int CLOSEDTRADES_MARGIN = 80;

bool IsNewOptionsTradeAction(TradeAction action);
bool IsNewSharesTradeAction(TradeAction action);
void ClosedTrades_ShowClosedTrades();
void ClosedTrades_SetShowTradeDetail(bool enable);

