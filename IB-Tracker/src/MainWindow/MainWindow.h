#pragma once

#include "..\Utilities\CWindowBase.h"


class CMainWindow : public CWindowBase<CMainWindow>
{
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#define IDI_MAINICON           106

void MainWindow_ShowPanels(int state);
