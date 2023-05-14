
#include "pch.h"
#include "..\CustomLabel\CustomLabel.h"
#include "..\CustomVScrollBar\CustomVScrollBar.h"
#include "..\Utilities\ListBoxData.h"

#include "DailyPanel.h"


HWND HWND_DAILYPANEL = NULL;

extern CDailyPanel DailyPanel;

extern std::vector<std::shared_ptr<Trade>> trades;
extern int nColWidth[];

extern void MainWindow_SetRightPanel(HWND hPanel);

void DailyPanel_OnSize(HWND hwnd, UINT state, int cx, int cy);



// ========================================================================================
// Populate the ListBox with the total earnings per day.
// ========================================================================================
void DailyPanel_ShowDailyTotals(const ListBoxData* ld)
{
    HWND hListBox = GetDlgItem(HWND_DAILYPANEL, IDC_DAILY_LISTBOX);
    HWND hListBoxSummary = GetDlgItem(HWND_DAILYPANEL, IDC_DAILY_LISTBOX_SUMMARY);
    HWND hCustomVScrollBar = GetDlgItem(HWND_DAILYPANEL, IDC_DAILY_CustomVScrollBar);


    // Ensure that the Daily panel is set
    MainWindow_SetRightPanel(HWND_DAILYPANEL);


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
        for (const auto& trans : trade->transactions) {
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


    ListBoxData_HistoryBlankLine(hListBox);

    // Set the ListBox to the previous topline.
    ListBox_SetTopIndex(hListBox, nTopLine);


    // Redraw the ListBox to ensure that any recalculated columns are 
    // displayed correctly. Re-enable redraw.
    SendMessage(hListBox, WM_SETREDRAW, TRUE, 0);
    AfxRedrawWindow(hListBox);


    // Need to force a resize of the panel in order to properly show (or not show) 
    // and position the Summary listbox and daily detail listbox.
    RECT rc; GetClientRect(HWND_DAILYPANEL, &rc);
    DailyPanel_OnSize(HWND_DAILYPANEL, 0, rc.right, rc.bottom);

    CustomVScrollBar_Recalculate(hCustomVScrollBar);
}


// ========================================================================================
// Header control subclass Window procedure
// ========================================================================================
LRESULT CALLBACK DailyPanel_Header_SubclassProc(
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
        break;
    }


    case WM_DESTROY:

        // REQUIRED: Remove control subclassing
        RemoveWindowSubclass(hWnd, DailyPanel_Header_SubclassProc, uIdSubclass);
        break;


    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);

}


// ========================================================================================
// Listbox subclass Window procedure
// ========================================================================================
LRESULT CALLBACK DailyPanel_ListBox_SubclassProc(
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
        HWND hCustomVScrollBar = GetDlgItem(HWND_DAILYPANEL, IDC_DAILY_CustomVScrollBar);
        CustomVScrollBar_Recalculate(hCustomVScrollBar);
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
        RemoveWindowSubclass(hWnd, DailyPanel_ListBox_SubclassProc, uIdSubclass);
        break;


    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);

}



// ========================================================================================
// Process WM_MEASUREITEM message for window/dialog: DailyPanel
// ========================================================================================
void DailyPanel_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT* lpMeasureItem)
{
    lpMeasureItem->itemHeight = AfxScaleY(DAILY_LISTBOX_ROWHEIGHT);
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: DailyPanel
// ========================================================================================
BOOL DailyPanel_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    // Handle all of the painting in WM_PAINT
    return TRUE;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: DailyPanel
// ========================================================================================
void DailyPanel_OnPaint(HWND hwnd)
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
// Process WM_SIZE message for window/dialog: DailyPanel
// ========================================================================================
void DailyPanel_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    HWND hHeaderDailySummary = GetDlgItem(hwnd, IDC_DAILY_HEADER_SUMMARY);
    HWND hHeaderDailyTotals = GetDlgItem(hwnd, IDC_DAILY_HEADER_TOTALS);
    HWND hListBox = GetDlgItem(hwnd, IDC_DAILY_LISTBOX);
    HWND hCustomVScrollBar = GetDlgItem(hwnd, IDC_DAILY_CustomVScrollBar);

    int margin = AfxScaleY(DAILYPANEL_MARGIN);

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
    int CustomVScrollBarWidth = bShowScrollBar ? AfxScaleX(CustomVScrollBar_WIDTH) : 0;

    int nTop = margin;
    int nLeft = 0;
    int nWidth = cx;
    int nHeight = AfxScaleY(DAILY_LISTBOX_ROWHEIGHT);


    nTop += AfxScaleY(6);
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
// Process WM_CREATE message for window/dialog: DailyPanel
// ========================================================================================
BOOL DailyPanel_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    HWND_DAILYPANEL = hwnd;

    HWND hCtl = CustomLabel_SimpleLabel(hwnd, IDC_DAILY_SYMBOL, L"Daily Totals",
        ThemeElement::WhiteLight, ThemeElement::Black);

    // Create an listbox that we will use to custom paint our various open trades.
    hCtl =
        DailyPanel.AddControl(Controls::ListBox, hwnd, IDC_DAILY_LISTBOX, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_EXTENDEDSEL | LBS_MULTIPLESEL |
            LBS_OWNERDRAWFIXED | LBS_NOTIFY,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)DailyPanel_ListBox_SubclassProc,
            IDC_DAILY_LISTBOX, NULL);
    ListBox_AddString(hCtl, NULL);

    // Create our custom vertical scrollbar and attach the ListBox to it.
    CreateCustomVScrollBar(hwnd, IDC_DAILY_CustomVScrollBar, hCtl);


    // Create Header control for our Daily History Summary output
    hCtl = DailyPanel.AddControl(Controls::Header, hwnd, IDC_DAILY_HEADER_SUMMARY,
        L"", 0, 0, 0, 0, -1, -1, NULL, (SUBCLASSPROC)DailyPanel_Header_SubclassProc,
        IDC_DAILY_HEADER_SUMMARY, NULL);
    int nWidth = AfxScaleX(50);
    Header_InsertNewItem(hCtl, 0, nWidth, L"Profit/Loss", HDF_CENTER);
    Header_InsertNewItem(hCtl, 1, nWidth, L"Stock Value", HDF_CENTER);
    Header_InsertNewItem(hCtl, 2, nWidth, L"Net Profit", HDF_CENTER);
    Header_InsertNewItem(hCtl, 3, nWidth, L"MTD", HDF_CENTER);
    Header_InsertNewItem(hCtl, 4, nWidth, L"YTD", HDF_CENTER);

    // Create Header control for our Daily History Totals output
    hCtl = DailyPanel.AddControl(Controls::Header, hwnd, IDC_DAILY_HEADER_TOTALS,
        L"", 0, 0, 0, 0, -1, -1, NULL, (SUBCLASSPROC)DailyPanel_Header_SubclassProc,
        IDC_DAILY_HEADER_TOTALS, NULL);
    Header_InsertNewItem(hCtl, 0, nWidth, L"", HDF_LEFT);
    Header_InsertNewItem(hCtl, 1, nWidth, L"Date", HDF_LEFT);
    Header_InsertNewItem(hCtl, 2, nWidth, L"Day/Description", HDF_LEFT);
    Header_InsertNewItem(hCtl, 3, nWidth, L"Amount", HDF_RIGHT);

    // Create an Ownerdraw listbox that we will use to custom
    // paint the Daily History Summary.
    hCtl =
        DailyPanel.AddControl(Controls::ListBox, hwnd, IDC_DAILY_LISTBOX_SUMMARY, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_EXTENDEDSEL | LBS_MULTIPLESEL |
            LBS_OWNERDRAWFIXED | LBS_NOTIFY,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)DailyPanel_ListBox_SubclassProc,
            IDC_DAILY_LISTBOX, NULL);
    ListBox_AddString(hCtl, NULL);

    return TRUE;
}


// ========================================================================================
// Process WM_COMMAND message for window/dialog: DailyPanel
// ========================================================================================
void DailyPanel_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    if (id == IDC_DAILY_LISTBOX && codeNotify == LBN_SELCHANGE) {
        // The date will be valid if we are "opening" the node, or simply null if
        // we will just show all closed nodes.
        int idx = ListBox_GetCurSel(hwndCtl);
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hwndCtl, idx);
        if (ld != nullptr) {
            DailyPanel_ShowDailyTotals(ld);
        }
    }
}



// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CDailyPanel::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(m_hwnd, WM_CREATE, DailyPanel_OnCreate);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, DailyPanel_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, DailyPanel_OnPaint);
        HANDLE_MSG(m_hwnd, WM_SIZE, DailyPanel_OnSize);
        HANDLE_MSG(m_hwnd, WM_COMMAND, DailyPanel_OnCommand);
        HANDLE_MSG(m_hwnd, WM_MEASUREITEM, DailyPanel_OnMeasureItem);
        HANDLE_MSG(m_hwnd, WM_DRAWITEM, ListBoxData_OnDrawItem);

    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}

