
#include "pch.h"
#include "..\SuperLabel\SuperLabel.h"
#include "..\Utilities\ListBoxData.h"
#include "..\Database\trade.h"
#include "..\Themes\Themes.h"
#include "..\VScrollBar\VScrollBar.h"
#include "HistoryPanel.h"


HWND HWND_HISTORYPANEL = NULL;

const int HISTORY_VSCROLLBAR_WIDTH = 14;
const int HISTORY_VSCROLLBAR_MINTHUMBSIZE = 20;

extern CHistoryPanel HistoryPanel;

extern std::vector<Trade*> trades;
extern int nColWidth[];


// ========================================================================================
// Populate the History ListBox with the current active/open trades
// ========================================================================================
void HistoryPanel_ShowTradesHistoryTable(Trade* trade)
{
    if (trade == nullptr) return;

    HWND hListBox = GetDlgItem(HWND_HISTORYPANEL, IDC_HISTORY_LISTBOX);
    HWND hVScrollBar = GetDlgItem(HWND_HISTORYPANEL, IDC_HISTORY_VSCROLLBAR);

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
    ListBoxData_ResizeColumnWidths(hListBox, -1);


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
    if (lpDrawItem == nullptr) return;

    if (lpDrawItem->itemAction == ODA_DRAWENTIRE ||
        lpDrawItem->itemAction == ODA_SELECT ||
        lpDrawItem->itemAction == ODA_FOCUS) {

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

        if (ListBox_GetSel(lpDrawItem->hwndItem, lpDrawItem->itemID)) bIsHot = true;


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
        if (linenum != -1) {
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
            }  // for

        }  // if

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
    lpMeasureItem->itemHeight = AfxScaleY(HISTORY_LISTBOX_ROWHEIGHT);
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

    int nLeft = 0;
    int nTop = margin;
    int nWidth = cx - VScrollBarWidth;
    int nHeight = cy - margin;
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


    // Create an Ownerdraw fixed row sized listbox that we will use to custom
    // paint our various open trades.
    hCtl =
        HistoryPanel.AddControl(Controls::ListBox, hwnd, IDC_HISTORY_LISTBOX, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_EXTENDEDSEL | LBS_MULTIPLESEL |
            LBS_OWNERDRAWFIXED | LBS_NOTIFY,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)HistoryPanel_ListBox_SubclassProc,
            IDC_HISTORY_LISTBOX, NULL);
    ListBox_AddString(hCtl, NULL);

    // Create our custom vertical scrollbar and attach the ListBox to it.
    CreateVScrollBar(hwnd, IDC_HISTORY_VSCROLLBAR, hCtl);

    return TRUE;
}


// ========================================================================================
// Process WM_COMMAND message for window/dialog: HistoryPanel
// ========================================================================================
void HistoryPanel_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    if (id == IDC_HISTORY_LISTBOX && codeNotify == LBN_SELCHANGE) {
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

