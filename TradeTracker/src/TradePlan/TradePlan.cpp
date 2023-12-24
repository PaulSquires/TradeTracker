/*

MIT License

Copyright(c) 2023-2024 Paul Squires

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
#include "CustomTextBox/CustomTextBox.h"
#include "CustomVScrollBar/CustomVScrollBar.h"
#include "MainWindow/MainWindow.h"
#include "Config/Config.h"
#include "Utilities/ListBoxData.h"
#include "Utilities/AfxWin.h"
#include "TradePlan.h"


HWND HWND_TRADEPLAN = NULL;

CTradePlan TradePlan;

void TradePlan_OnSize(HWND hwnd, UINT state, int cx, int cy);


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: TradePlan
// ========================================================================================
bool TradePlan_OnEraseBkgnd(HWND hwnd, HDC hdc) {
    // Handle all of the painting in WM_PAINT
    return true;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: TradePlan
// ========================================================================================
void TradePlan_OnPaint(HWND hwnd) {
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    Graphics graphics(hdc);

    // Create the background brush
    SolidBrush back_brush(COLOR_GRAYDARK);

    // Paint the background using brush.
    int width = (ps.rcPaint.right - ps.rcPaint.left);
    int height = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, width, height);

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Process WM_COMMAND message for window/dialog: TradePlan
// ========================================================================================
void TradePlan_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {
    static bool is_notes_dirty = false;
    static std::wstring notes = L"";

    switch (id) {
    case IDC_TRADEPLAN_TXTNOTES: {
        if (codeNotify == EN_KILLFOCUS) {
            if (is_notes_dirty == true) {

                // Save TradePlan because the data has changed.
                SetTradePlanText(notes);

                is_notes_dirty = false;
                notes = L"";
            }
            break;
        }
        else if (codeNotify == EN_CHANGE) {
            notes = AfxGetWindowText(hwndCtl);
            is_notes_dirty = true;
            break;
        }
        else if (codeNotify == EN_SETFOCUS) {
            is_notes_dirty = false;
            break;
        }

        break;
    }

    }
}


// ========================================================================================
// Process WM_SIZE message for window/dialog: TradePlan
// ========================================================================================
void TradePlan_OnSize(HWND hwnd, UINT state, int cx, int cy) {
    HWND hNotesLabel = GetDlgItem(hwnd, IDC_TRADEPLAN_LBLNOTES);
    HWND hNotesTextBox = GetDlgItem(hwnd, IDC_TRADEPLAN_TXTNOTES);
    HWND hCustomVScrollBar = GetDlgItem(hwnd, IDC_TRADEPLAN_CUSTOMVSCROLLBAR);

    HDWP hdwp = BeginDeferWindowPos(4);

    // Do not call the calcVThumbRect() function during a scrollbar move. 
    bool show_scrollbar = false;
    CustomVScrollBar* pData = CustomVScrollBar_GetPointer(hCustomVScrollBar);
    if (pData) {
        if (pData->drag_active) {
            show_scrollbar = true;
        }
        else {
            show_scrollbar = pData->calcVThumbRect();
        }
    }
    int custom_scrollbar_width = show_scrollbar ? AfxScaleX(CUSTOMVSCROLLBAR_WIDTH) : 0;

    // Move and size the top label into place
    int height_notes_label = AfxScaleY(24);

    hdwp = DeferWindowPos(hdwp, hNotesLabel, 0,
        0, 0, cx, height_notes_label, SWP_NOZORDER | SWP_SHOWWINDOW);

    hdwp = DeferWindowPos(hdwp, hNotesTextBox, 0,
        0, height_notes_label, cx - custom_scrollbar_width, cy - height_notes_label, SWP_NOZORDER | SWP_SHOWWINDOW);

    hdwp = DeferWindowPos(hdwp, hCustomVScrollBar, 0, 
        cx - custom_scrollbar_width, height_notes_label, 
        custom_scrollbar_width, cy - height_notes_label, 
        SWP_NOZORDER | (show_scrollbar ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));

    EndDeferWindowPos(hdwp);
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: TradePlan
// ========================================================================================
bool TradePlan_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct) {
    HWND_TRADEPLAN = hwnd;

    HWND hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TRADEPLAN_LBLNOTES, L"Trade Plan",
        COLOR_WHITELIGHT, COLOR_BLACK);

    hCtl = CreateCustomTextBox(hwnd, IDC_TRADEPLAN_TXTNOTES, true, ES_LEFT,
        L"", 0, 0, 0, 0);
    CustomTextBox_SetMargins(hCtl, 3, 3);
    CustomTextBox_SetColors(hCtl, COLOR_WHITELIGHT, COLOR_GRAYDARK);
    CustomTextBox_SetSelectOnFocus(hCtl, false);

    // Create our custom vertical scrollbar and attach the TextBox to it.
    HWND hScrollBar = CreateCustomVScrollBar(hwnd, IDC_TRADEPLAN_CUSTOMVSCROLLBAR, hCtl, Controls::MultilineTextBox);
    CustomTextBox_AttachScrollBar(hCtl, hScrollBar);

    return true;
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CTradePlan::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        HANDLE_MSG(m_hwnd, WM_CREATE, TradePlan_OnCreate);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, TradePlan_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, TradePlan_OnPaint);
        HANDLE_MSG(m_hwnd, WM_COMMAND, TradePlan_OnCommand);
        HANDLE_MSG(m_hwnd, WM_SIZE, TradePlan_OnSize);
    }
    return DefWindowProc(m_hwnd, msg, wParam, lParam);
}


// ========================================================================================
// Display the TradePlan side panel.
// ========================================================================================
void TradePlan_ShowTradePlan() {
    HWND hTextBox = GetDlgItem(HWND_TRADEPLAN, IDC_TRADEPLAN_TXTNOTES);
    HWND hCustomVScrollBar = GetDlgItem(HWND_TRADEPLAN, IDC_TRADEPLAN_CUSTOMVSCROLLBAR);

    // Load the TradePlan into the textbox
    std::wstring wszText = GetTradePlanText();
    CustomTextBox_SetText(hTextBox, wszText);

    // Ensure that the TradePlan panel is set
    MainWindow_SetRightPanel(HWND_TRADEPLAN);

    CustomVScrollBar_Recalculate(hCustomVScrollBar);

    SetFocus(hTextBox);
}
