#pragma once

#include "..\Utilities\CWindowBase.h"


class CDatePicker: public CWindowBase<CDatePicker>
{
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

};

enum class DatePickerReturnType 
{
    ShortDate,    // Mar 1
    LongDate,     // Mar 1, 2023
    ISODate       // 2023-03-01
};

const int IDC_DATEPICKER_MONTHLISTBOX = 100;
const int IDC_DATEPICKER_DAYLISTBOX = 101;
const int IDC_DATEPICKER_YEARLISTBOX = 102;

const int IDC_DATEPICKER_MOVEUP = 103;
const int IDC_DATEPICKER_MOVEDOWN = 104;

const int IDC_DATEPICKER_ACCEPT = 109;
const int IDC_DATEPICKER_CANCEL = 110;

const int DATEPICKER_LISTBOX_ROWHEIGHT = 28;
const int DATEPICKER_LISTBOX_VISIBLELINES = 9;

const int DATEPICKER_PANEL_WIDTH = 222;
const int DATEPICKER_PANEL_MONTHWIDTH = 100;
const int DATEPICKER_PANEL_DAYWIDTH = 60;
const int DATEPICKER_PANEL_YEARWIDTH = 60;

HWND DatePicker_CreateDatePicker(
    HWND hParent, HWND hParentCtl, std::wstring wszDate, DatePickerReturnType DateReturnType);

