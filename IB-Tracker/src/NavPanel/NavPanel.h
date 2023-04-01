#pragma once

#include "..\Utilities\CWindowBase.h"


class CNavPanel : public CWindowBase<CNavPanel>
{
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};


const int IDC_NAVPANEL = 100;

const int IDC_NAVPANEL_LOGO          = 101;
const int IDC_NAVPANEL_GEARICON      = 102;
const int IDC_NAVPANEL_USERNAME      = 103;
const int IDC_NAVPANEL_APPNAME       = 104;
const int IDC_NAVPANEL_ACTIVETRADES  = 105;
const int IDC_NAVPANEL_CLOSEDTRADES  = 106;
const int IDC_NAVPANEL_NEWTRADE      = 107;
const int IDC_NAVPANEL_SHORTSTRANGLE = 108;
const int IDC_NAVPANEL_SHORTPUT      = 109;
const int IDC_NAVPANEL_SHORTCALL     = 110;
const int IDC_NAVPANEL_TICKERTOTALS  = 111;
const int IDC_NAVPANEL_DAILYTOTALS   = 112;
const int IDC_NAVPANEL_RECONCILE     = 113;
const int IDC_NAVPANEL_CONFIGURE     = 114;
const int IDC_NAVPANEL_MESSAGES      = 115;

const int NAVPANEL_WIDTH = 180;

