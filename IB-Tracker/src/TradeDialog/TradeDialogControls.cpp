#include "pch.h"

#include "TradeDialog.h"
#include "..\VScrollBar\VScrollBar.h"
#include "..\Utilities\ListBoxData.h"
#include "..\SuperLabel\SuperLabel.h"

#include <uxtheme.h>
#include <vssym32.h>
#include <vsstyle.h>



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
void CalculateTradeTotal()
{
    HWND hWnd = TradeDialog.WindowHandle();
    double total = 0;
    double quantity = stod(AfxGetWindowText(GetDlgItem(hWnd, IDC_TRADEDIALOG_TXTQUANTITY)));
    double price = stod(AfxGetWindowText(GetDlgItem(hWnd, IDC_TRADEDIALOG_TXTPRICE)));
    double fees = stod(AfxGetWindowText(GetDlgItem(hWnd, IDC_TRADEDIALOG_TXTFEES)));

    std::wstring DRCR = AfxGetWindowText(GetDlgItem(hWnd, IDC_TRADEDIALOG_COMBODRCR));
    if (DRCR == L"CREDIT") fees = fees * -1;

    total = (quantity * 100 * price) + fees;
    AfxSetWindowText(GetDlgItem(hWnd, IDC_TRADEDIALOG_TXTTOTAL), std::to_wstring(total));
    FormatNumberFourDecimals(GetDlgItem(hWnd, IDC_TRADEDIALOG_TXTTOTAL));
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
// Generic Control subclass Window procedure to deal filtering only numeric input.
// ========================================================================================
LRESULT CALLBACK TradeDialog_TextBox_SubclassProc(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    switch (uMsg)
    {

    case WM_CHAR:
    {
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


    case WM_KILLFOCUS:
        // We will use special 4 decimal place formatting on the following TextBoxes
        switch (uIdSubclass)
        {
        case IDC_TRADEDIALOG_TXTQUANTITY:
        case IDC_TRADEDIALOG_TXTPRICE:
        case IDC_TRADEDIALOG_TXTFEES:
            FormatNumberFourDecimals(hWnd);
            CalculateTradeTotal();
        }
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
// Generic ComboBox subclass Window procedure to remove borders because "borders" do
// not actually exist for ComboBoxes. They are actually painted in the client area.
// Not sure why Microsoft made this design choice.
// ========================================================================================
LRESULT CALLBACK TradeDialog_ComboBox_SubclassProc(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{

    //HDC				hdc;
    //PAINTSTRUCT		ps;
    //RECT			rect;
    //RECT			rect2;

    static HTHEME   hTheme = NULL;   // theme handle (NULL when not themed)

    switch (uMsg)
    {
/*
    case WM_PAINT:
    {
        if (!hTheme)
            hTheme = OpenThemeData(hWnd, VSCLASS_COMBOBOX);

        if (wParam == 0)	hdc = BeginPaint(hWnd, &ps);
        else				hdc = (HDC)wParam;

        //
        //	Mask off the borders and draw ComboBox normally
        //
        GetClientRect(hWnd, &rect);

        InflateRect(&rect, -GetSystemMetrics(SM_CXEDGE), -GetSystemMetrics(SM_CYEDGE));

        rect.right -= GetSystemMetrics(SM_CXVSCROLL);

        IntersectClipRect(hdc, rect.left, rect.top, rect.right, rect.bottom);

        // Draw the ComboBox
        DefSubclassProc(hWnd, uMsg, wParam, lParam);

        //
        //	Now mask off inside and draw the borders
        //
        SelectClipRgn(hdc, NULL);
        rect.right += GetSystemMetrics(SM_CXVSCROLL);

        ExcludeClipRect(hdc, rect.left, rect.top, rect.right, rect.bottom);

        // draw borders (same color as table background so they "disappear"
        GetClientRect(hWnd, &rect2);
        FillRect(hdc, &rect2, GetSysColorBrush(COLOR_WINDOW));

        // now draw the button
        SelectClipRgn(hdc, NULL);
        rect.left = rect.right - GetSystemMetrics(SM_CXVSCROLL);

         
        if (hTheme) {
            DrawThemeBackground(hTheme, hdc,
                CP_DROPDOWNBUTTON, CBXSL_NORMAL, &rect, NULL);
        } else {
            DrawFrameControl(hdc, &rect, DFC_SCROLL, DFCS_SCROLLCOMBOBOX | DFCS_FLAT);
        }

        if (wParam == 0)
            EndPaint(hWnd, &ps);

        return 0;
    }
    break;


    case WM_THEMECHANGED:
        if (hTheme)
            CloseThemeData(hTheme);
        hTheme = OpenThemeData(hWnd, VSCLASS_COMBOBOX);
        InvalidateRect(hWnd, NULL, TRUE);
        return 0;
*/


    case WM_DESTROY:
        if (hTheme)
            CloseThemeData(hTheme);

        // REQUIRED: Remove control subclassing
        RemoveWindowSubclass(hWnd, TradeDialog_ComboBox_SubclassProc, uIdSubclass);
        break;


    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);

}


// ========================================================================================
// Generic DateTimePicker subclass Window procedure to remove borders.
// ========================================================================================
LRESULT CALLBACK TradeDialog_DateTimePicker_SubclassProc(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{

    //HDC				hdc;
    //PAINTSTRUCT		ps;
    //RECT			rect;
    //RECT			rect2;

    static HTHEME   hTheme = NULL;   // theme handle (NULL when not themed)

    switch (uMsg)
    {

        /*
    case WM_PAINT:
    {
       // if (!hTheme)
       //     hTheme = OpenThemeData(hWnd, VSCLASS_COMBOBOX);

        hdc = BeginPaint(hWnd, &ps);
        
        DefSubclassProc(hWnd, uMsg, wParam, lParam);

        DATETIMEPICKERINFO dtpi;
        dtpi.cbSize = sizeof(DATETIMEPICKERINFO);

        DateTime_GetDateTimePickerInfo(hWnd, &dtpi);


        std::cout << dtpi.hwndDropDown << std::endl;



        //
        //	Mask off the borders and draw ComboBox normally
        //
        //GetClientRect(hWnd, &rect);

        //InflateRect(&rect, -GetSystemMetrics(SM_CXEDGE), -GetSystemMetrics(SM_CYEDGE));

        //rect.right -= GetSystemMetrics(SM_CXVSCROLL);

        //IntersectClipRect(hdc, rect.left, rect.top, rect.right, rect.bottom);

        // Draw the ComboBox
        //DefSubclassProc(hWnd, uMsg, wParam, lParam);

        //
        //	Now mask off inside and draw the borders
        //
        //SelectClipRgn(hdc, NULL);
        //rect.right += GetSystemMetrics(SM_CXVSCROLL);

        //ExcludeClipRect(hdc, rect.left, rect.top, rect.right, rect.bottom);

        // draw borders (same color as table background so they "disappear"
        //GetClientRect(hWnd, &rect2);
        //FillRect(hdc, &rect2, GetSysColorBrush(COLOR_WINDOW));

        // now draw the button
        //SelectClipRgn(hdc, NULL);
        //rect.left = rect.right - GetSystemMetrics(SM_CXVSCROLL);


        //if (hTheme) {
        //    DrawThemeBackground(hTheme, hdc,
         //       CP_DROPDOWNBUTTON, CBXSL_NORMAL, &rect, NULL);
        //}
        //else {
        //    DrawFrameControl(hdc, &rect, DFC_SCROLL, DFCS_SCROLLCOMBOBOX | DFCS_FLAT);
        //}

        //if (wParam == 0)
            EndPaint(hWnd, &ps);

        //return 0;
    }
        break;

*/


    case WM_THEMECHANGED:
        //if (hTheme)
        //    CloseThemeData(hTheme);
        //hTheme = OpenThemeData(hWnd, VSCLASS_COMBOBOX);
        //InvalidateRect(hWnd, NULL, TRUE);
        //return 0;
        break;


    case WM_DESTROY:
        if (hTheme)
            CloseThemeData(hTheme);

        // REQUIRED: Remove control subclassing
        RemoveWindowSubclass(hWnd, TradeDialog_DateTimePicker_SubclassProc, uIdSubclass);
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


    int vmargin = AfxScaleY(6);
    int hmargin = AfxScaleX(10);

    // Do not call the calcVThumbRect() function during a scrollbar move. This WM_SIZE
    // gets triggered when the ListBox WM_DRAWITEM fires. If we do another calcVThumbRect()
    // calcualtion then the scrollbar will appear "jumpy" under the user's mouse cursor.
    bool bShowScrollBar = false;
    VScrollBar* pData = VScrollBar_GetPointer(hVScrollBar);
    if (pData != nullptr) bShowScrollBar = (pData->bDragActive) ? true : pData->calcVThumbRect();
    int VScrollBarWidth = bShowScrollBar ? AfxScaleX(VSCROLLBAR_WIDTH) : 0;


    int nTop = (vmargin * 2);
    int nLeft = 0;
    int nWidth = 0;
    int nHeight = cy - nTop;
    int nStartTop = 0;
    int nStartLeft = 0;

    int nCtlHeight = AfxScaleY(TRADEDIALOG_TRADETABLE_ROWHEIGHT);
    int nCtlWidth = AfxScaleY(90);

    nWidth = AfxScaleX(150);
    SetWindowPos(hTemplates, 0, nLeft, nTop, nWidth, nHeight,
        SWP_NOZORDER | SWP_SHOWWINDOW);

    nLeft = nLeft + nWidth;   // right edge of ListBox
    nWidth = VScrollBarWidth;
    SetWindowPos(hVScrollBar, 0, nLeft, nTop, nWidth, nHeight,
        SWP_NOZORDER | (bShowScrollBar ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));

    nStartLeft = nLeft + nWidth + AfxScaleX(20) + hmargin;
    
    nLeft = nStartLeft;
    nTop = (vmargin * 6);
    nWidth = AfxScaleX(100);
    nHeight = nCtlHeight;
    SetWindowPos(hTransDate, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nLeft = nLeft + nWidth + (hmargin * 2);
    nWidth = AfxScaleX(75);
    nHeight = nCtlHeight;
    SetWindowPos(txtTicker, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nLeft = nLeft + nWidth + hmargin;
    nWidth = AfxScaleX(235);
    nHeight = nCtlHeight;
    SetWindowPos(txtCompany, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nLeft = nStartLeft + AfxScaleX(100) + (hmargin * 2);
    nTop = nTop + nHeight + vmargin;
    nWidth = AfxScaleX(320);
    nHeight = nCtlHeight;
    SetWindowPos(txtDescription, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);



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
    SetWindowPos(hFrame1, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    // Table Header row (non-sizable)
    nLeft = nStartLeft + hsp;
    nTop = nStartTop + vsp;
    nWidth = AfxScaleX(546); 
    nHeight = nCtlHeight;
    SetWindowPos(hHeader, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    // 10 rows of the Trade Management table controls
    nLeft = nStartLeft;
    nTop = nTop + nHeight + (vsp * 2);
    nWidth = nCtlWidth;
    nHeight = nCtlHeight;

    for (int i = 0; i < 10; i++) {
        for (int ii = 0; ii < 6; ii++) {
            SetWindowPos(lCtrls.at(i).cols[ii], 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);
            nLeft += nWidth + hsp;
        }
        nLeft = nStartLeft;
        nTop = nTop + nHeight + vsp;
    }

    // Frame2 (bottome of the table)
    nLeft = nStartLeft;
    nTop = nTop + vsp;
    nWidth = AfxScaleX(TRADEDIALOG_TRADETABLE_WIDTH);
    nHeight = AfxScaleY(1);
    SetWindowPos(hFrame2, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);


    nStartTop = nTop + nHeight + (vmargin * 4);

    nLeft = nStartLeft + AfxScaleX(34);
    nWidth = AfxScaleX(60);
    nHeight = nCtlHeight;
    nStartLeft = nLeft;

    nTop = nStartTop + AfxScaleY(4);
    SetWindowPos(lblQuantity, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nTop = nTop + nHeight + vmargin;
    SetWindowPos(lblPrice, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nTop = nTop + nHeight + vmargin;
    SetWindowPos(lblFees, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nTop = nTop + nHeight + vmargin;
    SetWindowPos(lblTotal, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);


    nLeft = nStartLeft + nWidth;
    nTop = nStartTop;
    nWidth = nCtlWidth;
    nHeight = nCtlHeight;
    SetWindowPos(txtQuantity, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nTop = nTop + nHeight + vmargin;
    nWidth = nCtlWidth;
    nHeight = nCtlHeight;
    SetWindowPos(txtPrice, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nTop = nTop + nHeight + vmargin;
    nWidth = nCtlWidth;
    nHeight = nCtlHeight;
    SetWindowPos(txtFees, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nTop = nTop + nHeight + vmargin;
    nWidth = nCtlWidth;
    nHeight = nCtlHeight;
    SetWindowPos(txtTotal, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nLeft = nLeft + nWidth + hsp;
    nWidth = nCtlWidth;
    nHeight = nCtlHeight;
    SetWindowPos(comboDRCR, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

 
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

    hCtl = TradeDialog.AddControl(Controls::TextBox, hwnd, IDC_TRADEDIALOG_TXTTICKER);
    Edit_SetCueBannerText(hCtl, L"Ticker");

    hCtl = TradeDialog.AddControl(Controls::TextBox, hwnd, IDC_TRADEDIALOG_TXTCOMPANY);
    Edit_SetCueBannerText(hCtl, L"Company Name");

    hCtl = TradeDialog.AddControl(Controls::TextBox, hwnd, IDC_TRADEDIALOG_TXTDESCRIPTION);
    Edit_SetCueBannerText(hCtl, L"Description");


    TradeDialog.AddControl(Controls::Frame, hwnd, IDC_TRADEDIALOG_FRAME1);
    TradeDialog.AddControl(Controls::Frame, hwnd, IDC_TRADEDIALOG_FRAME2);


    hCtl = TradeDialog.AddControl(Controls::Header, hwnd, IDC_TRADEDIALOG_HEADER);
    int nWidth = AfxScaleX(91);
    Header_InsertNewItem(hCtl, 0, nWidth, L"Action", HDF_CENTER);
    Header_InsertNewItem(hCtl, 1, nWidth, L"Quantity", HDF_CENTER);
    Header_InsertNewItem(hCtl, 2, nWidth, L"Strike Price", HDF_CENTER);
    Header_InsertNewItem(hCtl, 3, nWidth, L"Expiry Date", HDF_CENTER);
    Header_InsertNewItem(hCtl, 4, nWidth, L"Put/Call", HDF_CENTER);
    Header_InsertNewItem(hCtl, 5, nWidth, L"DTE", HDF_CENTER);


    // Create the Trade Management table controls 
    lCtrls.reserve(10);

    for (int i = 0; i < 10; i++) {
        LineCtrl lc;

        // ACTION
        //hCtl = lc.cols[0] = TradeDialog.AddControl(Controls::ComboBox, hwnd, -1L, L"", 0, 0, 0, 0,
        //    WS_VISIBLE | WS_VSCROLL | WS_TABSTOP | CBS_DROPDOWNLIST | CBS_HASSTRINGS | CBS_NOINTEGRALHEIGHT, 0, NULL,
        //    (SUBCLASSPROC)TradeDialog_ComboBox_SubclassProc, NULL, NULL);
        hCtl = lc.cols[0] = TradeDialog.AddControl(Controls::ComboBox, hwnd, -1);
        ComboBox_AddString(hCtl, L"BTO");
        ComboBox_AddString(hCtl, L"STO");
        ComboBox_AddString(hCtl, L"BTC");
        ComboBox_AddString(hCtl, L"STC");

        // QUANTITY
        lc.cols[1] = TradeDialog.AddControl(Controls::TextBox, hwnd, -1, L"", 0, 0, 0, 0,
            WS_VISIBLE | WS_TABSTOP | ES_CENTER | ES_AUTOHSCROLL, -1, NULL,
            (SUBCLASSPROC)TradeDialog_TextBox_SubclassProc, NULL, NULL);

        // STRIKE PRICE
        lc.cols[2] = TradeDialog.AddControl(Controls::TextBox, hwnd, -1, L"", 0, 0, 0, 0,
            WS_VISIBLE | WS_TABSTOP | ES_CENTER | ES_AUTOHSCROLL, -1, NULL,
            (SUBCLASSPROC)TradeDialog_TextBox_SubclassProc, NULL, NULL);

        // EXPIRY DATE
        lc.cols[3] = TradeDialog.AddControl(Controls::DateTimePicker, hwnd, -1);


        // PUT/CALL
        hCtl = lc.cols[4] = TradeDialog.AddControl(Controls::ComboBox, hwnd, -1);
        ComboBox_AddString(hCtl, L"PUT");
        ComboBox_AddString(hCtl, L"CALL");

        // DTE
        lc.cols[5] = TradeDialog.AddControl(Controls::Label, hwnd, -1, L"0d", 0, 0, 0, 0,
            WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE | WS_GROUP | SS_NOTIFY, 0);

        lCtrls.push_back(lc);
    }

    TradeDialog.AddControl(Controls::Label, hwnd, IDC_TRADEDIALOG_LBLQUANTITY, L"Quantity");
    TradeDialog.AddControl(Controls::Label, hwnd, IDC_TRADEDIALOG_LBLPRICE, L"Price");
    TradeDialog.AddControl(Controls::Label, hwnd, IDC_TRADEDIALOG_LBLFEES, L"Fees");
    TradeDialog.AddControl(Controls::Label, hwnd, IDC_TRADEDIALOG_LBLTOTAL, L"Total");

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

    hCtl = TradeDialog.AddControl(Controls::TextBox, hwnd, IDC_TRADEDIALOG_TXTTOTAL, L"", 0, 0, 0, 0,
        WS_VISIBLE | ES_READONLY | ES_RIGHT | ES_AUTOHSCROLL, -1, NULL,
        (SUBCLASSPROC)TradeDialog_TextBox_SubclassProc, IDC_TRADEDIALOG_TXTTOTAL, NULL);
    FormatNumberFourDecimals(hCtl);

    hCtl = TradeDialog.AddControl(Controls::ComboBox, hwnd, IDC_TRADEDIALOG_COMBODRCR);
    ComboBox_AddString(hCtl, L"CREDIT");
    ComboBox_AddString(hCtl, L"DEBIT");
    ComboBox_SetCurSel(hCtl, 0);

    TradeDialog.AddControl(Controls::Button, hwnd, IDC_TRADEDIALOG_OK, L"OK", 650, 464, 74, 28);
    TradeDialog.AddControl(Controls::Button, hwnd, IDC_TRADEDIALOG_CANCEL, L"Cancel", 650, 500, 74, 28);

}
