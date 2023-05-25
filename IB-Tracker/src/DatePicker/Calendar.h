#pragma once

#include "..\Utilities\CWindowBase.h"


class CCalendar: public CWindowBase<CCalendar>
{
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

};

enum class CalendarPickerReturnType
{
    ShortDate,    // Mar 1
    LongDate,     // Mar 1, 2023
    ISODate       // 2023-03-01
};

const int IDC_CALENDAR_CALENDAR = 100;

const int CALENDAR_WIDTH = 210;
const int CALENDAR_HEIGHT = 310;

HWND Calendar_CreateDatePicker(
    HWND hParent, HWND hParentCtl, std::wstring wszDate, CalendarPickerReturnType DateReturnType);

