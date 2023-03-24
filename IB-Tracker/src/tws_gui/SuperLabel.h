#pragma once

#include "framework.h"


// User defined messages
const int MSG_SUPERLABEL_CLICK     = WM_USER + 1000;
const int MSG_SUPERLABEL_MOUSEMOVE = WM_USER + 1001;

#define IDB_GEAR                        103
#define IDB_GEARHOT                     104
#define IDB_LOGO                        105

enum class SuperLabelType
{
	TextOnly,
	ImageOnly,
	ImageAndText,
	LineHorizontal,
	LineVertical
};

enum class SuperLabelAlignment
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

enum class SuperLabelPointer
{
	Arrow,
	Hand
};


class SUPERLABEL_DATA
{
	public:

	HWND hWindow = NULL;
	HWND hParent = NULL;
	HINSTANCE hInst = NULL;
	HWND hToolTip = NULL;
	std::wstring wszToolTip;

	int CtrlId = 0;
	SuperLabelType CtrlType = SuperLabelType::TextOnly;
	bool HotTestEnable = false;
	DWORD BackColor = 0;    
	DWORD BackColorHot = 0; 

	// Selection
	bool SelectionMode = false;
	bool IsSelected = false;
	DWORD SelectorColor = Color::MakeARGB(255, 255, 255, 255);

	// Lines
	REAL LineWidth = 1;
	DWORD LineColor = Color::MakeARGB(255, 0, 0, 0);
	DWORD LineColorHot = Color::MakeARGB(255, 0, 0, 0);

	// Margins
	int MarginLeft = 0;
	int MarginTop = 0;
	int MarginBottom = 0;
	int MarginRight = 0;

	// Border
	bool BorderVisible = false;
	REAL BorderWidth = 1;
	DWORD BorderColor = Color::MakeARGB(255, 0, 0, 0);
	DWORD BorderColorHot = Color::MakeARGB(255, 0, 0, 0);
	int BorderRoundWidth = 0;
	int BorderRoundHeight = 0;

	// Images
	int rcImageId = 0;
	int rcImageHotId = 0;
	Bitmap* pImage = nullptr;
	Bitmap* pImageHot = nullptr;
	int ImageWidth = 0;
	int ImageHeight = 0;
	int ImageOffsetLeft = 0;
	int ImageOffsetTop = 0;

	// Text General
	SuperLabelAlignment TextAlignment = SuperLabelAlignment::MiddleCenter;
	int TextOffsetLeft = 0;
	int TextOffsetTop = 0;
	int TextCharacterExtra = 0;

	// Text Normal
	std::wstring wszText;
	std::wstring wszFontName;
	REAL FontSize = 0;
	bool FontBold = false;
	bool FontItalic = false;
	bool FontUnderline = false;
	DWORD TextColor = Color::MakeARGB(255, 0, 0, 0);
	SuperLabelPointer Pointer = SuperLabelPointer::Arrow;

	// Text Hot
	std::wstring wszTextHot;
	std::wstring wszFontNameHot;
	REAL FontSizeHot = 0;
	bool FontBoldHot = false;
	bool FontItalicHot = false;
	bool FontUnderlineHot = false;
	DWORD TextColorHot = Color::MakeARGB(255, 0, 0, 0);
	SuperLabelPointer PointerHot = SuperLabelPointer::Hand;
};


SUPERLABEL_DATA* SuperLabel_GetOptions(HWND hCtrl);
int SuperLabel_SetOptions(HWND hCtrl, SUPERLABEL_DATA* pData);

HWND CreateSuperLabel(HWND hWndParent, LONG_PTR CtrlId,	SuperLabelType nCtrlType, std::wstring wszText,
	int nLeft, int nTop, int nWidth, int nHeight);

Gdiplus::Bitmap* LoadImageFromResource(HMODULE hMod, const wchar_t* resid, const wchar_t* restype);

