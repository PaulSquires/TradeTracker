#include "pch.h"

#include "TradeDialog.h"
#include "..\Utilities\ListBoxData.h"
#include "..\Utilities\AfxWin.h"
#include "..\CustomLabel\CustomLabel.h"
#include "..\CustomTextBox\CustomTextBox.h"
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

    int NumTableRows = TRADEDIALOG_TRADETABLE_NUMROWS;
    if (tradeAction == ACTION_ROLL_LEG) NumTableRows *= 2;

    for (int i = 0; i < NumTableRows; ++i) {
        std::wstring expiryDate = AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEEXPIRY + i));
        int days = AfxDaysBetween(transDate, expiryDate);
        AfxSetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEDTE + i), std::to_wstring(days) + L"d");
    }
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

        // QUANTITY
        DefaultQuantity = leg->openQuantity;
        std::wstring legQuantity = std::to_wstring(leg->openQuantity * -1);
        AfxSetWindowText(lc.cols[0], legQuantity.c_str());

        // EXPIRY DATE
        AfxSetDateTimePickerDate(lc.cols[1], leg->expiryDate);

        // STRIKE PRICE
        AfxSetWindowText(lc.cols[3], leg->strikePrice.c_str());

        // PUT/CALL
        std::wstring PutCall = leg->PutCall;
        if (PutCall == L"P") PutCall = L"PUT";
        if (PutCall == L"C") PutCall = L"CALL";
        foundAt = ComboBox_FindStringExact(lc.cols[4], -1, PutCall.c_str());
        ComboBox_SetCurSel(lc.cols[4], foundAt);

        // ACTION
        std::wstring action = leg->action;
        if (action == L"STO") action = L"BTC";
        if (action == L"BTO") action = L"STC";
        int foundAt = ComboBox_FindStringExact(lc.cols[0], -1, action.c_str());
        ComboBox_SetCurSel(lc.cols[5], foundAt);

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


    //case WM_KEYDOWN:
    //{
    //    // Handle up/down arrows to move vertical amongst legs textboxes.
    //    if (wParam == VK_UP || wParam == VK_DOWN) {
    //        int i = 0;
    //        if (wParam == VK_UP) i = -1;
    //        if (wParam == VK_DOWN) i = 1;

    //        DWORD NumTableRows = TRADEDIALOG_TRADETABLE_NUMROWS;
    //        if (tradeAction == ACTION_ROLL_LEG) NumTableRows *= 2;

    //        if (uIdSubclass >= IDC_TRADEDIALOG_TABLEQUANTITY && 
    //            uIdSubclass <= IDC_TRADEDIALOG_TABLEQUANTITY + NumTableRows) {
    //            if (uIdSubclass + i < IDC_TRADEDIALOG_TABLEQUANTITY) i = 0;
    //            if (uIdSubclass + i > IDC_TRADEDIALOG_TABLEQUANTITY + NumTableRows - 1) i = 0;
    //            SetFocus(GetDlgItem(GetParent(hWnd), uIdSubclass + i));
    //        }
    //        
    //        if (uIdSubclass >= IDC_TRADEDIALOG_TABLESTRIKE && 
    //            uIdSubclass <= IDC_TRADEDIALOG_TABLESTRIKE + NumTableRows) {
    //            if (uIdSubclass + i < IDC_TRADEDIALOG_TABLESTRIKE) i = 0;
    //            if (uIdSubclass + i > IDC_TRADEDIALOG_TABLESTRIKE + NumTableRows - 1) i = 0;
    //            SetFocus(GetDlgItem(GetParent(hWnd), uIdSubclass + i));
    //        }
    //    }

    //}
    //break;


    //case WM_KEYUP:
    //{
    //    // Make the ENTER key behave like the TAB key. Need to catch VK_RETURN in the
    //    // WM_KEYUP rather than WM_KEYDOWN.
    //    if (wParam == VK_RETURN) {
    //        HWND hNextCtrl = GetNextDlgTabItem(GetParent(hWnd), hWnd, false);
    //        SetFocus(hNextCtrl);
    //    }
    //}
    //break;


//    case WM_KILLFOCUS:
        //case IDC_TRADEDIALOG_TXTTICKER:
        //    // If the Company Name textbox is empty then attempt to lookup the specified
        //    // Ticker and fill in the corresponding Company Name.
        //    std::wstring wszCompanyName = AfxGetWindowText(GetDlgItem(GetParent(hWnd), IDC_TRADEDIALOG_TXTCOMPANY));
        //    if (wszCompanyName.length() != 0) break;

        //    std::wstring tickerSymbol = AfxGetWindowText(GetDlgItem(GetParent(hWnd), IDC_TRADEDIALOG_TXTTICKER));

        //    auto iter = std::find_if(trades.begin(), trades.end(),
        //        [&](const Trade* t) { return (t->tickerSymbol == tickerSymbol); });

        //    if (iter != trades.end()) {
        //        auto index = std::distance(trades.begin(), iter);
        //        AfxSetWindowText(GetDlgItem(GetParent(hWnd), IDC_TRADEDIALOG_TXTCOMPANY), trades.at(index)->tickerName);
        //    }
        //    break;




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
    CustomLabel * pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->wszText = wszAction;
        CustomLabel_SetOptions(hCtl, pData);
    }
    AfxSetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTDESCRIPTION), wszDescription);

}


// ========================================================================================
// Helper function for WM_CREATE message for window/dialog: TradeDialog
// ========================================================================================
void TradeDialogControls_CreateControls(HWND hwnd)
{
    HWND hCtl = NULL;
    CustomLabel* pData = nullptr;

    int vmargin = 10;
    int hmargin = 10;
    int nLeft = 0;
    int nTop = 0;
    int nWidth = 0;
    int nHeight = 0;
    int hsp = 8;   // horizontal spacer
    int vsp = 4;   // vertical spacer

    ThemeElement TextColor = ThemeElement::TradesPanelText;
    ThemeElement TextColorDim = ThemeElement::TradesPanelTextDim;
    ThemeElement BackColor = ThemeElement::TradesPanelBack;

    COLORREF BorderColor = GetThemeCOLORREF(ThemeElement::MenuPanelTextDim);
    COLORREF BorderColorFocus = GetThemeCOLORREF(ThemeElement::MenuPanelText);


    // EDIT ACTION LABEL
    hCtl = CreateCustomLabel(
        hwnd, IDC_TRADEDIALOG_LBLEDITACTION, CustomLabelType::TextOnly,
        400, 10, 106, 40);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->BackColor = ThemeElement::TradesPanelBack;
        pData->TextColor = ThemeElement::TradesPanelText;
        pData->FontSize = 14;
        pData->TextAlignment = CustomLabelAlignment::TopRight;
        CustomLabel_SetOptions(hCtl, pData);
    }


    // NEW TRADE SHOWS TEXTBOXES, OTHERS JUST LABELS
    if (tradeAction == ACTION_NEW_TRADE) {
        CustomLabel_SimpleLabel(hwnd, -1, L"Ticker", TextColorDim, BackColor,
            CustomLabelAlignment::MiddleLeft, 60, 30, 65, 22);

        hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTTICKER, ES_CENTER, L"", 60, 55, 65, 22);
        CustomTextBox_SetBorderAttributes(hCtl, 1, BorderColorFocus, BorderColor, CustomTextBoxBorder::BorderUnderline);
        CustomTextBox_SetNumericAttributes(hCtl, 4, false);

        CustomLabel_SimpleLabel(hwnd, -1, L"Company Name", TextColorDim, BackColor,
            CustomLabelAlignment::MiddleLeft, 133, 30, 115, 22);

        hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTCOMPANY, ES_LEFT, L"", 133, 55, 252, 22);
        CustomTextBox_SetBorderAttributes(hCtl, 1, BorderColorFocus, BorderColor, CustomTextBoxBorder::BorderUnderline);
        CustomTextBox_SetNumericAttributes(hCtl, 4, false);

    }
    else {

        hCtl = CreateCustomLabel(
            hwnd, IDC_TRADEDIALOG_LBLTICKER, CustomLabelType::TextOnly,
            60, 55, 75, 26);
        pData = CustomLabel_GetOptions(hCtl);
        if (pData) {
            pData->BackColor = ThemeElement::TradesPanelBack;
            pData->TextColor = ThemeElement::TradesPanelText;
            pData->FontSize = 14;
            pData->TextAlignment = CustomLabelAlignment::TopLeft;
            pData->wszText = L"AAPL";
            CustomLabel_SetOptions(hCtl, pData);
        }

        hCtl = CreateCustomLabel(
            hwnd, IDC_TRADEDIALOG_LBLCOMPANY, CustomLabelType::TextOnly,
            135, 55, 235, 26);
        pData = CustomLabel_GetOptions(hCtl);
        if (pData) {
            pData->BackColor = ThemeElement::TradesPanelBack;
            pData->TextColor = ThemeElement::TradesPanelText;
            pData->FontSize = 14;
            pData->TextAlignment = CustomLabelAlignment::TopLeft;
            pData->wszText = L"Apple Inc.";
            CustomLabel_SetOptions(hCtl, pData);
        }
        
    }

    TradeDialogControls_SetEditLabelAndDescription(hwnd);


    CustomLabel_SimpleLabel(hwnd, -1, L"Transaction Date", TextColor, BackColor,
        CustomLabelAlignment::MiddleLeft, 402, 30, 100, 22);
    hCtl = TradeDialog.AddControl(Controls::DateTimePicker, hwnd, IDC_TRADEDIALOG_TRANSDATE,
        L"", 402, 55, 100, 22);
    DateTime_SetFormat(hCtl, L"yyyy-MM-dd");


    // Frame1 (around the table header)
    TradeDialog.AddControl(Controls::Frame, hwnd, -1, L"", 60, 100, TRADEDIALOG_TRADETABLE_WIDTH, 23);


    // Table Header row (non-sizable)
    nLeft = 61;
    nTop = 101;
    nWidth = TRADEDIALOG_TRADETABLE_WIDTH - 2;
    nHeight = 20;
    hCtl = TradeDialog.AddControl(Controls::Header, hwnd, IDC_TRADEDIALOG_HEADER,
        L"", nLeft, nTop, nWidth, nHeight, -1, -1, NULL, (SUBCLASSPROC)TradeDialog_Header_SubclassProc,
        IDC_TRADEDIALOG_HEADER, NULL);
    Header_InsertNewItem(hCtl, 0, 0, L"", HDF_CENTER);
    Header_InsertNewItem(hCtl, 1, AfxScaleX(65), L"Quantity", HDF_CENTER);
    Header_InsertNewItem(hCtl, 2, AfxScaleX(92), L"Expiry Date", HDF_CENTER);
    Header_InsertNewItem(hCtl, 3, AfxScaleX(76), L"DTE", HDF_CENTER);
    Header_InsertNewItem(hCtl, 4, AfxScaleX(70), L"Strike", HDF_CENTER);
    Header_InsertNewItem(hCtl, 5, AfxScaleX(70), L"Put/Call", HDF_CENTER);
    Header_InsertNewItem(hCtl, 6, AfxScaleX(70), L"Action", HDF_CENTER);


    // Create the Trade Management table controls 
    nLeft = 60;
    nTop = 130;
    nHeight = 20;

    int NumTableRows = TRADEDIALOG_TRADETABLE_NUMROWS;
    if (tradeAction == ACTION_ROLL_LEG) NumTableRows *= 2;

    lCtrls.clear();
    lCtrls.reserve(NumTableRows);

    for (int row = 0; row < NumTableRows; row++) {
        LineCtrl lc;

        // If we are rolling a trade then add a separator line after the first 4 lines
        if (row == TRADEDIALOG_TRADETABLE_NUMROWS) {
            nTop = nTop + vsp;
            nWidth = TRADEDIALOG_TRADETABLE_WIDTH;
            nHeight = 1;
            TradeDialog.AddControl(Controls::Frame, hwnd, -1, L"", nLeft, nTop, nWidth, nHeight);
            nLeft = 60;
            nTop = nTop + nHeight + (vsp * 2);
            nHeight = 20;
        }

        // QUANTITY
        nWidth = 65;
        hCtl = lc.cols[0] = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TABLEQUANTITY + row, ES_CENTER, 
            L"0", nLeft, nTop, nWidth, nHeight);
        CustomTextBox_SetBorderAttributes(hCtl, 1, BorderColorFocus, BorderColor, CustomTextBoxBorder::BorderUnderline);
        CustomTextBox_SetNumericAttributes(hCtl, 0, false);
        nLeft += nWidth + hsp;

        // EXPIRY DATE
        nWidth = 85;
        hCtl = lc.cols[1] = TradeDialog.AddControl(Controls::DateTimePicker, hwnd, IDC_TRADEDIALOG_TABLEEXPIRY + row,
            L"", nLeft, nTop, nWidth, nHeight);
        DateTime_SetFormat(hCtl, L"yyyy-MM-dd");
        nLeft += nWidth + hsp;

        // DTE
        nWidth = 65;
        hCtl = lc.cols[2] = TradeDialog.AddControl(Controls::Label, hwnd, IDC_TRADEDIALOG_TABLEDTE + row,
            L"0d", nLeft, nTop, nWidth, nHeight, WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE | WS_GROUP | SS_NOTIFY, 0);
        nLeft += nWidth + hsp;

        // STRIKE PRICE
        nWidth = 65;
        hCtl = lc.cols[3] = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TABLESTRIKE + row, ES_CENTER, 
            L"0", nLeft, nTop, nWidth, nHeight);
        CustomTextBox_SetBorderAttributes(hCtl, 1, BorderColorFocus, BorderColor, CustomTextBoxBorder::BorderUnderline);
        CustomTextBox_SetNumericAttributes(hCtl, 1, false);
        nLeft += nWidth + hsp;

        // PUT/CALL
        nWidth = 61;
        hCtl = lc.cols[4] = TradeDialog.AddControl(Controls::ComboBox, hwnd, IDC_TRADEDIALOG_TABLEPUTCALL + row,
            L"", nLeft, nTop, nWidth, nHeight);
        ComboBox_AddString(hCtl, L"PUT");
        ComboBox_AddString(hCtl, L"CALL");
        nLeft += nWidth + hsp;

        // ACTION
        nWidth = 61;
        hCtl = lc.cols[5] = TradeDialog.AddControl(Controls::ComboBox, hwnd, IDC_TRADEDIALOG_TABLEACTION + row,
            L"", nLeft, nTop, nWidth, nHeight);
        ComboBox_AddString(hCtl, L"STO");
        ComboBox_AddString(hCtl, L"BTC");
        ComboBox_AddString(hCtl, L"BTO");
        ComboBox_AddString(hCtl, L"STC");

        lCtrls.push_back(lc);

        nLeft = 60;
        nTop = nTop + nHeight + vsp;
    }


    // Frame2 (bottom of the table)
    nLeft = 60;
    nTop = nTop + vsp;
    nWidth = TRADEDIALOG_TRADETABLE_WIDTH;
    nHeight = 1;
    TradeDialog.AddControl(Controls::Frame, hwnd, -1, L"", nLeft, nTop, nWidth, nHeight);


    nTop = nTop + 20 + vsp;
    nLeft = 60;
    nHeight = 20;

    int nStartTop = nTop;

    
    nWidth = 80;
    CustomLabel_SimpleLabel(hwnd, -1, L"Quantity", TextColorDim, BackColor,
        CustomLabelAlignment::MiddleLeft, nLeft, nTop, nWidth, nHeight);
    hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTQUANTITY, ES_RIGHT, 
        L"0", nLeft, nTop + nHeight + vsp, nWidth, nHeight);
    CustomTextBox_SetBorderAttributes(hCtl, 1, BorderColorFocus, BorderColor, CustomTextBoxBorder::BorderUnderline);
    CustomTextBox_SetNumericAttributes(hCtl, 4, false);


    nLeft = nLeft + nWidth + hmargin;
    CustomLabel_SimpleLabel(hwnd, -1, L"Multiplier", TextColorDim, BackColor,
        CustomLabelAlignment::MiddleCenter, nLeft, nTop, nWidth, nHeight);
    hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTMULTIPLIER, ES_RIGHT, 
        L"100.0000", nLeft, nTop + nHeight + vsp, nWidth, nHeight);
    CustomTextBox_SetBorderAttributes(hCtl, 1, BorderColorFocus, BorderColor, CustomTextBoxBorder::BorderUnderline);
    CustomTextBox_SetNumericAttributes(hCtl, 4, false);


    nLeft = nLeft + nWidth + hmargin;
    CustomLabel_SimpleLabel(hwnd, -1, L"Price", TextColorDim, BackColor,
        CustomLabelAlignment::MiddleCenter, nLeft, nTop, nWidth, nHeight);
    hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTPRICE, ES_RIGHT,
        L"0", nLeft, nTop + nHeight + vsp, nWidth, nHeight);
    CustomTextBox_SetBorderAttributes(hCtl, 1, BorderColorFocus, BorderColor, CustomTextBoxBorder::BorderUnderline);
    CustomTextBox_SetNumericAttributes(hCtl, 4, false);


    nLeft = nLeft + nWidth + hmargin;
    CustomLabel_SimpleLabel(hwnd, -1, L"Fees", TextColorDim, BackColor,
        CustomLabelAlignment::MiddleCenter, nLeft, nTop, nWidth, nHeight);
    hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTFEES, ES_RIGHT,
        L"0", nLeft, nTop + nHeight + vsp, nWidth, nHeight);
    CustomTextBox_SetBorderAttributes(hCtl, 1, BorderColorFocus, BorderColor, CustomTextBoxBorder::BorderUnderline);
    CustomTextBox_SetNumericAttributes(hCtl, 4, false);


    nLeft = nLeft + nWidth + hmargin;
    CustomLabel_SimpleLabel(hwnd, -1, L"Total", TextColorDim, BackColor,
        CustomLabelAlignment::MiddleRight, nLeft, nTop, nWidth, nHeight);
    // Can not set the Totals as readonly because if we do then we won't be able to get the
    // OnCtlColorEdit message to color the text red/green.
    hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTTOTAL, ES_RIGHT,
        L"0", nLeft, nTop + nHeight + vsp, nWidth, nHeight);
    CustomTextBox_SetBorderAttributes(hCtl, 1, BorderColorFocus, BorderColor, CustomTextBoxBorder::BorderUnderline);
    CustomTextBox_SetNumericAttributes(hCtl, 4, false);


    nTop = nStartTop + ((nHeight + vsp) * 2) + vsp;
    hCtl = TradeDialog.AddControl(Controls::ComboBox, hwnd, IDC_TRADEDIALOG_COMBODRCR,
        L"", nLeft, nTop, nWidth, nHeight);
    ComboBox_AddString(hCtl, L"CREDIT");
    ComboBox_AddString(hCtl, L"DEBIT");
    ComboBox_SetCurSel(hCtl, 0);


    // Position OK and Cancel buttons from the right edge
    nLeft = 60;
    hCtl = TradeDialog.AddControl(Controls::Button, hwnd, IDC_TRADEDIALOG_SAVE, L"SAVE", nLeft, nTop, 80, nHeight);

    //nLeft = nLeft + 80 + hmargin;
    //hCtl = TradeDialog.AddControl(Controls::Button, hwnd, IDC_TRADEDIALOG_CANCEL, L"Cancel", nLeft, nTop, 80, nHeight);
}


