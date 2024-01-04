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

class CCustomPopupMenuItem {
public:
    std::wstring text;
    int id = 0;
    bool is_separator = false;
};


class CCustomPopupMenu: public CWindowBase<CCustomPopupMenu> {
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

    int Show(HWND hwnd, std::vector< CCustomPopupMenuItem> items, int initial_selected_item, 
        int left, int top, int width_override = -1);

private:
    bool OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
    void OnSize(HWND hwnd, UINT state, int cx, int cy);
    void OnPaint(HWND hwnd);
    bool OnEraseBkgnd(HWND hwnd, HDC hdc);
    void OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT* lpDrawItem);
    void OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT* lpMeasureItem);

    HWND hWindow = NULL;
    HWND hParent = NULL;

    std::vector<CCustomPopupMenuItem> items;
    int row_width = AfxScaleX(100);
    int row_height = AfxScaleY(24);
    int row_height_separator = AfxScaleY(10);
    int glyph_width = AfxScaleX(24);

    int row_count = 0;
    int sep_count = 0;

    int selected_item = -1;
};

extern HWND HWND_CUSTOMPOPUPMENU;

constexpr int IDC_CUSTOMPOPUPMENU_LISTBOX = 100;

extern CCustomPopupMenu CustomPopupMenu;
