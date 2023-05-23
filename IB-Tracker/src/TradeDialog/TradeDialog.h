#pragma once

#include "..\Utilities\CWindowBase.h"
#include "..\TradesPanel\TradesPanel.h"


class CTradeDialog : public CWindowBase<CTradeDialog>
{
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

};


const int IDC_TRADEDIALOG_LBLEDITACTION  = 101;
const int IDC_TRADEDIALOG_LBLTICKER      = 102;
const int IDC_TRADEDIALOG_LBLCOMPANY     = 103;
const int IDC_TRADEDIALOG_TXTTICKER      = 104;
const int IDC_TRADEDIALOG_TXTCOMPANY     = 105;
const int IDC_TRADEDIALOG_TXTQUANTITY    = 106;
const int IDC_TRADEDIALOG_TXTMULTIPLIER  = 107;
const int IDC_TRADEDIALOG_TXTPRICE       = 108;
const int IDC_TRADEDIALOG_TXTFEES        = 109;
const int IDC_TRADEDIALOG_TXTTOTAL       = 110;
const int IDC_TRADEDIALOG_COMBODRCR      = 111;
const int IDC_TRADEDIALOG_LBLSTRATEGY    = 112;
const int IDC_TRADEDIALOG_STRATEGY       = 113;
const int IDC_TRADEDIALOG_LBLDESCRIBE    = 114;
const int IDC_TRADEDIALOG_TXTDESCRIBE    = 115;

const int IDC_TRADEDIALOG_LBLGRIDMAIN    = 120;
const int IDC_TRADEDIALOG_LBLGRIDROLL    = 121;

const int IDC_TRADEDIALOG_LBLTRANSDATE   = 125;
const int IDC_TRADEDIALOG_CMDTRANSDATE   = 126;

const int IDC_TRADEDIALOG_LBLCONTRACT    = 130;
const int IDC_TRADEDIALOG_LBLCONTRACTDATE= 131;
const int IDC_TRADEDIALOG_CMDCONTRACTDATE= 132;

const int IDC_TRADEDIALOG_SAVE           = 140;

const int IDC_TRADEDIALOG_TABLEGRIDMAIN  = 150;
const int IDC_TRADEDIALOG_TABLEGRIDROLL  = 151;
const int IDC_TRADEDIALOG_BUYSHARES      = 152;
const int IDC_TRADEDIALOG_BUYSHARES_DROPDOWN = 153;
const int IDC_TRADEDIALOG_SELLSHARES     = 154;
const int IDC_TRADEDIALOG_SELLSHARES_DROPDOWN = 155;

const int IDC_TRADEDIALOG_CATEGORY       = 160;


int TradeDialog_Show(TradeAction inTradeAction);
void TradeDialog_CalculateTradeTotal(HWND hwnd);
void TradeDialog_LoadEditLegsInTradeTable(HWND hwnd);
void TradeDialog_SetComboDRCR(HWND hCtl, std::wstring wszText);
void TradeDialog_SetLongShortBackColor(HWND hCtl);
void TradeDialog_ToggleBuyLongShortText(HWND hCtl);
void TradeDialog_ToggleSellLongShortText(HWND hCtl);
bool TradeDialog_ValidateSharesTradeData(HWND hwnd);
void TradeDialog_CreateSharesTradeData(HWND hwnd);
bool TradeDialog_ValidateOptionsTradeData(HWND hwnd);
void TradeDialog_CreateOptionsTradeData(HWND hwnd);
void TradeDialogControls_ShowFuturesContractDate(HWND hwnd);

