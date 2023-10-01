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
#include "Transactions/TransPanel.h"
#include "TickerTotals/TickerTotals.h"
#include "Utilities/ListBoxData.h"
#include "SideMenu.h"

CSideMenu SideMenu;

HWND HWND_SIDEMENU = NULL;


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
    Color back_color(COLOR_BLACK);
    SolidBrush back_brush(back_color);

    // Paint the background using brush.
    int nWidth = (ps.rcPaint.right - ps.rcPaint.left);
    int nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

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
        CustomLabelType::line_horizontal,
        0, nTop, SIDEMENU_WIDTH, 10);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->back_color = COLOR_BLACK;
        pData->line_color = COLOR_SEPARATOR;
        pData->line_width = 2;
        pData->margin_left = 10;
        pData->margin_right = 10;
        CustomLabel_SetOptions(hCtl, pData);
    }
}


// ========================================================================================
// Generic helper function to create a menu item.
// ========================================================================================
void SideMenu_MakeMenuItem(HWND hwnd, int CtrlId, int nTop, const std::wstring text)
{
    CustomLabel* pData = nullptr;
    HWND hCtl = CreateCustomLabel(
        hwnd, CtrlId,
        CustomLabelType::text_only,
        0, nTop, SIDEMENU_WIDTH, 28);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->hot_test_enable = true;
        pData->allow_select = true;
        pData->allow_notch = true;
        pData->selector_color = COLOR_GRAYDARK;   // MenuNotch should be same color as middle panel
        pData->back_color = COLOR_BLACK;
        pData->back_color_hot = COLOR_SELECTION;
        pData->back_color_selected = COLOR_SELECTION;
        pData->back_color_button_down = COLOR_SELECTION;
        pData->text_color = COLOR_WHITELIGHT;
        pData->text_color_hot = COLOR_WHITELIGHT;
        pData->font_size = 10;
        pData->font_size_hot = 10;
        pData->text = text;
        pData->text_hot = pData->text;
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
    nLeft = (SIDEMENU_WIDTH - 68) / 2;
    hCtl = CreateCustomLabel(
        hwnd,
        IDC_SIDEMENU_LOGO,
        CustomLabelType::image_only,
        nLeft, 20, 68, 68);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->hot_test_enable = false;
        pData->back_color = COLOR_BLACK;
        pData->image_width = 68;
        pData->image_height = 68;
        pData->pImage = LoadImageFromResource(pData->hInst, MAKEINTRESOURCE(IDB_LOGO), L"PNG");
        pData->pImageHot = LoadImageFromResource(pData->hInst, MAKEINTRESOURCE(IDB_LOGO), L"PNG");
        CustomLabel_SetOptions(hCtl, pData);
    }


    hCtl = CreateCustomLabel(
        hwnd,
        IDC_SIDEMENU_APPNAME,
        CustomLabelType::text_only,
        0, 100, SIDEMENU_WIDTH, 18);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->hot_test_enable = false;
        pData->back_color = COLOR_BLACK;
        pData->text_color = COLOR_WHITELIGHT;
        pData->font_size = 10;
        pData->text_alignment = CustomLabelAlignment::middle_center;
        pData->text = L"IB-Tracker";
        pData->text_hot = pData->text;
        CustomLabel_SetOptions(hCtl, pData);
    }


    hCtl = CreateCustomLabel(
        hwnd,
        IDC_SIDEMENU_APPVERSION,
        CustomLabelType::text_only,
        0, 118, SIDEMENU_WIDTH, 18);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->hot_test_enable = false;
        pData->back_color = COLOR_BLACK;
        pData->text_color = COLOR_WHITEMEDIUM;
        pData->font_size = 9;
        pData->text_alignment = CustomLabelAlignment::middle_center;
        pData->text = L"v" + version;
        pData->text_hot = pData->text;
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
    SideMenu_MakeMenuItem(hwnd, IDC_SIDEMENU_NEWSHARESTRADE, nTop, L"Shares Trade");

    nTop += nItemHeight;
    SideMenu_MakeMenuItem(hwnd, IDC_SIDEMENU_NEWFUTURESTRADE, nTop, L"Futures Trade");

    nTop += nItemHeight + 6;
    SideMenu_MakeSeparator(hwnd, nTop);

    nTop += 10;
    SideMenu_MakeMenuItem(hwnd, IDC_SIDEMENU_NEWOPTIONSTRADE, nTop, L"Options Trade");

    nTop += nItemHeight;
    SideMenu_MakeMenuItem(hwnd, IDC_SIDEMENU_NEWIRONCONDOR, nTop, L"Iron Condor");

    nTop += nItemHeight;
    SideMenu_MakeMenuItem(hwnd, IDC_SIDEMENU_NEWSHORTSTRANGLE, nTop, L"Short Strangle");

    nTop += nItemHeight;
    SideMenu_MakeMenuItem(hwnd, IDC_SIDEMENU_NEWSHORTPUT, nTop, L"Short Put");

    nTop += nItemHeight;
    SideMenu_MakeMenuItem(hwnd, IDC_SIDEMENU_NEWSHORTCALL, nTop, L"Short Call");

    nTop += nItemHeight;
    SideMenu_MakeMenuItem(hwnd, IDC_SIDEMENU_NEWSHORTLT112, nTop, L"Short LT112");

    nTop += nItemHeight + 6;
    SideMenu_MakeSeparator(hwnd, nTop);

    nTop += 10;
    SideMenu_MakeMenuItem(hwnd, IDC_SIDEMENU_TICKERTOTALS, nTop, L"Ticker Totals");

    nTop += nItemHeight;
    SideMenu_MakeMenuItem(hwnd, IDC_SIDEMENU_TRANSACTIONS, nTop, L"Transactions");

    nTop += nItemHeight;
    SideMenu_MakeMenuItem(hwnd, IDC_SIDEMENU_RECONCILE, nTop, L"Reconcile");

    nTop += nItemHeight;
    SideMenu_MakeMenuItem(hwnd, IDC_SIDEMENU_MYNOTES, nTop, L"My Notes");

    nTop += nItemHeight + 6;
    SideMenu_MakeSeparator(hwnd, nTop);

    nTop += 10;
    SideMenu_MakeMenuItem(hwnd, IDC_SIDEMENU_CONNECTTWS, nTop, L"Connect to TWS");

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
            if (pData->is_selected) return ctrlId;
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
        SetCursor(LoadCursor(0, IDC_WAIT));
        CustomLabel* pData = nullptr;
        pData = CustomLabel_GetOptions(GetDlgItem(m_hwnd, IDC_SIDEMENU_CONNECTTWS));
        if (pData) {
            pData->text = L"Connecting to TWS";
            AfxRedrawWindow(GetDlgItem(m_hwnd, IDC_SIDEMENU_CONNECTTWS));
        }
        return 0;
        break;
    }


    case MSG_TWS_CONNECT_SUCCESS:
    {
        SetCursor(LoadCursor(0, IDC_ARROW));
        CustomLabel* pData = nullptr;
        pData = CustomLabel_GetOptions(GetDlgItem(m_hwnd, IDC_SIDEMENU_CONNECTTWS));
        if (pData) {
            pData->text = L"TWS Connected";
            pData->text_color = COLOR_GREEN;
            pData->text_color_hot = COLOR_GREEN;
            AfxRedrawWindow(GetDlgItem(m_hwnd, IDC_SIDEMENU_CONNECTTWS));
        }
        return 0;
        break;
    }


    case MSG_TWS_CONNECT_WAIT_RECONNECTION:
    {
        SetCursor(LoadCursor(0, IDC_ARROW));
        CustomLabel* pData = nullptr;
        pData = CustomLabel_GetOptions(GetDlgItem(m_hwnd, IDC_SIDEMENU_CONNECTTWS));
        if (pData) {
            pData->text = L"Reconnect Wait";
            pData->text_color = COLOR_RED;
            pData->text_color_hot = COLOR_RED;
            AfxRedrawWindow(GetDlgItem(m_hwnd, IDC_SIDEMENU_CONNECTTWS));
        }
        return 0;
        break;
    }


    case MSG_TWS_WARNING_EXCEPTION:
    {
        SetCursor(LoadCursor(0, IDC_ARROW));
        CustomLabel_SetText(GetDlgItem(HWND_MAINWINDOW, IDC_MAINWINDOW_WARNING),
            L"Monitoring thread exception! Please restart application.");
        ShowWindow(GetDlgItem(HWND_MAINWINDOW, IDC_MAINWINDOW_WARNING), SW_SHOWNORMAL);
        return 0;
        break;
    }


    case MSG_TWS_CONNECT_FAILURE:
    case MSG_TWS_CONNECT_DISCONNECT:
    {
        SetCursor(LoadCursor(0, IDC_ARROW));
        CustomLabel* pData = nullptr;
        pData = CustomLabel_GetOptions(GetDlgItem(m_hwnd, IDC_SIDEMENU_CONNECTTWS));
        if (pData) {
            pData->text = L"Connect to TWS";
            pData->text_color = COLOR_WHITELIGHT;
            pData->text_color_hot = COLOR_WHITELIGHT;
            AfxRedrawWindow(GetDlgItem(m_hwnd, IDC_SIDEMENU_CONNECTTWS));
        }
        tws_EndMonitorThread();
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
                int current_selection = SideMenu_GetActiveMenuItem(m_hwnd);
                SideMenu_SelectMenuItem(m_hwnd, CtrlId);
                if (TradeDialog_Show(TradeAction::new_options_trade) == DIALOG_RETURN_CANCEL) {
                    SideMenu_SelectMenuItem(m_hwnd, current_selection);
                }
            }
            break;

            case IDC_SIDEMENU_NEWSHARESTRADE:
            {
                int current_selection = SideMenu_GetActiveMenuItem(m_hwnd);
                SideMenu_SelectMenuItem(m_hwnd, CtrlId);
                if (TradeDialog_Show(TradeAction::new_shares_trade) == DIALOG_RETURN_CANCEL) {
                    SideMenu_SelectMenuItem(m_hwnd, current_selection);
                }
            }
            break;

            case IDC_SIDEMENU_NEWFUTURESTRADE:
            {
                int current_selection = SideMenu_GetActiveMenuItem(m_hwnd);
                SideMenu_SelectMenuItem(m_hwnd, CtrlId);
                if (TradeDialog_Show(TradeAction::new_futures_trade) == DIALOG_RETURN_CANCEL) {
                    SideMenu_SelectMenuItem(m_hwnd, current_selection);
                }
            }
            break;

            case IDC_SIDEMENU_NEWIRONCONDOR:
            {
                int current_selection = SideMenu_GetActiveMenuItem(m_hwnd);
                SideMenu_SelectMenuItem(m_hwnd, CtrlId);
                if (TradeDialog_Show(TradeAction::new_iron_condor) == DIALOG_RETURN_CANCEL) {
                    SideMenu_SelectMenuItem(m_hwnd, current_selection);
                }
            }
            break;

            case IDC_SIDEMENU_NEWSHORTLT112:
            {
                int current_selection = SideMenu_GetActiveMenuItem(m_hwnd);
                SideMenu_SelectMenuItem(m_hwnd, CtrlId);
                if (TradeDialog_Show(TradeAction::new_short_LT112) == DIALOG_RETURN_CANCEL) {
                    SideMenu_SelectMenuItem(m_hwnd, current_selection);
                }
            }
            break;

            case IDC_SIDEMENU_NEWSHORTSTRANGLE:
            {
                int current_selection = SideMenu_GetActiveMenuItem(m_hwnd);
                SideMenu_SelectMenuItem(m_hwnd, CtrlId);
                if (TradeDialog_Show(TradeAction::new_short_strangle) == DIALOG_RETURN_CANCEL) {
                    SideMenu_SelectMenuItem(m_hwnd, current_selection);
                }
            }
            break;

            case IDC_SIDEMENU_NEWSHORTPUT:
            {
                int current_selection = SideMenu_GetActiveMenuItem(m_hwnd);
                SideMenu_SelectMenuItem(m_hwnd, CtrlId);
                if (TradeDialog_Show(TradeAction::new_short_put) == DIALOG_RETURN_CANCEL) {
                    SideMenu_SelectMenuItem(m_hwnd, current_selection);
                }
            }
            break;

            case IDC_SIDEMENU_NEWSHORTCALL:
            {
                int current_selection = SideMenu_GetActiveMenuItem(m_hwnd);
                SideMenu_SelectMenuItem(m_hwnd, CtrlId);
                if (TradeDialog_Show(TradeAction::new_short_call) == DIALOG_RETURN_CANCEL) {
                    SideMenu_SelectMenuItem(m_hwnd, current_selection);
                }
            }
            break;

            case IDC_SIDEMENU_CONNECTTWS:
            {
                // If already connected then don't try to connect again
                if (tws_IsConnected()) break;

                // Prevent multiple clicks of the connect button by waiting until
                // the first click is finished.
                static bool processing_connect_click = false;
                if (processing_connect_click) break;
                processing_connect_click = true;
                bool res = tws_Connect();
                processing_connect_click = false;
                break;
            }

            case IDC_SIDEMENU_ACTIVETRADES:
            {
                SideMenu_SelectMenuItem(m_hwnd, CtrlId);
                ActiveTrades_ShowActiveTrades(false);
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

            case IDC_SIDEMENU_TRANSACTIONS:
            {
                SideMenu_SelectMenuItem(m_hwnd, CtrlId);
                TransPanel_ShowTransactions();
                break;
            }

            case IDC_SIDEMENU_RECONCILE:
            {
                int current_selection = SideMenu_GetActiveMenuItem(m_hwnd);
                SideMenu_SelectMenuItem(m_hwnd, CtrlId);
                tws_PerformReconciliation();
                SideMenu_SelectMenuItem(m_hwnd, current_selection);
                break;
            }

            }  // switch

            return 0;
        }   // if
    }  // case
    break;


    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);

    }   // end of switch statement

    return 0;

}
