#pragma once

#include "..\Utilities\CWindowBase.h"
#include "..\Templates\Templates.h"


class CTradeDialog : public CWindowBase<CTradeDialog>
{
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

};


const int IDC_TRADEDIALOG_TEMPLATES      = 100;
const int IDC_TRADEDIALOG_VSCROLLBAR     = 101;
const int IDC_TRADEDIALOG_TRANSDATE      = 105;
const int IDC_TRADEDIALOG_TXTTICKER      = 106;
const int IDC_TRADEDIALOG_TXTCOMPANY     = 107;
const int IDC_TRADEDIALOG_TXTDESCRIPTION = 108;
const int IDC_TRADEDIALOG_HEADER         = 109;
const int IDC_TRADEDIALOG_LBLQUANTITY    = 111;
const int IDC_TRADEDIALOG_LBLMULTIPLIER  = 112;
const int IDC_TRADEDIALOG_LBLPRICE       = 113;
const int IDC_TRADEDIALOG_LBLFEES        = 114;
const int IDC_TRADEDIALOG_LBLTOTAL       = 115;
const int IDC_TRADEDIALOG_TXTQUANTITY    = 116;
const int IDC_TRADEDIALOG_TXTMULTIPLIER  = 117;
const int IDC_TRADEDIALOG_TXTPRICE       = 118;
const int IDC_TRADEDIALOG_TXTFEES        = 119;
const int IDC_TRADEDIALOG_TXTTOTAL       = 120;
const int IDC_TRADEDIALOG_COMBODRCR      = 121;
const int IDC_TRADEDIALOG_FRAME1         = 122;
const int IDC_TRADEDIALOG_FRAME2         = 123;
const int IDC_TRADEDIALOG_LBLEDITACTION  = 124;

const int IDC_TRADEDIALOG_OK             = 130;
const int IDC_TRADEDIALOG_CANCEL         = 131;

const int IDC_TRADEDIALOG_TABLEACTION    = 140;   // 12 controls
const int IDC_TRADEDIALOG_TABLEQUANTITY  = 160;   // 12 controls
const int IDC_TRADEDIALOG_TABLESTRIKE    = 180;   // 12 controls
const int IDC_TRADEDIALOG_TABLEEXPIRY    = 200;   // 12 controls
const int IDC_TRADEDIALOG_TABLEPUTCALL   = 220;   // 12 controls
const int IDC_TRADEDIALOG_TABLEDTE       = 240;   // 12 controls


const int TRADEDIALOG_TEMPLATES_ROWHEIGHT  = 22;
const int TRADEDIALOG_TRADETABLE_ROWHEIGHT = 20;
const int TRADEDIALOG_TRADETABLE_WIDTH     = 560;
const int TRADEDIALOG_TRADETABLE_NUMROWS   = 12;
const int TRADEDIALOG_TRADETEMPLATES_WIDTH = 180;

void TradeDialog_Show(int inTradeAction, CTradeTemplate* pTradeTemplate = nullptr);
void FormatNumberFourDecimals(HWND hCtl);
void CalculateTradeTotal(HWND hwnd);
void CalculateTradeDTE(HWND hwnd);
void LoadTemplateInTradeTable(HWND hwnd, CTradeTemplate* pTradeTemplate);
void LoadEditLegsInTradeTable(HWND hwnd);
