#include "pch.h"

#include "TradeDialog.h"
#include "..\VScrollBar\VScrollBar.h"
#include "..\Utilities\ListBoxData.h"
#include "..\SuperLabel\SuperLabel.h"


struct LineCtrl {
    HWND cols[6]{ NULL };
};

std::vector<LineCtrl> lCtrls;

extern CTradeDialog TradeDialog;



// ========================================================================================
// Helper function to format certain TextBoxes to 4 decimal places
// ========================================================================================
void FormatNumberFourDecimals(HWND hCtl)
{
    static std::wstring DecimalSep = L".";
    static std::wstring ThousandSep = L",";

    static NUMBERFMTW num{};
    num.NumDigits = 4;
    num.LeadingZero = true;
    num.Grouping = 0;
    num.lpDecimalSep = (LPWSTR)DecimalSep.c_str();
    num.lpThousandSep = (LPWSTR)ThousandSep.c_str();
    num.NegativeOrder = 1;   // Negative sign, number; for example, -1.1

    std::wstring money = AfxGetWindowText(hCtl);

    std::wstring buffer(256, 0);
    int j = GetNumberFormatEx(LOCALE_NAME_USER_DEFAULT, 0, money.c_str(), &num, (LPWSTR)buffer.c_str(), 256);

    money = buffer.substr(0, j - 1);
    AfxSetWindowText(hCtl, money.c_str());
}



// ========================================================================================
// Helper function to calculate and update the Total TextBox
// ========================================================================================
void CalculateTradeTotal(HWND hwnd)
{
    double total = 0;
    double quantity = stod(AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTQUANTITY)));
    double price = stod(AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTPRICE)));
    double fees = stod(AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTFEES)));

    std::wstring DRCR = AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_COMBODRCR));
    if (DRCR == L"CREDIT") fees = fees * -1;

    total = (quantity * 100 * price) + fees;
    AfxSetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTTOTAL), std::to_wstring(total));
    FormatNumberFourDecimals(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTTOTAL));
}


// ========================================================================================
// Calculate Days To Expiration (DTE) and display it in the table
// ========================================================================================
void CalculateTradeDTE(HWND hwnd)
{
    std::wstring transDate = AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TRANSDATE));

    for (int i = 0; i < TRADEDIALOG_TRADETABLE_NUMROWS; ++i) {
        std::wstring expiryDate = AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEEXPIRY + i));
        int days = AfxDaysBetween(transDate, expiryDate);
        AfxSetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEDTE + i), std::to_wstring(days) + L"d");
    }
}


// ========================================================================================
// Reset all entries in the Trade Management table (except Ticker & Company Name)
// ========================================================================================
void ResetTradeTableControls(HWND hwnd)
{
    AfxSetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTDESCRIPTION), L"");

    // Trade Management Table entries
    for (int i = 0; i < TRADEDIALOG_TRADETABLE_NUMROWS; i++) {
        LineCtrl lc = lCtrls.at(i);
        ComboBox_SetCurSel(lc.cols[0], -1);  // action
        AfxSetWindowText(lc.cols[1], L"");   // quantity
        AfxSetWindowText(lc.cols[2], L"");   // expiry date
        AfxSetWindowText(lc.cols[3], L"");   // strike price
        ComboBox_SetCurSel(lc.cols[4], -1);  // put/call
        AfxSetWindowText(lc.cols[5], L"");   // DTE
    }

    AfxSetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTQUANTITY), L"0.0000");
    AfxSetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTPRICE), L"0.0000");
    AfxSetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTFEES), L"0.0000");
    AfxSetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTTOTAL), L"0.0000");

    ComboBox_SetCurSel(GetDlgItem(hwnd, IDC_TRADEDIALOG_COMBODRCR), 0);
    CalculateTradeDTE(hwnd);
}


// ========================================================================================
// Load the data for the selected Trade Template into the Trade Management table
// ========================================================================================
void LoadTemplateInTradeTable(HWND hwnd, CTradeTemplate* pTradeTemplate)
{
    if (pTradeTemplate == nullptr) return;

    ResetTradeTableControls(hwnd);

    // Select the Template in the listbox. The incoming pTradeTemplate could
    // have been a selection from the MenuPanel menu.
    HWND hTemplates = GetDlgItem(hwnd, IDC_TRADEDIALOG_TEMPLATES);
    int lbCount = ListBox_GetCount(hTemplates);
    for (int i = 0; i < lbCount; i++) {
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hTemplates, i);
        if (ld != nullptr) {
            if (ld->pTradeTemplate == pTradeTemplate) {
                ListBox_SetCurSel(hTemplates, i);
                break;
            }
        }
    }

    // Update the Trade Management table with the details of the selected template.
    AfxSetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTDESCRIPTION), pTradeTemplate->name);

    int i = 0;
    for (const auto& leg : pTradeTemplate->legs) {
        LineCtrl lc = lCtrls.at(i);
        i++;
        
        // action
        int foundAt = ComboBox_FindStringExact(lc.cols[0], -1, leg.action.c_str());
        ComboBox_SetCurSel(lc.cols[0], foundAt);

        // quantity
        AfxSetWindowText(lc.cols[1], leg.quantity.c_str());   
        
        // put/call
        foundAt = ComboBox_FindStringExact(lc.cols[4], -1, leg.PutCall.c_str());
        ComboBox_SetCurSel(lc.cols[4], foundAt);
    }

    // Set keyboard focus to the Transaction date control
    SetFocus(GetDlgItem(hwnd, IDC_TRADEDIALOG_TRANSDATE));
}


// ========================================================================================
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

        switch (uIdSubclass)
        {
        case IDC_TRADEDIALOG_TEMPLATES:
            VScrollBar_Recalculate(GetDlgItem(TradeDialog.WindowHandle(), IDC_TRADEDIALOG_VSCROLLBAR));
            break;
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
            SolidBrush backBrush(GetThemeColor(ThemeElement::MenuPanelBack));
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
// Generic Control subclass Window procedure to deal TextBoxes filtering numeric input
// and handling KILLFOCUS validation.
// ========================================================================================
LRESULT CALLBACK TradeDialog_TextBox_SubclassProc(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    switch (uMsg)
    {

    case WM_CHAR:
    {
        // Allow any character for these TextBoxes
        if (uIdSubclass == IDC_TRADEDIALOG_TXTTICKER) break;
        if (uIdSubclass == IDC_TRADEDIALOG_TXTCOMPANY) break;
        if (uIdSubclass == IDC_TRADEDIALOG_TXTDESCRIPTION) break;


        // wParam is the character code

        // Allow 0 to 9
        if (wParam >= 48 && wParam <= 57) break;

        
        // Allow backspace
        if (wParam == 8) break;

        
        // Allow decimal (but only once)
        if (wParam == 46) {
            std::wstring wszText = AfxGetWindowText(hWnd);
            int nFoundAt = wszText.find(L".");
            if (nFoundAt != std::wstring::npos) {
                // Period already exists but we will allow it if the TextBox text
                // is currently selected and entering the period will replace the
                // previously entered period.
                int nPos = SendMessage(hWnd, EM_GETSEL, 0, 0);
                int nStartPos = LOWORD(nPos);
                int nEndPos = HIWORD(nPos);
                if (nStartPos != nEndPos) {   // we have selected text
                    if (nFoundAt >= nStartPos && nFoundAt <= nEndPos) {
                        break;   // allow
                    }
                }

                return 0;  // do not allow
            }
            // Allow the the decimal
            break;
        }

        
        // Negative sign 
        if (wParam == 45) {

            // Do not allow negative sign in the following textboxes 
            switch (uIdSubclass)
            {
            case IDC_TRADEDIALOG_TXTQUANTITY:
            case IDC_TRADEDIALOG_TXTPRICE:
            case IDC_TRADEDIALOG_TXTFEES:
                return 0;
            }

            // Only allow the negative sign for other textboxes if it is 
            // the first character (position 0)
            DWORD nPos = LOWORD(SendMessage(hWnd, EM_GETSEL, 0, 0));
            if (nPos == 0) break; 

            // Otherwise, disallow the negative sign
            return 0;
        }

        
        // Disallow everything else by skipping calling the DefSubclassProc
        return 0;
    }


    case WM_KEYDOWN:
    {
        // Handle up/down arrowns for the Quantity and Allow any character for these TextBoxes
        int i = 0;
        if (wParam == VK_UP) i = -1;
        if (wParam == VK_DOWN) i = 1;

        if (uIdSubclass >= IDC_TRADEDIALOG_TABLEQUANTITY && 
            uIdSubclass <= IDC_TRADEDIALOG_TABLEQUANTITY + TRADEDIALOG_TRADETABLE_NUMROWS) {
            if (uIdSubclass + i < IDC_TRADEDIALOG_TABLEQUANTITY) i = 0;
            if (uIdSubclass + i > IDC_TRADEDIALOG_TABLEQUANTITY + TRADEDIALOG_TRADETABLE_NUMROWS - 1) i = 0;
            SetFocus(GetDlgItem(GetParent(hWnd), uIdSubclass + i));
        }
            
        if (uIdSubclass >= IDC_TRADEDIALOG_TABLESTRIKE && 
            uIdSubclass <= IDC_TRADEDIALOG_TABLESTRIKE + TRADEDIALOG_TRADETABLE_NUMROWS) {
            if (uIdSubclass + i < IDC_TRADEDIALOG_TABLESTRIKE) i = 0;
            if (uIdSubclass + i > IDC_TRADEDIALOG_TABLESTRIKE + TRADEDIALOG_TRADETABLE_NUMROWS - 1) i = 0;
            SetFocus(GetDlgItem(GetParent(hWnd), uIdSubclass + i));
        }
    }
    break;


    case WM_KILLFOCUS:
        // We will use special 4 decimal place formatting on the following TextBoxes
        switch (uIdSubclass)
        {
        case IDC_TRADEDIALOG_TXTQUANTITY:
        case IDC_TRADEDIALOG_TXTPRICE:
        case IDC_TRADEDIALOG_TXTFEES:
            FormatNumberFourDecimals(hWnd);
            CalculateTradeTotal(GetParent(hWnd));
            break;

        case IDC_TRADEDIALOG_TXTTICKER:
            // If the Company Name textbox is empty then attempt to lookup the specified
            // Ticker and fill in the corresponding Company Name.
            std::wstring wszCompanyName = AfxGetWindowText(GetDlgItem(GetParent(hWnd), IDC_TRADEDIALOG_TXTCOMPANY));
            if (wszCompanyName.length() != 0) break;

            std::wstring tickerSymbol = AfxGetWindowText(GetDlgItem(GetParent(hWnd), IDC_TRADEDIALOG_TXTTICKER));

            auto iter = std::find_if(trades.begin(), trades.end(),
                [&](const Trade* t) { return (t->tickerSymbol == tickerSymbol); });

            if (iter != trades.end()) {
                auto index = std::distance(trades.begin(), iter);
                AfxSetWindowText(GetDlgItem(GetParent(hWnd), IDC_TRADEDIALOG_TXTCOMPANY), trades.at(index)->tickerName);
            }
            break;

        }
        break;


    case WM_SETFOCUS:
        // Highlight the text in the TextBox as it gains focus
        Edit_SetSel(hWnd, 0, -1);
        break;


    case WM_DESTROY:
        // REQUIRED: Remove control subclassing
        RemoveWindowSubclass(hWnd, TradeDialog_TextBox_SubclassProc, uIdSubclass);
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
    HWND hTemplates = GetDlgItem(hwnd, IDC_TRADEDIALOG_TEMPLATES);
    HWND hVScrollBar = GetDlgItem(hwnd, IDC_TRADEDIALOG_VSCROLLBAR);
    HWND hTransDate = GetDlgItem(hwnd, IDC_TRADEDIALOG_TRANSDATE);
    HWND txtTicker = GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTTICKER);
    HWND txtCompany = GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTCOMPANY);
    HWND txtDescription = GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTDESCRIPTION);
    HWND hHeader = GetDlgItem(hwnd, IDC_TRADEDIALOG_HEADER);
    HWND lblQuantity = GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLQUANTITY);
    HWND lblPrice = GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLPRICE);
    HWND lblFees = GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLFEES);
    HWND lblTotal = GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLTOTAL);
    HWND txtQuantity = GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTQUANTITY);
    HWND txtPrice = GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTPRICE);
    HWND txtFees = GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTFEES);
    HWND txtTotal = GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTTOTAL);
    HWND comboDRCR = GetDlgItem(hwnd, IDC_TRADEDIALOG_COMBODRCR);
    HWND hFrame1 = GetDlgItem(hwnd, IDC_TRADEDIALOG_FRAME1);
    HWND hFrame2 = GetDlgItem(hwnd, IDC_TRADEDIALOG_FRAME2);

    HDWP hdwp = BeginDeferWindowPos(100);


    int vmargin = AfxScaleY(10);
    int hmargin = AfxScaleX(10);

    // Do not call the calcVThumbRect() function during a scrollbar move. This WM_SIZE
    // gets triggered when the ListBox WM_DRAWITEM fires. If we do another calcVThumbRect()
    // calcualtion then the scrollbar will appear "jumpy" under the user's mouse cursor.
    bool bShowScrollBar = false;
    VScrollBar* pData = VScrollBar_GetPointer(hVScrollBar);
    if (pData != nullptr) bShowScrollBar = (pData->bDragActive) ? true : pData->calcVThumbRect();
    int VScrollBarWidth = bShowScrollBar ? AfxScaleX(VSCROLLBAR_WIDTH) : 0;


    int nTop = 0;
    int nLeft = 0;
    int nWidth = 0;
    int nHeight = 0;
    int nStartTop = 0;
    int nStartLeft = 0;

    int nCtlHeight = AfxScaleY(TRADEDIALOG_TRADETABLE_ROWHEIGHT);
    int nCtlWidth = AfxScaleY(90);

    nLeft = hmargin;
    nTop = vmargin;
    nWidth = AfxScaleX(180);
    nHeight = cy - nTop - vmargin;
    hdwp = DeferWindowPos(hdwp, hTemplates, 0, nLeft, nTop, nWidth, nHeight,
        SWP_NOZORDER | SWP_SHOWWINDOW);

    nLeft = nLeft + nWidth;   // right edge of ListBox
    nWidth = VScrollBarWidth;
    hdwp = DeferWindowPos(hdwp, hVScrollBar, 0, nLeft, nTop, nWidth, nHeight,
        SWP_NOZORDER | (bShowScrollBar ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));

    nStartLeft = nLeft + nWidth + AfxScaleX(50) + hmargin;
    
    nLeft = nStartLeft;
    nTop = (vmargin * 3);
    nWidth = AfxScaleX(100);
    nHeight = nCtlHeight;
    hdwp = DeferWindowPos(hdwp, hTransDate, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nLeft = nLeft + nWidth + (hmargin * 2);
    nWidth = AfxScaleX(75);
    nHeight = nCtlHeight;
    hdwp = DeferWindowPos(hdwp, txtTicker, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nLeft = nLeft + nWidth + hmargin;
    nWidth = AfxScaleX(235);
    nHeight = nCtlHeight;
    hdwp = DeferWindowPos(hdwp, txtCompany, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nLeft = nStartLeft + AfxScaleX(100) + (hmargin * 2);
    nTop = nTop + nHeight + vmargin;
    nWidth = AfxScaleX(320);
    nHeight = nCtlHeight;
    hdwp = DeferWindowPos(hdwp, txtDescription, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);



    // Position all of the controls for the Trade Management table
    nLeft = nStartLeft;
    nTop = nTop + nHeight + (vmargin * 2);
    nStartTop = nTop;
    int hsp = AfxScaleX(4);   // horizontal spacer
    int vsp = AfxScaleX(4);   // vertical spacer


    // Frame1 (around the table header)
    nLeft = nStartLeft;
    nTop = nStartTop;
    nWidth = AfxScaleX(TRADEDIALOG_TRADETABLE_WIDTH);
    nHeight = nCtlHeight + (vsp * 2);
    hdwp = DeferWindowPos(hdwp, hFrame1, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    // Table Header row (non-sizable)
    nLeft = nStartLeft + hsp;
    nTop = nStartTop + vsp;
    nWidth = AfxScaleX((float)(TRADEDIALOG_TRADETABLE_WIDTH - hsp));
    nHeight = nCtlHeight;
    hdwp = DeferWindowPos(hdwp, hHeader, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    // Rows of the Trade Management table controls
    nLeft = nStartLeft;
    nTop = nTop + nHeight + (vsp * 2);
    nWidth = nCtlWidth;
    nHeight = nCtlHeight;


    for (int i = 0; i < TRADEDIALOG_TRADETABLE_NUMROWS; i++) {
        for (int ii = 0; ii < 6; ii++) {
           hdwp = DeferWindowPos(hdwp, lCtrls.at(i).cols[ii], 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);
            nLeft += nWidth + hsp;
        }
        nLeft = nStartLeft;
        nTop = nTop + nHeight + vsp;
    }

    // Frame2 (bottom of the table)
    nLeft = nStartLeft;
    nTop = nTop + vsp;
    nWidth = AfxScaleX(TRADEDIALOG_TRADETABLE_WIDTH);
    nHeight = AfxScaleY(1);
    hdwp = DeferWindowPos(hdwp, hFrame2, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);


    nStartTop = nTop + nHeight + (vmargin * 4);

    nLeft = nStartLeft + AfxScaleX(34);
    nWidth = AfxScaleX(60);
    nHeight = nCtlHeight;
    nStartLeft = nLeft;

    nTop = nStartTop;
    hdwp = DeferWindowPos(hdwp, lblQuantity, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nTop = nTop + nHeight + vmargin;
    hdwp = DeferWindowPos(hdwp, lblPrice, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nTop = nTop + nHeight + vmargin;
    hdwp = DeferWindowPos(hdwp, lblFees, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nTop = nTop + nHeight + vmargin;
    hdwp = DeferWindowPos(hdwp, lblTotal, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);


    nLeft = nStartLeft + nWidth;
    nTop = nStartTop;
    nWidth = nCtlWidth;
    nHeight = nCtlHeight;
    hdwp = DeferWindowPos(hdwp, txtQuantity, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nTop = nTop + nHeight + vmargin;
    nWidth = nCtlWidth;
    nHeight = nCtlHeight;
    hdwp = DeferWindowPos(hdwp, txtPrice, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nTop = nTop + nHeight + vmargin;
    nWidth = nCtlWidth;
    nHeight = nCtlHeight;
    hdwp = DeferWindowPos(hdwp, txtFees, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nTop = nTop + nHeight + vmargin;
    nWidth = nCtlWidth;
    nHeight = nCtlHeight;
    hdwp = DeferWindowPos(hdwp, txtTotal, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nLeft = nLeft + nWidth + AfxScaleX(20);
    nWidth = AfxScaleX(70);
    nHeight = nCtlHeight;
    hdwp = DeferWindowPos(hdwp, comboDRCR, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    EndDeferWindowPos(hdwp);

    VScrollBar_Recalculate(hVScrollBar);
}


// ========================================================================================
// Helper function for WM_CREATE message for window/dialog: TradeDialog
// ========================================================================================
void TradeDialogControls_CreateControls(HWND hwnd)
{
    HWND hCtl = NULL;

    // Create an Ownerdraw listbox that we will use to custom paint our various Trade Templates.
    hCtl =
        TradeDialog.AddControl(Controls::ListBox, hwnd, IDC_TRADEDIALOG_TEMPLATES, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWFIXED | LBS_NOTIFY,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)TradeDialog_ListBox_SubclassProc,
            IDC_TRADEDIALOG_TEMPLATES, NULL);

    // Create our custom vertical scrollbar and attach the ListBox to it.
    CreateVScrollBar(hwnd, IDC_TRADEDIALOG_VSCROLLBAR, hCtl);

    // Add all of our Trade Templates to the ListBox
    ListBoxData_OutputTradesTemplates(hCtl);

    // Resize columns
    ListBoxData_ResizeColumnWidths(hCtl, TableType::TradeTemplates, -1);

    hCtl = TradeDialog.AddControl(Controls::DateTimePicker, hwnd, IDC_TRADEDIALOG_TRANSDATE);
    DateTime_SetFormat(hCtl, L"yyyy-MM-dd");

    hCtl = TradeDialog.AddControl(Controls::TextBox, hwnd, IDC_TRADEDIALOG_TXTTICKER,
        L"", 0, 0, 0, 0,
        WS_VISIBLE | WS_TABSTOP | ES_CENTER | ES_AUTOHSCROLL, -1, NULL,
        (SUBCLASSPROC)TradeDialog_TextBox_SubclassProc, IDC_TRADEDIALOG_TXTTICKER, NULL);
    Edit_SetCueBannerText(hCtl, L"Ticker");

    hCtl = TradeDialog.AddControl(Controls::TextBox, hwnd, IDC_TRADEDIALOG_TXTCOMPANY,
        L"", 0, 0, 0, 0,
        WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL, -1, NULL,
        (SUBCLASSPROC)TradeDialog_TextBox_SubclassProc, IDC_TRADEDIALOG_TXTCOMPANY, NULL);
        Edit_SetCueBannerText(hCtl, L"Company Name");

    hCtl = TradeDialog.AddControl(Controls::TextBox, hwnd, IDC_TRADEDIALOG_TXTDESCRIPTION,
        L"", 0, 0, 0, 0,
        WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL, -1, NULL,
        (SUBCLASSPROC)TradeDialog_TextBox_SubclassProc, IDC_TRADEDIALOG_TXTDESCRIPTION, NULL);
        Edit_SetCueBannerText(hCtl, L"Description");


    TradeDialog.AddControl(Controls::Frame, hwnd, IDC_TRADEDIALOG_FRAME1);
    TradeDialog.AddControl(Controls::Frame, hwnd, IDC_TRADEDIALOG_FRAME2);


    hCtl = TradeDialog.AddControl(Controls::Header, hwnd, IDC_TRADEDIALOG_HEADER);
    int nWidth = AfxScaleX(91);
    Header_InsertNewItem(hCtl, 0, nWidth, L"Action", HDF_CENTER);
    Header_InsertNewItem(hCtl, 1, nWidth, L"Quantity", HDF_CENTER);
    Header_InsertNewItem(hCtl, 2, nWidth, L"Expiry Date", HDF_CENTER);
    Header_InsertNewItem(hCtl, 3, nWidth, L"Strike Price", HDF_CENTER);
    Header_InsertNewItem(hCtl, 4, nWidth, L"Put/Call", HDF_CENTER);
    Header_InsertNewItem(hCtl, 5, nWidth, L"DTE", HDF_CENTER);


    // Create the Trade Management table controls 
    lCtrls.clear();
    lCtrls.reserve(TRADEDIALOG_TRADETABLE_NUMROWS);

    for (int i = 0; i < TRADEDIALOG_TRADETABLE_NUMROWS; i++) {
        LineCtrl lc;

        // ACTION
        hCtl = lc.cols[0] = TradeDialog.AddControl(Controls::ComboBox, hwnd, IDC_TRADEDIALOG_TABLEACTION + i);
        ComboBox_AddString(hCtl, L"STO");
        ComboBox_AddString(hCtl, L"BTC");
        ComboBox_AddString(hCtl, L"BTO");
        ComboBox_AddString(hCtl, L"STC");

        // QUANTITY
        lc.cols[1] = TradeDialog.AddControl(Controls::TextBox, hwnd, IDC_TRADEDIALOG_TABLEQUANTITY + i, 
            L"", 0, 0, 0, 0,
            WS_VISIBLE | WS_TABSTOP | ES_CENTER | ES_AUTOHSCROLL, -1, NULL,
            (SUBCLASSPROC)TradeDialog_TextBox_SubclassProc, IDC_TRADEDIALOG_TABLEQUANTITY + i, NULL);

        // EXPIRY DATE
        lc.cols[2] = TradeDialog.AddControl(Controls::DateTimePicker, hwnd, IDC_TRADEDIALOG_TABLEEXPIRY + i);

        // STRIKE PRICE
        lc.cols[3] = TradeDialog.AddControl(Controls::TextBox, hwnd, IDC_TRADEDIALOG_TABLESTRIKE + i,
            L"", 0, 0, 0, 0,
            WS_VISIBLE | WS_TABSTOP | ES_CENTER | ES_AUTOHSCROLL, -1, NULL,
            (SUBCLASSPROC)TradeDialog_TextBox_SubclassProc, IDC_TRADEDIALOG_TABLESTRIKE + i, NULL);

        // PUT/CALL
        hCtl = lc.cols[4] = TradeDialog.AddControl(Controls::ComboBox, hwnd, IDC_TRADEDIALOG_TABLEPUTCALL + i);
        ComboBox_AddString(hCtl, L"PUT");
        ComboBox_AddString(hCtl, L"CALL");

        // DTE
        lc.cols[5] = TradeDialog.AddControl(Controls::Label, hwnd, IDC_TRADEDIALOG_TABLEDTE + i, L"0d", 0, 0, 0, 0,
            WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE | WS_GROUP | SS_NOTIFY, 0);

        lCtrls.push_back(lc);
    }


    ThemeElement TextColor = ThemeElement::TradesPanelText;
    ThemeElement BackColor = ThemeElement::TradesPanelBack;

    SuperLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_LBLQUANTITY, L"Quantity", TextColor, BackColor);
    SuperLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_LBLPRICE, L"Price", TextColor, BackColor);
    SuperLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_LBLFEES, L"Fees", TextColor, BackColor);
    SuperLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_LBLTOTAL, L"Total", TextColor, BackColor);

    hCtl = TradeDialog.AddControl(Controls::TextBox, hwnd, IDC_TRADEDIALOG_TXTQUANTITY, L"", 0, 0, 0, 0,
        WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, -1, NULL,
        (SUBCLASSPROC)TradeDialog_TextBox_SubclassProc, IDC_TRADEDIALOG_TXTQUANTITY, NULL);
    FormatNumberFourDecimals(hCtl);

    hCtl = TradeDialog.AddControl(Controls::TextBox, hwnd, IDC_TRADEDIALOG_TXTPRICE, L"", 0, 0, 0, 0,
        WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, -1, NULL,
        (SUBCLASSPROC)TradeDialog_TextBox_SubclassProc, IDC_TRADEDIALOG_TXTPRICE, NULL);
    FormatNumberFourDecimals(hCtl);

    hCtl = TradeDialog.AddControl(Controls::TextBox, hwnd, IDC_TRADEDIALOG_TXTFEES, L"", 0, 0, 0, 0,
        WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, -1, NULL,
        (SUBCLASSPROC)TradeDialog_TextBox_SubclassProc, IDC_TRADEDIALOG_TXTFEES, NULL);
    FormatNumberFourDecimals(hCtl);

    // Can not set the Totals as readonly because if we do then we won't be able to get the
    // OnCtlColorEdit message to color the text red/green.
    hCtl = TradeDialog.AddControl(Controls::TextBox, hwnd, IDC_TRADEDIALOG_TXTTOTAL, L"", 0, 0, 0, 0,
        WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, -1, NULL,
        (SUBCLASSPROC)TradeDialog_TextBox_SubclassProc, IDC_TRADEDIALOG_TXTTOTAL, NULL);
    FormatNumberFourDecimals(hCtl);

    hCtl = TradeDialog.AddControl(Controls::ComboBox, hwnd, IDC_TRADEDIALOG_COMBODRCR);
    ComboBox_AddString(hCtl, L"CREDIT");
    ComboBox_AddString(hCtl, L"DEBIT");
    ComboBox_SetCurSel(hCtl, 0);

    TradeDialog.AddControl(Controls::Button, hwnd, IDC_TRADEDIALOG_OK, L"OK", 730, 509, 74, 28);
    TradeDialog.AddControl(Controls::Button, hwnd, IDC_TRADEDIALOG_CANCEL, L"Cancel", 730, 547, 74, 28);
}


