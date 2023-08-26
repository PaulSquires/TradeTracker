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
#include "MainWindow/tws-client.h"
#include "CustomLabel/CustomLabel.h"
#include "Utilities/ListBoxData.h"
#include "SideMenu/SideMenu.h"
#include "MainWindow/MainWindow.h"
#include "CustomVScrollBar/CustomVScrollBar.h"
#include "TradeDialog/TradeDialog.h"
#include "TradeHistory/TradeHistory.h"
#include "Database/trade.h"
#include "Category/Category.h"
#include "Config/Config.h"
#include "ActiveTrades.h"


HWND HWND_ACTIVETRADES = NULL;

CActiveTrades ActiveTrades;


// ========================================================================================
// Returns True/False if incoming Trade action is consider a "New" Options type of action.
// ========================================================================================
bool IsNewOptionsTradeAction(TradeAction action)
{
    switch (action)
    {
    case TradeAction::NewOptionsTrade:
    case TradeAction::NewIronCondor:
    case TradeAction::NewShortLT112:
    case TradeAction::NewShortStrangle:
    case TradeAction::NewShortPut:
    case TradeAction::NewShortCall:
        return true;
    default:
        return false;
    }
}


// ========================================================================================
// Returns True/False if incoming Trade action is consider a "New" Shares/Futures 
// type of action.
// ========================================================================================
bool IsNewSharesTradeAction(TradeAction action)
{
    switch (action)
    {
    case TradeAction::NewSharesTrade:
    case TradeAction::NewFuturesTrade:
        return true;
    default:
        return false;
    }
}


// ========================================================================================
// Central function that actually selects and displays the incoming ListBox index item.
// ========================================================================================
void ActiveTrades_ShowListBoxItem(int index)
{
    HWND hListBox = GetDlgItem(HWND_ACTIVETRADES, IDC_TRADES_LISTBOX);
    HWND hCustomVScrollBar = GetDlgItem(HWND_ACTIVETRADES, IDC_TRADES_CUSTOMVSCROLLBAR);

    // Ensure that the ActiveTrades menu item is selected
    SideMenu_SelectMenuItem(HWND_SIDEMENU, IDC_SIDEMENU_ACTIVETRADES);

    //  update the scrollbar position if necessary
    CustomVScrollBar_Recalculate(hCustomVScrollBar);

    // Get the current line to determine if a valid Trade pointer exists so that we
    // can show the trade history.
    if (index > -1) {
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, index);
        if (ld != nullptr)
            TradeHistory_ShowTradesHistoryTable(ld->trade);
    }

    SetFocus(hListBox);
}


// ========================================================================================
// Populate the Trades ListBox with the current active/open trades
// ========================================================================================
void ActiveTrades_ShowActiveTrades()
{
    HWND hListBox = GetDlgItem(HWND_ACTIVETRADES, IDC_TRADES_LISTBOX);
    HWND hCustomVScrollBar = GetDlgItem(HWND_ACTIVETRADES, IDC_TRADES_CUSTOMVSCROLLBAR);
    HWND hLabel = GetDlgItem(HWND_ACTIVETRADES, IDC_TRADES_LABEL);

    static int tickerId = 100;
    int curSel = 0;

    tws_PauseTWS();

    // Select the correct menu panel item
    SideMenu_SelectMenuItem(HWND_SIDEMENU, IDC_SIDEMENU_ACTIVETRADES);

    CustomLabel_SetText(hLabel, L"Active Trades");

    // Ensure that the Trades panel is set
    MainWindow_SetMiddlePanel(HWND_ACTIVETRADES);


    // Determine if we need to initialize the listbox and request market data
    if (PrevMarketDataLoaded == false) {
        ListBox_ResetContent(hListBox);

        // Prevent ListBox redrawing until all calculations are completed
        SendMessage(hListBox, WM_SETREDRAW, FALSE, 0);

        // In case of newly added/deleted data ensure data is sorted.
        // Sort based on Category and then TickerSymbol
        std::sort(trades.begin(), trades.end(),
            [](const auto trade1, const auto trade2) {
                {
                    if (trade1->category < trade2->category) return true;
                    if (trade2->category < trade1->category) return false;

                    // a=b for primary condition, go to secondary
                    if (trade1->tickerSymbol < trade2->tickerSymbol) return true;
                    if (trade2->tickerSymbol < trade1->tickerSymbol) return false;

                    return false;
                } 
            });



        // Create the new ListBox line data and initiate the new market data.
        int categoryHeader = -1;
        for (auto& trade : trades) {
            // We are displaying only open trades 
            if (trade->isOpen) {
                // Set the decimals for this tickerSymbol. Most will be 2 but futures can have a lot more.
                trade->tickerDecimals = GetTickerDecimals(trade->tickerSymbol);

                if (trade->category != categoryHeader) {
                    ListBoxData_AddCategoryHeader(hListBox, trade);
                    categoryHeader = trade->category;
                }
                ListBoxData_OpenPosition(hListBox, trade, tickerId);

                tickerId++;
                if (tickerId > 500) tickerId = 100;
            }
        }

        // Calculate the actual column widths based on the size of the strings in
        // ListBoxData while respecting the minimum values as defined in nMinColWidth[].
        // This function is also called when receiving new price data from TWS because
        // that data may need the column width to be wider.
        ListBoxData_ResizeColumnWidths(hListBox, TableType::ActiveTrades, -1);


        // Redraw the ListBox to ensure that any recalculated columns are 
        // displayed correctly. Re-enable redraw.
        SendMessage(hListBox, WM_SETREDRAW, TRUE, 0);
        AfxRedrawWindow(hListBox);

    }


    CustomVScrollBar_Recalculate(hCustomVScrollBar);

    curSel = ListBox_GetCurSel(hListBox);
    if (curSel <= 1) curSel = 1;


    // If trades exist then select the first trade so that its history will show
    if (ListBox_GetCount(hListBox) == 0) {
        ListBoxData_AddBlankLine(hListBox);
        curSel = 0;
    }


    ListBox_SetSel(hListBox, true, curSel);
    ActiveTrades_ShowListBoxItem(curSel);

    RECT rc; GetClientRect(HWND_ACTIVETRADES, &rc);
    ActiveTrades_OnSize(HWND_ACTIVETRADES, 0, rc.right, rc.bottom);


    // Start getting the price data for all of the tickers
    tws_ResumeTWS();  // allow ticks to flow prior to asking for data so we don't miss any ticks
    if (PrevMarketDataLoaded == false) {
        ListBoxData_RequestMarketData(hListBox);
    }

    SetFocus(hListBox);

}


// ========================================================================================
// Select a line in the Listbox and deselect any other lines that do not match this
// new line's Trade pointer.
// ========================================================================================
bool ActiveTrades_SelectListBoxItem(HWND hListBox, int idx)
{
    // Get the trade pointer for the newly selected line.
    std::shared_ptr<Trade> trade;
    ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, idx);
    if (ld == nullptr) return false;

    trade = ld->trade;
    if (trade == nullptr) {
        ListBox_SetSel(hListBox, false, idx);
        return false;
    }

    
    // Show the trade history for the selected trade
    ActiveTrades_ShowListBoxItem(idx);


    // Check to see if other lines in the listbox are selected. We will deselect those other
    // lines if they do not belong to the current trade being selected.
    // If the line being clicked on is the main header line for the trade (LineType::TickerLine)
    // then we deselected everything else including legs of this same trade. We do this
    // because there are different popup menu actions applicable to the main header line
    // and/or the individual trade legs.
    LineType CurSelLineType = ld->lineType;

    int nCount = ListBox_GetSelCount(hListBox);

    if (nCount) {
        int* selItems = new int[nCount]();
        SendMessage(hListBox, LB_GETSELITEMS, (WPARAM)nCount, (LPARAM)selItems);

        for (int i = 0; i < nCount; ++i)
        {
            ld = (ListBoxData*)ListBox_GetItemData(hListBox, selItems[i]);
            if (ld != nullptr) {
                if (ld->trade != trade) {
                    ListBox_SetSel(hListBox, false, selItems[i]);
                }
                else {
                    // If selecting items within the same Trade then do not allow
                    // mixing selections of the header line and individual legs.
                    if (CurSelLineType != ld->lineType) {
                        ListBox_SetSel(hListBox, false, selItems[i]);
                    }
                }
            }
        }

        delete[] selItems;
    }

    AfxRedrawWindow(hListBox);

    return true;
}


// ========================================================================================
// Expire the selected legs. Basically, ask for confirmation via a messagebox and 
// then take appropriate action.
// ========================================================================================
void ActiveTrades_ExpireSelectedLegs(auto trade)
{
    // Do a check to ensure that there is actually legs selected to expire. It could
    // be that the user select SHARES or other non-options underlyings only.
    if (tdd.legs.size() == 0) {
        MessageBox(
            HWND_SIDEMENU,
            (LPCWSTR)(L"No valid option legs have been selected for expiration."),
            (LPCWSTR)L"Warning",
            MB_ICONWARNING | MB_OK);
        return;
    }
        
    int res = MessageBox(
        HWND_SIDEMENU,
        (LPCWSTR)(L"Are you sure you wish to EXPIRE the selected legs?"),
        (LPCWSTR)L"Confirm",
        MB_ICONWARNING | MB_YESNOCANCEL | MB_DEFBUTTON2);

    if (res != IDYES) return;

    std::shared_ptr<Transaction> trans = std::make_shared<Transaction>();

    trans->description = L"Expiration";
    trans->underlying = L"OPTIONS";
    trade->Transactions.push_back(trans);

    for (auto leg : tdd.legs) {

        // Save this transaction's leg quantities
        std::shared_ptr<Leg> newleg = std::make_shared<Leg>();

        trade->nextLegID += 1;
        newleg->legID = trade->nextLegID;

        newleg->underlying = trans->underlying;

        trans->transDate = AfxCurrentDate();
        newleg->origQuantity = leg->openQuantity * -1;
        newleg->openQuantity = 0;
        newleg->legBackPointerID = leg->legID;
        leg->openQuantity = 0;

        if (leg->action == L"STO") newleg->action = L"BTC";
        if (leg->action == L"BTO") newleg->action = L"STC"; 

        newleg->expiryDate = leg->expiryDate;
        newleg->strikePrice = leg->strikePrice;
        newleg->PutCall = leg->PutCall;
        trans->legs.push_back(newleg);
    }

    // Set the open status of the entire trade based on the new modified legs
    trade->setTradeOpenStatus();

    // Rebuild the openLegs position vector
    trade->createOpenLegsVector();

    // Save the new data to the database
    SaveDatabase();

    // Reload the trade list
    // Destroy any existing ListBox line data
    // This will also clear the LineData pointers and cancel any previous market data
    ListBoxData_DestroyItemData(GetDlgItem(HWND_ACTIVETRADES, IDC_TRADES_LISTBOX));
    ActiveTrades_ShowActiveTrades();

}


// ========================================================================================
// Create Transaction for Shares or Futures that have been called away.
// ========================================================================================
void ActiveTrades_CalledAwayAssignment(
    auto trade, auto leg, int NumSharesAggregate, int NumFuturesAggregate)
{
    std::shared_ptr<Transaction> trans;
    std::shared_ptr<Leg> newleg;

    bool isShares = (IsFuturesTicker(trade->tickerSymbol)) ? false : true;

    int QuantityAssigned = 0;
    int NumLegQuantity = 0;
    double multiplier = 1;

    std::wstring msg = L"Continue with OPTION ASSIGNMENT?\n\n";

    if (isShares == true) {
        QuantityAssigned = min(abs(leg->openQuantity * 100), abs(NumSharesAggregate));
        NumLegQuantity = QuantityAssigned / 100;
        msg += std::to_wstring(QuantityAssigned) + L" shares called away at $" + leg->strikePrice + L" per share.";
        multiplier = 1;
    }
    else {
        QuantityAssigned = min(abs(leg->openQuantity), abs(NumFuturesAggregate));
        NumLegQuantity = QuantityAssigned;
        msg += std::to_wstring(QuantityAssigned) + L" futures called away at $" + leg->strikePrice + L" per future.";
        multiplier = trade->multiplier;
    }
    NumLegQuantity = (leg->openQuantity < 0) ? NumLegQuantity * -1 : NumLegQuantity;

    int res = MessageBox(
        HWND_SIDEMENU,
        (LPCWSTR)(msg.c_str()),
        (LPCWSTR)L"Confirm",
        MB_ICONWARNING | MB_YESNOCANCEL | MB_DEFBUTTON2);

    if (res != IDYES) return;


    // Close the Option. Save this transaction's leg quantities
    trans = std::make_shared<Transaction>();
    trans->transDate = AfxCurrentDate();
    trans->description = L"Called away";
    trans->underlying = L"OPTIONS";
    trade->Transactions.push_back(trans);

    newleg = std::make_shared<Leg>();
    trade->nextLegID += 1;
    newleg->legID = trade->nextLegID;
    newleg->underlying = trans->underlying;
    newleg->origQuantity = NumLegQuantity * -1;
    newleg->openQuantity = 0;
    newleg->legBackPointerID = leg->legID;
    leg->openQuantity = leg->openQuantity - NumLegQuantity;

    if (leg->action == L"STO") newleg->action = L"BTC";
    if (leg->action == L"BTO") newleg->action = L"STC";

    newleg->expiryDate = leg->expiryDate;
    newleg->strikePrice = leg->strikePrice;
    newleg->PutCall = leg->PutCall;
    trans->legs.push_back(newleg);


    // Remove the SHARES/FUTURES that have been called away.
    trans = std::make_shared<Transaction>();
    trans->transDate = AfxCurrentDate();
    trans->description = L"Called away";
    trans->underlying = (isShares == true) ? L"SHARES" : L"FUTURES";
    trans->quantity = QuantityAssigned;
    trans->price = AfxValDouble(leg->strikePrice);
    trans->multiplier = multiplier;
    trans->fees = 0;
    trade->Transactions.push_back(trans);

    newleg = std::make_shared<Leg>();
    trade->nextLegID += 1;
    newleg->legID = trade->nextLegID;
    newleg->underlying = trans->underlying;
    newleg->strikePrice = leg->strikePrice;

    if (leg->PutCall == L"P") {
        newleg->action = L"BTC";
        newleg->origQuantity = QuantityAssigned;
        newleg->openQuantity = QuantityAssigned;
        trans->total = trans->quantity * multiplier * trans->price * -1;  // DR
        trade->ACB = trade->ACB + trans->total;
    }
    else {
        newleg->action = L"STC";
        newleg->origQuantity = QuantityAssigned * -1;
        newleg->openQuantity = QuantityAssigned * -1;
        trans->total = trans->quantity * multiplier * trans->price;  // CR
        trade->ACB = trade->ACB + trans->total;
    }

    trans->legs.push_back(newleg);

    // Set the open status of the entire trade based on the new modified legs
    trade->setTradeOpenStatus();

    // Rebuild the openLegs position vector
    trade->createOpenLegsVector();

    // Save the new data to the database
    SaveDatabase();

    // Reload the trade list
    // Destroy any existing ListBox line data
    // This will also clear the LineData pointers and cancel any previous market data
    ListBoxData_DestroyItemData(GetDlgItem(HWND_ACTIVETRADES, IDC_TRADES_LISTBOX));
    ActiveTrades_ShowActiveTrades();

}


// ========================================================================================
// Create Transaction for option assignment for the selected leg.
// ========================================================================================
void ActiveTrades_Assignment(auto trade, auto leg)
{
    std::shared_ptr<Transaction> trans;
    std::shared_ptr<Leg> newleg;

    bool isShares = (trade->tickerSymbol.substr(0,1) == L"/") ? false : true;

    int QuantityAssigned = 0;
    double multiplier = 1;

    std::wstring wszLongShort = (leg->PutCall == L"P") ? L"LONG " : L"SHORT ";
    std::wstring msg = L"Continue with OPTION ASSIGNMENT?\n\n";
        
    if (isShares == true) {
        QuantityAssigned = abs(leg->openQuantity * 100);
        multiplier = 1;
        msg += wszLongShort + std::to_wstring(QuantityAssigned) + L" shares at $" + leg->strikePrice + L" per share.";
    }
    else {
        QuantityAssigned = abs(leg->openQuantity);
        multiplier = trade->multiplier;
        msg += wszLongShort + std::to_wstring(QuantityAssigned) + L" futures at $" + leg->strikePrice + L" per future.";
    }

    int res = MessageBox(
        HWND_SIDEMENU,
        (LPCWSTR)(msg.c_str()),
        (LPCWSTR)L"Confirm",
        MB_ICONWARNING | MB_YESNOCANCEL | MB_DEFBUTTON2);

    if (res != IDYES) return;


    // Close the Option. Save this transaction's leg quantities
    trans = std::make_shared<Transaction>();
    trans->transDate = leg->expiryDate;  // AfxCurrentDate();
    trans->description = L"Assignment";
    trans->underlying = L"OPTIONS";
    trade->Transactions.push_back(trans);

    newleg = std::make_shared<Leg>();
    trade->nextLegID += 1;
    newleg->legID = trade->nextLegID;
    newleg->underlying = trans->underlying;
    newleg->origQuantity = leg->openQuantity * -1;
    newleg->openQuantity = 0;
    newleg->legBackPointerID = leg->legID;
    leg->openQuantity = 0;

    if (leg->action == L"STO") newleg->action = L"BTC";
    if (leg->action == L"BTO") newleg->action = L"STC";

    newleg->expiryDate = leg->expiryDate;
    newleg->strikePrice = leg->strikePrice;
    newleg->PutCall = leg->PutCall;
    trans->legs.push_back(newleg);


    // Make the SHARES/FUTURES that have been assigned.
    trans = std::make_shared<Transaction>();
    trans->transDate = leg->expiryDate;  // AfxCurrentDate();
    trans->description = L"Assignment";
    trans->underlying = (isShares == true) ? L"SHARES" : L"FUTURES";
    trans->quantity = QuantityAssigned;
    trans->price = AfxValDouble(leg->strikePrice);
    trans->multiplier = multiplier;
    trans->fees = 0;
    trade->Transactions.push_back(trans);

    newleg = std::make_shared<Leg>();
    trade->nextLegID += 1;
    newleg->legID = trade->nextLegID;
    newleg->underlying = trans->underlying;
    newleg->strikePrice = leg->strikePrice;

    if (leg->PutCall == L"P") {
        newleg->action = L"BTO";
        newleg->origQuantity = QuantityAssigned;
        newleg->openQuantity = QuantityAssigned;
        trans->total = trans->quantity * multiplier * trans->price * -1;  // DR
        trade->ACB = trade->ACB + trans->total;
    }
    else {
        newleg->action = L"STO";
        newleg->origQuantity = QuantityAssigned * -1;
        newleg->openQuantity = QuantityAssigned * -1;
        trans->total = trans->quantity * multiplier * trans->price;  // CR
        trade->ACB = trade->ACB + trans->total;
    }

    trans->legs.push_back(newleg);

    // Set the open status of the entire trade based on the new modified legs
    trade->setTradeOpenStatus();

    // Rebuild the openLegs position vector
    trade->createOpenLegsVector();

    // Save the new data to the database
    SaveDatabase();

    // Reload the trade list
    // Destroy any existing ListBox line data
    // This will also clear the LineData pointers and cancel any previous market data
    ListBoxData_DestroyItemData(GetDlgItem(HWND_ACTIVETRADES, IDC_TRADES_LISTBOX));
    ActiveTrades_ShowActiveTrades();
}


// ========================================================================================
// Create Transaction for option assignment for the selected leg.
// ========================================================================================
void ActiveTrades_OptionAssignment(auto trade)
{
    // Do a check to ensure that there is actually a leg selected for assignment. 
    if (tdd.legs.size() == 0) {
        MessageBox(
            HWND_SIDEMENU,
            (LPCWSTR)(L"No valid option leg has been selected for assignment."),
            (LPCWSTR)L"Warning",
            MB_ICONWARNING | MB_OK);
        return;
    }

    // Check to see if Shares/Futures exist that could be "called away". If yes, then process
    // that type of transaction rather than adding a new assigned to transaction.
    int NumSharesAggregate = 0;
    int NumFuturesAggregate = 0;

    for (const auto& trans : trade->Transactions) {
        for (const auto& leg : trans->legs) {
            if (leg->underlying == L"SHARES") {
                NumSharesAggregate += leg->openQuantity;
            }
            else if (leg->underlying == L"FUTURES") {
                NumFuturesAggregate += leg->openQuantity;
            }
        }
    }


    auto leg = tdd.legs.at(0);

    // Are LONG SHARES or LONG FUTURES being called away
    if ((NumSharesAggregate > 0 || NumFuturesAggregate > 0) && leg->PutCall == L"C") {
        ActiveTrades_CalledAwayAssignment(trade, leg, NumSharesAggregate, NumFuturesAggregate);
        return;
    }
    // Are SHORT SHARES or SHORT FUTURES being called away
    if ((NumSharesAggregate < 0 || NumFuturesAggregate < 0) && leg->PutCall == L"P") {
        ActiveTrades_CalledAwayAssignment(trade, leg, NumSharesAggregate, NumFuturesAggregate);
        return;
    }

    // Otherwise, Option is being assigned Shares or Futures
    ActiveTrades_Assignment(trade, leg);

}


// ========================================================================================
// Populate vector that holds all selected lines/legs. This will be passed to the 
// TradeDialog in order to perform actions on the legs.
// ========================================================================================
void ActiveTrades_PopulateLegsEditVector(HWND hListBox)
{
    tdd.legs.clear();

    int nCount = ListBox_GetSelCount(hListBox);
    tdd.legs.reserve(nCount);

    if (nCount) {
        int* selItems = new int[nCount]();
        SendMessage(hListBox, LB_GETSELITEMS, (WPARAM)nCount, (LPARAM)selItems);

        for (int i = 0; i < nCount; i++)
        {
            ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, selItems[i]);
            if (ld != nullptr) {
                // Only allow Option legs to be pushed to legEdit
                if (ld->leg != nullptr) {
                    tdd.legs.push_back(ld->leg);
                }
            }
        }

        delete[] selItems;
    }
}



// ========================================================================================
// Handle the right-click popup menu on the ListBox's selected lines.
// ========================================================================================
void ActiveTrades_RightClickMenu(HWND hListBox, int idx)
{
    HMENU hMenu = CreatePopupMenu();

    std::wstring wszText;
    std::wstring wszPlural;
    int nCount = ListBox_GetSelCount(hListBox);

    std::shared_ptr<Trade> trade = nullptr;

    bool IsTickerLine = false;

    // Clear the tdd module global trade variable
    tdd.ResetDefaults();

    int nCurSel = ListBox_GetCurSel(hListBox);
    ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, nCurSel);
    if (ld->trade == nullptr) return;

    trade = ld->trade;
    tdd.trade = ld->trade;
    tdd.trans = ld->trans;
    tdd.sharesAggregateEdit = ld->AggregateShares;

    if (nCount == 1) {
        // Is this the Trade header line
        if (ld != nullptr) {
            if (ld->lineType == LineType::TickerLine) {
                IsTickerLine = true;
            }
        }
    }
    else {
        wszPlural = L"s";
    }


    if (ld->lineType == LineType::OptionsLeg) {
        wszText = L"Roll Leg" + wszPlural;
        InsertMenu(hMenu, 0, MF_BYCOMMAND | MF_STRING | MF_ENABLED, (int)TradeAction::RollLeg, wszText.c_str());

        wszText = L"Close Leg" + wszPlural;
        InsertMenu(hMenu, 0, MF_BYCOMMAND | MF_STRING | MF_ENABLED, (int)TradeAction::CloseLeg, wszText.c_str());

        wszText = L"Expire Leg" + wszPlural;
        InsertMenu(hMenu, 0, MF_BYCOMMAND | MF_STRING | MF_ENABLED, (int)TradeAction::ExpireLeg, wszText.c_str());

        if (nCount == 1) {
            InsertMenu(hMenu, 0, MF_BYCOMMAND | MF_SEPARATOR | MF_ENABLED, (int)TradeAction::NoAction + 1, L"");
            InsertMenu(hMenu, 0, MF_BYCOMMAND | MF_STRING | MF_ENABLED, (int)TradeAction::Assignment, L"Option Assignment");
        }
        InsertMenu(hMenu, 0, MF_BYCOMMAND | MF_SEPARATOR | MF_ENABLED, (int)TradeAction::NoAction + 2, L"");
    }


    if (ld->lineType == LineType::Shares) {
        wszText = L"Manage Shares";
        InsertMenu(hMenu, 0, MF_BYCOMMAND | MF_STRING | MF_ENABLED, (int)TradeAction::ManageShares, wszText.c_str());
        InsertMenu(hMenu, 0, MF_BYCOMMAND | MF_SEPARATOR | MF_ENABLED, (int)TradeAction::NoAction + 2, L"");
    }

    if (ld->lineType == LineType::Futures) {
        wszText = L"Manage Futures";
        InsertMenu(hMenu, 0, MF_BYCOMMAND | MF_STRING | MF_ENABLED, (int)TradeAction::ManageFutures, wszText.c_str());
        InsertMenu(hMenu, 0, MF_BYCOMMAND | MF_SEPARATOR | MF_ENABLED, (int)TradeAction::NoAction + 2, L"");
    }

    InsertMenu(hMenu, 0, MF_BYCOMMAND | MF_STRING | MF_ENABLED, (int)TradeAction::AddOptionsToTrade, L"Add Options to Trade");
    InsertMenu(hMenu, 0, MF_BYCOMMAND | MF_STRING | MF_ENABLED, (int)TradeAction::AddPutToTrade, L"Add Put to Trade");
    InsertMenu(hMenu, 0, MF_BYCOMMAND | MF_STRING | MF_ENABLED, (int)TradeAction::AddCallToTrade, L"Add Call to Trade");
    InsertMenu(hMenu, 0, MF_BYCOMMAND | MF_SEPARATOR | MF_ENABLED, (int)TradeAction::NoAction + 3, L"");

    if (IsFuturesTicker(trade->tickerSymbol)) {
        InsertMenu(hMenu, 0, MF_BYCOMMAND | MF_STRING | MF_ENABLED, (int)TradeAction::AddFuturesToTrade, L"Add Futures to Trade");
    } else {
        InsertMenu(hMenu, 0, MF_BYCOMMAND | MF_STRING | MF_ENABLED, (int)TradeAction::AddSharesToTrade, L"Add Shares to Trade");
    }

    POINT pt; GetCursorPos(&pt);
    TradeAction selected =
        (TradeAction) TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RETURNCMD, pt.x, pt.y, 0, hListBox, NULL);



    switch (selected)
    {
    case TradeAction::ManageShares:
    case TradeAction::ManageFutures:
    case TradeAction::AddSharesToTrade:
    case TradeAction::AddFuturesToTrade:
    case TradeAction::AddOptionsToTrade:
    case TradeAction::AddPutToTrade:
    case TradeAction::AddCallToTrade:
        TradeDialog_Show(selected);
        break;

    case TradeAction::RollLeg:
    case TradeAction::CloseLeg:
        ActiveTrades_PopulateLegsEditVector(hListBox);
        TradeDialog_Show(selected);
        break;

    case TradeAction::ExpireLeg:
        ActiveTrades_PopulateLegsEditVector(hListBox);
        ActiveTrades_ExpireSelectedLegs(trade);
        break;

    case TradeAction::Assignment:
        ActiveTrades_PopulateLegsEditVector(hListBox);
        ActiveTrades_OptionAssignment(trade);
        break;
    }

}


// ========================================================================================
// Listbox subclass Window procedure
// ========================================================================================
LRESULT CALLBACK ActiveTrades_ListBox_SubclassProc(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    // Create static accumulation variable to collect the data from
    // a series of middle mouse wheel scrolls.
    static int accumDelta = 0;

    switch (uMsg)
    {

    case WM_MOUSEWHEEL:
    {
        // Accumulate delta until scroll one line (up +120, down -120). 
        // 120 is the Microsoft default delta
        int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        int nTopIndex = SendMessage(hWnd, LB_GETTOPINDEX, 0, 0);
        accumDelta += zDelta;
        if (accumDelta >= 120) {     // scroll up 3 lines
            nTopIndex -= 3;
            nTopIndex = max(0, nTopIndex);
            SendMessage(hWnd, LB_SETTOPINDEX, nTopIndex, 0);
            accumDelta = 0;
        }
        else {
            if (accumDelta <= -120) {     // scroll down 3 lines
                nTopIndex += +3;
                SendMessage(hWnd, LB_SETTOPINDEX, nTopIndex, 0);
                accumDelta = 0;
            }
        }
        HWND hCustomVScrollBar = GetDlgItem(HWND_ACTIVETRADES, IDC_TRADES_CUSTOMVSCROLLBAR);
        CustomVScrollBar_Recalculate(hCustomVScrollBar);
        return 0;
        break;
    }


    case WM_RBUTTONDOWN:
    {
        int idx = Listbox_ItemFromPoint(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        
        // The return value contains the index of the nearest item in the LOWORD. The HIWORD is zero 
        // if the specified point is in the client area of the list box, or one if it is outside the 
        // client area.
        if (HIWORD(idx) == 1) break;
            
        // Return to not select the line (eg. if a blank line was clicked on)
        if (ActiveTrades_SelectListBoxItem(hWnd, idx) == false) {
            return 0;
        }
        ListBox_SetSel(hWnd, true, idx);

        ActiveTrades_RightClickMenu(hWnd, idx);
        return 0;
    }
    break;


    case WM_LBUTTONDOWN:
    {
        int idx = Listbox_ItemFromPoint(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        // The return value contains the index of the nearest item in the LOWORD. The HIWORD is zero 
        // if the specified point is in the client area of the list box, or one if it is outside the 
        // client area.
        if (HIWORD(idx) == 1) break;
        ActiveTrades_SelectListBoxItem(hWnd, idx);
    }
    break;


    case WM_ERASEBKGND:
    {
        // If the number of lines in the listbox maybe less than the number per page then 
        // calculate from last item to bottom of listbox, otherwise calculate based on
        // the mod of the lineheight to listbox height so we can color the partial line
        // that won't be displayed at the bottom of the list.
        RECT rc; GetClientRect(hWnd, &rc);
        
        RECT rcItem{};
        SendMessage(hWnd, LB_GETITEMRECT, 0, (LPARAM)&rcItem);
        int itemHeight = (rcItem.bottom - rcItem.top);
        int NumItems = ListBox_GetCount(hWnd);
        int nTopIndex = SendMessage(hWnd, LB_GETTOPINDEX, 0, 0);
        int visible_rows = 0;
        int ItemsPerPage = 0;
        int bottom_index = 0;
        int nWidth = (rc.right - rc.left);
        int nHeight = (rc.bottom - rc.top);
        
        if (NumItems > 0) {
            ItemsPerPage = (nHeight) / itemHeight;
            bottom_index = (nTopIndex + ItemsPerPage);
            if (bottom_index >= NumItems)
                bottom_index = NumItems - 1;
            visible_rows = (bottom_index - nTopIndex) + 1;
            rc.top = visible_rows * itemHeight;
        }
        
        if (rc.top < rc.bottom) {
            nHeight = (rc.bottom - rc.top);
            HDC hDC = (HDC)wParam;
            Graphics graphics(hDC);
            Color backColor(COLOR_GRAYDARK);
            SolidBrush backBrush(backColor);
            graphics.FillRectangle(&backBrush, rc.left, rc.top, nWidth, nHeight);
        }

        ValidateRect(hWnd, &rc);
        return TRUE;
        break;

    }


    case WM_DESTROY:
        // Destroy all manually allocated ListBox display data that is held
        // in the LineData structures..
        ListBoxData_DestroyItemData(hWnd);

        // REQUIRED: Remove control subclassing
        RemoveWindowSubclass(hWnd, ActiveTrades_ListBox_SubclassProc, uIdSubclass);
        break;


    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);

}



// ========================================================================================
// Process WM_MEASUREITEM message for window/dialog: ActiveTrades
// ========================================================================================
void ActiveTrades_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT* lpMeasureItem)
{
    lpMeasureItem->itemHeight = AfxScaleY(ACTIVETRADES_LISTBOX_ROWHEIGHT);
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: ActiveTrades
// ========================================================================================
BOOL ActiveTrades_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    // Handle all of the painting in WM_PAINT
    return TRUE;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: ActiveTrades
// ========================================================================================
void ActiveTrades_OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    Graphics graphics(hdc);

    // Create the background brush
    Color backColor(COLOR_BLACK);
    SolidBrush backBrush(backColor);

    // Paint the background using brush.
    int nWidth = (ps.rcPaint.right - ps.rcPaint.left);
    int nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);
    
    graphics.FillRectangle(&backBrush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Show/Hide the Net and Excess liquidity labels and values.
// ========================================================================================
void ActiveTrades_ShowHideLiquidityLabels(HWND hwnd)
{
    int nShow = (tws_isConnected()) ? SW_SHOW : SW_HIDE;

    ShowWindow(GetDlgItem(hwnd, IDC_TRADES_NETLIQUIDATION), nShow);
    ShowWindow(GetDlgItem(hwnd, IDC_TRADES_NETLIQUIDATION_VALUE), nShow);
    ShowWindow(GetDlgItem(hwnd, IDC_TRADES_EXCESSLIQUIDITY), nShow);
    ShowWindow(GetDlgItem(hwnd, IDC_TRADES_EXCESSLIQUIDITY_VALUE), nShow);
}
    
    
 // ========================================================================================
// Process WM_SIZE message for window/dialog: ActiveTrades
// ========================================================================================
void ActiveTrades_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    HWND hHeader = GetDlgItem(hwnd, IDC_TRADES_HEADER);
    HWND hListBox = GetDlgItem(hwnd, IDC_TRADES_LISTBOX);
    HWND hCustomVScrollBar = GetDlgItem(hwnd, IDC_TRADES_CUSTOMVSCROLLBAR);
        
    int margin = AfxScaleY(ACTIVETRADES_MARGIN);

    // If no entries exist for the ListBox then don't show any child controls
    int showflag = (ListBox_GetCount(hListBox) <= 1) ? SWP_HIDEWINDOW : SWP_SHOWWINDOW;

    HDWP hdwp = BeginDeferWindowPos(10);

    // Move and size the top labels into place
    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_TRADES_LABEL), 0, 0, 0, AfxScaleX(200), margin, SWP_NOZORDER | SWP_SHOWWINDOW);
    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_TRADES_NETLIQUIDATION),        0, AfxScaleX(425), 0, AfxScaleX(60), margin, SWP_NOZORDER);
    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_TRADES_NETLIQUIDATION_VALUE),  0, AfxScaleX(485), 0, AfxScaleX(50), margin, SWP_NOZORDER);
    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_TRADES_EXCESSLIQUIDITY),       0, AfxScaleX(535), 0, AfxScaleX(65), margin, SWP_NOZORDER);
    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_TRADES_EXCESSLIQUIDITY_VALUE), 0, AfxScaleX(600), 0, AfxScaleX(50), margin, SWP_NOZORDER);

    // Do not call the calcVThumbRect() function during a scrollbar move. This WM_SIZE
    // gets triggered when the ListBox WM_DRAWITEM fires. If we do another calcVThumbRect()
    // calculation then the scrollbar will appear "jumpy" under the user's mouse cursor.
    bool bShowScrollBar = false;
    CustomVScrollBar* pData = CustomVScrollBar_GetPointer(hCustomVScrollBar);
    if (pData != nullptr) {
        if (pData->bDragActive) {
            bShowScrollBar = true;
        }
        else {
            bShowScrollBar = pData->calcVThumbRect();
        }
    }
    int CustomVScrollBarWidth = bShowScrollBar ? AfxScaleX(CUSTOMVSCROLLBAR_WIDTH) : 0;


    int nLeft = 0;
    int nTop = margin;
    int nHeight = cy - nTop;
    int nWidth = cx - CustomVScrollBarWidth;
    hdwp = DeferWindowPos(hdwp, hListBox, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | showflag);
    
    nLeft = nLeft + nWidth;   // right edge of ListBox
    nWidth = CustomVScrollBarWidth;
    int showscrollbar = (bShowScrollBar ? SWP_SHOWWINDOW : SWP_HIDEWINDOW);
    if (showflag == SWP_HIDEWINDOW) showscrollbar = SWP_HIDEWINDOW;
    hdwp = DeferWindowPos(hdwp, hCustomVScrollBar, 0, nLeft, nTop, nWidth, nHeight,
        SWP_NOZORDER | showflag);

    EndDeferWindowPos(hdwp);

    ActiveTrades_ShowHideLiquidityLabels(hwnd);

}


// ========================================================================================
// Process WM_CREATE message for window/dialog: ActiveTrades
// ========================================================================================
BOOL ActiveTrades_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    HWND_ACTIVETRADES = hwnd;
        
    HWND hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TRADES_LABEL, L"Active Trades", 
        COLOR_WHITELIGHT, COLOR_BLACK);

    hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TRADES_NETLIQUIDATION, L"Net Liq:",
        COLOR_WHITEDARK, COLOR_BLACK);

    hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TRADES_NETLIQUIDATION_VALUE, L"",
        COLOR_WHITELIGHT, COLOR_BLACK);

    hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TRADES_EXCESSLIQUIDITY, L"Excess Liq:",
        COLOR_WHITEDARK, COLOR_BLACK);

    hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TRADES_EXCESSLIQUIDITY_VALUE, L"",
        COLOR_WHITELIGHT, COLOR_BLACK);

    // Create an Ownerdraw fixed row sized listbox that we will use to custom
    // paint our various open trades.
    hCtl =
        ActiveTrades.AddControl(Controls::ListBox, hwnd, IDC_TRADES_LISTBOX, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_EXTENDEDSEL | 
            LBS_OWNERDRAWFIXED | LBS_NOTIFY | LBS_WANTKEYBOARDINPUT,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)ActiveTrades_ListBox_SubclassProc,
            IDC_TRADES_LISTBOX, NULL);
    ListBox_AddString(hCtl, NULL);

    // Create our custom vertical scrollbar and attach the ListBox to it.
    CreateCustomVScrollBar(hwnd, IDC_TRADES_CUSTOMVSCROLLBAR, hCtl);

    return TRUE;
}


// ========================================================================================
// Process WM_COMMAND message for window/dialog: ActiveTrades
// ========================================================================================
void ActiveTrades_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (codeNotify)
    {

    case LBN_SELCHANGE:
    {
        int nCurSel = ListBox_GetCurSel(hwndCtl);
        ActiveTrades_SelectListBoxItem(hwndCtl, nCurSel);
        break;
    }

    }
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CActiveTrades::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(m_hwnd, WM_CREATE, ActiveTrades_OnCreate);
        HANDLE_MSG(m_hwnd, WM_COMMAND, ActiveTrades_OnCommand);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, ActiveTrades_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, ActiveTrades_OnPaint);
        HANDLE_MSG(m_hwnd, WM_SIZE, ActiveTrades_OnSize);
        HANDLE_MSG(m_hwnd, WM_MEASUREITEM, ActiveTrades_OnMeasureItem);
        HANDLE_MSG(m_hwnd, WM_DRAWITEM, ListBoxData_OnDrawItem);

    
    case MSG_POSITIONS_READY:
        // Request Positions has completed and has sent this notification so 
        // we can now start requesting the portfolio updates real time data.
        client.cancelPortfolioUpdates();
        client.requestPortfolioUpdates();
        return 0;


    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}

