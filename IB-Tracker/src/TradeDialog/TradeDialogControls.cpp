#include "pch.h"

#include "TradeDialog.h"
#include "..\VScrollBar\VScrollBar.h"
#include "..\Utilities\ListBoxData.h"


extern CTradeDialog TradeDialog;



// ====================================================================// ========================================================================================
// Listbox subclass Window procedure
// ========================================================================================
LRESULT CALLBACK TradeDialog_ListBox_SubclassProc(
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
        HWND hVScrollBar = GetDlgItem(TradeDialog.WindowHandle(), IDC_TRADEDIALOG_VSCROLLBAR);
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
        RemoveWindowSubclass(hWnd, TradeDialog_ListBox_SubclassProc, uIdSubclass);
        break;


    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);

}


// ========================================================================================
// Helper function for WM_SIZE message for window/dialog: TradeDialog
// ========================================================================================
void TradeDialogControls_SizeControls(HWND hwnd, int cx, int cy)
{
    HWND hListBox = GetDlgItem(hwnd, IDC_TRADEDIALOG_LISTBOX);
    HWND hVScrollBar = GetDlgItem(hwnd, IDC_TRADEDIALOG_VSCROLLBAR);
    HWND lblTransDate = GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLTRANSDATE);
    HWND lblTicker = GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLTICKER);
    HWND lblCompany = GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLCOMPANY);
    HWND hTransDate = GetDlgItem(hwnd, IDC_TRADEDIALOG_TRANSDATE);
    HWND txtTicker = GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTTICKER);
    HWND txtCompany = GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTCOMPANY);
    HWND txtDescription = GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTDESCRIPTION);


    int vmargin = AfxScaleY(6);
    int hmargin = AfxScaleX(10);

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

    int nTop = vmargin;
    int nLeft = 0;
    int nWidth = 0;
    int nHeight = cy - nTop;

    nWidth = AfxScaleX(200);
    SetWindowPos(hListBox, 0, nLeft, nTop, nWidth, nHeight,
        SWP_NOZORDER | SWP_SHOWWINDOW);

    nLeft = nLeft + nWidth;   // right edge of ListBox
    nWidth = VScrollBarWidth;
    SetWindowPos(hVScrollBar, 0, nLeft, nTop, nWidth, nHeight,
        SWP_NOZORDER | (bShowScrollBar ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));

    int nStartLeft = nLeft + nWidth + hmargin;


    nLeft = nStartLeft;
    nWidth = AfxScaleX(100);
    nHeight = AfxScaleY(18);
    SetWindowPos(lblTransDate, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nLeft = nLeft + nWidth + (hmargin * 2);
    nWidth = AfxScaleX(75);
    nHeight = AfxScaleY(18);
    SetWindowPos(lblTicker, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nLeft = nLeft + nWidth + hmargin;
    nWidth = AfxScaleX(300);
    nHeight = AfxScaleY(18);
    SetWindowPos(lblCompany, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nLeft = nStartLeft;
    nTop = nTop + nHeight;
    nWidth = AfxScaleX(100);
    nHeight = AfxScaleY(24);
    SetWindowPos(hTransDate, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nLeft = nLeft + nWidth + (hmargin * 2);
    nWidth = AfxScaleX(75);
    nHeight = AfxScaleY(24);
    SetWindowPos(txtTicker, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nLeft = nLeft + nWidth + hmargin;
    nWidth = AfxScaleX(200);
    nHeight = AfxScaleY(24);
    SetWindowPos(txtCompany, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nLeft = nStartLeft + AfxScaleX(100) + (hmargin * 2);
    nTop = nTop + nHeight + vmargin;
    nWidth = AfxScaleX(300);
    nHeight = AfxScaleY(24);
    SetWindowPos(txtDescription, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    VScrollBar_Recalculate(hVScrollBar);

}


// ========================================================================================
// Helper function for WM_CREATE message for window/dialog: TradeDialog
// ========================================================================================
void TradeDialogControls_CreateControls(HWND hwnd)
{
    HWND hCtl = NULL;

    // Create an Ownerdraw variable row sized listbox that we will use to custom
    // paint our various Trade Templates.
    HWND hListBox =
        TradeDialog.AddControl(Controls::ListBox, hwnd, IDC_TRADEDIALOG_LISTBOX, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWVARIABLE | LBS_NOTIFY,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)TradeDialog_ListBox_SubclassProc,
            IDC_TRADEDIALOG_LISTBOX, NULL);

    // Create our custom vertical scrollbar and attach the ListBox to it.
    HWND hVScrollBar = CreateVScrollBar(hwnd, IDC_TRADEDIALOG_LISTBOX, hListBox);

    // Add all of our Trade Templates to the ListBox
    ListBoxData_OutputTradesTemplates(hListBox);

    // Resize columns
    ListBoxData_ResizeColumnWidths(hListBox, TableType::TradeTemplates, -1);


    hCtl = TradeDialog.AddControl(Controls::Label, hwnd, IDC_TRADEDIALOG_LBLTRANSDATE, L"Transaction Date");
    hCtl = TradeDialog.AddControl(Controls::Label, hwnd, IDC_TRADEDIALOG_LBLTICKER, L"Ticker");
    hCtl = TradeDialog.AddControl(Controls::Label, hwnd, IDC_TRADEDIALOG_LBLCOMPANY, L"Company Name");

    hCtl = TradeDialog.AddControl(Controls::DateTimePicker, hwnd, IDC_TRADEDIALOG_TRANSDATE);
    DateTime_SetFormat(hCtl, L"yyyy-MM-dd");

    hCtl = TradeDialog.AddControl(Controls::TextBox, hwnd, IDC_TRADEDIALOG_TXTTICKER);
    hCtl = TradeDialog.AddControl(Controls::TextBox, hwnd, IDC_TRADEDIALOG_TXTCOMPANY);
    hCtl = TradeDialog.AddControl(Controls::TextBox, hwnd, IDC_TRADEDIALOG_TXTDESCRIPTION);


    hCtl = TradeDialog.AddControl(Controls::Button, hwnd, IDC_TRADEDIALOG_OK,
        L"OK", 400, 200, 74, 28);

    hCtl = TradeDialog.AddControl(Controls::Button, hwnd, IDC_TRADEDIALOG_CANCEL,
        L"Cancel", 480, 200, 74, 28);

}
