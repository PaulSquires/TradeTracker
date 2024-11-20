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


#ifndef LISTBOXDATA_H
#define LISTBOXDATA_H

#include <string>
#include "appstate.h"
#include "import_dialog.h"
#include "icons_material_design.h"


const std::string GLYPH_TREEOPEN = ICON_MD_PLAY_ARROW;
const std::string GLYPH_CIRCLE = ICON_MD_CIRCLE_SMALL;

// Column where the Edit icon will show for TradeHistory line when mouse is over the line.
constexpr int COLUMN_EDIT_ICON = 8;

// These columns in the table are updated in real time when connected
// to TWS. The LineData pointer is updated via a call to SetColumnData.
// Refer to TwsClient::tickPrice in the tws-client.cpp file to see this in action.
constexpr int COLUMN_TICKER_ITM             = 2;    // ITM (In the Money)
constexpr int COLUMN_TICKER_CHANGE          = 5;    // price change
constexpr int COLUMN_TICKER_CURRENTPRICE    = 6;    // current price
constexpr int COLUMN_TICKER_PERCENTCHANGE   = 7;    // price percentage change

// These columns in the table are updated in real time when connected
// to TWS. The LineData pointer is updated via a call to SetColumnData.
// Refer to TwsClient::updatePortfolio in the tws-client.cpp file to see this in action.
constexpr int COLUMN_OPTIONLEG_DELTA    = 8; 
constexpr int COLUMN_TICKER_PORTFOLIO_1 = 9; 
constexpr int COLUMN_TICKER_PORTFOLIO_2 = 10; 
constexpr int COLUMN_TICKER_PORTFOLIO_3 = 11;
constexpr int COLUMN_TICKER_PORTFOLIO_4 = 12;
constexpr int COLUMN_TICKER_PORTFOLIO_5 = 13;

const int MAX_COLUMNS = 14;

typedef long TickerId;

enum class StringAlignment {
    left,
    center,
    right
};

enum class LineType {
    none,
    ticker_line,
    history_roi,
    history_selectable,
    nonselectable,
    options_leg,
    shares,
    futures,
    transaction_header,
    category_header
};


class CColumnData {
public:
    std::string      text;
    StringAlignment  Alignment    = StringAlignment::left;  
    ImU32            back_color   = 0;
    ImU32            text_color   = 0;
    int              font_size    = 8; 
    bool             is_bold      = false;
    int              column_width = 0;
};

class CListPanelData {
public:
    LineType         line_type = LineType::none;
    std::string      aggregate_shares;
    TickerId         ticker_id = -1;
    std::shared_ptr<Trade> trade = nullptr;
    std::shared_ptr<Transaction> trans = nullptr;
    std::shared_ptr<Leg> leg = nullptr;
    CColumnData      col[MAX_COLUMNS];
    bool             is_selected = false;
    ImportStruct*    ibkr_pointer = nullptr;

    void SetData(
        int index, const std::shared_ptr<Trade>& tradeptr, TickerId tickerid,
        const std::string& text, StringAlignment Alignment, 
        ImU32 back_color, ImU32 text_color, int font_size, bool is_bold)
    {
        if (tickerid != -1) ticker_id = tickerid;

        trade = tradeptr;

        if (trade && line_type == LineType::ticker_line) trade->ticker_id = ticker_id;
        if (leg && line_type == LineType::options_leg) leg->ticker_id = ticker_id;

        col[index].text = text;
        col[index].Alignment = Alignment;
        col[index].back_color = back_color;
        col[index].text_color = text_color;
        col[index].font_size = font_size;
        col[index].is_bold = is_bold;
    }

    void SetImportData(
        int index, ImportStruct* ibkrptr,
        const std::string& text, StringAlignment Alignment, 
        ImU32 back_color, ImU32 text_color, int font_size)
    {
        ibkr_pointer = ibkrptr;

        col[index].text = text;
        col[index].Alignment = Alignment;
        col[index].back_color = back_color;
        col[index].text_color = text_color;
        col[index].font_size = font_size;
        col[index].is_bold = false;
    }

    // Update Text & color only. This is called from tws-client when TWS
    // sends new price data that needs to be updated.
    void SetTextData(int index, const std::string& text, ImU32 text_color) {
        col[index].text = text;
        col[index].text_color = text_color;
    }
};


static int nHistoryMinColWidth[MAX_COLUMNS] =
{
    15,     /* dropdown arrow */
    62,     /* Description */
   -50,     /* position quantity */
   -50,     /* expiry date */
   -40,     /* DTE */
   -45,     /* strike price */
   -30,     /* put/call */
   -50,     /* ACB, BTC/STO, etc */
    15,     /* view transaction icon */
    0,
    0,
    0,
    0
};

static int nImportMinColWidth[MAX_COLUMNS] =
{
    40,     /* ticker symbol */
    40,     /* type */
    60,     /* date */
    40,     /* position quantity */
    60,     /* strike price */
    40,     /* put/call */
    40,     /* ID */
    0, 
    0, 
    0, 
    0, 
    0,  
    0,
    0    
};

static int nTradesMinColWidth[MAX_COLUMNS] =
{
    25,     /* dropdown arrow */
    50,     /* ticker symbol */
    50,     /* ITM */
    50,     /* position quantity */
    50,     /* expiry date */
    50,     /* DTE */
    60,     /* strike price / current price */
    50,     /* put/call */
    40,     /* option leg delta */
    75,     /* Adjusted Cost Basis*/
    60,     /* Market Value    COLUMN_TICKER_PORTFOLIO_1 = 10 */
    60,     /* Unrealized PNL  COLUMN_TICKER_PORTFOLIO_2 = 11 */
    50,     /* Percentages     COLUMN_TICKER_PORTFOLIO_3 = 12 */
    50      /* Percentages     COLUMN_TICKER_PORTFOLIO_4 = 13 */
};

static int nClosedMinColWidth[MAX_COLUMNS] =
{
    15,     /* empty */
    65,     /* Close Date */
    55,     /* Ticker Symbol */
    200,    /* Ticker Name */
    100,    /* Amount */
    5,      /* spacer */
    250,    /* Category Description */
    0,
    0,
    0,
    0,
    0,
    0,
    0
};

static int nTransMinColWidth[MAX_COLUMNS] =
{
    15,     /* empty */
    65,     /* Transaction Date */
    55,     /* Ticker Symbol */
    200,    /* Transaction Description */
    60,     /* Quantity */
    60,     /* Price */
    60,     /* Fees */
    70,     /* Total */
    5,      /* spacer */
    250,    /* Category Description */
    0,
    0,
    0,
    0
};


void ListBoxData_NoTradesExistMessage(AppState& state, std::vector<CListPanelData>& vec);
void ListPanelData_AddMenuItem(AppState& state, std::vector<CListPanelData>& vec, const std::string& description, int user_data, bool is_selected);
void ListPanelData_AddCategoryHeader(AppState& state, std::vector<CListPanelData>& vec, const std::shared_ptr<Trade>& trade, int num_trades_category);
void ListPanelData_OpenPosition(AppState& state, std::vector<CListPanelData>& vec, const std::shared_ptr<Trade>& trade, const bool is_history);
void ListPanelData_AddBlankLine(AppState& state, std::vector<CListPanelData>& vec);
void ListPanelData_OutputTickerTotals(AppState& state, std::vector<CListPanelData>& vec, const std::string& ticker, double amount);
void ListPanelData_OutputTickerTotalsHeader(AppState& state, std::vector<CListPanelData>& vec);
void ListPanelData_TradeROI(AppState& state, std::vector<CListPanelData>& vec, const std::shared_ptr<Trade>& trade, TickerId ticker_id);
void ListPanelData_HistoryHeader(AppState& state, std::vector<CListPanelData>& vec, const std::shared_ptr<Trade>& trade, 
    const std::shared_ptr<Transaction>& trans, const std::shared_ptr<Transaction>& trans_orig);
void ListPanelData_HistorySharesLeg(AppState& state, std::vector<CListPanelData>& vec, const std::shared_ptr<Trade>& trade, 
    const std::shared_ptr<Transaction>& trans, const std::shared_ptr<Leg>& leg);
void ListPanelData_HistoryDividendLeg(AppState& state, std::vector<CListPanelData>& vec, const std::shared_ptr<Trade>& trade, 
    const std::shared_ptr<Transaction>& trans, const std::shared_ptr<Leg>& leg);
void ListPanelData_OtherIncomeLeg(AppState& state, std::vector<CListPanelData>& vec, const std::shared_ptr<Trade>& trade, 
    const std::shared_ptr<Transaction>& trans, const std::shared_ptr<Leg>& leg);
void ListPanelData_HistoryOptionsLeg(AppState& state, std::vector<CListPanelData>& vec, const std::shared_ptr<Trade>& trade, 
    const std::shared_ptr<Transaction>& trans, const std::shared_ptr<Leg>& leg);
void ListPanelData_OutputClosedYearTotal(AppState& state, std::vector<CListPanelData>& vec, int year, double subtotal, int year_win, int year_loss);
void ListPanelData_OutputClosedMonthTotal(AppState& state, std::vector<CListPanelData>& vec, double monthly_total, int month_win, int month_loss);
void ListPanelData_OutputClosedWeekTotal(AppState& state, std::vector<CListPanelData>& vec, double weekly_total, int week_win, int week_loss);
void ListPanelData_OutputClosedDayTotal(AppState& state, std::vector<CListPanelData>& vec, double daily_total, int day_win, int day_loss);
void ListPanelData_OutputClosedMonthSubtotal(
    AppState& state, std::vector<CListPanelData>& vec, const std::string& closed_date, double subtotal, int month_win, int month_loss);
void ListPanelData_OutputClosedPosition(AppState& state, std::vector<CListPanelData>& vec, const std::shared_ptr<Trade>& trade, 
    const std::string& closed_date, const std::string& ticker_symbol, const std::string& description, double closed_amount);
void ListPanelData_OutputTransactionRunningTotal(AppState& state, std::vector<CListPanelData>& vec, 
    double running_gross_total, double running_fees_total, double running_net_total);
void ListPanelData_OutputTransaction(AppState& state, std::vector<CListPanelData>& vec, const std::shared_ptr<Trade>& trade, const std::shared_ptr<Transaction>& trans);


#endif //LISTBOXDATA_H

