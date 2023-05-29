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
#include "..\CustomLabel\CustomLabel.h"
#include "..\CustomVScrollBar\CustomVScrollBar.h"
#include "..\MainWindow\MainWindow.h"
#include "..\HistoryPanel\HistoryPanel.h"
#include "..\Utilities\ListBoxData.h"

#include "TransPanel.h"


HWND HWND_TRANSPANEL = NULL;

extern CTransPanel TransPanel;

extern HWND HWND_MAINWINDOW;
extern HWND HWND_HISTORYPANEL;
extern void MainWindow_SetMiddlePanel(HWND hPanel);
extern void MainWindow_SetRightPanel(HWND hPanel);
extern void HistoryPanel_ShowTradesHistoryTable(const std::shared_ptr<Trade>& trade);

void TransPanel_OnSize(HWND hwnd, UINT state, int cx, int cy);


// ========================================================================================
// Display the selected Transaction history (legs) and ability to Edit/Delete it.
// ========================================================================================
void TransPanel_ShowTransactionHistory(const std::shared_ptr<Trade> trade, const std::shared_ptr<Transaction> trans)
{
    // Clear the current transaction history table
    HWND hListBox = GetDlgItem(HWND_HISTORYPANEL, IDC_HISTORY_LISTBOX);
    ListBoxData_DestroyItemData(hListBox);

    ListBoxData_HistoryHeader(hListBox, trade, trans);

    //CustomLabel_SetText(
    //    GetDlgItem(HWND_HISTORYPANEL, IDC_HISTORY_SYMBOL),
    //    ld->trade->tickerSymbol + L": " + ld->trade->tickerName);
    CustomLabel_SetText(
        GetDlgItem(HWND_HISTORYPANEL, IDC_HISTORY_SYMBOL),
        L"Transaction Details");

    // Show the detail leg information for this transaction.
    for (const auto& leg : trans->legs) {
        if (leg->underlying == L"OPTIONS") { ListBoxData_HistoryOptionsLeg(hListBox, trade, trans, leg); }
        else if (leg->underlying == L"SHARES") { ListBoxData_HistorySharesLeg(hListBox, trade, trans, leg); }
        else if (leg->underlying == L"FUTURES") {
            ListBoxData_HistorySharesLeg(hListBox, trade, trans, leg);
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

}


// ========================================================================================
// Central function that actually selects and displays the incoming ListBox index item.
// ========================================================================================
void TransPanel_ShowListBoxItem(int index)
{
    HWND hListBox = GetDlgItem(HWND_TRANSPANEL, IDC_TRANS_LISTBOX);
    HWND hCustomVScrollBar = GetDlgItem(HWND_TRANSPANEL, IDC_TRANS_CUSTOMVSCROLLBAR);

    ListBox_SetCurSel(hListBox, index);

    //  update the scrollbar position if necessary
    CustomVScrollBar_Recalculate(hCustomVScrollBar);

    // Get the current line to determine if a valid Trade pointer exists so that we
    // can show the trade history.
    if (index > -1) {
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, index);
        if (ld != nullptr)
            TransPanel_ShowTransactionHistory(ld->trade, ld->trans);
    }

    SetFocus(hListBox);
}


// ========================================================================================
// Populate the Transaction ListBox with the transactions per the user selected dates.
// ========================================================================================
void TransPanel_ShowTransactions()
{
    HWND hListBox = GetDlgItem(HWND_TRANSPANEL, IDC_TRANS_LISTBOX);
    HWND hCustomVScrollBar = GetDlgItem(HWND_TRANSPANEL, IDC_TRANS_CUSTOMVSCROLLBAR);


    // Prevent ListBox redrawing until all calculations are completed
    SendMessage(hListBox, WM_SETREDRAW, FALSE, 0);


    struct TransData {
        std::shared_ptr<Trade> trade;
        std::shared_ptr<Transaction> trans;
    };
    std::vector<TransData> tdata;
    tdata.reserve(2000);    // reserve space for 2000 transactions

    for (auto& trade : trades) {
        for (auto& trans : trade->transactions) {
            TransData td;
            td.trade = trade;
            td.trans = trans;
            tdata.push_back(td);
        }
    }


    // Sort the vector based on most recent date
    //std::sort(transactions.begin(), transactions.end(),
    //    [](const Transaction data1, const Transaction data2) {
    //        return (data1.transDate > data2.transDate) ? true : false;
    //    });


    // Clear the current table
    ListBoxData_DestroyItemData(hListBox);


    // Create the new Listbox data that will display for the Transactions
    for (const auto& td: tdata) {
        ListBoxData_OutputTransaction(hListBox, td.trade, td.trans);
    }


    // Calculate the actual column widths based on the size of the strings in
    // ListBoxData while respecting the minimum values as defined in nMinColWidth[].
    // This function is also called when receiving new price data from TWS because
    // that data may need the column width to be wider.
    ListBoxData_ResizeColumnWidths(hListBox, TableType::Transactions, -1);


    // Set the ListBox to the topline.
    ListBox_SetTopIndex(hListBox, 0);


    // Redraw the ListBox to ensure that any recalculated columns are 
    // displayed correctly. Re-enable redraw.
    SendMessage(hListBox, WM_SETREDRAW, TRUE, 0);
    AfxRedrawWindow(hListBox);


    // If transactions exist then select the first transaction so that its detail will show
    if (ListBox_GetCount(hListBox)) {
        TransPanel_ShowListBoxItem(0);
    }
    else {
        ListBoxData_AddBlankLine(hListBox);
    }

    // Ensure that the Transactions panel and History Panel are set
    MainWindow_SetMiddlePanel(HWND_TRANSPANEL);
    MainWindow_SetRightPanel(HWND_HISTORYPANEL);

    // Hide the Category control
    ShowWindow(GetDlgItem(HWND_MAINWINDOW, IDC_MAINWINDOW_CATEGORY), SW_HIDE);

    CustomVScrollBar_Recalculate(hCustomVScrollBar);

    ListBox_SetCurSel(hListBox, 0);
    SetFocus(hListBox);
}



// ========================================================================================
// Header control subclass Window procedure
// ========================================================================================
LRESULT CALLBACK TransPanel_Header_SubclassProc(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    switch (uMsg)
    {

    case WM_ERASEBKGND:
    {
        return TRUE;
    }


    case WM_PAINT:
    {
        Header_OnPaint(hWnd);
        return 0;
        break;
    }


    case WM_DESTROY:

        // REQUIRED: Remove control subclassing
        RemoveWindowSubclass(hWnd, TransPanel_Header_SubclassProc, uIdSubclass);
        break;


    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);

}


// ========================================================================================
// Listbox subclass Window procedure
// ========================================================================================
LRESULT CALLBACK TransPanel_ListBox_SubclassProc(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    // Create static accumulation variable to collect the data from
    // a series of middle mouse wheel scrolls.
    static int accumDelta = 0;

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
        HWND hCustomVScrollBar = GetDlgItem(HWND_TRANSPANEL, IDC_TRANS_CUSTOMVSCROLLBAR);
        CustomVScrollBar_Recalculate(hCustomVScrollBar);
        return 0;
        break;
    }


    case WM_RBUTTONDOWN:
    {
    }
    break;


    case WM_LBUTTONDOWN:
    {
        int idx = Listbox_ItemFromPoint(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        // The return value contains the index of the nearest item in the LOWORD. The HIWORD is zero 
        // if the specified point is in the client area of the list box, or one if it is outside the 
        // client area.
        if (HIWORD(idx) == -1) break;

        // Show the Transaction detail for the selected line.
        TransPanel_ShowListBoxItem(idx);
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
            SolidBrush backBrush(GetThemeColor(ThemeElement::GrayDark));
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
        RemoveWindowSubclass(hWnd, TransPanel_ListBox_SubclassProc, uIdSubclass);
        break;


    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);

}



// ========================================================================================
// Process WM_MEASUREITEM message for window/dialog: TransPanel
// ========================================================================================
void TransPanel_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT* lpMeasureItem)
{
    lpMeasureItem->itemHeight = AfxScaleY(TRANSACTIONS_LISTBOX_ROWHEIGHT);
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: TransPanel
// ========================================================================================
BOOL TransPanel_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    // Handle all of the painting in WM_PAINT
    return TRUE;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: TransPanel
// ========================================================================================
void TransPanel_OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    Graphics graphics(hdc);

    DWORD nBackColor = GetThemeColor(ThemeElement::GrayDark);

    // Create the background brush
    SolidBrush backBrush(nBackColor);

    // Paint the background using brush.
    int nWidth = (ps.rcPaint.right - ps.rcPaint.left);
    int nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&backBrush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Process WM_SIZE message for window/dialog: TransPanel
// ========================================================================================
void TransPanel_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    HWND hHeader = GetDlgItem(hwnd, IDC_TRANS_HEADER);
    HWND hListBox = GetDlgItem(hwnd, IDC_TRANS_LISTBOX);
    HWND hCustomVScrollBar = GetDlgItem(hwnd, IDC_TRANS_CUSTOMVSCROLLBAR);

    int margin = AfxScaleY(TRANSPANEL_MARGIN);

    HDWP hdwp = BeginDeferWindowPos(5);

    // Move and size the top label into place
    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_TRANS_LABEL), 0,
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


    int nLeft = 0;
    int nTop = margin;
    int nWidth = cx;
    int nHeight = AfxScaleY(TRANSACTIONS_LISTBOX_ROWHEIGHT);

    hdwp = DeferWindowPos(hdwp, hHeader, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);
    nTop = nTop + nHeight + AfxScaleY(1);

    nWidth = cx - CustomVScrollBarWidth;
    nHeight = cy - nTop;
    hdwp = DeferWindowPos(hdwp, hListBox, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nLeft = nLeft + nWidth;   // right edge of ListBox
    nWidth = CustomVScrollBarWidth;
    hdwp = DeferWindowPos(hdwp, hCustomVScrollBar, 0, nLeft, nTop, nWidth, nHeight,
        SWP_NOZORDER | (bShowScrollBar ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));

    EndDeferWindowPos(hdwp);
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: TransPanel
// ========================================================================================
BOOL TransPanel_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    HWND_TRANSPANEL = hwnd;

    HWND hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TRANS_LABEL, L"Transactions",
        ThemeElement::WhiteLight, ThemeElement::Black);

    hCtl = TransPanel.AddControl(Controls::Header, hwnd, IDC_TRANS_HEADER, L"",
        0, 0, 0, 0, -1, -1, NULL, (SUBCLASSPROC)TransPanel_Header_SubclassProc,
        IDC_TRANS_HEADER, NULL);
    int nWidth = AfxScaleX(50);
    Header_InsertNewItem(hCtl, 0, nWidth, L"", HDF_CENTER);
    Header_InsertNewItem(hCtl, 1, nWidth, L"Date", HDF_LEFT);
    Header_InsertNewItem(hCtl, 2, nWidth, L"Ticker", HDF_LEFT);
    Header_InsertNewItem(hCtl, 3, nWidth, L"Description", HDF_LEFT);
    Header_InsertNewItem(hCtl, 4, nWidth, L"Quantity", HDF_RIGHT);
    Header_InsertNewItem(hCtl, 5, nWidth, L"Price", HDF_RIGHT);
    Header_InsertNewItem(hCtl, 6, nWidth, L"Fees", HDF_RIGHT);
    Header_InsertNewItem(hCtl, 7, nWidth, L"Total", HDF_RIGHT);
    // Must turn off Window Theming for the control in order to correctly apply colors
    SetWindowTheme(hCtl, L"", L"");


    // Create an Ownerdraw listbox that we will use to custom paint our transactions.
    hCtl =
        TransPanel.AddControl(Controls::ListBox, hwnd, IDC_TRANS_LISTBOX, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWFIXED | LBS_NOTIFY,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)TransPanel_ListBox_SubclassProc,
            IDC_TRANS_LISTBOX, NULL);
    ListBox_AddString(hCtl, NULL);


    // Create our custom vertical scrollbar and attach the ListBox to it.
    CreateCustomVScrollBar(hwnd, IDC_TRANS_CUSTOMVSCROLLBAR, hCtl);

    return TRUE;
}


// ========================================================================================
// Process WM_COMMAND message for window/dialog: TransPanel
// ========================================================================================
void TransPanel_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (codeNotify)
    {

    case (LBN_SELCHANGE):
        int nCurSel = ListBox_GetCurSel(hwndCtl);
        if (nCurSel == -1) break;
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hwndCtl, nCurSel);
        if (ld != nullptr) {
            // Show the transaction detail for the selected transaction
            TransPanel_ShowListBoxItem(nCurSel);
        }
        break;

    }
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CTransPanel::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(m_hwnd, WM_CREATE, TransPanel_OnCreate);
        HANDLE_MSG(m_hwnd, WM_COMMAND, TransPanel_OnCommand);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, TransPanel_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, TransPanel_OnPaint);
        HANDLE_MSG(m_hwnd, WM_SIZE, TransPanel_OnSize);
        HANDLE_MSG(m_hwnd, WM_MEASUREITEM, TransPanel_OnMeasureItem);
        HANDLE_MSG(m_hwnd, WM_DRAWITEM, ListBoxData_OnDrawItem);

    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}

