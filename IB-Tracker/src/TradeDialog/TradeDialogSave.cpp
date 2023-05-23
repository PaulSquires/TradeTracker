#include "pch.h"

#include "TradeDialog.h"
#include "..\Utilities\AfxWin.h"
#include "..\CustomLabel\CustomLabel.h"
#include "..\CustomTextBox\CustomTextBox.h"
#include "..\TradeGrid\TradeGrid.h"
#include "..\TradesPanel\TradesPanel.h"
#include "..\MainWindow\tws-client.h"
#include "..\Database\database.h"
#include "..\Category\Category.h"
#include "..\Strategy\StrategyButton.h"



extern HWND HWND_TRADEDIALOG;
extern CTradeDialog TradeDialog;
extern TradeAction tradeAction;
extern std::vector<std::shared_ptr<Leg>> legsEdit;
extern std::shared_ptr<Trade> tradeEdit;

extern void TradesPanel_ShowActiveTrades();


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
// Perform error checks on the Shares/Futures trade data prior to allowing the save 
// to the database.
// ========================================================================================
bool TradeDialog_ValidateSharesTradeData(HWND hwnd)
{
    // Do an error check to ensure that the data about to be saved does not contain
    // any missing data, etc.
    std::wstring wszErrMsg;
    std::wstring wszText;

    if (tradeAction == TradeAction::NewSharesTrade ||
        tradeAction == TradeAction::ManageShares ||
        tradeAction == TradeAction::AddSharesToTrade) {
        CustomTextBox_SetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTDESCRIBE), L"Shares");
    }
    if (tradeAction == TradeAction::NewFuturesTrade ||
        tradeAction == TradeAction::ManageFutures ||
        tradeAction == TradeAction::AddFuturesToTrade) {
        CustomTextBox_SetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTDESCRIBE), L"Futures");
    }

    if (IsNewSharesTradeAction(tradeAction) == true) {
        wszText = AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTTICKER));
        if (wszText.length() == 0) wszErrMsg += L"- Missing Ticker Symbol.\n";
        wszText = AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTCOMPANY));
        if (wszText.length() == 0) wszErrMsg += L"- Missing Company or Futures Name.\n";
        wszText = AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTDESCRIBE));
        if (wszText.length() == 0) wszErrMsg += L"- Missing Description.\n";
    }

    int quantity = stoi(AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTQUANTITY)));
    if (quantity == 0) wszErrMsg += L"- Missing Quantity.\n";


    if (wszErrMsg.length()) {
        MessageBox(hwnd, wszErrMsg.c_str(), (LPCWSTR)L"Warning", MB_ICONWARNING);
        return false;
    }

    return true;   // data is good, allow the save to continue
}


// ========================================================================================
// Create the Shares/Futures trade transaction data and save it to the database
// ========================================================================================
void TradeDialog_CreateSharesTradeData(HWND hwnd)
{
    // PROCEED TO SAVE THE TRADE DATA
    tws_PauseTWS();

    std::shared_ptr<Trade> trade;

    if (IsNewSharesTradeAction(tradeAction) == true) {
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
    
    if (tradeAction == TradeAction::NewSharesTrade) trans->underlying = L"SHARES";
    if (tradeAction == TradeAction::NewFuturesTrade) trans->underlying = L"FUTURES";
    if (tradeAction == TradeAction::ManageShares) trans->underlying = L"SHARES";
    if (tradeAction == TradeAction::ManageFutures) trans->underlying = L"FUTURES";
    if (tradeAction == TradeAction::AddSharesToTrade) trans->underlying = L"SHARES";
    if (tradeAction == TradeAction::AddFuturesToTrade) trans->underlying = L"FUTURES";

    trans->quantity = stoi(AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTQUANTITY)));
    trans->price = stod(AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTPRICE)));
    trans->multiplier = stod(AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTMULTIPLIER)));
    trans->fees = stod(AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTFEES)));
    trade->transactions.push_back(trans);

    std::wstring DRCR = CustomLabel_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_COMBODRCR));
    trans->total = stod(AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTTOTAL)));
    if (DRCR == L"DR") { trans->total = trans->total * -1; }
    trade->ACB = trade->ACB + trans->total;


    std::shared_ptr<Leg> leg = std::make_shared<Leg>();
    leg->underlying = trans->underlying;

    // Set the Share/Futures quantity based on whether Long or Short based on 
    // the IDC_TRADEDIALOG_BUYSHARES or IDC_TRADEDIALOG_SELLSHARES button.

    if (IsNewSharesTradeAction(tradeAction) == true ||
        tradeAction == TradeAction::AddSharesToTrade ||
        tradeAction == TradeAction::AddFuturesToTrade) {
        int sel = CustomLabel_GetUserDataInt(GetDlgItem(hwnd, IDC_TRADEDIALOG_BUYSHARES));
        if (sel == (int)LongShort::Long) {
            leg->origQuantity = trans->quantity;
            leg->openQuantity = trans->quantity;
            leg->action = L"BTO";
        }
        else {
            if (sel == (int)LongShort::Short) {
                leg->origQuantity = trans->quantity * -1;
                leg->openQuantity = trans->quantity * -1;
                leg->action = L"STO";
            }
        }
    }
    
    if (tradeAction == TradeAction::ManageShares ||
        tradeAction == TradeAction::ManageFutures) {
        int sel = CustomLabel_GetUserDataInt(GetDlgItem(hwnd, IDC_TRADEDIALOG_SELLSHARES));
        if (sel == (int)LongShort::Long) {
            leg->origQuantity = trans->quantity;
            leg->openQuantity = trans->quantity;
            leg->action = L"BTC";
        }
        else {
            if (sel == (int)LongShort::Short) {
                leg->origQuantity = trans->quantity * -1;
                leg->openQuantity = trans->quantity * -1;
                leg->action = L"STC";
            }
        }
    }

    trans->legs.push_back(leg);


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
// Perform error checks on the Options trade data prior to allowing the save to the database.
// ========================================================================================
bool TradeDialog_ValidateOptionsTradeData(HWND hwnd)
{

    // Do an error check to ensure that the data about to be saved does not contain
    // any missing data, etc.
    std::wstring wszErrMsg;
    std::wstring wszText;

    if (IsNewOptionsTradeAction(tradeAction) == true) {
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
// Create the Options trade transaction data and save it to the database
// ========================================================================================
void TradeDialog_CreateOptionsTradeData(HWND hwnd)
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

    if (IsNewOptionsTradeAction(tradeAction) == true) {
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

        case TradeAction::NewOptionsTrade:
        case TradeAction::NewShortStrangle:
        case TradeAction::NewShortPut:
        case TradeAction::NewShortCall:
        case TradeAction::AddOptionsToTrade:
        case TradeAction::AddPutToTrade:
        case TradeAction::AddCallToTrade:
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


