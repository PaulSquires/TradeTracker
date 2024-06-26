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

#pragma once

#include "Utilities/CWindowBase.h"
#include "Utilities/UserMessages.h"
#include "Utilities/ListBoxData.h"
#include "Database/trade.h"


typedef long TickerId;

enum class ActiveTradesFilterType {
    Category,
    Expiration,
    TickerSymbol
};

enum class NewTradeType {
    no_action,
    Custom,
    IronCondor,
    ShortStrangle,
    ShortPut,
    ShortPutVertical,
    ShortCall,
    ShortCallVertical,
    ShortPut112,
    SharesTrade,
    FuturesTrade,
    OtherIncomeExpense
};


class CActiveTrades : public CWindowBase<CActiveTrades> {
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

    HWND hWindow = NULL;

    HWND TradesListBox();
    HWND PaperTradingLabel();
    HWND SortFilterLabel();
    HWND SortFilterCombo();
    HWND SortFilterButton();
    HWND NewTradeLabel();
    HWND NewTradeCombo();
    HWND NewTradeButton();
    HWND NetLiquidationLabel();
    HWND NetLiquidationValueLabel();
    HWND ExcessLiquidityLabel();
    HWND ExcessLiquidityValueLabel();
    HWND MaintenanceLabel();
    HWND MaintenanceValueLabel();
    HWND VScrollBar();

    ActiveTradesFilterType filter_type = ActiveTradesFilterType::Category;
    NewTradeType new_trade_type = NewTradeType::Custom;

    bool pause_live_updates = false;
    bool positions_requested = false;

    void ShowActiveTrades();
    void PerformITMcalculation(std::shared_ptr<Trade>& trade);
    bool IsAddOptionToTradeAction(TradeAction action);
    bool IsAddSharesToTradeAction(TradeAction action);
    bool IsNewOptionsTradeAction(TradeAction action);
    bool IsNewFuturesTradeAction(TradeAction action);
    bool IsNewSharesTradeAction(TradeAction action);
    bool IsManageSharesTradeAction(TradeAction action);
    void UpdateTickerPrices();
    int ShowHideLiquidityLabels();
    void DisplayPaperTradingWarning();

private:
    bool OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
    void OnSize(HWND hwnd, UINT state, int cx, int cy);
    void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
    void OnPaint(HWND hwnd);
    bool OnEraseBkgnd(HWND hwnd, HDC hdc);
    void OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT* lpMeasureItem);

    std::wstring GetSortFilterDescription(int index);
    std::wstring GetNewTradeDescription(int index);
    void UpdateTickerPricesLine(int index, ListBoxData* ld);
    void UpdateTickerPortfolioLine(int index, int index_trade, ListBoxData* ld);
    void UpdateLegPortfolioLine(int index, ListBoxData* ld);
    void ExpireSelectedLegs(auto trade);
    void CalledAwayAssignment(auto trade, auto leg, int aggregate_shares, int aggregate_futures);
    void CreateAssignment(auto trade, auto leg);
    void OptionAssignment(auto trade);
    void PopulateLegsEditVector(HWND hListBox);
    
    void ShowListBoxItem(int index);
    void RightClickMenu(HWND hListBox, int idx);
    bool SelectListBoxItem(HWND hListBox, int idx);
    
    static LRESULT CALLBACK ListBox_SubclassProc(
        HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
        UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
};


constexpr int IDC_ACTIVETRADES_LISTBOX = 100;
constexpr int IDC_ACTIVETRADES_CUSTOMVSCROLLBAR = 102;
constexpr int IDC_ACTIVETRADES_LBLSORTFILTER = 104;
constexpr int IDC_ACTIVETRADES_SORTFILTER = 105;
constexpr int IDC_ACTIVETRADES_CMDSORTFILTER = 106;
constexpr int IDC_ACTIVETRADES_LBLNEWTRADE = 107;
constexpr int IDC_ACTIVETRADES_NEWTRADE = 108;
constexpr int IDC_ACTIVETRADES_CMDNEWTRADE = 109;
constexpr int IDC_ACTIVETRADES_NETLIQUIDATION = 110;
constexpr int IDC_ACTIVETRADES_NETLIQUIDATION_VALUE = 111;
constexpr int IDC_ACTIVETRADES_EXCESSLIQUIDITY = 112;
constexpr int IDC_ACTIVETRADES_EXCESSLIQUIDITY_VALUE = 113;
constexpr int IDC_ACTIVETRADES_MAINTENANCE = 114;
constexpr int IDC_ACTIVETRADES_MAINTENANCE_VALUE = 115;
constexpr int IDC_ACTIVETRADES_PAPERWARNING = 116;

constexpr int ACTIVETRADES_LISTBOX_ROWHEIGHT = 24;
constexpr int ACTIVETRADES_MARGIN = 80;


// These columns in the table are updated in real time when connected
// to TWS. The LineData pointer is updated via a call to SetColumnData
// and the correct ListBox line is invalidated/redrawn in order to force
// display of the new price data. Refer to TwsClient::tickPrice in the
// tws-client.cpp file to see this in action.
constexpr int COLUMN_TICKER_ITM             = 2;    // ITM (In the Money)
constexpr int COLUMN_TICKER_CHANGE          = 5;    // price change
constexpr int COLUMN_TICKER_CURRENTPRICE    = 6;    // current price
constexpr int COLUMN_TICKER_PERCENTCHANGE   = 7;    // price percentage change

// These columns in the table are updated in real time when connected
// to TWS. The LineData pointer is updated via a call to SetColumnData
// and the correct ListBox line is invalidated/redrawn in order to force
// display of the new price data. Refer to TwsClient::updatePortfolio in the
// tws-client.cpp file to see this in action.
constexpr int COLUMN_TICKER_PORTFOLIO_1 = 8; 
constexpr int COLUMN_TICKER_PORTFOLIO_2 = 9; 
constexpr int COLUMN_TICKER_PORTFOLIO_3 = 10;
constexpr int COLUMN_TICKER_PORTFOLIO_4 = 11;
constexpr int COLUMN_TICKER_PORTFOLIO_5 = 12;

extern CActiveTrades ActiveTrades;

constexpr int MSG_ACTIVETRADES_SELECTLISTBOXITEM = WM_USER + 1014;
constexpr int MSG_ACTIVETRADES_RIGHTCLICKMENU = WM_USER + 1015;
