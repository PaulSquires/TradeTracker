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
#include "..\Utilities\ListBoxData.h"
#include "..\MainWindow\MainWindow.h"
#include "..\CustomVScrollBar\CustomVScrollBar.h"
#include "..\Database\database.h"

#include "ClosedPanel.h"


HWND HWND_CLOSEDPANEL = NULL;

extern std::vector<std::shared_ptr<Trade>> trades;

extern HWND HWND_MAINWINDOW;
extern HWND HWND_HISTORYPANEL;
extern HWND HWND_MENUPANEL;
extern HWND HWND_MIDDLEPANEL;

extern CClosedPanel ClosedPanel;

extern void MainWindow_SetMiddlePanel(HWND hPanel);
extern void HistoryPanel_ShowTradesHistoryTable(const std::shared_ptr<Trade>& trade);



// ========================================================================================
// Central function that actually selects and displays the incoming ListBox index item.
// ========================================================================================
void ClosedPanel_ShowListBoxItem(int index)
{
    HWND hListBox = GetDlgItem(HWND_CLOSEDPANEL, IDC_CLOSED_LISTBOX);
    HWND hCustomVScrollBar = GetDlgItem(HWND_CLOSEDPANEL, IDC_CLOSED_CUSTOMVSCROLLBAR);

    ListBox_SetCurSel(hListBox, index);

    //  update the scrollbar position if necessary
    CustomVScrollBar_Recalculate(hCustomVScrollBar);

    // Get the current line to determine if a valid Trade pointer exists so that we
    // can show the trade history.
    if (index > -1) {
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, index);
        if (ld != nullptr)
            HistoryPanel_ShowTradesHistoryTable(ld->trade);
    }

    SetFocus(hListBox);
}


// ========================================================================================
// Populate the ListBox with the closed trades
// ========================================================================================
void ClosedPanel_ShowClosedTrades()
{
    HWND hListBox = GetDlgItem(HWND_CLOSEDPANEL, IDC_CLOSED_LISTBOX);
    HWND hCustomVScrollBar = GetDlgItem(HWND_CLOSEDPANEL, IDC_CLOSED_CUSTOMVSCROLLBAR);
    HWND hLabel = GetDlgItem(HWND_CLOSEDPANEL, IDC_CLOSED_LABEL);

    // No need to redisplay Closed trades if they are already showing.
    if (HWND_MIDDLEPANEL == HWND_CLOSEDPANEL) return;


    // Prevent ListBox redrawing until all calculations are completed.
    SendMessage(hListBox, WM_SETREDRAW, FALSE, 0);


    struct ClosedData {
        std::wstring closedDate;
        std::shared_ptr<Trade> trade;
    };

    std::vector<ClosedData> vectorClosed;
    vectorClosed.reserve(1000);         // reserve space for 1000 closed trades

    for (auto& trade : trades) {
        if (!trade->isOpen) {
            ClosedData data;

            // Iterate the transactions to find the latest closed date
            for (auto& trans : trade->transactions) {
                if (trans->transDate > data.closedDate) {
                    data.closedDate = trans->transDate;
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
            return (data1.closedDate > data2.closedDate) ? true : false;
        });


    for (const auto& ClosedData : vectorClosed) {
        ListBoxData_OutputClosedPosition(hListBox, ClosedData.trade, ClosedData.closedDate);
    }


    // Calculate the actual column widths based on the size of the strings in
    // ListBoxData while respecting the minimum values as defined in nMinColWidth[].
    // This function is also called when receiving new price data from TWS because
    // that data may need the column width to be wider.
    ListBoxData_ResizeColumnWidths(hListBox, TableType::ClosedTrades, -1);


    // Set the label text indicated the type of trades being listed
    CustomLabel_SetText(hLabel, L"Closed Trades");


    // Redraw the ListBox to ensure that any recalculated columns are 
    // displayed correctly. Re-enable redraw.
    SendMessage(hListBox, WM_SETREDRAW, TRUE, 0);
    AfxRedrawWindow(hListBox);


    // If closed trades exist then select the first trade so that its history will show
    if (ListBox_GetCount(hListBox)) {
        ClosedPanel_ShowListBoxItem(0);
    }
    else {
        ListBoxData_AddBlankLine(hListBox);
    }

    // Ensure that the Closed panel is set
    MainWindow_SetMiddlePanel(HWND_CLOSEDPANEL);

    // Hide the Category control
    ShowWindow(GetDlgItem(HWND_MAINWINDOW, IDC_MAINWINDOW_CATEGORY), SW_HIDE);

    CustomVScrollBar_Recalculate(hCustomVScrollBar);

    ListBox_SetCurSel(hListBox, 0);
    SetFocus(hListBox);
}


// ========================================================================================
// Header control subclass Window procedure
// ========================================================================================
LRESULT CALLBACK ClosedPanel_Header_SubclassProc(
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
        RemoveWindowSubclass(hWnd, ClosedPanel_Header_SubclassProc, uIdSubclass);
        break;


    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);

}


// ========================================================================================
// Listbox subclass Window procedure
// ========================================================================================
LRESULT CALLBACK ClosedPanel_ListBox_SubclassProc(
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
        HWND hCustomVScrollBar = GetDlgItem(HWND_CLOSEDPANEL, IDC_CLOSED_CUSTOMVSCROLLBAR);
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
        if (HIWORD(idx) == -1) break;

        ClosedPanel_ShowListBoxItem(idx);
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
        RemoveWindowSubclass(hWnd, ClosedPanel_ListBox_SubclassProc, uIdSubclass);
        break;


    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);

}



// ========================================================================================
// Process WM_MEASUREITEM message for window/dialog: ClosedPanel
// ========================================================================================
void ClosedPanel_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT* lpMeasureItem)
{
    lpMeasureItem->itemHeight = AfxScaleY(CLOSED_TRADES_LISTBOX_ROWHEIGHT);
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: ClosedPanel
// ========================================================================================
BOOL ClosedPanel_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    // Handle all of the painting in WM_PAINT
    return TRUE;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: ClosedPanel
// ========================================================================================
void ClosedPanel_OnPaint(HWND hwnd)
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
// Process WM_SIZE message for window/dialog: ClosedPanel
// ========================================================================================
void ClosedPanel_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    HWND hHeader = GetDlgItem(hwnd, IDC_CLOSED_HEADER);
    HWND hListBox = GetDlgItem(hwnd, IDC_CLOSED_LISTBOX);
    HWND hCustomVScrollBar = GetDlgItem(hwnd, IDC_CLOSED_CUSTOMVSCROLLBAR);

    int margin = AfxScaleY(CLOSEDPANEL_MARGIN);

    HDWP hdwp = BeginDeferWindowPos(5);

    // Move and size the top label into place
    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_CLOSED_LABEL), 0,
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
    int nHeight = AfxScaleY(CLOSED_TRADES_LISTBOX_ROWHEIGHT);

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
// Process WM_CREATE message for window/dialog: ClosedPanel
// ========================================================================================
BOOL ClosedPanel_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    HWND_CLOSEDPANEL = hwnd;

    HWND hCtl = CustomLabel_SimpleLabel(hwnd, IDC_CLOSED_LABEL, L"Closed Trades",
        ThemeElement::WhiteLight, ThemeElement::Black);

    hCtl = ClosedPanel.AddControl(Controls::Header, hwnd, IDC_CLOSED_HEADER, L"",
        0, 0, 0, 0, -1, -1, NULL, (SUBCLASSPROC)ClosedPanel_Header_SubclassProc,
        IDC_CLOSED_HEADER, NULL);
    int nWidth = AfxScaleX(50);
    Header_InsertNewItem(hCtl, 0, nWidth, L"", HDF_CENTER);
    Header_InsertNewItem(hCtl, 1, nWidth, L"Date", HDF_LEFT);
    Header_InsertNewItem(hCtl, 2, nWidth, L"Ticker", HDF_LEFT);
    Header_InsertNewItem(hCtl, 3, nWidth, L"Company Name", HDF_LEFT);
    Header_InsertNewItem(hCtl, 4, nWidth, L"Amount", HDF_RIGHT);
    // Must turn off Window Theming for the control in order to correctly apply colors
    SetWindowTheme(hCtl, L"", L"");


    // Create an Ownerdraw fixed row sized listbox that we will use to custom
    // paint our various closed trades.
    hCtl =
        ClosedPanel.AddControl(Controls::ListBox, hwnd, IDC_CLOSED_LISTBOX, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWFIXED | LBS_NOTIFY,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)ClosedPanel_ListBox_SubclassProc,
            IDC_CLOSED_LISTBOX, NULL);
    ListBox_AddString(hCtl, NULL);


    // Create our custom vertical scrollbar and attach the ListBox to it.
    CreateCustomVScrollBar(hwnd, IDC_CLOSED_CUSTOMVSCROLLBAR, hCtl);

    return TRUE;
}


// ========================================================================================
// Process WM_COMMAND message for window/dialog: ClosedPanel
// ========================================================================================
void ClosedPanel_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (codeNotify)
    {

    case (LBN_SELCHANGE):
        int nCurSel = ListBox_GetCurSel(hwndCtl);
        if (nCurSel == -1) break;
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hwndCtl, nCurSel);
        if (ld != nullptr) {
            // Show the trade history for the selected trade
            ClosedPanel_ShowListBoxItem(nCurSel);
        }
        break;

    }
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CClosedPanel::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(m_hwnd, WM_CREATE, ClosedPanel_OnCreate);
        HANDLE_MSG(m_hwnd, WM_COMMAND, ClosedPanel_OnCommand);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, ClosedPanel_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, ClosedPanel_OnPaint);
        HANDLE_MSG(m_hwnd, WM_SIZE, ClosedPanel_OnSize);
        HANDLE_MSG(m_hwnd, WM_MEASUREITEM, ClosedPanel_OnMeasureItem);
        HANDLE_MSG(m_hwnd, WM_DRAWITEM, ListBoxData_OnDrawItem);

    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}

