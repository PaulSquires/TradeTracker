
#include "pch.h"
#include "..\SuperLabel\SuperLabel.h"
#include "..\Utilities\UserMessages.h"
#include "..\Themes\Themes.h"
#include "..\MainWindow\tws-client.h"
#include "..\TradesPanel\TradesPanel.h"
#include "MenuPanel.h"


extern void TradesPanel_ShowActiveTrades();
extern void TradesPanel_ShowClosedTrades();
extern void HistoryPanel_ShowTickerTotals();
extern void HistoryPanel_ShowDailyTotals(const std::wstring& selectedDate);


HWND HWND_MENUPANEL = NULL;


// ========================================================================================
// Process WM_SIZE message for window/dialog: MenuPanel
// ========================================================================================
void MenuPanel_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    // Move the bottom separator and application name into place, but only do so
    // if the vertical height of NavBar window has not become less than the minimum
    // otherwise the two controls will bleed into the ones above them.
    int MinHeight = AfxScaleY(540);

    if (cy < MinHeight) return;

    SetWindowPos(GetDlgItem(hwnd, IDC_MENUPANEL_GEARICON), 0,
        AfxScaleX(12), cy - AfxScaleY(42), 0, 0, SWP_NOSIZE | SWP_NOZORDER);

    SetWindowPos(GetDlgItem(hwnd, IDC_MENUPANEL_MESSAGES), 0,
        AfxScaleX(35), cy - AfxScaleY(42), 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: MenuPanel
// ========================================================================================
BOOL MenuPanel_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    // Handle all of the painting in WM_PAINT
    return TRUE;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: MenuPanel
// ========================================================================================
void MenuPanel_OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    Graphics graphics(hdc);

    DWORD nBackColor = GetThemeColor(ThemeElement::MenuPanelBack);

    // Create the background brush
    SolidBrush backBrush(nBackColor);

    // Paint the background using brush.
    int nWidth = (ps.rcPaint.right - ps.rcPaint.left);
    int nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&backBrush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: MenuPanel
// ========================================================================================
BOOL MenuPanel_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    HWND_MENUPANEL = hwnd;

    int nTop, nLeft, nLeftOffset;
    int nItemHeight = 28;

    HWND hCtl;

    SuperLabel* pData = nullptr;

    // HEADER CONTROLS
    nLeft = (MENUPANEL_WIDTH - 68) / 2;
    hCtl = CreateSuperLabel(
        hwnd,
        IDC_MENUPANEL_LOGO,
        SuperLabelType::ImageOnly,
        nLeft, 20, 68, 68);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = false;
        pData->BackColor = ThemeElement::MenuPanelBack;
        pData->ImageWidth = 68;
        pData->ImageHeight = 68;
        pData->pImage = LoadImageFromResource(pData->hInst, MAKEINTRESOURCE(IDB_LOGO), L"PNG");
        pData->pImageHot = LoadImageFromResource(pData->hInst, MAKEINTRESOURCE(IDB_LOGO), L"PNG");
        SuperLabel_SetOptions(hCtl, pData);
    }


    hCtl = CreateSuperLabel(
        hwnd,
        IDC_MENUPANEL_USERNAME,
        SuperLabelType::TextOnly,
        0, 100, MENUPANEL_WIDTH, 18);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = false;
        pData->BackColor = ThemeElement::MenuPanelBack;
        pData->TextColor = ThemeElement::MenuPanelTextDim;
        pData->FontSize = 10;
        pData->TextAlignment = SuperLabelAlignment::MiddleCenter;
        pData->wszText = GetTraderName();
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }


    hCtl = CreateSuperLabel(
        hwnd,
        IDC_MENUPANEL_APPNAME,
        SuperLabelType::TextOnly,
        0, 118, MENUPANEL_WIDTH, 18);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = false;
        pData->BackColor = ThemeElement::MenuPanelBack;
        pData->TextColor = ThemeElement::MenuPanelText;
        pData->FontSize = 10;
        pData->TextAlignment = SuperLabelAlignment::MiddleCenter;
        pData->wszText = L"IB-Tracker v1.0";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }


    // SEPARATOR
    nTop = 150;
    hCtl = CreateSuperLabel(
        hwnd, -1,
        SuperLabelType::LineHorizontal,
        0, nTop, MENUPANEL_WIDTH, 10);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->BackColor = ThemeElement::MenuPanelBack;
        pData->LineColor = ThemeElement::MenuPanelSeparator;
        pData->LineWidth = 6;
        pData->MarginLeft = 10;
        pData->MarginRight = 10;
        SuperLabel_SetOptions(hCtl, pData);
    }


    // MENU ITEMS
    nLeftOffset = 0;
    nTop = nTop + 10;
    hCtl = CreateSuperLabel(
        hwnd,
        IDC_MENUPANEL_ACTIVETRADES,
        SuperLabelType::TextOnly,
        0, nTop, MENUPANEL_WIDTH, nItemHeight);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = true;
        pData->AllowSelect = true;
        pData->SelectorColor = ThemeElement::TradesPanelBack;   // selector should be same color as middle panel
        pData->BackColor = ThemeElement::MenuPanelBack;
        pData->BackColorHot = ThemeElement::MenuPanelBackHot;
        pData->BackColorSelected = ThemeElement::MenuPanelBackSelected;
        pData->TextColor = ThemeElement::MenuPanelText;
        pData->TextColorHot = ThemeElement::MenuPanelText;
        pData->TextOffsetLeft = nLeftOffset;
        pData->FontSize = 10;
        pData->FontSizeHot = 10;
        pData->wszText = L"Active Trades";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }


    nTop = nTop + nItemHeight;
    hCtl = CreateSuperLabel(
        hwnd,
        IDC_MENUPANEL_CLOSEDTRADES,
        SuperLabelType::TextOnly,
        0, nTop, MENUPANEL_WIDTH, nItemHeight);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = true;
        pData->AllowSelect = true;
        pData->SelectorColor = ThemeElement::TradesPanelBack;   // selector should be same color as middle panel
        pData->BackColor = ThemeElement::MenuPanelBack;
        pData->BackColorHot = ThemeElement::MenuPanelBackHot;
        pData->BackColorSelected = ThemeElement::MenuPanelBackSelected;
        pData->TextColor = ThemeElement::MenuPanelText;
        pData->TextColorHot = ThemeElement::MenuPanelText;
        pData->TextOffsetLeft = nLeftOffset;
        pData->FontSize = 10;
        pData->FontSizeHot = 10;
        pData->wszText = L"Closed Trades";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }


    // SEPARATOR
    nTop = nTop + nItemHeight + 6;
    hCtl = CreateSuperLabel(
        hwnd, -1,
        SuperLabelType::LineHorizontal,
        0, nTop, MENUPANEL_WIDTH, 10);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->BackColor = ThemeElement::MenuPanelBack;
        pData->LineColor = ThemeElement::MenuPanelSeparator;
        pData->LineWidth = 6;
        pData->MarginLeft = 10;
        pData->MarginRight = 10;
        SuperLabel_SetOptions(hCtl, pData);
    }


    nTop = nTop + 10;
    hCtl = CreateSuperLabel(
        hwnd,
        IDC_MENUPANEL_NEWTRADE,
        SuperLabelType::TextOnly,
        0, nTop, MENUPANEL_WIDTH, nItemHeight);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = true;
        pData->AllowSelect = true;
        pData->SelectorColor = ThemeElement::TradesPanelBack;   // selector should be same color as middle panel
        pData->BackColor = ThemeElement::MenuPanelBack;
        pData->BackColorHot = ThemeElement::MenuPanelBackHot;
        pData->BackColorSelected = ThemeElement::MenuPanelBackSelected;
        pData->TextColor = ThemeElement::MenuPanelText;
        pData->TextColorHot = ThemeElement::MenuPanelText;
        pData->TextOffsetLeft = nLeftOffset;
        pData->FontSize = 10;
        pData->FontSizeHot = 10;
        pData->wszText = L"New Trade";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }


    nTop = nTop + nItemHeight;
    hCtl = CreateSuperLabel(
        hwnd,
        IDC_MENUPANEL_SHORTSTRANGLE,
        SuperLabelType::TextOnly,
        0, nTop, MENUPANEL_WIDTH, nItemHeight);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = true;
        pData->AllowSelect = true;
        pData->SelectorColor = ThemeElement::TradesPanelBack;   // selector should be same color as middle panel
        pData->BackColor = ThemeElement::MenuPanelBack;
        pData->BackColorHot = ThemeElement::MenuPanelBackHot;
        pData->BackColorSelected = ThemeElement::MenuPanelBackSelected;
        pData->TextColor = ThemeElement::MenuPanelText;
        pData->TextColorHot = ThemeElement::MenuPanelText;
        pData->TextOffsetLeft = nLeftOffset;
        pData->FontSize = 10;
        pData->FontSizeHot = 10;
        pData->wszText = L"Short Strangle";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }


    nTop = nTop + nItemHeight;
    hCtl = CreateSuperLabel(
        hwnd,
        IDC_MENUPANEL_SHORTPUT,
        SuperLabelType::TextOnly,
        0, nTop, MENUPANEL_WIDTH, nItemHeight);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = true;
        pData->AllowSelect = true;
        pData->SelectorColor = ThemeElement::TradesPanelBack;   // selector should be same color as middle panel
        pData->BackColor = ThemeElement::MenuPanelBack;
        pData->BackColorHot = ThemeElement::MenuPanelBackHot;
        pData->BackColorSelected = ThemeElement::MenuPanelBackSelected;
        pData->TextColor = ThemeElement::MenuPanelText;
        pData->TextColorHot = ThemeElement::MenuPanelText;
        pData->TextOffsetLeft = nLeftOffset;
        pData->FontSize = 10;
        pData->FontSizeHot = 10;
        pData->wszText = L"Short Put";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }


    nTop = nTop + nItemHeight;
    hCtl = CreateSuperLabel(
        hwnd,
        IDC_MENUPANEL_SHORTCALL,
        SuperLabelType::TextOnly,
        0, nTop, MENUPANEL_WIDTH, nItemHeight);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = true;
        pData->AllowSelect = true;
        pData->SelectorColor = ThemeElement::TradesPanelBack;   // selector should be same color as middle panel
        pData->BackColor = ThemeElement::MenuPanelBack;
        pData->BackColorHot = ThemeElement::MenuPanelBackHot;
        pData->BackColorSelected = ThemeElement::MenuPanelBackSelected;
        pData->TextColor = ThemeElement::MenuPanelText;
        pData->TextColorHot = ThemeElement::MenuPanelText;
        pData->TextOffsetLeft = nLeftOffset;
        pData->FontSize = 10;
        pData->FontSizeHot = 10;
        pData->wszText = L"Short Call";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }


    // SEPARATOR
    nTop = nTop + nItemHeight + 6;
    hCtl = CreateSuperLabel(
        hwnd, -1,
        SuperLabelType::LineHorizontal,
        0, nTop, MENUPANEL_WIDTH, 10);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->BackColor = ThemeElement::MenuPanelBack;
        pData->LineColor = ThemeElement::MenuPanelSeparator;
        pData->LineWidth = 6;
        pData->MarginLeft = 10;
        pData->MarginRight = 10;
        SuperLabel_SetOptions(hCtl, pData);
    }


    nTop = nTop + 10;
    hCtl = CreateSuperLabel(
        hwnd,
        IDC_MENUPANEL_TICKERTOTALS,
        SuperLabelType::TextOnly,
        0, nTop, MENUPANEL_WIDTH, nItemHeight);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = true;
        pData->AllowSelect = true;
        pData->SelectorColor = ThemeElement::TradesPanelBack;   // selector should be same color as middle panel
        pData->BackColor = ThemeElement::MenuPanelBack;
        pData->BackColorHot = ThemeElement::MenuPanelBackHot;
        pData->BackColorSelected = ThemeElement::MenuPanelBackSelected;
        pData->TextColor = ThemeElement::MenuPanelText;
        pData->TextColorHot = ThemeElement::MenuPanelText;
        pData->TextOffsetLeft = nLeftOffset;
        pData->FontSize = 10;
        pData->FontSizeHot = 10;
        pData->wszText = L"Ticker Totals";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }


    nTop = nTop + nItemHeight;
    hCtl = CreateSuperLabel(
        hwnd,
        IDC_MENUPANEL_DAILYTOTALS,
        SuperLabelType::TextOnly,
        0, nTop, MENUPANEL_WIDTH, nItemHeight);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = true;
        pData->AllowSelect = true;
        pData->SelectorColor = ThemeElement::TradesPanelBack;   // selector should be same color as middle panel
        pData->BackColor = ThemeElement::MenuPanelBack;
        pData->BackColorHot = ThemeElement::MenuPanelBackHot;
        pData->BackColorSelected = ThemeElement::MenuPanelBackSelected;
        pData->TextColor = ThemeElement::MenuPanelText;
        pData->TextColorHot = ThemeElement::MenuPanelText;
        pData->TextOffsetLeft = nLeftOffset;
        pData->FontSize = 10;
        pData->FontSizeHot = 10;
        pData->wszText = L"Daily Totals";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }


    nTop = nTop + nItemHeight;
    hCtl = CreateSuperLabel(
        hwnd,
        IDC_MENUPANEL_RECONCILE,
        SuperLabelType::TextOnly,
        0, nTop, MENUPANEL_WIDTH, nItemHeight);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = true;
        pData->AllowSelect = true;
        pData->SelectorColor = ThemeElement::TradesPanelBack;
        pData->BackColor = ThemeElement::MenuPanelBack;
        pData->BackColorHot = ThemeElement::MenuPanelBackHot;
        pData->BackColorSelected = ThemeElement::MenuPanelBackSelected;
        pData->TextColor = ThemeElement::MenuPanelText;
        pData->TextColorHot = ThemeElement::MenuPanelText;
        pData->TextOffsetLeft = nLeftOffset;
        pData->FontSize = 10;
        pData->FontSizeHot = 10;
        pData->wszText = L"Reconcile";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }


    // SEPARATOR
    nTop = nTop + nItemHeight + 6;
    hCtl = CreateSuperLabel(
        hwnd, -1, 
        SuperLabelType::LineHorizontal,
        0, nTop, MENUPANEL_WIDTH, 10);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->BackColor = ThemeElement::MenuPanelBack;
        pData->LineColor = ThemeElement::MenuPanelSeparator;
        pData->LineWidth = 6;
        pData->MarginLeft = 10;
        pData->MarginRight = 10;
        SuperLabel_SetOptions(hCtl, pData);
    }


    nTop = nTop + 10;
    hCtl = CreateSuperLabel(
        hwnd,
        IDC_MENUPANEL_CONFIGURE,
        SuperLabelType::TextOnly,
        0, nTop, MENUPANEL_WIDTH, nItemHeight);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = true;
        pData->AllowSelect = true;
        pData->SelectorColor = ThemeElement::TradesPanelBack;
        pData->BackColor = ThemeElement::MenuPanelBack;
        pData->BackColorHot = ThemeElement::MenuPanelBackHot;
        pData->BackColorSelected = ThemeElement::MenuPanelBackSelected;
        pData->TextColor = ThemeElement::MenuPanelText;
        pData->TextColorHot = ThemeElement::MenuPanelText;
        pData->TextOffsetLeft = nLeftOffset;
        pData->FontSize = 10;
        pData->FontSizeHot = 10;
        pData->wszText = L"Configuration";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }


    hCtl = CreateSuperLabel(
        hwnd,
        IDC_MENUPANEL_GEARICON,
        SuperLabelType::ImageAndText,  
        0, 0, MENUPANEL_WIDTH, 26);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = true;
        pData->BackColor = ThemeElement::MenuPanelBack;
        pData->BackColorHot = ThemeElement::MenuPanelBack;
        pData->TextColor = ThemeElement::MenuPanelTextDim;
        pData->TextColorHot = ThemeElement::MenuPanelTextDim;
        pData->FontSize = 9;
        pData->FontSizeHot = pData->FontSize;
        pData->TextAlignment = SuperLabelAlignment::MiddleLeft;
        pData->wszText = L"TWS not connected";
        pData->wszTextHot = pData->wszText;
        pData->wszToolTip = L" Connect to TWS ";
        pData->TextOffsetLeft = 4;
        pData->ImageWidth = 20;
        pData->ImageHeight = 20;
        pData->ImageOffsetLeft = 10;
        pData->pImage = LoadImageFromResource(pData->hInst, MAKEINTRESOURCE(IDB_GEAR), L"PNG");
        pData->pImageHot = LoadImageFromResource(pData->hInst, MAKEINTRESOURCE(IDB_GEARHOT), L"PNG");
        SuperLabel_SetOptions(hCtl, pData);
    }

    return TRUE;
}



// ========================================================================================
// Select the specified menu item (and deselect any other menu items)
// ========================================================================================
void MenuPanel_SelectMenuItem(HWND hParent, int CtrlId)
{
    HWND hCtrl = NULL;
    for (int ctrlId = IDC_MENUPANEL_ACTIVETRADES; ctrlId <= IDC_MENUPANEL_CONFIGURE; ctrlId++)
    {
        hCtrl = GetDlgItem(hParent, ctrlId);
        SuperLabel* pData = SuperLabel_GetOptions(hCtrl);
        if (pData != nullptr) {
            pData->IsSelected = (pData->CtrlId == CtrlId) ? true : false;
            SuperLabel_SetOptions(hCtrl, pData);
            AfxRedrawWindow(hCtrl);
        }
    }
}


// ========================================================================================
// Gets the ID of the currently active menu item.
// ========================================================================================
int MenuPanel_GetActiveMenuItem(HWND hParent)
{
    HWND hCtrl = NULL;
    for (int ctrlId = IDC_MENUPANEL_ACTIVETRADES; ctrlId <= IDC_MENUPANEL_CONFIGURE; ctrlId++)
    {
        hCtrl = GetDlgItem(hParent, ctrlId);
        SuperLabel* pData = SuperLabel_GetOptions(hCtrl);
        if (pData != nullptr) {
            if (pData->IsSelected) return ctrlId;
        }
    }
    return 0;
}


// ========================================================================================
// MenuPanel Window procedure
// ========================================================================================
LRESULT CMenuPanel::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(m_hwnd, WM_CREATE, MenuPanel_OnCreate);
        HANDLE_MSG(m_hwnd, WM_SIZE, MenuPanel_OnSize);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, MenuPanel_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, MenuPanel_OnPaint);


    case MSG_TWS_CONNECT_START:
    {
        SuperLabel* pData = nullptr;
        pData = SuperLabel_GetOptions(GetDlgItem(m_hwnd, IDC_MENUPANEL_GEARICON));
        if (pData) {
            pData->wszText = L"Connecting to TWS";
            AfxRedrawWindow(GetDlgItem(m_hwnd, IDC_MENUPANEL_GEARICON));
        }
        break;
    }


    case MSG_TWS_CONNECT_SUCCESS:
    {
        SuperLabel* pData = nullptr;
        pData = SuperLabel_GetOptions(GetDlgItem(m_hwnd, IDC_MENUPANEL_GEARICON));
        if (pData) {
            pData->wszText = L"TWS Connected";
            AfxRedrawWindow(GetDlgItem(m_hwnd, IDC_MENUPANEL_GEARICON));
        }
        break;
    }


    case MSG_TWS_CONNECT_FAILURE:
    {
        SuperLabel* pData = nullptr;
        pData = SuperLabel_GetOptions(GetDlgItem(m_hwnd, IDC_MENUPANEL_GEARICON));
        if (pData) {
            pData->wszText = L"TWS connection failed";
            AfxRedrawWindow(GetDlgItem(m_hwnd, IDC_MENUPANEL_GEARICON));
        }
        break;
    }


    case MSG_TWS_CONNECT_DISCONNECT:
    {
        SuperLabel* pData = nullptr;
        pData = SuperLabel_GetOptions(GetDlgItem(m_hwnd, IDC_MENUPANEL_GEARICON));
        if (pData) {
            pData->wszText = L"TWS Disonnected";
            AfxRedrawWindow(GetDlgItem(m_hwnd, IDC_MENUPANEL_GEARICON));
        }
        break;
    }


    case MSG_SUPERLABEL_MOUSEMOVE:
        break;


    case MSG_SUPERLABEL_CLICK:
    {
        HWND hCtl = (HWND)lParam;
        int CtrlId = (int)wParam;

        if (hCtl == NULL) return 0;
        SuperLabel* pData = (SuperLabel*)GetWindowLongPtr(hCtl, 0);

        if (pData) {

            switch (CtrlId) {

            case IDC_MENUPANEL_MESSAGES:
            case IDC_MENUPANEL_GEARICON:
            {
                // If already connected then don't try to connect again
                if (tws_isConnected()) break;

                    // Prevent multiple clicks of the connect button by waiting until
                // the first click is finished.
                static bool bProcessingConnectClick = false;
                if (bProcessingConnectClick) break;
                bProcessingConnectClick = true;
                bool res = tws_connect();
                printf("Connect: %ld\n", res);
                //printf("isConnected: %ld\n", tws_isConnected());
                // If we connect to TWS successfully then we need to start showing
                // the price data for any active trades displaying in our table.
                if (tws_isConnected()) {
                    TradesPanel_ShowActiveTrades();
                }
                bProcessingConnectClick = false;
                break;
            }

            case IDC_MENUPANEL_ACTIVETRADES:
            {
                MenuPanel_SelectMenuItem(m_hwnd, CtrlId);
                TradesPanel_ShowActiveTrades();
                break;
            }

            case IDC_MENUPANEL_CLOSEDTRADES:
            {
                MenuPanel_SelectMenuItem(m_hwnd, CtrlId);
                TradesPanel_ShowClosedTrades();
                break;
            }

            case IDC_MENUPANEL_NEWTRADE:
            {
                MenuPanel_SelectMenuItem(m_hwnd, CtrlId);
                break;
            }

            case IDC_MENUPANEL_SHORTSTRANGLE:
            {
                MenuPanel_SelectMenuItem(m_hwnd, CtrlId);
                break;
            }

            case IDC_MENUPANEL_SHORTPUT:
            {
                MenuPanel_SelectMenuItem(m_hwnd, CtrlId);
                break;
            }

            case IDC_MENUPANEL_SHORTCALL:
            {
                MenuPanel_SelectMenuItem(m_hwnd, CtrlId);
                break;
            }

            case IDC_MENUPANEL_TICKERTOTALS:
            {
                MenuPanel_SelectMenuItem(m_hwnd, CtrlId);
                HistoryPanel_ShowTickerTotals();
                break;
            }

            case IDC_MENUPANEL_DAILYTOTALS:
            {
                MenuPanel_SelectMenuItem(m_hwnd, CtrlId);
                HistoryPanel_ShowDailyTotals(L"");
                break;
            }

            case IDC_MENUPANEL_RECONCILE:
            {
                MenuPanel_SelectMenuItem(m_hwnd, CtrlId);
                break;
            }

            case IDC_MENUPANEL_CONFIGURE:
            {
                MenuPanel_SelectMenuItem(m_hwnd, CtrlId);
                break;
            }

            }  // switch

        }   // if
    }  // case

    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);

    }   // end of switch statement

    return 0;

}
