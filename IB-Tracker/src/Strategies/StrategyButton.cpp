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

    DWORD nBackColor = GetThemeColor(ThemeElement::MenuPanelBack);

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

    int nHeight = 24;

    HWND hCtl = NULL;
    std::wstring wszFontName = L"Segoe UI";
    int FontSize = 8;
    ThemeElement BorderColor = ThemeElement::MenuPanelText;

    hCtl = CustomLabel_SimpleLabel(hwnd, IDC_STRATEGYBUTTON_SHORTLONG, L"SHORT",
        ThemeElement::TradesPanelText, ThemeElement::TradesPanelBack,
        CustomLabelAlignment::MiddleLeft, 0, 0, 50, nHeight);
    HWND_SHORTLONG = hCtl;
    CustomLabel_SetFont(hCtl, wszFontName, FontSize);
    CustomLabel_SetBorder(hCtl, 1, BorderColor, BorderColor);
    CustomLabel_SetTextOffset(hCtl, 5, 0);

    hCtl = CustomLabel_SimpleLabel(hwnd, IDC_STRATEGYBUTTON_PUTCALL, L"PUT",
        ThemeElement::TradesPanelText, ThemeElement::TradesPanelBack,
        CustomLabelAlignment::MiddleCenter, 50, 0, 50, nHeight);
    HWND_PUTCALL = hCtl;
    CustomLabel_SetFont(hCtl, wszFontName, FontSize);
    CustomLabel_SetBorder(hCtl, 1, BorderColor, BorderColor);

    hCtl = CustomLabel_SimpleLabel(hwnd, IDC_STRATEGYBUTTON_STRATEGY, L"IRON CONDOR",
        ThemeElement::TradesPanelText, ThemeElement::TradesPanelBack,
        CustomLabelAlignment::MiddleLeft, 100, 0, 100, nHeight);
    HWND_STRATEGY = hCtl;
    CustomLabel_SetFont(hCtl, wszFontName, FontSize);
    CustomLabel_SetBorder(hCtl, 1, BorderColor, BorderColor);
    CustomLabel_SetTextOffset(hCtl, 5, 0);

    hCtl = CustomLabel_SimpleLabel(hwnd, IDC_STRATEGYBUTTON_GO, L"GO",
        ThemeElement::TradesPanelText, ThemeElement::TradesPanelBack,
        CustomLabelAlignment::MiddleCenter, 200, 0, 30, nHeight);
    HWND_GO = hCtl;
    CustomLabel_SetFont(hCtl, wszFontName, FontSize);
    CustomLabel_SetBorder(hCtl, 1, BorderColor, BorderColor);

    hCtl = CustomLabel_SimpleLabel(hwnd, IDC_STRATEGYBUTTON_DROPDOWN, L"V",
        ThemeElement::TradesPanelText, ThemeElement::TradesPanelBack,
        CustomLabelAlignment::MiddleCenter, 230, 0, 30, nHeight);
    HWND_DROPDOWN = hCtl;
    CustomLabel_SetFont(hCtl, wszFontName, FontSize);
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

    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}

