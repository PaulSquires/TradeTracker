
#include "pch.h"
#include "..\SuperLabel\SuperLabel.h"
#include "..\Utilities\ListBoxData.h"
#include "..\Database\trade.h"
#include "..\Themes\Themes.h"
#include "..\VScrollBar\VScrollBar.h"
#include "..\MenuPanel\MenuPanel.h"

#include "TickerPanel.h"


HWND HWND_TICKERPANEL = NULL;

extern CTickerPanel TickerPanel;

extern std::vector<Trade*> trades;
extern int nColWidth[];

void TickerPanel_OnSize(HWND hwnd, UINT state, int cx, int cy);



// ========================================================================================
// Populate the ListBox with the total earnings from each Ticker.
// ========================================================================================
void TickerPanel_ShowTickerTotals()
{
    HWND hListBox = GetDlgItem(HWND_TICKERPANEL, IDC_TICKER_LISTBOX);
    HWND hVScrollBar = GetDlgItem(HWND_TICKERPANEL, IDC_TICKER_VSCROLLBAR);


    // Prevent ListBox redrawing until all calculations are completed
    SendMessage(hListBox, WM_SETREDRAW, FALSE, 0);


    // Clear the current table
    ListBoxData_DestroyItemData(hListBox);


    // Calculate the amounts per Ticker Symbol
    std::unordered_map<std::wstring, double> mapTicker;
    mapTicker.clear();

    for (const auto& trade : trades) {
        if (trade->tickerSymbol == L"OPENBAL") continue;
        double total = mapTicker[trade->tickerSymbol] + trade->ACB;
        mapTicker[trade->tickerSymbol] = total;
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

    ListBoxData_HistoryBlankLine(hListBox);


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


    // Need to force a resize of the HistoryPanel in order to properly show (or not show) 
    // and position the Summary listbox and daily detail listbox.
    RECT rc; GetClientRect(HWND_TICKERPANEL, &rc);
    TickerPanel_OnSize(HWND_TICKERPANEL, 0, rc.right, rc.bottom);


    VScrollBar_Recalculate(hVScrollBar);
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
        break;
    }


    case WM_DESTROY:

        // REQUIRED: Remove control subclassing
        RemoveWindowSubclass(hWnd, TickerPanel_Header_SubclassProc, uIdSubclass);
        break;


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
        HWND hVScrollBar = GetDlgItem(HWND_TICKERPANEL, IDC_TICKER_VSCROLLBAR);
        VScrollBar_Recalculate(hVScrollBar);
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
            SolidBrush backBrush(GetThemeColor(ThemeElement::TradesPanelBack));
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
        RemoveWindowSubclass(hWnd, TickerPanel_ListBox_SubclassProc, uIdSubclass);
        break;


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

    DWORD nBackColor = GetThemeColor(ThemeElement::TradesPanelBack);

    // Create the background brush
    SolidBrush backBrush(nBackColor);

    // Paint the background using brush.
    int nWidth = (ps.rcPaint.right - ps.rcPaint.left);
    int nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&backBrush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Process WM_SIZE message for window/dialog: TickerPanel
// ========================================================================================
void TickerPanel_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    HWND hHeaderTickerTotals = GetDlgItem(hwnd, IDC_TICKER_HEADER_TOTALS);
    HWND hListBox = GetDlgItem(hwnd, IDC_TICKER_LISTBOX);
    HWND hVScrollBar = GetDlgItem(hwnd, IDC_TICKER_VSCROLLBAR);

    int margin = AfxScaleY(TICKERPANEL_MARGIN);

    HDWP hdwp = BeginDeferWindowPos(10);

    // Move and size the top label into place
    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_TICKER_SYMBOL), 0,
        0, 0, cx, margin, SWP_NOZORDER | SWP_SHOWWINDOW);

    // Do not call the calcVThumbRect() function during a scrollbar move. This WM_SIZE
    // gets triggered when the ListBox WM_DRAWITEM fires. If we do another calcVThumbRect()
    // calcualtion then the scrollbar will appear "jumpy" under the user's mouse cursor.
    bool bShowScrollBar = false;
    VScrollBar* pData = VScrollBar_GetPointer(hVScrollBar);
    if (pData != nullptr) {
        if (pData->bDragActive) {
            bShowScrollBar = true;
        }
        else {
            bShowScrollBar = pData->calcVThumbRect();
        }
    }
    int VScrollBarWidth = bShowScrollBar ? AfxScaleX(VSCROLLBAR_WIDTH) : 0;

    int nTop = margin;
    int nLeft = 0;
    int nWidth = cx;
    int nHeight = AfxScaleY(TICKER_LISTBOX_ROWHEIGHT);

    hdwp = DeferWindowPos(hdwp, hHeaderTickerTotals, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);
    nTop = nTop + nHeight + AfxScaleX(1);

    nWidth = cx - VScrollBarWidth;
    nHeight = cy - nTop;
    hdwp = DeferWindowPos(hdwp, hListBox, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nLeft = nLeft + nWidth;   // right edge of ListBox
    nWidth = VScrollBarWidth;
    hdwp = DeferWindowPos(hdwp, hVScrollBar, 0, nLeft, nTop, nWidth, nHeight,
        SWP_NOZORDER | (bShowScrollBar ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));


    EndDeferWindowPos(hdwp);
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: TickerPanel
// ========================================================================================
BOOL TickerPanel_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    HWND_TICKERPANEL = hwnd;

    HWND hCtl = SuperLabel_SimpleLabel(hwnd, IDC_TICKER_SYMBOL, L"Ticker Totals",
        ThemeElement::MenuPanelText, ThemeElement::MenuPanelBack);

    // Create an Ownerdraw listbox that we will use to custom paint ticker names.
    hCtl =
        TickerPanel.AddControl(Controls::ListBox, hwnd, IDC_TICKER_LISTBOX, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_EXTENDEDSEL | LBS_MULTIPLESEL |
            LBS_OWNERDRAWFIXED | LBS_NOTIFY,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)TickerPanel_ListBox_SubclassProc,
            IDC_TICKER_LISTBOX, NULL);
    ListBox_AddString(hCtl, NULL);

    // Create our custom vertical scrollbar and attach the ListBox to it.
    CreateVScrollBar(hwnd, IDC_TICKER_VSCROLLBAR, hCtl);


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

