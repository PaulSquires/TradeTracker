#pragma once

#include "..\Themes\Themes.h"
#include "..\Utilities\AfxWin.h"
#include "..\Utilities\UserMessages.h"


enum class CustomTextBoxBorder
{
	BorderNone,
	BorderBox,
	BorderUnderline
};

enum class CustomTextBoxNegative
{
	Allow,
	Disallow
};

enum class CustomTextBoxFormatting
{
	Allow,
	Disallow
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
	std::wstring UserData;

	int CtrlId = 0;

	COLORREF BackColor{};
	HBRUSH hBackBrush = NULL;

	// Text
	std::wstring wszText;
	std::wstring wszFontName;
	int FontSize = 0;
	HFONT hFontText = NULL;
	int HTextMargin = 0;
	int VTextMargin = 0;
	COLORREF TextColor{};
	int Alignment = ES_LEFT;

	// Numeric
	bool isNumeric = false;
	int NumDecimals = 0;
	CustomTextBoxNegative AllowNegative = CustomTextBoxNegative::Disallow;
	CustomTextBoxFormatting AllowFormatting = CustomTextBoxFormatting::Disallow;

	// Border
	CustomTextBoxBorder BorderStyle = CustomTextBoxBorder::BorderNone;
	int BorderWidth = 0;
	COLORREF BorderColor{};
	COLORREF BorderColorFocus = BorderColor;

};


CustomTextBox* CustomTextBox_GetOptions(HWND hCtrl);
int CustomTextBox_SetOptions(HWND hCtrl, CustomTextBox* pData);
void CustomTextBox_SetText(HWND hCtrl, std::wstring wszText);
void CustomTextBox_SetFont(HWND hCtrl, std::wstring wszFontName, int FontSize);
void CustomTextBox_SetBorderAttributes(
	HWND hCtrl, int BorderWidth, COLORREF clrGotFocus, COLORREF clrLostFocus, CustomTextBoxBorder BorderStyle);
void CustomTextBox_SetNumericAttributes(
	HWND hCtrl, int DecimalPlaces, CustomTextBoxNegative AllowNegative, CustomTextBoxFormatting AllowFormatting);
void CustomTextBox_SetColors(HWND hCtrl, COLORREF TextColor, COLORREF BackColor);
void CustomTextBox_SetMargins(HWND hCtrl, int HTextMargin, int VTextMargin);
void CustomTextBox_SetUserData(HWND hCtrl, std::wstring UserData);
std::wstring CustomTextBox_GetUserData(HWND hCtrl);


HWND CreateCustomTextBox(HWND hWndParent, LONG_PTR CtrlId, 
	int Alignment, std::wstring wszText, int nLeft, int nTop, int nWidth, int nHeight);

