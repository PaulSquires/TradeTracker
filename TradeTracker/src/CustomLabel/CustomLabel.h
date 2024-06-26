/*

MIT License

Copyright(c) 2023-2024 Paul Squires

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


enum class CustomLabelType {
	text_only,
	image_only,
	image_and_text,
	line_horizontal,
	line_vertical,
	checkbox
};

enum class CustomLabelAlignment {
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

enum class CustomLabelPointer {
	arrow,
	hand
};


class CustomLabel {
public:
	HWND hWindow = NULL;
	HWND hParent = NULL;
	HINSTANCE hInst = NULL;
	HWND hToolTip = NULL;
	std::wstring tooltip_text{};
	std::wstring user_data{};
	int user_data_int = 0;
	bool allow_tab_stop = false;

	bool is_selected = false;
	DWORD back_color_selected{};
	bool is_selected_underline = false;
	DWORD selected_underline_color{};
	int selected_underline_width = 1;

	int ctrl_id = 0;
	CustomLabelType ctrl_type = CustomLabelType::text_only;
	bool hot_test_enable = false;
	DWORD back_color{};
	DWORD back_color_hot{};
	DWORD back_color_button_down{};
	bool LButtonDown = false;

	// Lines
	REAL line_width = 1;
	DWORD line_color{};

	// Checkbox
	DWORD check_color{};
	DWORD check_back_color{};
	bool is_checked = false;

	// Border
	bool border_visible = false;
	REAL border_width = 0;
	DWORD border_color{};
	DWORD border_color_focus{};

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
	REAL font_size = 9;
	bool font_bold = false;
	bool font_italic = false;
	bool font_underline = false;
	DWORD text_color{};
	CustomLabelPointer pointer = CustomLabelPointer::arrow;

	// Text Hot
	std::wstring text_hot{};
	std::wstring font_name_hot{};
	REAL font_size_hot = 9;
	bool font_bold_hot = false;
	bool font_italic_hot = false;
	bool font_underline_hot = false;
	DWORD text_color_hot{};
	CustomLabelPointer pointer_hot = CustomLabelPointer::arrow;

	void SetTextAlignment(StringFormat* stringF);
	void StartDoubleBuffering(HDC hdc);
	void DrawImageInBuffer();
	void DrawCheckBoxInBuffer();
	void DrawTextInBuffer();
	void DrawLabelInBuffer();
	void DrawBordersInBuffer();
	void EndDoubleBuffering(HDC hdc);

private:
	HDC m_memDC = NULL;           // Double buffering
	HBITMAP m_hbit = NULL;        // Double buffering
	RECT m_rcClient{};
	float m_rx = 0;
	float m_ry = 0;
	bool is_hot = false;
};


CustomLabel* CustomLabel_GetOptions(HWND hCtrl);
int CustomLabel_SetOptions(HWND hCtrl, CustomLabel* pData);
void CustomLabel_SetText(HWND hCtrl, std::wstring text);
std::wstring CustomLabel_GetText(HWND hCtrl);
void CustomLabel_SetHotTesting(HWND hCtrl, bool enable_hot_testing);
void CustomLabel_SetSelected(HWND hCtrl, bool is_selected);
bool CustomLabel_GetSelected(HWND hCtrl);
void CustomLabel_SetSelectedUnderline(HWND hCtrl, bool enable_underline, DWORD underline_color, int line_width);
void CustomLabel_SetToolTip(HWND hCtrl, std::wstring text);
void CustomLabel_SetTextColor(HWND hCtrl, DWORD text_color);
void CustomLabel_SetTextColorHot(HWND hCtrl, DWORD text_colorHot);
void CustomLabel_SetBackColor(HWND hCtrl, DWORD back_color);
void CustomLabel_SetBackColorHot(HWND hCtrl, DWORD back_color_hot);
void CustomLabel_SetBackColorSelected(HWND hCtrl, DWORD back_color_selected);
void CustomLabel_SetBorder(HWND hCtrl, REAL border_width, DWORD border_color, DWORD color_focus);
void CustomLabel_SetBorderColor(HWND hCtrl, DWORD border_color);
void CustomLabel_SetBorderColorFocus(HWND hCtrl, DWORD color_focus);
DWORD CustomLabel_GetBackColor(HWND hCtrl);
void CustomLabel_SetFont(HWND hCtrl, std::wstring font_name, int font_size, bool font_bold);
void CustomLabel_SetFontHot(HWND hCtrl, std::wstring font_name, int font_size, bool font_bold);
void CustomLabel_SetUserData(HWND hCtrl, std::wstring text);
std::wstring CustomLabel_GetUserData(HWND hCtrl);
void CustomLabel_SetUserDataInt(HWND hCtrl, int value);
int CustomLabel_GetUserDataInt(HWND hCtrl);
void CustomLabel_SetTextOffset(HWND hCtrl, int OffsetLeft, int OffsetTop);
void CustomLabel_SetImageOffset(HWND hCtrl, int OffsetLeft, int OffsetTop);
void CustomLabel_SetMousePointer(HWND hCtrl, CustomLabelPointer normal_pointer, CustomLabelPointer hot_pointer);
void CustomLabel_SetImages(HWND hCtrl, int image_id, int image_hot_id);
void CustomLabel_SetCheckState(HWND hCtrl, bool check_state);
bool CustomLabel_GetCheckState(HWND hCtrl);

HWND CustomLabel_SimpleLabel(HWND hParent, int ctrl_id, std::wstring text,
	DWORD text_color, DWORD back_color, CustomLabelAlignment alignment = CustomLabelAlignment::middle_left,
	int left = 0, int top = 0, int width = 0, int height = 0);

HWND CustomLabel_HorizontalLine(HWND hParent, int ctrl_id, DWORD line_color, DWORD back_color,
	int left = 0, int top = 0, int width = 0, int height = 0);

HWND CustomLabel_VerticalLine(HWND hParent, int ctrl_id, DWORD line_color, DWORD back_color,
	int left = 0, int top = 0, int width = 0, int height = 0);

HWND CustomLabel_ButtonLabel(HWND hParent, int ctrl_id, std::wstring text,
	DWORD text_color, DWORD back_color, DWORD back_color_hot, DWORD back_color_button_down, DWORD focus_border_color,
	CustomLabelAlignment alignment = CustomLabelAlignment::middle_left,
	int left = 0, int top = 0, int width = 0, int height = 0);

HWND CustomLabel_SimpleCheckBox(HWND hParent, int ctrl_id, std::wstring text,
	DWORD text_color, DWORD back_color, DWORD check_color, DWORD check_back_color, DWORD border_focus_color,
	int left, int top, int width, int height);

HWND CustomLabel_SimpleImageLabel(HWND hParent, int ctrl_id,
	int image_id, int image_hot_id,
	int image_width, int image_height,
	int left, int top, int width, int height);

HWND CreateCustomLabel(HWND hWndParent, LONG_PTR ctrl_id, CustomLabelType ctrl_type,
	int left, int top, int width, int height);

Gdiplus::Bitmap* LoadImageFromResource(HMODULE hMod, const wchar_t* resid, const wchar_t* restype);

constexpr int MSG_CUSTOMLABEL_CLICK = WM_USER + 1000;
constexpr int MSG_CUSTOMLABEL_MOUSEMOVE = WM_USER + 1001;
constexpr int MSG_CUSTOMLABEL_MOUSELEAVE = WM_USER + 1002;
