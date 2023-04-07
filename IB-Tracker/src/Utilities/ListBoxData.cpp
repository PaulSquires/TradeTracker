#include "pch.h"
#include "ListBoxData.h"
#include "..\Utilities\AfxWin.h"
#include "..\Database\trade.h"
#include "..\Themes\Themes.h"
#include "..\MainWindow\tws-client.h"
#include "..\TradesPanel\TradesPanel.h"
#include "..\HistoryPanel\HistoryPanel.h"


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
    0,
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
    0,
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


int nTickerTotalsMinColWidth[10] =
{
    5,     /* empty */
    50,    /* Ticker Symbol */
    70,    /* Ticker Name */
    45,    /* Amount */
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

int nColWidth[10] = { 0,0,0,0,0,0,0,0,0 };


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
    int nStart = 0;

    // If a specific line number was passed into this function then we only
    // test for that line rather than all lines (like when the arrays are first loaded.
    // A value of -1 will iterate all strings the columns.
    if (nIndex != -1) {
        nStart = nIndex; nEnd = nIndex;
    }

    for (int ii = nStart; ii <= nEnd; ii++) {
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, ii);
        for (int i = 0; i < 10; i++) {
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
        for (int i = 0; i < 10; i++) {
            ld->col[i].colWidth = nColWidth[i];
        }
        ListBox_SetItemData(hListBox, ii, ld);
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
            if (ld->isTickerLine) tws_cancelMktData(ld->tickerId);
            delete(ld);
        }
    }

    // Clear the current trades listbox
    ListBox_ResetContent(hListBox);
}



// ========================================================================================
// Create the display data for an Open Position that displays in Trades & History tables.
// ========================================================================================
void ListBoxData_OpenPosition(HWND hListBox, Trade* trade, TickerId tickerId)
{
    ListBoxData* ld = new ListBoxData;
    
    bool isHistory = GetDlgCtrlID(hListBox) == IDC_HISTORY_LISTBOX ? true : false;
    REAL font8 = 8;
    REAL font9 = 9;
    std::wstring text; 

    if (isHistory) {
        tickerId = -1;
        font8 = 8;  // make the History table a little smaller than the Trades table
        font9 = 8;

        ld->SetData(0, trade, tickerId, L"\u23F7", StringAlignmentCenter, ThemeElement::TradesPanelBack,
            ThemeElement::TradesPanelTextDim, font8, FontStyleRegular);

        std::wstring text = (trade->isOpen ? L"Open Pos" : L"Closed Pos");
        ld->SetData(1, trade, tickerId, text, StringAlignmentNear, ThemeElement::TradesPanelBack,
            ThemeElement::TradesPanelHistoryText, font8, FontStyleRegular);   // orange

        text = AfxMoney(std::abs(trade->ACB));
        ThemeElement clr = (trade->ACB >= 0) ? ThemeElement::valuePositive : ThemeElement::valueNegative;
        ld->SetData(7, trade, tickerId, text, StringAlignmentFar, ThemeElement::TradesPanelBack,
            clr, font8, FontStyleRegular);  

    }
    else {
        ld->SetData(0, trade, tickerId, L"\u23F7", StringAlignmentCenter, ThemeElement::TradesPanelBack,
            ThemeElement::TradesPanelTextDim, font8, FontStyleRegular);
        ld->SetData(1, trade, tickerId, trade->tickerSymbol, StringAlignmentNear, ThemeElement::TradesPanelBack,
            ThemeElement::TradesPanelText, font9, FontStyleRegular | FontStyleBold);
        // Col 1 to 6 are set based on incoming TWS price data 
        ld->SetData(COLUMN_TICKER_ITM, trade, tickerId, L"", StringAlignmentNear, ThemeElement::TradesPanelBack,
            ThemeElement::TradesPanelText, font8, FontStyleRegular);   // ITM
        ld->SetData(3, trade, tickerId, L"", StringAlignmentNear, ThemeElement::TradesPanelBack,
            ThemeElement::TradesPanelText, font8, FontStyleRegular);
        ld->SetData(4, trade, tickerId, L"", StringAlignmentNear, ThemeElement::TradesPanelBack,
            ThemeElement::TradesPanelText, font8, FontStyleRegular);
        ld->SetData(COLUMN_TICKER_CHANGE, trade, tickerId, L"", StringAlignmentFar, ThemeElement::TradesPanelBack,
            ThemeElement::TradesPanelTextDim, font8, FontStyleRegular);   // price change
        ld->SetData(COLUMN_TICKER_CURRENTPRICE, trade, tickerId, L"0.00", StringAlignmentCenter, ThemeElement::TradesPanelBack,
            ThemeElement::TradesPanelText, font9, FontStyleRegular | FontStyleBold);   // current price
        ld->SetData(COLUMN_TICKER_PERCENTAGE, trade, tickerId, L"", StringAlignmentNear, ThemeElement::TradesPanelBack,
            ThemeElement::TradesPanelTextDim, font8, FontStyleRegular);   // price percentage change

        tws_requestMktData(ld);
    }
    ListBox_AddString(hListBox, ld);


    // All tickerId will now be -1 because we are no longer dealing with the main isTickerLine.
    tickerId = -1;

    std::wstring wszDot = L"\u23FA";   // dot character
    ThemeElement WarningDTE = ThemeElement::TradesPanelNormalDTE;



    // *** SHARES ***
    // Roll up all of the SHARES or FUTURES transactions and display the aggregate rather than the individual legs.
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

        ld->SetData(0, trade, tickerId, L"", StringAlignmentNear, ThemeElement::TradesPanelBack,
            ThemeElement::TradesPanelText, font8, FontStyleRegular);

        int col = 1;
        if (!isHistory) {
            ld->SetData(col, trade, tickerId, wszDot, StringAlignmentFar, ThemeElement::TradesPanelBack,
                ThemeElement::TradesPanelNormalDTE, font8, FontStyleRegular);
            col++;
        }
        col++;

        ld->SetData(col, trade, tickerId, textShares, StringAlignmentNear, ThemeElement::TradesPanelColBackDark,
            ThemeElement::TradesPanelTextDim, font8, FontStyleRegular);
        col++;

        ld->SetData(col, trade, tickerId, L"", StringAlignmentNear, ThemeElement::TradesPanelColBackDark,
            ThemeElement::TradesPanelTextDim, font8, FontStyleRegular);
        col++;

        ld->SetData(col, trade, tickerId, L"", StringAlignmentNear, ThemeElement::TradesPanelColBackDark,
            ThemeElement::TradesPanelTextDim, font8, FontStyleRegular);
        col++;

        text = AfxMoney(std::abs(trade->ACB / aggregate));
        ld->SetData(col, trade, tickerId, text, StringAlignmentCenter, ThemeElement::TradesPanelColBackDark,
            ThemeElement::TradesPanelTextDim, font8, FontStyleRegular);
        col++;

        text = std::to_wstring(aggregate);
        ld->SetData(col, trade, tickerId, text, StringAlignmentCenter, ThemeElement::TradesPanelColBackDark,
            ThemeElement::TradesPanelTextDim, font8, FontStyleRegular);

        ListBox_AddString(hListBox, ld);
    }


    // *** OPTION LEGS ***
    for (const auto& leg : trade->openLegs) {
        if (leg->underlying == L"OPTIONS") {
            ld = new ListBoxData;

            std::wstring currentDate = AfxCurrentDate();
            std::wstring expiryDate = leg->expiryDate;
            std::wstring wszShortDate = AfxShortDate(expiryDate);
            std::wstring wszDTE;


            // If the ExpiryDate is 2 days or less then display in Yellow, otherwise Magenta.
            int DTE = AfxDaysBetween(currentDate, expiryDate);
            wszDTE = std::to_wstring(DTE) + L"d";

            if (DTE < 3) {
                WarningDTE = ThemeElement::TradesPanelWarningDTE;
            }

            // If the expiry year is greater than current year + 1 then add
            // the year to the display string. Useful for LEAP options.
            if (AfxGetYear(expiryDate) > AfxGetYear(currentDate) + 1) {
                wszShortDate.append(L"/");
                wszShortDate.append(std::to_wstring(AfxGetYear(expiryDate)));
            }


            int col = 1;

            if (!isHistory) {
                ld->SetData(col, trade, tickerId, wszDot, StringAlignmentFar, ThemeElement::TradesPanelBack,
                    WarningDTE, font8, FontStyleRegular);
                col++;

                ld->SetData(col, trade, tickerId, L"", StringAlignmentNear, ThemeElement::TradesPanelBack,
                    ThemeElement::TradesPanelText, font8, FontStyleRegular);  // empty column
            }
            col++;

            ld->SetData(col, trade, tickerId, std::to_wstring(leg->openQuantity), StringAlignmentFar, ThemeElement::TradesPanelColBackDark, ThemeElement::TradesPanelText, font8, FontStyleRegular);  // position quantity
            col++;

            ld->SetData(col, trade, tickerId, wszShortDate, StringAlignmentCenter, ThemeElement::TradesPanelColBackLight,
                ThemeElement::TradesPanelText, font8, FontStyleRegular);   // expiry date
            col++;

            ld->SetData(col, trade, tickerId, wszDTE, StringAlignmentCenter, ThemeElement::TradesPanelColBackDark,
                ThemeElement::TradesPanelTextDim, font8, FontStyleRegular);   // DTE
            col++;

            ld->SetData(col, trade, tickerId, leg->strikePrice, StringAlignmentCenter, ThemeElement::TradesPanelColBackLight,
                ThemeElement::TradesPanelText, font8, FontStyleRegular);   // strike price
            col++;

            ld->SetData(col, trade, tickerId, L"  " + leg->PutCall, StringAlignmentNear, ThemeElement::TradesPanelColBackDark,
                ThemeElement::TradesPanelTextDim, font8, FontStyleRegular);   // PutCall

            ListBox_AddString(hListBox, ld);
        }
    }


    // *** BLANK SEPARATION LINE ***
    ld = new ListBoxData;
    ListBox_AddString(hListBox, ld);

}


// ========================================================================================
// Create the display data for a blank line
// ========================================================================================
void ListBoxData_HistoryBlankLine(HWND hListBox)
{
    // *** BLANK SEPARATION LINE AT END OF HISTORY LIST ***
    ListBoxData* ld = new ListBoxData;
    ListBox_AddString(hListBox, ld);
}


// ========================================================================================
// Create the display data a History Header line.
// ========================================================================================
void ListBoxData_HistoryHeader(HWND hListBox, Trade* trade, Transaction* trans)
{
    // Display the transaction description, date, and total prior to showing the detail lines
    ListBoxData* ld = new ListBoxData;

    std::wstring text;
    REAL font8 = 8;

    TickerId tickerId = -1;

    ld->SetData(0, trade, tickerId, L"\u23F7", StringAlignmentCenter, ThemeElement::TradesPanelBack,
        ThemeElement::TradesPanelTextDim, font8, FontStyleRegular);

    text = trans->description;
    ld->SetData(1, trade, tickerId, text, StringAlignmentNear, ThemeElement::TradesPanelBack,
        ThemeElement::TradesPanelHistoryText, font8, FontStyleRegular);   // orange

    text = trans->transDate;
    ld->SetData(2, trade, tickerId, text, StringAlignmentNear, ThemeElement::TradesPanelBack,
        ThemeElement::TradesPanelText, font8, FontStyleRegular);

    text = AfxMoney(std::abs(trans->total));
    ThemeElement clr = (trans->total >= 0) ? ThemeElement::valuePositive : ThemeElement::valueNegative;
    ld->SetData(7, trade, tickerId, text, StringAlignmentFar, ThemeElement::TradesPanelBack,
        clr, font8, FontStyleRegular);   // green/red

    ListBox_AddString(hListBox, ld);

}


// ========================================================================================
// Create the display data for a History SHARES leg.
// ========================================================================================
void ListBoxData_HistorySharesLeg(HWND hListBox, Trade* trade, Transaction* trans, Leg* leg)
{
    ListBoxData* ld = new ListBoxData;

    TickerId tickerId = -1;
    REAL font8 = 8;

    ld->SetData(2, trade, tickerId, trans->underlying, StringAlignmentNear,
        ThemeElement::TradesPanelColBackDark, ThemeElement::TradesPanelTextDim, font8, FontStyleRegular);

    for (int i = 3; i < 7; i++) {
        ld->SetData(i, trade, tickerId, L"", StringAlignmentNear,
            ThemeElement::TradesPanelColBackDark, ThemeElement::TradesPanelTextDim, font8, FontStyleRegular);
    }
    
    ld->SetData(7, trade, tickerId, std::to_wstring(leg->openQuantity), StringAlignmentFar,
        ThemeElement::TradesPanelColBackDark, ThemeElement::TradesPanelTextDim, font8, FontStyleRegular);

    ListBox_AddString(hListBox, ld);
}


// ========================================================================================
// Create the display data for a History OPTIONS leg.
// ========================================================================================
void ListBoxData_HistoryOptionsLeg(HWND hListBox, Trade* trade, Transaction* trans, Leg* leg)
{
    ListBoxData* ld = new ListBoxData;

    TickerId tickerId = -1;
    REAL font8 = 8;

    std::wstring text = std::to_wstring(leg->origQuantity);
    ld->SetData(2, trade, tickerId, text, StringAlignmentFar,
        ThemeElement::TradesPanelColBackDark, ThemeElement::TradesPanelText, font8, FontStyleRegular);


    int DTE = AfxDaysBetween(trans->transDate, leg->expiryDate);
    std::wstring wszDays = std::to_wstring(DTE) + L"d";
    std::wstring wszShortDate = AfxShortDate(leg->expiryDate);

    // If the expiry year is greater than current year + 1 then add
    // the year to the display string. Useful for LEAP options.
    if (AfxGetYear(leg->expiryDate) > AfxGetYear(trans->transDate) + 1) {
        wszShortDate.append(L"/");
        wszShortDate.append(std::to_wstring(AfxGetYear(leg->expiryDate)));
    }

    ld->SetData(3, trade, tickerId, wszShortDate, StringAlignmentCenter,
        ThemeElement::TradesPanelColBackLight, ThemeElement::TradesPanelText, font8, FontStyleRegular);

    ld->SetData(4, trade, tickerId, wszDays, StringAlignmentCenter,
        ThemeElement::TradesPanelColBackDark, ThemeElement::TradesPanelTextDim, font8, FontStyleRegular);

    ld->SetData(5, trade, tickerId, leg->strikePrice, StringAlignmentCenter,
        ThemeElement::TradesPanelColBackLight, ThemeElement::TradesPanelText, font8, FontStyleRegular);

    ld->SetData(6, trade, tickerId, L" " + leg->PutCall, StringAlignmentNear,
        ThemeElement::TradesPanelColBackDark, ThemeElement::TradesPanelTextDim, font8, FontStyleRegular);

    ThemeElement clr = ThemeElement::valueNegative;
    if (leg->action == L"STO" || leg->action == L"STC") clr = ThemeElement::valuePositive;

    ld->SetData(7, trade, tickerId, leg->action, StringAlignmentCenter,
        ThemeElement::TradesPanelColBackLight, clr, font8, FontStyleRegular);

    ListBox_AddString(hListBox, ld);
}


// ========================================================================================
// Create the display data line for a closed position.
// ========================================================================================
void ListBoxData_OutputClosedPosition(HWND hListBox, Trade* trade, std::wstring closedDate)
{
    ListBoxData* ld = new ListBoxData;

    TickerId tickerId = -1;
    REAL font8 = 8;

    ld->SetData(0, trade, tickerId, L"", StringAlignmentNear,
        ThemeElement::TradesPanelBack, ThemeElement::TradesPanelHistoryText, font8, FontStyleRegular);

    ld->SetData(1, trade, tickerId, closedDate, StringAlignmentNear,
        ThemeElement::TradesPanelBack, ThemeElement::TradesPanelHistoryText, font8, FontStyleRegular);

    ld->SetData(2, trade, tickerId, trade->tickerSymbol, StringAlignmentNear,
        ThemeElement::TradesPanelBack, ThemeElement::TradesPanelText, font8, FontStyleRegular);

    ld->SetData(3, trade, tickerId, trade->tickerName, StringAlignmentNear,
        ThemeElement::TradesPanelBack, ThemeElement::TradesPanelText, font8, FontStyleRegular);

    ThemeElement clr = (trade->ACB >= 0) ? ThemeElement::valuePositive : ThemeElement::valueNegative;
    ld->SetData(4, trade, tickerId, AfxMoney(trade->ACB), StringAlignmentFar,
        ThemeElement::TradesPanelBack, clr, font8, FontStyleRegular);

    ListBox_AddString(hListBox, ld);
}


// ========================================================================================
// Create the display data line for a closed position.
// ========================================================================================
void ListBoxData_OutputTickerTotals(HWND hListBox, std::wstring ticker, double amount)
{
    ListBoxData* ld = new ListBoxData;

    TickerId tickerId = -1;
    REAL font8 = 8;
    REAL font9 = 9;

    ld->SetData(1, nullptr, tickerId, ticker, StringAlignmentNear,
        ThemeElement::TradesPanelBack, ThemeElement::TradesPanelHistoryText, font8, FontStyleRegular);

    // Look up the Company name based on the tickerid
    auto iter = std::find_if(trades.begin(), trades.end(),
         [&](const Trade* t) { return (t->tickerSymbol == ticker && t->tickerName.length()); });

    if (iter != trades.end()) {
        auto index = std::distance(trades.begin(), iter);
        ld->SetData(2, nullptr, tickerId, trades.at(index)->tickerName, StringAlignmentNear,
            ThemeElement::TradesPanelBack, ThemeElement::TradesPanelHistoryText, font8, FontStyleRegular);
    }

    ThemeElement clr = (amount >= 0) ? ThemeElement::valuePositive : ThemeElement::valueNegative;
    ld->SetData(3, nullptr, tickerId, AfxMoney(amount), StringAlignmentFar,
        ThemeElement::TradesPanelBack, clr, font8, FontStyleRegular);

    ListBox_AddString(hListBox, ld);
}


// ========================================================================================
// Create the display data line for a daily total node header line.
// ========================================================================================
void ListBoxData_OutputDailyTotalsNodeHeader(HWND hListBox, std::wstring date, double amount)
{
    ListBoxData* ld = new ListBoxData;

    TickerId tickerId = -1;
    REAL font8 = 8;
    REAL font9 = 9;


    ld->SetData(0, nullptr, tickerId, L"\u23F7", StringAlignmentCenter, ThemeElement::TradesPanelBack,
        ThemeElement::TradesPanelTextDim, font8, FontStyleRegular);

    std::wstring wszText = date;
    ld->SetData(1, nullptr, tickerId, wszText, StringAlignmentNear,
        ThemeElement::TradesPanelBack, ThemeElement::TradesPanelHistoryText, font8, FontStyleRegular);

    wszText = AfxGetShortDayName(date);
    ld->SetData(2, nullptr, tickerId, wszText, StringAlignmentNear,
        ThemeElement::TradesPanelBack, ThemeElement::TradesPanelHistoryText, font8, FontStyleRegular);

    ThemeElement clr = (amount >= 0) ? ThemeElement::valuePositive : ThemeElement::valueNegative;
    ld->SetData(3, nullptr, tickerId, AfxMoney(amount), StringAlignmentFar,
        ThemeElement::TradesPanelBack, clr, font8, FontStyleRegular);

    ListBox_AddString(hListBox, ld);
}


// ========================================================================================
// Create the display data line for a daily total detail line.
// ========================================================================================
void ListBoxData_OutputDailyTotalsDetailLine(HWND hListBox, Trade* trade, Transaction* trans)
{
    ListBoxData* ld = new ListBoxData;

    TickerId tickerId = -1;
    REAL font8 = 8;
    REAL font9 = 9;

    ld->SetData(0, nullptr, tickerId, L"", StringAlignmentCenter, ThemeElement::TradesPanelBack,
        ThemeElement::TradesPanelTextDim, font8, FontStyleRegular);

    ld->SetData(1, nullptr, tickerId, trade->tickerSymbol, StringAlignmentNear,
        ThemeElement::TradesPanelBack, ThemeElement::TradesPanelTextDim, font8, FontStyleRegular);

    ld->SetData(2, nullptr, tickerId, trans->description, StringAlignmentNear,
        ThemeElement::TradesPanelBack, ThemeElement::TradesPanelTextDim, font8, FontStyleRegular);

    ld->SetData(3, nullptr, tickerId, AfxMoney(trans->total), StringAlignmentFar,
        ThemeElement::TradesPanelBack, ThemeElement::TradesPanelTextDim, font8, FontStyleRegular);

    ListBox_AddString(hListBox, ld);
}

