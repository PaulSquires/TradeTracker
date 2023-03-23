#pragma once

#include "framework.h"


// User defined messages
const int MSG_SUPERLABEL_CLICK     = WM_USER + 1000;
const int MSG_SUPERLABEL_MOUSEMOVE = WM_USER + 1001;


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
	DWORD SelectorColor = 0;

	// Lines
	int LineWidth = 1;
	int LineStyle = PS_SOLID;
	DWORD LineColor = 0;  
	DWORD LineColorHot = 0;

	// Margins
	int MarginLeft = 0;
	int MarginTop = 0;
	int MarginBottom = 0;
	int MarginRight = 0;

	// Border
	bool BorderVisible = false;
	Gdiplus::REAL BorderWidth = 1;
	int BorderStyle = PS_SOLID;
	DWORD BorderColor = 0; 
	DWORD BorderColorHot = 0;
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
	SuperLabelAlignment TextAlignment = SuperLabelAlignment::MiddleLeft;
	int TextOffsetLeft = 0;
	int TextOffsetTop = 0;
	int TextCharacterExtra = 0;

	// Text Normal
	std::wstring wszText;
	std::wstring wszFontName;
	Gdiplus::REAL FontSize = 0;
	int FontWeight = FW_NORMAL;
	bool FontItalic = false;
	bool FontUnderline = false;
	DWORD TextColor = 0;   
	SuperLabelPointer Pointer = SuperLabelPointer::Arrow;

	// Text Hot
	std::wstring wszTextHot;
	std::wstring wszFontNameHot;
	Gdiplus::REAL FontSizeHot = 0;
	int FontWeightHot = FW_NORMAL;
	bool FontItalicHot = false;
	bool FontUnderlineHot = false;
	DWORD TextColorHot = 0;   
	SuperLabelPointer PointerHot = SuperLabelPointer::Hand;
};


SUPERLABEL_DATA* SuperLabel_GetOptions(HWND hCtrl);
int SuperLabel_SetOptions(HWND hCtrl, SUPERLABEL_DATA* pData);

HWND CreateSuperLabel(HWND hWndParent, LONG_PTR CtrlId,	SuperLabelType nCtrlType, std::wstring wszText,
	int nLeft, int nTop, int nWidth, int nHeight);

