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



class CategoryControl {
public:
    HWND hwnd = NULL;
    HWND hParent = NULL;
    HWND hListBox = NULL;
    int ctrl_id = 0;

    HWND hToolTip = NULL;
    std::wstring tooltip_text;

    DWORD back_color{};
    DWORD back_color_hot{};
    DWORD back_color_button_down{};
    DWORD back_color_selected{};

    DWORD text_color{};
    int font_size = 9;
    std::wstring text;
    bool allow_all_categories = false;  // Also includes OTHER category

};

CategoryControl* CategoryControl_GetOptions(HWND hCtrl);
int CategoryControl_SetOptions(HWND hCtrl, CategoryControl* pData);
int CategoryControl_GetSelectedIndex(HWND hwnd);
void CategoryControl_SetSelectedIndex(HWND hwnd, int index);
bool CategoryControl_Getallow_all_categories(HWND hwnd);
static LRESULT CALLBACK CategoryControl_CategoryControlProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HWND CreateCategoryControl(HWND hWndParent, int ctrl_id, int left, int top, int selected_index, bool allow_all_categories);


constexpr int CATEGORY_START = 0;
constexpr int CATEGORY_END = 15;
constexpr int CATEGORY_ALL = 99;
constexpr int CATEGORY_OTHER = 100;

constexpr int IDC_CATEGORYCONTROL_COMBOBOX = 100;
constexpr int IDC_CATEGORYCONTROL_COMMAND  = 101;
constexpr int IDC_CATEGORYCONTROL_SETUP    = 102;

constexpr int CATEGORYCONTROL_COMBOBOX_WIDTH = 165;
constexpr int CATEGORYCONTROL_COMMAND_WIDTH  = 23;
constexpr int CATEGORYCONTROL_SETUP_WIDTH    = 23;
constexpr int CATEGORYCONTROL_HMARGIN        = 1;
constexpr int CATEGORYCONTROL_HEIGHT         = 23;


constexpr int IDC_CATEGORYCONTROL_FIRST  = 140;
constexpr int IDC_CATEGORYCONTROL_0      = 140;
constexpr int IDC_CATEGORYCONTROL_1      = 141;
constexpr int IDC_CATEGORYCONTROL_2      = 142;
constexpr int IDC_CATEGORYCONTROL_3      = 143;
constexpr int IDC_CATEGORYCONTROL_4      = 144;
constexpr int IDC_CATEGORYCONTROL_5      = 145;
constexpr int IDC_CATEGORYCONTROL_6      = 146;
constexpr int IDC_CATEGORYCONTROL_7      = 147;
constexpr int IDC_CATEGORYCONTROL_8      = 148;
constexpr int IDC_CATEGORYCONTROL_9      = 149;
constexpr int IDC_CATEGORYCONTROL_10     = 150;
constexpr int IDC_CATEGORYCONTROL_11     = 151;
constexpr int IDC_CATEGORYCONTROL_12     = 152;
constexpr int IDC_CATEGORYCONTROL_13     = 153;
constexpr int IDC_CATEGORYCONTROL_14     = 154;
constexpr int IDC_CATEGORYCONTROL_15     = 155;
constexpr int IDC_CATEGORYCONTROL_LAST   = 155;


