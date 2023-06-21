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
#include "CustomLabel/CustomLabel.h"
#include "Utilities/UserMessages.h"
#include "Config/Config.h"
#include "MainWindow/tws-client.h"
#include "MainWindow/MainWindow.h"
#include "ActiveTrades/ActiveTrades.h"
#include "ClosedTrades/ClosedTrades.h"
#include "TradeDialog/TradeDialog.h"
#include "Utilities/ListBoxData.h"
#include "SideMenu.h"

extern void ActiveTrades_ShowActiveTrades();
extern void TransPanel_ShowTransactions();
extern void ClosedTrades_ShowClosedTrades();
extern void TickerPanel_ShowTickerTotals();
extern void DailyTotals_ShowDailyTotals(const ListBoxData* ld);

HWND HWND_SIDEMENU = NULL;

extern HWND HWND_ACTIVETRADES;
extern HWND HWND_TRADEHISTORY;
extern HWND HWND_DAILYTOTALS;
extern HWND HWND_TICKERPANEL;


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: SideMenu
// ========================================================================================
BOOL SideMenu_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    // Handle all of the painting in WM_PAINT
    return TRUE;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: SideMenu
// ========================================================================================
void SideMenu_OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    Graphics graphics(hdc);

    // Create the background brush
    SolidBrush backBrush(COLOR_BLACK);

    // Paint the background using brush.
    int nWidth = (ps.rcPaint.right - ps.rcPaint.left);
    int nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&backBrush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Generic helper function to create a menu separator.
// ========================================================================================
void SideMenu_MakeSeparator(HWND hwnd, int nTop)
{
    CustomLabel* pData = nullptr;
    HWND hCtl = CreateCustomLabel(
        hwnd, -1,
        CustomLabelType::LineHorizontal,
        0, nTop, SideMenu_WIDTH, 10);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->BackColor = COLOR_BLACK;
        pData->LineColor = COLOR_SEPARATOR;
        pData->LineWidth = 2;
        pData->MarginLeft = 10;
        pData->MarginRight = 10;
        CustomLabel_SetOptions(hCtl, pData);
    }
}


// ========================================================================================
// Generic helper function to create a menu item.
// ========================================================================================
void SideMenu_MakeMenuItem(HWND hwnd, int CtrlId, int nTop, const std::wstring wszText)
{
    CustomLabel* pData = nullptr;
    HWND hCtl = CreateCustomLabel(
        hwnd, CtrlId,
        CustomLabelType::TextOnly,
        0, nTop, SideMenu_WIDTH, 28);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = true;
        pData->AllowSelect = true;
        pData->AllowNotch = true;
        pData->SelectorColor = COLOR_GRAYDARK;   // MenuNotch should be same color as middle panel
        pData->BackColor = COLOR_BLACK;
        pData->BackColorHot = COLOR_SELECTION;
        pData->BackColorSelected = COLOR_SELECTION;
        pData->BackColorButtonDown = COLOR_SELECTION;
        pData->TextColor = COLOR_WHITELIGHT;
        pData->TextColorHot = COLOR_WHITELIGHT;
        pData->FontSize = 10;
        pData->FontSizeHot = 10;
        pData->wszText = wszText;
        pData->wszTextHot = pData->wszText;
        CustomLabel_SetOptions(hCtl, pData);
    }
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: SideMenu
// ========================================================================================
BOOL SideMenu_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    HWND_SIDEMENU = hwnd;

    int nTop = 0;
    int nLeft = 0;
    int nItemHeight = 28;

    HWND hCtl;

    CustomLabel* pData = nullptr;

    // HEADER CONTROLS
    nLeft = (SideMenu_WIDTH - 68) / 2;
    hCtl = CreateCustomLabel(
        hwnd,
        IDC_SIDEMENU_LOGO,
        CustomLabelType::ImageOnly,
        nLeft, 20, 68, 68);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = false;
        pData->BackColor = COLOR_BLACK;
        pData->ImageWidth = 68;
        pData->ImageHeight = 68;
        pData->pImage = LoadImageFromResource(pData->hInst, MAKEINTRESOURCE(IDB_LOGO), L"PNG");
        pData->pImageHot = LoadImageFromResource(pData->hInst, MAKEINTRESOURCE(IDB_LOGO), L"PNG");
        CustomLabel_SetOptions(hCtl, pData);
    }


    hCtl = CreateCustomLabel(
        hwnd,
        IDC_SIDEMENU_TRADERNAME,
        CustomLabelType::TextOnly,
        0, 100, SideMenu_WIDTH, 18);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = false;
        pData->BackColor = COLOR_BLACK;
        pData->TextColor = COLOR_WHITEMEDIUM;
        pData->FontSize = 10;
        pData->TextAlignment = CustomLabelAlignment::MiddleCenter;
        pData->wszText = GetTraderName();
        pData->wszTextHot = pData->wszText;
        CustomLabel_SetOptions(hCtl, pData);
    }


    hCtl = CreateCustomLabel(
        hwnd,
        IDC_SIDEMENU_APPNAME,
        CustomLabelType::TextOnly,
        0, 118, SideMenu_WIDTH, 18);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = false;
        pData->BackColor = COLOR_BLACK;
        pData->TextColor = COLOR_WHITELIGHT;
        pData->FontSize = 10;
        pData->TextAlignment = CustomLabelAlignment::MiddleCenter;
        pData->wszText = L"IB-Tracker v" + version;
        pData->wszTextHot = pData->wszText;
        CustomLabel_SetOptions(hCtl, pData);
    }


    // SEPARATOR & MENU ITEMS
    nTop = 150;
    SideMenu_MakeSeparator(hwnd, nTop);

    nTop += 10;
    SideMenu_MakeMenuItem(hwnd, IDC_SIDEMENU_ACTIVETRADES, nTop, L"Active Trades");

    nTop += nItemHeight;
    SideMenu_MakeMenuItem(hwnd, IDC_SIDEMENU_CLOSEDTRADES, nTop, L"Closed Trades");

    nTop += nItemHeight + 6;
    SideMenu_MakeSeparator(hwnd, nTop);

    nTop += 10;
    SideMenu_MakeMenuItem(hwnd, IDC_SIDEMENU_NEWOPTIONSTRADE, nTop, L"Options Trade");

    nTop += nItemHeight;
    SideMenu_MakeMenuItem(hwnd, IDC_SIDEMENU_NEWSHARESTRADE, nTop, L"Shares Trade");

    nTop += nItemHeight;
    SideMenu_MakeMenuItem(hwnd, IDC_SIDEMENU_NEWFUTURESTRADE, nTop, L"Futures Trade");

    nTop += nItemHeight + 6;
    SideMenu_MakeSeparator(hwnd, nTop);

    nTop += 10;
    SideMenu_MakeMenuItem(hwnd, IDC_SIDEMENU_NEWIRONCONDOR, nTop, L"Iron Condor");

    nTop += nItemHeight;
    SideMenu_MakeMenuItem(hwnd, IDC_SIDEMENU_NEWSHORTSTRANGLE, nTop, L"Short Strangle");

    nTop += nItemHeight;
    SideMenu_MakeMenuItem(hwnd, IDC_SIDEMENU_NEWSHORTPUT, nTop, L"Short Put");

    nTop += nItemHeight;
    SideMenu_MakeMenuItem(hwnd, IDC_SIDEMENU_NEWSHORTCALL, nTop, L"Short Call");

    nTop += nItemHeight + 6;
    SideMenu_MakeSeparator(hwnd, nTop);

    nTop += 10;
    SideMenu_MakeMenuItem(hwnd, IDC_SIDEMENU_TICKERTOTALS, nTop, L"Ticker Totals");

    nTop += nItemHeight;
    SideMenu_MakeMenuItem(hwnd, IDC_SIDEMENU_DAILYTOTALS, nTop, L"Daily Totals");

    nTop += nItemHeight;
    SideMenu_MakeMenuItem(hwnd, IDC_SIDEMENU_TRANSACTIONS, nTop, L"Transactions");

    nTop += nItemHeight;
    SideMenu_MakeMenuItem(hwnd, IDC_SIDEMENU_RECONCILE, nTop, L"Reconcile");

    nTop += nItemHeight + 6;
    SideMenu_MakeSeparator(hwnd, nTop);

    nTop += 10;
    SideMenu_MakeMenuItem(hwnd, IDC_SIDEMENU_CONNECTTWS, nTop, L"Connect to TWS");

    nTop += nItemHeight + 6;
    SideMenu_MakeSeparator(hwnd, nTop);

    // Create a label that will display at the very bottom of the Main window
    // that allows toggling Autoconnect on/off. 
    nTop += 10;
    hCtl = CreateCustomLabel(
        hwnd,
        IDC_SIDEMENU_AUTOCONNECT,
        CustomLabelType::TextOnly,
        40, nTop, 100, 23);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = true;
        pData->AllowSelect = false;
        pData->BackColor = COLOR_BLACK;
        pData->BackColorHot = COLOR_SELECTION;
        pData->TextColor = COLOR_WHITEMEDIUM;
        pData->TextColorHot = COLOR_WHITEMEDIUM;
        pData->FontSize = 8;
        pData->FontSizeHot = 8;
        pData->wszText = GetStartupConnect() ? L"Autoconnect: ON" : L"Autoconnect: OFF";
        pData->wszTextHot = pData->wszText;
        CustomLabel_SetOptions(hCtl, pData);
    }


    return TRUE;
}


// ========================================================================================
// Select the specified menu item (and deselect any other menu items)
// ========================================================================================
void SideMenu_SelectMenuItem(HWND hParent, int CtrlId)
{
    HWND hCtrlSelected = GetDlgItem(hParent, CtrlId);
    HWND hCtrl = NULL;
    for (int i = IDC_SIDEMENU_FIRSTITEM; i <= IDC_SIDEMENU_LASTITEM; i++) {
        hCtrl = GetDlgItem(hParent, i);
        if (hCtrl == hCtrlSelected) continue;
        CustomLabel_Select(hCtrl, false);
    }
    CustomLabel_Select(hCtrlSelected, true);
}


// ========================================================================================
// Gets the ID of the currently active menu item.
// ========================================================================================
int SideMenu_GetActiveMenuItem(HWND hParent)
{
    HWND hCtrl = NULL;
    for (int ctrlId = IDC_SIDEMENU_FIRSTITEM; ctrlId <= IDC_SIDEMENU_LASTITEM; ctrlId++)
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
// SideMenu Window procedure
// ========================================================================================
LRESULT CSideMenu::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(m_hwnd, WM_CREATE, SideMenu_OnCreate);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, SideMenu_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, SideMenu_OnPaint);


    case MSG_TWS_CONNECT_START:
    {
        CustomLabel* pData = nullptr;
        pData = CustomLabel_GetOptions(GetDlgItem(m_hwnd, IDC_SIDEMENU_CONNECTTWS));
        if (pData) {
            pData->wszText = L"Connecting to TWS";
            AfxRedrawWindow(GetDlgItem(m_hwnd, IDC_SIDEMENU_CONNECTTWS));
        }
        return 0;
        break;
    }


    case MSG_TWS_CONNECT_SUCCESS:
    {
        CustomLabel* pData = nullptr;
        pData = CustomLabel_GetOptions(GetDlgItem(m_hwnd, IDC_SIDEMENU_CONNECTTWS));
        if (pData) {
            pData->wszText = L"TWS Connected";
            pData->TextColor = COLOR_GREEN;
            pData->TextColorHot = COLOR_GREEN;
            AfxRedrawWindow(GetDlgItem(m_hwnd, IDC_SIDEMENU_CONNECTTWS));
        }
        return 0;
        break;
    }


    case MSG_TWS_CONNECT_FAILURE:
    case MSG_TWS_CONNECT_DISCONNECT:
    {
        CustomLabel* pData = nullptr;
        pData = CustomLabel_GetOptions(GetDlgItem(m_hwnd, IDC_SIDEMENU_CONNECTTWS));
        if (pData) {
            pData->wszText = L"Connect to TWS";
            pData->TextColor = COLOR_WHITELIGHT;
            pData->TextColorHot = COLOR_WHITELIGHT;
            AfxRedrawWindow(GetDlgItem(m_hwnd, IDC_SIDEMENU_CONNECTTWS));
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

            case IDC_SIDEMENU_NEWOPTIONSTRADE:
            {
                int currSelection = SideMenu_GetActiveMenuItem(m_hwnd);
                SideMenu_SelectMenuItem(m_hwnd, CtrlId);
                if (TradeDialog_Show(TradeAction::NewOptionsTrade) == DIALOG_RETURN_CANCEL) {
                    SideMenu_SelectMenuItem(m_hwnd, currSelection);
                }
            }
            break;

            case IDC_SIDEMENU_NEWSHARESTRADE:
            {
                int currSelection = SideMenu_GetActiveMenuItem(m_hwnd);
                SideMenu_SelectMenuItem(m_hwnd, CtrlId);
                if (TradeDialog_Show(TradeAction::NewSharesTrade) == DIALOG_RETURN_CANCEL) {
                    SideMenu_SelectMenuItem(m_hwnd, currSelection);
                }
            }
            break;

            case IDC_SIDEMENU_NEWFUTURESTRADE:
            {
                int currSelection = SideMenu_GetActiveMenuItem(m_hwnd);
                SideMenu_SelectMenuItem(m_hwnd, CtrlId);
                if (TradeDialog_Show(TradeAction::NewFuturesTrade) == DIALOG_RETURN_CANCEL) {
                    SideMenu_SelectMenuItem(m_hwnd, currSelection);
                }
            }
            break;

            case IDC_SIDEMENU_NEWIRONCONDOR:
            {
                int currSelection = SideMenu_GetActiveMenuItem(m_hwnd);
                SideMenu_SelectMenuItem(m_hwnd, CtrlId);
                if (TradeDialog_Show(TradeAction::NewIronCondor) == DIALOG_RETURN_CANCEL) {
                    SideMenu_SelectMenuItem(m_hwnd, currSelection);
                }
            }
            break;

            case IDC_SIDEMENU_NEWSHORTSTRANGLE:
            {
                int currSelection = SideMenu_GetActiveMenuItem(m_hwnd);
                SideMenu_SelectMenuItem(m_hwnd, CtrlId);
                if (TradeDialog_Show(TradeAction::NewShortStrangle) == DIALOG_RETURN_CANCEL) {
                    SideMenu_SelectMenuItem(m_hwnd, currSelection);
                }
            }
            break;

            case IDC_SIDEMENU_NEWSHORTPUT:
            {
                int currSelection = SideMenu_GetActiveMenuItem(m_hwnd);
                SideMenu_SelectMenuItem(m_hwnd, CtrlId);
                if (TradeDialog_Show(TradeAction::NewShortPut) == DIALOG_RETURN_CANCEL) {
                    SideMenu_SelectMenuItem(m_hwnd, currSelection);
                }
            }
            break;

            case IDC_SIDEMENU_NEWSHORTCALL:
            {
                int currSelection = SideMenu_GetActiveMenuItem(m_hwnd);
                SideMenu_SelectMenuItem(m_hwnd, CtrlId);
                if (TradeDialog_Show(TradeAction::NewShortCall) == DIALOG_RETURN_CANCEL) {
                    SideMenu_SelectMenuItem(m_hwnd, currSelection);
                }
            }
            break;

            case IDC_SIDEMENU_CONNECTTWS:
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
                    // Destroy any existing ListBox line data
                    // This will also clear the LineData pointers and cancel any previous market data
                    ListBoxData_DestroyItemData(GetDlgItem(HWND_ACTIVETRADES, IDC_TRADES_LISTBOX));
                    ActiveTrades_ShowActiveTrades();
                }
                bProcessingConnectClick = false;
                break;
            }

            case IDC_SIDEMENU_ACTIVETRADES:
            {
                SideMenu_SelectMenuItem(m_hwnd, CtrlId);
                ActiveTrades_ShowActiveTrades();
                break;
            }

            case IDC_SIDEMENU_CLOSEDTRADES:
            {
                SideMenu_SelectMenuItem(m_hwnd, CtrlId);
                ClosedTrades_ShowClosedTrades();
                break;
            }

            case IDC_SIDEMENU_TICKERTOTALS:
            {
                SideMenu_SelectMenuItem(m_hwnd, CtrlId);
                TickerPanel_ShowTickerTotals();
                break;
            }

            case IDC_SIDEMENU_DAILYTOTALS:
            {
                SideMenu_SelectMenuItem(m_hwnd, CtrlId);
                DailyTotals_ShowDailyTotals(nullptr);
                break;
            }

            case IDC_SIDEMENU_TRANSACTIONS:
            {
                SideMenu_SelectMenuItem(m_hwnd, CtrlId);
                TransPanel_ShowTransactions();
                break;
            }

            case IDC_SIDEMENU_RECONCILE:
            {
                int currSelection = SideMenu_GetActiveMenuItem(m_hwnd);
                SideMenu_SelectMenuItem(m_hwnd, CtrlId);
                tws_performReconciliation();
                SideMenu_SelectMenuItem(m_hwnd, currSelection);
                break;
            }

            case IDC_SIDEMENU_AUTOCONNECT:
            {
                SetStartupConnect(!GetStartupConnect());
                if (GetStartupConnect()) {
                    CustomLabel_SetText(hCtl, L"Autoconnect: ON");
                }
                else {
                    CustomLabel_SetText(hCtl, L"Autoconnect: OFF");
                }
                SaveConfig();
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
