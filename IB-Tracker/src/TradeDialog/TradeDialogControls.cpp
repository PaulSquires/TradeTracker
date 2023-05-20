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
extern TradeAction tradeAction;
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
// Remove pipe character from the string and return new copy. 
// ========================================================================================
std::wstring RemovePipeChar(const std::wstring& wszText)
{
    std::wstring wszString = wszText;
    //wszString.erase(L'|');   // C++20 combines erase/remove into one operation
    wszString.erase(remove(wszString.begin(), wszString.end(), L'|'), wszString.end());
    return wszString;
}


// ========================================================================================
// Perform error checks on the trade data prior to allowing the save to the database.
// ========================================================================================
bool TradeDialog_ValidateTradeData(HWND hwnd)
{

    // Do an error check to ensure that the data about to be saved does not contain
    // any missing data, etc.
    std::wstring wszErrMsg;
    std::wstring wszText;

    if (IsNewTradeAction(tradeAction) == true) {
        wszText = AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTTICKER));
        if (wszText.length() == 0) wszErrMsg += L"- Missing Ticker Symbol.\n";
        wszText = AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTCOMPANY));
        if (wszText.length() == 0) wszErrMsg += L"- Missing Company or Futures Name.\n";
        wszText = AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTDESCRIBE));
        if (wszText.length() == 0) wszErrMsg += L"- Missing Description.\n";
    }

    // In adition to validating each leg, we count the number of non blank legs. If all legs
    // are blank then there is nothing to save to add that to the error message.
    int NumGoodLegs = 0;

    for (int row = 0; row < 4; ++row) {
        std::wstring legQuantity = TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDMAIN), row, 0);
        std::wstring legExpiry = TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDMAIN), row, 1);
        std::wstring legStrike = TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDMAIN), row, 3);
        std::wstring legPutCall = TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDMAIN), row, 4);
        std::wstring legAction = TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDMAIN), row, 5);

        // All strings must be zero length in order to skip it from being included in the transaction. 
        if (legQuantity.length() == 0 && legExpiry.length() == 0 && legStrike.length() == 0
            && legPutCall.length() == 0 && legAction.length() == 0) {
            continue;
        }

        // If any of the strings are zero length at this point then the row has incompete data.
        bool bIncomplete = false;

        if (legQuantity.length() == 0) bIncomplete = true;
        if (legExpiry.length() == 0) bIncomplete = true;
        if (legStrike.length() == 0) bIncomplete = true;
        if (legPutCall.length() == 0) bIncomplete = true;
        if (legAction.length() == 0) bIncomplete = true;

        if (bIncomplete == true) {
            wszErrMsg += L"- Leg #" + std::to_wstring(row + 1) + L" has incomplete or missing data.\n";
        }
        else {
            NumGoodLegs++;
        }
    }

    if (NumGoodLegs == 0) {
        wszErrMsg += L"- No Legs exist to be saved.\n";
    }
        
    if (tradeAction == TradeAction::RollLeg) {
        for (int row = 0; row < 4; ++row) {
            std::wstring legQuantity = TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDROLL), row, 0);
            std::wstring legExpiry = TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDROLL), row, 1);
            std::wstring legStrike = TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDROLL), row, 3);
            std::wstring legPutCall = TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDROLL), row, 4);
            std::wstring legAction = TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDROLL), row, 5);

            // All strings must be zero length in order to skip it from being included in the transaction. 
            if (legQuantity.length() == 0 && legExpiry.length() == 0 && legStrike.length() == 0
                && legPutCall.length() == 0 && legAction.length() == 0) {
                continue;
            }

            // If any of the strings are zero length at this point then the row has incompete data.
            bool bIncomplete = false;

            if (legQuantity.length() == 0) bIncomplete = true;
            if (legExpiry.length() == 0) bIncomplete = true;
            if (legStrike.length() == 0) bIncomplete = true;
            if (legPutCall.length() == 0) bIncomplete = true;
            if (legAction.length() == 0) bIncomplete = true;

            if (bIncomplete == true) {
                wszErrMsg += L"- Roll Leg #" + std::to_wstring(row + 1) + L" has incomplete or missing data.\n";
            }
        }
    }

    if (wszErrMsg.length()) {
        MessageBox(hwnd, wszErrMsg.c_str(), (LPCWSTR)L"Warning", MB_ICONWARNING);
        return false;
    }

    return true;   // data is good, allow the save to continue
}


// ========================================================================================
// Create the trade transaction data and save it to the database
// ========================================================================================
void TradeDialog_CreateTradeData(HWND hwnd)
{
    // PROCEED TO SAVE THE TRADE DATA
    tws_PauseTWS();

    if (tradeAction == TradeAction::RollLeg) {
        AfxSetWindowText(GetDlgItem(HWND_TRADEDIALOG, IDC_TRADEDIALOG_TXTDESCRIBE), L"Roll");
    }
    if (tradeAction == TradeAction::CloseLeg) {
        AfxSetWindowText(GetDlgItem(HWND_TRADEDIALOG, IDC_TRADEDIALOG_TXTDESCRIBE), L"Close");
    }

    std::shared_ptr<Trade> trade;

    if (IsNewTradeAction(tradeAction) == true) {
        trade = std::make_shared<Trade>();
        trades.push_back(trade);
        trade->tickerSymbol = RemovePipeChar(AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTTICKER)));
        trade->tickerName = RemovePipeChar(AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTCOMPANY)));
        trade->futureExpiry = CustomLabel_GetUserData(GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLCONTRACTDATE));
        trade->category = CategoryControl_GetSelectedIndex(GetDlgItem(hwnd, IDC_TRADEDIALOG_CATEGORY));
    }
    else {
        trade = tradeEdit;
    }


    std::shared_ptr<Transaction> trans = std::make_shared<Transaction>();

    trans->transDate = CustomLabel_GetUserData(GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLTRANSDATE));
    trans->description = RemovePipeChar(AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTDESCRIBE)));
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

        case TradeAction::NewTrade:
        case TradeAction::NewShortStrangle:
        case TradeAction::NewShortPut:
        case TradeAction::NewShortCall:
        case TradeAction::AddToTrade:
            leg->origQuantity = intQuantity;
            leg->openQuantity = intQuantity;
            break;

        case TradeAction::CloseLeg:
        case TradeAction::RollLeg:
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


    if (tradeAction == TradeAction::RollLeg) {

        for (int row = 0; row < 4; ++row) {
            std::wstring legQuantity = TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDROLL), row, 0);
            std::wstring legExpiry = TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDROLL), row, 1);
            std::wstring legStrike = TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDROLL), row, 3);
            std::wstring legPutCall = TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDROLL), row, 4);
            std::wstring legAction = TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDROLL), row, 5);

            if (legQuantity.length() == 0) continue;
            int intQuantity = stoi(legQuantity);   // will GPF if empty legQuantity string
            if (intQuantity == 0) continue;

            std::shared_ptr<Leg> leg = std::make_shared<Leg>();

            leg->underlying = trans->underlying;
            leg->expiryDate = legExpiry;
            leg->strikePrice = legStrike;
            leg->PutCall = legPutCall;
            leg->action = legAction;

            leg->origQuantity = intQuantity;
            leg->openQuantity = intQuantity;

            trans->legs.push_back(leg);
        }

    }
        
    // Set the open status of the entire trade based on the new modified legs
    trade->setTradeOpenStatus();

    // Rebuild the openLegs position vector
    trade->createOpenLegsVector();

    // Save the new data
    SaveDatabase();

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

    if (tradeAction == TradeAction::NewTrade) return;

    if (tradeAction == TradeAction::NewIronCondor) {
        hCtl = GetDlgItem(HWND_STRATEGYBUTTON, IDC_STRATEGYBUTTON_LONGSHORT);
        CustomLabel_SetUserDataInt(hCtl, (int)LongShort::Short);
        wszText = AfxUpper(StrategyButton_GetLongShortEnumText(LongShort::Short));
        CustomLabel_SetText(hCtl, wszText);

        hCtl = GetDlgItem(HWND_STRATEGYBUTTON, IDC_STRATEGYBUTTON_PUTCALL);
        CustomLabel_SetText(hCtl, L"");

        hCtl = GetDlgItem(HWND_STRATEGYBUTTON, IDC_STRATEGYBUTTON_STRATEGY);
        CustomLabel_SetUserDataInt(hCtl,(int)Strategy::IronCondor);
        wszText = AfxUpper(StrategyButton_GetStrategyEnumText(Strategy::IronCondor));
        CustomLabel_SetText(hCtl, wszText);

        StrategyButton_InvokeStrategy();
        return;
    }

    if (tradeAction == TradeAction::NewShortStrangle) {
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

    if (tradeAction == TradeAction::NewShortPut) {
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

    if (tradeAction == TradeAction::NewShortCall) {
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

    int row = 0;
    for (const auto& leg : legsEdit) {

        // QUANTITY
        DefaultQuantity = leg->openQuantity;
        std::wstring legQuantity = std::to_wstring(leg->openQuantity * -1);
        TradeGrid_SetColData(hGridMain, row, 0, legQuantity);

        // EXPIRY DATE
        TradeGrid_SetColData(hGridMain, row, 1, leg->expiryDate);

        // STRIKE PRICE
        TradeGrid_SetColData(hGridMain, row, 3, leg->strikePrice);

        // PUT/CALL
        TradeGrid_SetColData(hGridMain, row, 4, leg->PutCall);

        // ACTION
        if (leg->action == L"STO") { wszText = L"BTC"; }
        if (leg->action == L"BTO") { wszText = L"STC"; }
        TradeGrid_SetColData(hGridMain, row, 5, wszText);

        row++;
    }
    
    // DTE
    TradeGrid_CalculateDTE(hGridMain);

    // QUANTITY
    AfxSetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTQUANTITY), std::to_wstring(abs(DefaultQuantity)));


    // Add some default information for the new Roll transaction
    if (tradeAction == TradeAction::RollLeg) {
        HWND hGridRoll = GetDlgItem(HWND_TRADEDIALOG, IDC_TRADEDIALOG_TABLEGRIDROLL);
        TradeGrid* pData = TradeGrid_GetOptions(hGridRoll);
        if (pData == nullptr) return;

        int row = 0;
        for (const auto& leg : legsEdit) {

            // QUANTITY
            std::wstring legQuantity = std::to_wstring(leg->openQuantity);
            TradeGrid_SetColData(hGridRoll, row, 0, legQuantity);

            // EXPIRY DATE
            wszText = AfxDateAddDays(leg->expiryDate, 7);
            TradeGrid_SetColData(hGridRoll, row, 1, wszText);

            // STRIKE PRICE
            TradeGrid_SetColData(hGridRoll, row, 3, leg->strikePrice);

            // PUT/CALL
            TradeGrid_SetColData(hGridRoll, row, 4, leg->PutCall);

            // ACTION
            TradeGrid_SetColData(hGridRoll, row, 5, leg->action);

            row++;
        }

        // DTE
        TradeGrid_CalculateDTE(hGridRoll);

    }
    // Set the DR/CR to debit if this is a closetrade
    if (tradeAction == TradeAction::CloseLeg) {
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
    case TradeAction::NewTrade:
        wszDescription = L"New";
        wszGridMain = L"New Transaction";
        break;
    case TradeAction::CloseLeg:
        wszDescription = L"Close";
        wszGridMain = L"Close Transaction";
        break;
    case TradeAction::ExpireLeg:
        wszDescription = L"Expire";
        break;
    case TradeAction::RollLeg:
        wszDescription = L"Roll";
        wszGridMain = L"Close Transaction";
        wszGridRoll = L"Rolled Transaction";
        break;
    case TradeAction::Assignment:
        wszDescription = L"Assignment";
        break;
    case TradeAction::AddToTrade:
    case TradeAction::AddPutToTrade:
    case TradeAction::AddCallToTrade:
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
    CustomLabel_SetMousePointer(hCtl, CustomLabelPointer::Hand, CustomLabelPointer::Hand);
    CustomLabel_SetUserData(hCtl, wszDate);

    CustomLabel_ButtonLabel(hwnd, IDC_TRADEDIALOG_CMDTRANSDATE, L"\uE015",
        TextColorDim, ThemeElement::GrayMedium, ThemeElement::GrayLight, ThemeElement::GrayMedium,
        CustomLabelAlignment::MiddleCenter, 126, 97, 23, 23);

    // We always want the Description textbox to exists because even for rolled and closed transaction
    // we need to set the description (even though the user will never see the actual textbox in those
    // types of actions).

    nTop = 72;
    CustomLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_LBLDESCRIBE, L"Description", TextColorDim, BackColor,
        CustomLabelAlignment::MiddleLeft, 159, 72, 115, 22);
    hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTDESCRIBE, ES_LEFT, L"", 159, 97, 171, 23);
    CustomTextBox_SetMargins(hCtl, HTextMargin, VTextMargin);
    CustomTextBox_SetColors(hCtl, lightTextColor, darkBackColor);

    if (tradeAction == TradeAction::RollLeg || tradeAction == TradeAction::CloseLeg) {
        ShowWindow(GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLDESCRIBE), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTDESCRIBE), SW_HIDE);
    }

    if (IsNewTradeAction(tradeAction) == true) {
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
    if (tradeAction == TradeAction::RollLeg) {
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
    hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTTOTAL, ES_RIGHT,
        L"0", nLeft, nTop + nHeight + vsp, nWidth, nHeight);
    CustomTextBox_SetMargins(hCtl, HTextMargin, VTextMargin);
    CustomTextBox_SetColors(hCtl, lightTextColor, darkBackColor);
    CustomTextBox_SetNumericAttributes(hCtl, 4, CustomTextBoxNegative::Disallow, CustomTextBoxFormatting::Allow);


    std::wstring wszFontName = L"Segoe UI";
    int FontSize = 8;
    bool bold = false;

    // DR / CR toggle label
    nLeft = nLeft + nWidth + hmargin;
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_TRADEDIALOG_COMBODRCR, L"CR",
        ThemeElement::Black, ThemeElement::Green, ThemeElement::Green, ThemeElement::GrayMedium,
        CustomLabelAlignment::MiddleCenter, nLeft, nTop + nHeight + vsp, 30, nHeight);
    CustomLabel_SetFont(hCtl, wszFontName, FontSize, true);
    CustomLabel_SetTextColorHot(hCtl, ThemeElement::WhiteLight);
    TradeDialog_SetComboDRCR(hCtl, L"CR");


    // SAVE button
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_TRADEDIALOG_SAVE, L"Save",
        ThemeElement::Black, ThemeElement::Green, ThemeElement::Green, ThemeElement::GrayMedium,
        CustomLabelAlignment::MiddleCenter, 580, nTop + nHeight + vsp, 80, 23);
    CustomLabel_SetFont(hCtl, wszFontName, FontSize, true);
    CustomLabel_SetTextColorHot(hCtl, ThemeElement::WhiteLight);


    TradeDialogControls_GetTradeDescription(hwnd);


    // CATEGORY SELECTOR
    if (IsNewTradeAction(tradeAction) == true) {
        CreateCategoryControl(hwnd, IDC_TRADEDIALOG_CATEGORY, 540, 45, 124, 23);
    }
}

