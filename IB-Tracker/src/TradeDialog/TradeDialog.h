#pragma once

#pragma once

#include "..\Utilities\CWindowBase.h"


class CTradeDialog : public CWindowBase<CTradeDialog>
{
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

};


const int IDC_TRADEDIALOG_LISTBOX        = 100;
const int IDC_TRADEDIALOG_VSCROLLBAR     = 101;
const int IDC_TRADEDIALOG_LBLTRANSDATE   = 102;
const int IDC_TRADEDIALOG_LBLTICKER      = 103;
const int IDC_TRADEDIALOG_LBLCOMPANY     = 104;
const int IDC_TRADEDIALOG_TRANSDATE      = 105;
const int IDC_TRADEDIALOG_TXTTICKER      = 106;
const int IDC_TRADEDIALOG_TXTCOMPANY     = 107;
const int IDC_TRADEDIALOG_TXTDESCRIPTION = 108;
const int IDC_TRADEDIALOG_LISTVIEW       = 109;
const int IDC_TRADEDIALOG_LBLQUANTITY    = 110;
const int IDC_TRADEDIALOG_LBLPRICE       = 111;
const int IDC_TRADEDIALOG_LBLFEES        = 112;
const int IDC_TRADEDIALOG_LBLTOTAL       = 113;
const int IDC_TRADEDIALOG_TXTQUANTITY    = 114;
const int IDC_TRADEDIALOG_TXTPRICE       = 115;
const int IDC_TRADEDIALOG_TXTFEES        = 116;
const int IDC_TRADEDIALOG_TXTTOTAL       = 117;
const int IDC_TRADEDIALOG_COMBODRCR      = 118;

const int IDC_TRADEDIALOG_OK             = 130;
const int IDC_TRADEDIALOG_CANCEL         = 131;

const int TRADEDIALOG_LISTBOX_ROWHEIGHT = 22;

void TradeDialog_Show();

