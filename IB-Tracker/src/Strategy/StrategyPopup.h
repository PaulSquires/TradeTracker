#pragma once

#include "..\Utilities\CWindowBase.h"


class CStrategyPopup : public CWindowBase<CStrategyPopup>
{
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

};

const int IDC_STRATEGYPOPUP_LONGSHORT = 100;
const int IDC_STRATEGYPOPUP_PUTCALL   = 120;
const int IDC_STRATEGYPOPUP_STRATEGY  = 140;
const int IDC_STRATEGYPOPUP_GO        = 150;
