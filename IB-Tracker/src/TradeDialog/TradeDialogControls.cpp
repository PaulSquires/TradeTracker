#include "pch.h"

#include "TradeDialog.h"
#include "..\Utilities\ListBoxData.h"
#include "..\Utilities\AfxWin.h"
#include "..\CustomLabel\CustomLabel.h"
#include "..\CustomTextBox\CustomTextBox.h"
#include "..\TradeGrid\TradeGrid.h"
#include "..\Strategy\StrategyButton.h"
#include "..\TradesPanel\TradesPanel.h"
#include "..\MainWindow\tws-client.h"
#include "..\Database\database.h"
#include "..\Category\Category.h"


extern HWND HWND_TRADEDIALOG;
extern CTradeDialog TradeDialog;
extern int tradeAction;
extern std::vector<std::shared_ptr<Leg>> legsEdit;
extern std::shared_ptr<Trade> tradeEdit;

extern void TradesPanel_ShowActiveTrades();

CStrategyButton StrategyButton;
extern HWND HWND_STRATEGYBUTTON;


// ========================================================================================
// Display or hide the Futures Contract data picker
// ========================================================================================
void TradeDialogControls_ShowFuturesContractDate(HWND hwnd)
{
    HWND hCtl1 = GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLCONTRACT);
    HWND hCtl2 = GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLCONTRACTDATE);
    HWND hCtl3 = GetDlgItem(hwnd, IDC_TRADEDIALOG_CMDCONTRACTDATE);

    // Futures Ticker symbols will start with a forward slash character.
    std::wstring wszTicker = AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTTICKER));
    int nShow = (wszTicker.substr(0, 1) == L"/") ? SW_SHOW : SW_HIDE;

    ShowWindow(hCtl1, nShow);
    ShowWindow(hCtl2, nShow);
    ShowWindow(hCtl3, nShow);

    if (nShow == SW_SHOW) {
        CustomLabel_SetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLCOMPANY), L"Futures Contract");
    } else {
        CustomLabel_SetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLCOMPANY), L"Company Name");
    }
}


// ========================================================================================
// Create the trade transaction data and save it to the database
// ========================================================================================
void TradeDialog_CreateTradeData(HWND hwnd)
{
    tws_PauseTWS();

    std::shared_ptr<Trade> trade;

    if (IsNewTradeAction(tradeAction) == true) {
        auto trade = std::make_shared<Trade>();
        trades.push_back(trade);
        trade->tickerSymbol = AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTTICKER));
        trade->tickerName = AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTCOMPANY));
        trade->futureExpiry = CustomLabel_GetUserData(GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLCONTRACTDATE));
    }
    else {
        trade = tradeEdit;
    }


    std::shared_ptr<Transaction> trans = std::make_shared<Transaction>();

    trans->transDate = CustomLabel_GetUserData(GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLTRANSDATE));
    trans->description = L"Strangle";  // RemovePipeChar(ui->txtDescription->text());
    trans->underlying = L"OPTIONS";
    trans->quantity = stoi(AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTQUANTITY)));
    trans->price = stod(AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTPRICE)));
    trans->multiplier = stod(AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTMULTIPLIER)));
    trans->fees = stod(AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTFEES)));
    trade->transactions.push_back(trans);

    std::wstring DRCR = CustomLabel_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_COMBODRCR));
    trans->total = stod(AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTTOTAL)));
    if (DRCR == L"DR") { trans->total = trans->total * -1; }
    trade->ACB = trade->ACB + trans->total;


    for (int row = 0; row < 4; ++row) {
        std::wstring legQuantity = TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDMAIN), row, 0);
        std::wstring legExpiry = TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDMAIN), row, 1);
        std::wstring legStrike = TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDMAIN), row, 3);
        std::wstring legPutCall = TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDMAIN), row, 4);
        std::wstring legAction = TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDMAIN), row, 5);

        if (legQuantity.length() == 0) continue;
        int intQuantity = stoi(legQuantity);   // will GPF if empty legQuantity string
        if (intQuantity == 0) continue;

        std::shared_ptr<Leg> leg = std::make_shared<Leg>();

        leg->underlying = trans->underlying;
        leg->expiryDate = legExpiry;
        leg->strikePrice = legStrike;
        leg->PutCall = legPutCall;
        leg->action = legAction;


        switch (tradeAction) {

        case ACTION_NEW_TRADE:
        case ACTION_NEW_SHORTSTRANGLE:
        case ACTION_NEW_SHORTPUT:
        case ACTION_NEW_SHORTCALL:
        case ACTION_ADDTO_TRADE:
            leg->origQuantity = intQuantity;
            leg->openQuantity = intQuantity;
            break;

        case ACTION_ROLL_LEG:
            // If this is a rolled trade then all table rows representing the old legs to roll will be
            // displayed first in the table. Those rows are dealt with after this new transaction
            // is created so we only now need to be concered with the table rows beyond the first
            // number of rolled legs.
/*
            if (i < legsEdit.count()) {
                // Update the original transaction being Closed/Expired quantities
                if (!legsEdit.empty()) {
                    if (i < legsEdit.size()) {
                        legsEdit.at(i)->openQuantity = legsEdit.at(i)->openQuantity + quantity.toInt();
                    }
                }

                leg->origQuantity = quantity.toInt();
                leg->openQuantity = 0;
            }
            else {
                leg->origQuantity = quantity.toInt();
                leg->openQuantity = quantity.toInt();
            }
*/
            break;


        case ACTION_CLOSE_LEG:
            // Save this transaction's leg quantities
            leg->origQuantity = intQuantity;
            leg->openQuantity = 0;

            // Update the original transaction being Closed quantities
            if (!legsEdit.empty()) {
                legsEdit.at(row)->openQuantity = legsEdit.at(row)->openQuantity + intQuantity;
            }

            break;
        }

        trans->legs.push_back(leg);

    }

    // Save the new data
    SaveDatabase();
        
    // Set the open status of the entire trade based on the new modified legs
    trade->setTradeOpenStatus();

    // Rebuild the openLegs position vector
    trade->createOpenLegsVector();

    // Show our new list of open trades
    TradesPanel_ShowActiveTrades();

    tws_ResumeTWS();

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

    COLORREF TextColor = GetThemeCOLORREF(ThemeElement::Red);
    COLORREF BackColor = GetThemeCOLORREF(ThemeElement::GrayMedium);

    std::wstring DRCR = CustomLabel_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_COMBODRCR));
    if (DRCR == L"CR") {
        fees = fees * -1;
        TextColor = GetThemeCOLORREF(ThemeElement::Green);
    }

    total = (quantity * multiplier * price) + fees;

    CustomTextBox_SetColors(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTTOTAL), TextColor, BackColor);
    CustomTextBox_SetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTTOTAL), std::to_wstring(total));
}



// ========================================================================================
// Load the legs for the edit Action into the Trade Management table
// ========================================================================================
void LoadEditLegsInTradeTable(HWND hwnd)
{
    HWND hCtl = NULL;
    std::wstring wszText;

    if (tradeAction == ACTION_NEW_TRADE) return;

    if (tradeAction == ACTION_NEW_SHORTSTRANGLE) {
        hCtl = GetDlgItem(HWND_STRATEGYBUTTON, IDC_STRATEGYBUTTON_LONGSHORT);
        CustomLabel_SetUserDataInt(hCtl, (int)LongShort::Short);
        wszText = AfxUpper(StrategyButton_GetLongShortEnumText(LongShort::Short));
        CustomLabel_SetText(hCtl, wszText);

        hCtl = GetDlgItem(HWND_STRATEGYBUTTON, IDC_STRATEGYBUTTON_PUTCALL);
        CustomLabel_SetText(hCtl, L"");

        hCtl = GetDlgItem(HWND_STRATEGYBUTTON, IDC_STRATEGYBUTTON_STRATEGY);
        CustomLabel_SetUserDataInt(hCtl,(int)Strategy::Strangle);
        wszText = AfxUpper(StrategyButton_GetStrategyEnumText(Strategy::Strangle));
        CustomLabel_SetText(hCtl, wszText);

        StrategyButton_InvokeStrategy();
        return;
    }

    if (tradeAction == ACTION_NEW_SHORTPUT) {
        hCtl = GetDlgItem(HWND_STRATEGYBUTTON, IDC_STRATEGYBUTTON_LONGSHORT);
        CustomLabel_SetUserDataInt(hCtl, (int)LongShort::Short);
        wszText = AfxUpper(StrategyButton_GetLongShortEnumText(LongShort::Short));
        CustomLabel_SetText(hCtl, wszText);

        hCtl = GetDlgItem(HWND_STRATEGYBUTTON, IDC_STRATEGYBUTTON_PUTCALL);
        CustomLabel_SetUserDataInt(hCtl, (int)PutCall::Put);
        wszText = AfxUpper(StrategyButton_GetPutCallEnumText(PutCall::Put));
        CustomLabel_SetText(hCtl, wszText);

        hCtl = GetDlgItem(HWND_STRATEGYBUTTON, IDC_STRATEGYBUTTON_STRATEGY);
        CustomLabel_SetUserDataInt(hCtl, (int)Strategy::Option);
        wszText = AfxUpper(StrategyButton_GetStrategyEnumText(Strategy::Option));
        CustomLabel_SetText(hCtl, wszText);

        StrategyButton_InvokeStrategy();
        return;
    }

    if (tradeAction == ACTION_NEW_SHORTCALL) {
        hCtl = GetDlgItem(HWND_STRATEGYBUTTON, IDC_STRATEGYBUTTON_LONGSHORT);
        CustomLabel_SetUserDataInt(hCtl, (int)LongShort::Short);
        wszText = AfxUpper(StrategyButton_GetLongShortEnumText(LongShort::Short));
        CustomLabel_SetText(hCtl, wszText);

        hCtl = GetDlgItem(HWND_STRATEGYBUTTON, IDC_STRATEGYBUTTON_PUTCALL);
        CustomLabel_SetUserDataInt(hCtl, (int)PutCall::Call);
        wszText = AfxUpper(StrategyButton_GetPutCallEnumText(PutCall::Call));
        CustomLabel_SetText(hCtl, wszText);

        hCtl = GetDlgItem(HWND_STRATEGYBUTTON, IDC_STRATEGYBUTTON_STRATEGY);
        CustomLabel_SetUserDataInt(hCtl, (int)Strategy::Option);
        wszText = AfxUpper(StrategyButton_GetStrategyEnumText(Strategy::Option));
        CustomLabel_SetText(hCtl, wszText);

        StrategyButton_InvokeStrategy();
        return;
    }

    // Update the Trade Management table with the details of the Trade.
    CustomLabel_SetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLCOMPANY), tradeEdit->tickerName);
    
    wszText = tradeEdit->tickerSymbol;
    if (tradeEdit->futureExpiry.length()) {
        wszText = wszText + L": " + AfxFormatFuturesDate(tradeEdit->futureExpiry);
    }
    CustomLabel_SetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLTICKER), wszText);


    if (legsEdit.size() == 0) return;

    int DefaultQuantity = 0;


    // Display the legs being closed and set each to the needed inverse action.
    HWND hGridMain = GetDlgItem(HWND_TRADEDIALOG, IDC_TRADEDIALOG_TABLEGRIDMAIN);
    TradeGrid* pData = TradeGrid_GetOptions(hGridMain);
    if (pData == nullptr) return;

    int nextRow = 0;
    for (const auto& leg : legsEdit) {
        int colIdx = (nextRow * 7);

        // QUANTITY
        DefaultQuantity = leg->openQuantity;
        std::wstring legQuantity = std::to_wstring(leg->openQuantity * -1);
        TradeGrid_SetText(pData->gridCols.at(colIdx), legQuantity);

        // EXPIRY DATE
        colIdx++;
        CustomLabel_SetUserData(pData->gridCols.at(colIdx)->hCtl, leg->expiryDate);
        TradeGrid_SetText(pData->gridCols.at(colIdx), AfxShortDate(leg->expiryDate));

        // STRIKE PRICE
        colIdx++;
        TradeGrid_SetText(pData->gridCols.at(colIdx), leg->strikePrice);

        // PUT/CALL
        colIdx++;
        TradeGrid_SetText(pData->gridCols.at(colIdx), leg->PutCall);

        // ACTION
        colIdx++;
        TradeGrid_SetText(pData->gridCols.at(colIdx), leg->action);

        nextRow++;
    }
    
    // DTE
    //CalculateTradeDTE(hwnd);

    // QUANTITY
    AfxSetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTQUANTITY), std::to_wstring(abs(DefaultQuantity)));
    //FormatNumberFourDecimals(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTQUANTITY));


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


    // Set the DR/CR to debit if this is a closetrade
    if (tradeAction == ACTION_CLOSE_LEG) {
        TradeDialog_SetComboDRCR(GetDlgItem(HWND_TRADEDIALOG, IDC_TRADEDIALOG_COMBODRCR), L"DR");
    }


}



// ========================================================================================
// Get the description for the type of action being performed.
// Also sets the descriptin labels above the Table main grid and Table roll grid.
// Also sets the Action label (in uppercase)
// ========================================================================================
std::wstring TradeDialogControls_GetTradeDescription(HWND hwnd)
{
    std::wstring wszDescription;
    std::wstring wszGridMain;
    std::wstring wszGridRoll;

    switch (tradeAction)
    {
    case ACTION_NEW_TRADE:
        wszDescription = L"New";
        wszGridMain = L"New Transaction";
        break;
    case ACTION_CLOSE_LEG:
        wszDescription = L"Close";
        wszGridMain = L"Close Transaction";
        break;
    case ACTION_EXPIRE_LEG:
        wszDescription = L"Expire";
        break;
    case ACTION_ROLL_LEG:
        wszDescription = L"Roll";
        wszGridMain = L"Close Transaction";
        wszGridRoll = L"Rolled Transaction";
        break;
    case ACTION_SHARE_ASSIGNMENT:
        wszDescription = L"Assignment";
        break;
    case ACTION_ADDTO_TRADE:
    case ACTION_ADDPUTTO_TRADE:
    case ACTION_ADDCALLTO_TRADE:
        wszDescription = L"Add To";
        wszGridMain = L"New Transaction";
        break;
    }

    CustomLabel_SetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLEDITACTION), AfxUpper(wszDescription));
    CustomLabel_SetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLGRIDMAIN), wszGridMain);
    CustomLabel_SetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLGRIDROLL), wszGridRoll);

    return wszDescription;
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

    COLORREF lightBackColor = GetThemeCOLORREF(ThemeElement::GrayLight);
    COLORREF lightTextColor = GetThemeCOLORREF(ThemeElement::WhiteLight);

    COLORREF darkBackColor = GetThemeCOLORREF(ThemeElement::GrayMedium);
    COLORREF darkTextColor = GetThemeCOLORREF(ThemeElement::WhiteDark);

    ThemeElement TextColor = ThemeElement::WhiteLight;
    ThemeElement TextColorDim = ThemeElement::WhiteDark;
    ThemeElement BackColor = ThemeElement::GrayDark;


    // EDIT ACTION LABEL
    hCtl = CreateCustomLabel(
        hwnd, IDC_TRADEDIALOG_LBLEDITACTION, CustomLabelType::TextOnly,
        575, 10, 90, 20);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->BackColor = ThemeElement::GrayDark;
        pData->BackColorButtonDown = ThemeElement::GrayDark;
        pData->TextColor = ThemeElement::WhiteLight;
        pData->FontSize = 12;
        pData->FontBold = true;
        pData->TextAlignment = CustomLabelAlignment::TopRight;
        CustomLabel_SetOptions(hCtl, pData);
    }


    // NEW TRADE SHOWS TEXTBOXES, OTHERS JUST LABELS
    if (IsNewTradeAction(tradeAction) == true) {
        CustomLabel_SimpleLabel(hwnd, -1, L"Ticker", TextColorDim, BackColor,
            CustomLabelAlignment::MiddleLeft, 40, 20, 65, 22);
        hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTTICKER, ES_LEFT | ES_UPPERCASE, L"", 40, 45, 65, 23);
        CustomTextBox_SetMargins(hCtl, HTextMargin, VTextMargin);
        CustomTextBox_SetColors(hCtl, lightTextColor, darkBackColor);

        CustomLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_LBLCOMPANY, L"Company Name", TextColorDim, BackColor,
            CustomLabelAlignment::MiddleLeft, 115, 20, 115, 22);
        hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTCOMPANY, ES_LEFT, L"", 115, 45, 215, 23);
        CustomTextBox_SetMargins(hCtl, HTextMargin, VTextMargin);
        CustomTextBox_SetColors(hCtl, lightTextColor, darkBackColor);

        hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_LBLCONTRACT, L"Contract Expiry", TextColorDim, BackColor,
            CustomLabelAlignment::MiddleLeft, 340, 20, 120, 22);
        ShowWindow(hCtl, SW_HIDE);
        std::wstring wszContractDate = AfxCurrentDate();
        CustomLabel_SetUserData(hCtl, wszContractDate);
        hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_LBLCONTRACTDATE, AfxLongDate(wszContractDate), 
            TextColor, ThemeElement::GrayMedium, CustomLabelAlignment::MiddleLeft, 340, 45, 86, 23);
        ShowWindow(hCtl, SW_HIDE);
        hCtl = CustomLabel_ButtonLabel(hwnd, IDC_TRADEDIALOG_CMDCONTRACTDATE, L"\uE015",
            TextColorDim, ThemeElement::GrayMedium, ThemeElement::GrayLight, ThemeElement::GrayMedium,
            CustomLabelAlignment::MiddleCenter, 426, 45, 23, 23);
        ShowWindow(hCtl, SW_HIDE);

    }
    else {
        hCtl = CreateCustomLabel(
            hwnd, IDC_TRADEDIALOG_LBLCOMPANY, CustomLabelType::TextOnly,
            40, 10, 250, 22);
        pData = CustomLabel_GetOptions(hCtl);
        if (pData) {
            pData->BackColor = ThemeElement::GrayDark;
            pData->BackColorButtonDown = ThemeElement::GrayDark;
            pData->TextColor = ThemeElement::WhiteLight;
            pData->FontSize = 12;
            pData->TextAlignment = CustomLabelAlignment::TopLeft;
            pData->wszText = L"";
            CustomLabel_SetOptions(hCtl, pData);
        }

        hCtl = CreateCustomLabel(
            hwnd, IDC_TRADEDIALOG_LBLTICKER, CustomLabelType::TextOnly,
            40, 33, 250, 22);
        pData = CustomLabel_GetOptions(hCtl);
        if (pData) {
            pData->BackColor = ThemeElement::GrayDark;
            pData->BackColorButtonDown = ThemeElement::GrayDark;
            pData->TextColor = ThemeElement::WhiteDark;
            pData->FontSize = 10;
            pData->TextAlignment = CustomLabelAlignment::TopLeft;
            pData->wszText = L"";
            CustomLabel_SetOptions(hCtl, pData);
        }
    }


    nTop = 72;
    CustomLabel_SimpleLabel(hwnd, -1, L"Date", TextColorDim, BackColor,
        CustomLabelAlignment::MiddleLeft, 40, nTop, 100, 22);
    std::wstring wszDate = AfxCurrentDate();
    hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_LBLTRANSDATE, AfxLongDate(wszDate), TextColor, ThemeElement::GrayMedium,
        CustomLabelAlignment::MiddleLeft, 40, 97, 86, 23);
    CustomLabel_SetUserData(hCtl, wszDate);

    CustomLabel_ButtonLabel(hwnd, IDC_TRADEDIALOG_CMDTRANSDATE, L"\uE015",
        TextColorDim, ThemeElement::GrayMedium, ThemeElement::GrayLight, ThemeElement::GrayMedium,
        CustomLabelAlignment::MiddleCenter, 126, 97, 23, 23);


    if (IsNewTradeAction(tradeAction) == true) {
        nTop = 72;
        CustomLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_LBLDESCRIBE, L"Description", TextColorDim, BackColor,
            CustomLabelAlignment::MiddleLeft, 159, 72, 115, 22);
        hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTDESCRIBE, ES_LEFT, L"", 159, 97, 171, 23);
        CustomTextBox_SetMargins(hCtl, HTextMargin, VTextMargin);
        CustomTextBox_SetColors(hCtl, lightTextColor, darkBackColor);

        CustomLabel_SimpleLabel(hwnd, -1, L"Strategy", TextColorDim, BackColor,
            CustomLabelAlignment::MiddleLeft, 340, 72, 100, 22);
        hCtl = StrategyButton.Create(hwnd, L"", 340, 97, 264, 23,
            WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CONTROLPARENT);

    }



    // Create the main trade grid
    nTop = 155;
    CustomLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_LBLGRIDMAIN, L"", TextColorDim, BackColor,
        CustomLabelAlignment::MiddleLeft, 40, nTop, 300, 22);
    HWND hGridMain = CreateTradeGrid(hwnd, IDC_TRADEDIALOG_TABLEGRIDMAIN, 40, nTop + 25, 0, 0);

    // If we are rolling then create the second trade grid.
    if (tradeAction == ACTION_ROLL_LEG) {
        nWidth = AfxUnScaleX((float)AfxGetWindowWidth(hGridMain)) + 20;
        CustomLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_LBLGRIDROLL, L"", TextColorDim, BackColor,
            CustomLabelAlignment::MiddleLeft, 40 + nWidth, nTop, 300, 22);
        HWND hGridRoll = CreateTradeGrid(hwnd, IDC_TRADEDIALOG_TABLEGRIDROLL, 40 + nWidth, nTop + 25, 0, 0);
    }
    nTop += 25;


    nTop += AfxUnScaleY((float)AfxGetWindowHeight(hGridMain)) + 30;
    nLeft = 40;
    nHeight = 23;

    int nStartTop = nTop;


    nWidth = 80;
    CustomLabel_SimpleLabel(hwnd, -1, L"Quantity", TextColorDim, BackColor,
        CustomLabelAlignment::MiddleRight, nLeft, nTop, nWidth, nHeight);
    hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTQUANTITY, ES_RIGHT,
        L"0", nLeft, nTop + nHeight + vsp, nWidth, nHeight);
    CustomTextBox_SetMargins(hCtl, HTextMargin, VTextMargin);
    CustomTextBox_SetColors(hCtl, lightTextColor, darkBackColor);
    CustomTextBox_SetNumericAttributes(hCtl, 4, CustomTextBoxNegative::Disallow, CustomTextBoxFormatting::Allow);


    nLeft = nLeft + nWidth + hmargin;
    CustomLabel_SimpleLabel(hwnd, -1, L"Multiplier", TextColorDim, BackColor,
        CustomLabelAlignment::MiddleRight, nLeft, nTop, nWidth, nHeight);
    hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTMULTIPLIER, ES_RIGHT,
        L"100.0000", nLeft, nTop + nHeight + vsp, nWidth, nHeight);
    CustomTextBox_SetMargins(hCtl, HTextMargin, VTextMargin);
    CustomTextBox_SetColors(hCtl, lightTextColor, darkBackColor);
    CustomTextBox_SetNumericAttributes(hCtl, 4, CustomTextBoxNegative::Disallow, CustomTextBoxFormatting::Allow);


    nLeft = nLeft + nWidth + hmargin;
    CustomLabel_SimpleLabel(hwnd, -1, L"Price", TextColorDim, BackColor,
        CustomLabelAlignment::MiddleRight, nLeft, nTop, nWidth, nHeight);
    hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTPRICE, ES_RIGHT,
        L"0", nLeft, nTop + nHeight + vsp, nWidth, nHeight);
    CustomTextBox_SetMargins(hCtl, HTextMargin, VTextMargin);
    CustomTextBox_SetColors(hCtl, lightTextColor, darkBackColor);
    CustomTextBox_SetNumericAttributes(hCtl, 4, CustomTextBoxNegative::Disallow, CustomTextBoxFormatting::Allow);


    nLeft = nLeft + nWidth + hmargin;
    CustomLabel_SimpleLabel(hwnd, -1, L"Fees", TextColorDim, BackColor,
        CustomLabelAlignment::MiddleRight, nLeft, nTop, nWidth, nHeight);
    hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTFEES, ES_RIGHT,
        L"0", nLeft, nTop + nHeight + vsp, nWidth, nHeight);
    CustomTextBox_SetMargins(hCtl, HTextMargin, VTextMargin);
    CustomTextBox_SetColors(hCtl, lightTextColor, darkBackColor);
    CustomTextBox_SetNumericAttributes(hCtl, 4, CustomTextBoxNegative::Disallow, CustomTextBoxFormatting::Allow);


    nLeft = nLeft + nWidth + hmargin;
    CustomLabel_SimpleLabel(hwnd, -1, L"Total", TextColorDim, BackColor,
        CustomLabelAlignment::MiddleRight, nLeft, nTop, nWidth, nHeight);
    // Can not set the Totals as readonly because if we do then we won't be able to get the
    // OnCtlColorEdit message to color the text red/green.
    hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTTOTAL, ES_RIGHT,
        L"0", nLeft, nTop + nHeight + vsp, nWidth, nHeight);
    CustomTextBox_SetMargins(hCtl, HTextMargin, VTextMargin);
    CustomTextBox_SetColors(hCtl, lightTextColor, darkBackColor);
    CustomTextBox_SetNumericAttributes(hCtl, 4, CustomTextBoxNegative::Disallow, CustomTextBoxFormatting::Allow);


    // DR / CR toggle label
    nLeft = nLeft + nWidth + hmargin;
    hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_COMBODRCR, L"", TextColor, BackColor,
        CustomLabelAlignment::MiddleCenter, nLeft, nTop + nHeight + vsp, 30, nHeight);
    TradeDialog_SetComboDRCR(hCtl, L"CR");


    // SAVE button
    CustomLabel_ButtonLabel(hwnd, IDC_TRADEDIALOG_SAVE, L"Save",
        ThemeElement::GrayLight, ThemeElement::Green, ThemeElement::Magenta, ThemeElement::Red,
        CustomLabelAlignment::MiddleCenter, 580, nTop + nHeight + vsp, 80, 23);

    TradeDialogControls_GetTradeDescription(hwnd);


    // CATEGORY SELECTOR
    if (IsNewTradeAction(tradeAction) == true) {
        CreateCategoryControl(hwnd, IDC_TRADEDIALOG_CATEGORY, 540, 45, 124, 23);
    }
}

