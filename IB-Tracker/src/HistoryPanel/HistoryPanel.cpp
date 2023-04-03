
#include "pch.h"
#include "..\Utilities\SuperLabel.h"
#include "..\Utilities\ListBoxData.h"
#include "..\Database\trade.h"
#include "..\Themes\Themes.h"
#include "HistoryPanel.h"


HWND HWND_HISTORYPANEL = NULL;

const int HISTORY_VSCROLLBAR_WIDTH = 14;
const int HISTORY_VSCROLLBAR_MINTHUMBSIZE = 20;

extern CHistoryPanel HistoryPanel;

extern std::vector<Trade*> trades;
extern int nColWidth[];


class VHistoryScrollBar
{
public:
    HWND hwnd = NULL;
    HWND hListBox = NULL;
    bool bDragActive = false;
    int listBoxHeight = 0;
    int itemHeight = 0;
    int numItems = 0;
    int itemsPerPage = 0;
    int thumbHeight = 0;
    RECT rc{};
};

VHistoryScrollBar vsb;



// ========================================================================================
// Calculate the RECT that holds the client coordinates of the scrollbar's vertical thumb
// Will return TRUE if RECT is not empty. 
// ========================================================================================
bool HistoryPanel_calcVThumbRect()
{
    // calculate the vertical scrollbar in client coordinates
    SetRectEmpty(&vsb.rc);
    int nTopIndex = SendMessage(vsb.hListBox, LB_GETTOPINDEX, 0, 0);

    RECT rc{};
    GetClientRect(vsb.hListBox, &rc);
    vsb.listBoxHeight = (rc.bottom - rc.top);
    vsb.itemHeight = ListBox_GetItemHeight(vsb.hListBox, 0);
    vsb.numItems = ListBox_GetCount(vsb.hListBox);

    // If no items exist then exit to avoid division by zero GPF's.
    if (vsb.numItems == 0) return FALSE;

    vsb.itemsPerPage = (int)(std::round(vsb.listBoxHeight / (float)vsb.itemHeight));
    vsb.thumbHeight = (int)(((float)vsb.itemsPerPage / (float)vsb.numItems) * (float)vsb.listBoxHeight);

    vsb.rc.left = rc.left;
    vsb.rc.top = (int)(rc.top + (((float)nTopIndex / (float)vsb.numItems) * (float)vsb.listBoxHeight));
    vsb.rc.right = rc.right;
    vsb.rc.bottom = (vsb.rc.top + vsb.thumbHeight);

    // If the number of items in the listbox is less than what could display
    // on the screen then there is no need to show the scrollbar.
    return (vsb.numItems < vsb.itemsPerPage) ? FALSE : TRUE;

}



// ========================================================================================
// Populate the History ListBox with the current active/open trades
// ========================================================================================
void HistoryPanel_ShowTradesHistoryTable(Trade* trade)
{
    HWND hListBox = GetDlgItem(HWND_HISTORYPANEL, IDC_HISTORY_LISTBOX);

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


    // Calculate the actual column widths based on the size of the strings in
    // ListBoxData while respecting the minimum values as defined in nMinColWidth[].
    // This function is also called when receiving new price data from TWS because
    // that data may need the column width to be wider.
    ListBoxData_ResizeColumnWidths(hListBox, -1);


    // Generate a WM_SIZE message to display and position the ListBox and vertical ScrollBar.
    RECT rc; GetWindowRect(HWND_HISTORYPANEL, &rc);
    SendMessage(HWND_HISTORYPANEL, WM_SIZE, SW_NORMAL, MAKELPARAM(rc.right - rc.left, rc.bottom - rc.top));

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
                StringFormat stringF(StringFormatFlagsNoWrap);
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
// Vertical scrollBar subclass Window procedure
// ========================================================================================
LRESULT CALLBACK HistoryPanel_VScrollBar_SubclassProc(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{

    static POINT prev_pt;             // screen pt.y cursor position

    switch (uMsg)
    {
    case WM_LBUTTONDOWN:
    {
        POINT pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        HistoryPanel_calcVThumbRect();
        if (PtInRect(&vsb.rc, pt)) {
            prev_pt = pt;
            vsb.bDragActive = true;
            SetCapture(hWnd);
        }
        else {
            // we have clicked on a PageUp or PageDn
            int nTopIndex = SendMessage(vsb.hListBox, LB_GETTOPINDEX, 0, 0);
            if (pt.y < vsb.rc.top) {
                nTopIndex = max(nTopIndex - vsb.itemsPerPage, 0);
                SendMessage(vsb.hListBox, LB_SETTOPINDEX, nTopIndex, 0);
                HistoryPanel_calcVThumbRect();
                AfxRedrawWindow(vsb.hwnd);
            }
            else {
                if (pt.y > vsb.rc.bottom) {
                    int nMaxTopIndex = vsb.numItems - vsb.itemsPerPage;
                    nTopIndex = min(nTopIndex + vsb.itemsPerPage, nMaxTopIndex);
                    SendMessage(vsb.hListBox, LB_SETTOPINDEX, nTopIndex, 0);
                    HistoryPanel_calcVThumbRect();
                    AfxRedrawWindow(vsb.hwnd);
                }
            }

        }
        break;
    }


    case WM_MOUSEMOVE:
    {
        if (vsb.bDragActive) {
            POINT pt;
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            if (pt.y != prev_pt.y) {
                int delta = (pt.y - prev_pt.y);

                RECT rc; GetClientRect(hWnd, &rc);
                vsb.rc.top = max(0, vsb.rc.top + delta);
                vsb.rc.top = min(vsb.rc.top, rc.bottom - vsb.thumbHeight);
                vsb.rc.bottom = vsb.rc.top + vsb.thumbHeight;

                prev_pt = pt;

                int nPrevTopLine = SendMessage(vsb.hListBox, LB_GETTOPINDEX, 0, 0);
                int nTopLine = (int)std::round(vsb.rc.top / (float)rc.bottom * vsb.numItems);
                if (nTopLine != nPrevTopLine)
                    SendMessage(vsb.hListBox, LB_SETTOPINDEX, (WPARAM)nTopLine, 0);

                AfxRedrawWindow(hWnd);
            }
        }
        break;
    }


    case WM_LBUTTONUP:
    {
        vsb.bDragActive = false;
        prev_pt.x = 0;
        prev_pt.y = 0;
        ReleaseCapture();
        break;
    }

    case WM_ERASEBKGND:
        return TRUE;
        break;


    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        SaveDC(hdc);

        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP hbit = CreateCompatibleBitmap(hdc, ps.rcPaint.right, ps.rcPaint.bottom);
        SelectBitmap(memDC, hbit);

        Graphics graphics(memDC);
        int nWidth = (ps.rcPaint.right - ps.rcPaint.left);
        int nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);

        SolidBrush backBrush(GetThemeColor(ThemeElement::TradesPanelScrollBarBack));
        graphics.FillRectangle(&backBrush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

        backBrush.SetColor(GetThemeColor(ThemeElement::TradesPanelScrollBarThumb));
        graphics.FillRectangle(&backBrush, vsb.rc.left, vsb.rc.top, nWidth, vsb.thumbHeight);

        Pen pen(GetThemeColor(ThemeElement::TradesPanelScrollBarLine), 1);
        graphics.DrawLine(&pen, (INT)ps.rcPaint.left, (INT)ps.rcPaint.top, (INT)ps.rcPaint.left, (INT)ps.rcPaint.bottom);

        // Copy the entire memory bitmap to the main display
        BitBlt(hdc, 0, 0, ps.rcPaint.right, ps.rcPaint.bottom, memDC, 0, 0, SRCCOPY);

        // Restore the original state of the DC
        RestoreDC(hdc, -1);

        // Cleanup
        DeleteObject(hbit);
        DeleteDC(memDC);

        EndPaint(hWnd, &ps);

        break;
    }



    case WM_DESTROY:
    {
        // REQUIRED: Remove control subclassing
        RemoveWindowSubclass(hWnd, HistoryPanel_VScrollBar_SubclassProc, uIdSubclass);
        break;
    }


    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);

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
        HistoryPanel_calcVThumbRect();
        AfxRedrawWindow(vsb.hwnd);
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
    int margin = AfxScaleY(HISTORY_LISTBOX_ROWHEIGHT);

    // Move and size the top label into place
    SetWindowPos(GetDlgItem(hwnd, IDC_HISTORY_SYMBOL), 0,
        0, 0, cx, margin, SWP_NOZORDER | SWP_SHOWWINDOW);

    // Do not call the calcVThumbRect() function during a scrollbar move. This WM_SIZE
    // gets triggered when the ListBox WM_DRAWITEM fires. If we do another calcVThumbRect()
    // calcualtion then the scrollbar will appear "jumpy" under the user's mouse cursor.
    bool bShowScrollBar = false;
    if (vsb.bDragActive) {
        bShowScrollBar = true;
    }
    else {
        bShowScrollBar = HistoryPanel_calcVThumbRect();
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
    vsb.hListBox =
        HistoryPanel.AddControl(Controls::ListBox, hwnd, IDC_HISTORY_LISTBOX, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_EXTENDEDSEL | LBS_MULTIPLESEL |
            LBS_OWNERDRAWFIXED | LBS_NOTIFY,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)HistoryPanel_ListBox_SubclassProc,
            IDC_HISTORY_LISTBOX, NULL);


    vsb.hwnd =
        HistoryPanel.AddControl(Controls::Custom, hwnd, IDC_HISTORY_VSCROLLBAR, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | SS_NOTIFY,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)HistoryPanel_VScrollBar_SubclassProc,
            IDC_HISTORY_VSCROLLBAR, NULL);

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

