/*

MIT License

Copyright(c) 2023-2024 Paul Squires

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#pragma once

#include "Utilities/CWindowBase.h"
#include "Database/trade.h"


class CStrategyButton : public CWindowBase<CStrategyButton>
{
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

};
extern CStrategyButton StrategyButton;
extern HWND HWND_STRATEGYBUTTON;


constexpr int IDC_STRATEGYBUTTON_LONGSHORT = 100;
constexpr int IDC_STRATEGYBUTTON_PUTCALL   = 101;
constexpr int IDC_STRATEGYBUTTON_STRATEGY  = 102;
constexpr int IDC_STRATEGYBUTTON_GO        = 103;
constexpr int IDC_STRATEGYBUTTON_DROPDOWN  = 104;

enum class Strategy {
    Vertical = 0,
    Strangle,
    Straddle,
    Option,
    IronCondor,
    Covered,
    Butterfly,
    RatioSpread,
    LT112,
    Count
};

std::wstring StrategyButton_GetLongShortEnumText(LongShort ls);
std::wstring StrategyButton_GetPutCallEnumText(PutCall pc);
std::wstring StrategyButton_GetStrategyEnumText(Strategy s);

void StrategyButton_ToggleLongShortText(HWND hCtl);
void StrategyButton_TogglePutCallText(HWND hCtlPutCall, HWND hCtlStrategy);
void StrategyButton_SetLongShortTextColor(HWND hCtl);
void StrategyButton_SetLongShortBackColor(HWND hCtl);
bool StrategyButton_StrategyAllowPutCall(HWND hCtl);
void StrategyButton_InvokeStrategy();

