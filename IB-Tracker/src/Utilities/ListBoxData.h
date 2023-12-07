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

constexpr int MAX_COLUMNS = 13;

typedef long TickerId;

class ColumnData {
public:
    std::wstring        text;
    StringAlignment     HAlignment = StringAlignmentNear;  
    StringAlignment     VAlignment = StringAlignmentCenter;
    DWORD               back_theme = COLOR_GRAYDARK;
    DWORD               text_theme = COLOR_WHITELIGHT;;
    REAL                font_size = 8;                    // 8, 10
    int                 font_style = FontStyleRegular;    // FontStyleRegular, FontStyleBold
    int                 column_width = 0;
};

enum class LineType {
    none,
    ticker_line,
    options_leg,
    shares,
    futures,
    transaction_header,
    category_header
};

class ListBoxData {
public:
    LineType        line_type = LineType::none;
    std::wstring    aggregate_shares;
    TickerId        tickerId = -1;
    std::shared_ptr<Trade> trade = nullptr;
    std::shared_ptr<Transaction> trans = nullptr;
    std::shared_ptr<Leg> leg = nullptr;
    ColumnData      col[MAX_COLUMNS];


    void SetData(
        int index, const std::shared_ptr<Trade> tradeptr, TickerId tickId,
        std::wstring text, StringAlignment HAlignment, StringAlignment VAlignment, 
        DWORD back_theme, DWORD text_theme, REAL font_size, int font_style)
    {
        if (tickId != -1) {
            line_type = LineType::ticker_line;
            tickerId = tickId;
        }
        trade = tradeptr;

        col[index].text = text;
        col[index].HAlignment = HAlignment;
        col[index].VAlignment = VAlignment;
        col[index].back_theme = back_theme;
        col[index].text_theme = text_theme;
        col[index].font_size = font_size;
        col[index].font_style = font_style;
    }

    // Update Text & color only. This is called from tws-client when TWS
    // sends new price data that needs to be updated.
    void SetTextData(int index, std::wstring text, DWORD textTheme)
    {
        col[index].text = text;
        col[index].text_theme = textTheme;
    }

};


enum class TableType
{
    active_trades,
    closed_trades,
    trade_history,
    ticker_totals,
    trade_templates,
    trade_management,
    trans_panel
};


bool ListBoxData_ResizeColumnWidths(HWND hListBox, TableType table_type, int index);
void ListBoxData_DestroyItemData(HWND hListBox);
void ListBoxData_RequestMarketData(HWND hListBox);
void ListBoxData_NoTradesExistMessage(HWND hListBox);
void ListBoxData_OpenPosition(HWND hListBox, const std::shared_ptr<Trade>& trade, TickerId tickerId);
void ListBoxData_TradeROI(HWND hListBox, const std::shared_ptr<Trade>& trade, TickerId tickerId);
void ListBoxData_HistoryHeader(
    HWND hListBox, const std::shared_ptr<Trade>& trade, const std::shared_ptr<Transaction>& trans);
void ListBoxData_HistoryOptionsLeg(
    HWND hListBox, const std::shared_ptr<Trade>& trade, const std::shared_ptr<Transaction>& trans, const std::shared_ptr<Leg>& leg);
void ListBoxData_HistorySharesLeg(
    HWND hListBox, const std::shared_ptr<Trade>& trade, const std::shared_ptr<Transaction>& trans, const std::shared_ptr<Leg>& leg);      
void ListBoxData_HistoryDividendLeg(
    HWND hListBox, const std::shared_ptr<Trade>& trade, const std::shared_ptr<Transaction>& trans, const std::shared_ptr<Leg>& leg);      
void ListBoxData_AddBlankLine(HWND hListBox);
void ListBoxData_AddCategoryHeader(HWND hListBox, const std::shared_ptr<Trade>& trade);
void ListBoxData_OutputClosedYearTotal(HWND hListBox, int year, double subtotal, int year_win, int year_loss);
void ListBoxData_OutputClosedMonthTotal(HWND hListBox, double monthly_total, int month_win, int month_loss);
void ListBoxData_OutputClosedWeekTotal(HWND hListBox, double weekly_total, int week_win, int week_loss);
void ListBoxData_OutputClosedDayTotal(HWND hListBox, double daily_total, int day_win, int day_loss);
void ListBoxData_OutputClosedMonthSubtotal(HWND hListBox, std::wstring closed_date, double subtotal, int month_win, int month_loss);
void ListBoxData_OutputClosedPosition(HWND hListBox, const std::shared_ptr<Trade>& trade, std::wstring closed_date);
void ListBoxData_OutputTickerTotals(HWND hListBox, std::wstring ticker, double amount);
void ListBoxData_OutputTransactionRunningTotal(HWND hListBox, double running_gross_total, 
    double running_fees_total, double running_net_total);
void ListBoxData_OutputTransactionDaySubtotal(HWND hListBox, std::wstring trans_date, 
    double sub_gross_amount, double sub_fees_amount, double sub_net_amount);
void ListBoxData_OutputTransaction(HWND hListBox, const std::shared_ptr<Trade>& trade, const std::shared_ptr<Transaction>& trans);
void ListBoxData_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT* lpDrawItem);
void Header_OnPaint(HWND hWnd);

