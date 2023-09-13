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
#include "Utilities/ListBoxData.h"
#include "SideMenu/SideMenu.h"
#include "MainWindow/MainWindow.h"
#include "Category/Category.h"
#include "CustomVScrollBar/CustomVScrollBar.h"
#include "TradeHistory/TradeHistory.h"
#include "ActiveTrades/ActiveTrades.h"
#include "YearEndDialog/YearEndDialog.h"
#include "Database/trade.h"
#include "ClosedTrades.h"


HWND HWND_CLOSEDTRADES = NULL;

CClosedTrades ClosedTrades;



// ========================================================================================
// Central function that actually selects and displays the incoming ListBox index item.
// ========================================================================================
void ClosedTrades_ShowListBoxItem(int index)
{
    HWND hListBox = GetDlgItem(HWND_CLOSEDTRADES, IDC_CLOSED_LISTBOX);
    HWND hCustomVScrollBar = GetDlgItem(HWND_CLOSEDTRADES, IDC_CLOSED_CUSTOMVSCROLLBAR);

    ListBox_SetCurSel(hListBox, index);

    // Ensure that the ActiveTrades menu item is selected
    SideMenu_SelectMenuItem(HWND_SIDEMENU, IDC_SIDEMENU_CLOSEDTRADES);

    //  update the scrollbar position if necessary
    CustomVScrollBar_Recalculate(hCustomVScrollBar);

    // Get the current line to determine if a valid Trade pointer exists so that we
    // can show the trade history.
    if (index > -1) {
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, index);
        if (ld != nullptr)
            TradeHistory_ShowTradesHistoryTable(ld->trade);
    }

    SetFocus(hListBox);
}


// ========================================================================================
// Populate the ListBox with the closed trades
// ========================================================================================
void ClosedTrades_ShowClosedTrades()
{
    HWND hListBox = GetDlgItem(HWND_CLOSEDTRADES, IDC_CLOSED_LISTBOX);
    HWND hCustomVScrollBar = GetDlgItem(HWND_CLOSEDTRADES, IDC_CLOSED_CUSTOMVSCROLLBAR);
    HWND hLabel = GetDlgItem(HWND_CLOSEDTRADES, IDC_CLOSED_LABEL);
    HWND hCategory = GetDlgItem(HWND_CLOSEDTRADES, IDC_CLOSED_CATEGORY);


    // Prevent ListBox redrawing until all calculations are completed.
    SendMessage(hListBox, WM_SETREDRAW, FALSE, 0);


    struct ClosedData {
        std::wstring closed_date;
        std::shared_ptr<Trade> trade;
    };

    std::vector<ClosedData> vectorClosed;
    vectorClosed.reserve(1000);         // reserve space for 1000 closed trades

    
    int selected_category = CategoryControl_GetSelectedIndex(hCategory);

    for (auto& trade : trades) {
        if (!trade->is_open) {

            if (selected_category != CATEGORY_ALL) {
                if (trade->category != selected_category) continue;
            }

            ClosedData data;

            // Iterate to find the latest closed date
            for (auto& trans : trade->transactions) {
                if (trans->trans_date > data.closed_date) {
                    data.closed_date = trans->trans_date;
                }
            }
            data.trade = trade;
            vectorClosed.push_back(data);
        }
    }


    // Destroy any existing ListBox line data
    ListBoxData_DestroyItemData(hListBox);


    // Sort the closed vector based on trade closed date
    std::sort(vectorClosed.begin(), vectorClosed.end(),
        [](const ClosedData data1, const ClosedData data2) {
            return (data1.closed_date > data2.closed_date) ? true : false;
        });


    // Output the closed trades and subtotals based on month when the month changes.
    double subtotal_amount = 0;
    double YTD = 0;
    double monthly_amount = 0;
    double weekly_amount = 0;
    double daily_amount = 0;
    
    int subtotal_month = 0;
    
    int current_month = 0;
    int current_year = 0;
    int current_month_win = 0;
    int current_month_loss = 0;

    int month_win = 0;
    int month_loss = 0;

    int week_win = 0;
    int week_loss = 0;

    int day_win = 0;
    int day_loss = 0;

    int year_win = 0;
    int year_loss = 0;

    std::wstring today_date = AfxCurrentDate();
    std::wstring week_start_date = AfxDateAddDays(today_date, -AfxLocalDayOfWeek());
    std::wstring week_end_date = AfxDateAddDays(week_start_date, 6);
    std::wstring curDate = L"";

    for (const auto& ClosedData : vectorClosed) {
        current_month = AfxGetMonth(ClosedData.closed_date);
        if (current_year == 0) current_year = AfxGetYear(ClosedData.closed_date);
        if (subtotal_month != current_month && subtotal_month != 0) {
            ListBoxData_OutputClosedMonthSubtotal(hListBox, curDate, subtotal_amount, current_month_win, current_month_loss);
            curDate = ClosedData.closed_date;
            subtotal_amount = 0;
            current_month_win = 0;
            current_month_loss = 0;
        }
        curDate = ClosedData.closed_date;
        subtotal_month = current_month;
        subtotal_amount += ClosedData.trade->acb;
        if (ClosedData.trade->acb >= 0) ++current_month_win;
        if (ClosedData.trade->acb < 0) ++current_month_loss;

        if (current_month == AfxGetMonth(today_date)) {
            monthly_amount += ClosedData.trade->acb;
            if (ClosedData.trade->acb >= 0) ++month_win;
            if (ClosedData.trade->acb < 0) ++month_loss;
        }

        if (curDate >= week_start_date && curDate <= week_end_date) {
            weekly_amount += ClosedData.trade->acb;
            if (ClosedData.trade->acb >= 0) ++week_win;
            if (ClosedData.trade->acb < 0) ++week_loss;
        }

        if (curDate == today_date) {
            daily_amount += ClosedData.trade->acb;
            if (ClosedData.trade->acb >= 0) ++day_win;
            if (ClosedData.trade->acb < 0) ++day_loss;
        }

        if (current_year == AfxGetYear(curDate)) {
            if (ClosedData.trade->acb >= 0) ++year_win;
            if (ClosedData.trade->acb < 0) ++year_loss;
            YTD += ClosedData.trade->acb;
        }
        ListBoxData_OutputClosedPosition(hListBox, ClosedData.trade, ClosedData.closed_date);
    }
    if (subtotal_amount != 0) {
        ListBoxData_OutputClosedMonthSubtotal(hListBox, curDate, subtotal_amount, current_month_win, current_month_loss);
    }

    ListBoxData_OutputClosedYearTotal(hListBox, current_year, YTD, year_win, year_loss);
    ListBoxData_OutputClosedMonthTotal(hListBox, monthly_amount, month_win, month_loss);
    ListBoxData_OutputClosedWeekTotal(hListBox, weekly_amount, week_win, week_loss);
    ListBoxData_OutputClosedDayTotal(hListBox, daily_amount, day_win, day_loss);


    // Calculate the actual column widths based on the size of the strings in
    // ListBoxData while respecting the minimum values as defined in nMinColWidth[].
    // This function is also called when receiving new price data from TWS because
    // that data may need the column width to be wider.
    ListBoxData_ResizeColumnWidths(hListBox, TableType::closed_trades, -1);


    // Set the label text indicated the type of trades being listed
    CustomLabel_SetText(hLabel, L"Closed Trades");


    // If no closed trades exist then add at least one line
    if (ListBox_GetCount(hListBox) == 0) {
        ListBoxData_AddBlankLine(hListBox);
    }


    // Ensure that the Closed panel is set
    MainWindow_SetMiddlePanel(HWND_CLOSEDTRADES);


    CustomVScrollBar_Recalculate(hCustomVScrollBar);

    // Select row past the DAY total line if possible
    int curSel = min(ListBox_GetCount(hListBox)-1, 5);
    ClosedTrades_ShowListBoxItem(curSel);  

    // Redraw the ListBox to ensure that any recalculated columns are 
    // displayed correctly. Re-enable redraw.
    SendMessage(hListBox, WM_SETREDRAW, TRUE, 0);
    AfxRedrawWindow(hListBox);
}


// ========================================================================================
// Header control subclass Window procedure
// ========================================================================================
LRESULT CALLBACK ClosedTrades_Header_SubclassProc(
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
        RemoveWindowSubclass(hWnd, ClosedTrades_Header_SubclassProc, uIdSubclass);
        break;


    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);

}


// ========================================================================================
// Listbox subclass Window procedure
// ========================================================================================
LRESULT CALLBACK ClosedTrades_ListBox_SubclassProc(
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
        int top_index = SendMessage(hWnd, LB_GETTOPINDEX, 0, 0);
        accumDelta += zDelta;
        if (accumDelta >= 120) {     // scroll up 3 lines
            top_index -= 3;
            top_index = max(0, top_index);
            SendMessage(hWnd, LB_SETTOPINDEX, top_index, 0);
            accumDelta = 0;
        }
        else {
            if (accumDelta <= -120) {     // scroll down 3 lines
                top_index += +3;
                SendMessage(hWnd, LB_SETTOPINDEX, top_index, 0);
                accumDelta = 0;
            }
        }
        HWND hCustomVScrollBar = GetDlgItem(HWND_CLOSEDTRADES, IDC_CLOSED_CUSTOMVSCROLLBAR);
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

        ClosedTrades_ShowListBoxItem(idx);
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
        int item_height = (rcItem.bottom - rcItem.top);
        int items_count = ListBox_GetCount(hWnd);
        int top_index = SendMessage(hWnd, LB_GETTOPINDEX, 0, 0);
        int visible_rows = 0;
        int items_per_page = 0;
        int bottom_index = 0;
        int nWidth = (rc.right - rc.left);
        int nHeight = (rc.bottom - rc.top);

        if (items_count > 0) {
            items_per_page = (nHeight) / item_height;
            bottom_index = (top_index + items_per_page);
            if (bottom_index >= items_count)
                bottom_index = items_count - 1;
            visible_rows = (bottom_index - top_index) + 1;
            rc.top = visible_rows * item_height;
        }

        if (rc.top < rc.bottom) {
            nHeight = (rc.bottom - rc.top);
            HDC hDC = (HDC)wParam;
            Graphics graphics(hDC);
            SolidBrush back_brush(COLOR_GRAYDARK);
            graphics.FillRectangle(&back_brush, rc.left, rc.top, nWidth, nHeight);
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
        RemoveWindowSubclass(hWnd, ClosedTrades_ListBox_SubclassProc, uIdSubclass);
        break;


    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);

}



// ========================================================================================
// Process WM_MEASUREITEM message for window/dialog: ClosedTrades
// ========================================================================================
void ClosedTrades_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT* lpMeasureItem)
{
    lpMeasureItem->itemHeight = AfxScaleY(CLOSED_TRADES_LISTBOX_ROWHEIGHT);
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: ClosedTrades
// ========================================================================================
BOOL ClosedTrades_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    // Handle all of the painting in WM_PAINT
    return TRUE;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: ClosedTrades
// ========================================================================================
void ClosedTrades_OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    Graphics graphics(hdc);

    DWORD back_color = COLOR_BLACK;

    // Create the background brush
    SolidBrush back_brush(back_color);

    // Paint the background using brush.
    int nWidth = (ps.rcPaint.right - ps.rcPaint.left);
    int nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Process WM_SIZE message for window/dialog: ClosedTrades
// ========================================================================================
void ClosedTrades_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    HWND hHeader = GetDlgItem(hwnd, IDC_CLOSED_HEADER);
    HWND hListBox = GetDlgItem(hwnd, IDC_CLOSED_LISTBOX);
    HWND hCategory = GetDlgItem(hwnd, IDC_CLOSED_CATEGORY);
    HWND hYearEnd = GetDlgItem(hwnd, IDC_CLOSED_CMDYEAREND);
    HWND hCustomVScrollBar = GetDlgItem(hwnd, IDC_CLOSED_CUSTOMVSCROLLBAR);

    int margin = AfxScaleY(CLOSEDTRADES_MARGIN);

    HDWP hdwp = BeginDeferWindowPos(5);


    // Do not call the calcVThumbRect() function during a scrollbar move. This WM_SIZE
    // gets triggered when the ListBox WM_DRAWITEM fires. If we do another calcVThumbRect()
    // calcualtion then the scrollbar will appear "jumpy" under the user's mouse cursor.
    bool bshow_scrollbar = false;
    CustomVScrollBar* pData = CustomVScrollBar_GetPointer(hCustomVScrollBar);
    if (pData != nullptr) {
        if (pData->drag_active) {
            bshow_scrollbar = true;
        }
        else {
            bshow_scrollbar = pData->calcVThumbRect();
        }
    }
    int custom_scrollbar_width = bshow_scrollbar ? AfxScaleX(CUSTOMVSCROLLBAR_WIDTH) : 0;


    int nLeft = 0;
    int nTop = 0;
    int nWidth = 0;
    int nHeight = AfxScaleY(23);

    // Move and size the top label into place
    nWidth = cx;
    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_CLOSED_LABEL), 0,
        0, 0, cx, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);
    nTop = nTop + nHeight;


    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_CLOSED_LBLCATEGORYFILTER), 0,
        nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);


    nTop = nTop + nHeight;
    hdwp = DeferWindowPos(hdwp, hCategory, 0, nLeft, nTop, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_SHOWWINDOW);


    nWidth = AfxScaleX(100);
    nLeft = cx - nWidth - custom_scrollbar_width;
    nHeight = AfxScaleY(CATEGORYCONTROL_HEIGHT);
    hdwp = DeferWindowPos(hdwp, hYearEnd, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    
    nLeft = 0;
    nTop = margin;
    nWidth = cx;
    nHeight = AfxScaleY(CLOSED_TRADES_LISTBOX_ROWHEIGHT);
    hdwp = DeferWindowPos(hdwp, hHeader, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);
    nTop = nTop + nHeight + AfxScaleY(1);


    nWidth = cx - custom_scrollbar_width;
    nHeight = cy - nTop;
    hdwp = DeferWindowPos(hdwp, hListBox, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nLeft = nLeft + nWidth;   // right edge of ListBox
    nWidth = custom_scrollbar_width;
    hdwp = DeferWindowPos(hdwp, hCustomVScrollBar, 0, nLeft, nTop, nWidth, nHeight,
        SWP_NOZORDER | (bshow_scrollbar ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));

    EndDeferWindowPos(hdwp);
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: ClosedTrades
// ========================================================================================
BOOL ClosedTrades_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    HWND_CLOSEDTRADES = hwnd;

    HWND hCtl = CustomLabel_SimpleLabel(hwnd, IDC_CLOSED_LABEL, L"Closed Trades",
        COLOR_WHITELIGHT, COLOR_BLACK);

    hCtl = ClosedTrades.AddControl(Controls::Header, hwnd, IDC_CLOSED_HEADER, L"",
        0, 0, 0, 0, -1, -1, NULL, (SUBCLASSPROC)ClosedTrades_Header_SubclassProc,
        IDC_CLOSED_HEADER, NULL);
    int nWidth = AfxScaleX(50);
    Header_InsertNewItem(hCtl, 0, nWidth, L"", HDF_CENTER);
    Header_InsertNewItem(hCtl, 1, nWidth, L"Date", HDF_LEFT);
    Header_InsertNewItem(hCtl, 2, nWidth, L"Ticker", HDF_LEFT);
    Header_InsertNewItem(hCtl, 3, nWidth, L"Company Name", HDF_LEFT);
    Header_InsertNewItem(hCtl, 4, nWidth, L"Amount", HDF_RIGHT);
    // Must turn off Window Theming for the control in order to correctly apply colors
    SetWindowTheme(hCtl, L"", L"");


    CustomLabel_SimpleLabel(hwnd, IDC_CLOSED_LBLCATEGORYFILTER, L"Category Filter",
        COLOR_WHITEDARK, COLOR_BLACK);


    // CATEGORY SELECTOR
    hCtl = CreateCategoryControl(hwnd, IDC_CLOSED_CATEGORY, 0, 0, CATEGORY_ALL, true);

    
    // YEAR END CLOSE
    std::wstring font_name = L"Segoe UI";
    int font_size = 9;
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_CLOSED_CMDYEAREND, L"Year End",
        COLOR_BLACK, COLOR_RED, COLOR_RED, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_center, 0, 0, 0, 0);
    CustomLabel_SetFont(hCtl, font_name, font_size, true);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);


    // Create an Ownerdraw fixed row sized listbox that we will use to custom
    // paint our various closed trades.
    hCtl =
        ClosedTrades.AddControl(Controls::ListBox, hwnd, IDC_CLOSED_LISTBOX, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWFIXED | LBS_NOTIFY,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)ClosedTrades_ListBox_SubclassProc,
            IDC_CLOSED_LISTBOX, NULL);
    ListBox_AddString(hCtl, NULL);


    // Create our custom vertical scrollbar and attach the ListBox to it.
    CreateCustomVScrollBar(hwnd, IDC_CLOSED_CUSTOMVSCROLLBAR, hCtl);

    return TRUE;
}


// ========================================================================================
// Process WM_COMMAND message for window/dialog: ClosedTrades
// ========================================================================================
void ClosedTrades_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (codeNotify)
    {

    case (LBN_SELCHANGE):
        int selected = ListBox_GetCurSel(hwndCtl);
        if (selected == -1) break;
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hwndCtl, selected);
        if (ld != nullptr) {
            // Show the trade history for the selected trade
            ClosedTrades_ShowListBoxItem(selected);
        }
        break;

    }
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CClosedTrades::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(m_hwnd, WM_CREATE, ClosedTrades_OnCreate);
        HANDLE_MSG(m_hwnd, WM_COMMAND, ClosedTrades_OnCommand);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, ClosedTrades_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, ClosedTrades_OnPaint);
        HANDLE_MSG(m_hwnd, WM_SIZE, ClosedTrades_OnSize);
        HANDLE_MSG(m_hwnd, WM_MEASUREITEM, ClosedTrades_OnMeasureItem);
        HANDLE_MSG(m_hwnd, WM_DRAWITEM, ListBoxData_OnDrawItem);


    case MSG_CATEGORY_CATEGORYCHANGED:
    {
        // Categories have change so update the Closed Trades list.
        ClosedTrades_ShowClosedTrades();

        // Also need to update Active Trades list because Category headers have changed.
        AfxRedrawWindow(GetDlgItem(HWND_ACTIVETRADES, IDC_TRADES_LISTBOX));
    }
    return 0;


    case MSG_CUSTOMLABEL_CLICK:
    {
        HWND hCtl = (HWND)lParam;
        int CtrlId = (int)wParam;

        if (hCtl == NULL) return 0;

        if (CtrlId == IDC_CLOSED_CMDYEAREND) {
            YearEndDialog_Show();
        }
        return 0;
    }


    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}

