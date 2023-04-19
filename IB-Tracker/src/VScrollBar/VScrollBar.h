#pragma once

#include "..\Utilities\UserMessages.h"
#include "..\Themes\Themes.h"
#include "..\Utilities\AfxWin.h"


class VScrollBar
{
public:
    HWND hwnd = NULL;
    HWND hParent = NULL;
    HWND hListBox = NULL;
    INT CtrlId = 0;
    bool bDragActive = false;

    POINT prev_pt{};
    int listBoxHeight = 0;
    int itemHeight = 0;
    int numItems = 0;
    int itemsPerPage = 0;
    int thumbHeight = 0;
    RECT rc{};
    
    ThemeElement ScrollBarLine = ThemeElement::TradesPanelScrollBarLine;
    ThemeElement ScrollBarBack = ThemeElement::TradesPanelScrollBarBack;
    ThemeElement ScrollBarThumb = ThemeElement::TradesPanelScrollBarThumb;

    bool calcVThumbRect();
    
};


const int VSCROLLBAR_WIDTH = 14;

HWND CreateVScrollBar(HWND hWndParent, LONG_PTR CtrlId, HWND hListBox);
VScrollBar* VScrollBar_GetPointer(HWND hCtrl);
void VScrollBar_Recalculate(HWND hCtrl);

