#pragma once

#include "..\Utilities\CWindowBase.h"


class CStrategyButton : public CWindowBase<CStrategyButton>
{
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

};

const int IDC_STRATEGYBUTTON_SHORTLONG = 100;
const int IDC_STRATEGYBUTTON_PUTCALL   = 101;
const int IDC_STRATEGYBUTTON_STRATEGY  = 102;
const int IDC_STRATEGYBUTTON_GO        = 103;
const int IDC_STRATEGYBUTTON_DROPDOWN  = 104;


