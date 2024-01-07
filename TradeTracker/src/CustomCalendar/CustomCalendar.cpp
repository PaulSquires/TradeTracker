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

#include "MainWindow/MainWindow.h"
#include "Utilities/UserMessages.h"

#include "CustomCalendar.h"


HWND HWND_CUSTOMCALENDAR = NULL;

CalendarReturn calendar_result{};

CCustomCalendar CustomCalendar;

HHOOK hCalanderPopupMouseHook = nullptr;



// ========================================================================================
// Increment the Calendar's date
// ========================================================================================
void CCustomCalendar::IncrementDate() {
    current_month++;
    current_day = 1;
    if (current_month > 12) {
        current_year++;
        current_month = 1;
    }
    AfxRedrawWindow(hWindow);
}


// ========================================================================================
// Decrement the Calendar's date
// ========================================================================================
void CCustomCalendar::DecrementDate() {
    current_month--;
    current_day = 1;
    if (current_month < 1) {
        current_year--;
        current_month = 12;
    }
    AfxRedrawWindow(hWindow);
}



// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: CustomCalendar
// ========================================================================================
bool CCustomCalendar::OnEraseBkgnd(HWND hwnd, HDC hdc) {
    // Handle all of the painting in WM_PAINT
    return true;
}



// ========================================================================================
// Load the MonthData vector to contain all of the necessary data to create
// and display the current, previous, next, and current+2 calendar data.
// ========================================================================================
void CCustomCalendar::SetMonthDataVector(std::vector<MonthData>& m) {
    
    for (int i = -1; i < 3; ++i) {
        MonthData mdata;
        
        mdata.month = current_month + i;
        mdata.year = current_year;
        if (mdata.month < 1) { mdata.month = 12; mdata.year--; }
        if (mdata.month > 12) { mdata.month -= 12; mdata.year++; }

        mdata.num_days_in_month = AfxDaysInMonth(mdata.month, mdata.year);
        mdata.first_weekday_of_month = AfxDateWeekday(1, mdata.month, mdata.year);

        m.push_back(mdata);
    }

    m.at(0).month_start_day = m.at(0).num_days_in_month - m.at(1).first_weekday_of_month + 1;
}


// ========================================================================================
// Draw one cell in the Calendar
// ========================================================================================
void CCustomCalendar::DrawCell(CalendarCell& cell, Graphics& graphics, Font& font, StringFormat& stringF) {
    
    if (cell.no_data) return;

    POINT pt; GetCursorPos(&pt);
    MapWindowPoints(NULL, hWindow, &pt, 1);

    SolidBrush text_brush(text_color);
    SolidBrush back_brush(back_color);
    
    if (cell.is_gray) text_brush.SetColor(text_color_dim);

    bool is_selected_day = 
        (cell.day == selected_day) && (cell.month == selected_month) && (cell.year == selected_year);

    if (is_selected_day) {
        back_brush.SetColor(back_color_selected);
    }

    bool is_hot = cell.rc.Contains((REAL)pt.x, (REAL)pt.y);
    if (is_hot) {
        text_brush.SetColor(text_color_hot);
        back_brush.SetColor(back_color_hot);
    }

    std::wstring text = std::to_wstring(cell.day);
    stringF.SetAlignment(StringAlignmentFar);
    graphics.FillRectangle(&back_brush, cell.rc);
    graphics.DrawString(text.c_str(), -1, &font, cell.rc, &stringF, &text_brush);

    if (is_selected_day) {
        Pen pen(border_color_selected, pen_width);
        graphics.DrawRectangle(&pen, 
            cell.rc.GetLeft(), cell.rc.GetTop(), 
            (cell.rc.Width - pen_width), (cell.rc.Height - pen_width));
    }

    // If over the Today date string label at the bottom of the calendar then paint it hot
    bool is_today_iso_text_hot = today_iso_text_rect.Contains((REAL)pt.x, (REAL)pt.y);
    text_brush.SetColor(text_color);
    if (is_today_iso_text_hot) {
        text_brush.SetColor(back_color_hot);
    }
    stringF.SetAlignment(StringAlignmentCenter);
    graphics.DrawString(today_iso_text.c_str(), -1, &font, today_iso_text_rect, &stringF, &text_brush);

}


// ========================================================================================
// Process WM_PAINT message for window/dialog: CustomCalendar
// ========================================================================================
void CCustomCalendar::OnPaint(HWND hwnd) {
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);
    RECT rc1 = ps.rcPaint;

    SaveDC(hdc);

    // Double buffering
    HDC memDC = CreateCompatibleDC(hdc);           
    HBITMAP hbit = CreateCompatibleBitmap(hdc, rc1.right, rc1.bottom);        
    SelectBitmap(memDC, hbit);

    Graphics graphics(memDC);

    // Create the background brush
    SolidBrush back_brush(border_color);

    // Paint the background using brush.
    graphics.FillRectangle(&back_brush, (int)rc1.left, (int)rc1.top,
        (int)(rc1.right - rc1.left), (int)(rc1.bottom - rc1.top));

    // Make the working area a little smaller and different color thereby giving us
    // a thin border around the rectangle.
    int margin = AfxScaleX(1);
    InflateRect(&rc1, -margin, -margin);
    back_brush.SetColor(back_color);
    graphics.FillRectangle(&back_brush, (int)rc1.left, (int)rc1.top,
        (int)(rc1.right - rc1.left), (int)(rc1.bottom - rc1.top));

    // Paint the top area containing the directional arrows and month/year text
    RectF rc((REAL)rc1.left, (REAL)rc1.top, (REAL)(rc1.right - rc1.left), (REAL)(rc1.bottom - rc1.top));

    FontFamily   fontFamily(font_name.c_str());
    Font         font(&fontFamily, font_size, FontStyleRegular, Unit::UnitPoint);
    FontFamily   fontFamilySymbol(font_name_symbol.c_str());
    Font         fontSymbol(&fontFamilySymbol, font_size, FontStyleRegular, Unit::UnitPoint);
    SolidBrush   text_brush(text_color);

    StringFormat stringF(StringFormatFlagsNoWrap);
    stringF.SetTrimming(StringTrimmingEllipsisWord);
    graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
    stringF.SetLineAlignment(StringAlignmentCenter);

    REAL line_height_top = (REAL)AfxScaleY(26);
    REAL line_height_bottom = (REAL)AfxScaleY(22);
    
    REAL cell_height = (REAL)AfxScaleY(18);
    REAL cell_width = (REAL)AfxScaleX(28);

    REAL weekday_height = cell_height;

    std::wstring text;

    // Left and Right Arrows
    stringF.SetAlignment(StringAlignmentFar);
    left_arrow_rect = { rc.GetLeft(), rc.GetTop(), cell_width, line_height_top };
    graphics.DrawString(L"\u23F4", -1, &font, left_arrow_rect, &stringF, &text_brush);
    
    stringF.SetAlignment(StringAlignmentNear);
    right_arrow_rect = { rc.GetRight() - cell_width, rc.GetTop(), cell_width, line_height_top };
    graphics.DrawString(L"\u23F5", -1, &font, right_arrow_rect, &stringF, &text_brush);


    // today_iso_text
    stringF.SetAlignment(StringAlignmentCenter);
    today_iso_text_rect = { rc.GetLeft(), rc.GetBottom() - line_height_bottom, rc.Width, line_height_bottom };
    today_iso_text = L"Today: " + AfxCurrentDate();
    DrawCell(cells[0], graphics, font, stringF);

    // Days of the month (previous, current, next)
    REAL left = rc.GetLeft();
    REAL top = rc.GetTop();

    // Load the 4 month calendar data that we may need
    std::vector<MonthData> m;
    SetMonthDataVector(m);

    int col = 0;
    std::wstring iso_date;
    std::wstring month_year_text;     // January, 2024

    CalendarCell cell{};
    cells.clear();

    for (int calnum = 0; calnum < num_calendars; ++calnum) {

        iso_date = AfxMakeISODate(m.at(calnum + 1).year, m.at(calnum + 1).month, m.at(calnum + 1).day);
        month_year_text = AfxGetLongMonthName(iso_date) + L", " + std::to_wstring(AfxGetYear(iso_date));

        // month_year_text
        stringF.SetAlignment(StringAlignmentCenter);
        RectF month_year_text_rect(rc.GetLeft(), top, rc.Width, line_height_top);
        graphics.DrawString(month_year_text.c_str(), -1, &font, month_year_text_rect, &stringF, &text_brush);
        top += line_height_top;

        // Weekday Names
        static const std::wstring day_name_string = L"  Sun  Mon  Tue  Wed  Thu   Fri    Sat";
        stringF.SetAlignment(StringAlignmentNear);
        RectF weekday_name_rect(rc.GetLeft(), top, rc.Width, weekday_height);
        graphics.DrawString(day_name_string.c_str(), -1, &font, weekday_name_rect, &stringF, &text_brush);
        top += weekday_height;

        // Days from previous month
        for (col = 0; col < m.at(calnum + 1).first_weekday_of_month; col++) {
            cell.no_data = false;
            cell.is_gray = true;
            cell.day = m.at(calnum + 0).month_start_day + col;
            cell.month = m.at(calnum + 0).month;
            cell.year = m.at(calnum + 0).year;
            cell.rc = { left + (col * cell_width), top, cell_width, cell_height };
            if (calnum == 1) cell.no_data = true;
            DrawCell(cell, graphics, font, stringF);
            cells.push_back(cell);
        }
    
        // Current months days
        for (int j = 1; j <= m.at(calnum + 1).num_days_in_month; j++) {
            cell.no_data = false;
            cell.is_gray = false;
            cell.day = j;
            cell.month = m.at(calnum + 1).month;
            cell.year = m.at(calnum + 1).year;

            // If the day is a Sat or Sun then also paint it gray
            int day_of_week = AfxDateWeekday(cell.day, cell.month, cell.year);
            if (day_of_week == 0 || day_of_week == 6) cell.is_gray = true;

            cell.rc = { left + (col * cell_width), top, cell_width, cell_height };
            DrawCell(cell, graphics, font, stringF);
            cells.push_back(cell);
            if (++col > 6) {
                top += cell_height;
                col = 0;
            }
        }
    
        // Days from next month
        int next_month_start_day = 1;
        while (cells.size() < (cells_per_calendar * (calnum + 1))) {
            cell.no_data = false;
            cell.is_gray = true;
            cell.day = next_month_start_day;
            cell.month = m.at(calnum + 2).month;
            cell.year = m.at(calnum + 2).year;
            cell.rc = { left + (col * cell_width), top, cell_width, cell_height };
            next_month_start_day++;
            if (calnum == 0 && num_calendars == 2) cell.no_data = true;
            DrawCell(cell, graphics, font, stringF);
            cells.push_back(cell);
            if (++col > 6) {
                if (calnum == 0) {
                    top += cell_height;
                    col = 0;
                }
            }
        }

    }

    // Copy the entire memory bitmap to the main display
    BitBlt(hdc, 0, 0, rc1.right, rc1.bottom, memDC, 0, 0, SRCCOPY);

    // Restore the original state of the DC
    RestoreDC(hdc, -1);

    // Cleanup
    DeleteObject(hbit);
    DeleteDC(memDC);

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: CustomCalendar
// ========================================================================================
bool CCustomCalendar::OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct) {
    HWND_CUSTOMCALENDAR = hwnd;
    hWindow = hwnd;

    return true;
}


// ========================================================================================
// Global mouse hook.
// ========================================================================================
LRESULT CALLBACK CustomCalanderPopupHook(int Code, WPARAM wParam, LPARAM lParam) {
    // messages are defined in a linear way the first being WM_LBUTTONUP up to WM_MBUTTONDBLCLK
    // this subset does not include WM_MOUSEMOVE, WM_MOUSEWHEEL and a few others
    // (Don't handle WM_LBUTTONUP here because the mouse is most likely outside the menu popup
    // at the point this hook is called).
    if (wParam == WM_LBUTTONDOWN) {
        if (HWND_CUSTOMCALENDAR) {
            POINT pt;       GetCursorPos(&pt);
            RECT rcWindow;  GetWindowRect(HWND_CUSTOMCALENDAR, &rcWindow);

            // if the mouse action is outside the menu, hide it. the window procedure will also unset this hook 
            if (!PtInRect(&rcWindow, pt)) {
                DestroyWindow(HWND_CUSTOMCALENDAR);
            }
        }
    }

    return CallNextHookEx(NULL, Code, wParam, lParam);
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CCustomCalendar::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {

    // Create static accumulation variable to collect the data from
    // a series of middle mouse wheel scrolls.
    static int accum_delta = 0;

    switch (msg) {
        HANDLE_MSG(m_hwnd, WM_CREATE, OnCreate);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, OnPaint);

    case WM_DESTROY: {
        // unhook and remove our global mouse hook
        UnhookWindowsHookEx(hCalanderPopupMouseHook);
        hCalanderPopupMouseHook = nullptr;

        // Reset our destroyed variable for future use of the popup
        HWND_CUSTOMCALENDAR = NULL;
        PostQuitMessage(0);
        return 0;
    }

    case WM_MOUSEWHEEL: {
        // Accumulate delta until scroll one line (up +120, down -120). 
        // 120 is the Microsoft default delta
        int zdelta = GET_WHEEL_DELTA_WPARAM(wParam);
        accum_delta += zdelta;
        if (accum_delta >= 120) {     // scroll up
            IncrementDate();
            accum_delta = 0;
        }
        else {
            if (accum_delta <= -120) {     // scroll down 
                DecrementDate();
                accum_delta = 0;
            }
        }
        return 0;
    }

    case WM_MOUSEMOVE: {
        // Repaint in order to show hot cell
        AfxRedrawWindow(m_hwnd);
        return 0;
    }

    case WM_LBUTTONDOWN: {
        POINT pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);

        if (left_arrow_rect.Contains((REAL)pt.x, (REAL)pt.y)) {
            DecrementDate();
            return 0;
        }

        if (right_arrow_rect.Contains((REAL)pt.x, (REAL)pt.y)) {
            IncrementDate();
            return 0;
        }

        return 0;
    }

    case WM_LBUTTONUP: {
        POINT pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);

        if (today_iso_text_rect.Contains((REAL)pt.x, (REAL)pt.y)) {
            calendar_result.exit_code = 0;
            selected_day = AfxLocalDay();
            selected_month = AfxLocalMonth();
            selected_year = AfxLocalYear();
            DestroyWindow(hWindow);
            return 0;
        }

        // Did we click on one of the cells
        for (const auto& cell : cells) {
            if (cell.no_data) continue;
            if (cell.rc.Contains((REAL)pt.x, (REAL)pt.y)) {
                calendar_result.exit_code = 0;
                selected_day = cell.day;
                selected_month = cell.month;
                selected_year = cell.year;
                DestroyWindow(hWindow);
                return 0;
            }
        }

        return 0;
    }

    case WM_MOUSEACTIVATE: {
        return MA_NOACTIVATE;
    }

    }
    
    return DefWindowProc(m_hwnd, msg, wParam, lParam);
}


// ========================================================================================
// Create Calendar control and move it into position under the specified incoming control.
// ========================================================================================
CalendarReturn CCustomCalendar::Show(
    HWND hParent, HWND hParentCtl, std::wstring initial_selected_date, int calendar_count)
{
    calendar_result.iso_date = L"";
    calendar_result.exit_code = -1;

    // If a popup menu is already active and showing then destroy it
    // before showing the new popup menu.
    if (IsWindow(hWindow)) {
        DestroyWindow(hWindow);
    }

    if (initial_selected_date.length() == 0) initial_selected_date = AfxCurrentDate();
    std::wstring iso_date = initial_selected_date;
    selected_day   = AfxGetDay(iso_date);
    selected_month = AfxGetMonth(iso_date);
    selected_year  = AfxGetYear(iso_date);
    current_day    = selected_day;
    current_month  = selected_month;
    current_year   = selected_year;
    num_calendars  = calendar_count;

    int margin = 1;
    RECT rc; GetWindowRect(hParentCtl, &rc);

    int left   = AfxUnScaleX(rc.left);
    int top    = AfxUnScaleY(rc.bottom) + margin;
    int width  = 210;
    int height = (num_calendars == 2) ? 324 : 174;

    cells.reserve(cells_per_calendar * num_calendars);


    Create(hParent, L"", left, top, width, height,
        WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR |
        WS_EX_NOACTIVATE);

    // Set our hook and store the handle in the global variable
    hCalanderPopupMouseHook = SetWindowsHookEx(WH_MOUSE, CustomCalanderPopupHook, 0, GetCurrentThreadId());

    ShowWindow(hWindow, SW_SHOWNA);
    UpdateWindow(hWindow);


    // Call modal message pump and wait for it to end.
    MSG msg{};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!IsWindow(MainWindow.hWindow)) break;

        if (msg.message == WM_KEYUP &&  msg.wParam == VK_ESCAPE) {
            calendar_result.exit_code = -1;
            DestroyWindow(hWindow);
        }

        // Determines whether a message is intended for the specified
        // dialog box and, if it is, processes the message.
        if (!IsDialogMessage(hWindow, &msg)) {
            // Translates virtual-key messages into character messages.
            TranslateMessage(&msg);
            // Dispatches a message to a window procedure.
            DispatchMessage(&msg);
        }
    }

    // exit_code would have already been set to 0 for success, or -1 for cancel.
    // Calling code should check exit_code == -1 to ensure that iso_date is valid.
    calendar_result.iso_date = AfxMakeISODate(selected_year, selected_month, selected_day);

    return calendar_result;
}

