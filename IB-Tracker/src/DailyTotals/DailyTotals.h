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


class CDailyTotals : public CWindowBase<CDailyTotals>
{
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

};


constexpr int IDC_DAILY_LISTBOX = 110;
constexpr int IDC_DAILY_SYMBOL = 111;
constexpr int IDC_DAILY_CUSTOMVSCROLLBAR = 112;
constexpr int IDC_DAILY_LISTBOX_SUMMARY = 113;
constexpr int IDC_DAILY_HEADER_TICKERTOTALS = 114;
constexpr int IDC_DAILY_HEADER_SUMMARY = 115;
constexpr int IDC_DAILY_HEADER_TOTALS = 116;
constexpr int IDC_DAILY_LISTBOX_TransDetailS = 117;

constexpr int DAILY_LISTBOX_ROWHEIGHT = 20;
constexpr int DAILYTOTALS_WIDTH = 400;
constexpr int DAILYTOTALS_MARGIN = 24;

