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
#include "..\CustomLabel\CustomLabel.h"
#include "..\Utilities\UserMessages.h"
#include "..\Config\Config.h"
#include "..\MainWindow\tws-client.h"
#include "..\MainWindow\MainWindow.h"
#include "..\TradesPanel\TradesPanel.h"
#include "..\TradeDialog\TradeDialog.h"
#include "..\Utilities\ListBoxData.h"
#include "MenuPanel.h"

extern void TradesPanel_ShowActiveTrades();
extern void TradesPanel_ShowClosedTrades();
extern void TickerPanel_ShowTickerTotals();
extern void TransPanel_ShowTransactions();
extern void DailyPanel_ShowDailyTotals(const ListBoxData* ld);

HWND HWND_MENUPANEL = NULL;

extern HWND HWND_HISTORYPANEL;
extern HWND HWND_DAILYPANEL;
extern HWND HWND_TICKERPANEL;


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

    DWORD nBackColor = GetThemeColor(ThemeElement::Black);

    // Create the background brush
    SolidBrush backBrush(nBackColor);

    // Paint the background using brush.
    int nWidth = (ps.rcPaint.right - ps.rcPaint.left);
    int nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&backBrush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Generic helper function to create a menu separator.
// ========================================================================================
void MenuPanel_MakeSeparator(HWND hwnd, int nTop)
{
    CustomLabel* pData = nullptr;
    HWND hCtl = CreateCustomLabel(
        hwnd, -1,
        CustomLabelType::LineHorizontal,
        0, nTop, MENUPANEL_WIDTH, 10);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->BackColor = ThemeElement::Black;
        pData->LineColor = ThemeElement::Separator;
        pData->LineWidth = 2;
        pData->MarginLeft = 10;
        pData->MarginRight = 10;
        CustomLabel_SetOptions(hCtl, pData);
    }
}


// ========================================================================================
// Generic helper function to create a menu item.
// ========================================================================================
void MenuPanel_MakeMenuItem(HWND hwnd, int CtrlId, int nTop, const std::wstring wszText)
{
    CustomLabel* pData = nullptr;
    HWND hCtl = CreateCustomLabel(
        hwnd, CtrlId,
        CustomLabelType::TextOnly,
        0, nTop, MENUPANEL_WIDTH, 28);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = true;
        pData->AllowSelect = true;
        pData->AllowNotch = true;
        pData->SelectorColor = ThemeElement::GrayDark;   // MenuNotch should be same color as middle panel
        pData->BackColor = ThemeElement::Black;
        pData->BackColorHot = ThemeElement::Selection;
        pData->BackColorSelected = ThemeElement::Selection;
        pData->TextColor = ThemeElement::WhiteLight;
        pData->TextColorHot = ThemeElement::WhiteLight;
        pData->FontSize = 10;
        pData->FontSizeHot = 10;
        pData->wszText = wszText;
        pData->wszTextHot = pData->wszText;
        CustomLabel_SetOptions(hCtl, pData);
    }
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: MenuPanel
// ========================================================================================
BOOL MenuPanel_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    HWND_MENUPANEL = hwnd;

    int nTop = 0;
    int nLeft = 0;
    int nItemHeight = 28;

    HWND hCtl;

    CustomLabel* pData = nullptr;

    // HEADER CONTROLS
    nLeft = (MENUPANEL_WIDTH - 68) / 2;
    hCtl = CreateCustomLabel(
        hwnd,
        IDC_MENUPANEL_LOGO,
        CustomLabelType::ImageOnly,
        nLeft, 20, 68, 68);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = false;
        pData->BackColor = ThemeElement::Black;
        pData->ImageWidth = 68;
        pData->ImageHeight = 68;
        pData->pImage = LoadImageFromResource(pData->hInst, MAKEINTRESOURCE(IDB_LOGO), L"PNG");
        pData->pImageHot = LoadImageFromResource(pData->hInst, MAKEINTRESOURCE(IDB_LOGO), L"PNG");
        CustomLabel_SetOptions(hCtl, pData);
    }


    hCtl = CreateCustomLabel(
        hwnd,
        IDC_MENUPANEL_TRADERNAME,
        CustomLabelType::TextOnly,
        0, 100, MENUPANEL_WIDTH, 18);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = false;
        pData->BackColor = ThemeElement::Black;
        pData->TextColor = ThemeElement::WhiteMedium;
        pData->FontSize = 10;
        pData->TextAlignment = CustomLabelAlignment::MiddleCenter;
        pData->wszText = GetTraderName();
        pData->wszTextHot = pData->wszText;
        CustomLabel_SetOptions(hCtl, pData);
    }


    hCtl = CreateCustomLabel(
        hwnd,
        IDC_MENUPANEL_APPNAME,
        CustomLabelType::TextOnly,
        0, 118, MENUPANEL_WIDTH, 18);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = false;
        pData->BackColor = ThemeElement::Black;
        pData->TextColor = ThemeElement::WhiteLight;
        pData->FontSize = 10;
        pData->TextAlignment = CustomLabelAlignment::MiddleCenter;
        pData->wszText = L"IB-Tracker v1.0";
        pData->wszTextHot = pData->wszText;
        CustomLabel_SetOptions(hCtl, pData);
    }


    // SEPARATOR & MENU ITEMS
    nTop = 150;
    MenuPanel_MakeSeparator(hwnd, nTop);

    nTop += 10;
    MenuPanel_MakeMenuItem(hwnd, IDC_MENUPANEL_ACTIVETRADES, nTop, L"Active Trades");

    nTop += nItemHeight;
    MenuPanel_MakeMenuItem(hwnd, IDC_MENUPANEL_CLOSEDTRADES, nTop, L"Closed Trades");

    nTop += nItemHeight + 6;
    MenuPanel_MakeSeparator(hwnd, nTop);

    nTop += 10;
    MenuPanel_MakeMenuItem(hwnd, IDC_MENUPANEL_NEWOPTIONSTRADE, nTop, L"Options Trade");

    nTop += nItemHeight;
    MenuPanel_MakeMenuItem(hwnd, IDC_MENUPANEL_NEWSHARESTRADE, nTop, L"Shares Trade");

    nTop += nItemHeight;
    MenuPanel_MakeMenuItem(hwnd, IDC_MENUPANEL_NEWFUTURESTRADE, nTop, L"Futures Trade");

    nTop += nItemHeight + 6;
    MenuPanel_MakeSeparator(hwnd, nTop);

    nTop += 10;
    MenuPanel_MakeMenuItem(hwnd, IDC_MENUPANEL_NEWIRONCONDOR, nTop, L"Iron Condor");

    nTop += nItemHeight;
    MenuPanel_MakeMenuItem(hwnd, IDC_MENUPANEL_NEWSHORTSTRANGLE, nTop, L"Short Strangle");

    nTop += nItemHeight;
    MenuPanel_MakeMenuItem(hwnd, IDC_MENUPANEL_NEWSHORTPUT, nTop, L"Short Put");

    nTop += nItemHeight;
    MenuPanel_MakeMenuItem(hwnd, IDC_MENUPANEL_NEWSHORTCALL, nTop, L"Short Call");

    nTop += nItemHeight + 6;
    MenuPanel_MakeSeparator(hwnd, nTop);

    nTop += 10;
    MenuPanel_MakeMenuItem(hwnd, IDC_MENUPANEL_TICKERTOTALS, nTop, L"Ticker Totals");

    nTop += nItemHeight;
    MenuPanel_MakeMenuItem(hwnd, IDC_MENUPANEL_DAILYTOTALS, nTop, L"Daily Totals");

    nTop += nItemHeight;
    MenuPanel_MakeMenuItem(hwnd, IDC_MENUPANEL_TRANSACTIONS, nTop, L"Transactions");

    nTop += nItemHeight;
    MenuPanel_MakeMenuItem(hwnd, IDC_MENUPANEL_RECONCILE, nTop, L"Reconcile");

    nTop += nItemHeight + 6;
    MenuPanel_MakeSeparator(hwnd, nTop);

    nTop += 10;
    MenuPanel_MakeMenuItem(hwnd, IDC_MENUPANEL_CONNECTTWS, nTop, L"Connect to TWS");

    nTop += nItemHeight + 6;
    MenuPanel_MakeSeparator(hwnd, nTop);

    return TRUE;
}


// ========================================================================================
// Select the specified menu item (and deselect any other menu items)
// ========================================================================================
void MenuPanel_SelectMenuItem(HWND hParent, int CtrlId)
{
    HWND hCtrl = NULL;
    for (int i = IDC_MENUPANEL_FIRSTITEM; i <= IDC_MENUPANEL_LASTITEM; i++) {
        hCtrl = GetDlgItem(hParent, i);
        CustomLabel_Select(hCtrl, false);
    }

    hCtrl = GetDlgItem(hParent, CtrlId);
    CustomLabel_Select(hCtrl, true);
}


// ========================================================================================
// Gets the ID of the currently active menu item.
// ========================================================================================
int MenuPanel_GetActiveMenuItem(HWND hParent)
{
    HWND hCtrl = NULL;
    for (int ctrlId = IDC_MENUPANEL_FIRSTITEM; ctrlId <= IDC_MENUPANEL_LASTITEM; ctrlId++)
    {
        hCtrl = GetDlgItem(hParent, ctrlId);
        CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
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
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, MenuPanel_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, MenuPanel_OnPaint);


    case MSG_TWS_CONNECT_START:
    {
        CustomLabel* pData = nullptr;
        pData = CustomLabel_GetOptions(GetDlgItem(m_hwnd, IDC_MENUPANEL_CONNECTTWS));
        if (pData) {
            pData->wszText = L"Connecting to TWS";
            AfxRedrawWindow(GetDlgItem(m_hwnd, IDC_MENUPANEL_CONNECTTWS));
        }
        return 0;
        break;
    }


    case MSG_TWS_CONNECT_SUCCESS:
    {
        CustomLabel* pData = nullptr;
        pData = CustomLabel_GetOptions(GetDlgItem(m_hwnd, IDC_MENUPANEL_CONNECTTWS));
        if (pData) {
            pData->wszText = L"TWS Connected";
            pData->TextColor = ThemeElement::Green;
            pData->TextColorHot = ThemeElement::Green;
            AfxRedrawWindow(GetDlgItem(m_hwnd, IDC_MENUPANEL_CONNECTTWS));
        }
        return 0;
        break;
    }


    case MSG_TWS_CONNECT_FAILURE:
    case MSG_TWS_CONNECT_DISCONNECT:
    {
        CustomLabel* pData = nullptr;
        pData = CustomLabel_GetOptions(GetDlgItem(m_hwnd, IDC_MENUPANEL_CONNECTTWS));
        if (pData) {
            pData->wszText = L"Connect to TWS";
            pData->TextColor = ThemeElement::WhiteLight;
            pData->TextColorHot = ThemeElement::WhiteLight;
            AfxRedrawWindow(GetDlgItem(m_hwnd, IDC_MENUPANEL_CONNECTTWS));
        }
        EndMonitorThread();
        return 0;
        break;
    }


    case MSG_CUSTOMLABEL_CLICK:
    {
        HWND hCtl = (HWND)lParam;
        int CtrlId = (int)wParam;

        if (hCtl == NULL) return 0;
        CustomLabel* pData = (CustomLabel*)GetWindowLongPtr(hCtl, 0);

        if (pData) {

            switch (CtrlId) {

            case IDC_MENUPANEL_NEWOPTIONSTRADE:
            {
                int currSelection = MenuPanel_GetActiveMenuItem(m_hwnd);
                MenuPanel_SelectMenuItem(m_hwnd, CtrlId);
                if (TradeDialog_Show(TradeAction::NewOptionsTrade) == DIALOG_RETURN_CANCEL) {
                    MenuPanel_SelectMenuItem(m_hwnd, currSelection);
                }
            }
            break;

            case IDC_MENUPANEL_NEWSHARESTRADE:
            {
                int currSelection = MenuPanel_GetActiveMenuItem(m_hwnd);
                MenuPanel_SelectMenuItem(m_hwnd, CtrlId);
                if (TradeDialog_Show(TradeAction::NewSharesTrade) == DIALOG_RETURN_CANCEL) {
                    MenuPanel_SelectMenuItem(m_hwnd, currSelection);
                }
            }
            break;

            case IDC_MENUPANEL_NEWFUTURESTRADE:
            {
                int currSelection = MenuPanel_GetActiveMenuItem(m_hwnd);
                MenuPanel_SelectMenuItem(m_hwnd, CtrlId);
                if (TradeDialog_Show(TradeAction::NewFuturesTrade) == DIALOG_RETURN_CANCEL) {
                    MenuPanel_SelectMenuItem(m_hwnd, currSelection);
                }
            }
            break;

            case IDC_MENUPANEL_NEWIRONCONDOR:
            {
                int currSelection = MenuPanel_GetActiveMenuItem(m_hwnd);
                MenuPanel_SelectMenuItem(m_hwnd, CtrlId);
                if (TradeDialog_Show(TradeAction::NewIronCondor) == DIALOG_RETURN_CANCEL) {
                    MenuPanel_SelectMenuItem(m_hwnd, currSelection);
                }
            }
            break;

            case IDC_MENUPANEL_NEWSHORTSTRANGLE:
            {
                int currSelection = MenuPanel_GetActiveMenuItem(m_hwnd);
                MenuPanel_SelectMenuItem(m_hwnd, CtrlId);
                if (TradeDialog_Show(TradeAction::NewShortStrangle) == DIALOG_RETURN_CANCEL) {
                    MenuPanel_SelectMenuItem(m_hwnd, currSelection);
                }
            }
            break;

            case IDC_MENUPANEL_NEWSHORTPUT:
            {
                int currSelection = MenuPanel_GetActiveMenuItem(m_hwnd);
                MenuPanel_SelectMenuItem(m_hwnd, CtrlId);
                if (TradeDialog_Show(TradeAction::NewShortPut) == DIALOG_RETURN_CANCEL) {
                    MenuPanel_SelectMenuItem(m_hwnd, currSelection);
                }
            }
            break;

            case IDC_MENUPANEL_NEWSHORTCALL:
            {
                int currSelection = MenuPanel_GetActiveMenuItem(m_hwnd);
                MenuPanel_SelectMenuItem(m_hwnd, CtrlId);
                if (TradeDialog_Show(TradeAction::NewShortCall) == DIALOG_RETURN_CANCEL) {
                    MenuPanel_SelectMenuItem(m_hwnd, currSelection);
                }
            }
            break;

            case IDC_MENUPANEL_CONNECTTWS:
            {
                // If already connected then don't try to connect again
                if (tws_isConnected()) break;

                // Prevent multiple clicks of the connect button by waiting until
                // the first click is finished.
                static bool bProcessingConnectClick = false;
                if (bProcessingConnectClick) break;
                bProcessingConnectClick = true;
                bool res = tws_connect();

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

            case IDC_MENUPANEL_TICKERTOTALS:
            {
                MenuPanel_SelectMenuItem(m_hwnd, CtrlId);
                TickerPanel_ShowTickerTotals();
                break;
            }

            case IDC_MENUPANEL_DAILYTOTALS:
            {
                MenuPanel_SelectMenuItem(m_hwnd, CtrlId);
                DailyPanel_ShowDailyTotals(nullptr);
                break;
            }

            case IDC_MENUPANEL_TRANSACTIONS:
            {
                MenuPanel_SelectMenuItem(m_hwnd, CtrlId);
                TransPanel_ShowTransactions();
                break;

                //int currSelection = MenuPanel_GetActiveMenuItem(m_hwnd);
                //MenuPanel_SelectMenuItem(m_hwnd, CtrlId);
                //MessageBox(
                //    m_hwnd,
                //    (LPCWSTR)(L"Transactions features not implemented yet."),
                //    (LPCWSTR)L"Information",
                //    MB_ICONINFORMATION
                //);
                //CustomLabel_Select(GetDlgItem(m_hwnd, CtrlId), false);
                //MenuPanel_SelectMenuItem(m_hwnd, currSelection);
                //break;
            }

            case IDC_MENUPANEL_RECONCILE:
            {
                int currSelection = MenuPanel_GetActiveMenuItem(m_hwnd);
                MenuPanel_SelectMenuItem(m_hwnd, CtrlId);
                tws_performReconciliation();
                MenuPanel_SelectMenuItem(m_hwnd, currSelection);
                break;
            }


            }  // switch

            return 0;
        }   // if
    }  // case

    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);

    }   // end of switch statement

    return 0;

}
