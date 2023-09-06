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
#include "ActiveTrades/ActiveTrades.h"
#include "MainWindow/tws-client.h"
#include "Database/database.h"
#include "Category/Category.h"
#include "Strategy/StrategyButton.h"


class CGuiData
{
public:
    std::wstring tickerSymbol;
    std::wstring tickerName;
    std::wstring futureExpiry;
    int category = 0;
    std::wstring transDate;
    std::wstring description;
    std::wstring underlying;
    int quantity = 0;
    double price = 0;
    double multiplier = 0;
    double fees = 0;
    double TradeBP = 0;
    std::wstring DRCR;
    double total = 0;
    double ACB = 0;

    std::vector<Leg> legs;
    std::vector<Leg> legsRoll;


    CGuiData() {
        HWND hwnd = HWND_TRADEDIALOG;

        // Do Total calculation because it is possible that the user did not move off of
        // the Fees textbox thereby not firing the KillFocus that triggers the calculation.
        TradeDialog_CalculateTradeTotal(hwnd);

        tickerSymbol = RemovePipeChar(AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTTICKER)));
        tickerName = RemovePipeChar(AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTCOMPANY)));
        futureExpiry = CustomLabel_GetUserData(GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLCONTRACTDATE));
        category = CategoryControl_GetSelectedIndex(GetDlgItem(hwnd, IDC_TRADEDIALOG_CATEGORY));

        transDate = CustomLabel_GetUserData(GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLTRANSDATE));

        description = RemovePipeChar(AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTDESCRIBE)));
        if (tdd.tradeAction == TradeAction::RollLeg) description = L"Roll";
        if (tdd.tradeAction == TradeAction::CloseLeg) description = L"Close";
        if (tdd.tradeAction == TradeAction::NewSharesTrade ||
            tdd.tradeAction == TradeAction::ManageShares ||
            tdd.tradeAction == TradeAction::AddSharesToTrade) {
            description = L"Shares";
        }
        if (tdd.tradeAction == TradeAction::NewFuturesTrade ||
            tdd.tradeAction == TradeAction::ManageFutures ||
            tdd.tradeAction == TradeAction::AddFuturesToTrade) {
            description = L"Futures";
        }

        underlying = L"OPTIONS";
        if (tdd.tradeAction == TradeAction::NewSharesTrade) underlying = L"SHARES";
        if (tdd.tradeAction == TradeAction::NewFuturesTrade) underlying = L"FUTURES";
        if (tdd.tradeAction == TradeAction::ManageShares) underlying = L"SHARES";
        if (tdd.tradeAction == TradeAction::ManageFutures) underlying = L"FUTURES";
        if (tdd.tradeAction == TradeAction::AddSharesToTrade) underlying = L"SHARES";
        if (tdd.tradeAction == TradeAction::AddFuturesToTrade) underlying = L"FUTURES";

        quantity = AfxValInteger(AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTQUANTITY)));
        price = AfxValDouble(AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTPRICE)));
        multiplier = AfxValDouble(AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTMULTIPLIER)));
        fees = AfxValDouble(AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTFEES)));
        TradeBP = AfxValDouble(AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTTRADEBP)));

        DRCR = CustomLabel_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_COMBODRCR));
        total = AfxValDouble(AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTTOTAL)));
        if (DRCR == L"DR") { total = total * -1; }
        ACB = ACB + total;

        for (int row = 0; row < 4; ++row) {
            Leg leg;
            leg.original_quantity = AfxValInteger(TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDMAIN), row, 0));
            leg.expiry_date = TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDMAIN), row, 1);
            leg.strike_price = TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDMAIN), row, 3);
            leg.PutCall = TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDMAIN), row, 4);
            leg.action = TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDMAIN), row, 5);
            legs.push_back(leg);
        }

        for (int row = 0; row < 4; ++row) {
            Leg leg;
            leg.original_quantity = AfxValInteger(TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDROLL), row, 0));
            leg.expiry_date = TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDROLL), row, 1);
            leg.strike_price = TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDROLL), row, 3);
            leg.PutCall = TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDROLL), row, 4);
            leg.action = TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDROLL), row, 5);
            legsRoll.push_back(leg);
        }

    }

};



// ========================================================================================
// Remove pipe character from the string and return new copy. 
// ========================================================================================
std::wstring RemovePipeChar(const std::wstring& text)
{
    std::wstring wszString = text;
    wszString.erase(remove(wszString.begin(), wszString.end(), L'|'), wszString.end());
    return wszString;
}


// ========================================================================================
// Perform error checks on the Shares/Futures trade data prior to allowing the save 
// to the database.
// ========================================================================================
bool TradeDialog_ValidateSharesTradeData(HWND hwnd)
{
    // Collect the GUI data as it currently exists
    CGuiData guiData;

    // Do an error check to ensure that the data about to be saved does not contain
    // any missing data, etc.
    std::wstring wszErrMsg;
    std::wstring text;

    if (tdd.tradeAction == TradeAction::ManageFutures ||
        tdd.tradeAction == TradeAction::AddFuturesToTrade) {
        if (guiData.tickerSymbol.substr(0, 1) != L"/") wszErrMsg += L"- Invalid Futures Ticker Symbol.\n";
    }

    if (guiData.tickerSymbol.length() == 0) wszErrMsg += L"- Missing Ticker Symbol.\n";
    if (guiData.tickerName.length() == 0) wszErrMsg += L"- Missing Company or Futures Name.\n";
    if (guiData.description.length() == 0) wszErrMsg += L"- Missing Description.\n";
    if (guiData.quantity == 0) wszErrMsg += L"- Missing Quantity.\n";

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

    // Collect the GUI data as it currently exists
    CGuiData guiData;

    std::shared_ptr<Trade> trade;

    if (IsNewSharesTradeAction(tdd.tradeAction) == true) {
        trade = std::make_shared<Trade>();
        trades.push_back(trade);
    }
    else {
        trade = tdd.trade;
    }

    trade->ticker_symbol = guiData.tickerSymbol;
    trade->ticker_name   = guiData.tickerName;
    trade->future_expiry = guiData.futureExpiry;
    trade->category     = guiData.category;
    trade->trade_bp      = guiData.TradeBP;
    trade->acb          = guiData.ACB;

    std::shared_ptr<Transaction> trans = std::make_shared<Transaction>();
    trans->trans_date    = guiData.transDate;
    trans->description  = guiData.description;
    trans->underlying   = guiData.underlying;
    trans->quantity     = guiData.quantity; 
    trans->price        = guiData.price;
    trans->multiplier   = guiData.multiplier;
    trans->fees         = guiData.fees;
    trans->total        = guiData.total;
    trade->transactions.push_back(trans);

    std::shared_ptr<Leg> leg = std::make_shared<Leg>();
    leg->underlying = trans->underlying;

    // Determine earliest and latest dates for BP ROI calculation.
    std::wstring date_text = AfxRemoveDateHyphens(trans->trans_date);
    if (AfxValDouble(date_text) < AfxValDouble(trade->bp_start_date)) trade->bp_start_date = date_text;
    if (AfxValDouble(date_text) > AfxValDouble(trade->bp_end_date)) trade->bp_end_date = date_text;
    if (AfxValDouble(date_text) > AfxValDouble(trade->oldest_trade_trans_date)) trade->oldest_trade_trans_date = date_text;

    // Set the Share/Futures quantity based on whether Long or Short based on 
    // the IDC_TRADEDIALOG_BUYSHARES or IDC_TRADEDIALOG_SELLSHARES button.

    if (IsNewSharesTradeAction(tdd.tradeAction) == true ||
        tdd.tradeAction == TradeAction::AddSharesToTrade ||
        tdd.tradeAction == TradeAction::AddFuturesToTrade) {
        int sel = CustomLabel_GetUserDataInt(GetDlgItem(hwnd, IDC_TRADEDIALOG_BUYSHARES));
        if (sel == (int)LongShort::Long) {
            leg->original_quantity = trans->quantity;
            leg->open_quantity = trans->quantity;
            leg->strike_price = std::to_wstring(trans->price);
            leg->action = L"BTO";
        }
        else {
            if (sel == (int)LongShort::Short) {
                leg->original_quantity = trans->quantity * -1;
                leg->open_quantity = trans->quantity * -1;
                leg->strike_price = std::to_wstring(trans->price);
                leg->action = L"STO";
            }
        }
    }
    
    if (tdd.tradeAction == TradeAction::ManageShares ||
        tdd.tradeAction == TradeAction::ManageFutures) {
        int sel = CustomLabel_GetUserDataInt(GetDlgItem(hwnd, IDC_TRADEDIALOG_SELLSHARES));
        if (sel == (int)LongShort::Long) {
            leg->original_quantity = trans->quantity;
            leg->open_quantity = trans->quantity;
            leg->action = L"BTC";
        }
        else {
            if (sel == (int)LongShort::Short) {
                leg->original_quantity = trans->quantity * -1;
                leg->open_quantity = trans->quantity * -1;
                leg->action = L"STC";
            }
        }
    }

    trans->legs.push_back(leg);


    // Set the open status of the entire trade based on the new modified legs
    trade->SetTradeOpenStatus();

    // Rebuild the openLegs position vector
    trade->CreateOpenLegsVector();

    // Recalculate the ACB for the trade
    trade->acb = 0;
    for (const auto trans : trade->transactions) {
        trade->acb = trade->acb + trans->total;
    }

    // Save the new data
    SaveDatabase();


    // Show our new list of open trades
    // Destroy any existing ListBox line data
    // This will also clear the LineData pointers and cancel any previous market data
    ListBoxData_DestroyItemData(GetDlgItem(HWND_ACTIVETRADES, IDC_TRADES_LISTBOX));
    ActiveTrades_ShowActiveTrades();

    tdd.trade = nullptr;

    tws_ResumeTWS();

}


// ========================================================================================
// Perform error checks on the Options trade data prior to allowing the save to the database.
// ========================================================================================
bool TradeDialog_ValidateOptionsTradeData(HWND hwnd)
{
    // Collect the GUI data as it currently exists
    CGuiData guiData;

    // Do an error check to ensure that the data about to be saved does not contain
    // any missing data, etc.
    std::wstring wszErrMsg;
    std::wstring text;

    if (guiData.tickerSymbol.length() == 0) wszErrMsg += L"- Missing Ticker Symbol.\n";
    if (guiData.tickerName.length() == 0) wszErrMsg += L"- Missing Company or Futures Name.\n";
    if (guiData.description.length() == 0) wszErrMsg += L"- Missing Description.\n";

    // In adition to validating each leg, we count the number of non blank legs. If all legs
    // are blank then there is nothing to save to add that to the error message.
    int NumBlankLegs = 0;

    for (int row = 0; row < 4; ++row) {
        // All strings must be zero length in order to skip it from being included in the transaction. 
        if (guiData.legs.at(row).original_quantity == 0 && 
            guiData.legs.at(row).expiry_date.length() == 0 && 
            guiData.legs.at(row).strike_price.length() == 0 && 
            guiData.legs.at(row).PutCall.length() == 0 && 
            guiData.legs.at(row).action.length() == 0) {
            NumBlankLegs++;
            continue;
        }

        // If any of the strings are zero length at this point then the row has incomplete data.
        bool bIncomplete = false;

        if (guiData.legs.at(row).original_quantity == 0) bIncomplete = true;
        if (guiData.legs.at(row).expiry_date.length() == 0) bIncomplete = true;
        if (guiData.legs.at(row).strike_price.length() == 0) bIncomplete = true;
        if (guiData.legs.at(row).PutCall.length() == 0) bIncomplete = true;
        if (guiData.legs.at(row).action.length() == 0) bIncomplete = true;

        if (bIncomplete == true) {
            wszErrMsg += L"- Leg #" + std::to_wstring(row + 1) + L" has incomplete or missing data.\n";
        }
    }

    if (NumBlankLegs == 4) {
        wszErrMsg += L"- No Legs exist to be saved.\n";
    }

    if (tdd.tradeAction == TradeAction::RollLeg) {
        for (int row = 0; row < 4; ++row) {
            if (guiData.legsRoll.at(row).original_quantity == 0 &&
                guiData.legsRoll.at(row).expiry_date.length() == 0 &&
                guiData.legsRoll.at(row).strike_price.length() == 0 &&
                guiData.legsRoll.at(row).PutCall.length() == 0 &&
                guiData.legsRoll.at(row).action.length() == 0) {
                NumBlankLegs++;
                continue;
            }

            // If any of the strings are zero length at this point then the row has incomplete data.
            bool bIncomplete = false;

            if (guiData.legsRoll.at(row).original_quantity == 0) bIncomplete = true;
            if (guiData.legsRoll.at(row).expiry_date.length() == 0) bIncomplete = true;
            if (guiData.legsRoll.at(row).strike_price.length() == 0) bIncomplete = true;
            if (guiData.legsRoll.at(row).PutCall.length() == 0) bIncomplete = true;
            if (guiData.legsRoll.at(row).action.length() == 0) bIncomplete = true;

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

    // Collect the GUI data as it currently exists
    CGuiData guiData;

    std::shared_ptr<Trade> trade;

    if (IsNewOptionsTradeAction(tdd.tradeAction) == true) {
        trade = std::make_shared<Trade>();
        trades.push_back(trade);
    }
    else {
        trade = tdd.trade;
    }

    trade->ticker_symbol = guiData.tickerSymbol;
    trade->ticker_name   = guiData.tickerName;
    trade->future_expiry = guiData.futureExpiry;
    trade->category     = guiData.category;
    trade->acb          = guiData.ACB;
    trade->trade_bp      = guiData.TradeBP;

    std::shared_ptr<Transaction> trans = std::make_shared<Transaction>();
    trans->trans_date    = guiData.transDate;
    trans->description  = guiData.description;
    trans->underlying   = guiData.underlying;
    trans->quantity     = guiData.quantity;
    trans->price        = guiData.price;
    trans->multiplier   = guiData.multiplier;
    trans->fees         = guiData.fees;
    trans->total        = guiData.total;
    trade->transactions.push_back(trans);

    // Determine earliest and latest dates for BP ROI calculation.
    std::wstring date_text = AfxRemoveDateHyphens(trans->trans_date);
    if (AfxValDouble(date_text) < AfxValDouble(trade->bp_start_date)) trade->bp_start_date = date_text;
    if (AfxValDouble(date_text) > AfxValDouble(trade->bp_end_date)) trade->bp_end_date = date_text;
    if (AfxValDouble(date_text) > AfxValDouble(trade->oldest_trade_trans_date)) trade->oldest_trade_trans_date = date_text;

    // Add the new transaction legs
    for (int row = 0; row < 4; ++row) {
        std::shared_ptr<Leg> leg = std::make_shared<Leg>();

        trade->nextleg_id += 1;
        leg->leg_id       = trade->nextleg_id;
        leg->underlying  = trans->underlying;
        leg->expiry_date  = guiData.legs.at(row).expiry_date;
        leg->strike_price = guiData.legs.at(row).strike_price;
        leg->PutCall     = guiData.legs.at(row).PutCall;
        leg->action      = guiData.legs.at(row).action;
        leg->trans       = trans;
        int intQuantity  = guiData.legs.at(row).original_quantity * trans->quantity;
        if (intQuantity == 0) continue;

        std::wstring wszexpiry_date = AfxRemoveDateHyphens(leg->expiry_date);
        if (AfxValDouble(wszexpiry_date) > AfxValDouble(trade->bp_end_date)) trade->bp_end_date = wszexpiry_date;

        switch (tdd.tradeAction) {

        case TradeAction::NewOptionsTrade:
        case TradeAction::NewIronCondor:
        case TradeAction::NewShortLT112:
        case TradeAction::NewShortStrangle:
        case TradeAction::NewShortPut:
        case TradeAction::NewShortCall:
        case TradeAction::AddOptionsToTrade:
        case TradeAction::AddPutToTrade:
        case TradeAction::AddCallToTrade:
            leg->original_quantity = intQuantity;
            leg->open_quantity = intQuantity;
            break;

        case TradeAction::CloseLeg:
        case TradeAction::RollLeg:
            leg->original_quantity = guiData.legs.at(row).original_quantity;
            leg->open_quantity = 0;
            // Update the original transaction being Closed quantities
            if (!tdd.legs.empty()) {
                tdd.legs.at(row)->open_quantity += guiData.legs.at(row).original_quantity;
                leg->leg_back_pointer_id = tdd.legs.at(row)->leg_id;
            }
            break;
        }

        trans->legs.push_back(leg);

    }

    // Add legs for rolled portion of the transaction
    if (tdd.tradeAction == TradeAction::RollLeg) {

        for (int row = 0; row < 4; ++row) {
            std::shared_ptr<Leg> leg = std::make_shared<Leg>();

            trade->nextleg_id += 1;
            leg->leg_id       = trade->nextleg_id;
            leg->underlying  = trans->underlying;
            leg->expiry_date  = guiData.legsRoll.at(row).expiry_date;
            leg->strike_price = guiData.legsRoll.at(row).strike_price;
            leg->PutCall     = guiData.legsRoll.at(row).PutCall;
            leg->action      = guiData.legsRoll.at(row).action;
            leg->trans       = trans;
            int intQuantity  = guiData.legsRoll.at(row).original_quantity;
            if (intQuantity == 0) continue;

            std::wstring wszexpiry_date = AfxRemoveDateHyphens(leg->expiry_date);
            if (AfxValDouble(wszexpiry_date) > AfxValDouble(trade->bp_end_date)) trade->bp_end_date = wszexpiry_date;

            leg->original_quantity = intQuantity;
            leg->open_quantity = intQuantity;

            trans->legs.push_back(leg);
        }

    }

    // Set the open status of the entire trade based on the new modified legs
    trade->SetTradeOpenStatus();

    // Rebuild the openLegs position vector
    trade->CreateOpenLegsVector();

    // Recalculate the ACB for the trade
    trade->acb = 0;
    for (const auto trans : trade->transactions) {
        trade->acb = trade->acb + trans->total;
    }

    // Save the new data
    SaveDatabase();


    // Show our new list of open trades
    // Destroy any existing ListBox line data
    // This will also clear the LineData pointers and cancel any previous market data
    ListBoxData_DestroyItemData(GetDlgItem(HWND_ACTIVETRADES, IDC_TRADES_LISTBOX));
    ActiveTrades_ShowActiveTrades();

    tdd.trade = nullptr;

    tws_ResumeTWS();

}


// ========================================================================================
// Perform error checks on the EDIT Options trade data prior to allowing the save to the database.
// ========================================================================================
bool TradeDialog_ValidateEditTradeData(HWND hwnd)
{

    // Collect the GUI data as it currently exists
    CGuiData guiData;

    // Do an error check to ensure that the data about to be saved does not contain
    // any missing data, etc.
    std::wstring wszErrMsg;
    std::wstring text;

    if (guiData.description.length() == 0) wszErrMsg += L"- Missing Description.\n";

    // In adition to validating each leg, we count the number of non blank legs. If all legs
    // are blank then there is nothing to save to add that to the error message.
    int NumBlankLegs = 0;

    for (int row = 0; row < 4; ++row) {
        // All strings must be zero length in order to skip it from being included in the transaction. 
        if (guiData.legs.at(row).original_quantity == 0 &&
            guiData.legs.at(row).open_quantity == 0 &&
            guiData.legs.at(row).expiry_date.length() == 0 &&
            guiData.legs.at(row).strike_price.length() == 0 &&
            guiData.legs.at(row).PutCall.length() == 0 &&
            guiData.legs.at(row).action.length() == 0) {
            NumBlankLegs++;
            continue;
        }

        // If any of the strings are zero length at this point then the row has incompete data.
        bool bIncomplete = false;

        if (guiData.legs.at(row).original_quantity == 0) bIncomplete = true;
        if (guiData.legs.at(row).expiry_date.length() == 0) bIncomplete = true;
        if (guiData.legs.at(row).strike_price.length() == 0) bIncomplete = true;
        if (guiData.legs.at(row).PutCall.length() == 0) bIncomplete = true;
        if (guiData.legs.at(row).action.length() == 0) bIncomplete = true;

        if (bIncomplete == true) {
            wszErrMsg += L"- Leg #" + std::to_wstring(row + 1) + L" has incomplete or missing data.\n";
        }
    }

    if (NumBlankLegs == 4) {
        wszErrMsg += L"- No Legs exist to be saved.\n";
    }

    for (int row = 0; row < 4; ++row) {
        // All strings must be zero length in order to skip it from being included in the transaction. 
        if (guiData.legsRoll.at(row).original_quantity == 0 &&
            guiData.legsRoll.at(row).open_quantity == 0 &&
            guiData.legsRoll.at(row).expiry_date.length() == 0 &&
            guiData.legsRoll.at(row).strike_price.length() == 0 &&
            guiData.legsRoll.at(row).PutCall.length() == 0 &&
            guiData.legsRoll.at(row).action.length() == 0) {
            NumBlankLegs++;
            continue;
        }

        // If any of the strings are zero length at this point then the row has incompete data.
        bool bIncomplete = false;

        if (guiData.legsRoll.at(row).original_quantity == 0) bIncomplete = true;
        if (guiData.legsRoll.at(row).expiry_date.length() == 0) bIncomplete = true;
        if (guiData.legsRoll.at(row).strike_price.length() == 0) bIncomplete = true;
        if (guiData.legsRoll.at(row).PutCall.length() == 0) bIncomplete = true;
        if (guiData.legsRoll.at(row).action.length() == 0) bIncomplete = true;

        if (bIncomplete == true) {
            wszErrMsg += L"- Leg #" + std::to_wstring(row + 5) + L" has incomplete or missing data.\n";
        }
    }

    if (wszErrMsg.length()) {
        MessageBox(hwnd, wszErrMsg.c_str(), (LPCWSTR)L"Warning", MB_ICONWARNING);
        return false;
    }

    return true;   // data is good, allow the save to continue
}


// ========================================================================================
// Create the EDIT Options trade transaction data and save it to the database
// ========================================================================================
void TradeDialog_CreateEditTradeData(HWND hwnd)
{
    // PROCEED TO SAVE THE TRADE DATA
    tws_PauseTWS();

    // Collect the GUI data as it currently exists
    CGuiData guiData;

    // Save the modified data
    tdd.trade->future_expiry = guiData.futureExpiry;
    tdd.trade->category = guiData.category;
    
    tdd.trans->trans_date   = guiData.transDate;
    tdd.trans->description = guiData.description;
    tdd.trans->underlying  = guiData.underlying;
    tdd.trans->quantity    = guiData.quantity;
    tdd.trans->price       = guiData.price;
    tdd.trans->multiplier  = guiData.multiplier;
    tdd.trans->fees        = guiData.fees;
    tdd.trade->trade_bp     = guiData.TradeBP;
    tdd.trans->total       = guiData.total;

    // Determine earliest and latest dates for BP ROI calculation.
    std::wstring date_text = AfxRemoveDateHyphens(tdd.trans->trans_date);
    if (AfxValDouble(date_text) < AfxValDouble(tdd.trade->bp_start_date)) tdd.trade->bp_start_date = date_text;
    if (AfxValDouble(date_text) > AfxValDouble(tdd.trade->bp_end_date)) tdd.trade->bp_end_date = date_text;
    if (AfxValDouble(date_text) > AfxValDouble(tdd.trade->oldest_trade_trans_date)) tdd.trade->oldest_trade_trans_date = date_text;

    std::vector<int> legsToDelete;

    // Cycle through both grids and add changes to the legs
    for (int row = 0; row < 8; ++row) {
        HWND hGrid = (row < 4)
            ? GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDMAIN)
            : GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDROLL);

        std::wstring legoriginal_quantity = TradeGrid_GetText(hGrid, row, 0);
        std::wstring legopen_quantity = TradeGrid_GetText(hGrid, row, 1);
        std::wstring legExpiry = TradeGrid_GetText(hGrid, row, 2);
        std::wstring legStrike = TradeGrid_GetText(hGrid, row, 4);
        std::wstring legPutCall = TradeGrid_GetText(hGrid, row, 5);
        std::wstring legAction = TradeGrid_GetText(hGrid, row, 6);

            
        // Nothing new or changed to add? Just iterate to next line.
        if (legoriginal_quantity.length() == 0) {
            // If row from original legs has been deleted then simply remove it from the 
            // legs vector.
            if (row < (int)tdd.trans->legs.size()) {
                legsToDelete.push_back(row);
            }
            continue;
        }

        int intoriginal_quantity = AfxValInteger(legoriginal_quantity);   // will GPF if empty legoriginal_quantity string
        if (intoriginal_quantity == 0) continue;

        int intopen_quantity = AfxValInteger(legopen_quantity);   // will GPF if empty legopen_quantity string
        if (intopen_quantity == 0) continue;


        // Determine if backpointers exist and if yes then re-use that leg instead
        // of creating a new leg in order to preserve the integrity of the data.
        std::shared_ptr<Leg> leg = nullptr;
        
        // Reuse existing leg
        if (row < (int)tdd.trans->legs.size()) {
            leg = tdd.trans->legs.at(row);
        }
        else {
            leg = std::make_shared<Leg>();
            tdd.trade->nextleg_id += 1;
            leg->leg_id = tdd.trade->nextleg_id;
        }

        leg->original_quantity = intoriginal_quantity * tdd.trans->quantity;
        leg->open_quantity = intopen_quantity * tdd.trans->quantity;

        leg->underlying = tdd.trans->underlying;
        leg->expiry_date = legExpiry;
        leg->strike_price = legStrike;
        leg->PutCall = legPutCall;
        leg->action = legAction;

        std::wstring wszexpiry_date = AfxRemoveDateHyphens(legExpiry);
        if (AfxValDouble(wszexpiry_date) > AfxValDouble(tdd.trade->bp_end_date)) tdd.trade->bp_end_date = wszexpiry_date;

        if (row >= (int)tdd.trans->legs.size()) {
            tdd.trans->legs.push_back(leg);
        }
    }

    // Remove any rows from the original legs that were deleted in full
    for (auto idx : legsToDelete) {
        tdd.trans->legs.erase(tdd.trans->legs.begin() + idx);
    }

    // Set the open status of the entire trade based on the new modified legs
    tdd.trade->SetTradeOpenStatus();

    // Rebuild the openLegs position vector
    tdd.trade->CreateOpenLegsVector();

    // Recalculate the ACB for the trade
    tdd.trade->acb = 0;
    for (const auto trans : tdd.trade->transactions) {
        tdd.trade->acb = tdd.trade->acb + trans->total;
    }

    // Save/Load the new data
    SaveDatabase();
    LoadDatabase();

    // Show our new list of open trades
    // Destroy any existing ListBox line data
    // This will also clear the LineData pointers and cancel any previous market data
    ListBoxData_DestroyItemData(GetDlgItem(HWND_ACTIVETRADES, IDC_TRADES_LISTBOX));
    ActiveTrades_ShowActiveTrades();

    tdd.trade = nullptr;
    
    tws_ResumeTWS();

}

