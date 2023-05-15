#include "pch.h"
#include "..\Utilities\CWindowBase.h"
#include "..\CustomLabel\CustomLabel.h"

#include "StrategyButton.h"


HWND HWND_STRATEGYBUTTON = NULL;

HWND HWND_SHORTLONG = NULL;
HWND HWND_PUTCALL = NULL;
HWND HWND_STRATEGY = NULL;
HWND HWND_GO = NULL;
HWND HWND_DROPDOWN = NULL;

extern CStrategyButton StrategyButton;

void StrategyButton_OnSize(HWND hwnd, UINT state, int cx, int cy);



// ========================================================================================
// Get the text for the specified LongShort
// ========================================================================================
std::wstring StrategyButton_GetLongShortEnumText(LongShort ls)
{
    switch (ls)
    {
    case LongShort::Long:
        return L"Long";
    case LongShort::Short:
        return L"Short";
    default:
        return L"";
    }
}

// ========================================================================================
// Get the text for the specified PutCall
// ========================================================================================
std::wstring StrategyButton_GetPutCallEnumText(PutCall pc)
{
    switch (pc)
    {
    case PutCall::Put:
        return L"Put";
    case PutCall::Call:
        return L"Call";
    default:
        return L"";
    }
}

// ========================================================================================
// Get the text for the specified Strategy
// ========================================================================================
std::wstring StrategyButton_GetStrategyEnumText(Strategy s)
{
    switch (s)
    {
    case Strategy::Vertical:
        return L"Vertical";
    case Strategy::Strangle:
        return L"Strangle";
    case Strategy::Straddle:
        return L"Straddle";
    case Strategy::Option:
        return L"Option";
    case Strategy::IronCondor:
        return L"Iron Condor";
    case Strategy::Covered:
        return L"Covered";
    case Strategy::Butterfly:
        return L"Butterfly";
    case Strategy::RatioSpread:
        return L"Ratio Spread";
    default:
        return L"";
    }
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: StrategyButton
// ========================================================================================
BOOL StrategyButton_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    // Handle all of the painting in WM_PAINT
    return TRUE;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: StrategyButton
// ========================================================================================
void StrategyButton_OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    // Child controls cover the full client area. No need to paint.

/*
    Graphics graphics(hdc);

    DWORD nBackColor = GetThemeColor(ThemeElement::Black);

    // Create the background brush
    SolidBrush backBrush(nBackColor);

    // Paint the background using brush.
    int nWidth = (ps.rcPaint.right - ps.rcPaint.left);
    int nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&backBrush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);
*/

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Process WM_SIZE message for window/dialog: StrategyButton
// ========================================================================================
void StrategyButton_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
}



// ========================================================================================
// Process WM_CREATE message for window/dialog: StrategyButton
// ========================================================================================
BOOL StrategyButton_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    HWND_STRATEGYBUTTON = hwnd;

    int nHeight = 23;

    HWND hCtl = NULL;
    std::wstring wszFontName = L"Segoe UI";
    std::wstring wszText;
    int FontSize = 8;
    ThemeElement BorderColor = ThemeElement::Black;

    hCtl = CustomLabel_SimpleLabel(hwnd, IDC_STRATEGYBUTTON_SHORTLONG, L"",
        ThemeElement::WhiteLight, ThemeElement::GrayMedium,
        CustomLabelAlignment::MiddleLeft, 0, 0, 50, nHeight);
    HWND_SHORTLONG = hCtl;
    CustomLabel_SetUserDataInt(hCtl, (int)LongShort::Short);
    CustomLabel_SetFont(hCtl, wszFontName, FontSize, true);
    CustomLabel_SetBorder(hCtl, 1, BorderColor, BorderColor);
    CustomLabel_SetTextOffset(hCtl, 5, 0);
    StrategyButton_SetLongShortColor(hCtl);
    wszText = AfxUpper(StrategyButton_GetLongShortEnumText(LongShort::Short));
    CustomLabel_SetText(hCtl, wszText);

    hCtl = CustomLabel_SimpleLabel(hwnd, IDC_STRATEGYBUTTON_PUTCALL, L"",
        ThemeElement::WhiteLight, ThemeElement::GrayMedium,
        CustomLabelAlignment::MiddleCenter, 50, 0, 50, nHeight);
    HWND_PUTCALL = hCtl;
    CustomLabel_SetUserDataInt(hCtl, (int)PutCall::Put);
    CustomLabel_SetFont(hCtl, wszFontName, FontSize, true);
    CustomLabel_SetBorder(hCtl, 1, BorderColor, BorderColor);
    CustomLabel_SetText(hCtl, wszText);
    wszText = AfxUpper(StrategyButton_GetPutCallEnumText(PutCall::Put));
    CustomLabel_SetText(hCtl, wszText);

    hCtl = CustomLabel_SimpleLabel(hwnd, IDC_STRATEGYBUTTON_STRATEGY, L"",
        ThemeElement::WhiteLight, ThemeElement::GrayMedium,
        CustomLabelAlignment::MiddleLeft, 100, 0, 100, nHeight);
    HWND_STRATEGY = hCtl;
    CustomLabel_SetUserDataInt(hCtl, (int)Strategy::Vertical);
    CustomLabel_SetFont(hCtl, wszFontName, FontSize, true);
    CustomLabel_SetBorder(hCtl, 1, BorderColor, BorderColor);
    CustomLabel_SetTextOffset(hCtl, 5, 0);
    wszText = AfxUpper(StrategyButton_GetStrategyEnumText(Strategy::Vertical));
    CustomLabel_SetText(hCtl, wszText);

    hCtl = CustomLabel_SimpleLabel(hwnd, IDC_STRATEGYBUTTON_GO, L"GO",
        ThemeElement::WhiteLight, ThemeElement::GrayDark,
        CustomLabelAlignment::MiddleCenter, 200, 0, 30, nHeight);
    HWND_GO = hCtl;
    CustomLabel_SetFont(hCtl, wszFontName, FontSize, true);
    CustomLabel_SetBorder(hCtl, 1, BorderColor, BorderColor);

    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_STRATEGYBUTTON_DROPDOWN, L"\uE015",
        ThemeElement::WhiteMedium, ThemeElement::GrayDark, ThemeElement::GrayMedium, ThemeElement::GrayLight,
        CustomLabelAlignment::MiddleCenter, 230, 0, 30, nHeight);
    HWND_DROPDOWN = hCtl;
    CustomLabel_SetFont(hCtl, wszFontName, FontSize, true);
    CustomLabel_SetBorder(hCtl, 1, BorderColor, BorderColor);

    return TRUE;
}


// ========================================================================================
// Process WM_COMMAND message for window/dialog: StrategyButton
// ========================================================================================
void StrategyButton_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
}


// ========================================================================================
// Set the Short/Long background color.
// ========================================================================================
void StrategyButton_SetLongShortColor(HWND hCtl)
{
    LongShort ls = (LongShort)CustomLabel_GetUserDataInt(hCtl);

    if (ls == LongShort::Long) {
        CustomLabel_SetBackColor(hCtl, ThemeElement::Green);
    }
    else if (ls == LongShort::Short) {
        CustomLabel_SetBackColor(hCtl, ThemeElement::Red);
    }
}


// ========================================================================================
// Toggle the Short/Long text and ensure correct colors are set.
// ========================================================================================
void StrategyButton_ToggleLongShortText(HWND hCtl)
{
    int sel = CustomLabel_GetUserDataInt(hCtl) + 1;
    if (sel == (int)LongShort::Count) sel = 0;

    CustomLabel_SetUserDataInt(hCtl, sel);
    StrategyButton_SetLongShortColor(hCtl);
    std::wstring wszText = AfxUpper(StrategyButton_GetLongShortEnumText((LongShort)sel));
    CustomLabel_SetText(hCtl, wszText);
}

// ========================================================================================
// Check to see if the currently selected Strategy allows PutCall
// ========================================================================================
bool StrategyButton_StrategyAllowPutCall()
{
    HWND hCtl = GetDlgItem(HWND_STRATEGYBUTTON, IDC_STRATEGYBUTTON_STRATEGY);
    Strategy s = (Strategy)CustomLabel_GetUserDataInt(hCtl);

    switch (s)
    {
    case Strategy::Vertical:
    case Strategy::Option:
    case Strategy::Covered:
    case Strategy::Butterfly:
    case Strategy::RatioSpread:
        return true;
    case Strategy::Straddle:
    case Strategy::Strangle:
    case Strategy::IronCondor:
        return false;
    default:
        return true;
    }
}

// ========================================================================================
// Toggle the Put/Call text.
// ========================================================================================
void StrategyButton_TogglePutCallText(HWND hCtl)
{
    if (!StrategyButton_StrategyAllowPutCall()) {
        CustomLabel_SetText(hCtl, L"");
        return;
    }

    int sel = CustomLabel_GetUserDataInt(hCtl) + 1;
    if (sel == (int)PutCall::Count) sel = 0;

    CustomLabel_SetUserDataInt(hCtl, sel);
    std::wstring wszText = AfxUpper(StrategyButton_GetPutCallEnumText((PutCall)sel));
    CustomLabel_SetText(hCtl, wszText);
}

// ========================================================================================
// Toggle through the different Strategies.
// ========================================================================================
void StrategyButton_ToggleStrategyText(HWND hCtl)
{
    int sel = CustomLabel_GetUserDataInt(hCtl) + 1;
    if (sel == (int)Strategy::Count) sel = 0;

    CustomLabel_SetUserDataInt(hCtl, sel);
    std::wstring wszText = AfxUpper(StrategyButton_GetStrategyEnumText((Strategy)sel));
    CustomLabel_SetText(hCtl, wszText);

    wszText = L"";
    hCtl = GetDlgItem(HWND_STRATEGYBUTTON, IDC_STRATEGYBUTTON_PUTCALL);
    if (StrategyButton_StrategyAllowPutCall()) {
        sel = CustomLabel_GetUserDataInt(hCtl);
        wszText = AfxUpper(StrategyButton_GetPutCallEnumText((PutCall)sel));
    }
    CustomLabel_SetText(hCtl, wszText);

    //CustomLabel_SetBackColor(hCtl, ThemeElement::Green);
    //CustomLabel_SetBackColor(hCtl, ThemeElement::Red);

}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CStrategyButton::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(m_hwnd, WM_CREATE, StrategyButton_OnCreate);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, StrategyButton_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, StrategyButton_OnPaint);
        HANDLE_MSG(m_hwnd, WM_SIZE, StrategyButton_OnSize);
        HANDLE_MSG(m_hwnd, WM_COMMAND, StrategyButton_OnCommand);


    case MSG_CUSTOMLABEL_CLICK:
    {
        HWND hCtl = (HWND)lParam;
        int CtrlId = (int)wParam;

        if (hCtl == NULL) return 0;

        if (CtrlId == IDC_STRATEGYBUTTON_SHORTLONG) {
            StrategyButton_ToggleLongShortText(hCtl);
        }
        if (CtrlId == IDC_STRATEGYBUTTON_PUTCALL) {
            StrategyButton_TogglePutCallText(hCtl);
        }
        if (CtrlId == IDC_STRATEGYBUTTON_STRATEGY) {
            StrategyButton_ToggleStrategyText(hCtl);
        }
        return 0;
    }
    break;

    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}

