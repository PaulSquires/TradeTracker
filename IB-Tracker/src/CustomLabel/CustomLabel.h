/*

MIT License

Copyright(c) 2023 Paul Squires

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

#include "Utilities/AfxWin.h"
#include "Utilities/UserMessages.h"


#define IDB_LOGO 105

enum class CustomLabelType
{
	TextOnly,
	ImageOnly,
	ImageAndText,
	LineHorizontal,
	LineVertical
};

enum class CustomLabelAlignment
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

enum class CustomLabelPointer
{
	Arrow,
	Hand
};


class CustomLabel
{
private:
	HDC m_memDC = NULL;           // Double buffering
	HBITMAP m_hbit = NULL;        // Double buffering
	RECT m_rcClient{};
	float m_rx = 0;
	float m_ry = 0;
	bool m_bIsHot = false;

public:
	HWND hWindow = NULL;
	HWND hParent = NULL;
	HINSTANCE hInst = NULL;
	HWND hToolTip = NULL;
	std::wstring wszToolTip{};
	std::wstring UserData{};
	int UserDataInt = 0;

	int CtrlId = 0;
	CustomLabelType CtrlType = CustomLabelType::TextOnly;
	bool HotTestEnable = false;
	DWORD BackColor{};
	DWORD BackColorHot{};
	DWORD BackColorButtonDown{};
	bool LButtonDown = false;

	// Selection
	bool AllowSelect = false;
	bool IsSelected = false;
	bool AllowNotch = false;
	DWORD SelectorColor{};
	DWORD BackColorSelected{};

	// Lines
	REAL LineWidth = 1;
	DWORD LineColor{};
	DWORD LineColorHot{};

	// Margins
	int MarginLeft = 0;
	int MarginTop = 0;
	int MarginBottom = 0;
	int MarginRight = 0;

	// Border
	bool BorderVisible = false;
	REAL BorderWidth = 0;
	DWORD BorderColor{};
	DWORD BorderColorHot{};
	int BorderRoundWidth = 0;
	int BorderRoundHeight = 0;

	// Images
	Bitmap* pImage = nullptr;
	Bitmap* pImageHot = nullptr;
	int ImageWidth = 0;
	int ImageHeight = 0;
	int ImageOffsetLeft = 0;
	int ImageOffsetTop = 0;

	// Text General
	CustomLabelAlignment TextAlignment = CustomLabelAlignment::MiddleCenter;
	int TextOffsetLeft = 0;
	int TextOffsetTop = 0;
	int TextCharacterExtra = 0;

	// Text Normal
	std::wstring wszText{};
	std::wstring wszFontName{};
	REAL FontSize = 0;
	bool FontBold = false;
	bool FontItalic = false;
	bool FontUnderline = false;
	DWORD TextColor{};
	CustomLabelPointer Pointer = CustomLabelPointer::Arrow;

	// Text Hot
	std::wstring wszTextHot{};
	std::wstring wszFontNameHot{};
	REAL FontSizeHot = 0;
	bool FontBoldHot = false;
	bool FontItalicHot = false;
	bool FontUnderlineHot = false;
	DWORD TextColorHot{};
	CustomLabelPointer PointerHot = CustomLabelPointer::Hand;

	void SetTextAlignment(StringFormat* stringF);
	void StartDoubleBuffering(HDC hdc);
	void DrawImageInBuffer();
	void DrawTextInBuffer();
	void DrawLabelInBuffer();
	void DrawNotchInBuffer();
	void DrawBordersInBuffer();
	void EndDoubleBuffering(HDC hdc);

};


CustomLabel* CustomLabel_GetOptions(HWND hCtrl);
int CustomLabel_SetOptions(HWND hCtrl, CustomLabel* pData);
void CustomLabel_SetText(HWND hCtrl, std::wstring wszText);
std::wstring CustomLabel_GetText(HWND hCtrl);
void CustomLabel_SetTextColor(HWND hCtrl, DWORD TextColor);
void CustomLabel_SetTextColorHot(HWND hCtrl, DWORD TextColorHot);
void CustomLabel_SetBackColor(HWND hCtrl, DWORD BackColor);
void CustomLabel_SetBackColorHot(HWND hCtrl, DWORD BackColorHot);
DWORD CustomLabel_GetBackColor(HWND hCtrl);
void CustomLabel_Select(HWND hCtrl, bool IsSelected);
void CustomLabel_SetFont(HWND hCtrl, std::wstring wszFontName, int FontSize, bool FontBold);
void CustomLabel_SetUserData(HWND hCtrl, std::wstring wszText);
std::wstring CustomLabel_GetUserData(HWND hCtrl);
void CustomLabel_SetUserDataInt(HWND hCtrl, int value);
int CustomLabel_GetUserDataInt(HWND hCtrl);
void CustomLabel_SetBorder(HWND hCtrl, REAL BorderWidth, DWORD BorderColor, DWORD BorderColorHot);
void CustomLabel_SetTextOffset(HWND hCtrl, int OffsetLeft, int OffsetTop);
void CustomLabel_SetMousePointer(HWND hCtrl, CustomLabelPointer NormalPointer, CustomLabelPointer HotPointer);

HWND CustomLabel_SimpleLabel(HWND hParent, int CtrlId, std::wstring wszText,
	DWORD TextColor, DWORD BackColor, CustomLabelAlignment alignment = CustomLabelAlignment::MiddleLeft,
	int nLeft = 0, int nTop = 0, int nWidth = 0, int nHeight = 0);

HWND CustomLabel_ButtonLabel(HWND hParent, int CtrlId, std::wstring wszText,
	DWORD TextColor, DWORD BackColor, DWORD BackColorHot, DWORD BackColorButtonDown,
	CustomLabelAlignment alignment = CustomLabelAlignment::MiddleLeft,
	int nLeft = 0, int nTop = 0, int nWidth = 0, int nHeight = 0);

	
HWND CustomLabel_SimpleImageLabel(HWND hParent, int CtrlId,
	std::wstring wszImage, std::wstring wszImageHot,
	int ImageWidth, int ImageHeight,
	int nLeft, int nTop, int nWidth, int nHeight);

HWND CreateCustomLabel(HWND hWndParent, LONG_PTR CtrlId, CustomLabelType nCtrlType,
	int nLeft, int nTop, int nWidth, int nHeight);

Gdiplus::Bitmap* LoadImageFromResource(HMODULE hMod, const wchar_t* resid, const wchar_t* restype);

