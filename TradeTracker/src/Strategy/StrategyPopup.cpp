/*

MIT License

Copyright(c) 2023 Paul Squires

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

#include "pch.h"
#include "Utilities/CWindowBase.h"
#include "CustomLabel/CustomLabel.h"

#include "StrategyButton.h"
#include "StrategyPopup.h"


HWND HWND_STRATEGYPOPUP = NULL;

CStrategyPopup StrategyPopup;

HHOOK hStrategyPopupMouseHook = nullptr;



// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: StrategyPopup
// ========================================================================================
BOOL StrategyPopup_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    // Handle all of the painting in WM_PAINT
    return TRUE;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: StrategyPopup
// ========================================================================================
void StrategyPopup_OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    Graphics graphics(hdc);

    // Create the background brush
    SolidBrush back_brush(COLOR_GRAYDARK);

    // Paint the background using brush.
    int nWidth = (ps.rcPaint.right - ps.rcPaint.left);
    int nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: StrategyPopup
// ========================================================================================
BOOL StrategyPopup_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    HWND_STRATEGYPOPUP = hwnd;

    int nHeight = 23;
    int nTop = 0;

    HWND hCtl = NULL;
    HWND hCtlPutCall = NULL;
    HWND hCtlLongShort = NULL;

    std::wstring font_name = AfxGetDefaultFont();
    std::wstring text;
    int font_size = 8;
    bool bold = true;

    for (int i = 0; i < (int)Strategy::Count; ++i) {

        hCtl = CustomLabel_SimpleLabel(hwnd, IDC_STRATEGYPOPUP_LONGSHORT + i, L"",
            COLOR_WHITELIGHT, COLOR_GRAYMEDIUM,
            CustomLabelAlignment::middle_left, 0, nTop, 50, nHeight);
        CustomLabel_SetUserDataInt(hCtl, (int)LongShort::Short);
        CustomLabel_SetFont(hCtl, font_name, font_size, bold);
        CustomLabel_SetTextOffset(hCtl, 5, 0);
        CustomLabel_SetMousePointer(hCtl, CustomLabelPointer::hand, CustomLabelPointer::hand);
        text = AfxUpper(StrategyButton_GetLongShortEnumText(LongShort::Short));
        CustomLabel_SetText(hCtl, text);
        hCtlLongShort = hCtl;

        hCtl = CustomLabel_SimpleLabel(hwnd, IDC_STRATEGYPOPUP_PutCall + i, L"",
            COLOR_WHITELIGHT, COLOR_GRAYMEDIUM,
            CustomLabelAlignment::middle_center, 51, nTop, 50, nHeight);
        CustomLabel_SetUserDataInt(hCtl, (int)PutCall::Put);
        CustomLabel_SetFont(hCtl, font_name, font_size, bold);
        CustomLabel_SetMousePointer(hCtl, CustomLabelPointer::hand, CustomLabelPointer::hand);
        text = AfxUpper(StrategyButton_GetPutCallEnumText(PutCall::Put));
        CustomLabel_SetText(hCtl, text);
        hCtlPutCall = hCtl;

        hCtl = CustomLabel_SimpleLabel(hwnd, IDC_STRATEGYPOPUP_STRATEGY + i, L"",
            COLOR_WHITELIGHT, COLOR_GRAYMEDIUM,
            CustomLabelAlignment::middle_left, 102, nTop, 100, nHeight);
        CustomLabel_SetUserDataInt(hCtl, (int)i);
        CustomLabel_SetFont(hCtl, font_name, font_size, bold);
        CustomLabel_SetTextOffset(hCtl, 5, 0);
        text = AfxUpper(StrategyButton_GetStrategyEnumText((Strategy)i));
        CustomLabel_SetText(hCtl, text);
        if (!StrategyButton_StrategyAllowPutCall(hCtl)) {
            CustomLabel_SetText(hCtlPutCall, L"");
        }
        StrategyButton_SetLongShortTextColor(hCtlLongShort);

        hCtl = CustomLabel_ButtonLabel(hwnd, IDC_STRATEGYPOPUP_GO + i, L"GO",
            COLOR_BLACK, COLOR_BLUE, COLOR_BLUE, COLOR_GRAYMEDIUM, COLOR_WHITE,
            CustomLabelAlignment::middle_center, 203, nTop, 30, nHeight);
        CustomLabel_SetMousePointer(hCtl, CustomLabelPointer::hand, CustomLabelPointer::hand);
        CustomLabel_SetFont(hCtl, font_name, font_size, true);
        CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);

        nTop += nHeight + 1;

    }

    return TRUE;
}


// ========================================================================================
// Global mouse hook.
// ========================================================================================
LRESULT CALLBACK StrategyPopupHook(int Code, WPARAM wParam, LPARAM lParam)
{
    // messages are defined in a linear way the first being WM_LBUTTONUP up to WM_MBUTTONDBLCLK
    // this subset does not include WM_MOUSEMOVE, WM_MOUSEWHEEL and a few others
    // (Don't handle WM_LBUTTONUP here because the mouse is most likely outside the menu popup
    // at the point this hook is called).
    if (wParam == WM_LBUTTONDOWN)
    {
        if (HWND_STRATEGYPOPUP)
        {
            POINT pt;       GetCursorPos(&pt);
            RECT rcWindow;  GetWindowRect(HWND_STRATEGYPOPUP, &rcWindow);

            // if the mouse action is outside the menu, hide it. the window procedure will also unset this hook 
            if (!PtInRect(&rcWindow, pt)) {
                DestroyWindow(HWND_STRATEGYPOPUP);
            }
        }
    }

    return CallNextHookEx(NULL, Code, wParam, lParam);
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CStrategyPopup::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(m_hwnd, WM_CREATE, StrategyPopup_OnCreate);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, StrategyPopup_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, StrategyPopup_OnPaint);


    case WM_DESTROY:
    {
        // unhook and remove our global mouse hook
        UnhookWindowsHookEx(hStrategyPopupMouseHook);
        hStrategyPopupMouseHook = nullptr;

        HWND_STRATEGYPOPUP = NULL;
        return 0;
    }
    break;


    case WM_MOUSEACTIVATE:
    {
        return MA_NOACTIVATE;
    }
    break;


    case MSG_CUSTOMLABEL_CLICK:
    {
        HWND hCtl = (HWND)lParam;
        int CtrlId = (int)wParam;

        if (hCtl == NULL) return 0;

        if (CtrlId >= IDC_STRATEGYPOPUP_LONGSHORT && CtrlId <= IDC_STRATEGYPOPUP_LONGSHORT + 40) {
            StrategyButton_ToggleLongShortText(hCtl);
            StrategyButton_SetLongShortTextColor(hCtl);
        }
        if (CtrlId >= IDC_STRATEGYPOPUP_PutCall && CtrlId <= IDC_STRATEGYPOPUP_PutCall + 40) {
            int offset = CtrlId - IDC_STRATEGYPOPUP_PutCall;
            int idStrategy = IDC_STRATEGYPOPUP_STRATEGY + offset;
            StrategyButton_TogglePutCallText(hCtl, GetDlgItem(HWND_STRATEGYPOPUP, idStrategy));
        }
        if (CtrlId >= IDC_STRATEGYPOPUP_GO && CtrlId <= IDC_STRATEGYPOPUP_GO + 40) {
            int offset = CtrlId - IDC_STRATEGYPOPUP_GO;
            std::wstring text;

            LongShort ls = (LongShort)CustomLabel_GetUserDataInt(GetDlgItem(m_hwnd, IDC_STRATEGYPOPUP_LONGSHORT+offset));
            HWND hCtlLongShort = GetDlgItem(HWND_STRATEGYBUTTON, IDC_STRATEGYBUTTON_LONGSHORT);
            CustomLabel_SetUserDataInt(hCtlLongShort, (int)ls);
            StrategyButton_SetLongShortBackColor(hCtlLongShort);
            text = AfxUpper(StrategyButton_GetLongShortEnumText(ls));
            CustomLabel_SetText(hCtlLongShort, text);

            PutCall pc = (PutCall)CustomLabel_GetUserDataInt(GetDlgItem(m_hwnd, IDC_STRATEGYPOPUP_PutCall+offset));
            text = AfxUpper(StrategyButton_GetPutCallEnumText(pc));
            HWND hCtlPutCall = GetDlgItem(HWND_STRATEGYBUTTON, IDC_STRATEGYBUTTON_PUTCALL);
            CustomLabel_SetUserDataInt(hCtlPutCall, (int)pc);
            text = AfxUpper(StrategyButton_GetPutCallEnumText(pc));
            CustomLabel_SetText(hCtlPutCall, text);

            Strategy s = (Strategy)CustomLabel_GetUserDataInt(GetDlgItem(m_hwnd, IDC_STRATEGYPOPUP_STRATEGY+offset));
            HWND hCtlStrategy = GetDlgItem(HWND_STRATEGYBUTTON, IDC_STRATEGYBUTTON_STRATEGY);
            CustomLabel_SetUserDataInt(hCtlStrategy, (int)s);
            text = AfxUpper(StrategyButton_GetStrategyEnumText(s));
            CustomLabel_SetText(hCtlStrategy, text);
            if (!StrategyButton_StrategyAllowPutCall(hCtlStrategy)) {
                CustomLabel_SetText(hCtlPutCall, L"");
            }

            StrategyButton_InvokeStrategy();
            DestroyWindow(m_hwnd);

        }
        return 0;
    }
    break;

    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}


// ========================================================================================
// Create an show the modeless popup Strategy picker panel
// ========================================================================================
HWND StrategyPopup_CreatePopup(HWND hParent, HWND hParentCtl)
{

    HWND hPopup = StrategyPopup.Create(hParent, L"", 0, 0, 0, 0,
        WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR |
        WS_EX_NOACTIVATE);

    // Get the currently selected Strategy from the StrategyButton and its associated
    // LongShort and PutCall status and use that data to set the associated line
    // in the popup.
    Strategy s = (Strategy)CustomLabel_GetUserDataInt(GetDlgItem(hParentCtl, IDC_STRATEGYBUTTON_STRATEGY));
    LongShort ls = (LongShort)CustomLabel_GetUserDataInt(GetDlgItem(hParentCtl, IDC_STRATEGYBUTTON_LONGSHORT));
    PutCall pc = (PutCall)CustomLabel_GetUserDataInt(GetDlgItem(hParentCtl, IDC_STRATEGYBUTTON_PUTCALL));

    std::wstring text;

    for (int i = 0; i < (int)Strategy::Count; ++i) {
        HWND hCtlStrategy = GetDlgItem(hPopup, IDC_STRATEGYPOPUP_STRATEGY + i);

        if ((Strategy)CustomLabel_GetUserDataInt(hCtlStrategy) == s) {
            HWND hCtlLongShort = GetDlgItem(hPopup, IDC_STRATEGYPOPUP_LONGSHORT + i);
            CustomLabel_SetUserDataInt(hCtlLongShort, (int)ls);
            text = AfxUpper(StrategyButton_GetLongShortEnumText(ls));
            CustomLabel_SetText(hCtlLongShort, text);
            StrategyButton_SetLongShortTextColor(hCtlLongShort);

            HWND hCtlPutCall = GetDlgItem(hPopup, IDC_STRATEGYPOPUP_PutCall + i);
            CustomLabel_SetUserDataInt(hCtlPutCall, (int)pc);
            text = AfxUpper(StrategyButton_GetPutCallEnumText(pc));
            if (!StrategyButton_StrategyAllowPutCall(hCtlStrategy)) {
                text = L"";
            }
            CustomLabel_SetText(hCtlPutCall, text);

            break;
        }
    }

    // Position the popup over the StrategyButton
    RECT rc; GetWindowRect(hParentCtl, &rc);
    SetWindowPos(hPopup, 0,
        rc.left, rc.top,
        AfxScaleX(233),
        AfxScaleY(24) * (int)Strategy::Count,    // 24 b/c 23 line height + 1 spacer
        SWP_NOZORDER | SWP_SHOWWINDOW | SWP_NOACTIVATE);

    // Set our hook and store the handle in the global variable
    hStrategyPopupMouseHook = SetWindowsHookEx(WH_MOUSE, StrategyPopupHook, 0, GetCurrentThreadId());

    return hPopup;
}


