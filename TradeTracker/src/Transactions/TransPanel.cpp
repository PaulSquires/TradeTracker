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
#include "MainWindow/MainWindow.h"
#include "Category/Category.h"
#include "TabPanel/TabPanel.h"
#include "Utilities/ListBoxData.h"

#include "FilterPanel/FilterPanel.h"

#include "TransDetail.h"
#include "TransPanel.h"


CTransPanel TransPanel;


// ========================================================================================
// Get the HWND's of the the controls on the form.
// ========================================================================================
inline HWND CTransPanel::TradesListBox() {
    return GetDlgItem(hWindow, IDC_TRANS_LISTBOX);
}
inline HWND CTransPanel::VScrollBar() {
    return GetDlgItem(hWindow, IDC_TRANS_CUSTOMVSCROLLBAR);
}
inline HWND CTransPanel::TradesHeader() {
    return GetDlgItem(hWindow, IDC_TRANS_HEADER);
}



// ========================================================================================
// Central function that actually selects and displays the incoming ListBox index item.
// ========================================================================================
void CTransPanel::ShowListBoxItem(int index) {
    ListBox_SetCurSel(TradesListBox(), index);

    // Ensure that the Transactions menu item is selected
    TabPanel_SelectPanelItem(HWND_TABPANEL, IDC_TABPANEL_TRANSACTIONS);

    //  update the scrollbar position if necessary
    CustomVScrollBar_Recalculate(VScrollBar());

    // Get the current line to determine if a valid Trade pointer exists so that we
    // can show the transaction detail.
    if (index > -1) {
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(TradesListBox(), index);
        if (ld)
            TransDetail.ShowTransDetail(ld->trade, ld->trans);
    }

    SetFocus(TradesListBox());
}


// ========================================================================================
// Populate the Transaction ListBox with the Transactions per the user selected dates.
// ========================================================================================
void CTransPanel::ShowTransactions() {

    // Select the correct menu panel item
    TabPanel_SelectPanelItem(HWND_TABPANEL, IDC_TABPANEL_TRANSACTIONS);

    // Ensure that the TransDetail panel and Detail Panel are set
    MainWindow.SetLeftPanel(hWindow);
    MainWindow.SetRightPanel(TransDetail.hWindow);

    std::wstring start_date = FilterPanel.filter_start_date;
    std::wstring end_date = FilterPanel.filter_end_date;
    std::wstring ticker = FilterPanel.ticker_symbol;
    int selected_category = FilterPanel.selected_category;


    // Prevent ListBox redrawing until all calculations are completed
    SendMessage(TradesListBox(), WM_SETREDRAW, false, 0);

    struct TransData {
        std::shared_ptr<Trade> trade;
        std::shared_ptr<Transaction> trans;
    };
    std::vector<TransData> tdata;
    tdata.reserve(2000);    // reserve space for 2000 Transactions

    for (auto& trade : trades) {
        for (auto& trans : trade->transactions) {
            if (ticker.length() > 0) {
                if (ticker != trade->ticker_symbol) continue;
            }

            if (trans->trans_date < start_date || trans->trans_date > end_date) continue;

            TransData td;
            td.trade = trade;
            td.trans = trans;
            tdata.push_back(td);
        }
    }

    // Sort the vector based on most recent date then by ticker
    std::sort(tdata.begin(), tdata.end(),
        [](const TransData data1, const TransData data2) {
            {
                if (data1.trans->trans_date > data2.trans->trans_date) return true;
                if (data2.trans->trans_date > data1.trans->trans_date) return false;

                // a=b for primary condition, go to secondary
                if (data1.trade->ticker_symbol < data2.trade->ticker_symbol) return true;
                if (data2.trade->ticker_symbol < data1.trade->ticker_symbol) return false;

                return false;
            }
        });


    // Clear the current table
    ListBoxData_DestroyItemData(TradesListBox());

    // Create the new Listbox data that will display for the Transactions
    double running_gross_total = 0;
    double running_fees_total = 0;
    double running_net_total = 0;

    for (const auto& td : tdata) {
        ListBoxData_OutputTransaction(TradesListBox(), td.trade, td.trans);
        running_net_total += td.trans->total;
        running_fees_total += td.trans->fees;
        running_gross_total += (td.trans->total + td.trans->fees);
    }
    ListBoxData_OutputTransactionRunningTotal(TradesListBox(), running_gross_total, running_fees_total, running_net_total);

    // Calculate the actual column widths based on the size of the strings in
    // ListBoxData while respecting the minimum values as defined in nMinColWidth[].
    ListBoxData_ResizeColumnWidths(TradesListBox(), TableType::trans_panel);

    // Set the ListBox to the topline.
    ListBox_SetTopIndex(TradesListBox(), 0);

    // If no transactions then add at least one line
    if (ListBox_GetCount(TradesListBox())) {
        ListBoxData_AddBlankLine(TradesListBox());
    }

    CustomVScrollBar_Recalculate(VScrollBar());

    // Select row past the YTD total line if possible
    int curSel = min(ListBox_GetCount(TradesListBox()) - 1, 2);
    ShowListBoxItem(curSel);

    // Redraw the ListBox to ensure that any recalculated columns are 
    // displayed correctly. Re-enable redraw.
    SendMessage(TradesListBox(), WM_SETREDRAW, true, 0);
    AfxRedrawWindow(TradesListBox());
}


// ========================================================================================
// Header control subclass Window procedure
// ========================================================================================
LRESULT CALLBACK CTransPanel::Header_SubclassProc(
    HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    switch (uMsg) {

    case WM_ERASEBKGND: {
        return true;
    }

    case WM_PAINT: {
        Header_OnPaint(hwnd);
        return 0;
    }

    case WM_DESTROY: {
        // REQUIRED: Remove control subclassing
        RemoveWindowSubclass(hwnd, Header_SubclassProc, uIdSubclass);
        break;
    }

    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}


// ========================================================================================
// Listbox subclass Window procedure
// ========================================================================================
LRESULT CALLBACK CTransPanel::ListBox_SubclassProc(
    HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    // Create static accumulation variable to collect the data from
    // a series of middle mouse wheel scrolls.
    static int accum_delta = 0;

    switch (uMsg) {

    case WM_KEYDOWN: {
        // Parent to handle the TAB navigation key to move amongst constrols.
        if (wParam == VK_TAB) {
            if (SendMessage(GetParent(hwnd), uMsg, wParam, lParam))
                return 0;
        }
        break;
    }

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
        HWND hCustomVScrollBar = GetDlgItem(GetParent(hwnd), IDC_TRANS_CUSTOMVSCROLLBAR);
        CustomVScrollBar_Recalculate(hCustomVScrollBar);
        return 0;
    }

    case WM_LBUTTONDOWN: {
        int idx = Listbox_ItemFromPoint(hwnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        // The return value contains the index of the nearest item in the LOWORD. The HIWORD is zero 
        // if the specified point is in the client area of the list box, or one if it is outside the 
        // client area.
        if (HIWORD(idx) == 1) break;

        // Show the Transaction detail for the selected line.
        SendMessage(GetParent(hwnd), MSG_TRANSPANEL_SHOWLISTBOXITEM, idx, 0);
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

        if (items_count > 0) {
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
        RemoveWindowSubclass(hwnd, ListBox_SubclassProc, uIdSubclass);
        break;
    }

    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}


// ========================================================================================
// Process WM_MEASUREITEM message for window/dialog: TransPanel
// ========================================================================================
void CTransPanel::OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT* lpMeasureItem) {
    lpMeasureItem->itemHeight = AfxScaleY(TRANSPANEL_LISTBOX_ROWHEIGHT);
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: TransPanel
// ========================================================================================
bool CTransPanel::OnEraseBkgnd(HWND hwnd, HDC hdc) {
    // Handle all of the painting in WM_PAINT
    return true;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: TransPanel
// ========================================================================================
void CTransPanel::OnPaint(HWND hwnd) {
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    Graphics graphics(hdc);

    // Create the background brush
    Color back_color(COLOR_BLACK);
    SolidBrush back_brush(back_color);

    // Paint the background using brush.
    int width = (ps.rcPaint.right - ps.rcPaint.left);
    int height = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, width, height);

    // Paint the area to the left of the ListBox in order to give the illusion
    // of a margin before the ListBox data is displyed.
    ps.rcPaint.top += AfxScaleY(TRANSPANEL_MARGIN);

    // Set the background brush
    back_color.SetValue(COLOR_GRAYDARK);
    back_brush.SetColor(back_color);
    width = (ps.rcPaint.right - ps.rcPaint.left);
    height = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, width, height);

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Process WM_SIZE message for window/dialog: TransPanel
// ========================================================================================
void CTransPanel::OnSize(HWND hwnd, UINT state, int cx, int cy) {

    // Do not call the calcVThumbRect() function during a scrollbar move. This WM_SIZE
    // gets triggered when the ListBox WM_DRAWITEM fires. If we do another calcVThumbRect()
    // calcualtion then the scrollbar will appear "jumpy" under the user's mouse cursor.
    bool bshow_scrollbar = false;
    CustomVScrollBar* pData = CustomVScrollBar_GetPointer(VScrollBar());
    if (pData) {
        if (pData->drag_active) {
            bshow_scrollbar = true;
        }
        else {
            bshow_scrollbar = pData->calcVThumbRect();
        }
    }
    int custom_scrollbar_width = bshow_scrollbar ? AfxScaleX(CUSTOMVSCROLLBAR_WIDTH) : 0;

    int margin = AfxScaleY(TRANSPANEL_MARGIN);
    int left = AfxScaleX(APP_LEFTMARGIN_WIDTH);
    int top = 0;
    int width = cx;
    int height = 0;

    HDWP hdwp = BeginDeferWindowPos(5);

    hdwp = DeferWindowPos(hdwp, FilterPanel.hWindow, 0, left, top, width, FilterPanel.fixed_height, SWP_NOZORDER | SWP_SHOWWINDOW);

    top = margin;
    height = AfxScaleY(TRANSPANEL_LISTBOX_ROWHEIGHT);
    hdwp = DeferWindowPos(hdwp, TradesHeader(), 0, left, top, width, height, SWP_NOZORDER | SWP_SHOWWINDOW);
    top += (height + AfxScaleY(1));

    width = cx - left - custom_scrollbar_width;
    height = cy - top;
    hdwp = DeferWindowPos(hdwp, TradesListBox(), 0, left, top, width, height, SWP_NOZORDER | SWP_SHOWWINDOW);

    left += width;   // right edge of ListBox
    width = custom_scrollbar_width;
    hdwp = DeferWindowPos(hdwp, VScrollBar(), 0, left, top, width, height,
        SWP_NOZORDER | (bshow_scrollbar ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));

    EndDeferWindowPos(hdwp);
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: TransPanel
// ========================================================================================
bool CTransPanel::OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct) {
    hWindow = hwnd;

    // Add the top Filter Panel
    HWND hCtl = FilterPanel.CreateFilterPanel(hwnd);
       
    hCtl = TransPanel.AddControl(Controls::Header, hwnd, IDC_TRANS_HEADER, L"",
        0, 0, 0, 0, -1, -1, NULL, (SUBCLASSPROC)Header_SubclassProc,
        IDC_TRANS_HEADER, NULL);
    Header_InsertNewItem(hCtl, 0, AfxScaleX(nTransMinColWidth[0]), L"", HDF_CENTER);
    Header_InsertNewItem(hCtl, 1, AfxScaleX(nTransMinColWidth[1]), L"Date", HDF_LEFT);
    Header_InsertNewItem(hCtl, 2, AfxScaleX(nTransMinColWidth[2]), L"Ticker", HDF_LEFT);
    Header_InsertNewItem(hCtl, 3, AfxScaleX(nTransMinColWidth[3]), L"Description", HDF_LEFT);
    Header_InsertNewItem(hCtl, 4, AfxScaleX(nTransMinColWidth[4]), L"Quantity", HDF_RIGHT);
    Header_InsertNewItem(hCtl, 5, AfxScaleX(nTransMinColWidth[5]), L"Price", HDF_RIGHT);
    Header_InsertNewItem(hCtl, 6, AfxScaleX(nTransMinColWidth[6]), L"Fees", HDF_RIGHT);
    Header_InsertNewItem(hCtl, 7, AfxScaleX(nTransMinColWidth[7]), L"Total", HDF_RIGHT);
    // Must turn off Window Theming for the control in order to correctly apply colors
    SetWindowTheme(hCtl, L"", L"");

    // Create an Ownerdraw listbox that we will use to custom paint our Transactions.
    hCtl =
        TransPanel.AddControl(Controls::ListBox, hwnd, IDC_TRANS_LISTBOX, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWFIXED | LBS_NOTIFY,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)ListBox_SubclassProc,
            IDC_TRANS_LISTBOX, NULL);
    ListBox_AddString(hCtl, NULL);

    // Create our custom vertical scrollbar and attach the ListBox to it.
    CreateCustomVScrollBar(hwnd, IDC_TRANS_CUSTOMVSCROLLBAR, hCtl, Controls::ListBox);

    return true;
}


// ========================================================================================
// Process WM_COMMAND message for window/dialog: TransPanel
// ========================================================================================
void CTransPanel::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {
    switch (codeNotify) {

    case LBN_SELCHANGE: {
        int current_sel = ListBox_GetCurSel(hwndCtl);
        if (current_sel == -1) break;
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hwndCtl, current_sel);
        if (ld) {
            // Show the transaction detail for the selected transaction
            ShowListBoxItem(current_sel);
        }
        break;
    }

    }
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CTransPanel::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        HANDLE_MSG(m_hwnd, WM_CREATE, OnCreate);
        HANDLE_MSG(m_hwnd, WM_COMMAND, OnCommand);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, OnPaint);
        HANDLE_MSG(m_hwnd, WM_SIZE, OnSize);
        HANDLE_MSG(m_hwnd, WM_MEASUREITEM, OnMeasureItem);
        HANDLE_MSG(m_hwnd, WM_DRAWITEM, ListBoxData_OnDrawItem);
    
    case MSG_DATEPICKER_DATECHANGED: {
        // Received from FilterPanel to indicate that we need to refresh our Transaction grid.
        ShowTransactions();
        return 0;
    }

    }

    return DefWindowProc(m_hwnd, msg, wParam, lParam);
}

