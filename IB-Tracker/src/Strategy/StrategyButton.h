#pragma once

#include "..\Utilities\CWindowBase.h"


class CStrategyButton : public CWindowBase<CStrategyButton>
{
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

};

const int IDC_STRATEGYBUTTON_LONGSHORT = 100;
const int IDC_STRATEGYBUTTON_PUTCALL   = 101;
const int IDC_STRATEGYBUTTON_STRATEGY  = 102;
const int IDC_STRATEGYBUTTON_GO        = 103;
const int IDC_STRATEGYBUTTON_DROPDOWN  = 104;

enum class LongShort
{
    Long = 0,
    Short,
    Count 
};

enum class PutCall
{
    Put = 0,
    Call,
    Count
};

enum class Strategy
{
    Vertical = 0,
    Strangle,
    Straddle,
    Option,
    IronCondor,
    Covered,
    Butterfly,
    RatioSpread,
    Count
};



std::wstring StrategyButton_GetLongShortEnumText(LongShort ls);
std::wstring StrategyButton_GetPutCallEnumText(PutCall pc);
std::wstring StrategyButton_GetStrategyEnumText(Strategy s);

void StrategyButton_ToggleLongShortText(HWND hCtl);
void StrategyButton_TogglePutCallText(HWND hCtl);
void StrategyButton_SetLongShortColor(HWND hCtl);
bool StrategyButton_StrategyAllowPutCall();

