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
#include "CustomVScrollBar/CustomVScrollBar.h"
#include "Utilities/ListBoxData.h"

#include "DailyTotals.h"


HWND HWND_DAILYTOTALS = NULL;

extern CDailyTotals DailyTotals;
extern HWND HWND_RIGHTPANEL;

extern std::vector<std::shared_ptr<Trade>> trades;
extern int nColWidth[];

extern void MainWindow_SetRightPanel(HWND hPanel);


// ========================================================================================
// Populate the ListBox with the total earnings per day.
// ========================================================================================
void DailyTotals_ShowDailyTotals(const ListBoxData* ld)
{
    HWND hListBox = GetDlgItem(HWND_DAILYTOTALS, IDC_DAILY_LISTBOX);
    HWND hListBoxSummary = GetDlgItem(HWND_DAILYTOTALS, IDC_DAILY_LISTBOX_SUMMARY);
    HWND hCustomVScrollBar = GetDlgItem(HWND_DAILYTOTALS, IDC_DAILY_CUSTOMVSCROLLBAR);


    // Default to opening the current date
    std::wstring selectedDate = AfxCurrentDate();
    bool isOpen = true;

    if (ld != nullptr) {
        if (ld->isDailyTotalsNode) {
            selectedDate = ld->DailyTotalsDate;
            // If the node is already open then we close it, otherwise open it.
            isOpen = ld->isDailyTotalsNodeOpen ? false : true;
            if (!isOpen) selectedDate = L"";
        }
        else {
            return;
        }
    }


    // Prevent ListBox redrawing until all calculations are completed
    SendMessage(hListBox, WM_SETREDRAW, FALSE, 0);

    // Save the previously top line of the ListBox so that it can be restored
    // after the new states are determined.
    int nTopLine = ListBox_GetTopIndex(hListBox);


    // Clear the current history table
    ListBoxData_DestroyItemData(hListBox);
    ListBoxData_DestroyItemData(hListBoxSummary);


    // Calculate the daily amounts
    // Map contains a vector for every unique date.
    struct MapData {
        std::shared_ptr<Trade> trade;
        std::shared_ptr<Transaction> trans;
    };

    std::map< std::wstring, std::vector<MapData> > mapTotals;
    mapTotals.clear();

    int currentYear = AfxLocalYear();
    int currentMonth = AfxLocalMonth();
    double MTD = 0;
    double YTD = 0;

    for (const auto& trade : trades) {
        for (const auto& trans : trade->Transactions) {
            MapData data{ trade, trans };
            mapTotals[trans->transDate].push_back(data);
        }
    }

    // Iterate the map in reverse and display the contents in the table
    double grandTotal = 0;

    std::wstring wszDate;
    for (auto iter = mapTotals.rbegin(); iter != mapTotals.rend(); ++iter) {

        double dayTotal = 0;
        wszDate = iter->first;

        // Calculate the day total for this date
        for (const auto& data : iter->second) {
            dayTotal += data.trans->total;
        }
        // Increase the MTD and YTD based on the date
        if (AfxGetYear(wszDate) == currentYear) {
            YTD += dayTotal;
            if (AfxGetMonth(wszDate) == currentMonth) MTD += dayTotal;
        }

        // Expand the line if necessary
        if (wszDate == selectedDate) {
            ListBoxData_OutputDailyTotalsNodeHeader(hListBox, wszDate, dayTotal, isOpen);
            for (auto& data : iter->second) {
                ListBoxData_OutputDailyTotalsDetailLine(hListBox, data.trade, data.trans);
            }
        }
        else {
            ListBoxData_OutputDailyTotalsNodeHeader(hListBox, wszDate, dayTotal, false);
        }
        grandTotal += dayTotal;
    }

    // Output the data for the Daily Totals Summary
    ListBoxData_OutputDailyTotalsSummary(hListBoxSummary, grandTotal, MTD, YTD);


    // Calculate the actual column widths based on the size of the strings in
    // ListBoxData while respecting the minimum values as defined in nMinColWidth[].
    // This function is also called when receiving new price data from TWS because
    // that data may need the column width to be wider.
    ListBoxData_ResizeColumnWidths(hListBox, TableType::DailyTotals, -1);
    ListBoxData_ResizeColumnWidths(hListBoxSummary, TableType::DailyTotalsSummary, -1);


    ListBoxData_AddBlankLine(hListBox);

    // Set the ListBox to the previous topline.
    ListBox_SetTopIndex(hListBox, nTopLine);


    // Redraw the ListBox to ensure that any recalculated columns are 
    // displayed correctly. Re-enable redraw.
    SendMessage(hListBox, WM_SETREDRAW, TRUE, 0);
    AfxRedrawWindow(hListBox);


    // Ensure that the Daily panel is set
    MainWindow_SetRightPanel(HWND_DAILYTOTALS);

    CustomVScrollBar_Recalculate(hCustomVScrollBar);
}


// ========================================================================================
// Header control subclass Window procedure
// ========================================================================================
LRESULT CALLBACK DailyTotals_Header_SubclassProc(
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
    {
        // REQUIRED: Remove control subclassing
        RemoveWindowSubclass(hWnd, DailyTotals_Header_SubclassProc, uIdSubclass);
        break;
    }


    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);

}


// ========================================================================================
// Listbox subclass Window procedure
// ========================================================================================
LRESULT CALLBACK DailyTotals_ListBox_SubclassProc(
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
        HWND hCustomVScrollBar = GetDlgItem(HWND_DAILYTOTALS, IDC_DAILY_CUSTOMVSCROLLBAR);
        CustomVScrollBar_Recalculate(hCustomVScrollBar);
        return 0;
        break;
    }


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
            SolidBrush backBrush(COLOR_GRAYDARK);
            graphics.FillRectangle(&backBrush, rc.left, rc.top, nWidth, nHeight);
        }

        ValidateRect(hWnd, &rc);
        return TRUE;
        break;
    }


    case WM_DESTROY:
    {
        // Destroy all manually allocated ListBox display data that is held
        // in the LineData structures..
        ListBoxData_DestroyItemData(hWnd);

        // REQUIRED: Remove control subclassing
        RemoveWindowSubclass(hWnd, DailyTotals_ListBox_SubclassProc, uIdSubclass);
        break;
    }


    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}



// ========================================================================================
// Process WM_MEASUREITEM message for window/dialog: DailyTotals
// ========================================================================================
void DailyTotals_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT* lpMeasureItem)
{
    lpMeasureItem->itemHeight = AfxScaleY(DAILY_LISTBOX_ROWHEIGHT);
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: DailyTotals
// ========================================================================================
BOOL DailyTotals_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    // Handle all of the painting in WM_PAINT
    return TRUE;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: DailyTotals
// ========================================================================================
void DailyTotals_OnPaint(HWND hwnd)
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
// Process WM_SIZE message for window/dialog: DailyTotals
// ========================================================================================
void DailyTotals_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    HWND hHeaderDailySummary = GetDlgItem(hwnd, IDC_DAILY_HEADER_SUMMARY);
    HWND hHeaderDailyTotals = GetDlgItem(hwnd, IDC_DAILY_HEADER_TOTALS);
    HWND hListBox = GetDlgItem(hwnd, IDC_DAILY_LISTBOX);
    HWND hCustomVScrollBar = GetDlgItem(hwnd, IDC_DAILY_CUSTOMVSCROLLBAR);

    int margin = AfxScaleY(DAILYTOTALS_MARGIN);

    HDWP hdwp = BeginDeferWindowPos(10);

    // Move and size the top label into place
    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_DAILY_SYMBOL), 0,
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

    int nTop = margin;
    int nLeft = 0;
    int nWidth = cx;
    int nHeight = AfxScaleY(DAILY_LISTBOX_ROWHEIGHT);


    hdwp = DeferWindowPos(hdwp, hHeaderDailySummary, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);
    nTop = nTop + nHeight + AfxScaleX(1);

    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_DAILY_LISTBOX_SUMMARY), 0,
        nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nTop = AfxScaleY(90);

    hdwp = DeferWindowPos(hdwp, hHeaderDailyTotals, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);
    nTop = nTop + nHeight + AfxScaleX(1);

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
// Process WM_CREATE message for window/dialog: DailyTotals
// ========================================================================================
BOOL DailyTotals_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    HWND_DAILYTOTALS = hwnd;

    HWND hCtl = CustomLabel_SimpleLabel(hwnd, IDC_DAILY_SYMBOL, L"Daily Totals",
        COLOR_WHITELIGHT, COLOR_BLACK);

    // Create an listbox that we will use to custom paint our daily totals.
    hCtl =
        DailyTotals.AddControl(Controls::ListBox, hwnd, IDC_DAILY_LISTBOX, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWFIXED | LBS_NOTIFY,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)DailyTotals_ListBox_SubclassProc,
            IDC_DAILY_LISTBOX, NULL);
    ListBox_AddString(hCtl, NULL);

    // Create our custom vertical scrollbar and attach the ListBox to it.
    CreateCustomVScrollBar(hwnd, IDC_DAILY_CUSTOMVSCROLLBAR, hCtl);


    // Create Header control for our Daily History Summary output
    hCtl = DailyTotals.AddControl(Controls::Header, hwnd, IDC_DAILY_HEADER_SUMMARY,
        L"", 0, 0, 0, 0, -1, -1, NULL, (SUBCLASSPROC)DailyTotals_Header_SubclassProc,
        IDC_DAILY_HEADER_SUMMARY, NULL);
    int nWidth = AfxScaleX(50);
    Header_InsertNewItem(hCtl, 0, nWidth, L"Profit/Loss", HDF_CENTER);
    Header_InsertNewItem(hCtl, 1, nWidth, L"Stock Value", HDF_CENTER);
    Header_InsertNewItem(hCtl, 2, nWidth, L"Net Profit", HDF_CENTER);
    Header_InsertNewItem(hCtl, 3, nWidth, L"MTD", HDF_CENTER);
    Header_InsertNewItem(hCtl, 4, nWidth, L"YTD", HDF_CENTER);

    // Create Header control for our Daily History Totals output
    hCtl = DailyTotals.AddControl(Controls::Header, hwnd, IDC_DAILY_HEADER_TOTALS,
        L"", 0, 0, 0, 0, -1, -1, NULL, (SUBCLASSPROC)DailyTotals_Header_SubclassProc,
        IDC_DAILY_HEADER_TOTALS, NULL);
    Header_InsertNewItem(hCtl, 0, nWidth, L"", HDF_LEFT);
    Header_InsertNewItem(hCtl, 1, nWidth, L"Date", HDF_LEFT);
    Header_InsertNewItem(hCtl, 2, nWidth, L"Day/Description", HDF_LEFT);
    Header_InsertNewItem(hCtl, 3, nWidth, L"Amount", HDF_RIGHT);

    // Create an Ownerdraw listbox that we will use to custom
    // paint the Daily History Summary.
    hCtl =
        DailyTotals.AddControl(Controls::ListBox, hwnd, IDC_DAILY_LISTBOX_SUMMARY, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWFIXED | LBS_NOSEL | LBS_NOTIFY,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)DailyTotals_ListBox_SubclassProc,
            IDC_DAILY_LISTBOX, NULL);
    ListBox_AddString(hCtl, NULL);

    return TRUE;
}


// ========================================================================================
// Process WM_COMMAND message for window/dialog: DailyTotals
// ========================================================================================
void DailyTotals_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    if (id == IDC_DAILY_LISTBOX && codeNotify == LBN_SELCHANGE) {
        // The date will be valid if we are "opening" the node, or simply null if
        // we will just show all closed nodes.
        int idx = ListBox_GetCurSel(hwndCtl);
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hwndCtl, idx);
        if (ld != nullptr) {
            DailyTotals_ShowDailyTotals(ld);
        }
    }
}



// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CDailyTotals::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(m_hwnd, WM_CREATE, DailyTotals_OnCreate);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, DailyTotals_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, DailyTotals_OnPaint);
        HANDLE_MSG(m_hwnd, WM_SIZE, DailyTotals_OnSize);
        HANDLE_MSG(m_hwnd, WM_COMMAND, DailyTotals_OnCommand);
        HANDLE_MSG(m_hwnd, WM_MEASUREITEM, DailyTotals_OnMeasureItem);
        HANDLE_MSG(m_hwnd, WM_DRAWITEM, ListBoxData_OnDrawItem);

    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}
