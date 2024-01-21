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

#include "pch.h"
#include "ListBoxData.h"

#include "MainWindow/tws-client.h"
#include "ActiveTrades/ActiveTrades.h"
#include "ClosedTrades/ClosedTrades.h"
#include "TradeHistory/TradeHistory.h"
#include "TickerTotals/TickerTotals.h"
#include "Transactions/TransPanel.h"
#include "Database/database.h"
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
bool ListBoxData_ResizeColumnWidths(HWND hListBox, TableType tabletype) {
    int listbox_end = (int)ListBox_GetCount(hListBox);
    if (listbox_end == 0) return false;
    int listbox_start = 0;

    int col_width[MAX_COLUMNS] = { 0,0,0,0,0,0,0,0,0,0,0 };

    // Initialize the col_width array based on the incoming ListBox
    for (int i = 0; i < MAX_COLUMNS; i++) {
        switch (tabletype) {
        case TableType::active_trades:
            col_width[i] = nTradesMinColWidth[i];
            break;

        case TableType::closed_trades:
            col_width[i] = nClosedMinColWidth[i];
            break;

        case TableType::trade_history:
            col_width[i] = nHistoryMinColWidth[i];
            break;

        case TableType::ticker_totals:
            col_width[i] = nTickerTotalsMinColWidth[i];
            break;

        case TableType::trans_panel:
            col_width[i] = nTransMinColWidth[i];
            break;
        }
    }

    HDC hdc = GetDC(hListBox);

    Graphics graphics(hdc);
    graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

    std::wstring font_name = AfxGetDefaultFont();
    FontFamily fontFamily(font_name.c_str());
    REAL font_size = 9;
    int font_style = FontStyleRegular;
    RectF boundRect;
    RectF layoutRect(0.0f, 0.0f, 1000.0f, 50.0f);
    StringFormat format;
    format.SetAlignment(StringAlignmentFar);

    bool bRedrawListBox = false;

    int nColMaxTextLength[MAX_COLUMNS] = { 0,0,0,0,0,0,0,0,0,0,0 };

    for (int ii = listbox_start; ii < listbox_end; ++ii) {
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, ii);
        if (!ld) continue;
        if (ld == (void*)-1) continue;
        if (ld->line_type == LineType::category_header) continue;

        for (int i = 0; i < MAX_COLUMNS; i++) {
            if (col_width[i] == 0) continue;

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
            font_size = ld->col[i].font_size;
            font_style = ld->col[i].font_style;
            Font font(&fontFamily, font_size, font_style, Unit::UnitPoint);

            graphics.MeasureString(ld->col[i].text.c_str(), (int)ld->col[i].text.length(),
                &font, layoutRect, &format, &boundRect);

            int text_length = AfxUnScaleX(boundRect.Width) + 5;  // add a bit for padding

            if (text_length > col_width[i]) {
                
                col_width[i] = text_length;

                if (tabletype == TableType::trade_history) {
                    col_width[i] = min(col_width[i], nHistoryMaxColWidth[i]);
                }

                if (tabletype == TableType::ticker_totals) {
                    col_width[i] = min(col_width[i], nTickerTotalsMaxColWidth[i]);
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
            ld->col[i].column_width = col_width[i];
        }
        ListBox_SetItemData(hListBox, ii, ld);
    }

    // Update the widths of any associated Header control. 
    HWND hHeader = NULL;
    if (tabletype == TableType::closed_trades) hHeader = ClosedTrades.TradesHeader();
    if (tabletype == TableType::ticker_totals) hHeader = TickerPanel.TickersHeader();
    if (tabletype == TableType::trans_panel) hHeader = TransPanel.TransHeader();
    
    if (hHeader) {
        int item_width = 0;
        for (int i = 0; i < MAX_COLUMNS; i++) {
            // Do a width change check in order to prevent redrawing the header and causing flash.
            item_width = AfxScaleX(col_width[i]);
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
void ListBoxData_DestroyItemData(HWND hListBox) {
    // delete any previously manually allocated ListBoxData structures.
    int item_count = ListBox_GetCount(hListBox);

    for (int i = 0; i < item_count; ++i) {
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, i);
        if (ld) {
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
void ListBoxData_RequestMarketData(HWND hListBox) {
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
        if (ld) {
            if (ld->line_type == LineType::ticker_line) {
                tws_RequestMarketData(ld);
            }
        }
    }
}


// ========================================================================================
// Create the display data for a Category Header line
// ========================================================================================
void ListBoxData_AddCategoryHeader(HWND hListBox, const std::shared_ptr<Trade>& trade, int num_trades_category) {
    ListBoxData* ld = new ListBoxData;

    std::wstring text = AfxUpper(config.GetCategoryDescription(trade->category)) +
        L" (" + std::to_wstring(num_trades_category) + L" trades)";

    ld->SetData(0, nullptr, -1, text, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_BLUE, 9, FontStyleBold);

    ld->line_type = LineType::category_header;
    ListBox_AddString(hListBox, ld);
}


// ========================================================================================
// Add a simple No Trades Exist message
// ========================================================================================
void ListBoxData_NoTradesExistMessage(HWND hListBox) {
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
void ListBoxData_TradeROI(HWND hListBox, const std::shared_ptr<Trade>& trade, TickerId ticker_id) {
    ListBoxData* ld = new ListBoxData;

    REAL font8 = 8;
    std::wstring text;

    std::wstring start_date = AfxInsertDateHyphens(trade->bp_start_date);
    std::wstring end_date = AfxInsertDateHyphens(trade->bp_end_date);

    // Buying Power
    text = AfxMoney(trade->trade_bp, true, 0);
    ld->SetData(2, trade, ticker_id, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
        COLOR_WHITELIGHT, font8, FontStyleRegular);
    text = L"BP";
    ld->SetData(3, trade, ticker_id, text, StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
        COLOR_WHITEDARK, font8, FontStyleRegular);
    
    // Days In Trade
    int days_in_trade = AfxDaysBetween(start_date, (trade->is_open ? AfxCurrentDate() : end_date));
    text = AfxMoney(days_in_trade, true, 0);
    ld->SetData(4, trade, ticker_id, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
        COLOR_WHITELIGHT, font8, FontStyleRegular);
    text = L"DIT";
    ld->SetData(5, trade, ticker_id, text, StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
        COLOR_WHITEDARK, font8, FontStyleRegular);

    ListBox_AddString(hListBox, ld);

    // Totals Days for Trade
    ld = new ListBoxData;
    int days_total = AfxDaysBetween(start_date, end_date);
    text = AfxMoney(days_total, true, 0);
    ld->SetData(2, trade, ticker_id, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
        COLOR_WHITELIGHT, font8, FontStyleRegular);
    text = L"Days";
    ld->SetData(3, trade, ticker_id, text, StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
        COLOR_WHITEDARK, font8, FontStyleRegular);
    
    // ROI% per 30 days  
    text = AfxMoney(0, true, 1) + L"%";
    if (days_total == 0) days_total = 1;
    if (trade->trade_bp != 0 && days_total != 0) {
        text = AfxMoney((trade->acb_total / trade->trade_bp * 100 / days_total * 30), true, 1) + L"%";
    }
    ld->SetData(4, trade, ticker_id, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
        COLOR_WHITELIGHT, font8, FontStyleRegular);
    text = L"30d";
    ld->SetData(5, trade, ticker_id, text, StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
        COLOR_WHITEDARK, font8, FontStyleRegular);

    ListBox_AddString(hListBox, ld);

    ListBoxData_AddBlankLine(hListBox);
}

    
// ========================================================================================
// Create the display data for an Open Position that displays in Trades & History tables.
// ========================================================================================
void ListBoxData_OpenPosition(HWND hListBox, const std::shared_ptr<Trade>& trade, TickerId ticker_id) {
    ListBoxData* ld = new ListBoxData;

    bool is_history = GetDlgCtrlID(hListBox) == IDC_HISTORY_LISTBOX ? true : false;
    REAL font8 = 8;
    REAL font9 = 9;
    DWORD clr = COLOR_WHITELIGHT;
    std::wstring text;

    bool has_shares = (trade->aggregate_shares || trade->aggregate_futures) ? true : false;
    double acb = (has_shares) ? trade->shares_acb : trade->acb_total;

    if (is_history) {
        ticker_id = -1;

        ld->SetData(0, trade, ticker_id, GLYPH_TREEOPEN, StringAlignmentCenter, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_WHITEDARK, font8, FontStyleRegular);

        text = (trade->is_open ? L"Open Pos" : L"Closed Pos");
        ld->SetData(1, trade, ticker_id, text, StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_ORANGE, font8, FontStyleRegular);   // orange

        text = AfxMoney(acb, true);
        clr = (acb >= 0) ? COLOR_GREEN : COLOR_RED;
        ld->SetData(7, trade, ticker_id, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
            clr, font8, FontStyleRegular);

    }
    else {
        ld->SetData(0, trade, ticker_id, GLYPH_CIRCLE, StringAlignmentCenter, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_WHITEDARK, font8, FontStyleRegular);
        ld->SetData(1, trade, ticker_id, trade->ticker_symbol, StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_WHITELIGHT, font9, FontStyleRegular | FontStyleBold);

        text = L"";

        text = (trade->itm_text.length()) ? trade->itm_text : L"";
        clr = (trade->itm_color != COLOR_WHITELIGHT) ? trade->itm_color : COLOR_WHITELIGHT;
        ld->SetData(COLUMN_TICKER_ITM, trade, ticker_id, text, StringAlignmentCenter, StringAlignmentCenter, COLOR_GRAYDARK,
            clr, font8, FontStyleRegular);   // ITM
        ld->SetData(3, trade, ticker_id, L"", StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_WHITELIGHT, font8, FontStyleRegular);
        ld->SetData(4, trade, ticker_id, L"", StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_WHITELIGHT, font8, FontStyleRegular);

        text = trade->ticker_column_1;
        clr = trade->ticker_column_1_clr;
        ld->SetData(COLUMN_TICKER_CHANGE, trade, ticker_id, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
            clr, font8, FontStyleRegular);   // price change

        text = trade->ticker_column_2;
        clr = trade->ticker_column_2_clr;
        ld->SetData(COLUMN_TICKER_CURRENTPRICE, trade, ticker_id, text, StringAlignmentCenter, StringAlignmentCenter, COLOR_GRAYDARK,
            clr, font9, FontStyleRegular | FontStyleBold);   // current price

        text = trade->ticker_column_3;
        clr = trade->ticker_column_3_clr;
        ld->SetData(COLUMN_TICKER_PERCENTCHANGE, trade, ticker_id, text, StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
            clr, font8, FontStyleRegular);   // price percentage change

        text = L"";
        clr = COLOR_WHITEDARK;
        ld->SetData(COLUMN_TICKER_PORTFOLIO_1, trade, ticker_id, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_WHITEDARK, font8, FontStyleRegular);   

        ld->SetData(COLUMN_TICKER_PORTFOLIO_2, trade, ticker_id, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_WHITEDARK, font8, FontStyleRegular);   

        ld->SetData(COLUMN_TICKER_PORTFOLIO_3, trade, ticker_id, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_WHITEDARK, font8, FontStyleRegular);   

        ld->SetData(COLUMN_TICKER_PORTFOLIO_4, trade, ticker_id, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_WHITEDARK, font8, FontStyleRegular);
        
        ld->SetData(COLUMN_TICKER_PORTFOLIO_5, trade, ticker_id, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_WHITEDARK, font8, FontStyleRegular);
    }
    ListBox_AddString(hListBox, ld);

    // If the Trade is closed then there is nothing to output as an "open position".
    if (!trade->is_open) return;

    // All ticker_id will now be -1 because we are no longer dealing with the main isTickerLine.
    ticker_id = -1;

    std::wstring dot = GLYPH_CIRCLE;   // dot character (Segue UI Windows 10/11)
    DWORD yellow = COLOR_MAGENTA;

    // *** SHARES/FUTURES ***
    if (has_shares) {

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

        ld->SetData(0, trade, ticker_id, L"", StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_WHITELIGHT, font8, FontStyleRegular);

        int col = 1;
        if (!is_history) {
            ld->SetData(col, trade, ticker_id, dot, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
                COLOR_MAGENTA, font8, FontStyleRegular);
            col++;
        }
        col++;


        ld->SetData(col, trade, ticker_id, text_shares, StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYMEDIUM,
            COLOR_WHITEDARK, font8, FontStyleRegular);
        col++;

        ld->SetData(col, trade, ticker_id, L"", StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYMEDIUM,
            COLOR_WHITEDARK, font8, FontStyleRegular);
        col++;

        ld->SetData(col, trade, ticker_id, L"", StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYMEDIUM,
            COLOR_WHITEDARK, font8, FontStyleRegular);
        col++;

        if (is_history) {
            ld->SetData(col, trade, ticker_id, L"", StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYMEDIUM,
                COLOR_WHITEDARK, font8, FontStyleRegular);
            col++;
        }

        text = AfxMoney(std::abs(trade->shares_acb / value_aggregate));
        ld->SetData(col, trade, ticker_id, text, StringAlignmentCenter, StringAlignmentCenter, COLOR_GRAYMEDIUM,
            COLOR_WHITEDARK, font8, FontStyleRegular);
        col++;

        text = text_aggregate;
        ld->aggregate_shares = text;
        ld->SetData(col, trade, ticker_id, text, StringAlignmentCenter, StringAlignmentCenter, COLOR_GRAYMEDIUM,
            COLOR_WHITEDARK, font8, FontStyleRegular);
        col++;

        text = L"";
        ld->SetData(col, trade, ticker_id, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_GRAYMEDIUM, font8, FontStyleRegular);
        col++;

        text = L"";
        ld->SetData(col, trade, ticker_id, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
            COLOR_WHITEDARK, font8, FontStyleRegular);

        ListBox_AddString(hListBox, ld);
    }


    // *** OPTION LEGS ***
    for (auto& leg : trade->open_legs) {
        if (leg->underlying == Underlying::Options) {
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
                ld->SetData(col, trade, ticker_id, dot, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
                    yellow, font8, FontStyleRegular);
                col++;

                ld->SetData(col, trade, ticker_id, L"", StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
                    COLOR_WHITELIGHT, font8, FontStyleRegular);  // empty column
            }
            col++;

            ld->SetData(col, trade, ticker_id, std::to_wstring(leg->open_quantity), StringAlignmentFar, StringAlignmentCenter,
                COLOR_GRAYMEDIUM, COLOR_WHITELIGHT, font8, FontStyleRegular);  // position quantity
            col++;

            ld->SetData(col, trade, ticker_id, short_date, StringAlignmentCenter, StringAlignmentCenter,
                COLOR_GRAYLIGHT, COLOR_WHITELIGHT, font8, FontStyleRegular);   // expiry date
            col++;

            ld->SetData(col, trade, ticker_id, dte_text, StringAlignmentCenter, StringAlignmentCenter, COLOR_GRAYMEDIUM,
                COLOR_WHITEDARK, font8, FontStyleRegular);   // DTE
            col++;

            ld->SetData(col, trade, ticker_id, leg->strike_price, StringAlignmentCenter, StringAlignmentCenter,
                COLOR_GRAYLIGHT, COLOR_WHITELIGHT, font8, FontStyleRegular);   // strike price
            col++;

            ld->SetData(col, trade, ticker_id, L"  " + db.PutCallToString(leg->put_call), StringAlignmentNear, StringAlignmentCenter,
                COLOR_GRAYMEDIUM, COLOR_WHITEDARK, font8, FontStyleRegular);   // PutCall
            col++;

            if (!is_history) {
                text = L"";
                ld->SetData(COLUMN_TICKER_PORTFOLIO_1, trade, ticker_id, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
                    COLOR_WHITEDARK, font8, FontStyleRegular);

                ld->SetData(COLUMN_TICKER_PORTFOLIO_2, trade, ticker_id, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
                    COLOR_WHITEDARK, font8, FontStyleRegular);   
                col++;

                ld->SetData(COLUMN_TICKER_PORTFOLIO_3, trade, ticker_id, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
                    COLOR_WHITEDARK, font8, FontStyleRegular);   
                col++;

                ld->SetData(COLUMN_TICKER_PORTFOLIO_4, trade, ticker_id, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
                    COLOR_WHITEDARK, font8, FontStyleRegular);
                col++;

                text = L"";
                ld->SetData(COLUMN_TICKER_PORTFOLIO_5, trade, ticker_id, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
                    COLOR_WHITEDARK, font8, FontStyleRegular);
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
void ListBoxData_AddBlankLine(HWND hListBox) {
    // *** BLANK SEPARATION LINE AT END OF LIST ***
    ListBoxData* ld = new ListBoxData;
    ld->line_type = LineType::none;
    ListBox_AddString(hListBox, ld);
}


// ========================================================================================
// Create the display data a History Header line.
// ========================================================================================
void ListBoxData_HistoryHeader(HWND hListBox, const std::shared_ptr<Trade>& trade, const std::shared_ptr<Transaction>& trans) {
    // Display the transaction description, date, and total prior to showing the detail lines
    ListBoxData* ld = new ListBoxData;

    std::wstring text;
    REAL font8 = 8;

    DWORD clr = COLOR_WHITEDARK;
        
    TickerId ticker_id = -1;

    ld->line_type = LineType::transaction_header;
    ld->trans = trans;

    ld->SetData(0, trade, ticker_id, GLYPH_TREEOPEN, StringAlignmentCenter, StringAlignmentCenter, COLOR_GRAYDARK,
        COLOR_WHITEDARK, font8, FontStyleRegular);

    text = trans->description;
    ld->SetData(1, trade, ticker_id, text, StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
        COLOR_ORANGE, font8, FontStyleRegular);   // orange

    text = trans->trans_date;
    ld->SetData(2, trade, ticker_id, text, StringAlignmentNear, StringAlignmentCenter, COLOR_GRAYDARK,
        COLOR_WHITELIGHT, font8, FontStyleRegular);

    if (trans->underlying == Underlying::Shares ||
        trans->underlying == Underlying::Futures) {

        if (trans->legs.at(0)->action == Action::STC ||
            trans->legs.at(0)->action == Action::BTC) {

            double quantity = trans->quantity;
            double total = quantity * trans->price;
            double cost = (quantity * trans->share_average_cost);
            double fees = trans->fees * -1;
            double diff = (total + cost + fees);

            text = L"";

            clr = (total >= 0) ? COLOR_GREEN : COLOR_RED;
            text = AfxMoney(total, true, 2);
            ld->SetData(4, trade, ticker_id, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
                clr, font8, FontStyleRegular);

            clr = (cost >= 0) ? COLOR_GREEN : COLOR_RED;
            text = AfxMoney(cost, true, 2);
            ld->SetData(5, trade, ticker_id, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
                clr, font8, FontStyleRegular);

            clr = (fees >= 0) ? COLOR_GREEN : COLOR_RED;
            text = AfxMoney(fees, true, 2);
            ld->SetData(6, trade, ticker_id, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
                clr, font8, FontStyleRegular);

            clr = (diff >= 0) ? COLOR_GREEN : COLOR_RED;
            text = AfxMoney(diff, true, 2);
            ld->SetData(7, trade, ticker_id, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
                clr, font8, FontStyleRegular);
        }

        if (trans->legs.at(0)->action == Action::BTO ||
            trans->legs.at(0)->action == Action::STO) {
            text = AfxMoney(trans->total);
            clr = (trans->total >= 0) ? COLOR_GREEN : COLOR_RED;
            ld->SetData(7, trade, ticker_id, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
                clr, font8, FontStyleRegular);   // green/red
        }
    }
    else {
        text = AfxMoney(trans->total);
        clr = (trans->total >= 0) ? COLOR_GREEN : COLOR_RED;
        ld->SetData(7, trade, ticker_id, text, StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK,
            clr, font8, FontStyleRegular);   // green/red
    }

    ld->SetData(8, trade, ticker_id, GLYPH_MAGNIFYGLASS, StringAlignmentCenter, StringAlignmentCenter,
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

    TickerId ticker_id = -1;
    REAL font8 = 8;

    std::wstring text;

    if (trans->legs.at(0)->action == Action::STC ||
        trans->legs.at(0)->action == Action::BTC) {

        text = L"SELL";
        if (trans->share_longshort == LongShort::Long) text = L"BUY";
        ld->SetData(2, trade, ticker_id, text, StringAlignmentNear, StringAlignmentCenter,
            COLOR_GRAYMEDIUM, COLOR_WHITEDARK, font8, FontStyleRegular);

        text = AfxMoney(leg->open_quantity, true, 0);
        ld->SetData(3, trade, ticker_id, text, StringAlignmentFar, StringAlignmentCenter,
            COLOR_GRAYMEDIUM, COLOR_WHITEDARK, font8, FontStyleRegular);

        text = AfxMoney(trans->price, true, trade->ticker_decimals);
        ld->SetData(4, trade, ticker_id, text, StringAlignmentFar, StringAlignmentCenter,
            COLOR_GRAYMEDIUM, COLOR_WHITEDARK, font8, FontStyleRegular);

        text = AfxMoney(trans->share_average_cost, true, trade->ticker_decimals);
        ld->SetData(5, trade, ticker_id, text, StringAlignmentFar, StringAlignmentCenter,
            COLOR_GRAYMEDIUM, COLOR_WHITEDARK, font8, FontStyleRegular);

        double diff = (trans->price + trans->share_average_cost);
        text = AfxMoney(diff, true, trade->ticker_decimals);
        ld->SetData(6, trade, ticker_id, text, StringAlignmentFar, StringAlignmentCenter,
            COLOR_GRAYMEDIUM, COLOR_WHITEDARK, font8, FontStyleRegular);

        double total = (abs(leg->open_quantity) * diff);
        text = AfxMoney(total, true, trade->ticker_decimals);
        ld->SetData(7, trade, ticker_id, text, StringAlignmentFar, StringAlignmentCenter,
            COLOR_GRAYMEDIUM, COLOR_WHITEDARK, font8, FontStyleRegular);

    }

    if (trans->legs.at(0)->action == Action::BTO ||
        trans->legs.at(0)->action == Action::STO) {

        text = L"BUY";
        if (trans->share_longshort == LongShort::Short) text = L"SELL";
        ld->SetData(2, trade, ticker_id, text, StringAlignmentNear, StringAlignmentCenter,
            COLOR_GRAYMEDIUM, COLOR_WHITEDARK, font8, FontStyleRegular);

        ld->SetData(3, trade, ticker_id, L"", StringAlignmentFar, StringAlignmentCenter,
            COLOR_GRAYMEDIUM, COLOR_WHITEDARK, font8, FontStyleRegular);

        ld->SetData(4, trade, ticker_id, L"", StringAlignmentFar, StringAlignmentCenter,
            COLOR_GRAYMEDIUM, COLOR_WHITEDARK, font8, FontStyleRegular);

        ld->SetData(5, trade, ticker_id, L"", StringAlignmentFar, StringAlignmentCenter,
            COLOR_GRAYMEDIUM, COLOR_WHITEDARK, font8, FontStyleRegular);

        text = AfxMoney(leg->open_quantity, true, 0);
        ld->SetData(6, trade, ticker_id, text, StringAlignmentFar, StringAlignmentCenter,
            COLOR_GRAYMEDIUM, COLOR_WHITEDARK, font8, FontStyleRegular);

        text = AfxMoney(trans->price, true, trade->ticker_decimals);
        ld->SetData(7, trade, ticker_id, text, StringAlignmentFar, StringAlignmentCenter,
            COLOR_GRAYMEDIUM, COLOR_WHITEDARK, font8, FontStyleRegular);
    }

    ListBox_AddString(hListBox, ld);
}


// ========================================================================================
// Create the display data for a History Dividend.
// ========================================================================================
void ListBoxData_HistoryDividendLeg(
    HWND hListBox, const std::shared_ptr<Trade>& trade, const std::shared_ptr<Transaction>& trans, const std::shared_ptr<Leg>& leg)
{
    ListBoxData* ld = new ListBoxData;

    TickerId ticker_id = -1;
    REAL font8 = 8;

    std::wstring text = L"DIVIDEND";
    ld->SetData(2, trade, ticker_id, text, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYMEDIUM, COLOR_WHITEDARK, font8, FontStyleRegular);

    for (int i = 3; i < 8; i++) {
        ld->SetData(i, trade, ticker_id, L"", StringAlignmentNear, StringAlignmentCenter,
            COLOR_GRAYMEDIUM, COLOR_WHITEDARK, font8, FontStyleRegular);
    }
    
    text = AfxMoney(trans->price);  // , false, trade->ticker_decimals);
    ld->SetData(7, trade, ticker_id, text, StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYMEDIUM, COLOR_WHITEDARK, font8, FontStyleRegular);

    ListBox_AddString(hListBox, ld);
}


// ========================================================================================
// Create the display data for a Other Income (for a Trade).
// ========================================================================================
void ListBoxData_OtherIncomeLeg(
    HWND hListBox, const std::shared_ptr<Trade>& trade, const std::shared_ptr<Transaction>& trans, const std::shared_ptr<Leg>& leg)
{
    ListBoxData* ld = new ListBoxData;

    TickerId ticker_id = -1;
    REAL font8 = 8;

    std::wstring text = L"OTHER";
    ld->SetData(2, trade, ticker_id, text, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYMEDIUM, COLOR_WHITEDARK, font8, FontStyleRegular);

    for (int i = 3; i < 8; i++) {
        ld->SetData(i, trade, ticker_id, L"", StringAlignmentNear, StringAlignmentCenter,
            COLOR_GRAYMEDIUM, COLOR_WHITEDARK, font8, FontStyleRegular);
    }
    
    text = AfxMoney(trans->price);  // , false, trade->ticker_decimals);
    ld->SetData(7, trade, ticker_id, text, StringAlignmentFar, StringAlignmentCenter,
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

    TickerId ticker_id = -1;
    REAL font8 = 8;

    std::wstring text = std::to_wstring(leg->original_quantity);
    ld->SetData(2, trade, ticker_id, text, StringAlignmentFar, StringAlignmentCenter,
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

    ld->SetData(3, trade, ticker_id, short_date, StringAlignmentCenter, StringAlignmentCenter,
        COLOR_GRAYLIGHT, COLOR_WHITELIGHT, font8, FontStyleRegular);

    ld->SetData(4, trade, ticker_id, days, StringAlignmentCenter, StringAlignmentCenter,
        COLOR_GRAYMEDIUM, COLOR_WHITEDARK, font8, FontStyleRegular);

    ld->SetData(5, trade, ticker_id, leg->strike_price, StringAlignmentCenter, StringAlignmentCenter,
        COLOR_GRAYLIGHT, COLOR_WHITELIGHT, font8, FontStyleRegular);

    ld->SetData(6, trade, ticker_id, L" " + db.PutCallToString(leg->put_call), StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYMEDIUM, COLOR_WHITEDARK, font8, FontStyleRegular);

    DWORD clr = COLOR_RED;
    if (leg->action == Action::BTO || leg->action == Action::BTC) clr = COLOR_GREEN;

    ld->SetData(7, trade, ticker_id, db.ActionToStringDescription(leg->action), StringAlignmentCenter, StringAlignmentCenter,
        COLOR_GRAYLIGHT, clr, font8, FontStyleRegular);

    ListBox_AddString(hListBox, ld);
}


// ========================================================================================
// Create the display data line for closed position yearly total.
// ========================================================================================
void ListBoxData_OutputClosedYearTotal(HWND hListBox, int year, double subtotal, int year_win, int year_loss) {
    ListBoxData* ld = new ListBoxData;

    TickerId ticker_id = -1;
    REAL font8 = 8;

    DWORD clr = (subtotal >= 0) ? COLOR_GREEN : COLOR_RED;
    
    if (year == 0) year = AfxGetYear(AfxCurrentDate());

    std::wstring text = std::to_wstring(year) + L" TOTAL";
    ld->SetData(3, nullptr, ticker_id, text, StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    ld->SetData(4, nullptr, ticker_id, AfxMoney(subtotal), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    // col 5 is a "spacer" column

    clr = (year_win >= year_loss) ? COLOR_GREEN : COLOR_RED;
    text = std::to_wstring(year_win) + L"W " + std::to_wstring(year_loss)
        + L"L  " + AfxMoney((double)year_win / (max(year_win + year_loss,1)) * 100, false, 0) + L"%";
    ld->SetData(6, nullptr, ticker_id, text, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    ListBox_InsertString(hListBox, 0, ld);
}


// ========================================================================================
// Create the display data line for total closed monthly total.
// ========================================================================================
void ListBoxData_OutputClosedMonthTotal(HWND hListBox, double monthly_total, int month_win, int month_loss) {
    ListBoxData* ld = new ListBoxData;

    TickerId ticker_id = -1;
    REAL font8 = 8;

    DWORD clr = (monthly_total >= 0) ? COLOR_GREEN : COLOR_RED;
    
    std::wstring text = L"THIS MONTH";
    ld->SetData(3, nullptr, ticker_id, text, StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    ld->SetData(4, nullptr, ticker_id, AfxMoney(monthly_total), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    // col 5 is a "spacer" column

    clr = (month_win >= month_loss) ? COLOR_GREEN : COLOR_RED;
    text = std::to_wstring(month_win) + L"W " + std::to_wstring(month_loss)
        + L"L  " + AfxMoney((double)month_win / (max(month_win + month_loss,1)) * 100, false, 0) + L"%";
    ld->SetData(6, nullptr, ticker_id, text, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    ListBox_InsertString(hListBox, 1, ld);
}


// ========================================================================================
// Create the display data line for total closed weekly total.
// ========================================================================================
void ListBoxData_OutputClosedWeekTotal(HWND hListBox, double weekly_total, int week_win, int week_loss) {
    ListBoxData* ld = new ListBoxData;

    TickerId ticker_id = -1;
    REAL font8 = 8;

    DWORD clr = (weekly_total >= 0) ? COLOR_GREEN : COLOR_RED;
    
    std::wstring text = L"THIS WEEK";
    ld->SetData(3, nullptr, ticker_id, text, StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    ld->SetData(4, nullptr, ticker_id, AfxMoney(weekly_total), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    // col 5 is a "spacer" column

    clr = (week_win >= week_loss) ? COLOR_GREEN : COLOR_RED;
    text = std::to_wstring(week_win) + L"W " + std::to_wstring(week_loss)
        + L"L  " + AfxMoney((double)week_win / (max(week_win + week_loss,1)) * 100, false, 0) + L"%";
    ld->SetData(6, nullptr, ticker_id, text, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    ListBox_InsertString(hListBox, 2, ld);
}


// ========================================================================================
// Create the display data line for total closed daily total.
// ========================================================================================
void ListBoxData_OutputClosedDayTotal(HWND hListBox, double daily_total, int day_win, int day_loss) {
    ListBoxData* ld = new ListBoxData;

    TickerId ticker_id = -1;
    REAL font8 = 8;

    DWORD clr = (daily_total >= 0) ? COLOR_GREEN : COLOR_RED;
    
    std::wstring text = L"TODAY";
    ld->SetData(3, nullptr, ticker_id, text, StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    ld->SetData(4, nullptr, ticker_id, AfxMoney(daily_total), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    // col 5 is a "spacer" column

    clr = (day_win >= day_loss) ? COLOR_GREEN : COLOR_RED;
    text = std::to_wstring(day_win) + L"W " + std::to_wstring(day_loss)
        + L"L  " + AfxMoney((double)day_win / (max(day_win + day_loss,1)) * 100, false, 0) + L"%";
    ld->SetData(6, nullptr, ticker_id, text, StringAlignmentNear, StringAlignmentCenter,
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
void ListBoxData_OutputClosedMonthSubtotal(
    HWND hListBox, const std::wstring& closed_date, double subtotal, int month_win, int month_loss)
{
    ListBoxData* ld = new ListBoxData;

    TickerId ticker_id = -1;
    REAL font8 = 8;

    DWORD clr = (subtotal >= 0) ? COLOR_GREEN : COLOR_RED;
    
    std::wstring text = AfxUpper(AfxGetLongMonthName(closed_date)) + L" " + std::to_wstring(AfxGetYear(closed_date));
    ld->SetData(3, nullptr, ticker_id, text, StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    ld->SetData(4, nullptr, ticker_id, AfxMoney(subtotal), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    // col 5 is a "spacer" column

    clr = (month_win >= month_loss) ? COLOR_GREEN : COLOR_RED;
    text = std::to_wstring(month_win) + L"W " + std::to_wstring(month_loss)
        + L"L  " + AfxMoney((double)month_win / (max(month_win + month_loss,1)) * 100, false, 0) + L"%";
    ld->SetData(6, nullptr, ticker_id, text, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    ListBox_AddString(hListBox, ld);
    ListBoxData_AddBlankLine(hListBox);
}


// ========================================================================================
// Create the display data line for a closed position.
// ========================================================================================
void ListBoxData_OutputClosedPosition(HWND hListBox, const std::shared_ptr<Trade>& trade, const std::wstring& closed_date) {
    ListBoxData* ld = new ListBoxData;

    TickerId ticker_id = -1;
    REAL font8 = 8;

    ld->SetData(0, trade, ticker_id, L"", StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_ORANGE, font8, FontStyleRegular);

    ld->SetData(1, trade, ticker_id, closed_date, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_ORANGE, font8, FontStyleRegular);

    ld->SetData(2, trade, ticker_id, trade->ticker_symbol, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_WHITELIGHT, font8, FontStyleRegular);

    std::wstring ticker_name = trade->ticker_name;
    if (config.IsFuturesTicker(trade->ticker_symbol)) ticker_name += L" (" + AfxFormatFuturesDate(trade->future_expiry) + L")";
    ld->SetData(3, trade, ticker_id, ticker_name, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_WHITELIGHT, font8, FontStyleRegular);

    DWORD clr = (trade->acb_total >= 0) ? COLOR_GREEN : COLOR_RED;
    ld->SetData(4, trade, ticker_id, AfxMoney(trade->acb_total), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    // col 5 is a "spacer" column

    ld->SetData(6, trade, ticker_id, config.GetCategoryDescription(trade->category), StringAlignmentNear, StringAlignmentCenter,
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

    TickerId ticker_id = -1;
    REAL font8 = 8;

    DWORD clr = (running_net_total >= 0) ? COLOR_GREEN : COLOR_RED;
    if (running_fees_total >= 0) running_fees_total = running_fees_total * -1;

    std::wstring text = L"TOTAL";
    ld->SetData(3, nullptr, ticker_id, text, StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    ld->SetData(5, nullptr, ticker_id, AfxMoney(running_gross_total), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    ld->SetData(6, nullptr, ticker_id, AfxMoney(running_fees_total), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    ld->SetData(7, nullptr, ticker_id, AfxMoney(running_net_total), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

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

    TickerId ticker_id = -1;
    REAL font8 = 8;

    ld->trans = trans;

    ld->SetData(0, trade, ticker_id, L"", StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_ORANGE, font8, FontStyleRegular);

    ld->SetData(1, trade, ticker_id, trans->trans_date, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_WHITELIGHT, font8, FontStyleRegular);

    ld->SetData(2, trade, ticker_id, trade->ticker_symbol, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_ORANGE, font8, FontStyleRegular);

    ld->SetData(3, trade, ticker_id, trans->description, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_WHITELIGHT, font8, FontStyleRegular);

    ld->SetData(4, trade, ticker_id, AfxMoney(trans->quantity), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_WHITELIGHT, font8, FontStyleRegular);

    ld->SetData(5, trade, ticker_id, AfxMoney(trans->price, false, config.GetTickerDecimals(trade->ticker_symbol)), 
        StringAlignmentFar, StringAlignmentCenter, COLOR_GRAYDARK, COLOR_WHITELIGHT, font8, FontStyleRegular);

    ld->SetData(6, trade, ticker_id, AfxMoney(trans->fees), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_WHITELIGHT, font8, FontStyleRegular);

    DWORD clr = (trans->total >= 0) ? COLOR_GREEN : COLOR_RED;
    ld->SetData(7, trade, ticker_id, AfxMoney(trans->total), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    // col 8 is a "spacer" column

    ld->SetData(9, trade, ticker_id, config.GetCategoryDescription(trade->category), StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_WHITEDARK, font8, FontStyleRegular);

    ListBox_AddString(hListBox, ld);
}


// ========================================================================================
// Create the display data line for a Ticker total.
// ========================================================================================
void ListBoxData_OutputTickerTotals(HWND hListBox, const std::wstring& ticker, double amount) {
    ListBoxData* ld = new ListBoxData;

    TickerId ticker_id = -1;
    REAL font8 = 8;

    ld->SetData(1, nullptr, ticker_id, ticker, StringAlignmentNear, StringAlignmentCenter,
        COLOR_GRAYDARK, COLOR_ORANGE, font8, FontStyleRegular);

    // Look up the Company name based on the tickerid
    auto iter = std::find_if(trades.begin(), trades.end(),
         [&](const auto t) { return (t->ticker_symbol == ticker && t->ticker_name.length()); });

    if (iter != trades.end()) {
        auto index = std::distance(trades.begin(), iter);
        ld->SetData(2, nullptr, ticker_id, trades.at(index)->ticker_name, StringAlignmentNear, StringAlignmentCenter,
            COLOR_GRAYDARK, COLOR_ORANGE, font8, FontStyleRegular);
    }

    DWORD clr = (amount >= 0) ? COLOR_GREEN : COLOR_RED;
    ld->SetData(3, nullptr, ticker_id, AfxMoney(amount), StringAlignmentFar, StringAlignmentCenter,
        COLOR_GRAYDARK, clr, font8, FontStyleRegular);

    ListBox_AddString(hListBox, ld);
}


// ========================================================================================
// Generic Header control WM_PAINT message handler (for all ListBox header controls)
// ========================================================================================
void Header_OnPaint(HWND hwnd) {
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    int width = (ps.rcPaint.right - ps.rcPaint.left);
    int height = (ps.rcPaint.bottom - ps.rcPaint.top);

    Graphics graphics(hdc);
    SolidBrush back_brush(COLOR_GRAYMEDIUM);
    graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, width, height);

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

    int item_count = Header_GetItemCount(hwnd);

    for (int i = 0; i < item_count; i++) {
        Header_GetItemRect(hwnd, i, &rcItem);

        switch (Header_GetItemAlignment(hwnd, i))
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
        std::wstring text = Header_GetItemText(hwnd, i);
        graphics.DrawString(text.c_str(), -1, &font, rcText, &stringF, &text_brush);

    }

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Process WM_DRAWITEM message for window/dialog 
// (common function for custom drawing listbox data)
// ========================================================================================
void ListBoxData_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT* lpDrawItem) {
   if (lpDrawItem->itemID == -1) return;

    if (lpDrawItem->itemAction == ODA_DRAWENTIRE ||
        lpDrawItem->itemAction == ODA_SELECT) {

        int width = (lpDrawItem->rcItem.right - lpDrawItem->rcItem.left);
        int height = (lpDrawItem->rcItem.bottom - lpDrawItem->rcItem.top);

        bool is_hot = false;

        SaveDC(lpDrawItem->hDC);

        HDC memDC = NULL;         // Double buffering
        HBITMAP hbit = NULL;      // Double buffering

        memDC = CreateCompatibleDC(lpDrawItem->hDC);
        hbit = CreateCompatibleBitmap(lpDrawItem->hDC, width, height);
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
        REAL font_size = 10;
        int font_style = FontStyleRegular;

        StringAlignment HAlignment = StringAlignmentNear;
        StringAlignment VAlignment = StringAlignmentCenter;

        // Paint the full width background using brush 
        SolidBrush back_brush(back_color);
        graphics.FillRectangle(&back_brush, 0, 0, width, height);

        // Get the current ListBox line data should a valid line exist
        // Paint the individual columns with the specific data.
        ListBoxData* ld = (ListBoxData*)(lpDrawItem->itemData);
        int left = 0;
        int column_width = 0;
        int column_start = 0;
        int column_end = MAX_COLUMNS;

        // If this is a Category separator line then we need to draw it differently
        // then a regular that would be drawn (full line width)
        if (ld) {
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
            font_size = ld->col[i].font_size;
            font_style = ld->col[i].font_style;

            column_width = (ld->line_type == LineType::category_header) ? width : AfxScaleX((float)ld->col[i].column_width);

            back_brush.SetColor(back_color);
            graphics.FillRectangle(&back_brush, left, 0, column_width, height);

            Font         font(&fontFamily, font_size, font_style, Unit::UnitPoint);
            SolidBrush   text_brush(text_color);
            StringFormat stringF(StringFormatFlagsNoWrap);
            stringF.SetAlignment(HAlignment);
            stringF.SetLineAlignment(VAlignment);

            // If right alignment then add a very small amount of right side
            // padding so that text is not pushed up right against the right side.
            int rightPadding = 0;
            if (HAlignment == StringAlignmentFar) rightPadding = AfxScaleX(2);

            RectF rcText((REAL)left, (REAL)0, (REAL)column_width - rightPadding, (REAL)height);
            graphics.DrawString(text.c_str(), -1, &font, rcText, &stringF, &text_brush);

            left += column_width;
        }

        BitBlt(lpDrawItem->hDC, lpDrawItem->rcItem.left,
            lpDrawItem->rcItem.top, width, height, memDC, 0, 0, SRCCOPY);

        // Cleanup
        RestoreDC(lpDrawItem->hDC, -1);
        if (hbit) DeleteObject(hbit);
        if (memDC) DeleteDC(memDC);
    }
}

