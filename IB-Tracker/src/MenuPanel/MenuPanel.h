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
const int IDC_MENUPANEL_TRANSACTIONS    = 110;
const int IDC_MENUPANEL_NEWSHARESTRADE  = 111;
const int IDC_MENUPANEL_NEWFUTURESTRADE = 112;
const int IDC_MENUPANEL_NEWOPTIONSTRADE = 113;

const int IDC_MENUPANEL_NEWIRONCONDOR   = 114;
const int IDC_MENUPANEL_NEWSHORTSTRANGLE= 115;
const int IDC_MENUPANEL_NEWSHORTPUT     = 116;
const int IDC_MENUPANEL_NEWSHORTCALL    = 117;

const int IDC_MENUPANEL_LASTITEM = IDC_MENUPANEL_NEWSHORTCALL;

const int MENUPANEL_WIDTH = 180;

void MenuPanel_SelectMenuItem(HWND hParent, int CtrlId);
int MenuPanel_GetActiveMenuItem(HWND hParent);
