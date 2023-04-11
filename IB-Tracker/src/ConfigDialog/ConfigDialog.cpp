
#include "pch.h"

#include "ConfigDialog.h"
#include "..\Themes\Themes.h"
#include "..\SuperLabel\SuperLabel.h"



HWND HWND_CONFIGDIALOG = NULL;




// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: ConfigDialog
// ========================================================================================
BOOL ConfigDialog_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    // Handle all of the painting in WM_PAINT
    return TRUE;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: ConfigDialog
// ========================================================================================
void ConfigDialog_OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    Graphics graphics(hdc);

    DWORD nBackColor = GetThemeColor(ThemeElement::TradesPanelBack);

    // Create the background brush
    SolidBrush backBrush(nBackColor);

    // Paint the background using brush.
    int nWidth = (ps.rcPaint.right - ps.rcPaint.left);
    int nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&backBrush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Process WM_SIZE message for window/dialog: ConfigDialog
// ========================================================================================
void ConfigDialog_OnSize(HWND hwnd, UINT state, int cx, int cy)
{

}


// ========================================================================================
// Process WM_CREATE message for window/dialog: HistoryPanel
// ========================================================================================
BOOL ConfigDialog_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{

    HWND_CONFIGDIALOG = hwnd;

    SuperLabel* pData = nullptr;

    HWND hCtl = CreateSuperLabel(
        hwnd,
        IDC_CONFIGDIALOG_DARKTHEME,
        SuperLabelType::TextOnly,
        0, 0, 0, 0);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = false;
        pData->BackColor = ThemeElement::MenuPanelBack;
        pData->TextColor = ThemeElement::MenuPanelText;
        pData->FontSize = 8;
        pData->TextAlignment = SuperLabelAlignment::MiddleLeft;
        pData->wszText = L"Dark Theme";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }


    return TRUE;
}


// ========================================================================================
// Process WM_COMMAND message for window/dialog: ConfigDialog
// ========================================================================================
void ConfigDialog_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
}



// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CConfigDialog::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(m_hwnd, WM_CREATE, ConfigDialog_OnCreate);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, ConfigDialog_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, ConfigDialog_OnPaint);
        HANDLE_MSG(m_hwnd, WM_SIZE, ConfigDialog_OnSize);
        HANDLE_MSG(m_hwnd, WM_COMMAND, ConfigDialog_OnCommand);

    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}

