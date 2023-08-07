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
#include "CustomLabel/CustomLabel.h"
#include "CustomTextBox/CustomTextBox.h"
#include "CustomVScrollBar/CustomVScrollBar.h"
#include "Utilities/ListBoxData.h"
#include "MainWindow/MainWindow.h"
#include "SideMenu/SideMenu.h"
#include "Transactions/TransDetail.h"
#include "Database/trade.h"
#include "Utilities/AfxWin.h"
#include "TradeHistory.h"


HWND HWND_TRADEHISTORY = NULL;

CTradeHistory TradeHistory;

// Save the Trade pointer in module scope because we need it when dealing with
// the Notes text in order to save that text to the correct Trade.
std::shared_ptr<Trade> tradeHistoryPtr = nullptr;



// ========================================================================================
// Populate the History ListBox with the current active/open trades
// ========================================================================================
void TradeHistory_ShowTradesHistoryTable(const std::shared_ptr<Trade>& trade)
{
    HWND hListBox = GetDlgItem(HWND_TRADEHISTORY, IDC_HISTORY_LISTBOX);
    HWND hCustomVScrollBar = GetDlgItem(HWND_TRADEHISTORY, IDC_HISTORY_CUSTOMVSCROLLBAR);

    tradeHistoryPtr = trade;

    // Ensure that the Trade History panel is set
    MainWindow_SetRightPanel(HWND_TRADEHISTORY);


    if (trade == nullptr) {
        // Clear the current trade history table
        ListBoxData_DestroyItemData(hListBox);
        CustomTextBox_SetText(GetDlgItem(HWND_TRADEHISTORY, IDC_HISTORY_TXTNOTES), L"");
        CustomLabel_SetText(GetDlgItem(HWND_TRADEHISTORY, IDC_HISTORY_SYMBOL), L"");
        ListBoxData_AddBlankLine(hListBox);
        AfxRedrawWindow(hListBox);
        return;
    }


    // Prevent ListBox redrawing until all calculations are completed
    SendMessage(hListBox, WM_SETREDRAW, FALSE, 0);

    CustomLabel_SetText(
        GetDlgItem(HWND_TRADEHISTORY, IDC_HISTORY_SYMBOL),
        trade->tickerSymbol + L": " + trade->tickerName);

    CustomTextBox_SetText(
        GetDlgItem(HWND_TRADEHISTORY, IDC_HISTORY_TXTNOTES), 
        trade->notes);


    // Clear the current trade history table
    ListBoxData_DestroyItemData(hListBox);

    // Show the final rolled up Open position for this trade
    ListBoxData_OpenPosition(hListBox, trade, -1);
        
    // Output the BP, Days, ROI
    ListBoxData_TradeROI(hListBox, trade, -1);



    // Read the TransDetail in reverse so that the newest history TransDetail get displayed first
    for (int i = trade->Transactions.size() - 1; i >= 0; --i) {
        auto trans = trade->Transactions.at(i);

        ListBoxData_HistoryHeader(hListBox, trade, trans);

        // Show the detail leg information for this transaction.
        for (const auto& leg : trans->legs) {
            if (leg->underlying == L"OPTIONS") { ListBoxData_HistoryOptionsLeg(hListBox, trade, trans, leg); }
            else if (leg->underlying == L"SHARES") { ListBoxData_HistorySharesLeg(hListBox, trade, trans, leg); }
            else if (leg->underlying == L"FUTURES") {
                ListBoxData_HistorySharesLeg(hListBox, trade, trans, leg);
            }
        }
    }
    ListBoxData_AddBlankLine(hListBox);


    // Calculate the actual column widths based on the size of the strings in
    // ListBoxData while respecting the minimum values as defined in nMinColWidth[].
    // This function is also called when receiving new price data from TWS because
    // that data may need the column width to be wider.
    ListBoxData_ResizeColumnWidths(hListBox, TableType::TradeHistory, -1);


    // Set the ListBox to the topline.
    ListBox_SetTopIndex(hListBox, 0);

    // Redraw the ListBox to ensure that any recalculated columns are 
    // displayed correctly. Re-enable redraw.
    SendMessage(hListBox, WM_SETREDRAW, TRUE, 0);
    AfxRedrawWindow(hListBox);

    CustomVScrollBar_Recalculate(hCustomVScrollBar);

}



// ========================================================================================
// Listbox subclass Window procedure
// ========================================================================================
LRESULT CALLBACK TradeHistory_ListBox_SubclassProc(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    // Create static accumulation variable to collect the data from
    // a series of middle mouse wheel scrolls.
    static int accumDelta = 0;

    // Track the last hot Transaction line
    static int idxLastHot = 0;


    switch (uMsg)
    {

    case WM_MOUSEWHEEL:
    {
        // Accumulate delta until scroll one line (up +120, down -120). 
        // 120 is the Microsoft default delta
        int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        int nTopIndex = SendMessage(hWnd, LB_GETTOPINDEX, 0, 0);
        accumDelta += zDelta;
        if (accumDelta >= 120) {     // scroll up 3 lines
            nTopIndex -= 3;
            nTopIndex = max(0, nTopIndex);
            SendMessage(hWnd, LB_SETTOPINDEX, nTopIndex, 0);
            accumDelta = 0;
        }
        else {
            if (accumDelta <= -120) {     // scroll down 3 lines
                nTopIndex += +3;
                SendMessage(hWnd, LB_SETTOPINDEX, nTopIndex, 0);
                accumDelta = 0;
            }
        }
        HWND hCustomVScrollBar = GetDlgItem(HWND_TRADEHISTORY, IDC_HISTORY_CUSTOMVSCROLLBAR);
        CustomVScrollBar_Recalculate(hCustomVScrollBar);
        return 0;
        break;
    }


    case WM_LBUTTONDOWN:
    {
        int idx = Listbox_ItemFromPoint(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        // The return value contains the index of the nearest item in the LOWORD. The HIWORD is zero 
        // if the specified point is in the client area of the list box, or one if it is outside the 
        // client area.
        if (HIWORD(idx) == 1) break;

        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hWnd, idx);
        if (ld != nullptr) {
            if (ld->lineType == LineType::TransactionHeader) {
                TransDetail_ShowTransDetail(ld->trade, ld->trans);
            }
        }

    }
    break;


    case WM_MOUSEMOVE:
    {
        // Track that we are over the control in order to catch the 
        // eventual WM_MOUSEHOVER and WM_MOUSELEAVE events
        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = hWnd;
        TrackMouseEvent(&tme);

        int idx = Listbox_ItemFromPoint(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        // The return value contains the index of the nearest item in the LOWORD. The HIWORD is zero 
        // if the specified point is in the client area of the list box, or one if it is outside the 
        // client area.
        // If we are over a Transaction line then display the "magnify glass" icon that indicates to
        // the user that clicking on it will take them directly to the transaction details.
        
        // Hot highlight the new line 
        if (HIWORD(idx) != 1 && idx > -1) {
            ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hWnd, idx);
            if (ld != nullptr) {
                if (ld->lineType == LineType::TransactionHeader && idx != idxLastHot) {
                    ld->col[8].textTheme = COLOR_WHITEDARK;

                    // Update the Text color for the previous highlight line (make icon hidden)
                    ld = (ListBoxData*)ListBox_GetItemData(hWnd, idxLastHot);
                    if (ld != nullptr) {
                        ld->col[8].textTheme = COLOR_GRAYDARK;
                    }
                    idxLastHot = idx;
                    AfxRedrawWindow(hWnd);
                }
            }
        }

        return 0;
    }
    break;


    case WM_MOUSELEAVE:
    {
        // Update the Text color for the previous highlight line
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hWnd, idxLastHot);
        if (ld != nullptr) {
            ld->col[8].textTheme = COLOR_GRAYDARK;
        }
        AfxRedrawWindow(hWnd);
        idxLastHot = 0;
        return 0;
    }
    break;


    case WM_ERASEBKGND:
    {
        // If the number of lines in the listbox maybe less than the number per page then 
        // calculate from last item to bottom of listbox, otherwise calculate based on
        // the mod of the lineheight to listbox height so we can color the partial line
        // that won't be displayed at the bottom of the list.
        RECT rc; GetClientRect(hWnd, &rc);
        
        RECT rcItem{};
        SendMessage(hWnd, LB_GETITEMRECT, 0, (LPARAM)&rcItem);
        int itemHeight = (rcItem.bottom - rcItem.top);
        int NumItems = ListBox_GetCount(hWnd);
        int nTopIndex = SendMessage(hWnd, LB_GETTOPINDEX, 0, 0);
        int visible_rows = 0;
        int ItemsPerPage = 0;
        int bottom_index = 0;
        int nWidth = (rc.right - rc.left);
        int nHeight = (rc.bottom - rc.top);

        if (NumItems > 0) {
            ItemsPerPage = (nHeight) / itemHeight;
            bottom_index = (nTopIndex + ItemsPerPage);
            if (bottom_index >= NumItems)
                bottom_index = NumItems - 1;
            visible_rows = (bottom_index - nTopIndex) + 1;
            rc.top = visible_rows * itemHeight;
        }

        if (rc.top < rc.bottom) {
            nHeight = (rc.bottom - rc.top);
            HDC hDC = (HDC)wParam;
            Graphics graphics(hDC);
            Color backColor(COLOR_GRAYDARK);
            SolidBrush backBrush(backColor);
            graphics.FillRectangle(&backBrush, rc.left, rc.top, nWidth, nHeight);
        }

        ValidateRect(hWnd, &rc);
        return TRUE;
        break;

    }


    case WM_DESTROY:
        // Destroy all manually allocated ListBox display data that is held
        // in the LineData structures..
        ListBoxData_DestroyItemData(hWnd);

        // REQUIRED: Remove control subclassing
        RemoveWindowSubclass(hWnd, TradeHistory_ListBox_SubclassProc, uIdSubclass);
        break;


    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);

}



// ========================================================================================
// Process WM_MEASUREITEM message for window/dialog: TradeHistory
// ========================================================================================
void TradeHistory_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT* lpMeasureItem)
{
    lpMeasureItem->itemHeight = AfxScaleY(HISTORY_LISTBOX_ROWHEIGHT);
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: TradeHistory
// ========================================================================================
BOOL TradeHistory_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    // Handle all of the painting in WM_PAINT
    return TRUE;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: TradeHistory
// ========================================================================================
void TradeHistory_OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    Graphics graphics(hdc);

    // Create the background brush
    SolidBrush backBrush(COLOR_GRAYDARK);

    // Paint the background using brush.
    int nWidth = (ps.rcPaint.right - ps.rcPaint.left);
    int nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&backBrush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Process WM_COMMAND message for window/dialog: TradeHistory
// ========================================================================================
void TradeHistory_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    static bool isNotesDirty = false;

    switch (id)
    {
    case (IDC_HISTORY_TXTNOTES):
        if (codeNotify == EN_KILLFOCUS) {
            if (tradeHistoryPtr != nullptr) {
                // Get the Notes text and save it into the Trade
                tradeHistoryPtr->notes = AfxGetWindowText(hwndCtl);

                // Save the database because the Notes for this Trade has been updated.
                SaveDatabase();
            }

            isNotesDirty = false;
            break;
        }

        if (codeNotify == EN_CHANGE) {
            isNotesDirty = true;
            break;
        }

        if (codeNotify == EN_SETFOCUS) {
            isNotesDirty = false;
            break;
        }

    default:
        break;
    }
}


// ========================================================================================
// Process WM_SIZE message for window/dialog: TradeHistory
// ========================================================================================
void TradeHistory_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    HWND hListBox = GetDlgItem(hwnd, IDC_HISTORY_LISTBOX);
    HWND hCustomVScrollBar = GetDlgItem(hwnd, IDC_HISTORY_CUSTOMVSCROLLBAR);
    HWND hSeparator = GetDlgItem(hwnd, IDC_HISTORY_SEPARATOR);
    HWND hNotesLabel = GetDlgItem(hwnd, IDC_HISTORY_LBLNOTES);
    HWND hNotesTextBox = GetDlgItem(hwnd, IDC_HISTORY_TXTNOTES);

    int margin = AfxScaleY(TRADEHISTORY_MARGIN);

    HDWP hdwp = BeginDeferWindowPos(10);

    // Move and size the top label into place
    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_HISTORY_SYMBOL), 0,
        0, 0, cx, margin, SWP_NOZORDER | SWP_SHOWWINDOW);

    // Do not call the calcVThumbRect() function during a scrollbar move. This WM_SIZE
    // gets triggered when the ListBox WM_DRAWITEM fires. If we do another calcVThumbRect()
    // calcualtion then the scrollbar will appear "jumpy" under the user's mouse cursor.
    bool bShowScrollBar = false;
    CustomVScrollBar* pData = CustomVScrollBar_GetPointer(hCustomVScrollBar);
    if (pData != nullptr) {
        if (pData->bDragActive) {
            bShowScrollBar = true;
        }
        else {
            bShowScrollBar = pData->calcVThumbRect();
        }
    }
    int CustomVScrollBarWidth = bShowScrollBar ? AfxScaleX(CUSTOMVSCROLLBAR_WIDTH) : 0;

    int heightNotesTextBox = AfxScaleY(100);
    int heightNotesLabel = AfxScaleY(24);

    int nLeft = 0;
    int nTop = margin;
    int nWidth = cx - CustomVScrollBarWidth;
    int nHeight = cy - nTop - heightNotesTextBox - heightNotesLabel - AfxScaleY(10);
    hdwp = DeferWindowPos(hdwp, hListBox, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nLeft = nLeft + nWidth;   // right edge of ListBox
    nTop = margin;
    nWidth = CustomVScrollBarWidth;
    hdwp = DeferWindowPos(hdwp, hCustomVScrollBar, 0, nLeft, nTop, nWidth, nHeight,
        SWP_NOZORDER | (bShowScrollBar ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));

    nLeft = 0;
    nTop = nTop + nHeight;
    nWidth = cx;
    hdwp = DeferWindowPos(hdwp, hSeparator, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nLeft = 0;
    nTop = cy - heightNotesTextBox - heightNotesLabel;
    nWidth = cx;
    hdwp = DeferWindowPos(hdwp, hNotesLabel, 0, nLeft, nTop, nWidth, heightNotesLabel, SWP_NOZORDER | SWP_SHOWWINDOW);

    nLeft = 0;
    nTop = cy - heightNotesTextBox;
    nWidth = cx;
    hdwp = DeferWindowPos(hdwp, hNotesTextBox, 0, nLeft, nTop, nWidth, heightNotesTextBox, SWP_NOZORDER | SWP_SHOWWINDOW);

    EndDeferWindowPos(hdwp);
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: TradeHistory
// ========================================================================================
BOOL TradeHistory_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
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
    CreateCustomVScrollBar(hwnd, IDC_HISTORY_CUSTOMVSCROLLBAR, hCtl);


    CustomLabel_SimpleLabel(hwnd, IDC_HISTORY_LBLNOTES, L"Notes", COLOR_WHITEDARK, COLOR_GRAYDARK,
        CustomLabelAlignment::MiddleLeft, 0, 0, 0, 0);
    hCtl = CreateCustomTextBox(hwnd, IDC_HISTORY_TXTNOTES, true, ES_LEFT,
        L"", 0, 0, 0, 0);
    CustomTextBox_SetMargins(hCtl, 3, 3);
    CustomTextBox_SetColors(hCtl, COLOR_WHITELIGHT, COLOR_GRAYMEDIUM);

    CustomLabel* pData = nullptr;
    hCtl = CreateCustomLabel(
        hwnd, IDC_HISTORY_SEPARATOR,
        CustomLabelType::LineHorizontal,
        0, 0, 0, 0);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->BackColor = COLOR_GRAYDARK;
        pData->LineColor = COLOR_SEPARATOR;
        pData->LineWidth = 2;
        pData->MarginLeft = 0;
        pData->MarginRight = 0;
        CustomLabel_SetOptions(hCtl, pData);
    }


    return TRUE;
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CTradeHistory::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(m_hwnd, WM_CREATE, TradeHistory_OnCreate);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, TradeHistory_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, TradeHistory_OnPaint);
        HANDLE_MSG(m_hwnd, WM_COMMAND, TradeHistory_OnCommand);
        HANDLE_MSG(m_hwnd, WM_SIZE, TradeHistory_OnSize);
        HANDLE_MSG(m_hwnd, WM_MEASUREITEM, TradeHistory_OnMeasureItem);
        HANDLE_MSG(m_hwnd, WM_DRAWITEM, ListBoxData_OnDrawItem);

    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}

