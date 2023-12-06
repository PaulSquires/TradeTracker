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
	text_only,
	image_only,
	image_and_text,
	line_horizontal,
	line_vertical
};

enum class CustomLabelAlignment
{
	// Text alignment
	middle_center,
	middle_left,
	middle_right,
	top_center,
	top_left,
	top_right,
	bottom_center,
	bottom_left,
	bottom_right
};

enum class CustomLabelPointer
{
	arrow,
	hand
};


class CustomLabel
{
private:
	HDC m_memDC = NULL;           // Double buffering
	HBITMAP m_hbit = NULL;        // Double buffering
	RECT m_rcClient{};
	float m_rx = 0;
	float m_ry = 0;
	bool m_is_hot = false;

public:
	HWND hWindow = NULL;
	HWND hParent = NULL;
	HINSTANCE hInst = NULL;
	HWND hToolTip = NULL;
	std::wstring tooltip_text{};
	std::wstring user_data{};
	int user_data_int = 0;
	bool allow_tab_stop = false;

	int CtrlId = 0;
	CustomLabelType CtrlType = CustomLabelType::text_only;
	bool hot_test_enable = false;
	DWORD back_color{};
	DWORD back_color_hot{};
	DWORD back_color_button_down{};
	bool LButtonDown = false;

	// Lines
	REAL line_width = 1;
	DWORD line_color{};

	// Border
	bool border_visible = false;
	REAL border_width = 0;
	DWORD border_color{};
	DWORD border_color_hot{};

	// Images
	Bitmap* pImage = nullptr;
	Bitmap* pImageHot = nullptr;
	int image_width = 0;
	int image_height = 0;
	int image_offset_left = 0;
	int image_offset_top = 0;

	// Text General
	CustomLabelAlignment text_alignment = CustomLabelAlignment::middle_center;
	int text_offset_left = 0;
	int text_offset_top = 0;

	// Text Normal
	std::wstring text{};
	std::wstring font_name{};
	REAL font_size = 0;
	bool font_bold = false;
	bool font_italic = false;
	bool font_underline = false;
	DWORD text_color{};
	CustomLabelPointer pointer = CustomLabelPointer::arrow;

	// Text Hot
	std::wstring text_hot{};
	std::wstring font_name_hot{};
	REAL font_size_hot = 0;
	bool font_bold_hot = false;
	bool font_italic_hot = false;
	bool font_underline_hot = false;
	DWORD text_color_hot{};
	CustomLabelPointer pointer_hot = CustomLabelPointer::hand;

	void SetTextAlignment(StringFormat* stringF);
	void StartDoubleBuffering(HDC hdc);
	void DrawImageInBuffer();
	void DrawTextInBuffer();
	void DrawLabelInBuffer();
	void DrawBordersInBuffer();
	void EndDoubleBuffering(HDC hdc);

};


CustomLabel* CustomLabel_GetOptions(HWND hCtrl);
int CustomLabel_SetOptions(HWND hCtrl, CustomLabel* pData);
void CustomLabel_SetText(HWND hCtrl, std::wstring text);
std::wstring CustomLabel_GetText(HWND hCtrl);
void CustomLabel_SetToolTip(HWND hCtrl, std::wstring text);
void CustomLabel_SetTextColor(HWND hCtrl, DWORD text_color);
void CustomLabel_SetTextColorHot(HWND hCtrl, DWORD text_colorHot);
void CustomLabel_SetBackColor(HWND hCtrl, DWORD back_color);
void CustomLabel_SetBackColorHot(HWND hCtrl, DWORD back_color_hot);
void CustomLabel_SetBorderColor(HWND hCtrl, DWORD BorderColor);
void CustomLabel_SetBorderColorHot(HWND hCtrl, DWORD BorderColorHot);
DWORD CustomLabel_GetBackColor(HWND hCtrl);
void CustomLabel_SetFont(HWND hCtrl, std::wstring font_name, int font_size, bool FontBold);
void CustomLabel_SetUserData(HWND hCtrl, std::wstring text);
std::wstring CustomLabel_GetUserData(HWND hCtrl);
void CustomLabel_SetUserDataInt(HWND hCtrl, int value);
int CustomLabel_GetUserDataInt(HWND hCtrl);
void CustomLabel_SetBorder(HWND hCtrl, REAL BorderWidth, DWORD BorderColor, DWORD BorderColorHot);
void CustomLabel_SetTextOffset(HWND hCtrl, int OffsetLeft, int OffsetTop);
void CustomLabel_SetMousePointer(HWND hCtrl, CustomLabelPointer NormalPointer, CustomLabelPointer HotPointer);

HWND CustomLabel_SimpleLabel(HWND hParent, int CtrlId, std::wstring text,
	DWORD text_color, DWORD back_color, CustomLabelAlignment alignment = CustomLabelAlignment::middle_left,
	int nLeft = 0, int nTop = 0, int nWidth = 0, int nHeight = 0);

HWND CustomLabel_HorizontalLine(HWND hParent, int CtrlId, DWORD line_color, DWORD back_color,
	int nLeft = 0, int nTop = 0, int nWidth = 0, int nHeight = 0);

HWND CustomLabel_ButtonLabel(HWND hParent, int CtrlId, std::wstring text,
	DWORD text_color, DWORD back_color, DWORD back_color_hot, DWORD back_color_button_down, DWORD FocusBorderColor,
	CustomLabelAlignment alignment = CustomLabelAlignment::middle_left,
	int nLeft = 0, int nTop = 0, int nWidth = 0, int nHeight = 0);

	
HWND CustomLabel_SimpleImageLabel(HWND hParent, int CtrlId,
	std::wstring image, std::wstring image_hot,
	int image_width, int image_height,
	int nLeft, int nTop, int nWidth, int nHeight);

HWND CreateCustomLabel(HWND hWndParent, LONG_PTR CtrlId, CustomLabelType nCtrlType,
	int nLeft, int nTop, int nWidth, int nHeight);

Gdiplus::Bitmap* LoadImageFromResource(HMODULE hMod, const wchar_t* resid, const wchar_t* restype);

