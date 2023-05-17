#pragma once

#include "..\Utilities\CWindowBase.h"


class CTradesPanel : public CWindowBase<CTradesPanel>
{
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

};


const int IDC_TRADES_LISTBOX = 100;
const int IDC_TRADES_LABEL = 101;
const int IDC_TRADES_CustomVScrollBar = 102;
const int IDC_TRADES_HEADER = 103;

const int ACTIVE_TRADES_LISTBOX_ROWHEIGHT = 24;
const int CLOSED_TRADES_LISTBOX_ROWHEIGHT = 18;
const int TRADESPANEL_MARGIN = 24;


// These columns in the table are updated in real time when connected
// to TWS. The LineData pointer is updated via a call to SetColumnData
// and the correct ListBox line is invalidated/redrawn in order to force
// display of the new price data. Refer to TwsClient::tickPrice in the
// tws-client.cpp file to see this in action.
const int COLUMN_TICKER_ITM          = 2;    // ITM (In the Money)
const int COLUMN_TICKER_CHANGE       = 5;    // price change
const int COLUMN_TICKER_CURRENTPRICE = 6;    // current price
const int COLUMN_TICKER_PERCENTAGE   = 7;    // price percentage change


// Various actions that can be performed on selected Trade or Legs.
// TODO: These Actions should probably be a enum class
const int ACTION_NEW_TRADE          = 1;
const int ACTION_ROLL_LEG           = 2;
const int ACTION_CLOSE_LEG          = 3;
const int ACTION_EXPIRE_LEG         = 4;
const int ACTION_SHARE_ASSIGNMENT   = 5;
const int ACTION_ADDTO_TRADE        = 6;
const int ACTION_ADDPUTTO_TRADE     = 7;
const int ACTION_ADDCALLTO_TRADE    = 8;
const int ACTION_NEW_IRONCONDOR     = 9;
const int ACTION_NEW_SHORTSTRANGLE  = 10;
const int ACTION_NEW_SHORTPUT       = 12;
const int ACTION_NEW_SHORTCALL      = 13;
const int ACTION_NOACTION           = 20;


bool IsNewTradeAction(const int action);

