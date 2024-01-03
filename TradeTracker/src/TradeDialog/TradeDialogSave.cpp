/*

MIT License

Copyright(c) 2023-2024 Paul Squires

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
#include "Database/database.h"
#include "CustomLabel/CustomLabel.h"
#include "CustomTextBox/CustomTextBox.h"
#include "CustomMessageBox/CustomMessageBox.h"
#include "TradeGrid/TradeGrid.h"
#include "ActiveTrades/ActiveTrades.h"
#include "MainWindow/tws-client.h"
#include "Category/Category.h"
#include "Strategy/StrategyButton.h"


class CGuiData
{
public:
    std::wstring ticker_symbol;
    std::wstring ticker_name;
    std::wstring future_expiry;
    int category = 0;
    std::wstring trans_date;
    std::wstring description;
    Underlying underlying = Underlying::Nothing;
    int quantity = 0;
    double price = 0;
    double multiplier = 0;
    double fees = 0;
    double trade_bp = 0;
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

        ticker_symbol = RemovePipeChar(CustomTextBox_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTTICKER)));
        ticker_name   = RemovePipeChar(CustomTextBox_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTCOMPANY)));
        future_expiry = CustomLabel_GetUserData(GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLCONTRACTDATE));
        
        category      = CategoryControl_GetSelectedIndex(GetDlgItem(hwnd, IDC_TRADEDIALOG_CATEGORY));
        most_recently_used_category = category;

        trans_date    = CustomLabel_GetUserData(GetDlgItem(hwnd, IDC_TRADEDIALOG_LBLTRANSDATE));
        description   = RemovePipeChar(CustomTextBox_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTDESCRIBE)));
        
        if (tdd.trade_action == TradeAction::new_shares_trade ||
            tdd.trade_action == TradeAction::manage_shares ||
            tdd.trade_action == TradeAction::add_shares_to_trade) {
            description = L"Shares";
        }
        if (tdd.trade_action == TradeAction::new_futures_trade ||
            tdd.trade_action == TradeAction::manage_futures ||
            tdd.trade_action == TradeAction::add_futures_to_trade) {
            description = L"Futures";
        }
        if (tdd.trade_action == TradeAction::add_dividend_to_trade) {
            description = L"Dividend";
        }
         
        underlying = Underlying::Options;
        if (tdd.trade_action == TradeAction::new_shares_trade) underlying = Underlying::Shares;
        if (tdd.trade_action == TradeAction::new_futures_trade) underlying = Underlying::Futures;
        if (tdd.trade_action == TradeAction::manage_shares) underlying = Underlying::Shares;
        if (tdd.trade_action == TradeAction::manage_futures) underlying = Underlying::Futures;
        if (tdd.trade_action == TradeAction::add_shares_to_trade) underlying = Underlying::Shares;
        if (tdd.trade_action == TradeAction::add_futures_to_trade) underlying = Underlying::Futures;
        if (tdd.trade_action == TradeAction::other_income_expense) underlying = Underlying::Other;
        if (tdd.trade_action == TradeAction::add_dividend_to_trade) underlying = Underlying::Dividend;
        if (tdd.trans) underlying = tdd.trans->underlying;

        quantity   = (int)AfxValDouble(CustomTextBox_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTQUANTITY)));
        price      = AfxValDouble(CustomTextBox_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTPRICE)));
        multiplier = AfxValDouble(CustomTextBox_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTMULTIPLIER)));
        fees       = AfxValDouble(CustomTextBox_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTFEES)));
        trade_bp   = AfxValDouble(CustomTextBox_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTTRADEBP)));
        DRCR       = CustomLabel_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_COMBODRCR));
        total      = AfxValDouble(CustomTextBox_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTTOTAL)));
        
        if (DRCR == L"DR") { total = total * -1; }
        ACB = ACB + total;

        int col = 0;
        for (int row = 0; row < 4; ++row) {
            Leg leg;
            col = 0;
            leg.original_quantity = AfxValInteger(TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDMAIN), row, col));
            if (tdd.trade_action == TradeAction::edit_transaction) {
                col += 1;
                leg.open_quantity = AfxValInteger(TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDMAIN), row, col));
            }
            col += 1;
            leg.expiry_date = TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDMAIN), row, col);
            col += 2;
            leg.strike_price = TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDMAIN), row, col);
            col += 1;
            leg.put_call = db.StringToPutCall(TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDMAIN), row, col));
            col += 1;
            leg.action = db.StringDescriptionToAction(TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDMAIN), row, col));
            legs.push_back(leg);
        }

        col = 0;
        for (int row = 0; row < 4; ++row) {
            Leg leg;
            col = 0;
            leg.original_quantity = AfxValInteger(TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDROLL), row, col));
            if (tdd.trade_action == TradeAction::edit_transaction) {
                col += 1;
                leg.open_quantity = AfxValInteger(TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDROLL), row, col));
            }
            col += 1;
            leg.expiry_date = TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDROLL), row, col);
            col += 2;
            leg.strike_price = TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDROLL), row, col);
            col += 1;
            leg.put_call = db.StringToPutCall(TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDROLL), row, col));
            col += 1;
            leg.action = db.StringDescriptionToAction(TradeGrid_GetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TABLEGRIDROLL), row, col));
            legsRoll.push_back(leg);
        }

    }

};



// ========================================================================================
// Remove pipe character from the string and return new copy. 
// ========================================================================================
std::wstring RemovePipeChar(const std::wstring& text)
{
    std::wstring temp = text;
    temp.erase(remove(temp.begin(), temp.end(), L'|'), temp.end());
    return temp;
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
    std::wstring error_message;
    std::wstring text;

    if (tdd.trade_action == TradeAction::manage_futures ||
        tdd.trade_action == TradeAction::add_futures_to_trade) {
        if (guiData.ticker_symbol.substr(0, 1) != L"/") error_message += L"- Invalid Futures Ticker Symbol.\n";
    }

    if (guiData.ticker_symbol.length() == 0) error_message += L"- Missing Ticker Symbol.\n";
    if (guiData.ticker_name.length() == 0) error_message += L"- Missing Company or Futures Name.\n";
    if (guiData.description.length() == 0) error_message += L"- Missing Description.\n";
    if (guiData.quantity == 0) error_message += L"- Missing Quantity.\n";

    if (error_message.length()) {
        CustomMessageBox.Show(hwnd, error_message, L"Warning", MB_ICONWARNING);
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

    // Collect the GUI data as it currently exists
    CGuiData guiData;

    std::shared_ptr<Trade> trade;

    if (ActiveTrades.IsNewSharesTradeAction(tdd.trade_action)) {
        trade = std::make_shared<Trade>();
        trades.push_back(trade);
    }
    else {
        trade = tdd.trade;
    }

    trade->ticker_symbol = guiData.ticker_symbol;
    trade->ticker_name   = guiData.ticker_name;
    trade->future_expiry = guiData.future_expiry;
    trade->category      = guiData.category;
    trade->trade_bp      = guiData.trade_bp;
    trade->acb           = guiData.ACB;

    std::shared_ptr<Transaction> trans = std::make_shared<Transaction>();
    trans->trans_date    = guiData.trans_date;
    trans->description   = guiData.description;
    trans->underlying    = guiData.underlying;
    trans->quantity      = guiData.quantity; 
    trans->price         = guiData.price;
    trans->multiplier    = guiData.multiplier;
    trans->fees          = guiData.fees;
    trans->total         = guiData.total;
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

    if (ActiveTrades.IsNewSharesTradeAction(tdd.trade_action) ||
        tdd.trade_action == TradeAction::add_shares_to_trade ||
        tdd.trade_action == TradeAction::add_futures_to_trade) {
        int sel = CustomLabel_GetUserDataInt(GetDlgItem(hwnd, IDC_TRADEDIALOG_BUYSHARES));
        if (sel == (int)LongShort::Long) {
            leg->original_quantity = trans->quantity;
            leg->open_quantity = trans->quantity;
            leg->strike_price = std::to_wstring(trans->price);
            leg->action = Action::BTO;
        }
        else {
            if (sel == (int)LongShort::Short) {
                leg->original_quantity = trans->quantity * -1;
                leg->open_quantity = trans->quantity * -1;
                leg->strike_price = std::to_wstring(trans->price);
                leg->action = Action::STO;
            }
        }
    }
    
    if (tdd.trade_action == TradeAction::manage_shares ||
        tdd.trade_action == TradeAction::manage_futures) {
        int sel = CustomLabel_GetUserDataInt(GetDlgItem(hwnd, IDC_TRADEDIALOG_SELLSHARES));
        if (sel == (int)LongShort::Long) {
            leg->original_quantity = trans->quantity;
            leg->open_quantity = trans->quantity;
            leg->action = Action::BTC;
        }
        else {
            if (sel == (int)LongShort::Short) {
                leg->original_quantity = trans->quantity * -1;
                leg->open_quantity = trans->quantity * -1;
                leg->action = Action::STC;
            }
        }
    }

    trans->legs.push_back(leg);

    tdd.trade = trade;
}



// ========================================================================================
// Perform error checks on the Other Income/Expense data prior to allowing the save 
// to the database.
// ========================================================================================
bool TradeDialog_ValidateOtherIncomeData(HWND hwnd)
{
    // Collect the GUI data as it currently exists
    CGuiData guiData;

    // Do an error check to ensure that the data about to be saved does not contain
    // any missing data, etc.
    std::wstring error_message;
    std::wstring text;

    if (guiData.description.length() == 0) error_message += L"- Missing Description.\n";
    if (guiData.quantity == 0) error_message += L"- Missing Quantity.\n";
    if (guiData.price == 0) error_message += L"- Amount can not be Zero.\n";

    if (error_message.length()) {
        CustomMessageBox.Show(hwnd, error_message, L"Warning", MB_ICONWARNING);
        return false;
    }

    return true;   // data is good, allow the save to continue
}


// ========================================================================================
// Create the Other Income/Expense transaction data and save it to the database
// ========================================================================================
void TradeDialog_CreateOtherIncomeData(HWND hwnd)
{
    // PROCEED TO SAVE THE OTHER INCOME/EXPENSE DATA

    // Collect the GUI data as it currently exists
    CGuiData guiData;

    std::shared_ptr<Trade> trade;

    trade = std::make_shared<Trade>();
    trades.push_back(trade);

    trade->ticker_symbol = L"OTHER";
    trade->ticker_name = L"Other Income/Expense";
    trade->future_expiry = guiData.future_expiry;
    trade->category = CATEGORY_OTHER;
    trade->trade_bp = guiData.trade_bp;
    trade->acb = guiData.ACB;

    std::shared_ptr<Transaction> trans = std::make_shared<Transaction>();
    trans->trans_date = guiData.trans_date;
    trans->description = guiData.description;
    trans->underlying = guiData.underlying;
    trans->quantity = guiData.quantity;
    trans->price = guiData.price;
    trans->multiplier = guiData.multiplier;
    trans->fees = guiData.fees;
    trans->total = guiData.total;
    trade->transactions.push_back(trans);

    std::shared_ptr<Leg> leg = std::make_shared<Leg>();
    leg->underlying = trans->underlying;

    leg->original_quantity = trans->quantity;
    leg->open_quantity = 0;
    leg->strike_price = std::to_wstring(trans->total);
    leg->action = Action::BTO;

    trans->legs.push_back(leg);

    tdd.trade = trade;
}



// ========================================================================================
// Perform error checks on the Dividend trade data prior to allowing the save 
// to the database.
// ========================================================================================
bool TradeDialog_ValidateDividendTradeData(HWND hwnd)
{
    // Collect the GUI data as it currently exists
    CGuiData guiData;

    // Do an error check to ensure that the data about to be saved does not contain
    // any missing data, etc.
    std::wstring error_message;
    std::wstring text;

    if (guiData.ticker_symbol.length() == 0) error_message += L"- Missing Ticker Symbol.\n";
    if (guiData.ticker_name.length() == 0) error_message += L"- Missing Company Name.\n";
    if (guiData.description.length() == 0) error_message += L"- Missing Description.\n";
    if (guiData.quantity == 0) error_message += L"- Missing Quantity.\n";
    if (guiData.price == 0) error_message += L"- Dividend Amount is Zero.\n";

    if (error_message.length()) {
        CustomMessageBox.Show(hwnd, error_message, L"Warning", MB_ICONWARNING);
        return false;
    }

    return true;   // data is good, allow the save to continue
}


// ========================================================================================
// Create the Dividend trade transaction data and save it to the database
// ========================================================================================
void TradeDialog_CreateDividendTradeData(HWND hwnd)
{
    // PROCEED TO SAVE THE TRADE DATA

    // Collect the GUI data as it currently exists
    CGuiData guiData;

    std::shared_ptr<Trade> trade;

    trade = tdd.trade;
    if (trade == nullptr) return;

    trade->ticker_symbol = guiData.ticker_symbol;
    trade->ticker_name = guiData.ticker_name;
    trade->future_expiry = guiData.future_expiry;
    trade->category = guiData.category;
    trade->trade_bp = guiData.trade_bp;
    trade->acb = guiData.ACB;

    std::shared_ptr<Transaction> trans = std::make_shared<Transaction>();
    trans->trans_date = guiData.trans_date;
    trans->description = guiData.description;
    trans->underlying = guiData.underlying;
    trans->quantity = guiData.quantity;
    trans->price = guiData.price;
    trans->multiplier = guiData.multiplier;
    trans->fees = guiData.fees;
    trans->total = guiData.total;
    trade->transactions.push_back(trans);

    std::shared_ptr<Leg> leg = std::make_shared<Leg>();
    leg->underlying = trans->underlying;

    // Determine earliest and latest dates for BP ROI calculation.
    std::wstring date_text = AfxRemoveDateHyphens(trans->trans_date);
    if (AfxValDouble(date_text) < AfxValDouble(trade->bp_start_date)) trade->bp_start_date = date_text;
    if (AfxValDouble(date_text) > AfxValDouble(trade->bp_end_date)) trade->bp_end_date = date_text;
    if (AfxValDouble(date_text) > AfxValDouble(trade->oldest_trade_trans_date)) trade->oldest_trade_trans_date = date_text;

    leg->original_quantity = trans->quantity;
    leg->open_quantity = 0;
    leg->strike_price = std::to_wstring(trans->total);
    leg->action = Action::BTO;

    trans->legs.push_back(leg);

    tdd.trade = trade;
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
    std::wstring error_message;
    std::wstring text;

    if (guiData.ticker_symbol.length() == 0) error_message += L"- Missing Ticker Symbol.\n";
    if (guiData.ticker_name.length() == 0) error_message += L"- Missing Company or Futures Name.\n";
    if (guiData.description.length() == 0) error_message += L"- Missing Description.\n";
    if (guiData.quantity == 0) error_message += L"- Missing Quantity.\n";

    // In adition to validating each leg, we count the number of non blank legs. If all legs
    // are blank then there is nothing to save to add that to the error message.
    int num_blank_legs = 0;

    for (int row = 0; row < 4; ++row) {
        // All strings must be zero length in order to skip it from being included in the transaction. 
        if (guiData.legs.at(row).original_quantity == 0 && 
            guiData.legs.at(row).expiry_date.length() == 0 && 
            guiData.legs.at(row).strike_price.length() == 0 && 
            guiData.legs.at(row).put_call == PutCall::Nothing && 
            guiData.legs.at(row).action == Action::Nothing) {
            num_blank_legs++;
            continue;
        }

        // If any of the strings are zero length at this point then the row has incomplete data.
        bool incomplete = false;

        if (guiData.legs.at(row).original_quantity == 0) incomplete = true;
        if (guiData.legs.at(row).expiry_date.length() == 0) incomplete = true;
        if (guiData.legs.at(row).strike_price.length() == 0) incomplete = true;
        if (guiData.legs.at(row).put_call == PutCall::Nothing) incomplete = true;
        if (guiData.legs.at(row).action == Action::Nothing) incomplete = true;

        if (incomplete == true) {
            error_message += L"- Leg #" + std::to_wstring(row + 1) + L" has incomplete or missing data.\n";
        }
    }

    if (num_blank_legs == 4) {
        error_message += L"- No Legs exist to be saved.\n";
    }

    if (tdd.trade_action == TradeAction::roll_leg) {
        for (int row = 0; row < 4; ++row) {
            if (guiData.legsRoll.at(row).original_quantity == 0 &&
                guiData.legsRoll.at(row).expiry_date.length() == 0 &&
                guiData.legsRoll.at(row).strike_price.length() == 0 &&
                guiData.legsRoll.at(row).put_call == PutCall::Nothing && 
                guiData.legsRoll.at(row).action == Action::Nothing) {
                num_blank_legs++;
                continue;
            }

            // If any of the strings are zero length at this point then the row has incomplete data.
            bool incomplete = false;

            if (guiData.legsRoll.at(row).original_quantity == 0) incomplete = true;
            if (guiData.legsRoll.at(row).expiry_date.length() == 0) incomplete = true;
            if (guiData.legsRoll.at(row).strike_price.length() == 0) incomplete = true;
            if (guiData.legsRoll.at(row).put_call == PutCall::Nothing) incomplete = true;
            if (guiData.legsRoll.at(row).action == Action::Nothing) incomplete = true;

            if (incomplete == true) {
                error_message += L"- Roll Leg #" + std::to_wstring(row + 1) + L" has incomplete or missing data.\n";
            }
        }
    }

    if (error_message.length()) {
        CustomMessageBox.Show(hwnd, error_message, L"Warning", MB_ICONWARNING);
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

    // Collect the GUI data as it currently exists
    CGuiData guiData;

    std::shared_ptr<Trade> trade;

    if (ActiveTrades.IsNewOptionsTradeAction(tdd.trade_action)) {
        trade = std::make_shared<Trade>();
        trades.push_back(trade);
    }
    else {
        trade = tdd.trade;
    }

    trade->ticker_symbol = guiData.ticker_symbol;
    trade->ticker_name   = guiData.ticker_name;
    trade->future_expiry = guiData.future_expiry;
    trade->category      = guiData.category;
    trade->acb           = guiData.ACB;
    trade->trade_bp      = guiData.trade_bp;

    std::shared_ptr<Transaction> trans = std::make_shared<Transaction>();
    trans->trans_date    = guiData.trans_date;
    trans->description   = guiData.description;
    trans->underlying    = guiData.underlying;
    trans->quantity      = guiData.quantity;
    trans->price         = guiData.price;
    trans->multiplier    = guiData.multiplier;
    trans->fees          = guiData.fees;
    trans->total         = guiData.total;
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
        leg->underlying   = trans->underlying;
        leg->expiry_date  = guiData.legs.at(row).expiry_date;
        leg->strike_price = guiData.legs.at(row).strike_price;
        leg->put_call     = guiData.legs.at(row).put_call;
        leg->action       = guiData.legs.at(row).action;
        leg->trans        = trans;
        int intQuantity   = guiData.legs.at(row).original_quantity * trans->quantity;
        if (intQuantity == 0) continue;

        std::wstring expiry_date = AfxRemoveDateHyphens(leg->expiry_date);
        if (AfxValDouble(expiry_date) > AfxValDouble(trade->bp_end_date)) trade->bp_end_date = expiry_date;

        switch (tdd.trade_action) {

        case TradeAction::new_options_trade:
        case TradeAction::new_iron_condor:
        case TradeAction::new_short_put_LT112:
        case TradeAction::new_short_strangle:
        case TradeAction::new_short_put:
        case TradeAction::new_short_call:
        case TradeAction::add_options_to_trade:
        case TradeAction::add_put_to_trade:
        case TradeAction::add_call_to_trade:
            leg->original_quantity = intQuantity;
            leg->open_quantity = intQuantity;
            break;

        case TradeAction::close_leg:
        case TradeAction::roll_leg:
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
    if (tdd.trade_action == TradeAction::roll_leg) {

        for (int row = 0; row < 4; ++row) {
            std::shared_ptr<Leg> leg = std::make_shared<Leg>();

            trade->nextleg_id += 1;
            leg->leg_id       = trade->nextleg_id;
            leg->underlying   = trans->underlying;
            leg->expiry_date  = guiData.legsRoll.at(row).expiry_date;
            leg->strike_price = guiData.legsRoll.at(row).strike_price;
            leg->put_call     = guiData.legsRoll.at(row).put_call;
            leg->action       = guiData.legsRoll.at(row).action;
            leg->trans        = trans;
            int intQuantity   = guiData.legsRoll.at(row).original_quantity;
            if (intQuantity == 0) continue;

            std::wstring expiry_date = AfxRemoveDateHyphens(leg->expiry_date);
            if (AfxValDouble(expiry_date) > AfxValDouble(trade->bp_end_date)) trade->bp_end_date = expiry_date;

            leg->original_quantity = intQuantity;
            leg->open_quantity = intQuantity;

            trans->legs.push_back(leg);
        }

    }

    tdd.trade = trade;
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
    std::wstring error_message;
    std::wstring text;

    if (guiData.description.length() == 0) error_message += L"- Missing Description.\n";
    if (guiData.quantity == 0) error_message += L"- Missing Quantity.\n";

    // In adition to validating each leg, we count the number of non blank legs. If all legs
    // are blank then there is nothing to save to add that to the error message.
    int num_blank_legs = 0;

    if (guiData.underlying == Underlying::Options) {

        for (int row = 0; row < 4; ++row) {
            // All strings must be zero length in order to skip it from being included in the transaction. 
            if (guiData.legs.at(row).original_quantity == 0 &&
                guiData.legs.at(row).open_quantity == 0 &&
                guiData.legs.at(row).expiry_date.length() == 0 &&
                guiData.legs.at(row).strike_price.length() == 0 &&
                guiData.legs.at(row).put_call == PutCall::Nothing &&
                guiData.legs.at(row).action == Action::Nothing) {
                num_blank_legs++;
                continue;
            }

            // If any of the strings are zero length at this point then the row has incompete data.
            bool incomplete = false;

            if (guiData.legs.at(row).original_quantity == 0) incomplete = true;
            if (guiData.legs.at(row).expiry_date.length() == 0) incomplete = true;
            if (guiData.legs.at(row).strike_price.length() == 0) incomplete = true;
            if (guiData.legs.at(row).put_call == PutCall::Nothing) incomplete = true;
            if (guiData.legs.at(row).action == Action::Nothing) incomplete = true;

            if (incomplete == true) {
                error_message += L"- Leg #" + std::to_wstring(row + 1) + L" has incomplete or missing data.\n";
            }
        }

        if (num_blank_legs == 4) {
            error_message += L"- No Legs exist to be saved.\n";
        }

        for (int row = 0; row < 4; ++row) {
            // All strings must be zero length in order to skip it from being included in the transaction. 
            if (guiData.legsRoll.at(row).original_quantity == 0 &&
                guiData.legsRoll.at(row).open_quantity == 0 &&
                guiData.legsRoll.at(row).expiry_date.length() == 0 &&
                guiData.legsRoll.at(row).strike_price.length() == 0 &&
                guiData.legsRoll.at(row).put_call == PutCall::Nothing &&
                guiData.legsRoll.at(row).action == Action::Nothing) {
                num_blank_legs++;
                continue;
            }

            // If any of the strings are zero length at this point then the row has incompete data.
            bool incomplete = false;

            if (guiData.legsRoll.at(row).original_quantity == 0) incomplete = true;
            if (guiData.legsRoll.at(row).expiry_date.length() == 0) incomplete = true;
            if (guiData.legsRoll.at(row).strike_price.length() == 0) incomplete = true;
            if (guiData.legsRoll.at(row).put_call == PutCall::Nothing) incomplete = true;
            if (guiData.legsRoll.at(row).action == Action::Nothing) incomplete = true;

            if (incomplete == true) {
                error_message += L"- Leg #" + std::to_wstring(row + 5) + L" has incomplete or missing data.\n";
            }
        }
    }

    if (error_message.length()) {
        CustomMessageBox.Show(hwnd, error_message, L"Warning", MB_ICONWARNING);
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

    // Collect the GUI data as it currently exists
    CGuiData guiData;

    // Save the modified data
    
    // If this is a Futures trade and the contract date has been changed then we need to
    // ensure that the ticker data is requested again.
    if (tdd.trade->future_expiry != guiData.future_expiry) {
        tdd.trade->ticker_data_requested = false;
    }

    tdd.trade->future_expiry = guiData.future_expiry;
    tdd.trade->category = guiData.category;
    
    tdd.trans->trans_date  = guiData.trans_date;
    tdd.trans->description = guiData.description;
    tdd.trans->underlying  = guiData.underlying;
    tdd.trans->quantity    = guiData.quantity;
    tdd.trans->price       = guiData.price;
    tdd.trans->multiplier  = guiData.multiplier;
    tdd.trans->fees        = guiData.fees;
    tdd.trade->trade_bp    = guiData.trade_bp;
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

        std::wstring leg_original_quantity = TradeGrid_GetText(hGrid, row, 0);
        std::wstring leg_open_quantity = TradeGrid_GetText(hGrid, row, 1);
        std::wstring leg_expiry = TradeGrid_GetText(hGrid, row, 2);
        std::wstring leg_strike = TradeGrid_GetText(hGrid, row, 4);
        PutCall leg_PutCall = db.StringToPutCall(TradeGrid_GetText(hGrid, row, 5));
        Action  leg_action  = db.StringDescriptionToAction(TradeGrid_GetText(hGrid, row, 6));
            
        // Nothing new or changed to add? Just iterate to next line.
        if (leg_original_quantity.length() == 0) {
            // If row from original legs has been deleted then simply remove it from the 
            // legs vector.
            if (row < (int)tdd.trans->legs.size()) {
                legsToDelete.push_back(row);
            }
            continue;
        }

        int int_original_quantity = AfxValInteger(leg_original_quantity);   // will GPF if empty leg_original_quantity string
        if (int_original_quantity == 0) continue;

        int int_open_quantity = AfxValInteger(leg_open_quantity);   // will GPF if empty leg_open_quantity string
        if (int_open_quantity == 0) continue;

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

        leg->original_quantity = int_original_quantity * tdd.trans->quantity;
        leg->open_quantity = int_open_quantity * tdd.trans->quantity;

        leg->underlying = tdd.trans->underlying;
        leg->expiry_date = leg_expiry;
        leg->strike_price = leg_strike;
        leg->put_call = leg_PutCall;
        leg->action = leg_action;

        std::wstring expiry_date = AfxRemoveDateHyphens(leg_expiry);
        if (AfxValDouble(expiry_date) > AfxValDouble(tdd.trade->bp_end_date)) tdd.trade->bp_end_date = expiry_date;

        if (row >= (int)tdd.trans->legs.size()) {
            tdd.trans->legs.push_back(leg);
        }
    }

    // Remove any rows from the original legs that were deleted in full
    for (auto idx : legsToDelete) {
        tdd.trans->legs.erase(tdd.trans->legs.begin() + idx);
    }
}

