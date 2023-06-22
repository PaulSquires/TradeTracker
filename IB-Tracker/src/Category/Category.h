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



class CategoryControl
{
public:
    HWND hwnd = NULL;
    HWND hParent = NULL;
    HWND hListBox = NULL;
    INT CtrlId = 0;

    HWND hToolTip = NULL;
    std::wstring wszToolTip;

    DWORD BackColor{};
    DWORD BackColorHot{};
    DWORD BackColorButtonDown{};

    int UnderlineHeight = 2;
    DWORD UnderlineColor = COLOR_WHITEMEDIUM;
    DWORD BackColorSelected{};

    DWORD TextColor{};
    int FontSize = 9;
    std::wstring wszText;
};


enum class Category
{
    Category0 = 0,
    Category1 = 1,
    Category2 = 2,
    Category3 = 3,
    Category4 = 4,
    CategoryAll = 5,
    CategorySetup = 200
};

const int IDC_CATEGORYCONTROL_FIRST  = 140;
const int IDC_CATEGORYCONTROL_GRAY   = 140;
const int IDC_CATEGORYCONTROL_BLUE   = 141;
const int IDC_CATEGORYCONTROL_PINK   = 142;
const int IDC_CATEGORYCONTROL_GREEN  = 143;
const int IDC_CATEGORYCONTROL_ORANGE = 144;
const int IDC_CATEGORYCONTROL_ALL    = 145;
const int IDC_CATEGORYCONTROL_LAST   = 145;
const int IDC_CATEGORYCONTROL_SETUP  = 200;

CategoryControl* CategoryControl_GetOptions(HWND hCtrl);
int CategoryControl_SetOptions(HWND hCtrl, CategoryControl* pData);
int CategoryControl_GetSelectedIndex(HWND hwnd);
void CategoryControl_SetSelectedIndex(HWND hwnd, int index);

HWND CreateCategoryControl(HWND hWndParent, int CtrlId,
    int nLeft, int nTop, int nWidth, int nHeight, bool AllowAllButton);

