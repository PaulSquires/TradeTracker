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

#include "Utilities/CWindowBase.h"
#include "CustomLabel/CustomLabel.h"
#include "Utilities/Colors.h"


class CCustomMessageBox: public CWindowBase<CCustomMessageBox> {
public:
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

    int Show(HWND hwnd, const std::wstring& message, const std::wstring& caption, int option_flags);

private:
    bool OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
    void OnSize(HWND hwnd, UINT state, int cx, int cy);
    void OnClose(HWND hwnd);
    void OnDestroy(HWND hwnd);
    void OnPaint(HWND hwnd);
    bool OnEraseBkgnd(HWND hwnd, HDC hdc);

    SIZEL CalculateMessageBoxSize(HWND hwnd);

    HWND hParent = NULL;
    std::wstring message_text;
    std::wstring caption_text;
    int flags = 0;

    int result_code = 0;
    DWORD back_color = COLOR_WHITE;
    DWORD back_color_button_area = (DWORD) Color::MakeARGB(255, 240, 240, 240);
    DWORD text_color = COLOR_BLACK;
    DWORD back_color_down = COLOR_GRAYLIGHT;
    DWORD focus_border_color = COLOR_BLACK;

	HWND hWindow = NULL;

    HWND hButton1 = NULL;
    HWND hButton2 = NULL;
    HWND hButton3 = NULL;
    int button_count = 0;
    int button_area_height = AfxScaleY(50);

    HWND hStaticMessage = NULL;
    HWND hStaticIcon = NULL;

    int text_height = 0;
    int text_width = 0;

    int left_margin = AfxScaleX(20);
    int right_margin = AfxScaleX(30);

    int image_text_margin = AfxScaleX(10);
    int image_top_margin = AfxScaleY(32);
    int image_bottom_margin = AfxScaleY(24);

    int icon_width = AfxScaleX(32);
    int icon_height = AfxScaleY(32);

};

constexpr int IDC_MESSAGEBOX_OK = 100;
constexpr int IDC_MESSAGEBOX_CANCEL = 101;
constexpr int IDC_MESSAGEBOX_YES = 102;
constexpr int IDC_MESSAGEBOX_NO = 103;
constexpr int IDC_MESSAGEBOX_STATIC = 104;
constexpr int IDC_MESSAGEBOX_ICON = 105;


