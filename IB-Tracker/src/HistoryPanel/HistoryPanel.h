#pragma once

#include "..\Utilities\CWindowBase.h"


class CHistoryPanel : public CWindowBase<CHistoryPanel>
{
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

};


const int IDC_HISTORY_LISTBOX = 110;
const int IDC_HISTORY_SYMBOL = 111;
const int IDC_HISTORY_VSCROLLBAR = 112;
const int IDC_HISTORY_LISTBOX_SUMMARY = 113;
const int IDC_HISTORY_LISTBOX_TRANSDETAILS = 117;


const int HISTORY_LISTBOX_ROWHEIGHT = 20;
const int TICKER_TOTALS_LISTBOX_ROWHEIGHT = 16;
const int HISTORYPANEL_WIDTH = 400;
const int HISTORYPANEL_MARGIN = 24;

