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
#include "Database/trade.h"
#include "MainWindow/MainWindow.h"
#include "TickerTotals.h"


HWND HWND_TICKERPANEL = NULL;

CTickerPanel TickerPanel;



// ========================================================================================
// Populate the ListBox with the total earnings from each Ticker.
// ========================================================================================
void TickerPanel_ShowTickerTotals()
{
    HWND hListBox = GetDlgItem(HWND_TICKERPANEL, IDC_TICKER_LISTBOX);
    HWND hCustomVScrollBar = GetDlgItem(HWND_TICKERPANEL, IDC_TICKER_CUSTOMVSCROLLBAR);


    // Prevent ListBox redrawing until all calculations are completed
    SendMessage(hListBox, WM_SETREDRAW, FALSE, 0);


    // Clear the current table
    ListBoxData_DestroyItemData(hListBox);


    // Calculate the amounts per Ticker Symbol
    std::unordered_map<std::wstring, double> mapTicker;
    mapTicker.clear();

    for (const auto& trade : trades) {
        if (trade->ticker_symbol == L"OPENBAL") continue;
        if (trade->is_open == true) continue;
        double total = mapTicker[trade->ticker_symbol] + trade->acb;
        mapTicker[trade->ticker_symbol] = total;
    }

    struct VecData {
        std::wstring ticker;
        double amount;
    };

    // create a empty vector of VecData
    std::vector<VecData> vec;

    // copy from the map to the vector
    for (const auto& [key, value] : mapTicker) {
        VecData data{ key, value };
        vec.push_back(data);
    }

    // sort the vector largest to smallest based on amount (using a lamda)
    std::sort(vec.begin(), vec.end(),
        [](VecData a, VecData b) {return (a.amount > b.amount); });


    for (const auto& i : vec) {
        ListBoxData_OutputTickerTotals(hListBox, i.ticker, i.amount);
    }

    ListBoxData_AddBlankLine(hListBox);


    // Calculate the actual column widths based on the size of the strings in
    // ListBoxData while respecting the minimum values as defined in nMinColWidth[].
    // This function is also called when receiving new price data from TWS because
    // that data may need the column width to be wider.
    ListBoxData_ResizeColumnWidths(hListBox, TableType::TickerTotals, -1);


    // Set the ListBox to the topline.
    ListBox_SetTopIndex(hListBox, 0);


    // Redraw the ListBox to ensure that any recalculated columns are 
    // displayed correctly. Re-enable redraw.
    SendMessage(hListBox, WM_SETREDRAW, TRUE, 0);
    AfxRedrawWindow(hListBox);


    // Ensure that the Ticker panel is set
    MainWindow_SetRightPanel(HWND_TICKERPANEL);

    CustomVScrollBar_Recalculate(hCustomVScrollBar);
}


// ========================================================================================
// Header control subclass Window procedure
// ========================================================================================
LRESULT CALLBACK TickerPanel_Header_SubclassProc(
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
        RemoveWindowSubclass(hWnd, TickerPanel_Header_SubclassProc, uIdSubclass);
        break;
    }


    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}


// ========================================================================================
// Listbox subclass Window procedure
// ========================================================================================
LRESULT CALLBACK TickerPanel_ListBox_SubclassProc(
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
        HWND hCustomVScrollBar = GetDlgItem(HWND_TICKERPANEL, IDC_TICKER_CUSTOMVSCROLLBAR);
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
        return true;
        break;
    }


    case WM_DESTROY:
    {
        // Destroy all manually allocated ListBox display data that is held
        // in the LineData structures..
        ListBoxData_DestroyItemData(hWnd);

        // REQUIRED: Remove control subclassing
        RemoveWindowSubclass(hWnd, TickerPanel_ListBox_SubclassProc, uIdSubclass);
        break;
    }


    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}


// ========================================================================================
// Process WM_MEASUREITEM message for window/dialog: TickerPanel
// ========================================================================================
void TickerPanel_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT* lpMeasureItem)
{
    lpMeasureItem->itemHeight = AfxScaleY(TICKER_LISTBOX_ROWHEIGHT);
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: TickerPanel
// ========================================================================================
BOOL TickerPanel_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    // Handle all of the painting in WM_PAINT
    return TRUE;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: TickerPanel
// ========================================================================================
void TickerPanel_OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    Graphics graphics(hdc);

    // Create the background brush
    SolidBrush back_brush(COLOR_GRAYDARK);

    // Paint the background using brush.
    int nWidth = (ps.rcPaint.right - ps.rcPaint.left);
    int nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Process WM_SIZE message for window/dialog: TickerPanel
// ========================================================================================
void TickerPanel_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    HWND hHeaderTickerTotals = GetDlgItem(hwnd, IDC_TICKER_HEADER_TOTALS);
    HWND hListBox = GetDlgItem(hwnd, IDC_TICKER_LISTBOX);
    HWND hCustomVScrollBar = GetDlgItem(hwnd, IDC_TICKER_CUSTOMVSCROLLBAR);

    int margin = AfxScaleY(TICKERTOTALS_MARGIN);

    HDWP hdwp = BeginDeferWindowPos(10);

    // Move and size the top label into place
    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_TICKER_SYMBOL), 0,
        0, 0, cx, margin, SWP_NOZORDER | SWP_SHOWWINDOW);

    // Do not call the calcVThumbRect() function during a scrollbar move. This WM_SIZE
    // gets triggered when the ListBox WM_DRAWITEM fires. If we do another calcVThumbRect()
    // calcualtion then the scrollbar will appear "jumpy" under the user's mouse cursor.
    bool show_scrollbar = false;
    CustomVScrollBar* pData = CustomVScrollBar_GetPointer(hCustomVScrollBar);
    if (pData != nullptr) {
        if (pData->drag_active) {
            show_scrollbar = true;
        }
        else {
            show_scrollbar = pData->calcVThumbRect();
        }
    }
    int custom_scrollbar_width = show_scrollbar ? AfxScaleX(CUSTOMVSCROLLBAR_WIDTH) : 0;

    int nTop = margin;
    int nLeft = 0;
    int nWidth = cx;
    int nHeight = AfxScaleY(TICKER_LISTBOX_ROWHEIGHT);

    hdwp = DeferWindowPos(hdwp, hHeaderTickerTotals, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);
    nTop = nTop + nHeight + AfxScaleX(1);

    nWidth = cx - custom_scrollbar_width;
    nHeight = cy - nTop;
    hdwp = DeferWindowPos(hdwp, hListBox, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nLeft = nLeft + nWidth;   // right edge of ListBox
    nWidth = custom_scrollbar_width;
    hdwp = DeferWindowPos(hdwp, hCustomVScrollBar, 0, nLeft, nTop, nWidth, nHeight,
        SWP_NOZORDER | (show_scrollbar ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));

    EndDeferWindowPos(hdwp);
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: TickerPanel
// ========================================================================================
BOOL TickerPanel_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    HWND_TICKERPANEL = hwnd;

    HWND hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TICKER_SYMBOL, L"Ticker Totals",
        COLOR_WHITELIGHT, COLOR_BLACK);

    // Create an Ownerdraw listbox that we will use to custom paint ticker names.
    hCtl =
        TickerPanel.AddControl(Controls::ListBox, hwnd, IDC_TICKER_LISTBOX, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWFIXED | LBS_NOTIFY,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)TickerPanel_ListBox_SubclassProc,
            IDC_TICKER_LISTBOX, NULL);
    ListBox_AddString(hCtl, NULL);

    // Create our custom vertical scrollbar and attach the ListBox to it.
    CreateCustomVScrollBar(hwnd, IDC_TICKER_CUSTOMVSCROLLBAR, hCtl);


    // Create Header control for our Ticker Totals output
    hCtl = TickerPanel.AddControl(Controls::Header, hwnd, IDC_TICKER_HEADER_TOTALS,
        L"", 0, 0, 0, 0, -1, -1, NULL, (SUBCLASSPROC)TickerPanel_Header_SubclassProc,
        IDC_TICKER_HEADER_TOTALS, NULL);
    int nWidth = AfxScaleX(50);
    Header_InsertNewItem(hCtl, 0, nWidth, L"", HDF_LEFT);
    Header_InsertNewItem(hCtl, 1, nWidth, L"Ticker", HDF_LEFT);
    Header_InsertNewItem(hCtl, 2, nWidth, L"Company Name", HDF_LEFT);
    Header_InsertNewItem(hCtl, 3, nWidth, L"Amount", HDF_RIGHT);

    return TRUE;
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CTickerPanel::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(m_hwnd, WM_CREATE, TickerPanel_OnCreate);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, TickerPanel_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, TickerPanel_OnPaint);
        HANDLE_MSG(m_hwnd, WM_SIZE, TickerPanel_OnSize);
        HANDLE_MSG(m_hwnd, WM_MEASUREITEM, TickerPanel_OnMeasureItem);
        HANDLE_MSG(m_hwnd, WM_DRAWITEM, ListBoxData_OnDrawItem);

    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}

