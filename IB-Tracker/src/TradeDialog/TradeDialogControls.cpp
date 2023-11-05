/*

MIT License

Copyright(c) 2023 Paul Squires

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include "pch.h"

#include "TradeDialog.h"
#include "Utilities/AfxWin.h"
#include "CustomLabel/CustomLabel.h"
#include "CustomTextBox/CustomTextBox.h"
#include "TradeGrid/TradeGrid.h"
#include "Strategy/StrategyButton.h"
#include "ActiveTrades/ActiveTrades.h"
#include "MainWindow/tws-client.h"
#include "Database/database.h"
#include "Category/Category.h"
#include "Config/Config.h"



// ========================================================================================
// Set the Short/Long background color.
// ========================================================================================
void TradeDialog_SetLongShortback_color(HWND hCtl)
{
    LongShort ls = (LongShort)CustomLabel_GetUserDataInt(hCtl);

    if (ls == LongShort::Long) {
        CustomLabel_SetBackColor(hCtl, COLOR_GREEN);
    }
    else if (ls == LongShort::Short) {
        CustomLabel_SetBackColor(hCtl, COLOR_RED);
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

    std::wstring text;

    switch ((LongShort)sel)
    {
    case LongShort::Long:
        text = L"BUY LONG ";
        TradeDialog_SetComboDRCR(GetDlgItem(HWND_TRADEDIALOG, IDC_TRADEDIALOG_COMBODRCR), L"DR");
        break;
    case LongShort::Short:
        text = L"SELL SHORT ";
        TradeDialog_SetComboDRCR(GetDlgItem(HWND_TRADEDIALOG, IDC_TRADEDIALOG_COMBODRCR), L"CR");
        break;
    default:
        text = L"";
    }

    if (tdd.trade_action == TradeAction::new_shares_trade) text += L"SHARES";
    if (tdd.trade_action == TradeAction::add_shares_to_trade) text += L"SHARES";
    if (tdd.trade_action == TradeAction::new_futures_trade) text += L"FUTURES";
    if (tdd.trade_action == TradeAction::add_futures_to_trade) text += L"FUTURES";
    CustomLabel_SetText(hCtl, text);
}


// ========================================================================================
// Toggle the SELL Short/Long Shares/Futures text.
// ========================================================================================
void TradeDialog_ToggleSellLongShortText(HWND hCtl)
{
    int sel = CustomLabel_GetUserDataInt(hCtl) + 1;
    if (sel == (int)LongShort::Count) sel = 0;

    CustomLabel_SetUserDataInt(hCtl, sel);

    std::wstring text;

    switch ((LongShort)sel)
    {
    case LongShort::Short:
        text = L"SELL LONG ";
        TradeDialog_SetComboDRCR(GetDlgItem(HWND_TRADEDIALOG, IDC_TRADEDIALOG_COMBODRCR), L"CR");
        break;
    case LongShort::Long:
        text = L"BUY SHORT ";
        TradeDialog_SetComboDRCR(GetDlgItem(HWND_TRADEDIALOG, IDC_TRADEDIALOG_COMBODRCR), L"DR");
        break;
    default:
        text = L"";
    }

    if (tdd.trade_action == TradeAction::manage_shares) text += L"SHARES";
    if (tdd.trade_action == TradeAction::manage_futures) text += L"FUTURES";
    CustomLabel_SetText(hCtl, text);
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
    std::wstring ticker = AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTTICKER));
    int nShow = (IsFuturesTicker(ticker)) ? SW_SHOW : SW_HIDE;

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

    DWORD text_color = COLOR_RED;
    DWORD back_color = COLOR_GRAYMEDIUM;

    std::wstring DRCR = CustomLabel_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_COMBODRCR));
    if (DRCR == L"CR") {
        fees = fees * -1;
        text_color = COLOR_GREEN;
    }

    total = (quantity * multiplier * price) + fees;

    CustomTextBox_SetColors(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTTOTAL), text_color, back_color);
    CustomTextBox_SetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTTOTAL), std::to_wstring(total));
}


// ========================================================================================
// Load the legs for the Transaction Edit Action into the Trade Management table(s)
// ========================================================================================
void TradeDialog_LoadEditTransactionInTradeTable(HWND hwnd)
{
    if (tdd.legs.size() == 0) return;

    HWND hGridMain = GetDlgItem(HWND_TRADEDIALOG, IDC_TRADEDIALOG_TABLEGRIDMAIN);
    HWND hGridRoll = GetDlgItem(HWND_TRADEDIALOG, IDC_TRADEDIALOG_TABLEGRIDROLL);
    HWND hGrid = hGridMain;

    // If more than 4 legs exist in the legs vector then we have to also use the rolled grid.
    int row = 0;
    for (const auto& leg : tdd.legs) {

        if (row > 3) {
            hGrid = hGridRoll;
            row = 0;
        }
        if (row > 7) break;

        // QUANTITY (ORIGINAL)
        std::wstring leg_original_quantity = std::to_wstring(leg->original_quantity / max(tdd.trans->quantity,1));
        TradeGrid_SetColData(hGrid, row, 0, leg_original_quantity);

        // QUANTITY (OPEN)
        std::wstring legopen_quantity = std::to_wstring(leg->open_quantity / max(tdd.trans->quantity,1));
        TradeGrid_SetColData(hGrid, row, 1, legopen_quantity);

        // EXPIRY DATE
        TradeGrid_SetColData(hGrid, row, 2, leg->expiry_date);

        // STRIKE PRICE
        TradeGrid_SetColData(hGrid, row, 4, leg->strike_price);

        // PUT/CALL
        TradeGrid_SetColData(hGrid, row, 5, leg->PutCall);

        // ACTION
        TradeGrid_SetColData(hGrid, row, 6, leg->action);

        row++;
    }

    // TRANSACTION DATE
    CustomLabel_SetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLTRANSDATE), AfxLongDate(tdd.trans->trans_date));
    CustomLabel_SetUserData(GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLTRANSDATE), tdd.trans->trans_date);

    // FUTURES CONTRACT DATE
    CustomLabel_SetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLCONTRACTDATE), AfxLongDate(tdd.trade->future_expiry));
    CustomLabel_SetUserData(GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLCONTRACTDATE), tdd.trade->future_expiry);

    // DESCRIPTION
    AfxSetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTDESCRIBE), tdd.trans->description);

    // DTE
    TradeGrid_CalculateDTE(hGridMain);
    TradeGrid_CalculateDTE(hGridRoll);

    // CATEGORY
    CategoryControl_SetSelectedIndex(GetDlgItem(hwnd, IDC_TRADEDIALOG_CATEGORY), tdd.trade->category);

    // QUANTITY
    CustomTextBox_SetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTQUANTITY), std::to_wstring(tdd.trans->quantity));
    CustomTextBox_SetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTPRICE), std::to_wstring(tdd.trans->price));
    CustomTextBox_SetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTMULTIPLIER), std::to_wstring(tdd.trans->multiplier));
    CustomTextBox_SetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTFEES), std::to_wstring(tdd.trans->fees));
    CustomTextBox_SetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTTOTAL), std::to_wstring(tdd.trans->total));
    CustomTextBox_SetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTTRADEBP), std::to_wstring(tdd.trade->trade_bp));

    if (tdd.trans->total < 0) {
        TradeDialog_SetComboDRCR(GetDlgItem(hwnd, IDC_TRADEDIALOG_COMBODRCR), L"DR");
    }
    else {
        TradeDialog_SetComboDRCR(GetDlgItem(hwnd, IDC_TRADEDIALOG_COMBODRCR), L"CR");
    }
}


// ========================================================================================
// Load the legs for the edit Action into the Trade Management table
// ========================================================================================
void TradeDialog_LoadEditLegsInTradeTable(HWND hwnd)
{
    HWND hCtl = NULL;
    std::wstring text;

    if (tdd.trade_action == TradeAction::new_options_trade) return;
    if (tdd.trade_action == TradeAction::new_shares_trade) return;
    if (tdd.trade_action == TradeAction::new_futures_trade) return;

    // Update the Trade Management table with the details of the Trade.
    if (tdd.trade != nullptr) {
        CustomLabel_SetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLCOMPANY), tdd.trade->ticker_name);
        CustomTextBox_SetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTCOMPANY), tdd.trade->ticker_name);  // hidden

        text = tdd.trade->ticker_symbol;
        if (tdd.trade->future_expiry.length()) {
            text = text + L": " + AfxFormatFuturesDate(tdd.trade->future_expiry);
            CustomLabel_SetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLCONTRACTDATE), AfxLongDate(tdd.trade->future_expiry));
            CustomLabel_SetUserData(GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLCONTRACTDATE), tdd.trade->future_expiry);
        }
        CustomLabel_SetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLTICKER), text);
        CustomTextBox_SetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTTICKER), tdd.trade->ticker_symbol);  // hidden

        // Set the multiplier based on the incoming trade. Ensure that multiplier is always 100 for Option
        // transactions because it could be set to 1 if the Trade only contains existing Shares.
        double multiplier = tdd.trade->multiplier;
        if (IsNewOptionsTradeAction(tdd.trade_action) ||
            tdd.trade_action == TradeAction::add_call_to_trade ||
            tdd.trade_action == TradeAction::add_put_to_trade ||
            tdd.trade_action == TradeAction::add_options_to_trade) {
            if (IsFuturesTicker(tdd.trade->ticker_symbol) == false) multiplier = 100;
        }
        else {
            multiplier = 1;
        }
        CustomTextBox_SetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTMULTIPLIER), std::to_wstring(multiplier));
        CustomTextBox_SetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTTRADEBP), std::to_wstring(tdd.trade->trade_bp));
    }

    
    // Editing a previously created Transaction requires a separate function to
    // load the existing data into the table(s).
    if (tdd.trade_action == TradeAction::edit_transaction) {
        TradeDialog_LoadEditTransactionInTradeTable(hwnd);
        return;
    }


    if (tdd.trade_action == TradeAction::new_iron_condor) {
        hCtl = GetDlgItem(HWND_STRATEGYBUTTON, IDC_STRATEGYBUTTON_LONGSHORT);
        CustomLabel_SetUserDataInt(hCtl, (int)LongShort::Short);
        text = AfxUpper(StrategyButton_GetLongShortEnumText(LongShort::Short));
        CustomLabel_SetText(hCtl, text);

        hCtl = GetDlgItem(HWND_STRATEGYBUTTON, IDC_STRATEGYBUTTON_PutCall);
        CustomLabel_SetText(hCtl, L"");

        hCtl = GetDlgItem(HWND_STRATEGYBUTTON, IDC_STRATEGYBUTTON_STRATEGY);
        CustomLabel_SetUserDataInt(hCtl,(int)Strategy::IronCondor);
        text = AfxUpper(StrategyButton_GetStrategyEnumText(Strategy::IronCondor));
        CustomLabel_SetText(hCtl, text);

        StrategyButton_InvokeStrategy();
        return;
    }

    if (tdd.trade_action == TradeAction::new_short_LT112) {
        hCtl = GetDlgItem(HWND_STRATEGYBUTTON, IDC_STRATEGYBUTTON_LONGSHORT);
        CustomLabel_SetUserDataInt(hCtl, (int)LongShort::Short);
        text = AfxUpper(StrategyButton_GetLongShortEnumText(LongShort::Short));
        CustomLabel_SetText(hCtl, text);

        hCtl = GetDlgItem(HWND_STRATEGYBUTTON, IDC_STRATEGYBUTTON_PutCall);
        CustomLabel_SetText(hCtl, L"");

        hCtl = GetDlgItem(HWND_STRATEGYBUTTON, IDC_STRATEGYBUTTON_STRATEGY);
        CustomLabel_SetUserDataInt(hCtl,(int)Strategy::LT112);
        text = AfxUpper(StrategyButton_GetStrategyEnumText(Strategy::LT112));
        CustomLabel_SetText(hCtl, text);

        StrategyButton_InvokeStrategy();
        return;
    }

    if (tdd.trade_action == TradeAction::new_short_strangle) {
        hCtl = GetDlgItem(HWND_STRATEGYBUTTON, IDC_STRATEGYBUTTON_LONGSHORT);
        CustomLabel_SetUserDataInt(hCtl, (int)LongShort::Short);
        text = AfxUpper(StrategyButton_GetLongShortEnumText(LongShort::Short));
        CustomLabel_SetText(hCtl, text);

        hCtl = GetDlgItem(HWND_STRATEGYBUTTON, IDC_STRATEGYBUTTON_PutCall);
        CustomLabel_SetText(hCtl, L"");

        hCtl = GetDlgItem(HWND_STRATEGYBUTTON, IDC_STRATEGYBUTTON_STRATEGY);
        CustomLabel_SetUserDataInt(hCtl,(int)Strategy::Strangle);
        text = AfxUpper(StrategyButton_GetStrategyEnumText(Strategy::Strangle));
        CustomLabel_SetText(hCtl, text);

        StrategyButton_InvokeStrategy();
        return;
    }

    if (tdd.trade_action == TradeAction::new_short_put ||
        tdd.trade_action == TradeAction::add_put_to_trade) {
        hCtl = GetDlgItem(HWND_STRATEGYBUTTON, IDC_STRATEGYBUTTON_LONGSHORT);
        CustomLabel_SetUserDataInt(hCtl, (int)LongShort::Short);
        text = AfxUpper(StrategyButton_GetLongShortEnumText(LongShort::Short));
        CustomLabel_SetText(hCtl, text);

        hCtl = GetDlgItem(HWND_STRATEGYBUTTON, IDC_STRATEGYBUTTON_PutCall);
        CustomLabel_SetUserDataInt(hCtl, (int)PutCall::Put);
        text = AfxUpper(StrategyButton_GetPutCallEnumText(PutCall::Put));
        CustomLabel_SetText(hCtl, text);

        hCtl = GetDlgItem(HWND_STRATEGYBUTTON, IDC_STRATEGYBUTTON_STRATEGY);
        CustomLabel_SetUserDataInt(hCtl, (int)Strategy::Option);
        text = AfxUpper(StrategyButton_GetStrategyEnumText(Strategy::Option));
        CustomLabel_SetText(hCtl, text);

        StrategyButton_InvokeStrategy();
        return;
    }

    if (tdd.trade_action == TradeAction::new_short_call ||
        tdd.trade_action == TradeAction::add_call_to_trade) {
        hCtl = GetDlgItem(HWND_STRATEGYBUTTON, IDC_STRATEGYBUTTON_LONGSHORT);
        CustomLabel_SetUserDataInt(hCtl, (int)LongShort::Short);
        text = AfxUpper(StrategyButton_GetLongShortEnumText(LongShort::Short));
        CustomLabel_SetText(hCtl, text);

        hCtl = GetDlgItem(HWND_STRATEGYBUTTON, IDC_STRATEGYBUTTON_PutCall);
        CustomLabel_SetUserDataInt(hCtl, (int)PutCall::Call);
        text = AfxUpper(StrategyButton_GetPutCallEnumText(PutCall::Call));
        CustomLabel_SetText(hCtl, text);

        hCtl = GetDlgItem(HWND_STRATEGYBUTTON, IDC_STRATEGYBUTTON_STRATEGY);
        CustomLabel_SetUserDataInt(hCtl, (int)Strategy::Option);
        text = AfxUpper(StrategyButton_GetStrategyEnumText(Strategy::Option));
        CustomLabel_SetText(hCtl, text);

        StrategyButton_InvokeStrategy();
        return;
    }


    if (tdd.legs.size() == 0) return;

    int default_quantity = 1;

    // Display the Trade's Buying Power BP in case that needs to be edited also.
    CustomTextBox_SetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTTRADEBP), std::to_wstring(tdd.trade->trade_bp));

     
    // Display the legs being closed and set each to the needed inverse action.
    HWND hGridMain = GetDlgItem(HWND_TRADEDIALOG, IDC_TRADEDIALOG_TABLEGRIDMAIN);
    TradeGrid* pData = TradeGrid_GetOptions(hGridMain);
    if (pData == nullptr) return;

    int row = 0;
    for (const auto& leg : tdd.legs) {

        // Only load a maximum of 4 legs even if the user had selected more than 4.
        if (row > 3) break;

        // QUANTITY
        std::wstring leg_quantity = std::to_wstring(leg->open_quantity * -1);
        TradeGrid_SetColData(hGridMain, row, 0, leg_quantity);

        // EXPIRY DATE
        TradeGrid_SetColData(hGridMain, row, 1, leg->expiry_date);

        // STRIKE PRICE
        TradeGrid_SetColData(hGridMain, row, 3, leg->strike_price);

        // PUT/CALL
        TradeGrid_SetColData(hGridMain, row, 4, leg->PutCall);

        // ACTION
        if (leg->action == L"STO") { text = L"BTC"; }
        if (leg->action == L"BTO") { text = L"STC"; }
        TradeGrid_SetColData(hGridMain, row, 5, text);

        row++;
    }
    
    // DTE
    TradeGrid_CalculateDTE(hGridMain);

    // QUANTITY
    AfxSetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTQUANTITY), std::to_wstring(abs(default_quantity)));


    // Add some default information for the new Roll transaction
    if (tdd.trade_action == TradeAction::roll_leg) {
        HWND hGridRoll = GetDlgItem(HWND_TRADEDIALOG, IDC_TRADEDIALOG_TABLEGRIDROLL);
        TradeGrid* pData = TradeGrid_GetOptions(hGridRoll);
        if (pData == nullptr) return;

        int row = 0;
        for (const auto& leg : tdd.legs) {

            // Only load a maximum of 4 legs even if the user had selected more than 4.
            if (row > 3) break;

            // QUANTITY
            std::wstring leg_quantity = std::to_wstring(leg->open_quantity);
            TradeGrid_SetColData(hGridRoll, row, 0, leg_quantity);

            // EXPIRY DATE
            text = AfxDateAddDays(leg->expiry_date, 7);
            TradeGrid_SetColData(hGridRoll, row, 1, text);

            // STRIKE PRICE
            TradeGrid_SetColData(hGridRoll, row, 3, leg->strike_price);

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
    if (tdd.trade_action == TradeAction::close_leg) {
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
    std::wstring description;
    std::wstring grid_main;
    std::wstring grid_roll;

    switch (tdd.trade_action)
    {
    case TradeAction::edit_transaction:
        description = L"Edit";
        grid_main = L"Transaction Data";
        grid_roll = L"Transaction Data";
        break;

    case TradeAction::new_options_trade:
    case TradeAction::new_short_strangle:
    case TradeAction::new_short_call:
    case TradeAction::new_iron_condor:
    case TradeAction::new_short_LT112:
    case TradeAction::new_short_put:
        description = L"Options";
        grid_main = L"New Transaction";
        break;
    case TradeAction::new_shares_trade:
    case TradeAction::manage_shares:
        description = L"Shares";
        grid_main = L"New Transaction";
        break;
    case TradeAction::new_futures_trade:
    case TradeAction::manage_futures:
        description = L"Futures";
        grid_main = L"New Transaction";
        break;
    case TradeAction::close_leg:
        description = L"Close";
        grid_main = L"Close Transaction";
        break;
    case TradeAction::expire_leg:
        description = L"Expire";
        break;
    case TradeAction::roll_leg:
        description = L"Roll";
        grid_main = L"Close Transaction";
        grid_roll = L"Rolled Transaction";
        break;
    case TradeAction::assignment:
        description = L"Assignment";
        break;
    case TradeAction::add_options_to_trade:
        description = L"Add Options";
        grid_main = L"New Transaction";
        break;
    case TradeAction::add_put_to_trade:
        description = L"Add Put";
        grid_main = L"New Transaction";
        break;
    case TradeAction::add_call_to_trade:
        description = L"Add Call";
        grid_main = L"New Transaction";
        break;
    case TradeAction::add_shares_to_trade:
        description = L"Add Shares";
        grid_main = L"New Transaction";
        break;
    case TradeAction::add_dividend_to_trade:
        description = L"Add Dividend";
        break;
    case TradeAction::add_futures_to_trade:
        description = L"Add Futures";
        grid_main = L"New Transaction";
        break;
    }

    CustomLabel_SetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLEDITACTION), AfxUpper(description));
    CustomLabel_SetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLGRIDMAIN), grid_main);
    CustomLabel_SetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLGRIDROLL), grid_roll);

    return description;
}


// ========================================================================================
// Helper function for WM_CREATE message for window/dialog: TradeDialog
// ========================================================================================
void TradeDialogControls_CreateControls(HWND hwnd)
{
    HWND hCtl = NULL;
    CustomLabel* pData = nullptr;

    int horiz_text_margin = 0;
    int vert_text_margin = 3;

    // EDIT ACTION LABEL
    hCtl = CreateCustomLabel(
        hwnd, IDC_TRADEDIALOG_LBLEDITACTION, CustomLabelType::text_only,
        542, 10, 120, 20);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->back_color = COLOR_GRAYDARK;
        pData->back_color_button_down = COLOR_GRAYDARK;
        pData->text_color = COLOR_WHITELIGHT;
        pData->font_size = 12;
        pData->font_bold = true;
        pData->text_alignment = CustomLabelAlignment::top_right;
        CustomLabel_SetOptions(hCtl, pData);
    }


    // NEW TRADE SHOWS TEXTBOXES, OTHERS JUST LABELS
    if (IsNewOptionsTradeAction(tdd.trade_action) == true ||
        IsNewSharesTradeAction(tdd.trade_action) == true) {
        CustomLabel_SimpleLabel(hwnd, -1, L"Ticker", COLOR_WHITEDARK, COLOR_GRAYDARK,
            CustomLabelAlignment::middle_left, 40, 20, 65, 22);
        hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTTICKER, false, ES_LEFT | ES_UPPERCASE, L"", 40, 45, 65, 23);
        CustomTextBox_SetMargins(hCtl, horiz_text_margin, vert_text_margin);
        CustomTextBox_SetColors(hCtl, COLOR_WHITELIGHT, COLOR_GRAYMEDIUM);

        hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_LBLCOMPANY, L"Company Name", COLOR_WHITEDARK, COLOR_GRAYDARK,
            CustomLabelAlignment::middle_left, 115, 20, 115, 22);
        if (tdd.trade_action == TradeAction::new_futures_trade) {
            CustomLabel_SetText(hCtl, L"Futures Contract");
        }
        hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTCOMPANY, false, ES_LEFT, L"", 115, 45, 215, 23);
        CustomTextBox_SetMargins(hCtl, horiz_text_margin, vert_text_margin);
        CustomTextBox_SetColors(hCtl, COLOR_WHITELIGHT, COLOR_GRAYMEDIUM);

    }
    else {
        hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTTICKER, false, ES_LEFT | ES_UPPERCASE, L"", 0, 0, 0, 0);
        ShowWindow(hCtl, SW_HIDE);
        hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTCOMPANY, false, ES_LEFT, L"", 0, 0, 0, 0);
        ShowWindow(hCtl, SW_HIDE);

        hCtl = CreateCustomLabel(
            hwnd, IDC_TRADEDIALOG_LBLCOMPANY, CustomLabelType::text_only,
            40, 10, 250, 22);
        pData = CustomLabel_GetOptions(hCtl);
        if (pData) {
            pData->back_color = COLOR_GRAYDARK;
            pData->back_color_button_down = COLOR_GRAYDARK;
            pData->text_color = COLOR_WHITELIGHT;
            pData->font_size = 12;
            pData->text_alignment = CustomLabelAlignment::top_left;
            pData->text = L"";
            CustomLabel_SetOptions(hCtl, pData);
        }

        hCtl = CreateCustomLabel(
            hwnd, IDC_TRADEDIALOG_LBLTICKER, CustomLabelType::text_only,
            40, 33, 250, 22);
        pData = CustomLabel_GetOptions(hCtl);
        if (pData) {
            pData->back_color = COLOR_GRAYDARK;
            pData->back_color_button_down = COLOR_GRAYDARK;
            pData->text_color = COLOR_WHITEDARK;
            pData->font_size = 10;
            pData->text_alignment = CustomLabelAlignment::top_left;
            pData->text = L"";
            CustomLabel_SetOptions(hCtl, pData);
        }
    }


    bool show_contract_expiry = false;
    std::wstring contract_date;

    if (tdd.trade_action == TradeAction::new_futures_trade) {
        contract_date = AfxCurrentDate();
        show_contract_expiry = true;
    }
    if (tdd.trade_action == TradeAction::edit_transaction) {
        if (IsFuturesTicker(tdd.trade->ticker_symbol)) {   // this is a Future
            contract_date = tdd.trade->future_expiry;
            show_contract_expiry = true;
        }
    }
    
    hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_LBLCONTRACT, L"Contract Expiry", COLOR_WHITEDARK, COLOR_GRAYDARK,
        CustomLabelAlignment::middle_left, 340, 20, 120, 22);
    CustomLabel_SetUserData(hCtl, contract_date);
    hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_LBLCONTRACTDATE, AfxLongDate(contract_date),
        COLOR_WHITELIGHT, COLOR_GRAYMEDIUM, CustomLabelAlignment::middle_left, 340, 45, 79, 23);
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_TRADEDIALOG_CMDCONTRACTDATE, GLYPH_DROPDOWN,
        COLOR_WHITEDARK, COLOR_GRAYMEDIUM, COLOR_GRAYLIGHT, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_center, 419, 45, 23, 23);
        
    if (show_contract_expiry == true) {
        CustomLabel_SetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLCOMPANY), L"Futures Contract");
    }
    else {
        ShowWindow(GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLCONTRACT), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLCONTRACTDATE), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, IDC_TRADEDIALOG_CMDCONTRACTDATE), SW_HIDE);
    }


    // CATEGORY SELECTOR
    int selected_index = (tdd.trade != nullptr) ? tdd.trade->category : 0;
    hCtl = CreateCategoryControl(hwnd, IDC_TRADEDIALOG_CATEGORY, 450, 45, selected_index, false);
    ShowWindow(hCtl, SW_HIDE);
    if (IsNewOptionsTradeAction(tdd.trade_action) == true ||
        IsNewSharesTradeAction(tdd.trade_action) == true ||
        tdd.trade_action == TradeAction::edit_transaction) {
        ShowWindow(hCtl, SW_SHOW);
    }


    CustomLabel_SimpleLabel(hwnd, -1, L"Date", COLOR_WHITEDARK, COLOR_GRAYDARK,
        CustomLabelAlignment::middle_left, 40, 72, 100, 22);
    std::wstring date_text = AfxCurrentDate();
    hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_LBLTRANSDATE, AfxLongDate(date_text), COLOR_WHITELIGHT, COLOR_GRAYMEDIUM,
        CustomLabelAlignment::middle_left, 40, 97, 86, 23);
    CustomLabel_SetMousePointer(hCtl, CustomLabelPointer::hand, CustomLabelPointer::hand);
    CustomLabel_SetUserData(hCtl, date_text);

    CustomLabel_ButtonLabel(hwnd, IDC_TRADEDIALOG_CMDTRANSDATE, GLYPH_DROPDOWN,
        COLOR_WHITEDARK, COLOR_GRAYMEDIUM, COLOR_GRAYLIGHT, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_center, 126, 97, 23, 23);

    // We always want the Description textbox to exists because even for rolled and closed transaction
    // we need to set the description (even though the user will never see the actual textbox in those
    // types of actions).

    CustomLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_LBLDESCRIBE, L"Description", COLOR_WHITEDARK, COLOR_GRAYDARK,
        CustomLabelAlignment::middle_left, 159, 72, 115, 22);
    hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTDESCRIBE, false, ES_LEFT, L"", 159, 97, 171, 23);
    CustomTextBox_SetMargins(hCtl, horiz_text_margin, vert_text_margin);
    CustomTextBox_SetColors(hCtl, COLOR_WHITELIGHT, COLOR_GRAYMEDIUM);

    if (IsNewOptionsTradeAction(tdd.trade_action) == false ||
        IsNewSharesTradeAction(tdd.trade_action) == true) {
        ShowWindow(GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLDESCRIBE), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTDESCRIBE), SW_HIDE);
    }

    if (tdd.trade_action == TradeAction::edit_transaction) {
        ShowWindow(GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLDESCRIBE), SW_SHOW);
        ShowWindow(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTDESCRIBE), SW_SHOW);
    }



    // We create the Strategy button and label but we only show it for New options
    // However we do need the window for other tradeAction cases for example "Add To"
    // because the tradeAction into the Strategy button and then InvokeStrategy.
    if (IsNewOptionsTradeAction(tdd.trade_action) == true ||
        tdd.trade_action == TradeAction::add_options_to_trade ||
        tdd.trade_action == TradeAction::add_put_to_trade ||
        tdd.trade_action == TradeAction::add_call_to_trade) {

        hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_LBLSTRATEGY, L"Strategy", COLOR_WHITEDARK, COLOR_GRAYDARK,
            CustomLabelAlignment::middle_left, 340, 72, 100, 22);
        hCtl = StrategyButton.Create(hwnd, L"", 340, 97, 264, 23,
            WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CONTROLPARENT);
    }


    CustomLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_LBLGRIDMAIN, L"", COLOR_WHITEDARK, COLOR_GRAYDARK,
        CustomLabelAlignment::middle_left, 40, 155, 300, 22);

    if (IsNewSharesTradeAction(tdd.trade_action) == true ||
        tdd.trade_action == TradeAction::add_shares_to_trade ||
        tdd.trade_action == TradeAction::add_futures_to_trade) {
        std::wstring font_name = L"Segoe UI";
        std::wstring text;
        int font_size = 8;

        hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_BUYSHARES, L"",
            COLOR_WHITELIGHT, COLOR_GRAYMEDIUM,
            CustomLabelAlignment::middle_left, 40, 180, 150, 23);
        CustomLabel_SetUserDataInt(hCtl, (int)LongShort::Long);
        CustomLabel_SetFont(hCtl, font_name, font_size, true);
        CustomLabel_SetTextOffset(hCtl, 5, 0);
        TradeDialog_SetLongShortback_color(hCtl);
        CustomLabel_SetMousePointer(hCtl, CustomLabelPointer::hand, CustomLabelPointer::hand);
        text = L"BUY LONG ";
        if (tdd.trade_action == TradeAction::new_shares_trade) text += L"SHARES";
        if (tdd.trade_action == TradeAction::add_shares_to_trade) text += L"SHARES";
        if (tdd.trade_action == TradeAction::new_futures_trade) text += L"FUTURES";
        if (tdd.trade_action == TradeAction::add_futures_to_trade) text += L"FUTURES";
        CustomLabel_SetText(hCtl, text);

        hCtl = CustomLabel_ButtonLabel(hwnd, IDC_TRADEDIALOG_BUYSHARES_DROPDOWN, GLYPH_DROPDOWN,
            COLOR_WHITEDARK, COLOR_GRAYMEDIUM, COLOR_GRAYLIGHT, COLOR_GRAYMEDIUM, COLOR_WHITE,
            CustomLabelAlignment::middle_center, 191, 180, 30, 23);
        CustomLabel_SetFont(hCtl, font_name, font_size, true);
        CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);

    } else if (tdd.trade_action == TradeAction::add_dividend_to_trade) {

    } else if (tdd.trade_action == TradeAction::manage_shares || tdd.trade_action == TradeAction::manage_futures) {
        std::wstring font_name = L"Segoe UI";
        std::wstring text;
        int font_size = 8;

        hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_SELLSHARES, L"",
            COLOR_WHITELIGHT, COLOR_GRAYMEDIUM,
            CustomLabelAlignment::middle_left, 40, 180, 150, 23);
        CustomLabel_SetUserDataInt(hCtl, (int)LongShort::Short);
        CustomLabel_SetFont(hCtl, font_name, font_size, true);
        CustomLabel_SetTextOffset(hCtl, 5, 0);
        TradeDialog_SetLongShortback_color(hCtl);
        CustomLabel_SetMousePointer(hCtl, CustomLabelPointer::hand, CustomLabelPointer::hand);
        text = L"SELL LONG ";
        if (tdd.trade_action == TradeAction::manage_shares) text += L"SHARES";
        if (tdd.trade_action == TradeAction::manage_futures) text += L"FUTURES";
        CustomLabel_SetText(hCtl, text);

        hCtl = CustomLabel_ButtonLabel(hwnd, IDC_TRADEDIALOG_SELLSHARES_DROPDOWN, GLYPH_DROPDOWN,
            COLOR_WHITEDARK, COLOR_GRAYMEDIUM, COLOR_GRAYLIGHT, COLOR_GRAYMEDIUM, COLOR_WHITE,
            CustomLabelAlignment::middle_center, 191, 180, 30, 23);
        CustomLabel_SetFont(hCtl, font_name, font_size, true);
        CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);

    }
    else
    {
        bool show_original_quantity = (tdd.trade_action == TradeAction::edit_transaction) ? true : false;

        // Create the main trade options leg grid
        HWND hGridMain = CreateTradeGrid(hwnd, IDC_TRADEDIALOG_TABLEGRIDMAIN, 40, 180, 0, 0, show_original_quantity);

        // If we are rolling or editing then create the second trade grid.
        if (tdd.trade_action == TradeAction::roll_leg || tdd.trade_action == TradeAction::edit_transaction) {
            int nWidth = AfxUnScaleX((float)AfxGetWindowWidth(hGridMain)) + 60;
            CustomLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_LBLGRIDROLL, L"", COLOR_WHITEDARK, COLOR_GRAYDARK,
                CustomLabelAlignment::middle_left, nWidth, 155, 300, 22);
            HWND hGridRoll = CreateTradeGrid(hwnd, IDC_TRADEDIALOG_TABLEGRIDROLL, nWidth, 180, 0, 0, show_original_quantity);
        }
    }


    CustomLabel_SimpleLabel(hwnd, -1, L"Quantity", COLOR_WHITEDARK, COLOR_GRAYDARK,
        CustomLabelAlignment::middle_right, 40, 310, 80, 23);
    hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTQUANTITY, false, ES_RIGHT,
        L"0", 40, 337, 80, 23);
    CustomTextBox_SetMargins(hCtl, horiz_text_margin, vert_text_margin);
    CustomTextBox_SetColors(hCtl, COLOR_WHITELIGHT, COLOR_GRAYMEDIUM);
    CustomTextBox_SetNumericAttributes(hCtl, 5, CustomTextBoxNegative::disallow, CustomTextBoxFormatting::allow);
    if (tdd.trade_action == TradeAction::manage_shares ||
        tdd.trade_action == TradeAction::manage_futures) {
        // If the aggregate shares are negative then toggle the sell to buy in order to close the trade
        int aggregate = AfxValInteger(tdd.shares_aggregate_edit);
        CustomTextBox_SetText(hCtl, std::to_wstring(abs(aggregate)));  // set quantity before doing the toggle
    }
    if (tdd.trade_action == TradeAction::add_dividend_to_trade) {
        CustomTextBox_SetText(hCtl, L"1"); 
    }

    
    CustomLabel_SimpleLabel(hwnd, -1, L"Price", COLOR_WHITEDARK, COLOR_GRAYDARK,
        CustomLabelAlignment::middle_right, 130, 310, 80, 23);
    hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTPRICE, false, ES_RIGHT,
        L"0", 130, 337, 80, 23);
    CustomTextBox_SetMargins(hCtl, horiz_text_margin, vert_text_margin);
    CustomTextBox_SetColors(hCtl, COLOR_WHITELIGHT, COLOR_GRAYMEDIUM);
    CustomTextBox_SetNumericAttributes(hCtl, 5, CustomTextBoxNegative::disallow, CustomTextBoxFormatting::allow);


    CustomLabel_SimpleLabel(hwnd, -1, L"Multiplier", COLOR_WHITEDARK, COLOR_GRAYDARK,
        CustomLabelAlignment::middle_right, 220, 310, 80, 23);
    hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTMULTIPLIER, false, ES_RIGHT,
        L"100.0000", 220, 337, 80, 23);
    CustomTextBox_SetMargins(hCtl, horiz_text_margin, vert_text_margin);
    CustomTextBox_SetColors(hCtl, COLOR_WHITELIGHT, COLOR_GRAYMEDIUM);
    CustomTextBox_SetNumericAttributes(hCtl, 5, CustomTextBoxNegative::disallow, CustomTextBoxFormatting::allow);
    if (tdd.trade_action == TradeAction::new_shares_trade ||
        tdd.trade_action == TradeAction::new_futures_trade ||
        tdd.trade_action == TradeAction::manage_shares ||
        tdd.trade_action == TradeAction::manage_futures ||
        tdd.trade_action == TradeAction::add_shares_to_trade ||
        tdd.trade_action == TradeAction::add_dividend_to_trade ||
        tdd.trade_action == TradeAction::add_futures_to_trade) {
        CustomTextBox_SetText(hCtl, L"1");
    }


    CustomLabel_SimpleLabel(hwnd, -1, L"Fees", COLOR_WHITEDARK, COLOR_GRAYDARK,
        CustomLabelAlignment::middle_right, 310, 310, 80, 23);
    hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTFEES, false, ES_RIGHT,
        L"0", 310, 337, 80, 23);
    CustomTextBox_SetMargins(hCtl, horiz_text_margin, vert_text_margin);
    CustomTextBox_SetColors(hCtl, COLOR_WHITELIGHT, COLOR_GRAYMEDIUM);
    CustomTextBox_SetNumericAttributes(hCtl, 2, CustomTextBoxNegative::disallow, CustomTextBoxFormatting::allow);


    CustomLabel_SimpleLabel(hwnd, -1, L"Total", COLOR_WHITEDARK, COLOR_GRAYDARK,
        CustomLabelAlignment::middle_right, 400, 310, 80, 23);
    hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTTOTAL, false, ES_RIGHT,
        L"0", 400, 337, 80, 23);
    CustomTextBox_SetMargins(hCtl, horiz_text_margin, vert_text_margin);
    CustomTextBox_SetColors(hCtl, COLOR_WHITELIGHT, COLOR_GRAYMEDIUM);
    CustomTextBox_SetNumericAttributes(hCtl, 2, CustomTextBoxNegative::disallow, CustomTextBoxFormatting::allow);


    std::wstring font_name = L"Segoe UI";
    int font_size = 9;
    bool bold = false;

    // DR / CR toggle label
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_TRADEDIALOG_COMBODRCR, L"CR",
        COLOR_BLACK, COLOR_GREEN, COLOR_GREEN, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_center, 490, 337, 30, 23);
    CustomLabel_SetFont(hCtl, font_name, font_size, true);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);
    if (IsNewSharesTradeAction(tdd.trade_action) == true ||
        tdd.trade_action == TradeAction::add_shares_to_trade ||
        tdd.trade_action == TradeAction::add_futures_to_trade) {
        TradeDialog_SetComboDRCR(hCtl, L"DR");
    } else {
        TradeDialog_SetComboDRCR(hCtl, L"CR");
    }


    // TRADE BUYING POWER
    CustomLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_LBLTRADEBP, L"Buying Power", COLOR_WHITEDARK, COLOR_GRAYDARK,
        CustomLabelAlignment::middle_left, 580, 310, 100, 23);
    hCtl = CreateCustomTextBox(hwnd, IDC_TRADEDIALOG_TXTTRADEBP, false, ES_RIGHT,
        L"0", 580, 337, 80, 23);
    CustomTextBox_SetMargins(hCtl, horiz_text_margin, vert_text_margin);
    CustomTextBox_SetColors(hCtl, COLOR_WHITELIGHT, COLOR_GRAYMEDIUM);
    CustomTextBox_SetNumericAttributes(hCtl, 2, CustomTextBoxNegative::allow, CustomTextBoxFormatting::allow);

    if (tdd.trade_action == TradeAction::new_shares_trade ||
        tdd.trade_action == TradeAction::manage_shares ||
        tdd.trade_action == TradeAction::add_shares_to_trade ||
        tdd.trade_action == TradeAction::add_dividend_to_trade ||
        tdd.trade_action == TradeAction::new_futures_trade ||
        tdd.trade_action == TradeAction::manage_futures ||
        tdd.trade_action == TradeAction::add_futures_to_trade) {
        ShowWindow(GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLTRADEBP), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTTRADEBP), SW_HIDE);
    }

    // SAVE button
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_TRADEDIALOG_SAVE, L"SAVE",
        COLOR_BLACK, COLOR_GREEN, COLOR_GREEN, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_center, 580, 396, 80, 23);
    CustomLabel_SetFont(hCtl, font_name, font_size, true);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);


    TradeDialogControls_GetTradeDescription(hwnd);


    // If the aggregate shares are negative then toggle the sell to buy in order to close the trade.
    // Must do this after the quantity, multiplier, etc have been set otherwise we'll get a GPF
    // when the calculate totals eventually gets called during the DR/CR toggle.
    if (tdd.trade_action == TradeAction::manage_shares ||
        tdd.trade_action == TradeAction::manage_futures) {
        int aggregate = AfxValInteger(tdd.shares_aggregate_edit);
        if (aggregate < 0) {
            TradeDialog_ToggleSellLongShortText(GetDlgItem(hwnd, IDC_TRADEDIALOG_SELLSHARES));
            TradeDialog_SetLongShortback_color(GetDlgItem(hwnd, IDC_TRADEDIALOG_SELLSHARES));
        }
    }


    // EDIT TRANSACTION WARNING
    CustomLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_LBLEDITWARNING1, L"", COLOR_WHITEDARK, COLOR_GRAYDARK,
        CustomLabelAlignment::middle_left, 40, 396, 80, 16);
    CustomLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_LBLEDITWARNING2, L"", COLOR_WHITEDARK, COLOR_GRAYDARK,
        CustomLabelAlignment::middle_left, 120, 396, 500, 16);
    CustomLabel_SimpleLabel(hwnd, IDC_TRADEDIALOG_LBLEDITWARNING3, L"", COLOR_WHITEDARK, COLOR_GRAYDARK,
        CustomLabelAlignment::middle_left, 120, 412, 500, 16);

    if (tdd.trade_action != TradeAction::new_shares_trade &&
        tdd.trade_action != TradeAction::add_shares_to_trade &&
        tdd.trade_action != TradeAction::add_dividend_to_trade &&
        tdd.trade_action != TradeAction::add_futures_to_trade &&
        tdd.trade_action != TradeAction::add_call_to_trade && 
        tdd.trade_action != TradeAction::add_put_to_trade && 
        tdd.trade_action != TradeAction::add_options_to_trade &&
        tdd.trade_action != TradeAction::close_leg &&
        tdd.trade_action != TradeAction::manage_shares &&
        tdd.trade_action != TradeAction::manage_futures &&
        tdd.trade_action != TradeAction::edit_transaction &&
        tdd.trade_action != TradeAction::roll_leg) {
        std::wstring text1 = L"NOTE:";
        std::wstring text2 = L"Future Ticker names must start with a forward slash.";
        std::wstring text3 = L"For example: /ES, /MES, /NQ, etc.";
        CustomLabel_SetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLEDITWARNING1), text1);
        CustomLabel_SetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLEDITWARNING2), text2);
        CustomLabel_SetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLEDITWARNING3), text3);
    }

}

