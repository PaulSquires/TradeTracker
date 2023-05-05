#include "pch.h"

#include "TradeDialog.h"
#include "..\VScrollBar\VScrollBar.h"
#include "..\Utilities\ListBoxData.h"
#include "..\Utilities\AfxWin.h"
#include "..\SuperLabel\SuperLabel.h"
#include "..\TradesPanel\TradesPanel.h"


struct LineCtrl {
    HWND cols[6]{ NULL };
};

std::vector<LineCtrl> lCtrls;

extern CTradeDialog TradeDialog;
extern int tradeAction;
extern std::vector<Leg*> legsEdit;
extern Trade* tradeEdit;




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
    double multiplier = stod(AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTMULTIPLIER)));
    double price = stod(AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTPRICE)));
    double fees = stod(AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTFEES)));

    std::wstring DRCR = AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_COMBODRCR));
    if (DRCR == L"CREDIT") fees = fees * -1;

    total = (quantity * multiplier * price) + fees;
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
    AfxSetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTMULTIPLIER), L"100.0000");
    AfxSetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTPRICE), L"0.0000");
    AfxSetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTFEES), L"0.0000");
    AfxSetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTTOTAL), L"0.0000");

    ComboBox_SetCurSel(GetDlgItem(hwnd, IDC_TRADEDIALOG_COMBODRCR), 0);
    CalculateTradeDTE(hwnd);
}



// ========================================================================================
// Load the legs for the edit Action into the Trade Management table
// ========================================================================================
void LoadEditLegsInTradeTable(HWND hwnd)
{
    if (legsEdit.size() == 0) return;

    // Update the Trade Management table with the details of the Trade.
    AfxSetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTTICKER), tradeEdit->tickerSymbol);
    AfxSetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTCOMPANY), tradeEdit->tickerName);
    
    int foundAt = ComboBox_FindStringExact(GetDlgItem(hwnd, IDC_TRADEDIALOG_COMBODRCR), -1, L"DEBIT");
    ComboBox_SetCurSel(GetDlgItem(hwnd, IDC_TRADEDIALOG_COMBODRCR), foundAt);

    int DefaultQuantity = 0;

    // Display the legs being closed and set each to the needed inverse action.
    int nextRow = 0;
    for (const auto& leg : legsEdit) {
        LineCtrl lc = lCtrls.at(nextRow);

        // ACTION
        std::wstring action = leg->action;
        if (action == L"STO") action = L"BTC";
        if (action == L"BTO") action = L"STC";
        int foundAt = ComboBox_FindStringExact(lc.cols[0], -1, action.c_str());
        ComboBox_SetCurSel(lc.cols[0], foundAt);

        // QUANTITY
        DefaultQuantity = leg->openQuantity;
        std::wstring legQuantity = std::to_wstring(leg->openQuantity * -1);
        AfxSetWindowText(lc.cols[1], legQuantity.c_str());

        // EXPIRY DATE
        AfxSetDateTimePickerDate(lc.cols[2], leg->expiryDate);

        // STRIKE PRICE
        AfxSetWindowText(lc.cols[3], leg->strikePrice.c_str());

        // PUT/CALL
        std::wstring PutCall = leg->PutCall;
        if (PutCall == L"P") PutCall = L"PUT";
        if (PutCall == L"C") PutCall = L"CALL";
        foundAt = ComboBox_FindStringExact(lc.cols[4], -1, PutCall.c_str());
        ComboBox_SetCurSel(lc.cols[4], foundAt);

        nextRow++;
    }
    
    // DTE
    CalculateTradeDTE(hwnd);

    // QUANTITY
    AfxSetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTQUANTITY), std::to_wstring(abs(DefaultQuantity)));
    FormatNumberFourDecimals(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTQUANTITY));


    // Add some default information for the new Roll transaction
    //if (tradeAction == ACTION_ROLL_LEG) {
    //    nextRow++;
    //    nextRow++;
    //    for (const auto& leg : legsEdit) {
    //        LineCtrl lc = lCtrls.at(nextRow);
    //        AfxSetWindowText(lc.cols[1], std::to_wstring(leg->openQuantity));
    //        // Set the new expiry date to be 1 week from the current expiry date as a new default
    //        QDate newExpiryDate = QDate::fromString(leg->expiryDate, "yyyy-MM-dd").addDays(7);
    //        ui->tableLegs->cellWidget(i + nextRow, 1)->setProperty("date", newExpiryDate.toString("yyyy-MM-dd"));
    //        ui->tableLegs->cellWidget(i + nextRow, 3)->setProperty("text", leg->strikePrice);
    //        ui->tableLegs->cellWidget(i + nextRow, 4)->setProperty("currentText", leg->PutCall);
    //        ui->tableLegs->cellWidget(i + nextRow, 5)->setProperty("currentText", leg->action);
    //    }
    //    ui->comboDRCR->setCurrentText("CR");
    //}

}


// ========================================================================================
// Header control subclass Window procedure
// ========================================================================================
LRESULT CALLBACK TradeDialog_Header_SubclassProc(
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
        RemoveWindowSubclass(hWnd, TradeDialog_Header_SubclassProc, uIdSubclass);
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
        // Handle up/down arrows to move vertical amongst legs textboxes.
        if (wParam == VK_UP || wParam == VK_DOWN) {
            int i = 0;
            if (wParam == VK_UP) i = -1;
            if (wParam == VK_DOWN) i = 1;

            DWORD NumTableRows = TRADEDIALOG_TRADETABLE_NUMROWS;
            if (tradeAction == ACTION_ROLL_LEG) {
                NumTableRows *= 2;
            }

            if (uIdSubclass >= IDC_TRADEDIALOG_TABLEQUANTITY && 
                uIdSubclass <= IDC_TRADEDIALOG_TABLEQUANTITY + NumTableRows) {
                if (uIdSubclass + i < IDC_TRADEDIALOG_TABLEQUANTITY) i = 0;
                if (uIdSubclass + i > IDC_TRADEDIALOG_TABLEQUANTITY + NumTableRows - 1) i = 0;
                SetFocus(GetDlgItem(GetParent(hWnd), uIdSubclass + i));
            }
            
            if (uIdSubclass >= IDC_TRADEDIALOG_TABLESTRIKE && 
                uIdSubclass <= IDC_TRADEDIALOG_TABLESTRIKE + NumTableRows) {
                if (uIdSubclass + i < IDC_TRADEDIALOG_TABLESTRIKE) i = 0;
                if (uIdSubclass + i > IDC_TRADEDIALOG_TABLESTRIKE + NumTableRows - 1) i = 0;
                SetFocus(GetDlgItem(GetParent(hWnd), uIdSubclass + i));
            }
        }

    }
    break;


    case WM_KEYUP:
    {
        // Make the ENTER key behave like the TAB key. Need to catch VK_RETURN in the
        // WM_KEYUP rather than WM_KEYDOWN.
        if (wParam == VK_RETURN) {
            HWND hNextCtrl = GetNextDlgTabItem(GetParent(hWnd), hWnd, false);
            SetFocus(hNextCtrl);
        }
    }
    break;


    case WM_KILLFOCUS:
        // We will use special 4 decimal place formatting on the following TextBoxes
        switch (uIdSubclass)
        {
        case IDC_TRADEDIALOG_TXTQUANTITY:
        case IDC_TRADEDIALOG_TXTMULTIPLIER:
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
// Set the Edit label and the trade description based on the type of trade action.
// ========================================================================================
void TradeDialogControls_SetEditLabelAndDescription(HWND hwnd)
{
    std::wstring wszAction;
    std::wstring wszDescription;

    switch (tradeAction)
    {
    case ACTION_NEW_TRADE:
        wszAction = L"NEW";
        break;
    case ACTION_CLOSE_TRADE:
    case ACTION_CLOSE_LEG:
        wszAction = L"CLOSE";
        wszDescription = L"Close";
        break;
    case ACTION_EXPIRE_TRADE:
    case ACTION_EXPIRE_LEG:
        wszAction = L"EXPIRE";
        wszDescription = L"Expiration";
        break;
    case ACTION_ROLL_LEG:
        wszAction = L"ROLL";
        wszDescription = L"Roll";
        break;
    case ACTION_SHARE_ASSIGNMENT:
        wszAction = L"ASSIGNMENT";
        break;
    case ACTION_ADDTO_TRADE:
    case ACTION_ADDPUTTO_TRADE:
    case ACTION_ADDCALLTO_TRADE:
        wszAction = L"ADD TO";
        break;
    }

    HWND hCtl = GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLEDITACTION);
    SuperLabel * pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->wszText = wszAction;
        SuperLabel_SetOptions(hCtl, pData);
    }
    AfxSetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTDESCRIPTION), wszDescription);

}


// ========================================================================================
// Helper function for WM_CREATE message for window/dialog: TradeDialog
// ========================================================================================
void TradeDialogControls_CreateControls(HWND hwnd)
{
    HWND hCtl = NULL;
    SuperLabel* pData = nullptr;

    int vmargin = 10;
    int hmargin = 10;
    int nLeft = 0;
    int nTop = 0;
    int nWidth = 0;
    int nHeight = 0;


    // TRANSACTION DATE PICKER
    hCtl = TradeDialog.AddControl(
        Controls::DateTimePicker, hwnd, IDC_TRADEDIALOG_TRANSDATE, L"", 60, 30, 100, 22);
    DateTime_SetFormat(hCtl, L"yyyy-MM-dd");

    
    // NEW TRADE SHOWS TEXTBOXES, OTHERS JUST LABELS
    if (tradeAction == ACTION_NEW_TRADE) {
        hCtl = TradeDialog.AddControl(Controls::TextBox, hwnd, IDC_TRADEDIALOG_TXTTICKER,
            L"", 180, 30, 75, 22,
            WS_VISIBLE | WS_TABSTOP | ES_CENTER | ES_AUTOHSCROLL, -1, NULL,
            (SUBCLASSPROC)TradeDialog_TextBox_SubclassProc, IDC_TRADEDIALOG_TXTTICKER, NULL);
            Edit_SetCueBannerText(hCtl, L"Ticker");

        hCtl = TradeDialog.AddControl(Controls::TextBox, hwnd, IDC_TRADEDIALOG_TXTCOMPANY,
            L"", 265, 30, 235, 22,
            WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL, -1, NULL,
            (SUBCLASSPROC)TradeDialog_TextBox_SubclassProc, IDC_TRADEDIALOG_TXTCOMPANY, NULL);
            Edit_SetCueBannerText(hCtl, L"Company Name");

        hCtl = TradeDialog.AddControl(Controls::TextBox, hwnd, IDC_TRADEDIALOG_TXTDESCRIPTION,
            L"", 180, 60, 320, 22,
            WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL, -1, NULL,
            (SUBCLASSPROC)TradeDialog_TextBox_SubclassProc, IDC_TRADEDIALOG_TXTDESCRIPTION, NULL);
            Edit_SetCueBannerText(hCtl, L"Description");

    }
    else {

        hCtl = CreateSuperLabel(
            hwnd, IDC_TRADEDIALOG_LBLTICKER, SuperLabelType::TextOnly,
            180, 30, 75, 20);
        pData = SuperLabel_GetOptions(hCtl);
        if (pData) {
            pData->BackColor = ThemeElement::TradesPanelBack;
            pData->TextColor = ThemeElement::TradesPanelText;
            pData->FontSize = 12;
            pData->TextAlignment = SuperLabelAlignment::TopLeft;
            pData->wszText = L"AAPL";
            SuperLabel_SetOptions(hCtl, pData);
        }

        hCtl = CreateSuperLabel(
            hwnd, IDC_TRADEDIALOG_LBLCOMPANY, SuperLabelType::TextOnly,
            265, 30, 235, 20);
        pData = SuperLabel_GetOptions(hCtl);
        if (pData) {
            pData->BackColor = ThemeElement::TradesPanelBack;
            pData->TextColor = ThemeElement::TradesPanelText;
            pData->FontSize = 14;
            pData->TextAlignment = SuperLabelAlignment::TopLeft;
            pData->wszText = L"Apple Inc.";
            SuperLabel_SetOptions(hCtl, pData);
        }
        
    }



    // EDIT ACTION LABEL
    hCtl = CreateSuperLabel(
        hwnd, IDC_TRADEDIALOG_LBLEDITACTION, SuperLabelType::TextOnly,
        540, 10, 110, 60);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->BackColor = ThemeElement::TradesPanelBack;
        pData->TextColor = ThemeElement::TradesPanelText;
        pData->FontSize = 14;
        pData->TextAlignment = SuperLabelAlignment::TopRight;
        SuperLabel_SetOptions(hCtl, pData);
    }
    TradeDialogControls_SetEditLabelAndDescription(hwnd);



    // Position all of the controls for the Trade Management table
    int hsp = 4;   // horizontal spacer
    int vsp = 4;   // vertical spacer


    // Frame1 (around the table header)
    TradeDialog.AddControl(Controls::Frame, hwnd, IDC_TRADEDIALOG_FRAME1,
        L"", 60, 100, TRADEDIALOG_TRADETABLE_WIDTH, 23);


    // Table Header row (non-sizable)
    nLeft = 61;
    nTop = 101;
    nWidth = TRADEDIALOG_TRADETABLE_WIDTH - 2;
    nHeight = 20;
    hCtl = TradeDialog.AddControl(Controls::Header, hwnd, IDC_TRADEDIALOG_HEADER,
        L"", nLeft, nTop, nWidth, nHeight, -1, -1, NULL, (SUBCLASSPROC)TradeDialog_Header_SubclassProc,
        IDC_TRADEDIALOG_HEADER, NULL);
    nWidth = AfxScaleX(91);
    Header_InsertNewItem(hCtl, 0, nWidth, L"Action", HDF_CENTER);
    Header_InsertNewItem(hCtl, 1, nWidth, L"Quantity", HDF_CENTER);
    Header_InsertNewItem(hCtl, 2, nWidth, L"Expiry Date", HDF_CENTER);
    Header_InsertNewItem(hCtl, 3, nWidth, L"Strike Price", HDF_CENTER);
    Header_InsertNewItem(hCtl, 4, nWidth, L"Put/Call", HDF_CENTER);
    Header_InsertNewItem(hCtl, 5, nWidth, L"DTE", HDF_CENTER);


    // Create the Trade Management table controls 
    nLeft = 60;
    nTop = 125;
    nWidth = 90;
    nHeight = 22;
    int NumTableRows = TRADEDIALOG_TRADETABLE_NUMROWS;
    if (tradeAction == ACTION_ROLL_LEG) {
        NumTableRows *= 2;
    }

    lCtrls.clear();
    lCtrls.reserve(NumTableRows);

    for (int row = 0; row < NumTableRows; row++) {
        LineCtrl lc;

        // If we are rolling a trade then add a separator line after the first 4 lines
        if (row == TRADEDIALOG_TRADETABLE_NUMROWS) {
            nTop = nTop + vsp;
            nWidth = TRADEDIALOG_TRADETABLE_WIDTH;
            nHeight = 1;
            TradeDialog.AddControl(Controls::Frame, hwnd, IDC_TRADEDIALOG_FRAMESEP, L"", nLeft, nTop, nWidth, nHeight);
            nLeft = 60;
            nTop = nTop + nHeight + (vsp * 2);
            nWidth = 90;
            nHeight = 22;
        }

        for (int col = 0; col < 6; col++) {
            // ACTION
            hCtl = lc.cols[col] = TradeDialog.AddControl(Controls::ComboBox, hwnd, IDC_TRADEDIALOG_TABLEACTION + row,
                L"", nLeft, nTop, nWidth, nHeight);
            ComboBox_AddString(hCtl, L"STO");
            ComboBox_AddString(hCtl, L"BTC");
            ComboBox_AddString(hCtl, L"BTO");
            ComboBox_AddString(hCtl, L"STC");
            nLeft += nWidth + hsp;

            // QUANTITY
            lc.cols[col] = TradeDialog.AddControl(Controls::TextBox, hwnd, IDC_TRADEDIALOG_TABLEQUANTITY + row,
                L"", nLeft, nTop, nWidth, nHeight,
                WS_VISIBLE | WS_TABSTOP | ES_CENTER | ES_AUTOHSCROLL, -1, NULL,
                (SUBCLASSPROC)TradeDialog_TextBox_SubclassProc, IDC_TRADEDIALOG_TABLEQUANTITY + row, NULL);
            nLeft += nWidth + hsp;

            // EXPIRY DATE
            lc.cols[col] = TradeDialog.AddControl(Controls::DateTimePicker, hwnd, IDC_TRADEDIALOG_TABLEEXPIRY + row,
                L"", nLeft, nTop, nWidth, nHeight);
            nLeft += nWidth + hsp;

            // STRIKE PRICE
            lc.cols[col] = TradeDialog.AddControl(Controls::TextBox, hwnd, IDC_TRADEDIALOG_TABLESTRIKE + row,
                L"", nLeft, nTop, nWidth, nHeight,
                WS_VISIBLE | WS_TABSTOP | ES_CENTER | ES_AUTOHSCROLL, -1, NULL,
                (SUBCLASSPROC)TradeDialog_TextBox_SubclassProc, IDC_TRADEDIALOG_TABLESTRIKE + row, NULL);
            nLeft += nWidth + hsp;

            // PUT/CALL
            hCtl = lc.cols[col] = TradeDialog.AddControl(Controls::ComboBox, hwnd, IDC_TRADEDIALOG_TABLEPUTCALL + row,
                L"", nLeft, nTop, nWidth, nHeight);
            ComboBox_AddString(hCtl, L"PUT");
            ComboBox_AddString(hCtl, L"CALL");
            nLeft += nWidth + hsp;

            // DTE
            lc.cols[col] = TradeDialog.AddControl(Controls::Label, hwnd, IDC_TRADEDIALOG_TABLEDTE + row, 
                L"0d", nLeft, nTop, nWidth, nHeight, WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE | WS_GROUP | SS_NOTIFY, 0);

            lCtrls.push_back(lc);
        }

        nLeft = 60;
        nTop = nTop + nHeight + vsp;
    }


    // Frame2 (bottom of the table)
    nLeft = 60;
    nTop = nTop + vsp;
    nWidth = TRADEDIALOG_TRADETABLE_WIDTH;
    nHeight = 1;
    TradeDialog.AddControl(Controls::Frame, hwnd, IDC_TRADEDIALOG_FRAME2, L"", nLeft, nTop, nWidth, nHeight);


    ThemeElement TextColor = ThemeElement::TradesPanelText;
    ThemeElement BackColor = ThemeElement::TradesPanelBack;

    nTop = 300;

    nLeft = 60 + 34;
    nWidth = 60;
    nHeight = 20;

    SuperLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_LBLQUANTITY, L"Quantity", TextColor, BackColor,
        SuperLabelAlignment::MiddleLeft, nLeft, nTop, nWidth, nHeight);

    nTop = nTop + nHeight + vmargin;
    SuperLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_LBLMULTIPLIER, L"Multiplier", TextColor, BackColor,
        SuperLabelAlignment::MiddleLeft, nLeft, nTop, nWidth, nHeight);

    nTop = nTop + nHeight + vmargin;
    SuperLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_LBLPRICE, L"Price", TextColor, BackColor,
        SuperLabelAlignment::MiddleLeft, nLeft, nTop, nWidth, nHeight);

    nTop = nTop + nHeight + vmargin;
    SuperLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_LBLFEES, L"Fees", TextColor, BackColor,
        SuperLabelAlignment::MiddleLeft, nLeft, nTop, nWidth, nHeight);

    nTop = nTop + nHeight + vmargin;
    SuperLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_LBLTOTAL, L"Total", TextColor, BackColor,
        SuperLabelAlignment::MiddleLeft, nLeft, nTop, nWidth, nHeight);


    nLeft = nLeft + nWidth;
    nTop = 300;
    nWidth = 90;
    nHeight = 20;
    hCtl = TradeDialog.AddControl(Controls::TextBox, hwnd, IDC_TRADEDIALOG_TXTQUANTITY, 
        L"", nLeft, nTop, nWidth, nHeight,
        WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, -1, NULL,
        (SUBCLASSPROC)TradeDialog_TextBox_SubclassProc, IDC_TRADEDIALOG_TXTQUANTITY, NULL);
    FormatNumberFourDecimals(hCtl);

    nTop = nTop + nHeight + vmargin;
    hCtl = TradeDialog.AddControl(Controls::TextBox, hwnd, IDC_TRADEDIALOG_TXTMULTIPLIER, 
        L"100.0000", nLeft, nTop, nWidth, nHeight,
        WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, -1, NULL,
        (SUBCLASSPROC)TradeDialog_TextBox_SubclassProc, IDC_TRADEDIALOG_TXTMULTIPLIER, NULL);
    FormatNumberFourDecimals(hCtl);

    nTop = nTop + nHeight + vmargin;
    hCtl = TradeDialog.AddControl(Controls::TextBox, hwnd, IDC_TRADEDIALOG_TXTPRICE, 
        L"", nLeft, nTop, nWidth, nHeight,
        WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, -1, NULL,
        (SUBCLASSPROC)TradeDialog_TextBox_SubclassProc, IDC_TRADEDIALOG_TXTPRICE, NULL);
    FormatNumberFourDecimals(hCtl);

    nTop = nTop + nHeight + vmargin;
    hCtl = TradeDialog.AddControl(Controls::TextBox, hwnd, IDC_TRADEDIALOG_TXTFEES,
        L"", nLeft, nTop, nWidth, nHeight,
        WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, -1, NULL,
        (SUBCLASSPROC)TradeDialog_TextBox_SubclassProc, IDC_TRADEDIALOG_TXTFEES, NULL);
    FormatNumberFourDecimals(hCtl);

    // Can not set the Totals as readonly because if we do then we won't be able to get the
    // OnCtlColorEdit message to color the text red/green.
    nTop = nTop + nHeight + vmargin;
    hCtl = TradeDialog.AddControl(Controls::TextBox, hwnd, IDC_TRADEDIALOG_TXTTOTAL, 
        L"", nLeft, nTop, nWidth, nHeight,
        WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, -1, NULL,
        (SUBCLASSPROC)TradeDialog_TextBox_SubclassProc, IDC_TRADEDIALOG_TXTTOTAL, NULL);
    FormatNumberFourDecimals(hCtl);


    nLeft = nLeft + nWidth + 20;
    nWidth = 70;
    nHeight = 20;
    hCtl = TradeDialog.AddControl(Controls::ComboBox, hwnd, IDC_TRADEDIALOG_COMBODRCR,
        L"", nLeft, nTop, nWidth, nHeight);
    ComboBox_AddString(hCtl, L"CREDIT");
    ComboBox_AddString(hCtl, L"DEBIT");
    ComboBox_SetCurSel(hCtl, 0);


    // Position OK and Cancel buttons from the right edge
    nLeft = nLeft + 282;
    nTop = 300 + 76;
    TradeDialog.AddControl(Controls::Button, hwnd, IDC_TRADEDIALOG_OK, L"OK", nLeft, nTop, 74, 28);

    nTop = nTop + 36;
    TradeDialog.AddControl(Controls::Button, hwnd, IDC_TRADEDIALOG_CANCEL, L"Cancel", nLeft, nTop, 74, 28);
}


