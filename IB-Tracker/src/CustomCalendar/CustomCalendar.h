#pragma once

#include "..\Themes\Themes.h"
#include "..\Utilities\AfxWin.h"
#include "..\Utilities\UserMessages.h"


class CustomCalendar
{
private:
	float m_rx = 0;
	float m_ry = 0;

public:
	HWND hWindow = NULL;
	HWND hParent = NULL;
	HWND hMonthCal = NULL;    // the actual child MonthCalendar control
	HINSTANCE hInst = NULL;
	int UserData = 0;

	int CtrlId = 0;

	COLORREF BackColor{};
	HBRUSH hBackBrush = NULL;

	// Text
	std::wstring wszText;
};


CustomCalendar* CustomCalendar_GetOptions(HWND hCtrl);
int CustomCalendar_SetOptions(HWND hCtrl, CustomCalendar* pData);

HWND CreateCustomCalendar(HWND hWndParent, LONG_PTR CtrlId,	std::wstring wszDate);

