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
#include "FilterPanel/TransDatePopup.h"

class CFilterPanel : public CWindowBase<CFilterPanel> {
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

    HWND hWindow = NULL;

    HWND TickerFilterLabel();
    HWND TickerTextBox();
    HWND TickerGoButton();
    HWND DateFilterLabel();
    HWND TransDateCombo();
    HWND TransDateButton();
    HWND StartDateLabel();
    HWND StartDateCombo();
    HWND StartDateButton();
    HWND EndDateLabel();
    HWND EndDateCombo();
    HWND EndDateButton();

    std::wstring start_date;
    std::wstring end_date;
    std::wstring ticker_symbol;

    void SetStartEndDates(HWND hwnd);

    HWND CreateFilterPanel(HWND hParent);

private:
    bool OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
    void OnSize(HWND hwnd, UINT state, int cx, int cy);
    void OnPaint(HWND hwnd);
    bool OnEraseBkgnd(HWND hwnd, HDC hdc);
};


constexpr int IDC_FILTER_LBLTICKERFILTER = 110;
constexpr int IDC_FILTER_TXTTICKER = 111;
constexpr int IDC_FILTER_CMDTICKERGO = 112;

constexpr int IDC_FILTER_LBLDATEFILTER = 115;
constexpr int IDC_FILTER_TRANSDATE = 116;
constexpr int IDC_FILTER_CMDTRANSDATE = 117;

constexpr int IDC_FILTER_LBLSTARTDATE = 120;
constexpr int IDC_FILTER_STARTDATE = 121;
constexpr int IDC_FILTER_CMDSTARTDATE = 122;

constexpr int IDC_FILTER_LBLENDDATE = 125;
constexpr int IDC_FILTER_ENDDATE = 126;
constexpr int IDC_FILTER_CMDENDDATE = 127;


extern CTransDatePopup TransDatePopup;
