#pragma once

#include "..\Utilities\CWindowBase.h"


class CDailyPanel : public CWindowBase<CDailyPanel>
{
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

};


const int IDC_DAILY_LISTBOX = 110;
const int IDC_DAILY_SYMBOL = 111;
const int IDC_DAILY_VSCROLLBAR = 112;
const int IDC_DAILY_LISTBOX_SUMMARY = 113;
const int IDC_DAILY_HEADER_TICKERTOTALS = 114;
const int IDC_DAILY_HEADER_SUMMARY = 115;
const int IDC_DAILY_HEADER_TOTALS = 116;
const int IDC_DAILY_LISTBOX_TRANSDETAILS = 117;


const int DAILY_LISTBOX_ROWHEIGHT = 20;
const int DAILYPANEL_WIDTH = 400;
const int DAILYPANEL_MARGIN = 24;

