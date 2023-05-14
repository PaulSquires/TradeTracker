#pragma once

#include "..\Utilities\UserMessages.h"
#include "..\Utilities\AfxWin.h"
#include "..\Themes\Themes.h"


class CategoryControl
{
public:
    HWND hwnd = NULL;
    HWND hParent = NULL;
    HWND hListBox = NULL;
    INT CtrlId = 0;

    HWND hToolTip = NULL;
    std::wstring wszToolTip;

    ThemeElement BackColor{};
    ThemeElement BackColorHot{};
    ThemeElement BackColorButtonDown{};

    int UnderlineHeight = 1;
    ThemeElement UnderlineColor = ThemeElement::WhiteMedium;
    ThemeElement BackColorSelected{};

    ThemeElement TextColor{};
    int FontSize = 9;
    std::wstring wszText;
};

const int IDC_CATEGORYCONTROL_FIRST  = 140;
const int IDC_CATEGORYCONTROL_GRAY   = 140;
const int IDC_CATEGORYCONTROL_BLUE   = 141;
const int IDC_CATEGORYCONTROL_PURPLE = 142;
const int IDC_CATEGORYCONTROL_GREEN  = 143;
const int IDC_CATEGORYCONTROL_ORANGE = 144;
const int IDC_CATEGORYCONTROL_LAST   = 144;

CategoryControl* CategoryControl_GetOptions(HWND hCtrl);
int CategoryControl_SetOptions(HWND hCtrl, CategoryControl* pData);
int CategoryControl_GetSelectedIndex(HWND hwnd);
void CategoryControl_SetSelectedIndex(HWND hwnd, int index);

HWND CreateCategoryControl(HWND hWndParent, int CtrlId,
    int nLeft, int nTop, int nWidth, int nHeight);

