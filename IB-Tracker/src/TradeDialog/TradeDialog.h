#pragma once

#include "..\Utilities\CWindowBase.h"


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
const int IDC_TRADEDIALOG_STRATEGY       = 112;
const int IDC_TRADEDIALOG_LBLDESCRIBE    = 113;
const int IDC_TRADEDIALOG_TXTDESCRIBE    = 114;

const int IDC_TRADEDIALOG_LBLGRIDMAIN    = 115;
const int IDC_TRADEDIALOG_LBLGRIDROLL    = 116;

const int IDC_TRADEDIALOG_LBLTRANSDATE   = 117;
const int IDC_TRADEDIALOG_CMDTRANSDATE   = 118;

const int IDC_TRADEDIALOG_LBLCONTRACT    = 119;
const int IDC_TRADEDIALOG_LBLCONTRACTDATE= 120;
const int IDC_TRADEDIALOG_CMDCONTRACTDATE= 121;

const int IDC_TRADEDIALOG_SAVE           = 125;

const int IDC_TRADEDIALOG_TABLEGRIDMAIN  = 130;
const int IDC_TRADEDIALOG_TABLEGRIDROLL  = 131;

const int IDC_TRADEDIALOG_CATEGORY       = 140;


void TradeDialog_Show(int inTradeAction);
void CalculateTradeTotal(HWND hwnd);
void LoadEditLegsInTradeTable(HWND hwnd);
void TradeDialog_SetComboDRCR(HWND hCtl, std::wstring wszText);
