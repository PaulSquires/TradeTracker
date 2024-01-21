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
#include "CustomLabel/CustomLabel.h"
#include "CustomTextBox/CustomTextBox.h"
#include "CustomVScrollBar/CustomVScrollBar.h"
#include "Utilities/ListBoxData.h"
#include "MainWindow/MainWindow.h"
#include "Transactions/TransDetail.h"
#include "Database/trade.h"
#include "Database/database.h"
#include "Utilities/AfxWin.h"
#include "Config/Config.h"
#include "TradeHistory.h"


HWND HWND_TRADEHISTORY = NULL;

CTradeHistory TradeHistory;

std::shared_ptr<Trade> tradeNotes = nullptr;


void TradeHistory_OnSize(HWND hwnd, UINT state, int cx, int cy);


// ========================================================================================
// Populate the History ListBox with the current active/open trades
// ========================================================================================
void TradeHistory_ShowTradesHistoryTable(std::shared_ptr<Trade>& trade) {
    HWND hListBox = GetDlgItem(HWND_TRADEHISTORY, IDC_HISTORY_LISTBOX);
    HWND hCustomVScrollBar = GetDlgItem(HWND_TRADEHISTORY, IDC_HISTORY_CUSTOMVSCROLLBAR);
    HWND hSymbol = GetDlgItem(HWND_TRADEHISTORY, IDC_HISTORY_SYMBOL);
    HWND hNotesText = GetDlgItem(HWND_TRADEHISTORY, IDC_HISTORY_TXTNOTES);

    // Ensure that the Trade History panel is set
    MainWindow.SetRightPanel(HWND_TRADEHISTORY);

    if (!trade) {
        // Clear the current trade history table
        ListBoxData_DestroyItemData(hListBox);
        CustomTextBox_SetText(hNotesText, L"");
        CustomLabel_SetText(hSymbol, L"Trade History");
        ListBoxData_AddBlankLine(hListBox);
        RECT rc; GetClientRect(HWND_TRADEHISTORY, &rc);
        TradeHistory_OnSize(HWND_TRADEHISTORY, 0, rc.right, rc.bottom);
        return;
    }

    tradeNotes = trade;

    // Prevent ListBox redrawing until all calculations are completed
    SendMessage(hListBox, WM_SETREDRAW, false, 0);
    
    std::wstring ticker = trade->ticker_symbol + L": " + trade->ticker_name;
    if (config.IsFuturesTicker(trade->ticker_symbol)) ticker += L" (" + AfxFormatFuturesDate(trade->future_expiry) + L")";
    CustomLabel_SetText(hSymbol, ticker);

    CustomTextBox_SetText(hNotesText, trade->notes);

    // Clear the current trade history table
    ListBoxData_DestroyItemData(hListBox);

    // Show the final rolled up Open position for this trade
    ListBoxData_OpenPosition(hListBox, trade, -1);
        
    // Output the BP, Days, ROI
    ListBoxData_TradeROI(hListBox, trade, -1);

    // Sort transactions based on date
    std::sort(trade->transactions.begin(), trade->transactions.end(),
        [](const auto trans1, const auto trans2) {
            {
                if (trans1->trans_date < trans2->trans_date) return true;
                return false;
            }
        });

    // Read the TransDetail in reverse so that the newest history TransDetail get displayed first
    for (int i = (int)trade->transactions.size() - 1; i >= 0; --i) {
        auto trans = trade->transactions.at(i);

        ListBoxData_HistoryHeader(hListBox, trade, trans);

        // Show the detail leg information for this transaction.
        for (const auto& leg : trans->legs) {
            switch (leg->underlying) {
            case Underlying::Options:
                ListBoxData_HistoryOptionsLeg(hListBox, trade, trans, leg); 
                break;
            case Underlying::Shares:
            case Underlying::Futures:
                ListBoxData_HistorySharesLeg(hListBox, trade, trans, leg);
                break;
            case Underlying::Dividend:
                ListBoxData_HistoryDividendLeg(hListBox, trade, trans, leg);
                break;
            case Underlying::Other:
                ListBoxData_OtherIncomeLeg(hListBox, trade, trans, leg);
                break;
            }
        }
    }
    ListBoxData_AddBlankLine(hListBox);

    // Calculate the actual column widths based on the size of the strings in
    // ListBoxData while respecting the minimum values as defined in nMinColWidth[].
    // This function is also called when receiving new price data from TWS because
    // that data may need the column width to be wider.
    ListBoxData_ResizeColumnWidths(hListBox, TableType::trade_history);

    // Set the ListBox to the topline.
    ListBox_SetTopIndex(hListBox, 0);

    // Redraw the ListBox to ensure that any recalculated columns are 
    // displayed correctly. Re-enable redraw.
    SendMessage(hListBox, WM_SETREDRAW, true, 0);
    AfxRedrawWindow(hListBox);

    RECT rc; GetClientRect(HWND_TRADEHISTORY, &rc);
    TradeHistory_OnSize(HWND_TRADEHISTORY, 0, rc.right, rc.bottom);

    CustomVScrollBar_Recalculate(hCustomVScrollBar);
}


// ========================================================================================
// Listbox subclass Window procedure
// ========================================================================================
LRESULT CALLBACK TradeHistory_ListBox_SubclassProc(
    HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    // Create static accumulation variable to collect the data from
    // a series of middle mouse wheel scrolls.
    static int accum_delta = 0;

    // Track the last hot Transaction line
    static int last_hot_index = 0;

    switch (uMsg) {

    case WM_MOUSEWHEEL: {
        // Accumulate delta until scroll one line (up +120, down -120). 
        // 120 is the Microsoft default delta
        int zdelta = GET_WHEEL_DELTA_WPARAM(wParam);
        int top_index = (int)SendMessage(hwnd, LB_GETTOPINDEX, 0, 0);
        accum_delta += zdelta;
        if (accum_delta >= 120) {     // scroll up 3 lines
            top_index -= 3;
            top_index = max(0, top_index);
            SendMessage(hwnd, LB_SETTOPINDEX, top_index, 0);
            accum_delta = 0;
        }
        else {
            if (accum_delta <= -120) {     // scroll down 3 lines
                top_index += +3;
                SendMessage(hwnd, LB_SETTOPINDEX, top_index, 0);
                accum_delta = 0;
            }
        }
        HWND hCustomVScrollBar = GetDlgItem(HWND_TRADEHISTORY, IDC_HISTORY_CUSTOMVSCROLLBAR);
        CustomVScrollBar_Recalculate(hCustomVScrollBar);
        return 0;
    }

    case WM_LBUTTONDOWN: {
        int idx = Listbox_ItemFromPoint(hwnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        // The return value contains the index of the nearest item in the LOWORD. The HIWORD is zero 
        // if the specified point is in the client area of the list box, or one if it is outside the 
        // client area.
        if (HIWORD(idx) == 1) break;

        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hwnd, idx);
        if (ld != nullptr) {
            if (ld->line_type == LineType::transaction_header) {
                TransDetail.ShowTransDetail(ld->trade, ld->trans);
            }
        }

        break;
    }

    case WM_MOUSEMOVE: {
        // Track that we are over the control in order to catch the 
        // eventual WM_MOUSEHOVER and WM_MOUSELEAVE events
        TRACKMOUSEEVENT tme{};
        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = hwnd;
        TrackMouseEvent(&tme);

        int idx = Listbox_ItemFromPoint(hwnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        // The return value contains the index of the nearest item in the LOWORD. The HIWORD is zero 
        // if the specified point is in the client area of the list box, or one if it is outside the 
        // client area.
        // If we are over a Transaction line then display the "magnify glass" icon that indicates to
        // the user that clicking on it will take them directly to the transaction details.
        
        // Hot highlight the new line 
        if (HIWORD(idx) != 1 && idx > -1) {
            ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hwnd, idx);
            if (ld != nullptr) {
                if (ld->line_type == LineType::transaction_header && idx != last_hot_index) {
                    ld->col[8].text_theme = COLOR_WHITEDARK;

                    // Update the Text color for the previous highlight line (make icon hidden)
                    ld = (ListBoxData*)ListBox_GetItemData(hwnd, last_hot_index);
                    if (ld != nullptr) {
                        ld->col[8].text_theme = COLOR_GRAYDARK;
                    }
                    last_hot_index = idx;
                    AfxRedrawWindow(hwnd);
                }
            }
        }

        return 0;
    }

    case WM_MOUSELEAVE: {
        // Update the Text color for the previous highlight line
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hwnd, last_hot_index);
        if (ld) {
            ld->col[8].text_theme = COLOR_GRAYDARK;
        }
        AfxRedrawWindow(hwnd);
        last_hot_index = 0;
        return 0;
    }

    case WM_ERASEBKGND: {
        // If the number of lines in the listbox maybe less than the number per page then 
        // calculate from last item to bottom of listbox, otherwise calculate based on
        // the mod of the lineheight to listbox height so we can color the partial line
        // that won't be displayed at the bottom of the list.
        RECT rc; GetClientRect(hwnd, &rc);
        
        RECT rcItem{};
        SendMessage(hwnd, LB_GETITEMRECT, 0, (LPARAM)&rcItem);
        int item_height = (rcItem.bottom - rcItem.top);
        int items_count = ListBox_GetCount(hwnd);
        int top_index = (int)SendMessage(hwnd, LB_GETTOPINDEX, 0, 0);
        int visible_rows = 0;
        int items_per_page = 0;
        int bottom_index = 0;
        int width = (rc.right - rc.left);
        int height = (rc.bottom - rc.top);

        if (items_count) {
            items_per_page = (height) / item_height;
            bottom_index = (top_index + items_per_page);
            if (bottom_index >= items_count)
                bottom_index = items_count - 1;
            visible_rows = (bottom_index - top_index) + 1;
            rc.top = visible_rows * item_height;
        }

        if (rc.top < rc.bottom) {
            height = (rc.bottom - rc.top);
            HDC hDC = (HDC)wParam;
            Graphics graphics(hDC);
            Color back_color(COLOR_GRAYDARK);
            SolidBrush back_brush(back_color);
            graphics.FillRectangle(&back_brush, rc.left, rc.top, width, height);
        }

        ValidateRect(hwnd, &rc);
        return true;
    }

    case WM_DESTROY: {
        // Destroy all manually allocated ListBox display data that is held
        // in the LineData structures..
        ListBoxData_DestroyItemData(hwnd);

        // REQUIRED: Remove control subclassing
        RemoveWindowSubclass(hwnd, TradeHistory_ListBox_SubclassProc, uIdSubclass);
        break;
    }

    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}


// ========================================================================================
// Process WM_MEASUREITEM message for window/dialog: TradeHistory
// ========================================================================================
void TradeHistory_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT* lpMeasureItem) {
    lpMeasureItem->itemHeight = AfxScaleY(HISTORY_LISTBOX_ROWHEIGHT);
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: TradeHistory
// ========================================================================================
bool TradeHistory_OnEraseBkgnd(HWND hwnd, HDC hdc) {
    // Handle all of the painting in WM_PAINT
    return true;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: TradeHistory
// ========================================================================================
void TradeHistory_OnPaint(HWND hwnd) {
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    Graphics graphics(hdc);

    // Create the background brush
    SolidBrush back_brush(COLOR_GRAYDARK);

    // Paint the background using brush.
    int width = (ps.rcPaint.right - ps.rcPaint.left);
    int height = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, width, height);

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Process WM_COMMAND message for window/dialog: TradeHistory
// ========================================================================================
void TradeHistory_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {
    static bool is_notes_dirty = false;
    static std::shared_ptr<Trade> trade = nullptr;
    static std::wstring notes = L"";

    switch (id) {
    case IDC_HISTORY_TXTNOTES: {
        if (codeNotify == EN_KILLFOCUS) {
            if (is_notes_dirty == true) {
                // Clean the notes to ensure that there are no embedded "|" pipe characters
                // that would cause the loading routine to incorrectly parse.
                if (trade) trade->notes = AfxReplace(notes, L"|", L" ");

                // Save the database because the Notes for this Trade has been updated.
                db.SaveDatabase();

                is_notes_dirty = false;
                trade = nullptr;
                notes = L"";
            }
            break;
        }
        else if (codeNotify == EN_CHANGE) {
            notes = CustomTextBox_GetText(hwndCtl);
            is_notes_dirty = true;
            break;
        }
        else if (codeNotify == EN_SETFOCUS) {
            trade = tradeNotes;
            is_notes_dirty = false;
            break;
        }

        break;
    }

    }
}


// ========================================================================================
// Process WM_SIZE message for window/dialog: TradeHistory
// ========================================================================================
void TradeHistory_OnSize(HWND hwnd, UINT state, int cx, int cy) {
    HWND hListBox = GetDlgItem(hwnd, IDC_HISTORY_LISTBOX);
    HWND hCustomVScrollBar = GetDlgItem(hwnd, IDC_HISTORY_CUSTOMVSCROLLBAR);
    HWND hSeparator = GetDlgItem(hwnd, IDC_HISTORY_SEPARATOR);
    HWND hNotesLabel = GetDlgItem(hwnd, IDC_HISTORY_LBLNOTES);
    HWND hNotesTextBox = GetDlgItem(hwnd, IDC_HISTORY_TXTNOTES);

    int margin = AfxScaleY(TRADEHISTORY_MARGIN);

    // If no entries exist for the ListBox then don't show any child controls
    int show_flag = (ListBox_GetCount(hListBox) <= 1) ? SWP_HIDEWINDOW : SWP_SHOWWINDOW;

    HDWP hdwp = BeginDeferWindowPos(10);

    // Move and size the top label into place
    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_HISTORY_SYMBOL), 0,
        0, 0, cx, margin, SWP_NOZORDER | SWP_SHOWWINDOW);

    // Do not call the calcVThumbRect() function during a scrollbar move. This WM_SIZE
    // gets triggered when the ListBox WM_DRAWITEM fires. If we do another calcVThumbRect()
    // calcualtion then the scrollbar will appear "jumpy" under the user's mouse cursor.
    bool show_scrollbar = false;
    CustomVScrollBar* pData = CustomVScrollBar_GetPointer(hCustomVScrollBar);
    if (pData) {
        if (pData->drag_active) {
            show_scrollbar = true;
        }
        else {
            show_scrollbar = pData->calcVThumbRect();
        }
    }
    int custom_scrollbar_width = (show_scrollbar ? AfxScaleX(CUSTOMVSCROLLBAR_WIDTH) : 0);

    int height_notes_textbox = AfxScaleY(100);
    int height_notes_label = AfxScaleY(24);

    int left = 0;
    int top = margin;
    int width = cx - custom_scrollbar_width;
    int height = cy - top - height_notes_textbox - height_notes_label - AfxScaleY(10);
    hdwp = DeferWindowPos(hdwp, hListBox, 0, left, top, width, height, SWP_NOZORDER | show_flag);

    left = left + width;   // right edge of ListBox
    top = margin;
    width = custom_scrollbar_width;

    int int_show_scrollbar = (show_scrollbar ? SWP_SHOWWINDOW : SWP_HIDEWINDOW);
    if (show_flag == SWP_HIDEWINDOW) int_show_scrollbar = SWP_HIDEWINDOW;
    hdwp = DeferWindowPos(hdwp, hCustomVScrollBar, 0, left, top, width, height,
        SWP_NOZORDER | int_show_scrollbar);

    left = 0;
    top = top + height + AfxScaleY(5);
    width = cx;
    hdwp = DeferWindowPos(hdwp, hSeparator, 0, left, top, width, height, SWP_NOZORDER | show_flag);

    left = 0;
    top = cy - height_notes_textbox - height_notes_label;
    width = cx;
    hdwp = DeferWindowPos(hdwp, hNotesLabel, 0, left, top, width, height_notes_label, SWP_NOZORDER | show_flag);

    left = 0;
    top = cy - height_notes_textbox;
    width = cx;
    hdwp = DeferWindowPos(hdwp, hNotesTextBox, 0, left, top, width, height_notes_textbox, SWP_NOZORDER | show_flag);

    EndDeferWindowPos(hdwp);
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: TradeHistory
// ========================================================================================
bool TradeHistory_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct) {
    HWND_TRADEHISTORY = hwnd;

    HWND hCtl = CustomLabel_SimpleLabel(hwnd, IDC_HISTORY_SYMBOL, L"Trade History",
        COLOR_WHITELIGHT, COLOR_BLACK);

    // Create an Ownerdraw listbox that we will use to custom paint our various open trades.
    hCtl =
        TradeHistory.AddControl(Controls::ListBox, hwnd, IDC_HISTORY_LISTBOX, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_NOSEL |
            LBS_OWNERDRAWFIXED | LBS_NOTIFY,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)TradeHistory_ListBox_SubclassProc,
            IDC_HISTORY_LISTBOX, NULL);
    ListBox_AddString(hCtl, NULL);

    // Create our custom vertical scrollbar and attach the ListBox to it.
    CreateCustomVScrollBar(hwnd, IDC_HISTORY_CUSTOMVSCROLLBAR, hCtl, Controls::ListBox);

    CustomLabel_SimpleLabel(hwnd, IDC_HISTORY_LBLNOTES, L"Notes", COLOR_WHITEDARK, COLOR_GRAYDARK,
        CustomLabelAlignment::middle_left, 0, 0, 0, 0);
    hCtl = CreateCustomTextBox(hwnd, IDC_HISTORY_TXTNOTES, true, ES_LEFT,
        L"", 0, 0, 0, 0);
    CustomTextBox_SetMargins(hCtl, 3, 3);
    CustomTextBox_SetColors(hCtl, COLOR_WHITELIGHT, COLOR_GRAYDARK);
    CustomTextBox_SetSelectOnFocus(hCtl, false);

    CustomLabel* pData = nullptr;
    hCtl = CreateCustomLabel(
        hwnd, IDC_HISTORY_SEPARATOR,
        CustomLabelType::line_horizontal,
        0, 0, 0, 0);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->back_color = COLOR_GRAYDARK;
        pData->line_color = COLOR_SEPARATOR;
        pData->line_width = 2;
        CustomLabel_SetOptions(hCtl, pData);
    }

    return true;
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CTradeHistory::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        HANDLE_MSG(m_hwnd, WM_CREATE, TradeHistory_OnCreate);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, TradeHistory_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, TradeHistory_OnPaint);
        HANDLE_MSG(m_hwnd, WM_COMMAND, TradeHistory_OnCommand);
        HANDLE_MSG(m_hwnd, WM_SIZE, TradeHistory_OnSize);
        HANDLE_MSG(m_hwnd, WM_MEASUREITEM, TradeHistory_OnMeasureItem);
        HANDLE_MSG(m_hwnd, WM_DRAWITEM, ListBoxData_OnDrawItem);
    }
    return DefWindowProc(m_hwnd, msg, wParam, lParam);
}

