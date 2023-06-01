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
#include "..\MainWindow\tws-client.h"
#include "..\CustomLabel\CustomLabel.h"
#include "..\Utilities\ListBoxData.h"
#include "..\MenuPanel\MenuPanel.h"
#include "..\MainWindow\MainWindow.h"
#include "..\CustomVScrollBar\CustomVScrollBar.h"
#include "..\TradeDialog\TradeDialog.h"
#include "..\Database\database.h"
#include "..\Category\Category.h"
#include "TradesPanel.h"


HWND HWND_TRADESPANEL = NULL;

extern std::vector<std::shared_ptr<Trade>> trades;

extern HWND HWND_MAINWINDOW;
extern HWND HWND_HISTORYPANEL;
extern HWND HWND_MENUPANEL;
extern HWND HWND_MIDDLEPANEL;
extern CTradesPanel TradesPanel;

extern void MainWindow_SetMiddlePanel(HWND hPanel);
extern void HistoryPanel_ShowTradesHistoryTable(const std::shared_ptr<Trade>& trade);
void TradesPanel_OnSize(HWND hwnd, UINT state, int cx, int cy);

// Vector to hold all selected legs that TradeDiaog will act on
std::vector<std::shared_ptr<Leg>> legsEdit;
std::shared_ptr<Trade> tradeEdit;
std::wstring sharesAggregateEdit = L"0";


// ========================================================================================
// Returns True/False if incoming Trade action is consider a "New" Options type of action.
// ========================================================================================
bool IsNewOptionsTradeAction(TradeAction action)
{
    switch (action)
    {
    case TradeAction::NewOptionsTrade:
    case TradeAction::NewIronCondor:
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
void TradesPanel_ShowListBoxItem(int index)
{
    HWND hListBox = GetDlgItem(HWND_TRADESPANEL, IDC_TRADES_LISTBOX);
    HWND hCustomVScrollBar = GetDlgItem(HWND_TRADESPANEL, IDC_TRADES_CUSTOMVSCROLLBAR);

    ListBox_SetSel(hListBox, true, index);

    //  update the scrollbar position if necessary
    CustomVScrollBar_Recalculate(hCustomVScrollBar);

    // Get the current line to determine if a valid Trade pointer exists so that we
    // can show the trade history.
    if (index > -1) {
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, index);
        if (ld != nullptr)
            HistoryPanel_ShowTradesHistoryTable(ld->trade);
    }

    SetFocus(hListBox);
}


// ========================================================================================
// Populate the Trades ListBox with the current active/open trades
// ========================================================================================
void TradesPanel_ShowActiveTrades()
{
    HWND hListBox = GetDlgItem(HWND_TRADESPANEL, IDC_TRADES_LISTBOX);
    HWND hCustomVScrollBar = GetDlgItem(HWND_TRADESPANEL, IDC_TRADES_CUSTOMVSCROLLBAR);
    HWND hLabel = GetDlgItem(HWND_TRADESPANEL, IDC_TRADES_LABEL);

    tws_PauseTWS();

    // Prevent ListBox redrawing until all calculations are completed
    SendMessage(hListBox, WM_SETREDRAW, FALSE, 0);


    // In case of newly added/deleted data ensure data is sorted.
    // Sort the trades vector based on ticker symbol
    std::sort(trades.begin(), trades.end(),
        [](const auto trade1, const auto trade2) {
            return (trade1->tickerSymbol < trade2->tickerSymbol) ? true : false;
        });


    // Destroy any existing ListBox line data
    // This will also clear the LineData pointers and cancel any previous market data
    ListBoxData_DestroyItemData(hListBox);


    // Get the currently active Category index. By default, this will be ALL categories
    // but the user may have selected a specific category.
    int category = CategoryControl_GetSelectedIndex(GetDlgItem(HWND_MAINWINDOW, IDC_MAINWINDOW_CATEGORY));


    // Create the new ListBox line data and initiate the new market data.
    int tickerId = 100;
    for (const auto& trade : trades) {
        // We are displaying only all open trades for the selected Category
        if (trade->isOpen) {
            if (category == (int)Category::CategoryAll) {   // ALL categories
                ListBoxData_OpenPosition(hListBox, trade, tickerId);
            }
            else if (trade->category == category) {   // specific selected category
                ListBoxData_OpenPosition(hListBox, trade, tickerId);
            }
        }
        tickerId++;
    }


    // Calculate the actual column widths based on the size of the strings in
    // ListBoxData while respecting the minimum values as defined in nMinColWidth[].
    // This function is also called when receiving new price data from TWS because
    // that data may need the column width to be wider.
    ListBoxData_ResizeColumnWidths(hListBox, TableType::ActiveTrades, -1);


    // Select the correct menu panel item
    MenuPanel_SelectMenuItem(HWND_MENUPANEL, IDC_MENUPANEL_ACTIVETRADES);


    // Set the label text indicated the type of trades being listed
    CustomLabel_SetText(hLabel, L"Active Trades");


    // Redraw the ListBox to ensure that any recalculated columns are 
    // displayed correctly. Re-enable redraw.
    SendMessage(hListBox, WM_SETREDRAW, TRUE, 0);
    AfxRedrawWindow(hListBox);


    // If trades exist then select the first trade so that its history will show
    if (ListBox_GetCount(hListBox)) {
        TradesPanel_ShowListBoxItem(0);
    }else {
        ListBoxData_AddBlankLine(hListBox);
    }
    

    // Ensure that the Trades panel is set
    MainWindow_SetMiddlePanel(HWND_TRADESPANEL);

    // Show the Category control
    ShowWindow(GetDlgItem(HWND_MAINWINDOW, IDC_MAINWINDOW_CATEGORY), SW_SHOW);

    CustomVScrollBar_Recalculate(hCustomVScrollBar);


    ListBox_SetSel(hListBox, true, 0);
    SetFocus(hListBox);

    tws_ResumeTWS();

    // Start getting the price data for all of the tickers
    ListBoxData_RequestMarketData(hListBox);

}


// ========================================================================================
// Select a line in the Listbox and deselect any other lines that do not match this
// new line's Trade pointer.
// ========================================================================================
bool TradesPanel_SelectListBoxItem(HWND hListBox, int idx)
{
    // Get the trade pointer for the newly selected line.
    std::shared_ptr<Trade> trade;
    ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, idx);
    if (ld == nullptr) return false;

    trade = ld->trade;
    if (trade == nullptr) return false;

    
    // Show the trade history for the selected trade
    TradesPanel_ShowListBoxItem(idx);


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

        for (int i = 0; i < nCount; i++)
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

    return true;
}


// ========================================================================================
// Expire the selected legs. Basically, ask for confirmation via a messagebox and 
// then take appropriate action.
// ========================================================================================
void TradesPanel_ExpireSelectedLegs(auto trade)
{
    // Do a check to ensure that there is actually legs selected to expire. It could
    // be that the user select SHARES or other non-options underlyings only.
    if (legsEdit.size() == 0) {
        MessageBox(
            HWND_MENUPANEL,
            (LPCWSTR)(L"No valid option legs have been selected for expiration."),
            (LPCWSTR)L"Warning",
            MB_ICONWARNING | MB_OK);
        return;
    }
        
    int res = MessageBox(
        HWND_MENUPANEL,
        (LPCWSTR)(L"Are you sure you wish to EXPIRE the selected legs?"),
        (LPCWSTR)L"Confirm",
        MB_ICONWARNING | MB_YESNOCANCEL | MB_DEFBUTTON2);

    if (res != IDYES) return;

    std::shared_ptr<Transaction> trans = std::make_shared<Transaction>();

    trans->description = L"Expiration";
    trans->underlying = L"OPTIONS";
    trade->transactions.push_back(trans);

    for (auto leg : legsEdit) {

        // Save this transaction's leg quantities
        std::shared_ptr<Leg> newleg = std::make_shared<Leg>();

        newleg->underlying = trans->underlying;

        trans->transDate = leg->expiryDate;
        newleg->origQuantity = leg->openQuantity * -1;
        newleg->openQuantity = 0;
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
    TradesPanel_ShowActiveTrades();

}


// ========================================================================================
// Create transactions for option assignment for the selected leg.
// ========================================================================================
void TradesPanel_OptionAssignment(auto trade)
{
    // Do a check to ensure that there is actually legs selected for assignment. 
    if (legsEdit.size() == 0) {
        MessageBox(
            HWND_MENUPANEL,
            (LPCWSTR)(L"No valid option legs have been selected for assignment."),
            (LPCWSTR)L"Warning",
            MB_ICONWARNING | MB_OK);
        return;
    }


    std::shared_ptr<Transaction> trans;
    std::shared_ptr<Leg> newleg;

    for (auto leg : legsEdit) {

        int numShares = abs(leg->openQuantity * 100);

        std::wstring msg = L"Continue with OPTION ASSIGNMENT?\n\n";
        msg += std::to_wstring(numShares) + L" shares at $" + leg->strikePrice + L" per share.";
        int res = MessageBox(
            HWND_MENUPANEL,
            (LPCWSTR)(msg.c_str()),
            (LPCWSTR)L"Confirm",
            MB_ICONWARNING | MB_YESNOCANCEL | MB_DEFBUTTON2);

        if (res != IDYES) return;


        // Close the Option. Save this transaction's leg quantities
        trans = std::make_shared<Transaction>();
        trans->transDate = leg->expiryDate;
        trans->description = L"Assignment";
        trans->underlying = L"OPTIONS";
        trade->transactions.push_back(trans);

        newleg = std::make_shared<Leg>();
        newleg->underlying = trans->underlying;
        newleg->origQuantity = leg->openQuantity * -1;
        newleg->openQuantity = 0;
        leg->openQuantity = 0;

        if (leg->action == L"STO") newleg->action = L"BTC";
        if (leg->action == L"BTO") newleg->action = L"STC";

        newleg->expiryDate = leg->expiryDate;
        newleg->strikePrice = leg->strikePrice;
        newleg->PutCall = leg->PutCall;
        trans->legs.push_back(newleg);


        // Make the SHARES that have been assigned.
        trans = std::make_shared<Transaction>();
        trans->transDate = leg->expiryDate;
        trans->description = L"Shares";
        trans->underlying = L"SHARES";
        trans->quantity = numShares;
        trans->price = stod(leg->strikePrice);
        trans->multiplier = 1;
        trans->fees = 0;
        trans->total = trans->quantity * trans->price * -1;  // DR
        trade->ACB = trade->ACB + trans->total;
        trade->transactions.push_back(trans);

        newleg = std::make_shared<Leg>();
        newleg->underlying = trans->underlying;
        newleg->origQuantity = numShares;
        newleg->openQuantity = numShares;
        newleg->strikePrice = leg->strikePrice;
        newleg->action = L"BTO";
        trans->legs.push_back(newleg);

    }

    // Set the open status of the entire trade based on the new modified legs
    trade->setTradeOpenStatus();

    // Rebuild the openLegs position vector
    trade->createOpenLegsVector();

    // Save the new data to the database
    SaveDatabase();

    // Reload the trade list
    TradesPanel_ShowActiveTrades();
}


// ========================================================================================
// Populate vector that holds all selected lines/legs. This will be passed to the 
// TradeDialog in order to perform actions on the legs.
// ========================================================================================
void TradesPanel_PopulateLegsEditVector(HWND hListBox)
{
    legsEdit.clear();

    int nCount = ListBox_GetSelCount(hListBox);
    legsEdit.reserve(nCount);

    if (nCount) {
        int* selItems = new int[nCount]();
        SendMessage(hListBox, LB_GETSELITEMS, (WPARAM)nCount, (LPARAM)selItems);

        for (int i = 0; i < nCount; i++)
        {
            ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, selItems[i]);
            if (ld != nullptr) {
                // Only allow Option legs to be pushed to legEdit
                if (ld->leg != nullptr) {
                    legsEdit.push_back(ld->leg);
                }
            }
        }

        delete[] selItems;
    }
}



// ========================================================================================
// Handle the right-click popup menu on the ListBox's selected lines.
// ========================================================================================
void TradesPanel_RightClickMenu(HWND hListBox, int idx)
{
    HMENU hMenu = CreatePopupMenu();

    std::wstring wszText;
    std::wstring wszPlural;
    int nCount = ListBox_GetSelCount(hListBox);

    std::shared_ptr<Trade> trade = nullptr;

    bool IsTickerLine = false;

    int nCurSel = ListBox_GetCurSel(hListBox);
    ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, nCurSel);
    trade = ld->trade;
    tradeEdit = ld->trade;
    sharesAggregateEdit = ld->AggregateShares;

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
    InsertMenu(hMenu, 0, MF_BYCOMMAND | MF_STRING | MF_ENABLED, (int)TradeAction::AddSharesToTrade, L"Add Shares to Trade");
    InsertMenu(hMenu, 0, MF_BYCOMMAND | MF_STRING | MF_ENABLED, (int)TradeAction::AddFuturesToTrade, L"Add Futures to Trade");

    POINT pt; GetCursorPos(&pt);
    TradeAction selected =
        (TradeAction) TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RETURNCMD, pt.x, pt.y, 0, hListBox, NULL);



    switch (selected)
    {
    case TradeAction::ManageShares:
    case TradeAction::ManageFutures:
    case TradeAction::AddSharesToTrade:
    case TradeAction::AddFuturesToTrade:
        TradeDialog_Show(selected);
        break;

    case TradeAction::RollLeg:
    case TradeAction::CloseLeg:
        TradesPanel_PopulateLegsEditVector(hListBox);
        TradeDialog_Show(selected);
        break;
    case TradeAction::ExpireLeg:
        TradesPanel_PopulateLegsEditVector(hListBox);
        TradesPanel_ExpireSelectedLegs(trade);
        break;
    case TradeAction::Assignment:
        TradesPanel_PopulateLegsEditVector(hListBox);
        TradesPanel_OptionAssignment(trade);
        break;
    case TradeAction::AddOptionsToTrade:
    case TradeAction::AddPutToTrade:
    case TradeAction::AddCallToTrade:
        //TradesPanel_PopulateLegsEditVector(hListBox);
        TradeDialog_Show(selected);
    }

}


// ========================================================================================
// Listbox subclass Window procedure
// ========================================================================================
LRESULT CALLBACK TradesPanel_ListBox_SubclassProc(
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
        HWND hCustomVScrollBar = GetDlgItem(HWND_TRADESPANEL, IDC_TRADES_CUSTOMVSCROLLBAR);
        CustomVScrollBar_Recalculate(hCustomVScrollBar);
        return 0;
        break;
    }


    case WM_RBUTTONDOWN:
    {
        int menuId = MenuPanel_GetActiveMenuItem(HWND_MENUPANEL);
        
        if (menuId == IDC_MENUPANEL_ACTIVETRADES) {
            int idx = Listbox_ItemFromPoint(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        
            // The return value contains the index of the nearest item in the LOWORD. The HIWORD is zero 
            // if the specified point is in the client area of the list box, or one if it is outside the 
            // client area.
            if (HIWORD(idx) == -1) break;
            
            // Return to not select the line (eg. if a blank line was clicked on)
            if (TradesPanel_SelectListBoxItem(hWnd, idx) == false) {
                return 0;
            }
            ListBox_SetSel(hWnd, true, idx);

            TradesPanel_RightClickMenu(hWnd, idx);
        }
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

        // Return to not select the line (eg. if a blank line was clicked on)
        if (TradesPanel_SelectListBoxItem(hWnd, idx) == false) {
            return 0;
        }
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
            SolidBrush backBrush(GetThemeColor(ThemeElement::GrayDark));
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
        RemoveWindowSubclass(hWnd, TradesPanel_ListBox_SubclassProc, uIdSubclass);
        break;


    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);

}



// ========================================================================================
// Process WM_MEASUREITEM message for window/dialog: TradesPanel
// ========================================================================================
void TradesPanel_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT* lpMeasureItem)
{
    lpMeasureItem->itemHeight = AfxScaleY(ACTIVE_TRADES_LISTBOX_ROWHEIGHT);
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: TradesPanel
// ========================================================================================
BOOL TradesPanel_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    // Handle all of the painting in WM_PAINT
    return TRUE;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: TradesPanel
// ========================================================================================
void TradesPanel_OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    Graphics graphics(hdc);

    DWORD nBackColor = GetThemeColor(ThemeElement::GrayDark);

    // Create the background brush
    SolidBrush backBrush(nBackColor);

    // Paint the background using brush.
    int nWidth = (ps.rcPaint.right - ps.rcPaint.left);
    int nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&backBrush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Process WM_SIZE message for window/dialog: TradesPanel
// ========================================================================================
void TradesPanel_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    HWND hHeader = GetDlgItem(hwnd, IDC_TRADES_HEADER);
    HWND hListBox = GetDlgItem(hwnd, IDC_TRADES_LISTBOX);
    HWND hCustomVScrollBar = GetDlgItem(hwnd, IDC_TRADES_CUSTOMVSCROLLBAR);
        
    int margin = AfxScaleY(TRADESPANEL_MARGIN);

    HDWP hdwp = BeginDeferWindowPos(5);

    // Move and size the top label into place
    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_TRADES_LABEL), 0,
        0, 0, cx, margin, SWP_NOZORDER | SWP_SHOWWINDOW);

    // Do not call the calcVThumbRect() function during a scrollbar move. This WM_SIZE
    // gets triggered when the ListBox WM_DRAWITEM fires. If we do another calcVThumbRect()
    // calcualtion then the scrollbar will appear "jumpy" under the user's mouse cursor.
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
    hdwp = DeferWindowPos(hdwp, hListBox, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nLeft = nLeft + nWidth;   // right edge of ListBox
    nWidth = CustomVScrollBarWidth;
    hdwp = DeferWindowPos(hdwp, hCustomVScrollBar, 0, nLeft, nTop, nWidth, nHeight,
        SWP_NOZORDER | (bShowScrollBar ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));

    EndDeferWindowPos(hdwp);
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: TradesPanel
// ========================================================================================
BOOL TradesPanel_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    HWND_TRADESPANEL = hwnd;
        
    HWND hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TRADES_LABEL, L"Active Trades", 
        ThemeElement::WhiteLight, ThemeElement::Black);

    // Create an Ownerdraw fixed row sized listbox that we will use to custom
    // paint our various open trades.
    hCtl =
        TradesPanel.AddControl(Controls::ListBox, hwnd, IDC_TRADES_LISTBOX, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_MULTIPLESEL | LBS_EXTENDEDSEL | 
            LBS_OWNERDRAWFIXED | LBS_NOTIFY,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)TradesPanel_ListBox_SubclassProc,
            IDC_TRADES_LISTBOX, NULL);
    ListBox_AddString(hCtl, NULL);


    // Create our custom vertical scrollbar and attach the ListBox to it.
    CreateCustomVScrollBar(hwnd, IDC_TRADES_CUSTOMVSCROLLBAR, hCtl);

    return TRUE;
}


// ========================================================================================
// Process WM_COMMAND message for window/dialog: TradesPanel
// ========================================================================================
void TradesPanel_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (codeNotify)
    {

    case (LBN_SELCHANGE):
        int nCurSel = ListBox_GetCurSel(hwndCtl);
        if (nCurSel == -1) break;
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hwndCtl, nCurSel);
        if (ld != nullptr) {
            // Show the trade history for the selected trade
            TradesPanel_ShowListBoxItem(nCurSel);
        }
        break;

    }
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CTradesPanel::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(m_hwnd, WM_CREATE, TradesPanel_OnCreate);
        HANDLE_MSG(m_hwnd, WM_COMMAND, TradesPanel_OnCommand);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, TradesPanel_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, TradesPanel_OnPaint);
        HANDLE_MSG(m_hwnd, WM_SIZE, TradesPanel_OnSize);
        HANDLE_MSG(m_hwnd, WM_MEASUREITEM, TradesPanel_OnMeasureItem);
        HANDLE_MSG(m_hwnd, WM_DRAWITEM, ListBoxData_OnDrawItem);

    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}

