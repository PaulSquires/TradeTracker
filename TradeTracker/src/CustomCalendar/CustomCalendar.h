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

#pragma once

#include "Utilities/CWindowBase.h"
#include "Utilities/Colors.h"


struct CalendarReturn {
    std::wstring iso_date;
    int exit_code = -1;
};

struct CalendarCell {
    bool no_data{ false };
    int day{};
    int month{};
    int year{};
    RectF rc{};
    bool is_gray{};
};

// 0: Previous month
// 1: Current month
// 2: Next month
// 3: Current month +2
struct MonthData {
    int year{};
    int month{};
    int day{ 1 };
    int num_days_in_month{};
    int first_weekday_of_month{};
    int month_start_day{};
};



class CCustomCalendar: public CWindowBase<CCustomCalendar> {
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

    CalendarReturn Show(HWND hParent, HWND hParentCtl, std::wstring initial_selected_date, int calendar_count);

private:
    bool OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
    void OnPaint(HWND hwnd);
    bool OnEraseBkgnd(HWND hwnd, HDC hdc);

    HWND hParent = NULL;
    HWND hWindow = NULL;

    int num_calendars = 1;
    int cells_per_calendar = 42;      // max 6 weeks x 7 days per calendar
    std::vector<CalendarCell> cells{};   

    std::wstring today_iso_text;      // Today: 2024-01-05

    int selected_month = 0;
    int selected_year = 0;
    int selected_day = 0;

    int current_month = 0;
    int current_year = 0;
    int current_day = 0;

    std::wstring font_name = AfxGetDefaultFont();
    std::wstring font_name_symbol = L"Segoe UI Symbol";

    REAL font_size = 9;

    DWORD text_color = COLOR_WHITELIGHT;
    DWORD text_color_hot = COLOR_WHITELIGHT;
    DWORD text_color_dim = COLOR_WHITEDARK;
    
    DWORD back_color = COLOR_GRAYMEDIUM;
    DWORD back_color_hot = COLOR_BLUE;
    DWORD back_color_selected = COLOR_GRAYLIGHT;
    
    DWORD border_color = COLOR_GRAYLIGHT;
    DWORD border_color_selected = COLOR_BLUE;
    REAL  pen_width = 1;

    RectF left_arrow_rect;
    RectF right_arrow_rect;
    RectF today_iso_text_rect;

    void IncrementDate();
    void DecrementDate();
    void SetMonthDataVector(std::vector<MonthData>& m);

    void DrawCell(CalendarCell& cell, Graphics& graphics, Font& font, StringFormat& stringF);

};


constexpr int IDC_CUSTOMCALENDAR_CALENDAR = 100;

extern CCustomCalendar CustomCalendar;
extern HWND HWND_CUSTOMCALENDAR;

constexpr int MSG_CUSTOMCALENDAR_DOSELECTED = WM_USER + 1022;
constexpr int MSG_CUSTOMCALENDAR_CANCEL = WM_USER + 1023;
