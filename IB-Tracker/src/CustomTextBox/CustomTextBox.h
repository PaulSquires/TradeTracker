#pragma once

#include "..\Themes\Themes.h"
#include "..\Utilities\AfxWin.h"
#include "..\Utilities\UserMessages.h"


enum class CustomTextBoxBorder
{
	BorderBox,
	BorderUnderline
};


class CustomTextBox
{
private:
	float m_rx = 0;
	float m_ry = 0;

public:
	HWND hWindow = NULL;
	HWND hParent = NULL;
	HWND hTextBox = NULL;    // the actual child textbox
	HINSTANCE hInst = NULL;

	int CtrlId = 0;

	COLORREF BackColor{};
	HBRUSH hBackBrush = NULL;

	// Text
	std::wstring wszText;
	std::wstring wszFontName;
	int FontSize = 0;
	HFONT hFontText = NULL;
	int TextMargin = 0;
	COLORREF TextColor{};
	int Alignment;

	// Numeric
	bool isNumeric = false;
	int NumDecimals = 0;
	bool AllowNegative = true;

	// Border
	CustomTextBoxBorder BorderStyle;
	int BorderWidth = 1;
	COLORREF BorderColor{};
	COLORREF BorderColorFocus = BorderColor;

};


CustomTextBox* CustomTextBox_GetOptions(HWND hCtrl);
int CustomTextBox_SetOptions(HWND hCtrl, CustomTextBox* pData);
void CustomTextBox_SetText(HWND hCtrl, std::wstring wszText);
void CustomTextBox_SetFont(HWND hCtrl, std::wstring wszFontName, int FontSize);
void CustomTextBox_SetBorderAttributes(
	HWND hCtrl, int BorderWidth, COLORREF clrGotFocus, COLORREF clrLostFocus, CustomTextBoxBorder BorderStyle);
void CustomTextBox_SetNumericAttributes(HWND hCtrl, int DecimalPlaces, bool AllowNegative);

HWND CreateCustomTextBox(HWND hWndParent, LONG_PTR CtrlId, 
	int Alignment, std::wstring wszText, int nLeft, int nTop, int nWidth, int nHeight);

