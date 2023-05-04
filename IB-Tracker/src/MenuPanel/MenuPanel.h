#pragma once

#include "..\Utilities\CWindowBase.h"


class CMenuPanel : public CWindowBase<CMenuPanel>
{
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

const int IDC_MENUPANEL_FIRSTITEM       = 100;
const int IDC_MENUPANEL_LOGO            = IDC_MENUPANEL_FIRSTITEM;
const int IDC_MENUPANEL_TRADERNAME      = 102;
const int IDC_MENUPANEL_APPNAME         = 103;
const int IDC_MENUPANEL_ACTIVETRADES    = 104;
const int IDC_MENUPANEL_CLOSEDTRADES    = 105;
const int IDC_MENUPANEL_CONNECTTWS      = 106;
const int IDC_MENUPANEL_TICKERTOTALS    = 107;
const int IDC_MENUPANEL_DAILYTOTALS     = 108;
const int IDC_MENUPANEL_RECONCILE       = 109;
const int IDC_MENUPANEL_ACCOUNTING      = 110;
const int IDC_MENUPANEL_NEWSHARESTRADE  = 111;
const int IDC_MENUPANEL_NEWFUTURESTRADE = 112;
const int IDC_MENUPANEL_NEWOPTIONSTRADE = 113;

const int IDC_MENUPANEL_FIRSTTEMPLATE = IDC_MENUPANEL_NEWOPTIONSTRADE;
// Constants between NEWOPTIONSTRADE and TICKERTOTALS 
// are generatedautomatically when the MenuPanel is 
// being constructed and identified trade template 
// items are being added.
const int IDC_MENUPANEL_LASTTEMPLATE = IDC_MENUPANEL_FIRSTTEMPLATE + 20;

const int IDC_MENUPANEL_LASTITEM = IDC_MENUPANEL_LASTTEMPLATE;

const int MENUPANEL_WIDTH = 180;

void MenuPanel_SelectMenuItem(HWND hParent, int CtrlId);
int MenuPanel_GetActiveMenuItem(HWND hParent);
