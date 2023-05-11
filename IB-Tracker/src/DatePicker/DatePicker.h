#pragma once

#include "..\Utilities\CWindowBase.h"


class CDatePicker: public CWindowBase<CDatePicker>
{
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

};


const int IDC_DATEPICKER_MONTHLISTBOX = 100;
const int IDC_DATEPICKER_DAYLISTBOX = 101;
const int IDC_DATEPICKER_YEARLISTBOX = 102;

const int IDC_DATEPICKER_MONTHUP = 103;
const int IDC_DATEPICKER_MONTHDOWN = 104;
const int IDC_DATEPICKER_DAYUP = 105;
const int IDC_DATEPICKER_DAYDOWN = 106;
const int IDC_DATEPICKER_YEARUP = 107;
const int IDC_DATEPICKER_YEARDOWN = 108;

const int DATEPICKER_LISTBOX_ROWHEIGHT = 28;
const int DATEPICKER_LISTBOX_VISIBLELINES = 9;
const int DATEPICKER_PANEL_HEIGHT = 300;

const int DATEPICKER_PANEL_WIDTH = 200;
const int DATEPICKER_PANEL_MONTHWIDTH = 100;
const int DATEPICKER_PANEL_DAYWIDTH = 50;
const int DATEPICKER_PANEL_YEARWIDTH = 50;

HWND DatePicker_CreateDatePicker(HWND hParent, HWND hParentCtl);

