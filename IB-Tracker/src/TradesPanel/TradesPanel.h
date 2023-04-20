#pragma once

#include "..\Utilities\CWindowBase.h"


class CTradesPanel : public CWindowBase<CTradesPanel>
{
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

};


const int IDC_TRADES_LISTBOX = 100;
const int IDC_TRADES_LABEL = 101;
const int IDC_TRADES_VSCROLLBAR = 102;
const int IDC_TRADES_HEADER = 103;

const int ACTIVE_TRADES_LISTBOX_ROWHEIGHT = 24;
const int CLOSED_TRADES_LISTBOX_ROWHEIGHT = 18;


// These columns in the table are updated in real time when connected
// to TWS. The LineData pointer is updated via a call to SetColumnData
// and the correct ListBox line is invalidated/redrawn in order to force
// display of the new price data. Refer to TwsClient::tickPrice in the
// tws-client.cpp file to see this in action.
const int COLUMN_TICKER_ITM          = 2;    // ITM (In the Money)
const int COLUMN_TICKER_CHANGE       = 5;    // price change
const int COLUMN_TICKER_CURRENTPRICE = 6;    // current price
const int COLUMN_TICKER_PERCENTAGE   = 7;    // price percentage change

