#include "pch.h"
#include "ListBoxData.h"
#include "..\Utilities\AfxWin.h"
#include "..\Database\trade.h"
#include "..\Themes\Themes.h"
#include "..\MainWindow\tws-client.h"
#include "..\TradesPanel\TradesPanel.h"
#include "..\HistoryPanel\HistoryPanel.h"


int nHistoryMinColWidth[8] =
{
    25,     /* dropdown arrow*/
    45,     /* ticker symbol */
    50,     /* ITM */
    50,     /* position quantity */
    50,     /* expiry date */
    40,     /* DTE */
    45,     /* strike price */
    40      /* put/call */
};

int nHistoryColWidth[8] = { 0,0,0,0,0,0,0 };

int nMinColWidth[8] =
{
    25,     /* dropdown arrow*/
    45,     /* ticker symbol */
    50,     /* ITM */
    50,     /* position quantity */
    50,     /* expiry date */
    40,     /* DTE */
    45,     /* strike price */
    40      /* put/call */
};

int nColWidth[8] = { 0,0,0,0,0,0,0 };


// ========================================================================================
// Calculate the actual column widths based on the size of the strings in
// ListBoxData while respecting the minimum values as defined in nMinColWidth[].
// This function is also called when receiving new price data from TWS because
// that data may need the column width to be wider.
// ========================================================================================
void ListBoxData_ResizeColumnWidths(HWND hListBox, int nIndex)
{
    HDC hdc = GetDC(hListBox);

    Graphics graphics(hdc);
    graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

    std::wstring wszFontName = AfxGetDefaultFont();
    FontFamily fontFamily(wszFontName.c_str());
    REAL fontSize = 10;
    int fontStyle = FontStyleRegular;
    RectF boundRect;
    RectF layoutRect(0.0f, 0.0f, 100.0f, 50.0f);
    StringFormat format;
    format.SetAlignment(StringAlignmentFar);

    bool bRedrawListBox = false;

    int nEnd = ListBox_GetCount(hListBox) - 1;
    int nStart = 0;
    // If a specific line number was passed into this function then we only
    // test for that line rather than all lines (like when the arrays are first loaded.
    // A value of -1 will iterate all strings the columns.
    if (nIndex != -1) {
        nStart = nIndex; nEnd = nIndex;
    }

    for (int nIndex = nStart; nIndex = nEnd; nIndex++) {
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, nIndex);
        for (int i = 0; i < 8; i++) {
            fontSize = ld->col[i].fontSize;
            fontStyle = ld->col[i].fontStyle;
            Font font(&fontFamily, fontSize, fontStyle, Unit::UnitPoint);
            graphics.MeasureString(ld->col[i].wszText.c_str(), ld->col[i].wszText.length(),
                &font, layoutRect, &format, &boundRect);

            nHistoryColWidth[i] = max(nHistoryColWidth[i], nHistoryMinColWidth[i]);
            int textLength = AfxUnScaleX(boundRect.Width) + 10;  // add a bit for padding
            if (textLength > nHistoryColWidth[i]) {
                nHistoryColWidth[i] = textLength;
                bRedrawListBox = true;
            }
        }
        if (nIndex != -1) break;
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

    if (isHistory) {
        tickerId = -1;
        std::wstring text = (trade->isOpen ? L"Open Pos" : L"Closed Pos");
        ld->SetData(0, trade, tickerId, text, StringAlignmentNear, ThemeElement::TradesPanelBack,
            ThemeElement::TradesPanelText, 8, FontStyleRegular);   // orange

        //text = QString::number(std::abs(trade->ACB), 'f', 2);
        text = L"999.99";
        ld->SetData(6, trade, tickerId, text, StringAlignmentNear, ThemeElement::TradesPanelBack,
            ThemeElement::TradesPanelText, 8, FontStyleRegular);   // orange

    }
    else {
        ld->SetData(0, trade, tickerId, L"\u23F7", StringAlignmentCenter, ThemeElement::TradesPanelBack,
            ThemeElement::TradesPanelTextDim, 8, FontStyleRegular);
        ld->SetData(1, trade, tickerId, trade->tickerSymbol, StringAlignmentNear, ThemeElement::TradesPanelBack,
            ThemeElement::TradesPanelText, 9, FontStyleRegular | FontStyleBold);
        // Col 1 to 6 are set based on incoming TWS price data 
        ld->SetData(COLUMN_TICKER_ITM, trade, tickerId, L"", StringAlignmentNear, ThemeElement::TradesPanelBack,
            ThemeElement::TradesPanelText, 8, FontStyleRegular);   // ITM
        ld->SetData(3, trade, tickerId, L"", StringAlignmentNear, ThemeElement::TradesPanelBack,
            ThemeElement::TradesPanelText, 8, FontStyleRegular);
        ld->SetData(4, trade, tickerId, L"", StringAlignmentNear, ThemeElement::TradesPanelBack,
            ThemeElement::TradesPanelText, 8, FontStyleRegular);
        ld->SetData(COLUMN_TICKER_CHANGE, trade, tickerId, L"", StringAlignmentFar, ThemeElement::TradesPanelBack,
            ThemeElement::TradesPanelTextDim, 8, FontStyleRegular);   // price change
        ld->SetData(COLUMN_TICKER_CURRENTPRICE, trade, tickerId, L"0.00", StringAlignmentCenter, ThemeElement::TradesPanelBack,
            ThemeElement::TradesPanelText, 9, FontStyleRegular | FontStyleBold);   // current price
        ld->SetData(COLUMN_TICKER_PERCENTAGE, trade, tickerId, L"", StringAlignmentNear, ThemeElement::TradesPanelBack,
            ThemeElement::TradesPanelTextDim, 8, FontStyleRegular);   // price percentage change

        std::cout << ld->tickerId << " " << tickerId << std::endl;

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
            ThemeElement::TradesPanelText, 8, FontStyleRegular);

        int col = 1;
        if (!isHistory) {
            ld->SetData(col, trade, tickerId, wszDot, StringAlignmentFar, ThemeElement::TradesPanelBack,
                ThemeElement::TradesPanelNormalDTE, 7, FontStyleRegular);
            col += 2;
        }

        ld->SetData(col, trade, tickerId, textShares, StringAlignmentNear, ThemeElement::TradesPanelColBackDark,
            ThemeElement::TradesPanelTextDim, 8, FontStyleRegular);
        col++;

        ld->SetData(col, trade, tickerId, L"", StringAlignmentNear, ThemeElement::TradesPanelColBackDark,
            ThemeElement::TradesPanelTextDim, 8, FontStyleRegular);
        col++;

        ld->SetData(col, trade, tickerId, L"", StringAlignmentNear, ThemeElement::TradesPanelColBackDark,
            ThemeElement::TradesPanelTextDim, 8, FontStyleRegular);
        col++;

        ld->SetData(col, trade, tickerId, L"11.58", StringAlignmentCenter, ThemeElement::TradesPanelColBackDark,
            ThemeElement::TradesPanelTextDim, 8, FontStyleRegular);
        col++;

        ld->SetData(col, trade, tickerId, L"200", StringAlignmentCenter, ThemeElement::TradesPanelColBackDark,
            ThemeElement::TradesPanelTextDim, 8, FontStyleRegular);

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
                    WarningDTE, 7, FontStyleRegular);
                col++;
            }

            ld->SetData(col, trade, tickerId, L"", StringAlignmentNear, ThemeElement::TradesPanelBack,
                ThemeElement::TradesPanelText, 8, FontStyleRegular);  // empty column
            col++;

            ld->SetData(col, trade, tickerId, std::to_wstring(leg->openQuantity), StringAlignmentFar, ThemeElement::TradesPanelColBackDark, ThemeElement::TradesPanelText, 8, FontStyleRegular);  // position quantity
            col++;

            ld->SetData(col, trade, tickerId, wszShortDate, StringAlignmentCenter, ThemeElement::TradesPanelColBackLight,
                ThemeElement::TradesPanelText, 8, FontStyleRegular);   // expiry date
            col++;

            ld->SetData(col, trade, tickerId, wszDTE, StringAlignmentCenter, ThemeElement::TradesPanelColBackDark,
                ThemeElement::TradesPanelTextDim, 8, FontStyleRegular);   // DTE
            col++;

            ld->SetData(col, trade, tickerId, leg->strikePrice, StringAlignmentCenter, ThemeElement::TradesPanelColBackLight,
                ThemeElement::TradesPanelText, 8, FontStyleRegular);   // strike price
            col++;

            ld->SetData(col, trade, tickerId, L"  " + leg->PutCall, StringAlignmentNear, ThemeElement::TradesPanelColBackDark,
                ThemeElement::TradesPanelTextDim, 8, FontStyleRegular);   // PutCall

            ListBox_AddString(hListBox, ld);
        }
    }


    // *** BLANK SEPARATION LINE ***
    ld = new ListBoxData;
    ListBox_AddString(hListBox, ld);

}


void ListBoxData_TradesHistoryHeader(HWND hListBox, Trade* trade, Transaction* trans)
{
    // Display the transaction description, date, and total prior to showing the detail lines
    ListBoxData* ld = new ListBoxData;
    std::wstring text;

    TickerId tickerId = -1;

    text = trans->description;
    ld->SetData(0, trade, tickerId, text, StringAlignmentNear, ThemeElement::TradesPanelBack,
        ThemeElement::TradesPanelText, 8, FontStyleRegular);   // orange

    text = trans->transDate;
    ld->SetData(1, trade, tickerId, text, StringAlignmentNear, ThemeElement::TradesPanelBack,
        ThemeElement::TradesPanelText, 8, FontStyleRegular);   

    text = L"123.45";  // QString::number(std::abs(trans->total), 'f', 2);
    ld->SetData(6, trade, tickerId, text, StringAlignmentFar, ThemeElement::TradesPanelBack,
        ThemeElement::TradesPanelText, 8, FontStyleRegular);   // green/red

    ListBox_AddString(hListBox, ld);

}


void ListBoxData_SharesLeg(HWND hListBox, Trade* trade, Transaction* trans, Leg* leg)
{
    ListBoxData* ld = new ListBoxData;
    std::wstring text;
    TickerId tickerId = -1;

    text = trans->underlying;
    ld->SetData(1, trade, tickerId, text, StringAlignmentNear,
        ThemeElement::TradesPanelBack, ThemeElement::TradesPanelText, 8, FontStyleRegular);

    text = L"1000";   //  QString::number(leg->openQuantity);
    ld->SetData(6, trade, tickerId, text, StringAlignmentFar,
        ThemeElement::TradesPanelBack, ThemeElement::TradesPanelText, 8, FontStyleRegular);

    ListBox_AddString(hListBox, ld);
}


void ListBoxData_OptionsLeg(HWND hListBox, Trade* trade, Transaction* trans, Leg* leg)
{
    ListBoxData* ld = new ListBoxData;
    std::wstring text;
    TickerId tickerId = -1;

    text = L"-1";   // QString::number(leg->origQuantity);
    ld->SetData(1, trade, tickerId, text, StringAlignmentFar,
        ThemeElement::TradesPanelBack, ThemeElement::TradesPanelText, 8, FontStyleRegular);

    /*
        QDate expiryDate = QDate::fromString(leg->expiryDate, "yyyy-MM-dd");
        QString strShortDate = expiryDate.toString("MMM dd");

        // If the expiry year is greater than current year + 1 then add
        // the year to the display string. Useful for LEAP options.
        if (expiryDate.year() > QDate::currentDate().year() + 1) {
            strShortDate = strShortDate + "/" + QString::number(expiryDate.year());
        }
        QDate transDate = QDate::fromString(trans->transDate, "yyyy-MM-dd");
        QString strDays = QString::number(transDate.daysTo(expiryDate)) + "d";
    */

    text = L"Mar 31";  // strShortDate;
    ld->SetData(2, trade, tickerId, text, StringAlignmentCenter,
        ThemeElement::TradesPanelBack, ThemeElement::TradesPanelText, 8, FontStyleRegular);

    text = L"7";  // strDays;
    ld->SetData(3, trade, tickerId, text, StringAlignmentCenter,
        ThemeElement::TradesPanelBack, ThemeElement::TradesPanelText, 8, FontStyleRegular);

    text = leg->strikePrice;
    ld->SetData(4, trade, tickerId, text, StringAlignmentCenter,
        ThemeElement::TradesPanelBack, ThemeElement::TradesPanelText, 8, FontStyleRegular);

    text = L" " + leg->PutCall;
    ld->SetData(5, trade, tickerId, text, StringAlignmentNear,
        ThemeElement::TradesPanelBack, ThemeElement::TradesPanelText, 8, FontStyleRegular);

    /*
    QColor clr;
    if (leg->action == "BTO" || leg->action == "BTC") { clr = green; }
    if (leg->action == "STO" || leg->action == "STC") { clr = red; }
*/
    text = leg->action;
    ld->SetData(6, trade, tickerId, text, StringAlignmentCenter,
        ThemeElement::TradesPanelBack, ThemeElement::TradesPanelText, 8, FontStyleRegular);

    ListBox_AddString(hListBox, ld);
}


