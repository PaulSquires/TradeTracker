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

#pragma once

#include "Utilities/CWindowBase.h"
#include "Utilities/UserMessages.h"
#include "Database/trade.h"


class CTradeDialog : public CWindowBase<CTradeDialog>
{
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

};


class TradeDialogData
{
public:
    TradeAction trade_action = TradeAction::no_action;
    std::shared_ptr<Trade> trade = nullptr;
    std::shared_ptr<Transaction> trans = nullptr;
    std::vector< std::shared_ptr<Leg> > legs;
    std::wstring shares_aggregate_edit = L"0";

    void ResetDefaults() {
        trade_action = TradeAction::no_action;
        trade = nullptr;
        trans = nullptr;
        legs.clear();
        shares_aggregate_edit = L"0";
    }
};

extern TradeDialogData tdd;
extern HWND HWND_TRADEDIALOG;
extern int dialog_return_code;


constexpr int IDC_TRADEDIALOG_LBLEDITACTION  = 101;
constexpr int IDC_TRADEDIALOG_LBLTICKER      = 102;
constexpr int IDC_TRADEDIALOG_LBLCOMPANY     = 103;
constexpr int IDC_TRADEDIALOG_TXTTICKER      = 104;
constexpr int IDC_TRADEDIALOG_TXTCOMPANY     = 105;
constexpr int IDC_TRADEDIALOG_TXTQUANTITY    = 106;
constexpr int IDC_TRADEDIALOG_TXTMULTIPLIER  = 107;
constexpr int IDC_TRADEDIALOG_TXTPRICE       = 108;
constexpr int IDC_TRADEDIALOG_TXTFEES        = 109;
constexpr int IDC_TRADEDIALOG_TXTTOTAL       = 110;
constexpr int IDC_TRADEDIALOG_COMBODRCR      = 111;
constexpr int IDC_TRADEDIALOG_TXTTRADEBP     = 112;
constexpr int IDC_TRADEDIALOG_LBLTRADEBP     = 113;
constexpr int IDC_TRADEDIALOG_LBLSTRATEGY    = 114;
constexpr int IDC_TRADEDIALOG_STRATEGY       = 115;
constexpr int IDC_TRADEDIALOG_LBLDESCRIBE    = 116;
constexpr int IDC_TRADEDIALOG_TXTDESCRIBE    = 117;
constexpr int IDC_TRADEDIALOG_LBLEDITWARNING1= 118;
constexpr int IDC_TRADEDIALOG_LBLEDITWARNING2= 119;
constexpr int IDC_TRADEDIALOG_LBLEDITWARNING3= 120;

constexpr int IDC_TRADEDIALOG_LBLGRIDMAIN    = 122;
constexpr int IDC_TRADEDIALOG_LBLGRIDROLL    = 123;

constexpr int IDC_TRADEDIALOG_LBLTRANSDATE   = 125;
constexpr int IDC_TRADEDIALOG_CMDTRANSDATE   = 126;

constexpr int IDC_TRADEDIALOG_LBLCONTRACT    = 130;
constexpr int IDC_TRADEDIALOG_LBLCONTRACTDATE= 131;
constexpr int IDC_TRADEDIALOG_CMDCONTRACTDATE= 132;

constexpr int IDC_TRADEDIALOG_SAVE           = 140;

constexpr int IDC_TRADEDIALOG_TABLEGRIDMAIN  = 150;
constexpr int IDC_TRADEDIALOG_TABLEGRIDROLL  = 151;
constexpr int IDC_TRADEDIALOG_BUYSHARES      = 152;
constexpr int IDC_TRADEDIALOG_BUYSHARES_DROPDOWN = 153;
constexpr int IDC_TRADEDIALOG_SELLSHARES     = 154;
constexpr int IDC_TRADEDIALOG_SELLSHARES_DROPDOWN = 155;

constexpr int IDC_TRADEDIALOG_CATEGORY       = 160;

std::wstring RemovePipeChar(const std::wstring& text);
int TradeDialog_Show(TradeAction inTradeAction);
void TradeDialog_CalculateTradeTotal(HWND hwnd);
void TradeDialog_LoadEditLegsInTradeTable(HWND hwnd);
void TradeDialog_SetComboDRCR(HWND hCtl, std::wstring text);
void TradeDialog_SetLongShortback_color(HWND hCtl);
void TradeDialog_ToggleBuyLongShortText(HWND hCtl);
void TradeDialog_ToggleSellLongShortText(HWND hCtl);
bool TradeDialog_ValidateSharesTradeData(HWND hwnd);
void TradeDialog_CreateSharesTradeData(HWND hwnd);
bool TradeDialog_ValidateOptionsTradeData(HWND hwnd);
void TradeDialog_CreateOptionsTradeData(HWND hwnd);
void TradeDialogControls_ShowFuturesContractDate(HWND hwnd);
bool TradeDialog_ValidateEditTradeData(HWND hwnd);
void TradeDialog_CreateEditTradeData(HWND hwnd);
bool TradeDialog_ValidateDividendTradeData(HWND hwnd);
void TradeDialog_CreateDividendTradeData(HWND hwnd);
bool TradeDialog_ValidateOtherIncomeData(HWND hwnd);
void TradeDialog_CreateOtherIncomeData(HWND hwnd);

