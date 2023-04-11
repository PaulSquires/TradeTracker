#pragma once

#include "..\Utilities\CWindowBase.h"


class CConfigDialog: public CWindowBase<CConfigDialog>
{
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

};


const int IDC_CONFIGDIALOG_DARKTHEME = 100;