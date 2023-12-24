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
#include "Utilities/UserMessages.h"


class CAssignment: public CWindowBase<CAssignment> {
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

    HWND hWindow = NULL;

    HWND QuantityTextBox();
    HWND Text1Label();
    HWND Text2Label();
    HWND OkayButton();
    HWND CancelButton();

    int ShowModal(const std::wstring& long_short_text, 
        const std::wstring& quantity_assigned_text, const std::wstring& strike_price_text);

private:
    bool OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
    void OnPaint(HWND hwnd);
    bool OnEraseBkgnd(HWND hwnd, HDC hdc);
    void OnClose(HWND hwnd);
    void OnDestroy(HWND hwnd);
};


constexpr int IDC_ASSIGNMENT_LBLTEXT1 = 100;
constexpr int IDC_ASSIGNMENT_LBLTEXT2 = 101;
constexpr int IDC_ASSIGNMENT_TXTQUANTITY = 102;
constexpr int IDC_ASSIGNMENT_OK = 103;
constexpr int IDC_ASSIGNMENT_CANCEL = 104;

extern CAssignment Assignment;
