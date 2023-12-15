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


struct CustomComboItemData
{
    std::wstring item_text;
    int item_data = 0;
};

class CustomComboControl
{
public:
    HWND hwnd = NULL;
    HWND hParent = NULL;
    HWND hListBox = NULL;
    int CtrlId = 0;
    int item_count = 0;

    std::vector< CustomComboItemData> items;

    HWND hToolTip = NULL;
    std::wstring tooltip_text;

    DWORD back_color{};
    DWORD back_color_hot{};
    DWORD back_color_button_down{};
    DWORD back_color_selected{};

    DWORD text_color{};
    int font_size = 9;
    std::wstring text;
};


constexpr int IDC_CUSTOMCOMBOCONTROL_COMBOBOX = 100;
constexpr int IDC_CUSTOMCOMBOCONTROL_COMMAND = 101;

constexpr int CUSTOMCOMBOCONTROL_COMBOBOX_WIDTH = 165;
constexpr int CUSTOMCOMBOCONTROL_COMMAND_WIDTH = 23;
constexpr int CUSTOMCOMBOCONTROL_HEIGHT = 23;


CustomComboControl* CustomComboControl_GetOptions(HWND hCtrl);
int CustomComboControl_SetOptions(HWND hCtrl, CustomComboControl* pData);
int CustomComboControl_GetSelectedIndex(HWND hwnd);
void CustomComboControl_SetSelectedIndex(HWND hwnd, int index);

HWND CreateCustomComboControl(HWND hWndParent, int CtrlId, int nLeft, int nTop, int item_count);

