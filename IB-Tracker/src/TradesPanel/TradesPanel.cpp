
#include "pch.h"
#include "..\MainWindow\tws-client.h"
#include "..\Database\trade.h"
#include "..\SuperLabel\SuperLabel.h"
#include "..\Utilities\ListBoxData.h"
#include "..\Themes\Themes.h"
#include "..\MenuPanel\MenuPanel.h"
#include "..\VScrollBar\VScrollBar.h"
#include "TradesPanel.h"

HWND HWND_TRADESPANEL = NULL;

const int VSCROLLBAR_WIDTH = 14;
const int VSCROLLBAR_MINTHUMBSIZE = 20;

extern CTradesPanel TradesPanel;

extern std::vector<Trade*> trades;

extern void HistoryPanel_ShowTradesHistoryTable(Trade* trade);

extern HWND HWND_MENUPANEL;



// ========================================================================================
// Populate the Trades ListBox with the current active/open trades
// ========================================================================================
void TradesPanel_ShowActiveTrades()
{
    HWND hListBox = GetDlgItem(HWND_TRADESPANEL, IDC_TRADES_LISTBOX);
    HWND hVScrollBar = GetDlgItem(HWND_TRADESPANEL, IDC_TRADES_VSCROLLBAR);
    HWND hLabel = GetDlgItem(HWND_TRADESPANEL, IDC_TRADES_LABEL);

    tws_PauseTWS();

    // Prevent ListBox redrawing until all calculations are completed
    SendMessage(hListBox, WM_SETREDRAW, FALSE, 0);


    // TODO: optimize this by sorting after initial database load and after
    // newly added/deleted data, rather than here every time we display the
    // active trades.
    // Sort the trades vector based on ticker symbol
    std::sort(trades.begin(), trades.end(),
        [](const Trade* trade1, const Trade* trade2) {
            return (trade1->tickerSymbol < trade2->tickerSymbol) ? true : false;
        });


    // Destroy any existing ListBox line data
    // This will also clear the LineData pointers and cancel any previous market data
    ListBoxData_DestroyItemData(hListBox);


    // Create the new ListBox line data and initiate the new market data.
    int tickerId = 100;
    for (const auto& trade : trades) {
        // We are displaying only all open trades
        if (trade->isOpen) {
            ListBoxData_OpenPosition(hListBox, trade, tickerId);
        }
        tickerId++;
    }


    // Calculate the actual column widths based on the size of the strings in
    // ListBoxData while respecting the minimum values as defined in nMinColWidth[].
    // This function is also called when receiving new price data from TWS because
    // that data may need the column width to be wider.
    ListBoxData_ResizeColumnWidths(hListBox, TableType::ActiveTrades, -1);

    
    // Select the correct menu panel item
    MenuPanel_SelectMenuItem(HWND_MENUPANEL, IDC_MENUPANEL_ACTIVETRADES);

    
    // Set the label text indicated the type of trades being listed
    SuperLabel_SetText(hLabel, L"Active Trades");


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


    // If trades exist then select the first trade so that its history will show
    if (ListBox_GetCount(hListBox)) {
        ListBox_SetCurSel(hListBox, 0);
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, 0);
        if (ld != nullptr) {
            HistoryPanel_ShowTradesHistoryTable(ld->trade);
        }
    }

    tws_ResumeTWS();
}


// ========================================================================================
// Populate the ListBox with the closed trades
// ========================================================================================
void TradesPanel_ShowClosedTrades()
{
    HWND hListBox = GetDlgItem(HWND_TRADESPANEL, IDC_TRADES_LISTBOX);
    HWND hVScrollBar = GetDlgItem(HWND_TRADESPANEL, IDC_TRADES_VSCROLLBAR);
    HWND hLabel = GetDlgItem(HWND_TRADESPANEL, IDC_TRADES_LABEL);

    tws_PauseTWS();

    // Prevent ListBox redrawing until all calculations are completed
    SendMessage(hListBox, WM_SETREDRAW, FALSE, 0);


    struct ClosedData {
        std::wstring closedDate;
        Trade* trade = nullptr;
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


    // Select the correct menu panel item
    MenuPanel_SelectMenuItem(HWND_MENUPANEL, IDC_MENUPANEL_CLOSEDTRADES);

    
    // Set the label text indicated the type of trades being listed
    SuperLabel_SetText(hLabel, L"Closed Trades");


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


    // If closed trades exist then select the first trade so that its history will show
    if (ListBox_GetCount(hListBox)) {
        ListBox_SetCurSel(hListBox, 0);
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, 0);
        if (ld != nullptr) {
            HistoryPanel_ShowTradesHistoryTable(ld->trade);
        }
    }

    SetFocus(hListBox);
    AfxRedrawWindow(hListBox);

    tws_ResumeTWS();

}


// ========================================================================================
// Process WM_DRAWITEM message for window/dialog: TradesPanel
// ========================================================================================
void TradesPanel_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT* lpDrawItem)
{
    if (lpDrawItem->itemID == -1) return;

    if (lpDrawItem->itemAction == ODA_DRAWENTIRE ||
        lpDrawItem->itemAction == ODA_SELECT) {

        int nWidth = (lpDrawItem->rcItem.right - lpDrawItem->rcItem.left);
        int nHeight = (lpDrawItem->rcItem.bottom - lpDrawItem->rcItem.top);

        bool bIsHot = false;

        SaveDC(lpDrawItem->hDC);

        HDC memDC = NULL;         // Double buffering
        HBITMAP hbit = NULL;      // Double buffering

        memDC = CreateCompatibleDC(lpDrawItem->hDC);
        hbit = CreateCompatibleBitmap(lpDrawItem->hDC, nWidth, nHeight);
        if (hbit) SelectObject(memDC, hbit);

        if ((lpDrawItem->itemAction | ODA_SELECT) &&
            (lpDrawItem->itemState & ODS_SELECTED)) {
            bIsHot = true;
        }

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

            int colWidth = AfxScaleX((float)ld->col[i].colWidth);

            backBrush.SetColor(nBackColor);
            graphics.FillRectangle(&backBrush, nLeft, 0, colWidth, nHeight);

            Font         font(&fontFamily, fontSize, fontStyle, Unit::UnitPoint);
            SolidBrush   textBrush(nTextColor);
            StringFormat stringF(StringFormatFlagsNoWrap);
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
LRESULT CALLBACK TradesPanel_ListBox_SubclassProc(
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
        HWND hListBox = GetDlgItem(HWND_TRADESPANEL, IDC_TRADES_VSCROLLBAR);
        VScrollBar* pData = VScrollBar_GetPointer(hListBox);
        if (pData != nullptr) {
            pData->calcVThumbRect();
            AfxRedrawWindow(pData->hwnd);
        }
        break;
    }

    case WM_MOUSEMOVE:
        break;

    case WM_MOUSEHOVER:
        break;

    case WM_MOUSELEAVE:
        break;


    case WM_RBUTTONDOWN:
    {
        // Create the popup menu
        int idx = Listbox_ItemFromPoint(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        // The return value contains the index of the nearest item in the LOWORD. The HIWORD is zero 
        // if the specified point is in the client area of the list box, or one if it is outside the 
        // client area.
        if (HIWORD(idx) != 1) {
            //ListBox_SetSel(hWnd, TRUE, idx);
            //dim as HMENU hPopupMenu = CreateExplorerContextMenu(pDoc)
            //dim as POINT pt : GetCursorPos(@pt)
            //dim as long id = TrackPopupMenu(hPopUpMenu, TPM_RETURNCMD, pt.x, pt.y, 0, HWND_FRMMAIN, byval null)
            //AfxRedrawWindow(hWnd);
        }
        break;
    }


    case WM_LBUTTONUP:
// Prevent this programmatic selection if Ctrl or Shift is active
// because we want the listbox to select the listbox item rather for
// us to mess with that selection via SetTabIndexByDocumentPtr().
//dim as boolean isCtrl = (GetAsyncKeyState(VK_CONTROL) and &H8000)
//dim as boolean isShift = (GetAsyncKeyState(VK_SHIFT) and &H8000)
//if (isCtrl = false) andalso(isShift = false) then
// determine if we clicked on a regular file or a node header
//dim as RECT rc
//dim as long idx = Listbox_ItemFromPoint(hWin, GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam))
// The return value contains the index of the nearest item in the LOWORD. The HIWORD is zero 
// if the specified point is in the client area of the list box, or one if it is outside the 
// client area.
//if hiword(idx) < > 1 then
//dim as CWSTR wszCaption = AfxGetListBoxText(hWin, idx)
//if left(wszCaption, 1) = "%" then
// Toggle the show/hide of files under this node
//dim as long idxArray = getExplorerNodeHeaderIndex(wszCaption)
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
        RemoveWindowSubclass(hWnd, TradesPanel_ListBox_SubclassProc, uIdSubclass);
        break;


    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);

}



// ========================================================================================
// Process WM_MEASUREITEM message for window/dialog: TradesPanel
// ========================================================================================
void TradesPanel_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT* lpMeasureItem)
{
    // The ListBox was created using LBS_OWNERDRAWVARIABLE so that this OnMeasureItem
    // would get called for every item being drawn. This allows us to change the item
    // height depending on what type of table data is being displayed.

    int menuId = MenuPanel_GetActiveMenuItem(HWND_MENUPANEL);
    switch (menuId)
    {
    case IDC_MENUPANEL_ACTIVETRADES:
        lpMeasureItem->itemHeight = AfxScaleY(ACTIVE_TRADES_LISTBOX_ROWHEIGHT);
        break;
    case IDC_MENUPANEL_CLOSEDTRADES:
        lpMeasureItem->itemHeight = AfxScaleY(CLOSED_TRADES_LISTBOX_ROWHEIGHT);
        break;

    default:
        lpMeasureItem->itemHeight = AfxScaleY(ACTIVE_TRADES_LISTBOX_ROWHEIGHT);
    }
}


// ========================================================================================
// Process WM_DESTROY message for window/dialog: TradesPanel
// ========================================================================================
void TradesPanel_OnDestroy(HWND hwnd)
{
    // TODO: Add your message processing code here...
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: TradesPanel
// ========================================================================================
BOOL TradesPanel_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    // Handle all of the painting in WM_PAINT
    return TRUE;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: TradesPanel
// ========================================================================================
void TradesPanel_OnPaint(HWND hwnd)
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
// Process WM_SIZE message for window/dialog: TradesPanel
// ========================================================================================
void TradesPanel_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    HWND hListBox = GetDlgItem(HWND_TRADESPANEL, IDC_TRADES_VSCROLLBAR);
    VScrollBar* pData = VScrollBar_GetPointer(hListBox);
        
    int margin = AfxScaleY(ACTIVE_TRADES_LISTBOX_ROWHEIGHT);

    // Move and size the top label into place
    SetWindowPos(GetDlgItem(hwnd, IDC_TRADES_LABEL), 0,
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
    int VScrollBarWidth = bShowScrollBar ? AfxScaleX(VSCROLLBAR_WIDTH) : 0;

    int nLeft = 0;
    int nTop = margin;
    int nWidth = cx - VScrollBarWidth;
    int nHeight = cy - margin;
    SetWindowPos(GetDlgItem(hwnd, IDC_TRADES_LISTBOX), 0,
        nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nLeft = nLeft + nWidth;   // right edge of ListBox
    nWidth = VScrollBarWidth;
    SetWindowPos(GetDlgItem(hwnd, IDC_TRADES_VSCROLLBAR), 0,
        nLeft, nTop, nWidth, nHeight,
        SWP_NOZORDER | (bShowScrollBar ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: TradesPanel
// ========================================================================================
BOOL TradesPanel_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    HWND_TRADESPANEL = hwnd;
        
    SuperLabel* pData = nullptr;

    HWND hCtl = CreateSuperLabel(
        hwnd,
        IDC_TRADES_LABEL,
        SuperLabelType::TextOnly,
        0, 0, 0, 0);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = false;
        pData->BackColor = ThemeElement::MenuPanelBack;
        pData->TextColor = ThemeElement::MenuPanelText;
        pData->FontSize = 8;
        pData->TextAlignment = SuperLabelAlignment::MiddleLeft;
        pData->wszText = L"Active Trades";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }


    // Create an Ownerdraw fixed row sized listbox that we will use to custom
    // paint our various open trades.
    hCtl = 
        TradesPanel.AddControl(Controls::ListBox, hwnd, IDC_TRADES_LISTBOX, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_EXTENDEDSEL | LBS_MULTIPLESEL |
            LBS_OWNERDRAWVARIABLE | LBS_NOTIFY,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)TradesPanel_ListBox_SubclassProc,
            IDC_TRADES_LISTBOX, NULL);
    ListBox_AddString(hCtl, NULL);


    // Create our custom vertical scrollbar and attach the ListBox to it.
    CreateVScrollBar(hwnd, IDC_TRADES_VSCROLLBAR, hCtl);

    return TRUE;
}


// ========================================================================================
// Process WM_COMMAND message for window/dialog: TradesPanel
// ========================================================================================
void TradesPanel_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    if (id == IDC_TRADES_LISTBOX && codeNotify == LBN_SELCHANGE) {
        HWND hListBox = GetDlgItem(HWND_TRADESPANEL, IDC_TRADES_VSCROLLBAR);
        VScrollBar* pData = VScrollBar_GetPointer(hListBox);
        // update the highlighting of the current line
        // TODO: Handle the repaint in the subclass (WM_LBUTTONDOWN) rather than this notification because
        // of the delay in repainting the switched lines.
        AfxRedrawWindow(hwndCtl);
        //  update the scrollbar position if necessary
        if (pData != nullptr) {
            pData->calcVThumbRect();
            AfxRedrawWindow(pData->hwnd);
        }
        // Get the current line to determine if a valid Trade pointer exists so that we
        // can show the trade history.
        int nIndex = ListBox_GetCurSel(hwndCtl);
        if (nIndex > -1) {
            ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hwndCtl, nIndex);
            if (ld != nullptr)
                HistoryPanel_ShowTradesHistoryTable(ld->trade);
        }
    }
}



// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CTradesPanel::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(m_hwnd, WM_CREATE, TradesPanel_OnCreate);
        HANDLE_MSG(m_hwnd, WM_DESTROY, TradesPanel_OnDestroy);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, TradesPanel_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, TradesPanel_OnPaint);
        HANDLE_MSG(m_hwnd, WM_SIZE, TradesPanel_OnSize);
        HANDLE_MSG(m_hwnd, WM_DRAWITEM, TradesPanel_OnDrawItem);
        HANDLE_MSG(m_hwnd, WM_MEASUREITEM, TradesPanel_OnMeasureItem);
        HANDLE_MSG(m_hwnd, WM_COMMAND, TradesPanel_OnCommand);

    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}

