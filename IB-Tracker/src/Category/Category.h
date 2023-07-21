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
    DWORD BackColorSelected{};

    DWORD TextColor{};
    int FontSize = 9;
    std::wstring wszText;
};


enum class Category
{
    Category_Start = 0,
    Category0 = 0,   
    Category1 = 1,   
    Category2 = 2,   
    Category3 = 3,   
    Category4 = 4,   
    Category5 = 5,   
    Category6 = 6,   
    Category7 = 7,   
    Category_End = 7
};


const int IDC_CATEGORYCONTROL_COMBOBOX = 100;
const int IDC_CATEGORYCONTROL_COMMAND  = 101;
const int IDC_CATEGORYCONTROL_SETUP    = 102;

const int CATEGORYCONTROL_COMBOBOX_WIDTH = 165;
const int CATEGORYCONTROL_COMMAND_WIDTH  = 23;
const int CATEGORYCONTROL_SETUP_WIDTH    = 23;
const int CATEGORYCONTROL_HMARGIN        = 1;
const int CATEGORYCONTROL_HEIGHT         = 23;


const int IDC_CATEGORYCONTROL_FIRST  = 140;
const int IDC_CATEGORYCONTROL_0      = 140;
const int IDC_CATEGORYCONTROL_1      = 141;
const int IDC_CATEGORYCONTROL_2      = 142;
const int IDC_CATEGORYCONTROL_3      = 143;
const int IDC_CATEGORYCONTROL_4      = 144;
const int IDC_CATEGORYCONTROL_5      = 145;
const int IDC_CATEGORYCONTROL_6      = 146;
const int IDC_CATEGORYCONTROL_7      = 147;
const int IDC_CATEGORYCONTROL_8      = 148;
const int IDC_CATEGORYCONTROL_LAST   = 148;

CategoryControl* CategoryControl_GetOptions(HWND hCtrl);
int CategoryControl_SetOptions(HWND hCtrl, CategoryControl* pData);
int CategoryControl_GetSelectedIndex(HWND hwnd);
void CategoryControl_SetSelectedIndex(HWND hwnd, int index);

HWND CreateCategoryControl(HWND hWndParent, int CtrlId, int nLeft, int nTop, int SelectedIndex);

