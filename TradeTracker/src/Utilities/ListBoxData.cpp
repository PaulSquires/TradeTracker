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
#include "Config/Config.h"
#include "Utilities/Colors.h"


bool previous_market_data_loaded = false;


// ========================================================================================
// Calculate the actual column widths based on the size of the strings in
// ListBoxData while respecting the minimum values as defined in nMinColWidth[].
// This function is also called when receiving new price data from TWS because
// that data may need the column width to be wider.
// Returns bool to indicate whether ListBox should be redrawn.
// ========================================================================================
bool ListBoxData_ResizeColumnWidths(HWND hListBox, TableType tabletype)
{
    int listbox_end = (int)ListBox_GetCount(hListBox);
    if (listbox_end == 0) return false;
    int listbox_start = 0;

    int nColWidth[MAX_COLUMNS] = { 0,0,0,0,0,0,0,0,0,0,0 };

    // Initialize the nColWidth array based on the incoming ListBox
    for (int i = 0; i < MAX_COLUMNS; i++) {
        switch (tabletype)
        {
        case TableType::active_trades:
            nColWidth[i] = nTradesMinColWidth[i];
            break;

        case TableType::closed_trades:
            nColWidth[i] = nClosedMinColWidth[i];
            break;

        case TableType::trade_history:
            nColWidth[i] = nHistoryMinColWidth[i];
            break;

        case TableType::ticker_totals:
            nColWidth[i] = nTickerTotalsMinColWidth[i];
            break;

        case TableType::trans_panel:
            nColWidth[i] = nTransMinColWidth[i];
            break;
            
        }
    }


    HDC hdc = GetDC(hListBox);

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

    int nColMaxTextLength[MAX_COLUMNS] = { 0,0,0,0,0,0,0,0,0,0,0 };

    for (int ii = listbox_start; ii < listbox_end; ++ii) {
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, ii);
        if (ld == nullptr) continue;
        if (ld == (void*)-1) continue;
        if (ld->line_type == LineType::category_header) continue;

        for (int i = 0; i < MAX_COLUMNS; i++) {
            if (nColWidth[i] == 0) continue;

            // The MeasureString is an expensive call that we need to try to minimize
            // it in this loop and because this function gets called often. We check the 
            // textlength of the current line/col and compare it to the current maximum
            // value of that column and only perform the calculations if the current 
            // length is greater than the stored value.
            // This optimization cuts the execution time from 50ms to 2ms.
            if (ld->col[i].text.length() <= nColMaxTextLength[i]) {
                continue;
            }
            nColMaxTextLength[i] = (int)ld->col[i].text.length();


            // If we get this far then the text length of the 
            fontSize = ld->col[i].font_size;
            fontStyle = ld->col[i].font_style;
            Font font(&fontFamily, fontSize, fontStyle, Unit::UnitPoint);

            graphics.MeasureString(ld->col[i].text.c_str(), (int)ld->col[i].text.length(),
                &font, layoutRect, &format, &boundRect);

            int textLength = AfxUnScaleX(boundRect.Width) + 5;  // add a bit for padding

                
            if (textLength > nColWidth[i]) {
                
                nColWidth[i] = textLength;

                if (tabletype == TableType::trade_history) {
                    nColWidth[i] = min(nColWidth[i], nHistoryMaxColWidth[i]);
                }

                if (tabletype == TableType::ticker_totals) {
                    nColWidth[i] = min(nColWidth[i], nTickerTotalsMaxColWidth[i]);
                }

                bRedrawListBox = true;
            }
        }
            
    }


    // Update the newly calculated column widths into each of the ld structures
    for (int ii = listbox_start; ii < listbox_end; ++ii) {
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, ii);
        if (ld == nullptr) continue;
        if (ld == (void*)-1) continue;
        for (int i = 0; i < MAX_COLUMNS; i++) {
            ld->col[i].column_width = nColWidth[i];
        }
        ListBox_SetItemData(hListBox, ii, ld);
    }

    // Update the widths of any associated Header control. 
    HWND hHeader = NULL;
    if (tabletype == TableType::closed_trades) hHeader = GetDlgItem(HWND_CLOSEDTRADES, IDC_CLOSED_HEADER);
    if (tabletype == TableType::ticker_totals) hHeader = GetDlgItem(HWND_TICKERPANEL, IDC_TICKER_HEADER_TOTALS);
    if (tabletype == TableType::trans_panel) hHeader = GetDlgItem(HWND_TRANSPANEL, IDC_TRANS_HEADER);
    
    if (hHeader) {
        int item_width = 0;
        for (int i = 0; i < MAX_COLUMNS; i++) {
            // Do a width change check in order to prevent redrawing the header and causing flash.
            item_width = AfxScaleX(nColWidth[i]);
            if (item_width != Header_GetItemWidth(hHeader, i)) {
                Header_SetItemWidth(hHeader, i, item_width);
            }
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
    // delete any previously manually allocated ListBoxData structures.
    int item_count = ListBox_GetCount(hListBox);

    for (int i = 0; i < item_count; ++i) {
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, i);
        if (ld != nullptr) {
            delete(ld);
            ld = nullptr;
        }
    }

    // Clear the current trades listbox
    ListBox_ResetContent(hListBox);
}


// ========================================================================================
// Loop through the ListBox data and request market data for tickers
// ========================================================================================
void ListBoxData_RequestMarketData(HWND hListBox)
{
    // If no trades exist then simply exit
    if (trades.size() == 0) return;
    if (!tws_IsConnected()) return;
    
    // TWS API has a pacing limitation of 50 messages per second. As such, a total of 
    // 50 messages may be sent AND received within each one second span.
    
    // Request market data for each open trade
    int item_count = ListBox_GetCount(hListBox);
    if (item_count == 0) return;
    for (int i = 0; i < item_count; i++) {
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, i);
        if (ld != nullptr) {
            if (ld->line_type == LineType::ticker_line) {
                tws_RequestMarketData(ld);
            }
        }
    }
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

    ld->line_type = LineType::category_header;
    ListBox_AddString(hListBox, ld);
}


// ========================================================================================
// Add a simple No Trades Exist message
// ========================================================================================
void ListBoxData_NoTradesExistMessage(HWND hListBox)
{
    ListBoxData* ld = new ListBoxData;
    std::wstring text = L"No locally created Trades exist.";
    ld->line_type = LineType::category_header;
    ld->SetData(0, nullptr, -1, text, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_BLUE, 9, FontStyleRegular);
    ListBox_AddString(hListBox, ld);

    ld = new ListBoxData;
    text = L"Create Trades manually and TradeTracker will retrieve data from IBKR TWS.";
    ld->line_type = LineType::category_header;
    ld->SetData(0, nullptr, -1, text, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_BLUE, 9, FontStyleRegular);
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

    std::wstring start_date = AfxInsertDateHyphens(trade->bp_start_date);
    std::wstring end_date = AfxInsertDateHyphens(trade->bp_end_date);

    // Buying Power
    text = AfxMoney(trade->trade_bp, true, 0);
    ld->SetData(2, trade, tickerId, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
        COLOR_WHITELIGHT, font8, FontStyleRegular);
    text = L"BP";
    ld->SetData(3, trade, tickerId, text, StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
        COLOR_WHITEDARK, font8, FontStyleRegular);
    
    // Days In Trade
    int days_in_trade = AfxDaysBetween(start_date, (trade->is_open ? AfxCurrentDate() : end_date));
    text = AfxMoney(days_in_trade, true, 0);
    ld->SetData(4, trade, tickerId, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
        COLOR_WHITELIGHT, font8, FontStyleRegular);
    text = L"DIT";
    ld->SetData(5, trade, tickerId, text, StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
        COLOR_WHITEDARK, font8, FontStyleRegular);

    ListBox_AddString(hListBox, ld);


    // Totals Days for Trade
    ld = new ListBoxData;
    int days_total = AfxDaysBetween(start_date, end_date);
    text = AfxMoney(days_total, true, 0);
    ld->SetData(2, trade, tickerId, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
        COLOR_WHITELIGHT, font8, FontStyleRegular);
    text = L"Days";
    ld->SetData(3, trade, tickerId, text, StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
        COLOR_WHITEDARK, font8, FontStyleRegular);
    
    // ROI% per 30 days  
    text = AfxMoney(0, true, 1) + L"%";
    if (days_total == 0) days_total = 1;
    if (trade->trade_bp != 0 && days_total != 0) {
        text = AfxMoney((trade->acb / trade->trade_bp * 100 / days_total * 30), true, 1) + L"%";
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

    bool is_history = GetDlgCtrlID(hListBox) == IDC_HISTORY_LISTBOX ? true : false;
    REAL font8 = 8;
    REAL font9 = 9;
    DWORD clr = COLOR_WHITELIGHT;
    std::wstring text;

    if (is_history) {
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

        text = L"";


        text = (trade->itm_text.length()) ? trade->itm_text : L"";
        clr = (trade->itm_color != COLOR_WHITELIGHT) ? trade->itm_color : COLOR_WHITELIGHT;
        ld->SetData(COLUMN_TICKER_ITM, trade, tickerId, text, StringAlignmentCenter, StringAlignmentCenter, COLOR_GRAYDARK,
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

        text = (trade->column_ticker_portfolio_1.length()) ? trade->column_ticker_portfolio_1 : L"";
        ld->SetData(COLUMN_TICKER_PORTFOLIO_1, trade, tickerId, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_WHITEDARK, font8, FontStyleRegular);   

        text = (trade->column_ticker_portfolio_2.length()) ? trade->column_ticker_portfolio_2 : L"";
        ld->SetData(COLUMN_TICKER_PORTFOLIO_2, trade, tickerId, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_WHITEDARK, font8, FontStyleRegular);   

        text = (trade->column_ticker_portfolio_3.length()) ? trade->column_ticker_portfolio_3 : L"";
        clr = (trade->column_ticker_portfolio_3_color != COLOR_WHITEDARK) ? trade->column_ticker_portfolio_3_color : COLOR_WHITEDARK;
        ld->SetData(COLUMN_TICKER_PORTFOLIO_3, trade, tickerId, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_WHITEDARK, font8, FontStyleRegular);   

        text = (trade->column_ticker_portfolio_4.length()) ? trade->column_ticker_portfolio_4 : L"";
        ld->SetData(COLUMN_TICKER_PORTFOLIO_4, trade, tickerId, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
            clr, font8, FontStyleRegular);   
    }
    ListBox_AddString(hListBox, ld);

    // If the Trade is closed then there is nothing to output as an "open position".
    if (!trade->is_open) return;


    // All tickerId will now be -1 because we are no longer dealing with the main isTickerLine.
    tickerId = -1;

    std::wstring dot = GLYPH_CIRCLE;   // dot character (Segue UI Windows 10/11)
    DWORD yellow = COLOR_MAGENTA;



    // *** SHARES/FUTURES ***
    if (trade->aggregate_shares || trade->aggregate_futures) {

        ld = new ListBoxData;

        std::wstring text_shares;
        std::wstring text_aggregate;
        double value_aggregate = 0;

        if (trade->aggregate_futures) {
            ld->line_type = LineType::futures;
            text_shares = L"FUTURES: " + AfxFormatFuturesDate(trade->future_expiry);
            text_aggregate = std::to_wstring(trade->aggregate_futures);
            value_aggregate = trade->aggregate_futures;
        }

        if (trade->aggregate_shares) {
            ld->line_type = LineType::shares;
            text_shares = L"SHARES";
            text_aggregate = std::to_wstring(trade->aggregate_shares);
            value_aggregate = trade->aggregate_shares;
        }

        ld->SetData(0, trade, tickerId, L"", StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_WHITELIGHT, font8, FontStyleRegular);

        int col = 1;
        if (!is_history) {
            ld->SetData(col, trade, tickerId, dot, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
                COLOR_MAGENTA, font8, FontStyleRegular);
            col++;
        }
        col++;


        ld->SetData(col, trade, tickerId, text_shares, StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYMEDIUM,
            COLOR_WHITEDARK, font8, FontStyleRegular);
        col++;

        ld->SetData(col, trade, tickerId, L"", StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYMEDIUM,
            COLOR_WHITEDARK, font8, FontStyleRegular);
        col++;

        ld->SetData(col, trade, tickerId, L"", StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYMEDIUM,
            COLOR_WHITEDARK, font8, FontStyleRegular);
        col++;

        if (is_history) {
            ld->SetData(col, trade, tickerId, L"", StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYMEDIUM,
                COLOR_WHITEDARK, font8, FontStyleRegular);
            col++;
        }

        text = AfxMoney(std::abs(trade->acb / value_aggregate));
        ld->SetData(col, trade, tickerId, text, StringAlignmentCenter, StringAlignmentCenter, COLOR_GRAYMEDIUM,
            COLOR_WHITEDARK, font8, FontStyleRegular);
        col++;

        text = text_aggregate;
        ld->aggregate_shares = text;
        ld->SetData(col, trade, tickerId, text, StringAlignmentCenter, StringAlignmentCenter, COLOR_GRAYMEDIUM,
            COLOR_WHITEDARK, font8, FontStyleRegular);
        col++;

        text = L"";
        ld->SetData(col, trade, tickerId, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_GRAYMEDIUM, font8, FontStyleRegular);
        col++;

        text = L"";
        ld->SetData(col, trade, tickerId, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_WHITEDARK, font8, FontStyleRegular);

        ListBox_AddString(hListBox, ld);
    }


    // *** OPTION LEGS ***
    for (const auto& leg : trade->open_legs) {
        if (leg->underlying == L"OPTIONS") {
            ld = new ListBoxData;

            ld->leg = leg;
            ld->line_type = LineType::options_leg;

            std::wstring current_date = AfxCurrentDate();
            std::wstring expiry_date = leg->expiry_date;
            std::wstring short_date = AfxShortDate(expiry_date);
            std::wstring dte_text;


            // If the expiry_date is 2 days or less then display in Yellow, otherwise Magenta.
            int dte = AfxDaysBetween(current_date, expiry_date);
            dte_text = std::to_wstring(dte) + L"d";

            yellow = COLOR_MAGENTA;
            if (dte < 3) {
                yellow = COLOR_YELLOW;
            }

            // If the expiry year is greater than current year + 1 then add
            // the year to the display string. Useful for LEAP options.
            if (AfxGetYear(expiry_date) > AfxGetYear(current_date) + 1) {
                short_date.append(L"/");
                short_date.append(std::to_wstring(AfxGetYear(expiry_date)));
            }


            int col = 1;

            if (!is_history) {
                ld->SetData(col, trade, tickerId, dot, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
                    yellow, font8, FontStyleRegular);
                col++;

                ld->SetData(col, trade, tickerId, L"", StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
                    COLOR_WHITELIGHT, font8, FontStyleRegular);  // empty column
            }
            col++;

            ld->SetData(col, trade, tickerId, std::to_wstring(leg->open_quantity), StringAlignmentFar, StringAlignmentCenter,
                COLOR_GRAYMEDIUM, COLOR_WHITELIGHT, font8, FontStyleRegular);  // position quantity
            col++;

            ld->SetData(col, trade, tickerId, short_date, StringAlignmentCenter, StringAlignmentCenter,
                COLOR_GRAYLIGHT, COLOR_WHITELIGHT, font8, FontStyleRegular);   // expiry date
            col++;

            ld->SetData(col, trade, tickerId, dte_text, StringAlignmentCenter, StringAlignmentCenter, COLOR_GRAYMEDIUM,
                COLOR_WHITEDARK, font8, FontStyleRegular);   // DTE
            col++;

            ld->SetData(col, trade, tickerId, leg->strike_price, StringAlignmentCenter, StringAlignmentCenter,
                COLOR_GRAYLIGHT, COLOR_WHITELIGHT, font8, FontStyleRegular);   // strike price
            col++;

            ld->SetData(col, trade, tickerId, L"  " + leg->PutCall, StringAlignmentNear, StringAlignmentCenter,
                COLOR_GRAYMEDIUM, COLOR_WHITEDARK, font8, FontStyleRegular);   // PutCall
            col++;

            if (!is_history) {
                text = (leg->market_value_text.length()) ? leg->market_value_text : L"";
                ld->SetData(COLUMN_TICKER_PORTFOLIO_1, trade, tickerId, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
                    COLOR_WHITEDARK, font8, FontStyleRegular);   
                col++;

                text = L"";
                ld->SetData(COLUMN_TICKER_PORTFOLIO_2, trade, tickerId, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
                    COLOR_WHITEDARK, font8, FontStyleRegular);   
                col++;

                ld->SetData(COLUMN_TICKER_PORTFOLIO_3, trade, tickerId, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
                    clr, font8, FontStyleRegular);   
                col++;

                ld->SetData(COLUMN_TICKER_PORTFOLIO_4, trade, tickerId, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
                    clr, font8, FontStyleRegular);   
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
    ld->line_type = LineType::none;
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

    ld->line_type = LineType::transaction_header;
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
// Create the display data for a History Dividend.
// ========================================================================================
void ListBoxData_HistoryDividendLeg(
    HWND hListBox, const std::shared_ptr<Trade>& trade, const std::shared_ptr<Transaction>& trans, const std::shared_ptr<Leg>& leg)
{
    ListBoxData* ld = new ListBoxData;

    TickerId tickerId = -1;
    REAL font8 = 8;

    ld->SetData(2, trade, tickerId, trans->underlying, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYMEDIUM, COLOR_WHITEDARK, font8, FontStyleRegular);

    for (int i = 3; i < 8; i++) {
        ld->SetData(i, trade, tickerId, L"", StringAlignmentNear, StringAlignmentCenter,
            COLOR_GRAYMEDIUM, COLOR_WHITEDARK, font8, FontStyleRegular);
    }
    
    std::wstring text = AfxMoney(trans->price);  // , false, trade->ticker_decimals);
    ld->SetData(7, trade, tickerId, text, StringAlignmentFar, StringAlignmentCenter,
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


    int dte = AfxDaysBetween(trans->trans_date, leg->expiry_date);
    std::wstring days = std::to_wstring(dte) + L"d";
    std::wstring short_date = AfxShortDate(leg->expiry_date);

    // If the expiry year is greater than current year + 1 then add
    // the year to the display string. Useful for LEAP options.
    if (AfxGetYear(leg->expiry_date) > AfxGetYear(trans->trans_date) + 1) {
        short_date.append(L"/");
        short_date.append(std::to_wstring(AfxGetYear(leg->expiry_date)));
    }

    ld->SetData(3, trade, tickerId, short_date, StringAlignmentCenter, StringAlignmentCenter,
        COLOR_GRAYLIGHT, COLOR_WHITELIGHT, font8, FontStyleRegular);

    ld->SetData(4, trade, tickerId, days, StringAlignmentCenter, StringAlignmentCenter,
        COLOR_GRAYMEDIUM, COLOR_WHITEDARK, font8, FontStyleRegular);

    ld->SetData(5, trade, tickerId, leg->strike_price, StringAlignmentCenter, StringAlignmentCenter,
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
// Create the display data line for closed position yearly total.
// ========================================================================================
void ListBoxData_OutputClosedYearTotal(HWND hListBox, int year, double subtotal, int year_win, int year_loss)
{
    ListBoxData* ld = new ListBoxData;

    TickerId tickerId = -1;
    REAL font8 = 8;

    DWORD clr = (subtotal >= 0) ? COLOR_GREEN : COLOR_RED;
    
    if (year == 0) year = AfxGetYear(AfxCurrentDate());

    std::wstring text = std::to_wstring(year) + L" TOTAL";
    ld->SetData(3, nullptr, tickerId, text, StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleBold);

    ld->SetData(4, nullptr, tickerId, AfxMoney(subtotal), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleBold);

    // col 5 is a "spacer" column

    clr = (year_win >= year_loss) ? COLOR_GREEN : COLOR_RED;
    text = std::to_wstring(year_win) + L"W " + std::to_wstring(year_loss)
        + L"L  " + AfxMoney((double)year_win / (max(year_win + year_loss,1)) * 100, false, 0) + L"%";
    ld->SetData(6, nullptr, tickerId, text, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    ListBox_InsertString(hListBox, 0, ld);
}


// ========================================================================================
// Create the display data line for total closed monthly total.
// ========================================================================================
void ListBoxData_OutputClosedMonthTotal(HWND hListBox, double monthly_total, int month_win, int month_loss)
{
    ListBoxData* ld = new ListBoxData;

    TickerId tickerId = -1;
    REAL font8 = 8;

    DWORD clr = (monthly_total >= 0) ? COLOR_GREEN : COLOR_RED;
    
    std::wstring text = L"THIS MONTH";
    ld->SetData(3, nullptr, tickerId, text, StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleBold);

    ld->SetData(4, nullptr, tickerId, AfxMoney(monthly_total), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleBold);

    // col 5 is a "spacer" column

    clr = (month_win >= month_loss) ? COLOR_GREEN : COLOR_RED;
    text = std::to_wstring(month_win) + L"W " + std::to_wstring(month_loss)
        + L"L  " + AfxMoney((double)month_win / (max(month_win + month_loss,1)) * 100, false, 0) + L"%";
    ld->SetData(6, nullptr, tickerId, text, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    ListBox_InsertString(hListBox, 1, ld);
}


// ========================================================================================
// Create the display data line for total closed weekly total.
// ========================================================================================
void ListBoxData_OutputClosedWeekTotal(HWND hListBox, double weekly_total, int week_win, int week_loss)
{
    ListBoxData* ld = new ListBoxData;

    TickerId tickerId = -1;
    REAL font8 = 8;

    DWORD clr = (weekly_total >= 0) ? COLOR_GREEN : COLOR_RED;
    
    std::wstring text = L"THIS WEEK";
    ld->SetData(3, nullptr, tickerId, text, StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleBold);

    ld->SetData(4, nullptr, tickerId, AfxMoney(weekly_total), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleBold);

    // col 5 is a "spacer" column

    clr = (week_win >= week_loss) ? COLOR_GREEN : COLOR_RED;
    text = std::to_wstring(week_win) + L"W " + std::to_wstring(week_loss)
        + L"L  " + AfxMoney((double)week_win / (max(week_win + week_loss,1)) * 100, false, 0) + L"%";
    ld->SetData(6, nullptr, tickerId, text, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    ListBox_InsertString(hListBox, 2, ld);
}


// ========================================================================================
// Create the display data line for total closed daily total.
// ========================================================================================
void ListBoxData_OutputClosedDayTotal(HWND hListBox, double daily_total, int day_win, int day_loss)
{
    ListBoxData* ld = new ListBoxData;

    TickerId tickerId = -1;
    REAL font8 = 8;

    DWORD clr = (daily_total >= 0) ? COLOR_GREEN : COLOR_RED;
    
    std::wstring text = L"TODAY";
    ld->SetData(3, nullptr, tickerId, text, StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleBold);

    ld->SetData(4, nullptr, tickerId, AfxMoney(daily_total), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleBold);

    // col 5 is a "spacer" column

    clr = (day_win >= day_loss) ? COLOR_GREEN : COLOR_RED;
    text = std::to_wstring(day_win) + L"W " + std::to_wstring(day_loss)
        + L"L  " + AfxMoney((double)day_win / (max(day_win + day_loss,1)) * 100, false, 0) + L"%";
    ld->SetData(6, nullptr, tickerId, text, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    ListBox_InsertString(hListBox, 3, ld);

    // *** BLANK SEPARATION LINE AFTER THE DAY TOTAL ***
    ld = new ListBoxData;
    ld->line_type = LineType::none;
    ListBox_InsertString(hListBox, 4, ld);
}


// ========================================================================================
// Create the display data line for a closed position month subtotal.
// ========================================================================================
void ListBoxData_OutputClosedMonthSubtotal(HWND hListBox, const std::wstring& closed_date, double subtotal, int month_win, int month_loss)
{
    ListBoxData* ld = new ListBoxData;

    TickerId tickerId = -1;
    REAL font8 = 8;

    DWORD clr = (subtotal >= 0) ? COLOR_GREEN : COLOR_RED;
    
    std::wstring text = AfxUpper(AfxGetLongMonthName(closed_date)) + L" " + std::to_wstring(AfxGetYear(closed_date));
    ld->SetData(3, nullptr, tickerId, text, StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleBold);

    ld->SetData(4, nullptr, tickerId, AfxMoney(subtotal), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleBold);

    // col 5 is a "spacer" column

    clr = (month_win >= month_loss) ? COLOR_GREEN : COLOR_RED;
    text = std::to_wstring(month_win) + L"W " + std::to_wstring(month_loss)
        + L"L  " + AfxMoney((double)month_win / (max(month_win + month_loss,1)) * 100, false, 0) + L"%";
    ld->SetData(6, nullptr, tickerId, text, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    ListBox_AddString(hListBox, ld);
    ListBoxData_AddBlankLine(hListBox);

}


// ========================================================================================
// Create the display data line for a closed position.
// ========================================================================================
void ListBoxData_OutputClosedPosition(HWND hListBox, const std::shared_ptr<Trade>& trade, const std::wstring& closed_date)
{
    ListBoxData* ld = new ListBoxData;

    TickerId tickerId = -1;
    REAL font8 = 8;

    ld->SetData(0, trade, tickerId, L"", StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_ORANGE, font8, FontStyleRegular);

    ld->SetData(1, trade, tickerId, closed_date, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_ORANGE, font8, FontStyleRegular);

    ld->SetData(2, trade, tickerId, trade->ticker_symbol, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_WHITELIGHT, font8, FontStyleRegular);

    std::wstring ticker_name = trade->ticker_name;
    if (IsFuturesTicker(trade->ticker_symbol)) ticker_name += L" (" + AfxFormatFuturesDate(trade->future_expiry) + L")";
    ld->SetData(3, trade, tickerId, ticker_name, StringAlignmentNear, StringAlignmentCenter,
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
    double running_gross_total, double running_fees_total, double running_net_total)
{
    ListBoxData* ld = new ListBoxData;

    TickerId tickerId = -1;
    REAL font8 = 8;

    DWORD clr = (running_net_total >= 0) ? COLOR_GREEN : COLOR_RED;

    std::wstring text = L"TOTAL";
    ld->SetData(3, nullptr, tickerId, text, StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    ld->SetData(5, nullptr, tickerId, AfxMoney(running_gross_total), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleBold);

    ld->SetData(6, nullptr, tickerId, AfxMoney(running_fees_total), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleBold);

    ld->SetData(7, nullptr, tickerId, AfxMoney(running_net_total), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleBold);

    ListBox_InsertString(hListBox, 0, ld);
    // *** BLANK SEPARATION LINE AFTER THE TOTAL ***
    ld = new ListBoxData;
    ld->line_type = LineType::none;
    ListBox_InsertString(hListBox, 1, ld);
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
void ListBoxData_OutputTickerTotals(HWND hListBox, const std::wstring& ticker, double amount)
{
    ListBoxData* ld = new ListBoxData;

    TickerId tickerId = -1;
    REAL font8 = 8;

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

    int item_count = Header_GetItemCount(hWnd);

    for (int i = 0; i < item_count; i++) {
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
        if (i > 0 && i < item_count) {
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

        bool is_hot = false;

        SaveDC(lpDrawItem->hDC);

        HDC memDC = NULL;         // Double buffering
        HBITMAP hbit = NULL;      // Double buffering

        memDC = CreateCompatibleDC(lpDrawItem->hDC);
        hbit = CreateCompatibleBitmap(lpDrawItem->hDC, nWidth, nHeight);
        if (hbit) SelectObject(memDC, hbit);

        if ((lpDrawItem->itemAction | ODA_SELECT) &&
            (lpDrawItem->itemState & ODS_SELECTED)) {
            is_hot = true;
        }

        Graphics graphics(memDC);
        graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);


        // Set some defaults in case there is no valid ListBox line number
        std::wstring text;

        DWORD back_color = (is_hot) ? COLOR_SELECTION : COLOR_GRAYDARK;
        DWORD text_color = COLOR_WHITELIGHT;

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
        int column_width = 0;
        int column_start = 0;
        int column_end = MAX_COLUMNS;

        // If this is a Category separator line then we need to draw it differently
        // then a regular that would be drawn (full line width)
        if (ld != nullptr) {
            if (ld->line_type == LineType::category_header) {
                column_start = 0;
                column_end = 1;
            }
        }

        // Draw each of the columns
        for (int i = column_start; i < column_end; ++i) {
            if (ld == nullptr) break;
            if (ld->col[i].column_width == 0) break;

            // Prepare and draw the text
            text = ld->col[i].text;

            HAlignment = ld->col[i].HAlignment;
            VAlignment = ld->col[i].VAlignment;
            back_color = (is_hot) ? back_color : ld->col[i].back_theme;
            text_color = ld->col[i].text_theme;
            fontSize = ld->col[i].font_size;
            fontStyle = ld->col[i].font_style;

            column_width = (ld->line_type == LineType::category_header) ? nWidth : AfxScaleX((float)ld->col[i].column_width);

            back_brush.SetColor(back_color);
            graphics.FillRectangle(&back_brush, nLeft, 0, column_width, nHeight);

            Font         font(&fontFamily, fontSize, fontStyle, Unit::UnitPoint);
            SolidBrush   text_brush(text_color);
            StringFormat stringF(StringFormatFlagsNoWrap);
            stringF.SetAlignment(HAlignment);
            stringF.SetLineAlignment(VAlignment);

            // If right alignment then add a very small amount of right side
            // padding so that text is not pushed up right against the right side.
            int rightPadding = 0;
            if (HAlignment == StringAlignmentFar) rightPadding = AfxScaleX(2);

            RectF rcText((REAL)nLeft, (REAL)0, (REAL)column_width - rightPadding, (REAL)nHeight);
            graphics.DrawString(text.c_str(), -1, &font, rcText, &stringF, &text_brush);

            nLeft += column_width;
        }


        BitBlt(lpDrawItem->hDC, lpDrawItem->rcItem.left,
            lpDrawItem->rcItem.top, nWidth, nHeight, memDC, 0, 0, SRCCOPY);


        // Cleanup
        RestoreDC(lpDrawItem->hDC, -1);
        if (hbit) DeleteObject(hbit);
        if (memDC) DeleteDC(memDC);
    }
}
