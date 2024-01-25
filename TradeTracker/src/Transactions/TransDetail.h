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
#include "Database/trade.h"


class CTransDetail: public CWindowBase<CTransDetail> {
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

    HWND hWindow = NULL;

    HWND TransListBox();
    HWND VScrollBar();
    HWND Label1();
    HWND CostLabel();
    HWND SymbolLabel();
    HWND EditButton();
    HWND DeleteButton();

    void ShowTransDetail(const std::shared_ptr<Trade> trade, const std::shared_ptr<Transaction> trans);
    std::shared_ptr<Leg> GetLegBackPointer(std::shared_ptr<Trade> trade, int back_pointer_id);

private:
    bool OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
    void OnSize(HWND hwnd, UINT state, int cx, int cy);
    void OnPaint(HWND hwnd);
    bool OnEraseBkgnd(HWND hwnd, HDC hdc);
    void OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT* lpMeasureItem);

    void EditTransaction(HWND hwnd);
    void DeleteTransaction(HWND hwnd);

    static LRESULT CALLBACK ListBox_SubclassProc(
        HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
        UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

};

extern CTransDetail TransDetail;


constexpr int IDC_TRANSDETAIL_LISTBOX = 110;
constexpr int IDC_TRANSDETAIL_LABEL1 = 111;
constexpr int IDC_TRANSDETAIL_LBLCOST = 112;
constexpr int IDC_TRANSDETAIL_SYMBOL = 113;
constexpr int IDC_TRANSDETAIL_CMDEDIT = 114;
constexpr int IDC_TRANSDETAIL_CMDDELETE = 115;
constexpr int IDC_TRANSDETAIL_CUSTOMVSCROLLBAR = 116;

constexpr int TRANSDETAIL_LISTBOX_ROWHEIGHT = 20;
constexpr int TRANSDETAIL_WIDTH = 400;
constexpr int TRANSDETAIL_MARGIN = 24;

