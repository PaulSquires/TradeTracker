
#include "pch.h"
#include "..\SuperLabel\SuperLabel.h"
#include "..\Utilities\ListBoxData.h"
#include "..\Database\trade.h"
#include "..\Themes\Themes.h"
#include "..\VScrollBar\VScrollBar.h"
#include "..\MenuPanel\MenuPanel.h"

#include "HistoryPanel.h"


HWND HWND_HISTORYPANEL = NULL;

const int HISTORY_VSCROLLBAR_WIDTH = 14;
const int HISTORY_VSCROLLBAR_MINTHUMBSIZE = 20;

extern CHistoryPanel HistoryPanel;

extern std::vector<Trade*> trades;
extern int nColWidth[];

extern HWND HWND_MENUPANEL;

void HistoryPanel_OnSize(HWND hwnd, UINT state, int cx, int cy);



// ========================================================================================
// Populate the History ListBox with the current active/open trades
// ========================================================================================
void HistoryPanel_ShowTradesHistoryTable(Trade* trade)
{
    if (trade == nullptr) return;

    HWND hListBox = GetDlgItem(HWND_HISTORYPANEL, IDC_HISTORY_LISTBOX);
    HWND hVScrollBar = GetDlgItem(HWND_HISTORYPANEL, IDC_HISTORY_VSCROLLBAR);

    // Resize the panel to ensure that the correct controls are shown and are
    // positioned correctly.
    RECT rc; GetClientRect(HWND_HISTORYPANEL, &rc);
    HistoryPanel_OnSize(HWND_HISTORYPANEL, 0, rc.right, rc.bottom);


    // Prevent ListBox redrawing until all calculations are completed
    SendMessage(hListBox, WM_SETREDRAW, FALSE, 0);


    // Clear the current trade history table
    ListBoxData_DestroyItemData(hListBox);


    SuperLabel_SetText(
        GetDlgItem(HWND_HISTORYPANEL, IDC_HISTORY_SYMBOL),
        trade->tickerSymbol + L": " + trade->tickerName);


    // Show the final rolled up Open position for this trade
    ListBoxData_OpenPosition(hListBox, trade, -1);
        

    // Read the transactions in reverse so that the newest history transactions get displayed first
    for (int i = trade->transactions.size() - 1; i >= 0; --i) {
        Transaction* trans = trade->transactions.at(i);

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
    ListBoxData_HistoryBlankLine(hListBox);


    // Calculate the actual column widths based on the size of the strings in
    // ListBoxData while respecting the minimum values as defined in nMinColWidth[].
    // This function is also called when receiving new price data from TWS because
    // that data may need the column width to be wider.
    ListBoxData_ResizeColumnWidths(hListBox, TableType::TradeHistory, -1);


    // Re-calculate scrollbar and show thumb if necessary
    VScrollBar* pData = VScrollBar_GetPointer(hVScrollBar);
    if (pData != nullptr) {
        pData->calcVThumbRect();
        AfxRedrawWindow(pData->hwnd);
    }

    // Redraw the ListBox to ensure that any recalculated columns are 
    // displayed correctly. Re-enable redraw.
    SendMessage(hListBox, WM_SETREDRAW, TRUE, 0);
    AfxRedrawWindow(hListBox);

}


// ========================================================================================
// Populate the ListBox with the total earnings from each Ticker.
// ========================================================================================
void HistoryPanel_ShowTickerTotals()
{
    HWND hListBox = GetDlgItem(HWND_HISTORYPANEL, IDC_HISTORY_LISTBOX);
    HWND hVScrollBar = GetDlgItem(HWND_HISTORYPANEL, IDC_HISTORY_VSCROLLBAR);

    // Resize the panel to ensure that the correct controls are shown and are
    // positioned correctly.
    RECT rc; GetClientRect(HWND_HISTORYPANEL, &rc);
    HistoryPanel_OnSize(HWND_HISTORYPANEL, 0, rc.right, rc.bottom);


    // Prevent ListBox redrawing until all calculations are completed
    SendMessage(hListBox, WM_SETREDRAW, FALSE, 0);


    // Clear the current trade history table
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


    SuperLabel_SetText(
        GetDlgItem(HWND_HISTORYPANEL, IDC_HISTORY_SYMBOL),
        L"Ticker Totals");


    // Re-calculate scrollbar and show thumb if necessary
    VScrollBar* pData = VScrollBar_GetPointer(hVScrollBar);
    if (pData != nullptr) {
        pData->calcVThumbRect();
        AfxRedrawWindow(pData->hwnd);
    }

    // Redraw the ListBox to ensure that any recalculated columns are 
    // displayed correctly. Re-enable redraw.
    SendMessage(hListBox, WM_SETREDRAW, TRUE, 0);
    AfxRedrawWindow(hListBox);

}


// ========================================================================================
// Populate the ListBox with the total earnings per day.
// ========================================================================================
void HistoryPanel_ShowDailyTotals(const ListBoxData* ld)
{
    HWND hListBox = GetDlgItem(HWND_HISTORYPANEL, IDC_HISTORY_LISTBOX);
    HWND hVScrollBar = GetDlgItem(HWND_HISTORYPANEL, IDC_HISTORY_VSCROLLBAR);

    // Resize the panel to ensure that the correct controls are shown and are
    // positioned correctly.
    RECT rc; GetClientRect(HWND_HISTORYPANEL, &rc);
    HistoryPanel_OnSize(HWND_HISTORYPANEL, 0, rc.right, rc.bottom);


    // Default to opening the current date
    std::wstring selectedDate = AfxCurrentDate();
    bool isOpen = true;

    if (ld != nullptr) {
        if (ld->isDailyTotalsNode) {
            selectedDate = ld->DailyTotalsDate;
            // If the node is already open then we close it, otherwise open it.
            isOpen = ld->isDailyTotalsNodeOpen ? false : true;
            if (!isOpen) selectedDate = L"";
        } else {
            return;
        }
    }

    // Prevent ListBox redrawing until all calculations are completed
    SendMessage(hListBox, WM_SETREDRAW, FALSE, 0);

    // Save the previously top line of the ListBox so that it can be restored
    // after the new states are determined.
    int nTopLine = ListBox_GetTopIndex(hListBox);


    // Clear the current trade history table
    ListBoxData_DestroyItemData(hListBox);


    // Calculate the daily amounts
    // Map contains a vector for every unique date.
    struct MapData {
        Trade* trade;
        Transaction* trans;
    };

    std::map< std::wstring, std::vector<MapData> > mapTotals;
    mapTotals.clear();

    int currentYear = AfxLocalYear();
    int currentMonth = AfxLocalMonth();
    double MTD = 0;
    double YTD = 0;

    for (const auto& trade : trades) {
        for (const auto& trans : trade->transactions) {
            MapData data{trade, trans};
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
            dayTotal+= data.trans->total;
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
        } else {
            ListBoxData_OutputDailyTotalsNodeHeader(hListBox, wszDate, dayTotal, false);
        }
        grandTotal += dayTotal;
    }

/*

    // Populate the summary data
    text = q.toCurrencyString(grandTotal, " ", 2);
    SetItemAttributes(parent, 0, text, Qt::AlignCenter, font, grandTotal >= 0 ? green : red, baseGrayBack);

    double stockValue = 0;
    for (const auto& trade : trades) {
        if (!trade->isOpen) { continue; }
        for (const auto& leg : trade->openLegs) {
            if (leg->underlying == L"SHARES") {
                stockValue = stockValue + (leg->openQuantity * trade->tickerLastPrice);
            }
        }
    }
    text = q.toCurrencyString(stockValue, " ", 2);
    SetItemAttributes(parent, 1, text, Qt::AlignCenter, font, stockValue >= 0 ? green : red, baseGrayBack);

    double netValue = grandTotal + stockValue;
    text = q.toCurrencyString(netValue, " ", 2);
    SetItemAttributes(parent, 2, text, Qt::AlignCenter, font, netValue >= 0 ? green : red, baseGrayBack);

    text = q.toCurrencyString(MTD, " ", 2);
    SetItemAttributes(parent, 3, text, Qt::AlignCenter, font, MTD >= 0 ? green : red, baseGrayBack);

    text = q.toCurrencyString(YTD, " ", 2);
    SetItemAttributes(parent, 4, text, Qt::AlignCenter, font, YTD >= 0 ? green : red, baseGrayBack);
*/


    // Calculate the actual column widths based on the size of the strings in
    // ListBoxData while respecting the minimum values as defined in nMinColWidth[].
    // This function is also called when receiving new price data from TWS because
    // that data may need the column width to be wider.
    ListBoxData_ResizeColumnWidths(hListBox, TableType::DailyTotals, -1);


    SuperLabel_SetText(
        GetDlgItem(HWND_HISTORYPANEL, IDC_HISTORY_SYMBOL),
        L"Daily Totals");


    // Set the ListBox to the previous topline.
    ListBox_SetTopIndex(hListBox, nTopLine);


    // Re-calculate scrollbar and show thumb if necessary
    VScrollBar* pData = VScrollBar_GetPointer(hVScrollBar);
    if (pData != nullptr) {
        pData->calcVThumbRect();
        AfxRedrawWindow(pData->hwnd);
    }

    // Redraw the ListBox to ensure that any recalculated columns are 
    // displayed correctly. Re-enable redraw.
    SendMessage(hListBox, WM_SETREDRAW, TRUE, 0);
    AfxRedrawWindow(hListBox);
}


// ========================================================================================
// Process WM_DRAWITEM message for window/dialog: HistoryPanel
// ========================================================================================
void HistoryPanel_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT* lpDrawItem)
{
    if (lpDrawItem->itemID == -1) return;

    if (lpDrawItem->itemAction == ODA_DRAWENTIRE ||
        lpDrawItem->itemAction == ODA_SELECT) {

        int linenum = lpDrawItem->itemID;
        int nWidth = (lpDrawItem->rcItem.right - lpDrawItem->rcItem.left);
        int nHeight = (lpDrawItem->rcItem.bottom - lpDrawItem->rcItem.top);

        bool bIsHot = false;

        SaveDC(lpDrawItem->hDC);

        HDC memDC = NULL;         // Double buffering
        HBITMAP hbit = NULL;      // Double buffering

        memDC = CreateCompatibleDC(lpDrawItem->hDC);
        hbit = CreateCompatibleBitmap(lpDrawItem->hDC, nWidth, nHeight);
        if (hbit) SelectObject(memDC, hbit);

        // We do not do hot tracking for the History table because we do not
        // need to visually show any selected lines.


        Graphics graphics(memDC);
        graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);


        // Set some defaults in case there is no valid ListBox line number
        std::wstring wszText;

        DWORD nBackColor = (bIsHot)
            ? GetThemeColor(ThemeElement::TradesPanelBackHot)
            : GetThemeColor(ThemeElement::TradesPanelBack);
        DWORD nTextColor = GetThemeColor(ThemeElement::TradesPanelText);

        std::wstring wszFontName = AfxGetDefaultFont();
        FontFamily   fontFamily(wszFontName.c_str());
        REAL fontSize = 10;
        int fontStyle = FontStyleRegular;

        StringAlignment alignment = StringAlignmentNear;

        // Paint the full width background using brush 
        SolidBrush backBrush(nBackColor);
        graphics.FillRectangle(&backBrush, 0, 0, nWidth, nHeight);

        // Get the current ListBox line data should a valid line exist
        // Paint the individual columns with the specific data.
        ListBoxData* ld = (ListBoxData*)(lpDrawItem->itemData);
        int nLeft = 0;

        // Draw each of the columns
        for (int i = 0; i < 8; i++) {
            if (ld == nullptr) break;

            wszText = ld->col[i].wszText;

            alignment = ld->col[i].alignment;
            nBackColor = (bIsHot)
                ? GetThemeColor(ThemeElement::TradesPanelBackHot)
                : GetThemeColor(ld->col[i].backTheme);
            nTextColor = GetThemeColor(ld->col[i].textTheme);
            fontSize = ld->col[i].fontSize;
            fontStyle = ld->col[i].fontStyle;

            int colWidth = AfxScaleX((float)nColWidth[i]);

            backBrush.SetColor(nBackColor);
            graphics.FillRectangle(&backBrush, nLeft, 0, colWidth, nHeight);

            Font         font(&fontFamily, fontSize, fontStyle, Unit::UnitPoint);
            SolidBrush   textBrush(nTextColor);
            StringFormat stringF(0);
            stringF.SetAlignment(alignment);
            stringF.SetLineAlignment(StringAlignmentCenter);

            RectF rcText((REAL)nLeft, (REAL)0, (REAL)colWidth, (REAL)nHeight);
            graphics.DrawString(wszText.c_str(), -1, &font, rcText, &stringF, &textBrush);

            nLeft += colWidth;
        }


        BitBlt(lpDrawItem->hDC, lpDrawItem->rcItem.left,
            lpDrawItem->rcItem.top, nWidth, nHeight, memDC, 0, 0, SRCCOPY);


        // Cleanup
        RestoreDC(lpDrawItem->hDC, -1);
        if (hbit) DeleteObject(hbit);
        if (memDC) DeleteDC(memDC);

    }
}


// ========================================================================================
// Listbox subclass Window procedure
// ========================================================================================
LRESULT CALLBACK HistoryPanel_ListBox_SubclassProc(
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
        HWND hListBox = GetDlgItem(HWND_HISTORYPANEL, IDC_HISTORY_VSCROLLBAR);
        VScrollBar* pData = VScrollBar_GetPointer(hListBox);
        if (pData != nullptr) {
            pData->calcVThumbRect();
            AfxRedrawWindow(pData->hwnd);
        }
        break;
    }


    case WM_LBUTTONUP:
    {
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
        RemoveWindowSubclass(hWnd, HistoryPanel_ListBox_SubclassProc, uIdSubclass);
        break;


    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);

}



// ========================================================================================
// Process WM_MEASUREITEM message for window/dialog: HistoryPanel
// ========================================================================================
void HistoryPanel_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT* lpMeasureItem)
{
    int menuId = MenuPanel_GetActiveMenuItem(HWND_MENUPANEL);
    switch (menuId)
    {
    case IDC_MENUPANEL_TICKERTOTALS:
    case IDC_MENUPANEL_DAILYTOTALS:
        lpMeasureItem->itemHeight = AfxScaleY(TICKER_TOTALS_LISTBOX_ROWHEIGHT);
        break;

    default:
        lpMeasureItem->itemHeight = AfxScaleY(HISTORY_LISTBOX_ROWHEIGHT);
    }
}


// ========================================================================================
// Process WM_DESTROY message for window/dialog: HistoryPanel
// ========================================================================================
void HistoryPanel_OnDestroy(HWND hwnd)
{
    // TODO: Add your message processing code here...
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: HistoryPanel
// ========================================================================================
BOOL HistoryPanel_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    // Handle all of the painting in WM_PAINT
    return TRUE;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: HistoryPanel
// ========================================================================================
void HistoryPanel_OnPaint(HWND hwnd)
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
// Process WM_SIZE message for window/dialog: HistoryPanel
// ========================================================================================
void HistoryPanel_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    HWND hListBox = GetDlgItem(HWND_HISTORYPANEL, IDC_HISTORY_VSCROLLBAR);
    VScrollBar* pData = VScrollBar_GetPointer(hListBox);

    int margin = AfxScaleY(HISTORY_LISTBOX_ROWHEIGHT);

    // Move and size the top label into place
    SetWindowPos(GetDlgItem(hwnd, IDC_HISTORY_SYMBOL), 0,
        0, 0, cx, margin, SWP_NOZORDER | SWP_SHOWWINDOW);

    // Do not call the calcVThumbRect() function during a scrollbar move. This WM_SIZE
    // gets triggered when the ListBox WM_DRAWITEM fires. If we do another calcVThumbRect()
    // calcualtion then the scrollbar will appear "jumpy" under the user's mouse cursor.
    bool bShowScrollBar = false;
    if (pData != nullptr) {
        if (pData->bDragActive) {
            bShowScrollBar = true;
        }
        else {
            bShowScrollBar = pData->calcVThumbRect();
        }
    }
    int VScrollBarWidth = bShowScrollBar ? AfxScaleX(HISTORY_VSCROLLBAR_WIDTH) : 0;

    int nTop = margin;
    int nLeft = 0;
    int nWidth = cx - VScrollBarWidth;
    int nHeight = 0;

    int menuId = MenuPanel_GetActiveMenuItem(HWND_MENUPANEL);
    if (menuId == IDC_MENUPANEL_DAILYTOTALS) {
        nHeight = (HISTORY_LISTBOX_ROWHEIGHT * 2);
        SetWindowPos(GetDlgItem(hwnd, IDC_HISTORY_LISTBOX_SUMMARY), 0,
            nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);
        nTop = AfxScaleY(120);
    } else {
        ShowWindow(GetDlgItem(hwnd, IDC_HISTORY_LISTBOX_SUMMARY), SW_HIDE);
    }

    
    nHeight = cy - margin;
    SetWindowPos(GetDlgItem(hwnd, IDC_HISTORY_LISTBOX), 0,
        nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nLeft = nLeft + nWidth;   // right edge of ListBox
    nWidth = VScrollBarWidth;
    SetWindowPos(GetDlgItem(hwnd, IDC_HISTORY_VSCROLLBAR), 0,
        nLeft, nTop, nWidth, nHeight,
        SWP_NOZORDER | (bShowScrollBar ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: HistoryPanel
// ========================================================================================
BOOL HistoryPanel_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    HWND_HISTORYPANEL = hwnd;

    SuperLabel* pData = nullptr;

    HWND hCtl = CreateSuperLabel(
        hwnd,
        IDC_HISTORY_SYMBOL,
        SuperLabelType::TextOnly,
        0, 0, 0, 0);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = false;
        pData->BackColor = ThemeElement::MenuPanelBack;
        pData->TextColor = ThemeElement::MenuPanelText;
        pData->FontSize = 8;
        pData->TextAlignment = SuperLabelAlignment::MiddleLeft;
        pData->wszText = L"Trade History";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }


    // Create an Ownerdraw variable row sized listbox that we will use to custom
    // paint our various open trades.
    hCtl =
        HistoryPanel.AddControl(Controls::ListBox, hwnd, IDC_HISTORY_LISTBOX, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_EXTENDEDSEL | LBS_MULTIPLESEL |
            LBS_OWNERDRAWVARIABLE | LBS_NOTIFY,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)HistoryPanel_ListBox_SubclassProc,
            IDC_HISTORY_LISTBOX, NULL);
    ListBox_AddString(hCtl, NULL);

    // Create our custom vertical scrollbar and attach the ListBox to it.
    CreateVScrollBar(hwnd, IDC_HISTORY_VSCROLLBAR, hCtl);


    // Create an Ownerdraw variable row sized listbox that we will use to custom
    // paint the Daily History Summary.
    hCtl =
        HistoryPanel.AddControl(Controls::ListBox, hwnd, IDC_HISTORY_LISTBOX_SUMMARY, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_EXTENDEDSEL | LBS_MULTIPLESEL |
            LBS_OWNERDRAWVARIABLE | LBS_NOTIFY,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)HistoryPanel_ListBox_SubclassProc,
            IDC_HISTORY_LISTBOX, NULL);
    ListBox_AddString(hCtl, NULL);

    return TRUE;
}


// ========================================================================================
// Process WM_COMMAND message for window/dialog: HistoryPanel
// ========================================================================================
void HistoryPanel_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    if (id == IDC_HISTORY_LISTBOX && codeNotify == LBN_SELCHANGE) {
        if (MenuPanel_GetActiveMenuItem(HWND_MENUPANEL) == IDC_MENUPANEL_DAILYTOTALS) {
            // The date will be valid if we are "opening" the node, or simply null if
            // we will just show all closed nodes.
            int idx = ListBox_GetCurSel(hwndCtl);
            ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hwndCtl, idx);
            if (ld != nullptr) {
                HistoryPanel_ShowDailyTotals(ld);
            }
        }
    }
}



// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CHistoryPanel::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(m_hwnd, WM_CREATE, HistoryPanel_OnCreate);
        HANDLE_MSG(m_hwnd, WM_DESTROY, HistoryPanel_OnDestroy);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, HistoryPanel_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, HistoryPanel_OnPaint);
        HANDLE_MSG(m_hwnd, WM_SIZE, HistoryPanel_OnSize);
        HANDLE_MSG(m_hwnd, WM_DRAWITEM, HistoryPanel_OnDrawItem);
        HANDLE_MSG(m_hwnd, WM_MEASUREITEM, HistoryPanel_OnMeasureItem);
        HANDLE_MSG(m_hwnd, WM_COMMAND, HistoryPanel_OnCommand);

    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}

