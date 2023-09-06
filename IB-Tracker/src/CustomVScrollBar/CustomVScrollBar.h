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

#include "Utilities/Colors.h"


class CustomVScrollBar
{
public:
    HWND hwnd = NULL;
    HWND hParent = NULL;
    HWND hListBox = NULL;
    INT CtrlId = 0;
    bool drag_active = false;

    POINT prev_pt{};
    int listbox_height = 0;
    int item_height = 0;
    int items_count = 0;
    int items_per_page = 0;
    int thumb_height = 0;
    RECT rc{};
    
    DWORD scrollbar_line_color = COLOR_SCROLLBARDIVIDER;
    DWORD scrollbar_back_color = COLOR_SCROLLBARBACK;
    DWORD scrollbar_thumb_color = COLOR_SCROLLBARTHUMB;

    bool calcVThumbRect();
    
};


constexpr int CUSTOMVSCROLLBAR_WIDTH = 14;

HWND CreateCustomVScrollBar(HWND hWndParent, LONG_PTR CtrlId, HWND hListBox);
CustomVScrollBar* CustomVScrollBar_GetPointer(HWND hCtrl);
void CustomVScrollBar_Recalculate(HWND hCtrl);

