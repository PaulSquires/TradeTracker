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
#include "MainWindow/MainWindow.h"
#include "Category/Category.h"
#include "Transactions/TransDateFilter.h"
#include "Transactions/TransDetail.h"
#include "TabPanel/TabPanel.h"
#include "DatePicker/Calendar.h"
#include "Utilities/ListBoxData.h"
#include "TransPanel.h"


HWND HWND_TRANSPANEL = NULL;

CTransPanel TransPanel;


// ========================================================================================
// Set the StartDate and EndDate based on the current value of the Date Filter.
// ========================================================================================
void TransPanel_SetStartEndDates(HWND hwnd)
{
    int idx = CustomLabel_GetUserDataInt(GetDlgItem(hwnd, IDC_TRANS_TRANSDATE));

    // Do not modify dates if Custom has been set.
    if ((TransDateFilterType)idx == TransDateFilterType::Custom) return;

    std::wstring wszEndDate = AfxCurrentDate();   // ISO format
    std::wstring wszStartDate = wszEndDate;       // ISO format
    int AdjustDays = 0;

    if ((TransDateFilterType)idx == TransDateFilterType::YearToDate) {
        wszStartDate = wszEndDate.substr(0, 4)+ L"-01-01";
    }
    else if ((TransDateFilterType)idx == TransDateFilterType::Yesterday) {
        wszStartDate = AfxDateAddDays(wszEndDate, -1);
        wszEndDate = wszStartDate;
    }
    else if ((TransDateFilterType)idx == TransDateFilterType::Today) {
        // Dates are already set to current date
    }
    else {
        switch ((TransDateFilterType)idx)
        {
        case TransDateFilterType::Days7: AdjustDays = 7; break;
        case TransDateFilterType::Days14: AdjustDays = 14; break;
        case TransDateFilterType::Days30: AdjustDays = 30; break;
        case TransDateFilterType::Days60: AdjustDays = 60; break;
        case TransDateFilterType::Days120: AdjustDays = 120; break;
        }
        wszStartDate = AfxDateAddDays(wszEndDate, -(AdjustDays));
    }

    CustomLabel_SetUserData(GetDlgItem(hwnd, IDC_TRANS_STARTDATE), wszStartDate);
    CustomLabel_SetText(GetDlgItem(hwnd, IDC_TRANS_STARTDATE), AfxLongDate(wszStartDate));

    CustomLabel_SetUserData(GetDlgItem(hwnd, IDC_TRANS_ENDDATE), wszEndDate);
    CustomLabel_SetText(GetDlgItem(hwnd, IDC_TRANS_ENDDATE), AfxLongDate(wszEndDate));
}


// ========================================================================================
// Central function that actually selects and displays the incoming ListBox index item.
// ========================================================================================
void TransPanel_ShowListBoxItem(int index)
{
    HWND hListBox = GetDlgItem(HWND_TRANSPANEL, IDC_TRANS_LISTBOX);
    HWND hCustomVScrollBar = GetDlgItem(HWND_TRANSPANEL, IDC_TRANS_CUSTOMVSCROLLBAR);

    ListBox_SetCurSel(hListBox, index);

    // Ensure that the Transactions menu item is selected
    //SideMenu_SelectMenuItem(HWND_SIDEMENU, IDC_SIDEMENU_TRANSACTIONS);

    //  update the scrollbar position if necessary
    CustomVScrollBar_Recalculate(hCustomVScrollBar);

    // Get the current line to determine if a valid Trade pointer exists so that we
    // can show the transaction detail.
    if (index > -1) {
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, index);
        if (ld != nullptr)
            TransDetail_ShowTransDetail(ld->trade, ld->trans);
    }

    SetFocus(hListBox);
}


// ========================================================================================
// Populate the Transaction ListBox with the Transactions per the user selected dates.
// ========================================================================================
void TransPanel_ShowTransactions()
{
    HWND hListBox = GetDlgItem(HWND_TRANSPANEL, IDC_TRANS_LISTBOX);
    HWND hCustomVScrollBar = GetDlgItem(HWND_TRANSPANEL, IDC_TRANS_CUSTOMVSCROLLBAR);

    // Select the correct menu panel item
    TabPanel_SelectPanelItem(HWND_TABPANEL, IDC_TABPANEL_TRANSACTIONS);

    // Prevent ListBox redrawing until all calculations are completed
    SendMessage(hListBox, WM_SETREDRAW, FALSE, 0);


    struct TransData {
        std::shared_ptr<Trade> trade;
        std::shared_ptr<Transaction> trans;
    };
    std::vector<TransData> tdata;
    tdata.reserve(2000);    // reserve space for 2000 Transactions

    bool processTrade = false;
    std::wstring wszStartDate = CustomLabel_GetUserData(GetDlgItem(HWND_TRANSPANEL, IDC_TRANS_STARTDATE));
    std::wstring wszEndDate = CustomLabel_GetUserData(GetDlgItem(HWND_TRANSPANEL, IDC_TRANS_ENDDATE));

    std::wstring wszTicker = AfxGetWindowText(GetDlgItem(HWND_TRANSPANEL, IDC_TRANS_TXTTICKER));
    wszTicker = AfxTrim(wszTicker);

    for (auto& trade : trades) {
        for (auto& trans : trade->transactions) {
            if (wszTicker.length() > 0) {
                if (wszTicker != trade->ticker_symbol) continue;
            }

            if (trans->trans_date < wszStartDate || trans->trans_date > wszEndDate) continue;

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
    ListBoxData_DestroyItemData(hListBox);


    // Create the new Listbox data that will display for the Transactions
    double subGrossAmount = 0;
    double subFeesAmount = 0;
    double subNetAmount = 0;

    double runningGrossTotal = 0;
    double runningFeesTotal = 0;
    double runningNetTotal = 0;

    int subTotalDay = 0;
    int curDay = 0;
    int current_year = 0;
    std::wstring curDate = L"";

    for (const auto& td : tdata) {
        curDay = AfxGetDay(td.trans->trans_date);
        if (subTotalDay != curDay && subTotalDay != 0 && wszTicker.length() == 0) {
            ListBoxData_OutputTransactionDaySubtotal(hListBox, curDate, subGrossAmount, subFeesAmount, subNetAmount);
            curDate = td.trans->trans_date;
            subGrossAmount = 0;
            subFeesAmount = 0;
            subNetAmount = 0;
        }
        curDate = td.trans->trans_date;
        subTotalDay = curDay;
            
        subNetAmount += td.trans->total;
        subFeesAmount += td.trans->fees;
        subGrossAmount += (td.trans->total + td.trans->fees);
        ListBoxData_OutputTransaction(hListBox, td.trade, td.trans);

        runningNetTotal += td.trans->total;
        runningFeesTotal += td.trans->fees;
        runningGrossTotal += (td.trans->total + td.trans->fees);
    }
    if (subNetAmount != 0 && wszTicker.length() == 0) {
        ListBoxData_OutputTransactionDaySubtotal(hListBox, curDate, subGrossAmount, subFeesAmount, subNetAmount);
    }
    ListBoxData_OutputTransactionRunningTotal(hListBox, runningGrossTotal, runningFeesTotal, runningNetTotal);


    // Calculate the actual column widths based on the size of the strings in
    // ListBoxData while respecting the minimum values as defined in nMinColWidth[].
    ListBoxData_ResizeColumnWidths(hListBox, TableType::trans_panel, -1);


    // Set the ListBox to the topline.
    ListBox_SetTopIndex(hListBox, 0);


    // If no transactions then add at least one line
    if (ListBox_GetCount(hListBox)) {
        ListBoxData_AddBlankLine(hListBox);
    }

    // Ensure that the TransDetail panel and Detail Panel are set
    MainWindow_SetLeftPanel(HWND_TRANSPANEL);
    MainWindow_SetRightPanel(HWND_TRANSDETAIL);

    CustomVScrollBar_Recalculate(hCustomVScrollBar);

    // Select row past the YTD total line if possible
    int curSel = min(ListBox_GetCount(hListBox) - 1, 2);
    TransPanel_ShowListBoxItem(curSel);

    // Redraw the ListBox to ensure that any recalculated columns are 
    // displayed correctly. Re-enable redraw.
    SendMessage(hListBox, WM_SETREDRAW, TRUE, 0);
    AfxRedrawWindow(hListBox);

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

    case WM_KEYDOWN:
    {
        // Parent to handle the TAB navigation key to move amongst constrols.
        if (wParam == VK_TAB) {
            if (SendMessage(GetParent(hWnd), uMsg, wParam, lParam) == TRUE)
                return 0;
        }
    }
    break;


    case WM_MOUSEWHEEL:
    {
        // Accumulate delta until scroll one line (up +120, down -120). 
        // 120 is the Microsoft default delta
        int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        int top_index = (int)SendMessage(hWnd, LB_GETTOPINDEX, 0, 0);
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
        HWND hCustomVScrollBar = GetDlgItem(HWND_TRANSPANEL, IDC_TRANS_CUSTOMVSCROLLBAR);
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

        // Show the Transaction detail for the selected line.
        TransPanel_ShowListBoxItem(idx);
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
        int items_count = ListBox_GetCount(hWnd);
        int top_index = (int)SendMessage(hWnd, LB_GETTOPINDEX, 0, 0);
        int visible_rows = 0;
        int items_per_page = 0;
        int bottom_index = 0;
        int nWidth = (rc.right - rc.left);
        int nHeight = (rc.bottom - rc.top);

        if (items_count > 0) {
            items_per_page = (nHeight) / itemHeight;
            bottom_index = (top_index + items_per_page);
            if (bottom_index >= items_count)
                bottom_index = items_count - 1;
            visible_rows = (bottom_index - top_index) + 1;
            rc.top = visible_rows * itemHeight;
        }

        if (rc.top < rc.bottom) {
            nHeight = (rc.bottom - rc.top);
            HDC hDC = (HDC)wParam;
            Graphics graphics(hDC);
            Color back_color(COLOR_GRAYDARK);
            SolidBrush back_brush(back_color);
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
    lpMeasureItem->itemHeight = AfxScaleY(TRANSPANEL_LISTBOX_ROWHEIGHT);
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

    // Create the background brush
    Color back_color(COLOR_BLACK);
    SolidBrush back_brush(back_color);

    // Paint the background using brush.
    int nWidth = (ps.rcPaint.right - ps.rcPaint.left);
    int nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

    // Paint the area to the left of the ListBox in order to give the illusion
    // of a margin before the ListBox data is displyed.
    ps.rcPaint.top += AfxScaleY(TRANSPANEL_MARGIN);
    ps.rcPaint.right = ps.rcPaint.left + AfxScaleX(APP_LEFTMARGIN_WIDTH);

    // Set the background brush
    back_color.SetValue(COLOR_GRAYDARK);
    back_brush.SetColor(back_color);
    nWidth = (ps.rcPaint.right - ps.rcPaint.left);
    nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

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


    HDWP hdwp = BeginDeferWindowPos(15);

    int margin = AfxScaleY(TRANSPANEL_MARGIN);
    int nLeft = AfxScaleY(APP_LEFTMARGIN_WIDTH);
    int nTop = 0;
    int nWidth = 0;
    int nHeight = AfxScaleY(23);
    int nStartTop = nTop;

    nStartTop = nHeight;

    nTop = nStartTop;
    nWidth = AfxScaleX(75);
    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_TRANS_LBLTICKERFILTER), 0,
        nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);
    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_TRANS_TXTTICKER), 0,
        nLeft, nTop + nHeight, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);
    nLeft += nWidth;
    nWidth = AfxScaleX(23);
    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_TRANS_CMDTICKERGO), 0,
        nLeft, nTop + nHeight, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nTop = nStartTop;
    nLeft += nWidth + AfxScaleX(18);
    nWidth = AfxScaleX(90);
    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_TRANS_LBLDATEFILTER), 0,
        nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);
    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_TRANS_TRANSDATE), 0,
        nLeft, nTop + nHeight, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);
    nLeft += nWidth;
    nWidth = AfxScaleX(23);
    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_TRANS_CMDTRANSDATE), 0,
        nLeft, nTop + nHeight, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nTop = nStartTop;
    nLeft += nWidth + AfxScaleX(18);
    nWidth = AfxScaleX(90);
    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_TRANS_LBLSTARTDATE), 0,
        nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);
    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_TRANS_STARTDATE), 0,
        nLeft, nTop + nHeight, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);
    nLeft += nWidth;
    nWidth = AfxScaleX(23);
    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_TRANS_CMDSTARTDATE), 0,
        nLeft, nTop + nHeight, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nTop = nStartTop;
    nLeft += nWidth + AfxScaleX(18);
    nWidth = AfxScaleX(90);
    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_TRANS_LBLENDDATE), 0,
        nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);
    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_TRANS_ENDDATE), 0,
        nLeft, nTop + nHeight, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);
    nLeft += nWidth;
    nWidth = AfxScaleX(23);
    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_TRANS_CMDENDDATE), 0,
        nLeft, nTop + nHeight, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nLeft = AfxScaleX(APP_LEFTMARGIN_WIDTH);
    nTop = margin;
    nWidth = cx;
    nHeight = AfxScaleY(TRANSPANEL_LISTBOX_ROWHEIGHT);

    hdwp = DeferWindowPos(hdwp, hHeader, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);
    nTop = nTop + nHeight + AfxScaleY(1);

    nWidth = cx - nLeft - custom_scrollbar_width;
    nHeight = cy - nTop;
    hdwp = DeferWindowPos(hdwp, hListBox, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nLeft = nLeft + nWidth;   // right edge of ListBox
    nWidth = custom_scrollbar_width;
    hdwp = DeferWindowPos(hdwp, hCustomVScrollBar, 0, nLeft, nTop, nWidth, nHeight,
        SWP_NOZORDER | (bshow_scrollbar ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));

    EndDeferWindowPos(hdwp);
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: TransPanel
// ========================================================================================
BOOL TransPanel_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    HWND_TRANSPANEL = hwnd;

    int horiz_text_margin = 0;
    int vert_text_margin = 3;

    DWORD light_text_color = COLOR_WHITEDARK;
    DWORD dark_back_color = COLOR_GRAYMEDIUM;
    std::wstring font_name = AfxGetDefaultFont();
    int font_size = 8;

    CustomLabel_SimpleLabel(hwnd, IDC_TRANS_LBLTICKERFILTER, L"Ticker Filter",
        COLOR_WHITEDARK, COLOR_BLACK);
    HWND hCtl = CreateCustomTextBox(hwnd, IDC_TRANS_TXTTICKER, false, ES_LEFT | ES_UPPERCASE, L"", 0, 0, 0, 0);
    CustomTextBox_SetMargins(hCtl, horiz_text_margin, vert_text_margin);
    CustomTextBox_SetColors(hCtl, light_text_color, dark_back_color);
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_TRANS_CMDTICKERGO, L"GO",
        COLOR_BLACK, COLOR_BLUE, COLOR_BLUE, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_center, 0, 0, 0, 0);
    CustomLabel_SetMousePointer(hCtl, CustomLabelPointer::hand, CustomLabelPointer::hand);
    CustomLabel_SetFont(hCtl, font_name, font_size, true);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);

    
    CustomLabel_SimpleLabel(hwnd, IDC_TRANS_LBLDATEFILTER, L"Date Filter",
        COLOR_WHITEDARK, COLOR_BLACK);
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_TRANS_TRANSDATE, L"7 days",
        COLOR_WHITEDARK, COLOR_GRAYMEDIUM, COLOR_GRAYMEDIUM, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_left, 0, 0, 0, 0);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);
    CustomLabel_SetMousePointer(hCtl, CustomLabelPointer::hand, CustomLabelPointer::hand);
    CustomLabel_SetUserDataInt(hCtl, (int)TransDateFilterType::Days7);

    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_TRANS_CMDTRANSDATE, GLYPH_DROPDOWN,
        COLOR_WHITEDARK, COLOR_GRAYMEDIUM, COLOR_GRAYLIGHT, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_center, 0, 0, 0, 0);
    CustomLabel_SetFont(hCtl, font_name, font_size, true);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);


    CustomLabel_SimpleLabel(hwnd, IDC_TRANS_LBLSTARTDATE, L"Start Date",
        COLOR_WHITEDARK, COLOR_BLACK);
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_TRANS_STARTDATE, L"",
        COLOR_WHITEDARK, COLOR_GRAYMEDIUM, COLOR_GRAYMEDIUM, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_left, 0, 0, 0, 0);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);
    CustomLabel_SetMousePointer(hCtl, CustomLabelPointer::hand, CustomLabelPointer::hand);
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_TRANS_CMDSTARTDATE, GLYPH_DROPDOWN,
        COLOR_WHITEDARK, COLOR_GRAYMEDIUM, COLOR_GRAYLIGHT, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_center, 0, 0, 0, 0);
    CustomLabel_SetFont(hCtl, font_name, font_size, true);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);


    CustomLabel_SimpleLabel(hwnd, IDC_TRANS_LBLENDDATE, L"End Date",
        COLOR_WHITEDARK, COLOR_BLACK);
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_TRANS_ENDDATE, L"",
        COLOR_WHITEDARK, COLOR_GRAYMEDIUM, COLOR_GRAYMEDIUM, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_left, 0, 0, 0, 0);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);
    CustomLabel_SetMousePointer(hCtl, CustomLabelPointer::hand, CustomLabelPointer::hand);
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_TRANS_CMDENDDATE, GLYPH_DROPDOWN,
        COLOR_WHITEDARK, COLOR_GRAYMEDIUM, COLOR_GRAYLIGHT, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_center, 0, 0, 0, 0);
    CustomLabel_SetFont(hCtl, font_name, font_size, true);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);


    // Set the Start & End dates based on the filter type.
    TransPanel_SetStartEndDates(hwnd);


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


    // Create an Ownerdraw listbox that we will use to custom paint our Transactions.
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

    SetFocus(GetDlgItem(hwnd, IDC_TRANS_TXTTICKER));

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


    case WM_KEYDOWN:
    {
        // We are handling the TAB naviagation ourselves.
        if (wParam == VK_TAB) {
            HWND hFocus = GetFocus();
            HWND hNextCtrl = NULL;
            if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
                hNextCtrl = GetNextDlgTabItem(m_hwnd, hFocus, TRUE);
            }
            else {
                hNextCtrl = GetNextDlgTabItem(m_hwnd, hFocus, FALSE);
            }
            SetFocus(hNextCtrl);
            return TRUE;
        }
        return 0;
    }
    break;


    case WM_KEYUP:
    {
        // Handle ENTER key if pressed in the Ticker textbox.
        HWND hTicker = GetDlgItem(m_hwnd, IDC_TRANS_TXTTICKER);
        if (GetFocus() == GetDlgItem(hTicker, 100)) {
            if (wParam == 13) {
                TransPanel_ShowTransactions();
                return true;
            }
        }
        return 0;
    }
    break;


    case MSG_DATEPICKER_DATECHANGED:
    {
        // If the StartDate or EndDate is changed then we set the DateFilter to Custom.
        if ((HWND)lParam == GetDlgItem(m_hwnd, IDC_TRANS_STARTDATE) ||
            (HWND)lParam == GetDlgItem(m_hwnd, IDC_TRANS_ENDDATE)) {
            CustomLabel_SetUserDataInt(GetDlgItem(m_hwnd, IDC_TRANS_TRANSDATE), 
                (int)TransDateFilterType::Custom);
            CustomLabel_SetText(GetDlgItem(m_hwnd, IDC_TRANS_TRANSDATE),
                TransDateFilter_GetString((int)TransDateFilterType::Custom).c_str());
        }

        TransPanel_SetStartEndDates(m_hwnd);
        TransPanel_ShowTransactions();
        return 0;
    }
    break;


    case MSG_CUSTOMLABEL_CLICK:
    {
        HWND hCtl = (HWND)lParam;
        int CtrlId = (int)wParam;

        if (hCtl == NULL) return 0;

        if (CtrlId == IDC_TRANS_CMDTICKERGO) {
            TransPanel_ShowTransactions();
        }

        if (CtrlId == IDC_TRANS_CMDTRANSDATE || CtrlId == IDC_TRANS_TRANSDATE) {
            // Clicked on the Date Filter dropdown or label itself
            TransDateFilter_CreatePicker(m_hwnd, GetDlgItem(m_hwnd, IDC_TRANS_TRANSDATE));
        }

        if (CtrlId == IDC_TRANS_CMDSTARTDATE || CtrlId == IDC_TRANS_STARTDATE) {
            // Clicked on the Start Date dropdown or label itself
            std::wstring date_text = CustomLabel_GetUserData(GetDlgItem(m_hwnd, IDC_TRANS_STARTDATE));
            Calendar_CreateDatePicker(
                m_hwnd, GetDlgItem(m_hwnd, IDC_TRANS_STARTDATE), date_text, CalendarPickerReturnType::long_date, 1);
        }

        if (CtrlId == IDC_TRANS_CMDENDDATE || CtrlId == IDC_TRANS_ENDDATE) {
            // Clicked on the End Date dropdown or label itself
            std::wstring date_text = CustomLabel_GetUserData(GetDlgItem(m_hwnd, IDC_TRANS_ENDDATE));
            Calendar_CreateDatePicker(
                m_hwnd, GetDlgItem(m_hwnd, IDC_TRANS_ENDDATE), date_text, CalendarPickerReturnType::long_date, 1);
        }

        return 0;
    }
    break;

    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}

