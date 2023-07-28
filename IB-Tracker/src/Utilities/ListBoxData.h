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


#include "Database/trade.h"
#include "Utilities/Colors.h"

constexpr std::wstring GLYPH_TREEOPEN = L"\u23F7";
constexpr std::wstring GLYPH_TREECLOSED = L"\u23F5";
constexpr std::wstring GLYPH_CIRCLE = L"\u23FA";
constexpr std::wstring GLYPH_DROPDOWN = L"\uE015";
constexpr std::wstring GLYPH_MAGNIFYGLASS = L"\uE0BD";
constexpr std::wstring GLYPH_RESETLINE = L"\u2B8C";
constexpr std::wstring GLYPH_CHECKMARK = L"\u2713";
constexpr std::wstring GLYPH_SETUP = L"\uE292";

// ListBox data structure that will be directly accessed for each row 
// during the WM_DRAWITEM notification. 

typedef long TickerId;

class ColumnData {
public:
    std::wstring        wszText;
    StringAlignment     HAlignment = StringAlignmentNear;  
    StringAlignment     VAlignment = StringAlignmentCenter;
    DWORD               backTheme = COLOR_GRAYDARK;
    DWORD               textTheme = COLOR_WHITELIGHT;;
    REAL                fontSize = 8;                    // 8, 10
    int                 fontStyle = FontStyleRegular;    // FontStyleRegular, FontStyleBold
    int                 colWidth = 0;
};

enum class LineType {
    None,
    TickerLine,
    OptionsLeg,
    Shares,
    Futures,
    TransactionHeader,
    CategoryHeader
};

class ListBoxData {
public:
    LineType        lineType = LineType::None;
    bool            isDailyTotalsNode = false;
    bool            isDailyTotalsNodeOpen = false;
    std::wstring    DailyTotalsDate;
    std::wstring    AggregateShares;
    TickerId        tickerId = -1;
    std::shared_ptr<Trade> trade = nullptr;
    std::shared_ptr<Transaction> trans = nullptr;
    std::shared_ptr<Leg> leg = nullptr;
    ColumnData      col[10];


    void SetData(
        int index, std::shared_ptr<Trade> tradeptr, TickerId tickId,
        std::wstring wszText, StringAlignment HAlignment, StringAlignment VAlignment, 
        DWORD backTheme, DWORD textTheme, REAL fontSize, int fontStyle)
    {
        if (tickId != -1) lineType = LineType::TickerLine;
        tickerId = tickId;
        trade = tradeptr;
        col[index].wszText = wszText;
        col[index].HAlignment = HAlignment;
        col[index].VAlignment = VAlignment;
        col[index].backTheme = backTheme;
        col[index].textTheme = textTheme;
        col[index].fontSize = fontSize;
        col[index].fontStyle = fontStyle;
    }

    // Update Text & color only. This is called from tws-client when TWS
    // sends new price data that needs to be updated.
    void SetTextData(int index, std::wstring wszText, DWORD textTheme)
    {
        col[index].wszText = wszText;
        col[index].textTheme = textTheme;
    }

};


enum class TableType
{
    ActiveTrades,
    ClosedTrades,
    TradeHistory,
    TickerTotals,
    TradeTemplates,
    TradeManagement,
    TransPanel
};

DWORD GetCategoryColor(int category);
void ListBoxData_ResizeColumnWidths(HWND hListBox, TableType tabletype, int nIndex);
void ListBoxData_DestroyItemData(HWND hListBox);
void ListBoxData_RequestMarketData(HWND hListBox);
void ListBoxData_OpenPosition(HWND hListBox, const std::shared_ptr<Trade>& trade, TickerId tickerId);
void ListBoxData_TradeROI(HWND hListBox, const std::shared_ptr<Trade>& trade, TickerId tickerId);
void ListBoxData_HistoryHeader(
    HWND hListBox, const std::shared_ptr<Trade>& trade, const std::shared_ptr<Transaction>& trans);
void ListBoxData_HistoryOptionsLeg(
    HWND hListBox, const std::shared_ptr<Trade>& trade, const std::shared_ptr<Transaction>& trans, const std::shared_ptr<Leg>& leg);
void ListBoxData_HistorySharesLeg(
    HWND hListBox, const std::shared_ptr<Trade>& trade, const std::shared_ptr<Transaction>& trans, const std::shared_ptr<Leg>& leg);
void ListBoxData_AddBlankLine(HWND hListBox);
void ListBoxData_AddCategoryHeader(HWND hListBox, const std::shared_ptr<Trade>& trade);
void ListBoxData_OutputClosedYearTotal(HWND hListBox, int year, double subtotal);
void ListBoxData_OutputClosedMonthSubtotal(HWND hListBox, std::wstring closedDate, double subtotal);
void ListBoxData_OutputClosedPosition(HWND hListBox, const std::shared_ptr<Trade>& trade, std::wstring closedDate);
void ListBoxData_OutputTickerTotals(HWND hListBox, std::wstring ticker, double amount);
void ListBoxData_OutputDailyTotalsNodeHeader(HWND hListBox, std::wstring date, double amount, bool isOpen);
void ListBoxData_OutputDailyTotalsDetailLine(HWND hListBox, const std::shared_ptr<Trade>& trade, const std::shared_ptr<Transaction>& trans);
void ListBoxData_OutputDailyTotalsSummary(HWND hListBox, double grandTotal, double MTD, double YTD);
void ListBoxData_OutputTransaction(HWND hListBox, const std::shared_ptr<Trade>& trade, const std::shared_ptr<Transaction>& trans);
void ListBoxData_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT* lpDrawItem);
void Header_OnPaint(HWND hWnd);

