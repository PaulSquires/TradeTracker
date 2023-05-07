#include "pch.h"

#include "TradeDialog.h"
#include "..\Utilities\ListBoxData.h"
#include "..\Utilities\AfxWin.h"
#include "..\CustomLabel\CustomLabel.h"
#include "..\CustomTextBox\CustomTextBox.h"
#include "..\CustomTradeGrid\CustomTradeGrid.h"
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

    COLORREF TextColor = GetThemeCOLORREF(ThemeElement::valuePositive);
    COLORREF BackColor = GetThemeCOLORREF(ThemeElement::TradesPanelBack);

    std::wstring DRCR = CustomLabel_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_COMBODRCR));
    if (DRCR == L"DR") {
        fees = fees * -1;
        TextColor = GetThemeCOLORREF(ThemeElement::valueNegative);
    }

    total = (quantity * multiplier * price) + fees;

    CustomTextBox_SetColors(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTTOTAL), TextColor, BackColor);
    CustomTextBox_SetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTTOTAL), std::to_wstring(total));
}


// ========================================================================================
// Calculate Days To Expiration (DTE) and display it in the table
// ========================================================================================
void CalculateTradeDTE(HWND hwnd)
{
    std::wstring transDate = AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TRANSDATE));

    int NumTableRows = TRADEDIALOG_TRADETABLE_NUMROWS;
    if (tradeAction == ACTION_ROLL_LEG) NumTableRows *= 2;

    //for (int i = 0; i < NumTableRows; ++i) {
    //    std::wstring expiryDate = AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEEXPIRY + i));
    //    int days = AfxDaysBetween(transDate, expiryDate);
    //    AfxSetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEDTE + i), std::to_wstring(days) + L"d");
    //}
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

    int HTextMargin = 0;
    int VTextMargin = 3;

    COLORREF lightBackColor = GetThemeCOLORREF(ThemeElement::TradesPanelColBackLight);
    COLORREF lightTextColor = GetThemeCOLORREF(ThemeElement::TradesPanelText);

    COLORREF darkBackColor = GetThemeCOLORREF(ThemeElement::TradesPanelColBackDark);
    COLORREF darkTextColor = GetThemeCOLORREF(ThemeElement::TradesPanelTextDim);

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
            CustomLabelAlignment::MiddleLeft, 40, 30, 65, 22);

        hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTTICKER, ES_LEFT | ES_UPPERCASE, L"", 40, 55, 65, 24);
        CustomTextBox_SetMargins(hCtl, HTextMargin, VTextMargin);
        CustomTextBox_SetColors(hCtl, lightTextColor, darkBackColor);

        CustomLabel_SimpleLabel(hwnd, -1, L"Company Name", TextColorDim, BackColor,
            CustomLabelAlignment::MiddleLeft, 115, 30, 115, 22);

        hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTCOMPANY, ES_LEFT, L"", 115, 55, 252, 24);
        CustomTextBox_SetMargins(hCtl, HTextMargin, VTextMargin);
        CustomTextBox_SetColors(hCtl, lightTextColor, darkBackColor);

    }
    else {

        hCtl = CreateCustomLabel(
            hwnd, IDC_TRADEDIALOG_LBLTICKER, CustomLabelType::TextOnly,
            40, 55, 75, 26);
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
            115, 55, 235, 26);
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

    
    hCtl = CreateCustomTradeGrid(hwnd, IDC_TRADEDIALOG_TABLEGRID, 40, 100, 0, 0);

    nTop = nTop + AfxGetWindowHeight(hCtl);


    nTop = nTop + 20 + vsp;
    nLeft = 40;
    nHeight = 20;

    int nStartTop = nTop;

    
    nWidth = 80;
    CustomLabel_SimpleLabel(hwnd, -1, L"Quantity", TextColorDim, BackColor,
        CustomLabelAlignment::MiddleRight, nLeft, nTop, nWidth, nHeight);
    hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTQUANTITY, ES_RIGHT, 
        L"0", nLeft, nTop + nHeight + vsp, nWidth, nHeight);
    CustomTextBox_SetBorderAttributes(hCtl, 1, BorderColorFocus, BorderColor, CustomTextBoxBorder::BorderUnderline);
    CustomTextBox_SetNumericAttributes(hCtl, 4, CustomTextBoxNegative::Disallow, CustomTextBoxFormatting::Allow);


    nLeft = nLeft + nWidth + hmargin;
    CustomLabel_SimpleLabel(hwnd, -1, L"Multiplier", TextColorDim, BackColor,
        CustomLabelAlignment::MiddleRight, nLeft, nTop, nWidth, nHeight);
    hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTMULTIPLIER, ES_RIGHT, 
        L"100.0000", nLeft, nTop + nHeight + vsp, nWidth, nHeight);
    CustomTextBox_SetBorderAttributes(hCtl, 1, BorderColorFocus, BorderColor, CustomTextBoxBorder::BorderUnderline);
    CustomTextBox_SetNumericAttributes(hCtl, 4, CustomTextBoxNegative::Disallow, CustomTextBoxFormatting::Allow);


    nLeft = nLeft + nWidth + hmargin;
    CustomLabel_SimpleLabel(hwnd, -1, L"Price", TextColorDim, BackColor,
        CustomLabelAlignment::MiddleRight, nLeft, nTop, nWidth, nHeight);
    hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTPRICE, ES_RIGHT,
        L"0", nLeft, nTop + nHeight + vsp, nWidth, nHeight);
    CustomTextBox_SetBorderAttributes(hCtl, 1, BorderColorFocus, BorderColor, CustomTextBoxBorder::BorderUnderline);
    CustomTextBox_SetNumericAttributes(hCtl, 4, CustomTextBoxNegative::Disallow, CustomTextBoxFormatting::Allow);


    nLeft = nLeft + nWidth + hmargin;
    CustomLabel_SimpleLabel(hwnd, -1, L"Fees", TextColorDim, BackColor,
        CustomLabelAlignment::MiddleRight, nLeft, nTop, nWidth, nHeight);
    hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTFEES, ES_RIGHT,
        L"0", nLeft, nTop + nHeight + vsp, nWidth, nHeight);
    CustomTextBox_SetBorderAttributes(hCtl, 1, BorderColorFocus, BorderColor, CustomTextBoxBorder::BorderUnderline);
    CustomTextBox_SetNumericAttributes(hCtl, 4, CustomTextBoxNegative::Disallow, CustomTextBoxFormatting::Allow);


    nLeft = nLeft + nWidth + hmargin;
    CustomLabel_SimpleLabel(hwnd, -1, L"Total", TextColorDim, BackColor,
        CustomLabelAlignment::MiddleRight, nLeft, nTop, nWidth, nHeight);
    // Can not set the Totals as readonly because if we do then we won't be able to get the
    // OnCtlColorEdit message to color the text red/green.
    hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTTOTAL, ES_RIGHT,
        L"0", nLeft, nTop + nHeight + vsp, nWidth, nHeight);
    CustomTextBox_SetBorderAttributes(hCtl, 1, BorderColorFocus, BorderColor, CustomTextBoxBorder::BorderUnderline);
    CustomTextBox_SetNumericAttributes(hCtl, 4, CustomTextBoxNegative::Disallow, CustomTextBoxFormatting::Allow);


    // DR / CR toggle label
    nLeft = nLeft + nWidth + hmargin;
    hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_COMBODRCR, L"", TextColor, BackColor,
        CustomLabelAlignment::MiddleCenter, nLeft, nTop + nHeight + vsp, 30, nHeight);
    TradeDialog_SetComboDRCR(hCtl, L"CR");


    // Position OK and Cancel buttons from the right edge
    nTop = nStartTop + (nHeight + vsp) * 2;
    nLeft = 60;
    hCtl = TradeDialog.AddControl(Controls::Button, hwnd, IDC_TRADEDIALOG_SAVE, L"SAVE", nLeft, nTop, 80, nHeight);

}


