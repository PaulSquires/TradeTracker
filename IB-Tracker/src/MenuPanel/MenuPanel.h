#pragma once

#include "..\Utilities\CWindowBase.h"


class CMenuPanel : public CWindowBase<CMenuPanel>
{
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};


const int IDC_MENUPANEL_LOGO          = 101;
const int IDC_MENUPANEL_GEARICON      = 102;
const int IDC_MENUPANEL_TRADERNAME    = 103;
const int IDC_MENUPANEL_APPNAME       = 104;
const int IDC_MENUPANEL_ACTIVETRADES  = 105;
const int IDC_MENUPANEL_CLOSEDTRADES  = 106;
const int IDC_MENUPANEL_NEWTRADE      = 107;

// Constants between NEWTRADE and TICKERTOTALS are generated
// automatically when the MenuPanel is being constructed and 
// identified trade template items are being added.

const int IDC_MENUPANEL_TICKERTOTALS  = 200;
const int IDC_MENUPANEL_DAILYTOTALS   = 201;
const int IDC_MENUPANEL_RECONCILE     = 202;
const int IDC_MENUPANEL_MESSAGES      = 203;

const int MENUPANEL_WIDTH = 180;

void MenuPanel_SelectMenuItem(HWND hParent, int CtrlId);
int MenuPanel_GetActiveMenuItem(HWND hParent);
