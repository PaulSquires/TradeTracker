#pragma once

#include "framework.h"
#include <string>

// User defined messages
const int MSG_SUPERLABEL_CLICK     = WM_USER + 1000;
const int MSG_SUPERLABEL_MOUSEMOVE = WM_USER + 1001;


enum SuperLabelType
{
	TextOnly,
	ImageOnly,
	ImageAndText,
	LineHorizontal,
	LineVertical
};

enum SuperLabelAlignment
{
	// Text alignment
	MiddleCenter,
	MiddleLeft,
	MiddleRight,
	TopCenter,
	TopLeft,
	TopRight,
	BottomCenter,
	BottomLeft,
	BottomRight
};

struct SUPERLABEL_DATA
{
	HWND hWindow = NULL;
	HWND hParent = NULL;
	HINSTANCE hInst = NULL;
	HWND hToolTip = NULL;
	std::wstring wszToolTip;

	int CtrlId = 0;
	SuperLabelType CtrlType;
	bool HotTestEnable = false;
	COLORREF BackColor = GetSysColor(COLOR_3DFACE);
	COLORREF BackColorHot = GetSysColor(COLOR_3DFACE);

	// Selection
	bool SelectionMode = false;
	bool IsSelected = false;
	COLORREF SelectorColor = RGB(251, 253, 254);

	// Lines
	int LineWidth = 1;
	int LineStyle = PS_SOLID;
	COLORREF LineColor = RGB(61, 156, 228);
	COLORREF LineColorHot = RGB(61, 156, 228);

	// Margins
	int MarginLeft = 0;
	int MarginTop = 0;
	int MarginBottom = 0;
	int MarginRight = 0;

	// Border
	bool BorderVisible = false;
	int BorderWidth = 1;
	int BorderStyle = PS_SOLID;
	COLORREF BorderColor = GetSysColor(COLOR_3DFACE);
	COLORREF BorderColorHot = GetSysColor(COLOR_3DFACE);
	int BorderRoundWidth = 0;
	int BorderRoundHeight = 0;

	// Images
	std::wstring wszImage;
	std::wstring wszImageHot;
	//pImage             As GpImage Ptr  ' Image object pointer
	//pImageHot          As GpImage Ptr  ' Image object pointer
	int ImageWidth = 0;
	int ImageHeight = 0;
	int ImageOffsetLeft = 0;
	int ImageOffsetTop = 0;

	// Text General
	SuperLabelAlignment TextAlignment = MiddleLeft;
	int TextOffsetLeft = 0;
	int TextOffsetTop = 0;
	int TextCharacterExtra = 0;

	// Text Normal
	std::wstring wszText;
	std::wstring wszFontName;
	int FontSize = 0;
	int FontWeight = FW_NORMAL;
	bool FontItalic = false;
	bool FontUnderline = false;
	COLORREF TextColor = RGB(0, 0, 0);

	// Text Hot
	std::wstring wszTextHot;
	std::wstring wszFontNameHot;
	int FontSizeHot = 0;
	int FontWeightHot = FW_NORMAL;
	bool FontItalicHot = false;
	bool FontUnderlineHot = false;
	COLORREF TextColorHot = RGB(0, 0, 0);

};
