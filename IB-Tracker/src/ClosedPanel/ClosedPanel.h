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

#include "..\Utilities\CWindowBase.h"


class CClosedPanel : public CWindowBase<CClosedPanel>
{
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

};


const int IDC_CLOSED_LISTBOX = 100;
const int IDC_CLOSED_LABEL = 101;
const int IDC_CLOSED_CUSTOMVSCROLLBAR = 102;
const int IDC_CLOSED_HEADER = 103;

const int CLOSED_TRADES_LISTBOX_ROWHEIGHT = 18;
const int CLOSEDPANEL_MARGIN = 24;

bool IsNewOptionsTradeAction(TradeAction action);
bool IsNewSharesTradeAction(TradeAction action);

