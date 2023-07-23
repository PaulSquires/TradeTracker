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
#include "TickerTotals/TickerTotals.h"
#include "Transactions/TransPanel.h"
#include "DailyTotals/DailyTotals.h"
#include "Database/database.h"
#include "Config/Config.h"
#include "Utilities/Colors.h"

extern HWND HWND_SIDEMENU;
extern HWND HWND_ACTIVETRADES;
extern HWND HWND_CLOSEDTRADES;
extern HWND HWND_TRADEHISTORY;
extern HWND HWND_TICKERPANEL;
extern HWND HWND_DAILYTOTALS;
extern HWND HWND_TRANSPANEL;
extern HWND HWND_TRADEDIALOG;



int nHistoryMinColWidth[10] =
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
    0
};

// We need a maximum column size for the History table because the
// user may end a very long description and we don't want the column
// to expand to fit this whole trade description. We will still
// display the description but it will wrap in the display rectangle.
int nHistoryMaxColWidth[10] =
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
    0
};

int nTradesMinColWidth[10] =
{
    25,     /* dropdown arrow */
    50,     /* ticker symbol */
    50,     /* ITM */
    50,     /* position quantity */  
    50,     /* expiry date */
    45,     /* DTE */
    50,     /* strike price / current price */
    45,     /* put/call */
    0,
    0
};

int nClosedMinColWidth[10] =
{
    15,     /* empty */
    65,     /* Close Date */
    50,     /* Ticker Symbol */
    200,    /* Ticker Name */
    100,    /* Amount */
    0,     
    0,     
    0,     
    0,
    0
};

int nTransMinColWidth[10] =
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
    0
};

int nTickerTotalsMinColWidth[10] =
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
    0
};

int nTickerTotalsMaxColWidth[10] =
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
    0
};

int nDailyTotalsMinColWidth[10] =
{
    5,      /* dropdown arrow */
    75,     /* Date/Ticker */
    150,    /* Day/Description */
    100,    /* Amount */
    0,
    0,
    0,
    0,
    0,
    0
};

int nDailyTotalsSummaryMinColWidth[10] =
{
    75,     /* Profit/Loss */
    75,     /* Stock value */
    75,     /* Net Profit/Loss */
    75,     /* MTD */
    75,     /* YTD */
    0,
    0,
    0,
    0,
    0
};

int nTradeTemplatesMinColWidth[10] =
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
    0
};

int nColWidth[10] = { 0,0,0,0,0,0,0,0,0 };

bool PrevMarketDataLoaded = false;


// ========================================================================================
// Helper function to get the Color for the specified Category
// ========================================================================================
DWORD GetCategoryColor(int category)
{
    DWORD catTextColor = COLOR_WHITEDARK;
    switch (category)
    {
    case 0: catTextColor = COLOR_WHITEDARK; break;
    case 1: catTextColor = COLOR_BLUE; break;
    case 2: catTextColor = COLOR_PINK; break;
    case 3: catTextColor = COLOR_GREEN; break;
    case 4: catTextColor = COLOR_ORANGE; break;
    case 5: catTextColor = COLOR_RED; break;
    case 6: catTextColor = COLOR_TEAL; break;
    case 7: catTextColor = COLOR_KHAKI; break;
    default: catTextColor = COLOR_WHITEDARK;
    }
    return catTextColor;
}


// ========================================================================================
// Calculate the actual column widths based on the size of the strings in
// ListBoxData while respecting the minimum values as defined in nMinColWidth[].
// This function is also called when receiving new price data from TWS because
// that data may need the column width to be wider.
// ========================================================================================
void ListBoxData_ResizeColumnWidths(HWND hListBox, TableType tabletype, int nIndex)
{
    HDC hdc = GetDC(hListBox);

    // Initialize the nColWidth array based on the incoming ListBox
    for (int i = 0; i < 10; i++) {
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

        case TableType::DailyTotals:
            nColWidth[i] = nDailyTotalsMinColWidth[i];
            break;

        case TableType::DailyTotalsSummary:
            nColWidth[i] = nDailyTotalsSummaryMinColWidth[i];
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

    std::wstring wszFontName = AfxGetDefaultFont();
    FontFamily fontFamily(wszFontName.c_str());
    REAL fontSize = 0;
    int fontStyle = FontStyleRegular;
    RectF boundRect;
    RectF layoutRect(0.0f, 0.0f, 1000.0f, 50.0f);
    StringFormat format;
    format.SetAlignment(StringAlignmentFar);

    bool bRedrawListBox = false;

    int nEnd = (int)ListBox_GetCount(hListBox) - 1;
    if (nEnd < 0) return;
    int nStart = 0;

    // If a specific line number was passed into this function then we only
    // test for that line rather than all lines (like when the arrays are first loaded).
    // A value of -1 will iterate all strings the columns.
    if (nIndex != -1) {
        nStart = nIndex; nEnd = nIndex;
    }

    for (int ii = nStart; ii <= nEnd; ii++) {
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, ii);
        if (ld == nullptr) continue;
        if (ld->lineType == LineType::CategoryHeader) break;
        for (int i = 0; i < 10; i++) {
            if (nColWidth[i] == 0) continue;
            fontSize = ld->col[i].fontSize;
            fontStyle = ld->col[i].fontStyle;
            Font font(&fontFamily, fontSize, fontStyle, Unit::UnitPoint);
            
            graphics.MeasureString(ld->col[i].wszText.c_str(), ld->col[i].wszText.length(),
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

            
        if (nIndex != -1) break;
    }


    // Update the newly calculated column widths into each of the ld structures
    for (int ii = nStart; ii <= nEnd; ii++) {
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, ii);
        if (ld == nullptr) continue;
        for (int i = 0; i < 10; i++) {
            ld->col[i].colWidth = nColWidth[i];
        }
        ListBox_SetItemData(hListBox, ii, ld);
    }

    // Update the widths of any associated Header control. 
    HWND hHeader = NULL;
    if (tabletype == TableType::ClosedTrades) hHeader = GetDlgItem(HWND_CLOSEDTRADES, IDC_CLOSED_HEADER);
    if (tabletype == TableType::TickerTotals) hHeader = GetDlgItem(HWND_TICKERPANEL, IDC_TICKER_HEADER_TOTALS);
    if (tabletype == TableType::DailyTotalsSummary) hHeader = GetDlgItem(HWND_DAILYTOTALS, IDC_DAILY_HEADER_SUMMARY);
    if (tabletype == TableType::DailyTotals) hHeader = GetDlgItem(HWND_DAILYTOTALS, IDC_DAILY_HEADER_TOTALS);
    if (tabletype == TableType::TransPanel) hHeader = GetDlgItem(HWND_TRANSPANEL, IDC_TRANS_HEADER);
    
    if (hHeader) {
        for (int i = 0; i < 10; i++) {
            Header_SetItemWidth(hHeader, i, AfxScaleX((float)nColWidth[i]));
        }
    }

    ReleaseDC(hListBox, hdc);
    
    if (bRedrawListBox) {
        AfxRedrawWindow(hListBox);
    }
}


// ========================================================================================
// Destroy all manually allocated ListBox display data that is held in LineData structures.
// ========================================================================================
void ListBoxData_DestroyItemData(HWND hListBox)
{
    // Cancel any previous market data requests and delete any previously
    // allocated ListBoxData structures.
    int lbCount = ListBox_GetCount(hListBox);

    for (int i = 0; i < lbCount; i++) {
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, i);
        if (ld != nullptr) {
            if (ld->lineType == LineType::TickerLine && PrevMarketDataLoaded) {
                // Cancelling a market data symbol is not synchronous therefore we need
                // to use ever increasing tickerid numbers to avoid duplicate ticker id
                // errors when the listbox is reloaded.
                tws_cancelMktData(ld->tickerId);
            }
            delete(ld);
        }
    }

    // Clear the current trades listbox
    ListBox_ResetContent(hListBox);
    if (hListBox == GetDlgItem(HWND_ACTIVETRADES, IDC_TRADES_LISTBOX)) {
        PrevMarketDataLoaded = false;
    }
}


// ========================================================================================
// Loop through the ListBox data and request market data for tickers
// ========================================================================================
void ListBoxData_RequestMarketData(HWND hListBox)
{
    // Request market data for each open trade
    int lbCount = ListBox_GetCount(hListBox);
    for (int i = 0; i < lbCount; i++) {
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, i);
        if (ld != nullptr) {
            if (ld->lineType == LineType::TickerLine) {
                tws_requestMktData(ld);
            }
        }
    }

    if (tws_isConnected())
        PrevMarketDataLoaded = true;
}


// ========================================================================================
// Create the display data for a Category Header line
// ========================================================================================
void ListBoxData_AddCategoryHeader(HWND hListBox, const std::shared_ptr<Trade>& trade)
{
    ListBoxData* ld = new ListBoxData;

    std::wstring wszText = AfxUpper(GetCategoryDescription(trade->category));

    ld->SetData(0, nullptr, -1, wszText, StringAlignmentNear, StringAlignmentCenter,
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

    text = AfxMoney(trade->TradeBP, true, 0);
    ld->SetData(2, trade, tickerId, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
        COLOR_WHITELIGHT, font8, FontStyleRegular);
    text = L"BP";
    ld->SetData(3, trade, tickerId, text, StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
        COLOR_WHITEDARK, font8, FontStyleRegular);
    ListBox_AddString(hListBox, ld);

    ld = new ListBoxData;
    std::wstring startDate = InsertDateHyphens(trade->BPstartDate);
    std::wstring endDate = InsertDateHyphens(trade->BPendDate);
    int days = AfxDaysBetween(startDate, endDate);
    text = AfxMoney(days, true, 0);
    ld->SetData(2, trade, tickerId, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
        COLOR_WHITELIGHT, font8, FontStyleRegular);
    text = L"Days";
    ld->SetData(3, trade, tickerId, text, StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
        COLOR_WHITEDARK, font8, FontStyleRegular);
    text = AfxMoney(0, true, 1) + L"%";
    if (trade->TradeBP != 0 && days != 0) {
        text = AfxMoney((trade->ACB / trade->TradeBP * 100 / days * 30), true, 1) + L"%";
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
    std::wstring text; 

    if (isHistory) {
        tickerId = -1;

        ld->SetData(0, trade, tickerId, GLYPH_TREEOPEN, StringAlignmentCenter, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_WHITEDARK, font8, FontStyleRegular);

        text = (trade->isOpen ? L"Open Pos" : L"Closed Pos");
        ld->SetData(1, trade, tickerId, text, StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_ORANGE, font8, FontStyleRegular);   // orange

        text = AfxMoney(std::abs(trade->ACB));
        DWORD clr = (trade->ACB >= 0) ? COLOR_GREEN : COLOR_RED;
        ld->SetData(7, trade, tickerId, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
            clr, font8, FontStyleRegular);  

    }
    else {
        ld->SetData(0, trade, tickerId, GLYPH_CIRCLE, StringAlignmentCenter, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_WHITEDARK, font8, FontStyleRegular);
        ld->SetData(1, trade, tickerId, trade->tickerSymbol, StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_WHITELIGHT, font9, FontStyleRegular | FontStyleBold);
        // Col 1 to 6 are set based on incoming TWS price data 
        ld->SetData(COLUMN_TICKER_ITM, trade, tickerId, L"", StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_WHITELIGHT, font8, FontStyleRegular);   // ITM
        ld->SetData(3, trade, tickerId, L"", StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_WHITELIGHT, font8, FontStyleRegular);
        ld->SetData(4, trade, tickerId, L"", StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_WHITELIGHT, font8, FontStyleRegular);
        ld->SetData(COLUMN_TICKER_CHANGE, trade, tickerId, L"", StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_WHITEDARK, font8, FontStyleRegular);   // price change
        ld->SetData(COLUMN_TICKER_CURRENTPRICE, trade, tickerId, L"0.00", StringAlignmentCenter, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_WHITELIGHT, font9, FontStyleRegular | FontStyleBold);   // current price
        ld->SetData(COLUMN_TICKER_PERCENTAGE, trade, tickerId, L"", StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_WHITEDARK, font8, FontStyleRegular);   // price percentage change
    }
    ListBox_AddString(hListBox, ld);

    // If the Trade is closed then there is nothing to output as an "open position".
    if (!trade->isOpen) return;


    // All tickerId will now be -1 because we are no longer dealing with the main isTickerLine.
    tickerId = -1;

    std::wstring wszDot = GLYPH_CIRCLE;   // dot character (Segue UI Windows 10/11)
    DWORD Yellow = COLOR_MAGENTA;



    // *** SHARES ***
    // Roll up all of the SHARES or FUTURES Transactions and display the aggregate rather than the individual legs.
    std::wstring textShares;
    int aggregate = 0;
    for (const auto& leg : trade->openLegs) {
        if (leg->underlying == L"SHARES") {
            textShares = L"SHARES";
            aggregate = aggregate + leg->openQuantity;
        }
        else if (leg->underlying == L"FUTURES") {
            textShares = L"FUTURES";
            aggregate = aggregate + leg->openQuantity;
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
        if (textShares == L"FUTURES") ld->lineType = LineType::Futures;

        ld->SetData(col, trade, tickerId, textShares, StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYMEDIUM,
            COLOR_WHITEDARK, font8, FontStyleRegular);
        col++;

        ld->SetData(col, trade, tickerId, L"", StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYMEDIUM,
            COLOR_WHITEDARK, font8, FontStyleRegular);
        col++;

        ld->SetData(col, trade, tickerId, L"", StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYMEDIUM,
            COLOR_WHITEDARK, font8, FontStyleRegular);
        col++;

        text = AfxMoney(std::abs(trade->ACB / aggregate));
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
    for (const auto& leg : trade->openLegs) {
        if (leg->underlying == L"OPTIONS") {
            ld = new ListBoxData;

            ld->leg = leg;
            ld->lineType = LineType::OptionsLeg;

            std::wstring currentDate = AfxCurrentDate();
            std::wstring expiryDate = leg->expiryDate;
            std::wstring wszShortDate = AfxShortDate(expiryDate);
            std::wstring wszDTE;


            // If the ExpiryDate is 2 days or less then display in Yellow, otherwise Magenta.
            int DTE = AfxDaysBetween(currentDate, expiryDate);
            wszDTE = std::to_wstring(DTE) + L"d";

            Yellow = COLOR_MAGENTA;
            if (DTE < 3) {
                Yellow = COLOR_YELLOW;
            }

            // If the expiry year is greater than current year + 1 then add
            // the year to the display string. Useful for LEAP options.
            if (AfxGetYear(expiryDate) > AfxGetYear(currentDate) + 1) {
                wszShortDate.append(L"/");
                wszShortDate.append(std::to_wstring(AfxGetYear(expiryDate)));
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

            ld->SetData(col, trade, tickerId, std::to_wstring(leg->openQuantity), StringAlignmentFar, StringAlignmentCenter,
                COLOR_GRAYMEDIUM, COLOR_WHITELIGHT, font8, FontStyleRegular);  // position quantity
            col++;

            ld->SetData(col, trade, tickerId, wszShortDate, StringAlignmentCenter, StringAlignmentCenter,
                COLOR_GRAYLIGHT, COLOR_WHITELIGHT, font8, FontStyleRegular);   // expiry date
            col++;

            ld->SetData(col, trade, tickerId, wszDTE, StringAlignmentCenter, StringAlignmentCenter, COLOR_GRAYMEDIUM,
                COLOR_WHITEDARK, font8, FontStyleRegular);   // DTE
            col++;

            ld->SetData(col, trade, tickerId, leg->strikePrice, StringAlignmentCenter, StringAlignmentCenter,
                COLOR_GRAYLIGHT, COLOR_WHITELIGHT, font8, FontStyleRegular);   // strike price
            col++;

            ld->SetData(col, trade, tickerId, L"  " + leg->PutCall, StringAlignmentNear, StringAlignmentCenter, 
                COLOR_GRAYMEDIUM, COLOR_WHITEDARK, font8, FontStyleRegular);   // PutCall

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

    text = trans->transDate;
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
// Create the display data for a History SHARES leg.
// ========================================================================================
void ListBoxData_HistorySharesLeg(
    HWND hListBox, const std::shared_ptr<Trade>& trade, const std::shared_ptr<Transaction>& trans, const std::shared_ptr<Leg>& leg)
{
    ListBoxData* ld = new ListBoxData;

    TickerId tickerId = -1;
    REAL font8 = 8;

    ld->SetData(2, trade, tickerId, trans->underlying, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYMEDIUM, COLOR_WHITEDARK, font8, FontStyleRegular);

    for (int i = 3; i < 7; i++) {
        ld->SetData(i, trade, tickerId, L"", StringAlignmentNear, StringAlignmentCenter,
            COLOR_GRAYMEDIUM, COLOR_WHITEDARK, font8, FontStyleRegular);
    }
    
    ld->SetData(7, trade, tickerId, std::to_wstring(leg->openQuantity), StringAlignmentFar, StringAlignmentCenter,
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

    std::wstring text = std::to_wstring(leg->origQuantity);
    ld->SetData(2, trade, tickerId, text, StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYMEDIUM, COLOR_WHITELIGHT, font8, FontStyleRegular);


    int DTE = AfxDaysBetween(trans->transDate, leg->expiryDate);
    std::wstring wszDays = std::to_wstring(DTE) + L"d";
    std::wstring wszShortDate = AfxShortDate(leg->expiryDate);

    // If the expiry year is greater than current year + 1 then add
    // the year to the display string. Useful for LEAP options.
    if (AfxGetYear(leg->expiryDate) > AfxGetYear(trans->transDate) + 1) {
        wszShortDate.append(L"/");
        wszShortDate.append(std::to_wstring(AfxGetYear(leg->expiryDate)));
    }

    ld->SetData(3, trade, tickerId, wszShortDate, StringAlignmentCenter, StringAlignmentCenter,
        COLOR_GRAYLIGHT, COLOR_WHITELIGHT, font8, FontStyleRegular);

    ld->SetData(4, trade, tickerId, wszDays, StringAlignmentCenter, StringAlignmentCenter,
        COLOR_GRAYMEDIUM, COLOR_WHITEDARK, font8, FontStyleRegular);

    ld->SetData(5, trade, tickerId, leg->strikePrice, StringAlignmentCenter, StringAlignmentCenter,
        COLOR_GRAYLIGHT, COLOR_WHITELIGHT, font8, FontStyleRegular);

    ld->SetData(6, trade, tickerId, L" " + leg->PutCall, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYMEDIUM, COLOR_WHITEDARK, font8, FontStyleRegular);

    DWORD clr = COLOR_RED;
    if (leg->action == L"BTO" || leg->action == L"BTC") clr = COLOR_GREEN;

    ld->SetData(7, trade, tickerId, leg->action, StringAlignmentCenter, StringAlignmentCenter,
        COLOR_GRAYLIGHT, clr, font8, FontStyleRegular);

    ListBox_AddString(hListBox, ld);
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

    ld->SetData(2, trade, tickerId, trade->tickerSymbol, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_WHITELIGHT, font8, FontStyleRegular);

    ld->SetData(3, trade, tickerId, trade->tickerName, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_WHITELIGHT, font8, FontStyleRegular);

    DWORD clr = (trade->ACB >= 0) ? COLOR_GREEN : COLOR_RED;
    ld->SetData(4, trade, tickerId, AfxMoney(trade->ACB), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    ListBox_AddString(hListBox, ld);
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

    ld->SetData(1, trade, tickerId, trans->transDate, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_WHITELIGHT, font8, FontStyleRegular);

    ld->SetData(2, trade, tickerId, trade->tickerSymbol, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_ORANGE, font8, FontStyleRegular);

    ld->SetData(3, trade, tickerId, trans->description, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_WHITELIGHT, font8, FontStyleRegular);

    ld->SetData(4, trade, tickerId, AfxMoney(trans->quantity), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_WHITELIGHT, font8, FontStyleRegular);

    ld->SetData(5, trade, tickerId, AfxMoney(trans->price, false, GetTickerDecimals(trade->tickerSymbol)), 
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
         [&](const auto t) { return (t->tickerSymbol == ticker && t->tickerName.length()); });

    if (iter != trades.end()) {
        auto index = std::distance(trades.begin(), iter);
        ld->SetData(2, nullptr, tickerId, trades.at(index)->tickerName, StringAlignmentNear, StringAlignmentCenter,
            COLOR_GRAYDARK, COLOR_ORANGE, font8, FontStyleRegular);
    }

    DWORD clr = (amount >= 0) ? COLOR_GREEN : COLOR_RED;
    ld->SetData(3, nullptr, tickerId, AfxMoney(amount), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    ListBox_AddString(hListBox, ld);
}


// ========================================================================================
// Create the display data line for a daily total node header line.
// ========================================================================================
void ListBoxData_OutputDailyTotalsNodeHeader(HWND hListBox, std::wstring date, double amount, bool isOpen)
{
    ListBoxData* ld = new ListBoxData;

    TickerId tickerId = -1;
    REAL font8 = 8;
    REAL font9 = 9;

    ld->isDailyTotalsNode = true;
    ld->isDailyTotalsNodeOpen = isOpen;
    ld->DailyTotalsDate = date;

    // Triangle open/closed
    ld->SetData(0, nullptr, tickerId, (isOpen ? GLYPH_TREEOPEN : GLYPH_TREECLOSED), StringAlignmentCenter, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_WHITEDARK, font8, FontStyleRegular);

    std::wstring wszText = date;
    ld->SetData(1, nullptr, tickerId, wszText, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_ORANGE, font8, FontStyleRegular);

    wszText = AfxGetShortDayName(date);
    ld->SetData(2, nullptr, tickerId, wszText, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_ORANGE, font8, FontStyleRegular);

    DWORD clr = (amount >= 0) ? COLOR_GREEN : COLOR_RED;
    ld->SetData(3, nullptr, tickerId, AfxMoney(amount), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    ListBox_AddString(hListBox, ld);
}


// ========================================================================================
// Create the display data line for a daily total detail line.
// ========================================================================================
void ListBoxData_OutputDailyTotalsDetailLine(
    HWND hListBox, const std::shared_ptr<Trade>& trade, const std::shared_ptr<Transaction>& trans)
{
    ListBoxData* ld = new ListBoxData;

    TickerId tickerId = -1;
    REAL font8 = 8;
    REAL font9 = 9;

    ld->SetData(0, nullptr, tickerId, L"", StringAlignmentCenter, StringAlignmentCenter, 
        COLOR_GRAYDARK, COLOR_WHITEDARK, font8, FontStyleRegular);

    ld->SetData(1, nullptr, tickerId, trade->tickerSymbol, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_WHITEDARK, font8, FontStyleRegular);

    ld->SetData(2, nullptr, tickerId, trans->description, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_WHITEDARK, font8, FontStyleRegular);

    ld->SetData(3, nullptr, tickerId, AfxMoney(trans->total), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_WHITEDARK, font8, FontStyleRegular);

    ListBox_AddString(hListBox, ld);
}


// ========================================================================================
// Create the display data line for a daily total SUMMARY lines.
// ========================================================================================
void ListBoxData_OutputDailyTotalsSummary(HWND hListBox, double grandTotal, double MTD, double YTD)
{
    ListBoxData* ld = new ListBoxData;

    TickerId tickerId = -1;
    REAL font8 = 8;
    REAL font9 = 9;


    // Populate the summary data
    ld = new ListBoxData;
    
    double stockValue = 0;
    for (const auto& trade : trades) {
        if (!trade->isOpen) continue;
        for (const auto& leg : trade->openLegs) {
            if (leg->underlying == L"SHARES") {
                stockValue = stockValue + (leg->openQuantity * trade->tickerLastPrice);
            }
        }
    }

    DWORD clr = (grandTotal >= 0) ? COLOR_GREEN : COLOR_RED;
    ld->SetData(0, nullptr, tickerId, AfxMoney(grandTotal), StringAlignmentCenter, StringAlignmentCenter, 
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    clr = (stockValue >= 0) ? COLOR_GREEN : COLOR_RED;
    ld->SetData(1, nullptr, tickerId, AfxMoney(stockValue), StringAlignmentCenter, StringAlignmentCenter, 
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    double netValue = grandTotal + stockValue;
    clr = (netValue >= 0) ? COLOR_GREEN : COLOR_RED;
    ld->SetData(2, nullptr, tickerId, AfxMoney(netValue), StringAlignmentCenter, StringAlignmentCenter, 
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    clr = (MTD >= 0) ? COLOR_GREEN : COLOR_RED;
    ld->SetData(3, nullptr, tickerId, AfxMoney(MTD), StringAlignmentCenter, StringAlignmentCenter, 
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    clr = (YTD >= 0) ? COLOR_GREEN : COLOR_RED;
    ld->SetData(4, nullptr, tickerId, AfxMoney(YTD), StringAlignmentCenter, StringAlignmentCenter, 
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
    SolidBrush backBrush(COLOR_GRAYMEDIUM);
    graphics.FillRectangle(&backBrush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);


    graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

    std::wstring wszFontName = AfxGetDefaultFont();
    FontFamily fontFamily(wszFontName.c_str());
    Font       font(&fontFamily, 9, FontStyleRegular, Unit::UnitPoint);
    SolidBrush textBrush(COLOR_WHITELIGHT);

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
        std::wstring wszText = Header_GetItemText(hWnd, i);
        graphics.DrawString(wszText.c_str(), -1, &font, rcText, &stringF, &textBrush);

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
        std::wstring wszText;

        DWORD nBackColor = (bIsHot) ? COLOR_SELECTION : COLOR_GRAYDARK;
        DWORD nBackColorHot = COLOR_SELECTION;
        DWORD nTextColor = COLOR_WHITELIGHT;

        std::wstring wszFontName = AfxGetDefaultFont();
        FontFamily   fontFamily(wszFontName.c_str());
        REAL fontSize = 10;
        int fontStyle = FontStyleRegular;

        StringAlignment HAlignment = StringAlignmentNear;
        StringAlignment VAlignment = StringAlignmentCenter;

        // Paint the full width background using brush 
        SolidBrush backBrush(nBackColor);
        graphics.FillRectangle(&backBrush, 0, 0, nWidth, nHeight);

        // Get the current ListBox line data should a valid line exist
        // Paint the individual columns with the specific data.
        ListBoxData* ld = (ListBoxData*)(lpDrawItem->itemData);
        int nLeft = 0;
        int colWidth = 0;
        int colStart = 0;
        int colEnd = 10;

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
            wszText = ld->col[i].wszText;

            HAlignment = ld->col[i].HAlignment;
            VAlignment = ld->col[i].VAlignment;
            nBackColor = (bIsHot) ? nBackColor : ld->col[i].backTheme;
            nTextColor = ld->col[i].textTheme;
            fontSize = ld->col[i].fontSize;
            fontStyle = ld->col[i].fontStyle;

            colWidth = (ld->lineType == LineType::CategoryHeader) ? nWidth : AfxScaleX((float)ld->col[i].colWidth);

            backBrush.SetColor(nBackColor);
            graphics.FillRectangle(&backBrush, nLeft, 0, colWidth, nHeight);

            Font         font(&fontFamily, fontSize, fontStyle, Unit::UnitPoint);
            SolidBrush   textBrush(nTextColor);
            StringFormat stringF(StringFormatFlagsNoWrap);
            stringF.SetAlignment(HAlignment);
            stringF.SetLineAlignment(VAlignment);

            // If right alignment then add a very small amount of right side
            // padding so that text is not pushed up right against the right side.
            int rightPadding = 0;
            if (HAlignment == StringAlignmentFar) rightPadding = AfxScaleX(2);

            RectF rcText((REAL)nLeft, (REAL)0, (REAL)colWidth - rightPadding, (REAL)nHeight);
            graphics.DrawString(wszText.c_str(), -1, &font, rcText, &stringF, &textBrush);

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

