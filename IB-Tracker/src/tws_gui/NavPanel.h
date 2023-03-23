#pragma once

#include "CWindow.h"
#include "SuperLabel.h"

const int IDC_FRMNAVPANEL     = 100;

const int IDC_NAVPANEL_LOGO     = 101;
const int IDC_NAVPANEL_GEARICON = 102;
const int IDC_NAVPANEL_USERNAME = 103;

const int NAVPANEL_WIDTH = 180;

CWindow* NavPanel_Show(HWND hWndParent);
