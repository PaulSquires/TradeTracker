#pragma once

#include "..\Utilities\CWindowBase.h"


class CTickerPanel : public CWindowBase<CTickerPanel>
{
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

};


const int IDC_TICKER_LISTBOX = 110;
const int IDC_TICKER_SYMBOL = 111;
const int IDC_TICKER_VSCROLLBAR = 112;
const int IDC_TICKER_LISTBOX_SUMMARY = 113;
const int IDC_TICKER_HEADER_TOTALS = 114;

const int TICKER_LISTBOX_ROWHEIGHT = 20;
const int TICKERPANEL_WIDTH = 400;
const int TICKERPANEL_MARGIN = 24;

