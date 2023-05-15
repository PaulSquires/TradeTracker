#include "pch.h"
#include "..\Utilities\CWindowBase.h"
#include "..\CustomLabel\CustomLabel.h"

#include "StrategyButton.h"
#include "StrategyPopup.h"


HWND HWND_STRATEGYPOPUP = NULL;

extern CStrategyButton StrategyButton;
extern CStrategyPopup StrategyPopup;



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

    DWORD nBackColor = GetThemeColor(ThemeElement::GrayDark);

    // Create the background brush
    SolidBrush backBrush(nBackColor);

    // Paint the background using brush.
    int nWidth = (ps.rcPaint.right - ps.rcPaint.left);
    int nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&backBrush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

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

    std::wstring wszFontName = L"Segoe UI";
    std::wstring wszText;
    int FontSize = 8;

    for (int i = 0; i < (int)Strategy::Count; ++i) {

        hCtl = CustomLabel_SimpleLabel(hwnd, IDC_STRATEGYPOPUP_LONGSHORT + i, L"",
            ThemeElement::WhiteLight, ThemeElement::GrayMedium,
            CustomLabelAlignment::MiddleLeft, 0, nTop, 50, nHeight);
        CustomLabel_SetUserDataInt(hCtl, (int)LongShort::Short);
        CustomLabel_SetFont(hCtl, wszFontName, FontSize, true);
        CustomLabel_SetTextOffset(hCtl, 5, 0);
        wszText = AfxUpper(StrategyButton_GetLongShortEnumText(LongShort::Short));
        CustomLabel_SetText(hCtl, wszText);
        hCtlLongShort = hCtl;

        hCtl = CustomLabel_SimpleLabel(hwnd, IDC_STRATEGYPOPUP_PUTCALL + i, L"",
            ThemeElement::WhiteLight, ThemeElement::GrayMedium,
            CustomLabelAlignment::MiddleCenter, 51, nTop, 50, nHeight);
        CustomLabel_SetUserDataInt(hCtl, (int)PutCall::Put);
        CustomLabel_SetFont(hCtl, wszFontName, FontSize, true);
        wszText = AfxUpper(StrategyButton_GetPutCallEnumText(PutCall::Put));
        CustomLabel_SetText(hCtl, wszText);
        hCtlPutCall = hCtl;

        hCtl = CustomLabel_SimpleLabel(hwnd, IDC_STRATEGYPOPUP_STRATEGY + i, L"",
            ThemeElement::WhiteLight, ThemeElement::GrayMedium,
            CustomLabelAlignment::MiddleLeft, 102, nTop, 100, nHeight);
        CustomLabel_SetUserDataInt(hCtl, (int)i);
        CustomLabel_SetFont(hCtl, wszFontName, FontSize, true);
        CustomLabel_SetTextOffset(hCtl, 5, 0);
        wszText = AfxUpper(StrategyButton_GetStrategyEnumText((Strategy)i));
        CustomLabel_SetText(hCtl, wszText);
        if (!StrategyButton_StrategyAllowPutCall(hCtl)) {
            CustomLabel_SetText(hCtlPutCall, L"");
        }
        StrategyButton_SetLongShortTextColor(hCtlLongShort);

        hCtl = CustomLabel_ButtonLabel(hwnd, IDC_STRATEGYPOPUP_GO + i, L"GO",
            ThemeElement::Black, ThemeElement::Blue, ThemeElement::Blue, ThemeElement::GrayMedium,
            CustomLabelAlignment::MiddleCenter, 203, nTop, 30, nHeight);
        CustomLabel_SetFont(hCtl, wszFontName, FontSize, true);
        CustomLabel_SetTextColorHot(hCtl, ThemeElement::WhiteLight);

        nTop += nHeight + 1;

    }

    return TRUE;
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CStrategyPopup::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Prevent a recursive calling of the WM_NCACTIVATE message as DestroyWindow deactivates the window.
    static bool destroyed = false;

    switch (msg)
    {
        HANDLE_MSG(m_hwnd, WM_CREATE, StrategyPopup_OnCreate);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, StrategyPopup_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, StrategyPopup_OnPaint);


    case WM_DESTROY:
    {
        // Reset our destroyed variable for future use of the popup
        destroyed = false;
        return 0;
    }
    break;


    case WM_NCACTIVATE:
    {
        // Detect that we have clicked outside the popup StrategyPopup and will now close it.
        if (wParam == false) {
            // Set our static flag to prevent recursion
            if (destroyed == false) {
                destroyed = true;
                DestroyWindow(m_hwnd);
            }
            return TRUE;
        }
        return 0;
    }
    break;


    case MSG_CUSTOMLABEL_CLICK:
    {
        HWND hCtl = (HWND)lParam;
        int CtrlId = (int)wParam;

        if (hCtl == NULL) return 0;

        //if (CtrlId == IDC_STRATEGYBUTTON_LONGSHORT) {
        //    StrategyButton_ToggleLongShortText(hCtl);
        //}
        //if (CtrlId == IDC_STRATEGYBUTTON_PUTCALL) {
        //    StrategyButton_TogglePutCallText(hCtl);
        //}
        //if (CtrlId == IDC_STRATEGYBUTTON_STRATEGY) {
        //    StrategyButton_ToggleStrategyText(hCtl);
        //}
        //if (CtrlId == IDC_STRATEGYBUTTON_DROPDOWN) {
        //    //StrategyButton_ToggleStrategyText(hCtl);
        //}
        //if (CtrlId == IDC_STRATEGYBUTTON_GO) {
        //    StrategyButton_ToggleStrategyText(hCtl);
        //}
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
        WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);

    // Get the currently selected Strategy from the StrategyButton and its associated
    // LongShort and PutCall status and use that data to set the associated line
    // in the popup.
    Strategy s = (Strategy)CustomLabel_GetUserDataInt(GetDlgItem(hParentCtl, IDC_STRATEGYBUTTON_STRATEGY));
    LongShort ls = (LongShort)CustomLabel_GetUserDataInt(GetDlgItem(hParentCtl, IDC_STRATEGYBUTTON_LONGSHORT));
    PutCall pc = (PutCall)CustomLabel_GetUserDataInt(GetDlgItem(hParentCtl, IDC_STRATEGYBUTTON_PUTCALL));

    std::wstring wszText;

    for (int i = 0; i < (int)Strategy::Count; ++i) {
        HWND hCtlStrategy = GetDlgItem(hPopup, IDC_STRATEGYPOPUP_STRATEGY + i);

        if ((Strategy)CustomLabel_GetUserDataInt(hCtlStrategy) == s) {
            HWND hCtlLongShort = GetDlgItem(hPopup, IDC_STRATEGYPOPUP_LONGSHORT + i);
            CustomLabel_SetUserDataInt(hCtlLongShort, (int)ls);
            wszText = AfxUpper(StrategyButton_GetLongShortEnumText(ls));
            CustomLabel_SetText(hCtlLongShort, wszText);
            StrategyButton_SetLongShortTextColor(hCtlLongShort);

            HWND hCtlPutCall = GetDlgItem(hPopup, IDC_STRATEGYPOPUP_PUTCALL + i);
            CustomLabel_SetUserDataInt(hCtlPutCall, (int)pc);
            wszText = AfxUpper(StrategyButton_GetPutCallEnumText(pc));
            if (!StrategyButton_StrategyAllowPutCall(hCtlStrategy)) {
                wszText = L"";
            }
            CustomLabel_SetText(hCtlPutCall, wszText);

            break;
        }
    }

    // Position the popup over the StrategyButton
    RECT rc; GetWindowRect(hParentCtl, &rc);
    SetWindowPos(hPopup, HWND_TOP,
        rc.left, rc.top,
        AfxScaleX(233),
        AfxScaleY(24) * (int)Strategy::Count,    // 24 b/c 23 line height + 1 spacer
        SWP_SHOWWINDOW);


    return hPopup;

}


