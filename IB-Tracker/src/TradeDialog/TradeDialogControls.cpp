#include "pch.h"

#include "TradeDialog.h"
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
extern std::wstring sharesAggregateEdit;

extern void TradesPanel_ShowActiveTrades();

CStrategyButton StrategyButton;
extern HWND HWND_STRATEGYBUTTON;


// ========================================================================================
// Set the Short/Long background color.
// ========================================================================================
void TradeDialog_SetLongShortBackColor(HWND hCtl)
{
    LongShort ls = (LongShort)CustomLabel_GetUserDataInt(hCtl);

    if (ls == LongShort::Long) {
        CustomLabel_SetBackColor(hCtl, ThemeElement::Green);
    }
    else if (ls == LongShort::Short) {
        CustomLabel_SetBackColor(hCtl, ThemeElement::Red);
    }
}


// ========================================================================================
// Toggle the BUY Short/Long Shares/Futures text.
// ========================================================================================
void TradeDialog_ToggleBuyLongShortText(HWND hCtl)
{
    int sel = CustomLabel_GetUserDataInt(hCtl) + 1;
    if (sel == (int)LongShort::Count) sel = 0;

    CustomLabel_SetUserDataInt(hCtl, sel);

    std::wstring wszText;

    switch ((LongShort)sel)
    {
    case LongShort::Long:
        wszText = L"BUY LONG ";
        TradeDialog_SetComboDRCR(GetDlgItem(HWND_TRADEDIALOG, IDC_TRADEDIALOG_COMBODRCR), L"DR");
        break;
    case LongShort::Short:
        wszText = L"SELL SHORT ";
        TradeDialog_SetComboDRCR(GetDlgItem(HWND_TRADEDIALOG, IDC_TRADEDIALOG_COMBODRCR), L"CR");
        break;
    default:
        wszText = L"";
    }

    if (tradeAction == TradeAction::NewSharesTrade) wszText += L"SHARES";
    if (tradeAction == TradeAction::NewFuturesTrade) wszText += L"FUTURES";
    CustomLabel_SetText(hCtl, wszText);
}


// ========================================================================================
// Toggle the SELL Short/Long Shares/Futures text.
// ========================================================================================
void TradeDialog_ToggleSellLongShortText(HWND hCtl)
{
    int sel = CustomLabel_GetUserDataInt(hCtl) + 1;
    if (sel == (int)LongShort::Count) sel = 0;

    CustomLabel_SetUserDataInt(hCtl, sel);

    std::wstring wszText;

    switch ((LongShort)sel)
    {
    case LongShort::Short:
        wszText = L"SELL LONG ";
        TradeDialog_SetComboDRCR(GetDlgItem(HWND_TRADEDIALOG, IDC_TRADEDIALOG_COMBODRCR), L"CR");
        break;
    case LongShort::Long:
        wszText = L"BUY SHORT ";
        TradeDialog_SetComboDRCR(GetDlgItem(HWND_TRADEDIALOG, IDC_TRADEDIALOG_COMBODRCR), L"DR");
        break;
    default:
        wszText = L"";
    }

    if (tradeAction == TradeAction::ManageShares) wszText += L"SHARES";
    if (tradeAction == TradeAction::ManageFutures) wszText += L"FUTURES";
    CustomLabel_SetText(hCtl, wszText);
}


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
// Helper function to calculate and update the Total TextBox
// ========================================================================================
void TradeDialog_CalculateTradeTotal(HWND hwnd)
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
void TradeDialog_LoadEditLegsInTradeTable(HWND hwnd)
{
    HWND hCtl = NULL;
    std::wstring wszText;

    if (tradeAction == TradeAction::NewOptionsTrade) return;
    if (tradeAction == TradeAction::NewSharesTrade) return;
    if (tradeAction == TradeAction::NewFuturesTrade) return;

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

        // Only load a maximum of 4 legs even if the user had selected more than 4.
        if (row > 3) break;

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

            // Only load a maximum of 4 legs even if the user had selected more than 4.
            if (row > 3) break;

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
    case TradeAction::NewOptionsTrade:
    case TradeAction::NewShortStrangle:
    case TradeAction::NewShortCall:
    case TradeAction::NewIronCondor:
    case TradeAction::NewShortPut:
        wszDescription = L"Options";
        wszGridMain = L"New Transaction";
        break;
    case TradeAction::NewSharesTrade:
    case TradeAction::ManageShares:
        wszDescription = L"Shares";
        wszGridMain = L"New Transaction";
        break;
    case TradeAction::NewFuturesTrade:
    case TradeAction::ManageFutures:
        wszDescription = L"Futures";
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
    case TradeAction::AddOptionsToTrade:
    case TradeAction::AddPutToTrade:
    case TradeAction::AddCallToTrade:
        wszDescription = L"Add Options";
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
        545, 10, 120, 20);
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
    if (IsNewOptionsTradeAction(tradeAction) == true ||
        IsNewSharesTradeAction(tradeAction) == true) {
        CustomLabel_SimpleLabel(hwnd, -1, L"Ticker", TextColorDim, BackColor,
            CustomLabelAlignment::MiddleLeft, 40, 20, 65, 22);
        hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTTICKER, ES_LEFT | ES_UPPERCASE, L"", 40, 45, 65, 23);
        CustomTextBox_SetMargins(hCtl, HTextMargin, VTextMargin);
        CustomTextBox_SetColors(hCtl, lightTextColor, darkBackColor);

        hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_LBLCOMPANY, L"Company Name", TextColorDim, BackColor,
            CustomLabelAlignment::MiddleLeft, 115, 20, 115, 22);
        if (tradeAction == TradeAction::NewFuturesTrade) {
            CustomLabel_SetText(hCtl, L"Futures Contract");
        }
        hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTCOMPANY, ES_LEFT, L"", 115, 45, 215, 23);
        CustomTextBox_SetMargins(hCtl, HTextMargin, VTextMargin);
        CustomTextBox_SetColors(hCtl, lightTextColor, darkBackColor);

        hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_LBLCONTRACT, L"Contract Expiry", TextColorDim, BackColor,
            CustomLabelAlignment::MiddleLeft, 340, 20, 120, 22);
        if (tradeAction != TradeAction::NewFuturesTrade) ShowWindow(hCtl, SW_HIDE);
        std::wstring wszContractDate = AfxCurrentDate();
        CustomLabel_SetUserData(hCtl, wszContractDate);
        hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_LBLCONTRACTDATE, AfxLongDate(wszContractDate),
            TextColor, ThemeElement::GrayMedium, CustomLabelAlignment::MiddleLeft, 340, 45, 86, 23);
        if (tradeAction != TradeAction::NewFuturesTrade) ShowWindow(hCtl, SW_HIDE);
        hCtl = CustomLabel_ButtonLabel(hwnd, IDC_TRADEDIALOG_CMDCONTRACTDATE, L"\uE015",
            TextColorDim, ThemeElement::GrayMedium, ThemeElement::GrayLight, ThemeElement::GrayMedium,
            CustomLabelAlignment::MiddleCenter, 426, 45, 23, 23);
        if (tradeAction != TradeAction::NewFuturesTrade) ShowWindow(hCtl, SW_HIDE);

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

    if (IsNewOptionsTradeAction(tradeAction) == false ||
        IsNewSharesTradeAction(tradeAction) == true) {
        ShowWindow(GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLDESCRIBE), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTDESCRIBE), SW_HIDE);
    }

    if (IsNewOptionsTradeAction(tradeAction) == true) {
        // However do not show it for Shares/Futures 
        CustomLabel_SimpleLabel(hwnd, -1, L"Strategy", TextColorDim, BackColor,
            CustomLabelAlignment::MiddleLeft, 340, 72, 100, 22);
        hCtl = StrategyButton.Create(hwnd, L"", 340, 97, 264, 23,
            WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CONTROLPARENT);
    }


    nTop = 155;

    CustomLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_LBLGRIDMAIN, L"", TextColorDim, BackColor,
        CustomLabelAlignment::MiddleLeft, 40, nTop, 300, 22);

    if (IsNewSharesTradeAction(tradeAction) == true) {
        std::wstring wszFontName = L"Segoe UI";
        std::wstring wszText;
        int FontSize = 8;

        hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_BUYSHARES, L"",
            ThemeElement::WhiteLight, ThemeElement::GrayMedium,
            CustomLabelAlignment::MiddleLeft, 40, nTop + 25, 150, 23);
        CustomLabel_SetUserDataInt(hCtl, (int)LongShort::Long);
        CustomLabel_SetFont(hCtl, wszFontName, FontSize, true);
        CustomLabel_SetTextOffset(hCtl, 5, 0);
        TradeDialog_SetLongShortBackColor(hCtl);
        CustomLabel_SetMousePointer(hCtl, CustomLabelPointer::Hand, CustomLabelPointer::Hand);
        wszText = L"BUY LONG ";
        if (tradeAction == TradeAction::NewSharesTrade) wszText += L"SHARES";
        if (tradeAction == TradeAction::NewFuturesTrade) wszText += L"FUTURES";
        CustomLabel_SetText(hCtl, wszText);

        hCtl = CustomLabel_ButtonLabel(hwnd, IDC_TRADEDIALOG_BUYSHARES_DROPDOWN, L"\uE015",
            ThemeElement::WhiteDark, ThemeElement::GrayMedium, ThemeElement::GrayLight, ThemeElement::GrayMedium,
            CustomLabelAlignment::MiddleCenter, 191, nTop + 25, 30, 23);
        CustomLabel_SetFont(hCtl, wszFontName, FontSize, true);
        CustomLabel_SetTextColorHot(hCtl, ThemeElement::WhiteLight);

    } else if (tradeAction == TradeAction::ManageShares || tradeAction == TradeAction::ManageFutures) {
        std::wstring wszFontName = L"Segoe UI";
        std::wstring wszText;
        int FontSize = 8;

        hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_SELLSHARES, L"",
            ThemeElement::WhiteLight, ThemeElement::GrayMedium,
            CustomLabelAlignment::MiddleLeft, 40, nTop + 25, 150, 23);
        CustomLabel_SetUserDataInt(hCtl, (int)LongShort::Short);
        CustomLabel_SetFont(hCtl, wszFontName, FontSize, true);
        CustomLabel_SetTextOffset(hCtl, 5, 0);
        TradeDialog_SetLongShortBackColor(hCtl);
        CustomLabel_SetMousePointer(hCtl, CustomLabelPointer::Hand, CustomLabelPointer::Hand);
        wszText = L"SELL LONG ";
        if (tradeAction == TradeAction::ManageShares) wszText += L"SHARES";
        if (tradeAction == TradeAction::ManageFutures) wszText += L"FUTURES";
        CustomLabel_SetText(hCtl, wszText);

        hCtl = CustomLabel_ButtonLabel(hwnd, IDC_TRADEDIALOG_SELLSHARES_DROPDOWN, L"\uE015",
            ThemeElement::WhiteDark, ThemeElement::GrayMedium, ThemeElement::GrayLight, ThemeElement::GrayMedium,
            CustomLabelAlignment::MiddleCenter, 191, nTop + 25, 30, 23);
        CustomLabel_SetFont(hCtl, wszFontName, FontSize, true);
        CustomLabel_SetTextColorHot(hCtl, ThemeElement::WhiteLight);

    }
    else
    {
        // Create the main trade options leg grid
        HWND hGridMain = CreateTradeGrid(hwnd, IDC_TRADEDIALOG_TABLEGRIDMAIN, 40, nTop + 25, 0, 0);

        // If we are rolling then create the second trade grid.
        if (tradeAction == TradeAction::RollLeg) {
            nWidth = AfxUnScaleX((float)AfxGetWindowWidth(hGridMain)) + 20;
            CustomLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_LBLGRIDROLL, L"", TextColorDim, BackColor,
                CustomLabelAlignment::MiddleLeft, 40 + nWidth, nTop, 300, 22);
            HWND hGridRoll = CreateTradeGrid(hwnd, IDC_TRADEDIALOG_TABLEGRIDROLL, 40 + nWidth, nTop + 25, 0, 0);
        }
    }


    nLeft = 40;
    nHeight = 23;
    nTop = 310;

    int nStartTop = nTop;


    nWidth = 80;
    CustomLabel_SimpleLabel(hwnd, -1, L"Quantity", TextColorDim, BackColor,
        CustomLabelAlignment::MiddleRight, nLeft, nTop, nWidth, nHeight);
    hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTQUANTITY, ES_RIGHT,
        L"0", nLeft, nTop + nHeight + vsp, nWidth, nHeight);
    CustomTextBox_SetMargins(hCtl, HTextMargin, VTextMargin);
    CustomTextBox_SetColors(hCtl, lightTextColor, darkBackColor);
    CustomTextBox_SetNumericAttributes(hCtl, 4, CustomTextBoxNegative::Disallow, CustomTextBoxFormatting::Allow);
    if (tradeAction == TradeAction::ManageShares ||
        tradeAction == TradeAction::ManageFutures) {
        // If the aggregate shares are negative then toggle the sell to buy in order to close the trade
        int aggregate = stoi(sharesAggregateEdit);
        CustomTextBox_SetText(hCtl, std::to_wstring(abs(aggregate)));  // set quantity before doing the toggle
    }

    nLeft = nLeft + nWidth + hmargin;
    CustomLabel_SimpleLabel(hwnd, -1, L"Multiplier", TextColorDim, BackColor,
        CustomLabelAlignment::MiddleRight, nLeft, nTop, nWidth, nHeight);
    hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTMULTIPLIER, ES_RIGHT,
        L"100.0000", nLeft, nTop + nHeight + vsp, nWidth, nHeight);
    CustomTextBox_SetMargins(hCtl, HTextMargin, VTextMargin);
    CustomTextBox_SetColors(hCtl, lightTextColor, darkBackColor);
    CustomTextBox_SetNumericAttributes(hCtl, 4, CustomTextBoxNegative::Disallow, CustomTextBoxFormatting::Allow);
    if (tradeAction == TradeAction::NewSharesTrade ||
        tradeAction == TradeAction::NewFuturesTrade ||
        tradeAction == TradeAction::ManageShares ||
        tradeAction == TradeAction::ManageFutures ) {
        CustomTextBox_SetText(hCtl, L"1");
    }

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
    if (IsNewSharesTradeAction(tradeAction) == true) {
        TradeDialog_SetComboDRCR(hCtl, L"DR");
    } else {
        TradeDialog_SetComboDRCR(hCtl, L"CR");
    }

    // SAVE button
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_TRADEDIALOG_SAVE, L"Save",
        ThemeElement::Black, ThemeElement::Green, ThemeElement::Green, ThemeElement::GrayMedium,
        CustomLabelAlignment::MiddleCenter, 580, nTop + nHeight + vsp, 80, 23);
    CustomLabel_SetFont(hCtl, wszFontName, FontSize, true);
    CustomLabel_SetTextColorHot(hCtl, ThemeElement::WhiteLight);


    TradeDialogControls_GetTradeDescription(hwnd);


    // If the aggregate shares are negative then toggle the sell to buy in order to close the trade.
    // Must do this after the quantity, multiplier, etc have been set otherwise we'll get a GPF
    // when the calculate totals eventually gets called during the DR/CR toggle.
    if (tradeAction == TradeAction::ManageShares ||
        tradeAction == TradeAction::ManageFutures) {
        int aggregate = stoi(sharesAggregateEdit);
        if (aggregate < 0) {
            TradeDialog_ToggleSellLongShortText(GetDlgItem(hwnd, IDC_TRADEDIALOG_SELLSHARES));
            TradeDialog_SetLongShortBackColor(GetDlgItem(hwnd, IDC_TRADEDIALOG_SELLSHARES));
        }
    }

    // CATEGORY SELECTOR
    if (IsNewOptionsTradeAction(tradeAction) == true ||
        IsNewSharesTradeAction(tradeAction) == true) {
        CreateCategoryControl(hwnd, IDC_TRADEDIALOG_CATEGORY, 540, 45, 124, 23);
    }
}

