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
#include "CustomPopupMenu/CustomPopupMenu.h"
#include "DatePicker/Calendar.h"
#include "Category/Category.h"
#include "Utilities/ListBoxData.h"
#include "MainWindow/MainWindow.h"

#include "FilterPanel.h"



// ========================================================================================
// Get the HWND's of the the controls on the form.
// ========================================================================================
inline HWND CFilterPanel::TickerFilterLabel() {
    return GetDlgItem(hWindow, IDC_FILTER_LBLTICKERFILTER);
}
inline HWND CFilterPanel::TickerTextBox() {
    return GetDlgItem(hWindow, IDC_FILTER_TXTTICKER);
}
inline HWND CFilterPanel::TickerGoButton() {
    return GetDlgItem(hWindow, IDC_FILTER_CMDTICKERGO);
}
inline HWND CFilterPanel::DateFilterLabel() {
    return GetDlgItem(hWindow, IDC_FILTER_LBLDATEFILTER);
}
inline HWND CFilterPanel::TransDateCombo() {
    return GetDlgItem(hWindow, IDC_FILTER_TRANSDATE);
}
inline HWND CFilterPanel::TransDateButton() {
    return GetDlgItem(hWindow, IDC_FILTER_CMDTRANSDATE);
}
inline HWND CFilterPanel::StartDateLabel() {
    return GetDlgItem(hWindow, IDC_FILTER_LBLSTARTDATE);
}
inline HWND CFilterPanel::StartDateCombo() {
    return GetDlgItem(hWindow, IDC_FILTER_STARTDATE);
}
inline HWND CFilterPanel::StartDateButton() {
    return GetDlgItem(hWindow, IDC_FILTER_CMDSTARTDATE);
}
inline HWND CFilterPanel::EndDateLabel() {
    return GetDlgItem(hWindow, IDC_FILTER_LBLENDDATE);
}
inline HWND CFilterPanel::EndDateCombo() {
    return GetDlgItem(hWindow, IDC_FILTER_ENDDATE);
}
inline HWND CFilterPanel::EndDateButton() {
    return GetDlgItem(hWindow, IDC_FILTER_CMDENDDATE);
}
inline HWND CFilterPanel::CategoryFilterLabel() {
    return GetDlgItem(hWindow, IDC_FILTER_LBLCATEGORYFILTER);
}
inline HWND CFilterPanel::CategoryCombo() {
    return GetDlgItem(hWindow, IDC_FILTER_CATEGORY);
}


// ========================================================================================
// Return string based on TransDateFilterType
// ========================================================================================
std::wstring CFilterPanel::GetFilterDescription(int idx) {
    switch ((TransDateFilterType)idx) {
    case TransDateFilterType::Today: return L"Today";
    case TransDateFilterType::Yesterday: return L"Yesterday";
    case TransDateFilterType::Days7: return L"7 days";
    case TransDateFilterType::Days14: return L"14 days";
    case TransDateFilterType::Days30: return L"30 days";
    case TransDateFilterType::Days60: return L"60 days";
    case TransDateFilterType::Days120: return L"120 days";
    case TransDateFilterType::YearToDate: return L"Year to Date";
    case TransDateFilterType::Custom: return L"Custom";
    default: return L"";
    }
}


// ========================================================================================
// Set the StartDate and EndDate based on the current value of the Date Filter.
// ========================================================================================
void CFilterPanel::SetStartEndDates(HWND hwnd) {

    // Do not modify dates if Custom has been set.
    if (selected_transdate == TransDateFilterType::Custom) return;

    std::wstring end_date = AfxCurrentDate();   // ISO format
    std::wstring start_date = end_date;       // ISO format
    int adjust_days = 0;

    if (selected_transdate == TransDateFilterType::YearToDate) {
        start_date = end_date.substr(0, 4) + L"-01-01";
    }
    else if (selected_transdate == TransDateFilterType::Yesterday) {
        start_date = AfxDateAddDays(end_date, -1);
        end_date = start_date;
    }
    else if (selected_transdate == TransDateFilterType::Today) {
        // Dates are already set to current date
    }
    else {
        switch (selected_transdate) {
        case TransDateFilterType::Days7: adjust_days = 7; break;
        case TransDateFilterType::Days14: adjust_days = 14; break;
        case TransDateFilterType::Days30: adjust_days = 30; break;
        case TransDateFilterType::Days60: adjust_days = 60; break;
        case TransDateFilterType::Days120: adjust_days = 120; break;
        }
        start_date = AfxDateAddDays(end_date, -(adjust_days));
    }

    filter_start_date = start_date;
    filter_end_date = end_date;

    CustomLabel_SetText(StartDateCombo(), AfxLongDate(filter_start_date));
    CustomLabel_SetText(EndDateCombo(), AfxLongDate(filter_end_date));
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: FilterPanel
// ========================================================================================
bool CFilterPanel::OnEraseBkgnd(HWND hwnd, HDC hdc) {
    // Handle all of the painting in WM_PAINT
    return true;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: FilterPanel
// ========================================================================================
void CFilterPanel::OnPaint(HWND hwnd) {
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    Graphics graphics(hdc);

    // Create the background brush
    SolidBrush back_brush(COLOR_BLACK);

    // Paint the background using brush.
    int width = (ps.rcPaint.right - ps.rcPaint.left);
    int height = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, width, height);

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Process WM_SIZE message for window/dialog: FilterPanel
// ========================================================================================
void CFilterPanel::OnSize(HWND hwnd, UINT state, int cx, int cy) {


    int left = 0;
    int top = 0;
    int width = 0;
    int height = AfxScaleY(23);
    int start_top = top;

    start_top = height;

    HDWP hdwp = BeginDeferWindowPos(15);

    top = start_top;
    width = AfxScaleX(75);
    hdwp = DeferWindowPos(hdwp, TickerFilterLabel(), 0,
        left, top, width, height, SWP_NOZORDER | SWP_SHOWWINDOW);
    hdwp = DeferWindowPos(hdwp, TickerTextBox(), 0,
        left, top + height, width, height, SWP_NOZORDER | SWP_SHOWWINDOW);
    left += width;
    width = AfxScaleX(23);
    hdwp = DeferWindowPos(hdwp, TickerGoButton(), 0,
        left, top + height, width, height, SWP_NOZORDER | SWP_SHOWWINDOW);

    top = start_top;
    left += width + AfxScaleX(18);
    width = AfxScaleX(90);
    hdwp = DeferWindowPos(hdwp, DateFilterLabel(), 0,
        left, top, width, height, SWP_NOZORDER | SWP_SHOWWINDOW);
    hdwp = DeferWindowPos(hdwp, TransDateCombo(), 0,
        left, top + height, width, height, SWP_NOZORDER | SWP_SHOWWINDOW);
    left += width;
    width = AfxScaleX(23);
    hdwp = DeferWindowPos(hdwp, TransDateButton(), 0,
        left, top + height, width, height, SWP_NOZORDER | SWP_SHOWWINDOW);

    top = start_top;
    left += width + AfxScaleX(18);
    width = AfxScaleX(90);
    hdwp = DeferWindowPos(hdwp, StartDateLabel(), 0,
        left, top, width, height, SWP_NOZORDER | SWP_SHOWWINDOW);
    hdwp = DeferWindowPos(hdwp, StartDateCombo(), 0,
        left, top + height, width, height, SWP_NOZORDER | SWP_SHOWWINDOW);
    left += width;
    width = AfxScaleX(23);
    hdwp = DeferWindowPos(hdwp, StartDateButton(), 0,
        left, top + height, width, height, SWP_NOZORDER | SWP_SHOWWINDOW);

    top = start_top;
    left += width + AfxScaleX(18);
    width = AfxScaleX(90);
    hdwp = DeferWindowPos(hdwp, EndDateLabel(), 0,
        left, top, width, height, SWP_NOZORDER | SWP_SHOWWINDOW);
    hdwp = DeferWindowPos(hdwp, EndDateCombo(), 0,
        left, top + height, width, height, SWP_NOZORDER | SWP_SHOWWINDOW);
    left += width;
    width = AfxScaleX(23);
    hdwp = DeferWindowPos(hdwp, EndDateButton(), 0,
        left, top + height, width, height, SWP_NOZORDER | SWP_SHOWWINDOW);

    top = start_top;
    left += width + AfxScaleX(18);
    width = AfxScaleX(90);
    hdwp = DeferWindowPos(hdwp, CategoryFilterLabel(), 0,
        left, top, width, height, SWP_NOZORDER | SWP_SHOWWINDOW);
    hdwp = DeferWindowPos(hdwp, CategoryCombo(), 0,
        left, top + height, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_SHOWWINDOW);

    EndDeferWindowPos(hdwp);

    return;
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: FilterPanel
// ========================================================================================
bool CFilterPanel::OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct) {
    hWindow = hwnd;

    int horiz_text_margin = 0;
    int vert_text_margin = 3;

    DWORD light_text_color = COLOR_WHITEDARK;
    DWORD dark_back_color = COLOR_GRAYMEDIUM;
    std::wstring font_name = AfxGetDefaultFont();
    int font_size = 8;

    CustomLabel_SimpleLabel(hwnd, IDC_FILTER_LBLTICKERFILTER, L"Ticker Filter",
        COLOR_WHITEDARK, COLOR_BLACK);
    HWND hCtl = CreateCustomTextBox(hwnd, IDC_FILTER_TXTTICKER, false, ES_LEFT | ES_UPPERCASE, L"", 0, 0, 0, 0);
    CustomTextBox_SetMargins(hCtl, horiz_text_margin, vert_text_margin);
    CustomTextBox_SetColors(hCtl, light_text_color, dark_back_color);
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_FILTER_CMDTICKERGO, L"GO",
        COLOR_BLACK, COLOR_BLUE, COLOR_BLUE, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_center, 0, 0, 0, 0);
    CustomLabel_SetMousePointer(hCtl, CustomLabelPointer::hand, CustomLabelPointer::hand);
    CustomLabel_SetFont(hCtl, font_name, font_size, true);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);

    CustomLabel_SimpleLabel(hwnd, IDC_FILTER_LBLDATEFILTER, L"Date Filter",
        COLOR_WHITEDARK, COLOR_BLACK);
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_FILTER_TRANSDATE, L"7 days",
        COLOR_WHITEDARK, COLOR_GRAYMEDIUM, COLOR_GRAYMEDIUM, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_left, 0, 0, 0, 0);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);
    CustomLabel_SetMousePointer(hCtl, CustomLabelPointer::hand, CustomLabelPointer::hand);
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_FILTER_CMDTRANSDATE, GLYPH_DROPDOWN,
        COLOR_WHITEDARK, COLOR_GRAYMEDIUM, COLOR_GRAYLIGHT, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_center, 0, 0, 0, 0);
    CustomLabel_SetFont(hCtl, font_name, font_size, true);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);

    CustomLabel_SimpleLabel(hwnd, IDC_FILTER_LBLSTARTDATE, L"Start Date",
        COLOR_WHITEDARK, COLOR_BLACK);
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_FILTER_STARTDATE, L"",
        COLOR_WHITEDARK, COLOR_GRAYMEDIUM, COLOR_GRAYMEDIUM, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_left, 0, 0, 0, 0);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);
    CustomLabel_SetMousePointer(hCtl, CustomLabelPointer::hand, CustomLabelPointer::hand);
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_FILTER_CMDSTARTDATE, GLYPH_DROPDOWN,
        COLOR_WHITEDARK, COLOR_GRAYMEDIUM, COLOR_GRAYLIGHT, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_center, 0, 0, 0, 0);
    CustomLabel_SetFont(hCtl, font_name, font_size, true);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);

    CustomLabel_SimpleLabel(hwnd, IDC_FILTER_LBLENDDATE, L"End Date",
        COLOR_WHITEDARK, COLOR_BLACK);
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_FILTER_ENDDATE, L"",
        COLOR_WHITEDARK, COLOR_GRAYMEDIUM, COLOR_GRAYMEDIUM, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_left, 0, 0, 0, 0);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);
    CustomLabel_SetMousePointer(hCtl, CustomLabelPointer::hand, CustomLabelPointer::hand);
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_FILTER_CMDENDDATE, GLYPH_DROPDOWN,
        COLOR_WHITEDARK, COLOR_GRAYMEDIUM, COLOR_GRAYLIGHT, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_center, 0, 0, 0, 0);
    CustomLabel_SetFont(hCtl, font_name, font_size, true);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);

    CustomLabel_SimpleLabel(hwnd, IDC_FILTER_LBLCATEGORYFILTER, L"Category Filter",
        COLOR_WHITEDARK, COLOR_BLACK);
    hCtl = CreateCategoryControl(hwnd, IDC_FILTER_CATEGORY, 0, 0, CATEGORY_ALL, true);

    // Set the Start & End dates based on the filter type.
    SetStartEndDates(hwnd);

    return true;
}


// ========================================================================================
// Assemble the filter data and send message to parent window notifying that the data
// has changed and the grids will need to be updated.
// ========================================================================================
void CFilterPanel::SendDateChangedMessageToParent() {
    // The SetStartEndDates() function will have already set the filter dates so
    // no need to do it again here.
    selected_category = CategoryControl_GetSelectedIndex(CategoryCombo());
    ticker_symbol = AfxTrim(CustomTextBox_GetText(TickerTextBox()));
    SendMessage(GetParent(hWindow), MSG_DATEPICKER_DATECHANGED, 0, 0);
}


// ========================================================================================
// Assemble the filter data and send message to parent window notifying that the data
// has changed and the grids will need to be updated.
// ========================================================================================
void CFilterPanel::SendCategoryChangedMessageToParent() {
    // The SetStartEndDates() function will have already set the filter dates so
    // no need to do it again here.
    selected_category = CategoryControl_GetSelectedIndex(CategoryCombo());
    ticker_symbol = AfxTrim(CustomTextBox_GetText(TickerTextBox()));
    SendMessage(GetParent(hWindow), MSG_CATEGORY_CATEGORYCHANGED, 0, 0);
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CFilterPanel::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        HANDLE_MSG(m_hwnd, WM_CREATE, OnCreate);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, OnPaint);
        HANDLE_MSG(m_hwnd, WM_SIZE, OnSize);

    case WM_KEYDOWN: {
        // We are handling the TAB naviagation ourselves.
        if (wParam == VK_TAB) {
            HWND hFocus = GetFocus();
            HWND hNextCtrl = NULL;
            if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
                hNextCtrl = GetNextDlgTabItem(m_hwnd, hFocus, true);
            }
            else {
                hNextCtrl = GetNextDlgTabItem(m_hwnd, hFocus, false);
            }
            SetFocus(hNextCtrl);
            return true;
        }
        return 0;
    }

    case WM_KEYUP: {
        // Handle ENTER key if pressed in the Ticker textbox.
        if (GetFocus() == GetDlgItem(TickerTextBox(), 100)) {
            if (wParam == 13) {
                // Show underlying Transactions or ClosedTrades 
                SendDateChangedMessageToParent();
                return true;
            }
        }
        return 0;
    }

    case MSG_TRANSPANEL_SHOWLISTBOXITEM: {
        //ShowListBoxItem((int)wParam);
        return 0;
    }

    case MSG_CATEGORY_CATEGORYCHANGED: {
        // Show underlying Transactions or ClosedTrades 
        SendCategoryChangedMessageToParent();
        return 0;
    }

    case MSG_CUSTOMLABEL_CLICK: {
        HWND hCtl = (HWND)lParam;
        int ctrl_id = (int)wParam;

        if (!hCtl) return 0;

        if (ctrl_id == IDC_FILTER_CMDTICKERGO) {
            // Show underlying Transactions or ClosedTrades 
            SendDateChangedMessageToParent();
        }

        if (ctrl_id == IDC_FILTER_CMDTRANSDATE || ctrl_id == IDC_FILTER_TRANSDATE) {
            // Clicked on the Date Filter dropdown or label itself
            std::vector<CCustomPopupMenuItem> items;
            for (int i = (int)TransDateFilterType::Today; i <= (int)TransDateFilterType::Custom; ++i) {
                items.push_back({ GetFilterDescription(i), i, false });
            }

            // Position the popup menu immediately under the control that was clicked on
            int top_offset = AfxScaleY(1);
            RECT rc{}; GetWindowRect(TransDateCombo(), &rc);
            POINT pt{ rc.left, rc.bottom + top_offset};
            int selected = CustomPopupMenu.Show(hWindow, items, (int)selected_transdate, pt.x, pt.y);

            if (selected != -1 && selected != (int)selected_transdate) {
                // Set selected_transdate prior to calling SetStartEndDates()
                selected_transdate = (TransDateFilterType)selected;  
                SetStartEndDates(m_hwnd);
                CustomLabel_SetText(TransDateCombo(), GetFilterDescription(selected));
                SendDateChangedMessageToParent();
            }
        }

        if (ctrl_id == IDC_FILTER_CMDSTARTDATE || ctrl_id == IDC_FILTER_STARTDATE) {
            // Clicked on the Start Date dropdown or label itself
            CalendarReturn calendar_result = Calendar_CreateDatePicker(m_hwnd, StartDateCombo(), filter_start_date, 1);

            if (calendar_result.exit_code != -1) {
                // Set selected_transdate prior to calling SetStartEndDates()
                selected_transdate = TransDateFilterType::Custom;
                CustomLabel_SetText(TransDateCombo(), GetFilterDescription((int)TransDateFilterType::Custom));
                filter_start_date = calendar_result.iso_date;
                CustomLabel_SetText(StartDateCombo(), AfxLongDate(calendar_result.iso_date));
                SendDateChangedMessageToParent();
            }
        }

        if (ctrl_id == IDC_FILTER_CMDENDDATE || ctrl_id == IDC_FILTER_ENDDATE) {
            // Clicked on the End Date dropdown or label itself
            CalendarReturn calendar_result = Calendar_CreateDatePicker(m_hwnd, EndDateCombo(), filter_end_date, 1);

            if (calendar_result.exit_code != -1) {
                // Set selected_transdate prior to calling SetStartEndDates()
                selected_transdate = TransDateFilterType::Custom;
                CustomLabel_SetText(TransDateCombo(), GetFilterDescription((int)TransDateFilterType::Custom));
                filter_end_date = calendar_result.iso_date;
                CustomLabel_SetText(EndDateCombo(), AfxLongDate(calendar_result.iso_date));
                SendDateChangedMessageToParent();
            }
        }


        return 0;
    }

    }

    return DefWindowProc(m_hwnd, msg, wParam, lParam);
}


// ========================================================================================
// Create FilterPanel child control 
// ========================================================================================
HWND CFilterPanel::CreateFilterPanel(HWND hwndParent) {
    HWND hCtl = Create(hwndParent, L"", 0, 0, 500, 80,
        WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_CONTROLPARENT | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);
    
    return hCtl;
}


