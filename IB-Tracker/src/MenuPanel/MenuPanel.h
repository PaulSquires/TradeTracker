#pragma once

#include "..\Utilities\CWindowBase.h"


class CMenuPanel : public CWindowBase<CMenuPanel>
{
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};


const int IDC_MENUPANEL_LOGO          = 101;
const int IDC_MENUPANEL_GEARICON      = 102;
const int IDC_MENUPANEL_USERNAME      = 103;
const int IDC_MENUPANEL_APPNAME       = 104;
const int IDC_MENUPANEL_ACTIVETRADES  = 105;
const int IDC_MENUPANEL_CLOSEDTRADES  = 106;
const int IDC_MENUPANEL_NEWTRADE      = 107;
const int IDC_MENUPANEL_SHORTSTRANGLE = 108;
const int IDC_MENUPANEL_SHORTPUT      = 109;
const int IDC_MENUPANEL_SHORTCALL     = 110;
const int IDC_MENUPANEL_TICKERTOTALS  = 111;
const int IDC_MENUPANEL_DAILYTOTALS   = 112;
const int IDC_MENUPANEL_RECONCILE     = 113;
const int IDC_MENUPANEL_CONFIGURE     = 114;
const int IDC_MENUPANEL_MESSAGES      = 115;

const int MENUPANEL_WIDTH = 180;

