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

#include "ActiveTrades/ActiveTrades.h"
#include "ClosedTrades/ClosedTrades.h"
#include "Transactions/TransPanel.h"
#include "TickerTotals/TickerTotals.h"
#include "TradePlan/TradePlan.h"
#include "JournalNotes/JournalNotes.h"
#include "MainWindow/MainWindow.h"
#include "MainWindow/tws-client.h"
#include "CustomLabel/CustomLabel.h"

#include "TabPanel.h"


CTabPanel TabPanel;

HWND HWND_TABPANEL = NULL;

const int panel_count = 6;
static int panel_ids[panel_count] =
{ IDC_TABPANEL_ACTIVETRADES,
    IDC_TABPANEL_CLOSEDTRADES,
    IDC_TABPANEL_TRANSACTIONS,
    IDC_TABPANEL_TICKERTOTALS,
    IDC_TABPANEL_JOURNALNOTES,
    IDC_TABPANEL_TRADEPLAN
};


// ========================================================================================
// Select the incoming CtrlId in the TabPanel and deselect the others
// ========================================================================================
void TabPanel_SelectPanelItem(HWND hwnd, int CtrlId)
{
    // Use pData pointers directly with setting the label data instead of calling
    // the wrappers in order to prevent redrawing on every SetSelected and CustomLabel_SetSelectedUnderline.
    CustomLabel* pData = nullptr;
    
    int previous_selected_id = -1;  // Save previously selected so it can be redrawn afterwards.

    for (int i = 0; i < panel_count; ++i) {
        HWND hCtl = GetDlgItem(hwnd, panel_ids[i]);
        pData = CustomLabel_GetOptions(hCtl);
        if (!pData) continue;

        if (pData->is_selected) previous_selected_id = panel_ids[i];

        if (panel_ids[i] == CtrlId) {
            pData->is_selected = true;
            pData->is_selected_underline = true;
            pData->selected_underline_color = COLOR_GREEN;
            pData->selected_underline_width = 3;
        }
        else {
            pData->is_selected = false;
        }
        SetWindowLongPtr(hCtl, 0, (LONG_PTR)pData);
    }

    if (previous_selected_id != CtrlId) {
        AfxRedrawWindow(GetDlgItem(hwnd, previous_selected_id));
        AfxRedrawWindow(GetDlgItem(hwnd, CtrlId));
    }
}


// ========================================================================================
// Return the Control ID of the selected TabPanel panel.
// ========================================================================================
int TabPanel_GetSelectedPanel(HWND hwnd)
{
    CustomLabel* pData = nullptr;
    
    for (int i = 0; i < panel_count; ++i) {
        HWND hCtl = GetDlgItem(hwnd, panel_ids[i]);
        pData = CustomLabel_GetOptions(hCtl);
        if (!pData) continue;
        if (pData->is_selected) return panel_ids[i];
    }
    return -1;
}




// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: TabPanel
// ========================================================================================
BOOL TabPanel_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    // Handle all of the painting in WM_PAINT
    return TRUE;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: TabPanel
// ========================================================================================
void TabPanel_OnPaint(HWND hwnd)
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
// Process WM_CREATE message for window/dialog: TabPanel
// ========================================================================================
BOOL TabPanel_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    HWND_TABPANEL = hwnd;

    int item_top = 0;
    int item_left = 0;
    int item_height = TABPANEL_HEIGHT - 2;
    int item_width = 0;

    int separator_left = 0;
    int separator_top = 6;
    int separator_width = 6;
    int separator_height = item_height - (separator_top * 2);

    std::wstring font_name = AfxGetDefaultFont();
    int font_size = 9;

    item_left = 12;
    item_top = 5;
    item_width = 26;
    item_height = 26;

    HWND hCtl = CustomLabel_SimpleImageLabel(
        hwnd, IDC_TABPANEL_CONNECT, 
        IDB_DISCONNECT, IDB_DISCONNECT,
        20, 20, item_left, item_top, item_width, item_height);
    std::wstring tooltip_text = L"Click to connect";
    CustomLabel_SetImageOffset(hCtl, 3, 3);
    CustomLabel_SetToolTip(hCtl, tooltip_text);
    CustomLabel_SetBackColorHot(hCtl, COLOR_SELECTION);

    item_left += item_width + 3;
    hCtl = CustomLabel_SimpleImageLabel(
        hwnd, IDC_TABPANEL_RECONCILE, 
        IDB_RECONCILE, IDB_RECONCILE,
        22, 22, item_left, item_top, item_width, item_height);
    tooltip_text = L"Reconcile";
    CustomLabel_SetImageOffset(hCtl, 2, 2);
    CustomLabel_SetToolTip(hCtl, tooltip_text);
    CustomLabel_SetBackColorHot(hCtl, COLOR_SELECTION);
    
    
    item_left = 74;
    item_top = 0;
    item_width = 90;
    item_height = TABPANEL_HEIGHT - 2;
    hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TABPANEL_ACTIVETRADES, L"Active Trades",
        COLOR_WHITEMEDIUM, COLOR_BLACK, CustomLabelAlignment::middle_center,
        item_left, item_top, item_width, item_height);
    CustomLabel_SetFontHot(hCtl, font_name, font_size, true);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITE);
    CustomLabel_SetBackColorSelected(hCtl, COLOR_GRAYDARK);

    separator_left = item_left + item_width;
    hCtl = CustomLabel_VerticalLine(hwnd, IDC_TABPANEL_SEPARATOR, COLOR_WHITEDARK, COLOR_BLACK,
        separator_left, separator_top, separator_width, separator_height);
        
    
    item_left = separator_left + separator_width;
    item_width = 90;
    hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TABPANEL_CLOSEDTRADES, L"Closed Trades",
        COLOR_WHITEMEDIUM, COLOR_BLACK, CustomLabelAlignment::middle_center,
        item_left, item_top, item_width, item_height);
    CustomLabel_SetFontHot(hCtl, font_name, font_size, true);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITE);
    CustomLabel_SetBackColorSelected(hCtl, COLOR_GRAYDARK);

    separator_left = item_left + item_width;
    hCtl = CustomLabel_VerticalLine(hwnd, IDC_TABPANEL_SEPARATOR, COLOR_WHITEDARK, COLOR_BLACK,
        separator_left, separator_top, separator_width, separator_height);

    
    item_left = separator_left + separator_width;
    item_width = 84;
    hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TABPANEL_TRANSACTIONS, L"Transactions",
        COLOR_WHITEMEDIUM, COLOR_BLACK, CustomLabelAlignment::middle_center,
        item_left, item_top, item_width, item_height);
    CustomLabel_SetFontHot(hCtl, font_name, font_size, true);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITE);
    CustomLabel_SetBackColorSelected(hCtl, COLOR_GRAYDARK);

    separator_left = item_left + item_width;
    hCtl = CustomLabel_VerticalLine(hwnd, IDC_TABPANEL_SEPARATOR, COLOR_WHITEDARK, COLOR_BLACK,
        separator_left, separator_top, separator_width, separator_height);

    
    item_left = separator_left + separator_width;
    item_width = 84;
    hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TABPANEL_TICKERTOTALS, L"Ticker Totals",
        COLOR_WHITEMEDIUM, COLOR_BLACK, CustomLabelAlignment::middle_center,
        item_left, item_top, item_width, item_height);
    CustomLabel_SetFontHot(hCtl, font_name, font_size, true);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITE);
    CustomLabel_SetBackColorSelected(hCtl, COLOR_GRAYDARK);

    separator_left = item_left + item_width;
    hCtl = CustomLabel_VerticalLine(hwnd, IDC_TABPANEL_SEPARATOR, COLOR_WHITEDARK, COLOR_BLACK,
        separator_left, separator_top, separator_width, separator_height);


    item_left = separator_left + separator_width;
    item_width = 90;
    hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TABPANEL_JOURNALNOTES, L"Journal Notes",
        COLOR_WHITEMEDIUM, COLOR_BLACK, CustomLabelAlignment::middle_center,
        item_left, item_top, item_width, item_height);
    CustomLabel_SetFontHot(hCtl, font_name, font_size, true);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITE);
    CustomLabel_SetBackColorSelected(hCtl, COLOR_GRAYDARK);

    separator_left = item_left + item_width;
    hCtl = CustomLabel_VerticalLine(hwnd, IDC_TABPANEL_SEPARATOR, COLOR_WHITEDARK, COLOR_BLACK,
        separator_left, separator_top, separator_width, separator_height);


    item_left = separator_left + separator_width;
    item_width = 76;
    hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TABPANEL_TRADEPLAN, L"Trade Plan",
        COLOR_WHITEMEDIUM, COLOR_BLACK, CustomLabelAlignment::middle_center,
        item_left, item_top, item_width, item_height);
    CustomLabel_SetFontHot(hCtl, font_name, font_size, true);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITE);
    CustomLabel_SetBackColorSelected(hCtl, COLOR_GRAYDARK);

    separator_left = item_left + item_width;
    hCtl = CustomLabel_VerticalLine(hwnd, IDC_TABPANEL_SEPARATOR, COLOR_WHITEDARK, COLOR_BLACK,
        separator_left, separator_top, separator_width, separator_height);


    TabPanel_SelectPanelItem(hwnd, IDC_TABPANEL_ACTIVETRADES);

    return TRUE;
}


// ========================================================================================
// TabPanel Window procedure
// ========================================================================================
LRESULT CTabPanel::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(m_hwnd, WM_CREATE, TabPanel_OnCreate);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, TabPanel_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, TabPanel_OnPaint);


    case MSG_TWS_CONNECT_START:
    {
        SetCursor(LoadCursor(0, IDC_WAIT));
        return 0;
    }
    break;


    case MSG_TWS_CONNECT_SUCCESS:
    {
        SetCursor(LoadCursor(0, IDC_ARROW));
        HWND hCtl = GetDlgItem(HWND_TABPANEL, IDC_TABPANEL_CONNECT);
        CustomLabel_SetImages(hCtl, IDB_CONNECT, IDB_CONNECT);
        CustomLabel_SetToolTip(hCtl, L"Connected");
        return 0;
    }
    break;


    case MSG_TWS_CONNECT_WAIT_RECONNECTION:
    {
        SetCursor(LoadCursor(0, IDC_ARROW));
        return 0;
    }
    break;


    case MSG_TWS_WARNING_EXCEPTION:
    {
        SetCursor(LoadCursor(0, IDC_ARROW));
        //CustomLabel_SetText(GetDlgItem(HWND_MAINWINDOW, IDC_MAINWINDOW_WARNING),
        //    L"Monitoring thread exception! Please restart application.");
        //ShowWindow(GetDlgItem(HWND_MAINWINDOW, IDC_MAINWINDOW_WARNING), SW_SHOWNORMAL);
        return 0;
    }
    break;


    case MSG_TWS_CONNECT_FAILURE:
    case MSG_TWS_CONNECT_DISCONNECT:
    {
        SetCursor(LoadCursor(0, IDC_ARROW));
        HWND hCtl = GetDlgItem(HWND_TABPANEL, IDC_TABPANEL_CONNECT);
        CustomLabel_SetImages(hCtl, IDB_DISCONNECT, IDB_DISCONNECT);
        CustomLabel_SetToolTip(hCtl, L"Click to connect");
        tws_EndMonitorThread();
        return 0;
    }
    break;


    case MSG_CUSTOMLABEL_CLICK:
    {
        HWND hCtl = (HWND)lParam;
        int CtrlId = (int)wParam;

        if (hCtl == NULL) return 0;

        if (CtrlId == IDC_TABPANEL_CONNECT) {
            // If already connected then don't try to connect again
            if (!tws_IsConnected()) {
                // Prevent multiple clicks of the connect button by waiting until
                // the first click is finished.
                static bool processing_connect_click = false;
                if (processing_connect_click) break;
                processing_connect_click = true;
                tws_Connect();
                processing_connect_click = false;
            }
            break;
        }

        if (CtrlId == IDC_TABPANEL_RECONCILE) {
            tws_PerformReconciliation();
            break;
        }

        // Do not refresh if the panel is already active
        if (CtrlId == TabPanel_GetSelectedPanel(m_hwnd)) {
            break;
        }


        TabPanel_SelectPanelItem(m_hwnd, CtrlId);

        if (CtrlId == IDC_TABPANEL_ACTIVETRADES) {
            ActiveTrades_ShowActiveTrades();
        }
        if (CtrlId == IDC_TABPANEL_CLOSEDTRADES) {
            ClosedTrades_ShowClosedTrades();
        }
        if (CtrlId == IDC_TABPANEL_TRANSACTIONS) {
            TransPanel_ShowTransactions();
        }
        if (CtrlId == IDC_TABPANEL_TICKERTOTALS) {
            TickerPanel_ShowTickerTotals();
        }
        if (CtrlId == IDC_TABPANEL_JOURNALNOTES) {
            JournalNotes_ShowJournalNotes();
        }
        if (CtrlId == IDC_TABPANEL_TRADEPLAN) {
            TradePlan_ShowTradePlan();
        }

        return 0;
    }
    break;

    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);

    }   // end of switch statement

    return 0;

}
