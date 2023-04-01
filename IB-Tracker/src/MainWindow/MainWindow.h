#pragma once

#include "..\Utilities\CWindowBase.h"


class MainWindow : public CWindowBase<MainWindow>
{
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#define IDI_MAINICON           106
