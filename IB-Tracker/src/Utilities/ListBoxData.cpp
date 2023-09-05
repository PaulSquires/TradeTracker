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

#include "pch.h"
#include "ListBoxData.h"

#include "MainWindow/tws-client.h"
#include "ActiveTrades/ActiveTrades.h"
#include "ClosedTrades/ClosedTrades.h"
#include "TradeHistory/TradeHistory.h"
#include "TradeDialog/TradeDialog.h"
#include "TickerTotals/TickerTotals.h"
#include "Transactions/TransPanel.h"
#include "SideMenu/SideMenu.h"
#include "Config/Config.h"
#include "Utilities/Colors.h"




int nHistoryMinColWidth[MAX_COLUMNS] =
{
    15,     /* dropdown arrow */
    60,     /* Description */
    50,     /* position quantity */
    50,     /* expiry date */
    40,     /* DTE */
    45,     /* strike price */
    30,     /* put/call */
    40,     /* ACB, BTC/STO, etc */
    15,     /* view transaction icon */
    0,
    0,
    0
};

// We need a maximum column size for the History table because the
// user may end a very long description and we don't want the column
// to expand to fit this whole trade description. We will still
// display the description but it will wrap in the display rectangle.
int nHistoryMaxColWidth[MAX_COLUMNS] =
{
    15,      /* dropdown arrow */
    100,     /* Description */
    100,     /* position quantity */
    100,     /* expiry date */
    100,     /* DTE */
    100,     /* strike price */
    100,     /* put/call */
    100,     /* ACB, BTC/STO, etc */
    15,      /* view transaction icon */
    0,
    0,
    0
};

int nTradesMinColWidth[MAX_COLUMNS] =
{
    25,     /* dropdown arrow */
    50,     /* ticker symbol */
    50,     /* ITM */
    50,     /* position quantity */  
    50,     /* expiry date */
    50,     /* DTE */
    60,     /* strike price / current price */
    45,     /* put/call */
    50,     /* AvgPX and Cost Basis*/
    50,     /* Market Value and Last Price */
    50,     /* Percentages */
    50      /* Unrealized PNL */
};

int nClosedMinColWidth[MAX_COLUMNS] =
{
    15,     /* empty */
    65,     /* Close Date */
    50,     /* Ticker Symbol */
    200,    /* Ticker Name */
    100,    /* Amount */
    5,      /* spacer */
    150,    /* Category Description */
    0,     
    0,     
    0,     
    0,
    0
};

int nTransMinColWidth[MAX_COLUMNS] =
{
    15,     /* empty */
    65,     /* Transaction Date */
    45,     /* Ticker Symbol */
    90,     /* Transaction Description */
    60,     /* Quantity */
    60,     /* Price */
    60,     /* Fees */
    70,     /* Total */
    0,
    0,
    0,
    0
};

int nTickerTotalsMinColWidth[MAX_COLUMNS] =
{
    5,     /* empty */
    50,    /* Ticker Symbol */
    140,   /* Ticker Name */
    60,    /* Amount */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};

int nTickerTotalsMaxColWidth[MAX_COLUMNS] =
{
    5,        /* empty */
    50,       /* Ticker Symbol */
    250,      /* Ticker Name */
    120,      /* Amount */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};

int nTradeTemplatesMinColWidth[MAX_COLUMNS] =
{
    5,     /* spacer */
    80,    /* Description */
    0,     
    0,     
    0,     
    0,
    0,
    0,
    0,
    0,
    0,
    0
};


bool previous_market_data_loaded = false;


// ========================================================================================
// Helper function to get the Color for the specified Category
// ========================================================================================
DWORD GetCategoryColor(int category)
{
    DWORD cattext_color = COLOR_WHITEDARK;
    switch (category)
    {
    case 0: cattext_color = COLOR_WHITEDARK; break;
    case 1: cattext_color = COLOR_BLUE; break;
    case 2: cattext_color = COLOR_PINK; break;
    case 3: cattext_color = COLOR_GREEN; break;
    case 4: cattext_color = COLOR_ORANGE; break;
    case 5: cattext_color = COLOR_RED; break;
    case 6: cattext_color = COLOR_TEAL; break;
    case 7: cattext_color = COLOR_KHAKI; break;
    default: cattext_color = COLOR_WHITEDARK;
    }
    return cattext_color;
}


// ========================================================================================
// Calculate the actual column widths based on the size of the strings in
// ListBoxData while respecting the minimum values as defined in nMinColWidth[].
// This function is also called when receiving new price data from TWS because
// that data may need the column width to be wider.
// Returns bool to indicate whether ListBox should be redrawn.
// ========================================================================================
bool ListBoxData_ResizeColumnWidths(HWND hListBox, TableType tabletype, int index)
{
    HDC hdc = GetDC(hListBox);

    int nColWidth[MAX_COLUMNS] = { 0,0,0,0,0,0,0,0,0,0,0 };

    // Initialize the nColWidth array based on the incoming ListBox
    for (int i = 0; i < MAX_COLUMNS; i++) {
        switch (tabletype)
        {
        case TableType::ActiveTrades:
            nColWidth[i] = nTradesMinColWidth[i];
            break;

        case TableType::ClosedTrades:
            nColWidth[i] = nClosedMinColWidth[i];
            break;

        case TableType::TradeHistory:
            nColWidth[i] = nHistoryMinColWidth[i];
            break;

        case TableType::TickerTotals:
            nColWidth[i] = nTickerTotalsMinColWidth[i];
            break;

        case TableType::TradeTemplates:
            nColWidth[i] = nTradeTemplatesMinColWidth[i];
            break;

        case TableType::TransPanel:
            nColWidth[i] = nTransMinColWidth[i];
            break;
            
        }
    }


    Graphics graphics(hdc);
    graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

    std::wstring font_name = AfxGetDefaultFont();
    FontFamily fontFamily(font_name.c_str());
    REAL fontSize = 0;
    int fontStyle = FontStyleRegular;
    RectF boundRect;
    RectF layoutRect(0.0f, 0.0f, 1000.0f, 50.0f);
    StringFormat format;
    format.SetAlignment(StringAlignmentFar);

    bool bRedrawListBox = false;

    int nEnd = (int)ListBox_GetCount(hListBox);
    if (nEnd == 0) return false;
    int nStart = 0;

    // If a specific line number was passed into this function then we only
    // test for that line rather than all lines (like when the arrays are first loaded).
    // A value of -1 will iterate all strings the columns.
    if (index != -1) {
        nStart = index; nEnd = index;
    }

    for (int ii = nStart; ii < nEnd; ++ii) {
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, ii);
        if (ld == nullptr) continue;
        if (ld == (void*)-1) continue;
        if (ld->lineType != LineType::CategoryHeader) {
            for (int i = 0; i < MAX_COLUMNS; i++) {
                if (nColWidth[i] == 0) continue;
                fontSize = ld->col[i].fontSize;
                fontStyle = ld->col[i].fontStyle;
                Font font(&fontFamily, fontSize, fontStyle, Unit::UnitPoint);

                graphics.MeasureString(ld->col[i].text.c_str(), ld->col[i].text.length(),
                    &font, layoutRect, &format, &boundRect);

                int textLength = AfxUnScaleX(boundRect.Width) + 5;  // add a bit for padding

                
                if (textLength > nColWidth[i]) {
                
                    nColWidth[i] = textLength;

                    if (tabletype == TableType::TradeHistory) {
                        nColWidth[i] = min(nColWidth[i], nHistoryMaxColWidth[i]);
                    }

                    if (tabletype == TableType::TickerTotals) {
                        nColWidth[i] = min(nColWidth[i], nTickerTotalsMaxColWidth[i]);
                    }

                    bRedrawListBox = true;
                }
            }
        }
            
        if (index != -1) break;
    }


    // Update the newly calculated column widths into each of the ld structures
    for (int ii = nStart; ii < nEnd; ++ii) {
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, ii);
        if (ld == nullptr) continue;
        if (ld == (void*)-1) continue;
        for (int i = 0; i < MAX_COLUMNS; i++) {
            ld->col[i].colWidth = nColWidth[i];
        }
        ListBox_SetItemData(hListBox, ii, ld);
    }

    // Update the widths of any associated Header control. 
    HWND hHeader = NULL;
    if (tabletype == TableType::ClosedTrades) hHeader = GetDlgItem(HWND_CLOSEDTRADES, IDC_CLOSED_HEADER);
    if (tabletype == TableType::TickerTotals) hHeader = GetDlgItem(HWND_TICKERPANEL, IDC_TICKER_HEADER_TOTALS);
    if (tabletype == TableType::TransPanel) hHeader = GetDlgItem(HWND_TRANSPANEL, IDC_TRANS_HEADER);
    
    if (hHeader) {
        for (int i = 0; i < MAX_COLUMNS; i++) {
            Header_SetItemWidth(hHeader, i, AfxScaleX((float)nColWidth[i]));
        }
    }

    ReleaseDC(hListBox, hdc);
    
    return bRedrawListBox;
}


// ========================================================================================
// Destroy all manually allocated ListBox display data that is held in LineData structures.
// ========================================================================================
void ListBoxData_DestroyItemData(HWND hListBox)
{
    // Cancel any previous market data requests and delete any previously
    // allocated ListBoxData structures.
    int item_count = ListBox_GetCount(hListBox);

    for (int i = 0; i < item_count; i++) {
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, i);
        if (ld != nullptr) {
            if (ld->lineType == LineType::TickerLine && previous_market_data_loaded) {
                // Cancelling a market data symbol is not synchronous therefore we need
                // to use ever increasing tickerid numbers to avoid duplicate ticker id
                // errors when the listbox is reloaded.
                tws_CancelMarketData(ld->tickerId);
                previous_market_data_loaded = false;
            }
            ld = nullptr;
            delete(ld);
        }
    }

    // Clear the current trades listbox
    ListBox_ResetContent(hListBox);
    if (hListBox == GetDlgItem(HWND_ACTIVETRADES, IDC_TRADES_LISTBOX)) {
        previous_market_data_loaded = false;
    }
}


// ========================================================================================
// Loop through the ListBox data and request market data for tickers
// ========================================================================================
void ListBoxData_RequestMarketData(HWND hListBox)
{
    // Request market data for each open trade
    int item_count = ListBox_GetCount(hListBox);
    for (int i = 0; i < item_count; i++) {
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, i);
        if (ld != nullptr) {
            if (ld->lineType == LineType::TickerLine) {
                tws_RequestMarketData(ld);
            }
        }
    }

    if (tws_IsConnected()) {
        previous_market_data_loaded = true;
    }


    // Request portfolio updates. This also loads the IBKR and local vectors that 
    // are used for performing Reconciliations and also displaying position/leg real time data.
    if (tws_IsConnected())
        tws_RequestPortfolioUpdates();
}


// ========================================================================================
// Create the display data for a Category Header line
// ========================================================================================
void ListBoxData_AddCategoryHeader(HWND hListBox, const std::shared_ptr<Trade>& trade)
{
    ListBoxData* ld = new ListBoxData;

    std::wstring text = AfxUpper(GetCategoryDescription(trade->category));

    ld->SetData(0, nullptr, -1, text, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_BLUE, 9, FontStyleBold);

    ld->lineType = LineType::CategoryHeader;
    ListBox_AddString(hListBox, ld);
}


// ========================================================================================
// Create the display data for BP, Days,ROI (for a Trade History trade).
// ========================================================================================
void ListBoxData_TradeROI(HWND hListBox, const std::shared_ptr<Trade>& trade, TickerId tickerId)
{
    ListBoxData* ld = new ListBoxData;

    REAL font8 = 8;
    std::wstring text;

    std::wstring startDate = AfxInsertDateHyphens(trade->bp_start_date);
    std::wstring endDate = AfxInsertDateHyphens(trade->bp_end_date);

    // Buying Power
    text = AfxMoney(trade->trade_bp, true, 0);
    ld->SetData(2, trade, tickerId, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
        COLOR_WHITELIGHT, font8, FontStyleRegular);
    text = L"BP";
    ld->SetData(3, trade, tickerId, text, StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
        COLOR_WHITEDARK, font8, FontStyleRegular);
    
    // Days In Trade
    int daysInTrade = AfxDaysBetween(startDate, (trade->is_open ? AfxCurrentDate() : endDate));
    text = AfxMoney(daysInTrade, true, 0);
    ld->SetData(4, trade, tickerId, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
        COLOR_WHITELIGHT, font8, FontStyleRegular);
    text = L"DIT";
    ld->SetData(5, trade, tickerId, text, StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
        COLOR_WHITEDARK, font8, FontStyleRegular);
    ListBox_AddString(hListBox, ld);

    // Totals Days for Trade
    ld = new ListBoxData;
    int daysTotal = AfxDaysBetween(startDate, endDate);
    text = AfxMoney(daysTotal, true, 0);
    ld->SetData(2, trade, tickerId, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
        COLOR_WHITELIGHT, font8, FontStyleRegular);
    text = L"Days";
    ld->SetData(3, trade, tickerId, text, StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
        COLOR_WHITEDARK, font8, FontStyleRegular);
    
    // ROI% per 30 days  
    text = AfxMoney(0, true, 1) + L"%";
    if (trade->trade_bp != 0 && daysTotal != 0) {
        text = AfxMoney((trade->acb / trade->trade_bp * 100 / daysTotal * 30), true, 1) + L"%";
    }
    ld->SetData(4, trade, tickerId, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
        COLOR_WHITELIGHT, font8, FontStyleRegular);
    text = L"30d";
    ld->SetData(5, trade, tickerId, text, StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
        COLOR_WHITEDARK, font8, FontStyleRegular);
    ListBox_AddString(hListBox, ld);

    ListBoxData_AddBlankLine(hListBox);
}

    
// ========================================================================================
// Create the display data for an Open Position that displays in Trades & History tables.
// ========================================================================================
void ListBoxData_OpenPosition(HWND hListBox, const std::shared_ptr<Trade>& trade, TickerId tickerId)
{
    ListBoxData* ld = new ListBoxData;
    
    bool isHistory = GetDlgCtrlID(hListBox) == IDC_HISTORY_LISTBOX ? true : false;
    REAL font8 = 8;
    REAL font9 = 9;
    DWORD clr = COLOR_WHITELIGHT;
    std::wstring text; 

    if (isHistory) {
        tickerId = -1;

        ld->SetData(0, trade, tickerId, GLYPH_TREEOPEN, StringAlignmentCenter, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_WHITEDARK, font8, FontStyleRegular);

        text = (trade->is_open ? L"Open Pos" : L"Closed Pos");
        ld->SetData(1, trade, tickerId, text, StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_ORANGE, font8, FontStyleRegular);   // orange

        text = AfxMoney(std::abs(trade->acb));
        clr = (trade->acb >= 0) ? COLOR_GREEN : COLOR_RED;
        ld->SetData(7, trade, tickerId, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
            clr, font8, FontStyleRegular);  

    }
    else {
        ld->SetData(0, trade, tickerId, GLYPH_CIRCLE, StringAlignmentCenter, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_WHITEDARK, font8, FontStyleRegular);
        ld->SetData(1, trade, tickerId, trade->ticker_symbol, StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_WHITELIGHT, font9, FontStyleRegular | FontStyleBold);

        std::wstring text = L"";

        
        text = (trade->itm_text.length()) ? trade->itm_text : L"";
        clr = (trade->itm_color != COLOR_WHITELIGHT) ? trade->itm_color : COLOR_WHITELIGHT;
        ld->SetData(COLUMN_TICKER_ITM, trade, tickerId, text, StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
            clr, font8, FontStyleRegular);   // ITM
        ld->SetData(3, trade, tickerId, L"", StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_WHITELIGHT, font8, FontStyleRegular);
        ld->SetData(4, trade, tickerId, L"", StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_WHITELIGHT, font8, FontStyleRegular);


        text = (trade->ticker_change_text.length()) ? trade->ticker_change_text : L"";
        clr = (trade->ticker_change_color != COLOR_WHITELIGHT) ? trade->ticker_change_color : COLOR_WHITELIGHT;
        ld->SetData(COLUMN_TICKER_CHANGE, trade, tickerId, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
            clr, font8, FontStyleRegular);   // price change
        
        text = (trade->ticker_last_price_text.length()) ? trade->ticker_last_price_text : L"";
        ld->SetData(COLUMN_TICKER_CURRENTPRICE, trade, tickerId, text, StringAlignmentCenter, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_WHITELIGHT, font9, FontStyleRegular | FontStyleBold);   // current price
        
        text = (trade->ticker_percent_change_text.length()) ? trade->ticker_percent_change_text : L"";
        clr = (trade->ticker_percent_change_color != COLOR_WHITELIGHT) ? trade->ticker_percent_change_color : COLOR_WHITELIGHT;
        ld->SetData(COLUMN_TICKER_PERCENTCHANGE, trade, tickerId, text, StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
            clr, font8, FontStyleRegular);   // price percentage change

        ld->SetData(COLUMN_TICKER_AVGPX, trade, tickerId, L"", StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_WHITEDARK, font8, FontStyleRegular);   // Book Value and average Price
        ld->SetData(COLUMN_TICKER_LASTPX, trade, tickerId, L"", StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_WHITEDARK, font8, FontStyleRegular);   // Market Value and Last Price

        text = (trade->percentage_text.length()) ? trade->percentage_text : L"";
        ld->SetData(COLUMN_TICKER_PERCENTCOMPLETE, trade, tickerId, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_WHITEDARK, font8, FontStyleRegular);   // unrealized PNL
        
        text = (trade->unrealized_pnl_text.length()) ? trade->unrealized_pnl_text : L"";
        clr = (trade->unrealized_pnl_color != COLOR_WHITEDARK) ? trade->unrealized_pnl_color : COLOR_WHITEDARK;
        ld->SetData(COLUMN_TICKER_UPNL, trade, tickerId, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
            clr, font8, FontStyleRegular);   // unrealized PNL
    }
    ListBox_AddString(hListBox, ld);

    // If the Trade is closed then there is nothing to output as an "open position".
    if (!trade->is_open) return;


    // All tickerId will now be -1 because we are no longer dealing with the main isTickerLine.
    tickerId = -1;

    std::wstring wszDot = GLYPH_CIRCLE;   // dot character (Segue UI Windows 10/11)
    DWORD Yellow = COLOR_MAGENTA;



    // *** SHARES/FUTURES ***
    // Roll up all of the SHARES or FUTURES Transactions and display the aggregate rather than the individual legs.
    std::wstring textShares;
    int aggregate = 0;
    for (const auto& leg : trade->open_legs) {
        if (leg->underlying == L"SHARES") {
            textShares = L"SHARES";
            aggregate = aggregate + leg->open_quantity;
        }
        else if (leg->underlying == L"FUTURES") {
            textShares = L"FUTURES";
            aggregate = aggregate + leg->open_quantity;
        }
    }

    if (aggregate) {
        ld = new ListBoxData;

        ld->SetData(0, trade, tickerId, L"", StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_WHITELIGHT, font8, FontStyleRegular);

        int col = 1;
        if (!isHistory) {
            ld->SetData(col, trade, tickerId, wszDot, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
                COLOR_MAGENTA, font8, FontStyleRegular);
            col++;
        }
        col++;

        if (textShares == L"SHARES") ld->lineType = LineType::Shares;
        if (textShares == L"FUTURES") {
            ld->lineType = LineType::Futures;
            textShares = textShares + L": " + AfxFormatFuturesDate(trade->future_expiry);
        }

        ld->SetData(col, trade, tickerId, textShares, StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYMEDIUM,
            COLOR_WHITEDARK, font8, FontStyleRegular);
        col++;

        ld->SetData(col, trade, tickerId, L"", StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYMEDIUM,
            COLOR_WHITEDARK, font8, FontStyleRegular);
        col++;

        ld->SetData(col, trade, tickerId, L"", StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYMEDIUM,
            COLOR_WHITEDARK, font8, FontStyleRegular);
        col++;

        if (isHistory) {
            ld->SetData(col, trade, tickerId, L"", StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYMEDIUM,
                COLOR_WHITEDARK, font8, FontStyleRegular);
            col++;
        }

        text = AfxMoney(std::abs(trade->acb / aggregate));
        ld->SetData(col, trade, tickerId, text, StringAlignmentCenter, StringAlignmentCenter, COLOR_GRAYMEDIUM,
            COLOR_WHITEDARK, font8, FontStyleRegular);
        col++;

        text = std::to_wstring(aggregate);
        ld->AggregateShares = text;
        ld->SetData(col, trade, tickerId, text, StringAlignmentCenter, StringAlignmentCenter, COLOR_GRAYMEDIUM,
            COLOR_WHITEDARK, font8, FontStyleRegular);

        ListBox_AddString(hListBox, ld);
    }


    // *** OPTION LEGS ***
    for (const auto& leg : trade->open_legs) {
        if (leg->underlying == L"OPTIONS") {
            ld = new ListBoxData;

            ld->leg = leg;
            ld->lineType = LineType::OptionsLeg;

            std::wstring currentDate = AfxCurrentDate();
            std::wstring expiry_date = leg->expiry_date;
            std::wstring wszShortDate = AfxShortDate(expiry_date);
            std::wstring wszDTE;


            // If the expiry_date is 2 days or less then display in Yellow, otherwise Magenta.
            int DTE = AfxDaysBetween(currentDate, expiry_date);
            wszDTE = std::to_wstring(DTE) + L"d";

            Yellow = COLOR_MAGENTA;
            if (DTE < 3) {
                Yellow = COLOR_YELLOW;
            }

            // If the expiry year is greater than current year + 1 then add
            // the year to the display string. Useful for LEAP options.
            if (AfxGetYear(expiry_date) > AfxGetYear(currentDate) + 1) {
                wszShortDate.append(L"/");
                wszShortDate.append(std::to_wstring(AfxGetYear(expiry_date)));
            }


            int col = 1;

            if (!isHistory) {
                ld->SetData(col, trade, tickerId, wszDot, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
                    Yellow, font8, FontStyleRegular);
                col++;

                ld->SetData(col, trade, tickerId, L"", StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
                    COLOR_WHITELIGHT, font8, FontStyleRegular);  // empty column
            }
            col++;

            ld->SetData(col, trade, tickerId, std::to_wstring(leg->open_quantity), StringAlignmentFar, StringAlignmentCenter,
                COLOR_GRAYMEDIUM, COLOR_WHITELIGHT, font8, FontStyleRegular);  // position quantity
            col++;

            ld->SetData(col, trade, tickerId, wszShortDate, StringAlignmentCenter, StringAlignmentCenter,
                COLOR_GRAYLIGHT, COLOR_WHITELIGHT, font8, FontStyleRegular);   // expiry date
            col++;

            ld->SetData(col, trade, tickerId, wszDTE, StringAlignmentCenter, StringAlignmentCenter, COLOR_GRAYMEDIUM,
                COLOR_WHITEDARK, font8, FontStyleRegular);   // DTE
            col++;

            ld->SetData(col, trade, tickerId, leg->strike_price, StringAlignmentCenter, StringAlignmentCenter,
                COLOR_GRAYLIGHT, COLOR_WHITELIGHT, font8, FontStyleRegular);   // strike price
            col++;

            ld->SetData(col, trade, tickerId, L"  " + leg->put_call, StringAlignmentNear, StringAlignmentCenter, 
                COLOR_GRAYMEDIUM, COLOR_WHITEDARK, font8, FontStyleRegular);   // put_call
            col++;

            if (!isHistory) {

                std::wstring text = L"";

                text = (leg->average_price_text.length()) ? leg->average_price_text : L"";
                ld->SetData(COLUMN_TICKER_AVGPX, trade, tickerId, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
                    COLOR_WHITEDARK, font8, FontStyleRegular);   // Book Value and average Price
                col++;
                
                text = (leg->market_price_text.length()) ? leg->market_price_text : L"";
                ld->SetData(COLUMN_TICKER_LASTPX, trade, tickerId, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
                    COLOR_WHITEDARK, font8, FontStyleRegular);   // Market Value and Last Price
                col++;
                
                text = (leg->percentage_text.length()) ? leg->percentage_text: L"";
                ld->SetData(COLUMN_TICKER_PERCENTCOMPLETE, trade, tickerId, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
                    COLOR_WHITEDARK, font8, FontStyleRegular);   // Percentage values for the previous two columns data
                col++;
                
                text = (leg->unrealized_pnl_text.length()) ? leg->unrealized_pnl_text : L"";
                clr = (leg->unrealized_pnl_color != COLOR_WHITEDARK) ? leg->unrealized_pnl_color : COLOR_WHITEDARK;
                ld->SetData(COLUMN_TICKER_UPNL, trade, tickerId, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
                    clr, font8, FontStyleRegular);   // Unrealized profit or loss
            }

            ListBox_AddString(hListBox, ld);
        }
    }


    // *** BLANK SEPARATION LINE ***
    ListBoxData_AddBlankLine(hListBox);

}


// ========================================================================================
// Create the display data for a blank line
// ========================================================================================
void ListBoxData_AddBlankLine(HWND hListBox)
{
    // *** BLANK SEPARATION LINE AT END OF LIST ***
    ListBoxData* ld = new ListBoxData;
    ld->lineType = LineType::None;
    ListBox_AddString(hListBox, ld);
}


// ========================================================================================
// Create the display data a History Header line.
// ========================================================================================
void ListBoxData_HistoryHeader(HWND hListBox, const std::shared_ptr<Trade>& trade, const std::shared_ptr<Transaction>& trans)
{
    // Display the transaction description, date, and total prior to showing the detail lines
    ListBoxData* ld = new ListBoxData;

    std::wstring text;
    REAL font8 = 8;

    TickerId tickerId = -1;

    ld->lineType = LineType::TransactionHeader;
    ld->trans = trans;

    ld->SetData(0, trade, tickerId, GLYPH_TREEOPEN, StringAlignmentCenter, StringAlignmentCenter, COLOR_GRAYDARK,
        COLOR_WHITEDARK, font8, FontStyleRegular);

    text = trans->description;
    ld->SetData(1, trade, tickerId, text, StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
        COLOR_ORANGE, font8, FontStyleRegular);   // orange

    text = trans->trans_date;
    ld->SetData(2, trade, tickerId, text, StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
        COLOR_WHITELIGHT, font8, FontStyleRegular);

    text = AfxMoney(std::abs(trans->total));
    DWORD clr = (trans->total >= 0) ? COLOR_GREEN : COLOR_RED;
    ld->SetData(7, trade, tickerId, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
        clr, font8, FontStyleRegular);   // green/red

    ld->SetData(8, trade, tickerId, GLYPH_MAGNIFYGLASS, StringAlignmentCenter, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_GRAYDARK, font8, FontStyleRegular);

    ListBox_AddString(hListBox, ld);

}


// ========================================================================================
// Create the display data for a History SHARES/FUTURES leg.
// ========================================================================================
void ListBoxData_HistorySharesLeg(
    HWND hListBox, const std::shared_ptr<Trade>& trade, const std::shared_ptr<Transaction>& trans, const std::shared_ptr<Leg>& leg)
{
    ListBoxData* ld = new ListBoxData;

    TickerId tickerId = -1;
    REAL font8 = 8;

    ld->SetData(2, trade, tickerId, trans->underlying, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYMEDIUM, COLOR_WHITEDARK, font8, FontStyleRegular);

    for (int i = 3; i < 6; i++) {
        ld->SetData(i, trade, tickerId, L"", StringAlignmentNear, StringAlignmentCenter,
            COLOR_GRAYMEDIUM, COLOR_WHITEDARK, font8, FontStyleRegular);
    }
    
    std::wstring text = AfxMoney(trans->price, false, trade->ticker_decimals);
    ld->SetData(6, trade, tickerId, text, StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYMEDIUM, COLOR_WHITEDARK, font8, FontStyleRegular);

    ld->SetData(7, trade, tickerId, std::to_wstring(leg->open_quantity), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYMEDIUM, COLOR_WHITEDARK, font8, FontStyleRegular);

    ListBox_AddString(hListBox, ld);
}


// ========================================================================================
// Create the display data for a History OPTIONS leg.
// ========================================================================================
void ListBoxData_HistoryOptionsLeg(
    HWND hListBox, const std::shared_ptr<Trade>& trade, const std::shared_ptr<Transaction>& trans, const std::shared_ptr<Leg>& leg)
{
    ListBoxData* ld = new ListBoxData;

    TickerId tickerId = -1;
    REAL font8 = 8;

    std::wstring text = std::to_wstring(leg->original_quantity);
    ld->SetData(2, trade, tickerId, text, StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYMEDIUM, COLOR_WHITELIGHT, font8, FontStyleRegular);


    int DTE = AfxDaysBetween(trans->trans_date, leg->expiry_date);
    std::wstring wszDays = std::to_wstring(DTE) + L"d";
    std::wstring wszShortDate = AfxShortDate(leg->expiry_date);

    // If the expiry year is greater than current year + 1 then add
    // the year to the display string. Useful for LEAP options.
    if (AfxGetYear(leg->expiry_date) > AfxGetYear(trans->trans_date) + 1) {
        wszShortDate.append(L"/");
        wszShortDate.append(std::to_wstring(AfxGetYear(leg->expiry_date)));
    }

    ld->SetData(3, trade, tickerId, wszShortDate, StringAlignmentCenter, StringAlignmentCenter,
        COLOR_GRAYLIGHT, COLOR_WHITELIGHT, font8, FontStyleRegular);

    ld->SetData(4, trade, tickerId, wszDays, StringAlignmentCenter, StringAlignmentCenter,
        COLOR_GRAYMEDIUM, COLOR_WHITEDARK, font8, FontStyleRegular);

    ld->SetData(5, trade, tickerId, leg->strike_price, StringAlignmentCenter, StringAlignmentCenter,
        COLOR_GRAYLIGHT, COLOR_WHITELIGHT, font8, FontStyleRegular);

    ld->SetData(6, trade, tickerId, L" " + leg->put_call, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYMEDIUM, COLOR_WHITEDARK, font8, FontStyleRegular);

    DWORD clr = COLOR_RED;
    if (leg->action == L"BTO" || leg->action == L"BTC") clr = COLOR_GREEN;

    ld->SetData(7, trade, tickerId, leg->action, StringAlignmentCenter, StringAlignmentCenter,
        COLOR_GRAYLIGHT, clr, font8, FontStyleRegular);

    ListBox_AddString(hListBox, ld);
}


// ========================================================================================
// Create the display data line for closed position yearly total.
// ========================================================================================
void ListBoxData_OutputClosedYearTotal(HWND hListBox, int year, double subtotal, int YearWin, int YearLoss)
{
    ListBoxData* ld = new ListBoxData;

    TickerId tickerId = -1;
    REAL font8 = 8;

    DWORD clr = (subtotal >= 0) ? COLOR_GREEN : COLOR_RED;
    
    std::wstring text = std::to_wstring(year) + L" TOTAL";
    ld->SetData(3, nullptr, tickerId, text, StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleBold);

    ld->SetData(4, nullptr, tickerId, AfxMoney(subtotal), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleBold);

    // col 5 is a "spacer" column

    clr = (YearWin >= YearLoss) ? COLOR_GREEN : COLOR_RED;
    text = std::to_wstring(YearWin) + L"W " + std::to_wstring(YearLoss)
        + L"L  " + AfxMoney((double)YearWin / (max(YearWin + YearLoss,1)) * 100, false, 0) + L"%";
    ld->SetData(6, nullptr, tickerId, text, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    ListBox_InsertString(hListBox, 0, ld);
}


// ========================================================================================
// Create the display data line for total closed weekly total.
// ========================================================================================
void ListBoxData_OutputClosedWeekTotal(HWND hListBox, double weeklyTotal, int WeekWin, int WeekLoss)
{
    ListBoxData* ld = new ListBoxData;

    TickerId tickerId = -1;
    REAL font8 = 8;

    DWORD clr = (weeklyTotal >= 0) ? COLOR_GREEN : COLOR_RED;
    
    std::wstring text = L"THIS WEEK";
    ld->SetData(3, nullptr, tickerId, text, StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleBold);

    ld->SetData(4, nullptr, tickerId, AfxMoney(weeklyTotal), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleBold);

    // col 5 is a "spacer" column

    clr = (WeekWin >= WeekLoss) ? COLOR_GREEN : COLOR_RED;
    text = std::to_wstring(WeekWin) + L"W " + std::to_wstring(WeekLoss)
        + L"L  " + AfxMoney((double)WeekWin / (max(WeekWin + WeekLoss,1)) * 100, false, 0) + L"%";
    ld->SetData(6, nullptr, tickerId, text, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    ListBox_InsertString(hListBox, 1, ld);
}


// ========================================================================================
// Create the display data line for total closed daily total.
// ========================================================================================
void ListBoxData_OutputClosedDayTotal(HWND hListBox, double dailyTotal, int DayWin, int DayLoss)
{
    ListBoxData* ld = new ListBoxData;

    TickerId tickerId = -1;
    REAL font8 = 8;

    DWORD clr = (dailyTotal >= 0) ? COLOR_GREEN : COLOR_RED;
    
    std::wstring text = L"TODAY";
    ld->SetData(3, nullptr, tickerId, text, StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleBold);

    ld->SetData(4, nullptr, tickerId, AfxMoney(dailyTotal), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleBold);

    // col 5 is a "spacer" column

    clr = (DayWin >= DayLoss) ? COLOR_GREEN : COLOR_RED;
    text = std::to_wstring(DayWin) + L"W " + std::to_wstring(DayLoss)
        + L"L  " + AfxMoney((double)DayWin / (max(DayWin + DayLoss,1)) * 100, false, 0) + L"%";
    ld->SetData(6, nullptr, tickerId, text, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    ListBox_InsertString(hListBox, 2, ld);

    // *** BLANK SEPARATION LINE AFTER THE DAY TOTAL ***
    ld = new ListBoxData;
    ld->lineType = LineType::None;
    ListBox_InsertString(hListBox, 3, ld);
}


// ========================================================================================
// Create the display data line for a closed position month subtotal.
// ========================================================================================
void ListBoxData_OutputClosedMonthSubtotal(HWND hListBox, std::wstring closedDate, double subtotal, int MonthWin, int MonthLoss)
{
    ListBoxData* ld = new ListBoxData;

    TickerId tickerId = -1;
    REAL font8 = 8;

    DWORD clr = (subtotal >= 0) ? COLOR_GREEN : COLOR_RED;
    
    std::wstring text = AfxUpper(AfxGetLongMonthName(closedDate)) + L" " + std::to_wstring(AfxGetYear(closedDate));
    ld->SetData(3, nullptr, tickerId, text, StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleBold);

    ld->SetData(4, nullptr, tickerId, AfxMoney(subtotal), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleBold);

    // col 5 is a "spacer" column

    clr = (MonthWin >= MonthLoss) ? COLOR_GREEN : COLOR_RED;
    text = std::to_wstring(MonthWin) + L"W " + std::to_wstring(MonthLoss)
        + L"L  " + AfxMoney((double)MonthWin / (max(MonthWin + MonthLoss,1)) * 100, false, 0) + L"%";
    ld->SetData(6, nullptr, tickerId, text, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    ListBox_AddString(hListBox, ld);
    ListBoxData_AddBlankLine(hListBox);

}


// ========================================================================================
// Create the display data line for a closed position.
// ========================================================================================
void ListBoxData_OutputClosedPosition(HWND hListBox, const std::shared_ptr<Trade>& trade, std::wstring closedDate)
{
    ListBoxData* ld = new ListBoxData;

    TickerId tickerId = -1;
    REAL font8 = 8;

    ld->SetData(0, trade, tickerId, L"", StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_ORANGE, font8, FontStyleRegular);

    ld->SetData(1, trade, tickerId, closedDate, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_ORANGE, font8, FontStyleRegular);

    ld->SetData(2, trade, tickerId, trade->ticker_symbol, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_WHITELIGHT, font8, FontStyleRegular);

    std::wstring wszTickerName = trade->ticker_name;
    if (IsFuturesTicker(trade->ticker_symbol)) wszTickerName = wszTickerName + L" (" + AfxFormatFuturesDate(trade->future_expiry) + L")";
    ld->SetData(3, trade, tickerId, wszTickerName, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_WHITELIGHT, font8, FontStyleRegular);

    DWORD clr = (trade->acb >= 0) ? COLOR_GREEN : COLOR_RED;
    ld->SetData(4, trade, tickerId, AfxMoney(trade->acb), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    // col 5 is a "spacer" column

    ld->SetData(6, trade, tickerId, GetCategoryDescription(trade->category), StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_WHITEDARK, font8, FontStyleRegular);

    ListBox_AddString(hListBox, ld);
}


// ========================================================================================
// Create the display data line for Transaction running total.
// ========================================================================================
void ListBoxData_OutputTransactionRunningTotal(HWND hListBox, 
    double runningGrossTotal, double runningFeesTotal, double runningNetTotal)
{
    ListBoxData* ld = new ListBoxData;

    TickerId tickerId = -1;
    REAL font8 = 8;

    DWORD clr = (runningNetTotal >= 0) ? COLOR_GREEN : COLOR_RED;

    std::wstring text = L"TOTAL";
    ld->SetData(3, nullptr, tickerId, text, StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    ld->SetData(5, nullptr, tickerId, AfxMoney(runningGrossTotal), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleBold);

    ld->SetData(6, nullptr, tickerId, AfxMoney(runningFeesTotal), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleBold);

    ld->SetData(7, nullptr, tickerId, AfxMoney(runningNetTotal), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleBold);

    ListBox_InsertString(hListBox, 0, ld);
    // *** BLANK SEPARATION LINE AFTER THE TOTAL ***
    ld = new ListBoxData;
    ld->lineType = LineType::None;
    ListBox_InsertString(hListBox, 1, ld);
}


// ========================================================================================
// Create the display data line for a transaction day subtotal.
// ========================================================================================
void ListBoxData_OutputTransactionDaySubtotal(HWND hListBox, std::wstring transDate, 
        double subGrossAmount, double subFeesAmount, double subNetAmount)
{
    ListBoxData* ld = new ListBoxData;

    TickerId tickerId = -1;
    REAL font8 = 8;

    DWORD clr = (subNetAmount >= 0) ? COLOR_GREEN : COLOR_RED;

    //  example: MON JUL 5
    //std::wstring text = AfxUpper(AfxGetShortDayName(transDate) + L" " + 
    //    AfxGetShortMonthName(transDate)) + L" " + std::to_wstring(AfxGetDay(transDate));
    std::wstring text = AfxUpper(AfxGetLongDayName(transDate));
    ld->SetData(3, nullptr, tickerId, text, StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    ld->SetData(5, nullptr, tickerId, AfxMoney(subGrossAmount), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleBold);

    ld->SetData(6, nullptr, tickerId, AfxMoney(subFeesAmount), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleBold);

    ld->SetData(7, nullptr, tickerId, AfxMoney(subNetAmount), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleBold);

    ListBox_AddString(hListBox, ld);
    ListBoxData_AddBlankLine(hListBox);
}


// ========================================================================================
// Create the display data line for a Transaction.
// ========================================================================================
void ListBoxData_OutputTransaction(
    HWND hListBox, const std::shared_ptr<Trade>& trade, const std::shared_ptr<Transaction>& trans)
{
    ListBoxData* ld = new ListBoxData;

    TickerId tickerId = -1;
    REAL font8 = 8;

    ld->trans = trans;

    ld->SetData(0, trade, tickerId, L"", StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_ORANGE, font8, FontStyleRegular);

    ld->SetData(1, trade, tickerId, trans->trans_date, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_WHITELIGHT, font8, FontStyleRegular);

    ld->SetData(2, trade, tickerId, trade->ticker_symbol, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_ORANGE, font8, FontStyleRegular);

    ld->SetData(3, trade, tickerId, trans->description, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_WHITELIGHT, font8, FontStyleRegular);

    ld->SetData(4, trade, tickerId, AfxMoney(trans->quantity), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_WHITELIGHT, font8, FontStyleRegular);

    ld->SetData(5, trade, tickerId, AfxMoney(trans->price, false, GetTickerDecimals(trade->ticker_symbol)), 
        StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK, COLOR_WHITELIGHT, font8, FontStyleRegular);

    ld->SetData(6, trade, tickerId, AfxMoney(trans->fees), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_WHITELIGHT, font8, FontStyleRegular);

    DWORD clr = (trans->total >= 0) ? COLOR_GREEN : COLOR_RED;
    ld->SetData(7, trade, tickerId, AfxMoney(trans->total), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    ListBox_AddString(hListBox, ld);
}


// ========================================================================================
// Create the display data line for a Ticker total.
// ========================================================================================
void ListBoxData_OutputTickerTotals(HWND hListBox, std::wstring ticker, double amount)
{
    ListBoxData* ld = new ListBoxData;

    TickerId tickerId = -1;
    REAL font8 = 8;
    REAL font9 = 9;

    ld->SetData(1, nullptr, tickerId, ticker, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_ORANGE, font8, FontStyleRegular);

    // Look up the Company name based on the tickerid
    auto iter = std::find_if(trades.begin(), trades.end(),
         [&](const auto t) { return (t->ticker_symbol == ticker && t->ticker_name.length()); });

    if (iter != trades.end()) {
        auto index = std::distance(trades.begin(), iter);
        ld->SetData(2, nullptr, tickerId, trades.at(index)->ticker_name, StringAlignmentNear, StringAlignmentCenter,
            COLOR_GRAYDARK, COLOR_ORANGE, font8, FontStyleRegular);
    }

    DWORD clr = (amount >= 0) ? COLOR_GREEN : COLOR_RED;
    ld->SetData(3, nullptr, tickerId, AfxMoney(amount), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    ListBox_AddString(hListBox, ld);
}


// ========================================================================================
// Generic Header control WM_PAINT message handler (for all ListBox header controls)
// ========================================================================================
void Header_OnPaint(HWND hWnd)
{
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hWnd, &ps);

    int nWidth = (ps.rcPaint.right - ps.rcPaint.left);
    int nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);

    Graphics graphics(hdc);
    SolidBrush back_brush(COLOR_GRAYMEDIUM);
    graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);


    graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

    std::wstring font_name = AfxGetDefaultFont();
    FontFamily fontFamily(font_name.c_str());
    Font       font(&fontFamily, 9, FontStyleRegular, Unit::UnitPoint);
    SolidBrush text_brush(COLOR_WHITELIGHT);

    StringFormat stringF(StringFormatFlagsNoWrap);
    stringF.SetTrimming(StringTrimmingEllipsisWord);
    stringF.SetAlignment(StringAlignmentCenter);       // horizontal
    stringF.SetLineAlignment(StringAlignmentCenter);   // vertical

    RECT rcItem{};

    int nCount = Header_GetItemCount(hWnd);

    for (int i = 0; i < nCount; i++) {
        Header_GetItemRect(hWnd, i, &rcItem);

        switch (Header_GetItemAlignment(hWnd, i))
        {
        case HDF_LEFT:
            stringF.SetAlignment(StringAlignmentNear);
            break;
        case HDF_CENTER:
            stringF.SetAlignment(StringAlignmentCenter);
            break;
        case HDF_RIGHT:
            stringF.SetAlignment(StringAlignmentFar);
            break;
        }

        // Draw our right side separator line
        if (i > 0 && i < nCount) {
            ARGB clrPen = COLOR_SCROLLBARDIVIDER;
            Pen pen(clrPen, 1);
            graphics.DrawLine(&pen, (REAL)rcItem.right, (REAL)rcItem.top, (REAL)rcItem.right, (REAL)rcItem.bottom);
        }

        // Add left/right padding to our text rectangle
        InflateRect(&rcItem, -AfxScaleX(4), 0);
        RectF rcText((REAL)rcItem.left, (REAL)rcItem.top, (REAL)(rcItem.right - rcItem.left), (REAL)(rcItem.bottom - rcItem.top));
        std::wstring text = Header_GetItemText(hWnd, i);
        graphics.DrawString(text.c_str(), -1, &font, rcText, &stringF, &text_brush);

    }

    EndPaint(hWnd, &ps);

}


// ========================================================================================
// Process WM_DRAWITEM message for window/dialog 
// (common function for custom drawing listbox data)
// ========================================================================================
void ListBoxData_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT* lpDrawItem)
{
   if (lpDrawItem->itemID == -1) return;

    if (lpDrawItem->itemAction == ODA_DRAWENTIRE ||
        lpDrawItem->itemAction == ODA_SELECT) {

        int nWidth = (lpDrawItem->rcItem.right - lpDrawItem->rcItem.left);
        int nHeight = (lpDrawItem->rcItem.bottom - lpDrawItem->rcItem.top);

        bool bIsHot = false;

        SaveDC(lpDrawItem->hDC);

        HDC memDC = NULL;         // Double buffering
        HBITMAP hbit = NULL;      // Double buffering

        memDC = CreateCompatibleDC(lpDrawItem->hDC);
        hbit = CreateCompatibleBitmap(lpDrawItem->hDC, nWidth, nHeight);
        if (hbit) SelectObject(memDC, hbit);

        if ((lpDrawItem->itemAction | ODA_SELECT) &&
            (lpDrawItem->itemState & ODS_SELECTED)) {
            bIsHot = true;
        }

        Graphics graphics(memDC);
        graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);


        // Set some defaults in case there is no valid ListBox line number
        std::wstring text;

        DWORD back_color = (bIsHot) ? COLOR_SELECTION : COLOR_GRAYDARK;
        DWORD back_color_hot = COLOR_SELECTION;
        DWORD ntext_color = COLOR_WHITELIGHT;

        std::wstring font_name = AfxGetDefaultFont();
        FontFamily   fontFamily(font_name.c_str());
        REAL fontSize = 10;
        int fontStyle = FontStyleRegular;

        StringAlignment HAlignment = StringAlignmentNear;
        StringAlignment VAlignment = StringAlignmentCenter;

        // Paint the full width background using brush 
        SolidBrush back_brush(back_color);
        graphics.FillRectangle(&back_brush, 0, 0, nWidth, nHeight);

        // Get the current ListBox line data should a valid line exist
        // Paint the individual columns with the specific data.
        ListBoxData* ld = (ListBoxData*)(lpDrawItem->itemData);
        int nLeft = 0;
        int colWidth = 0;
        int colStart = 0;
        int colEnd = MAX_COLUMNS;

        // If this is a Category separator line then we need to draw it differently
        // then a regular that would be drawn (full line width)
        if (ld != nullptr) {
            if (ld->lineType == LineType::CategoryHeader) {
                colStart = 0;
                colEnd = 1;
            }
        }

        // Draw each of the columns
        for (int i = colStart; i < colEnd; i++) {
            if (ld == nullptr) break;
            if (ld->col[i].colWidth == 0) break;

            // Prepare and draw the text
            text = ld->col[i].text;

            HAlignment = ld->col[i].HAlignment;
            VAlignment = ld->col[i].VAlignment;
            back_color = (bIsHot) ? back_color : ld->col[i].backTheme;
            ntext_color = ld->col[i].textTheme;
            fontSize = ld->col[i].fontSize;
            fontStyle = ld->col[i].fontStyle;

            colWidth = (ld->lineType == LineType::CategoryHeader) ? nWidth : AfxScaleX((float)ld->col[i].colWidth);

            back_brush.SetColor(back_color);
            graphics.FillRectangle(&back_brush, nLeft, 0, colWidth, nHeight);

            Font         font(&fontFamily, fontSize, fontStyle, Unit::UnitPoint);
            SolidBrush   text_brush(ntext_color);
            StringFormat stringF(StringFormatFlagsNoWrap);
            stringF.SetAlignment(HAlignment);
            stringF.SetLineAlignment(VAlignment);

            // If right alignment then add a very small amount of right side
            // padding so that text is not pushed up right against the right side.
            int rightPadding = 0;
            if (HAlignment == StringAlignmentFar) rightPadding = AfxScaleX(2);

            RectF rcText((REAL)nLeft, (REAL)0, (REAL)colWidth - rightPadding, (REAL)nHeight);
            graphics.DrawString(text.c_str(), -1, &font, rcText, &stringF, &text_brush);

            nLeft += colWidth;
        }


        BitBlt(lpDrawItem->hDC, lpDrawItem->rcItem.left,
            lpDrawItem->rcItem.top, nWidth, nHeight, memDC, 0, 0, SRCCOPY);


        // Cleanup
        RestoreDC(lpDrawItem->hDC, -1);
        if (hbit) DeleteObject(hbit);
        if (memDC) DeleteDC(memDC);
    }
}

