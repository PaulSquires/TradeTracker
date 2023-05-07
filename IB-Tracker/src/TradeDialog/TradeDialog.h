#pragma once

#include "..\Utilities\CWindowBase.h"


class CTradeDialog : public CWindowBase<CTradeDialog>
{
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

};


const int IDC_TRADEDIALOG_TRANSDATE      = 105;
const int IDC_TRADEDIALOG_TXTTICKER      = 106;
const int IDC_TRADEDIALOG_TXTCOMPANY     = 107;
const int IDC_TRADEDIALOG_TXTDESCRIPTION = 108;
const int IDC_TRADEDIALOG_HEADER         = 109;
const int IDC_TRADEDIALOG_TXTQUANTITY    = 116;
const int IDC_TRADEDIALOG_TXTMULTIPLIER  = 117;
const int IDC_TRADEDIALOG_TXTPRICE       = 118;
const int IDC_TRADEDIALOG_TXTFEES        = 119;
const int IDC_TRADEDIALOG_TXTTOTAL       = 120;
const int IDC_TRADEDIALOG_COMBODRCR      = 121;
const int IDC_TRADEDIALOG_LBLEDITACTION  = 125;
const int IDC_TRADEDIALOG_LBLTICKER      = 126;
const int IDC_TRADEDIALOG_LBLCOMPANY     = 127;

const int IDC_TRADEDIALOG_SAVE           = 130;

const int IDC_TRADEDIALOG_TABLEGRID      = 140;


const int TRADEDIALOG_TRADETABLE_WIDTH     = 442;
const int TRADEDIALOG_TRADETABLE_NUMROWS   = 4;

void TradeDialog_Show(int inTradeAction);
void FormatNumberFourDecimals(HWND hCtl);
void CalculateTradeTotal(HWND hwnd);
void CalculateTradeDTE(HWND hwnd);
void LoadEditLegsInTradeTable(HWND hwnd);
void TradeDialog_SetComboDRCR(HWND hCtl, std::wstring wszText);
