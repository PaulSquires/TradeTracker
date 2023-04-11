#pragma once

#include "..\Utilities\CWindowBase.h"


class CConfigDialog: public CWindowBase<CConfigDialog>
{
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

};


const int IDC_CONFIGDIALOG_DARKTHEME = 100;
const int IDC_CONFIGDIALOG_DARKPLUSTHEME = 101; 
const int IDC_CONFIGDIALOG_BLUETHEME = 102; 
const int IDC_CONFIGDIALOG_LIGHTTHEME = 103; 
const int IDC_CONFIGDIALOG_TRADERNAME = 104;
const int IDC_CONFIGDIALOG_STARTUPCONNECT = 105;
const int IDC_CONFIGDIALOG_OK = 106;
const int IDC_CONFIGDIALOG_CANCEL = 107;


