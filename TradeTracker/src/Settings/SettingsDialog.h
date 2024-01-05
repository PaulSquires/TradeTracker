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


class CSettingsDialog : public CWindowBase<CSettingsDialog> {
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

};


constexpr int IDC_SETTINGSDIALOG_SAVE = 100;
constexpr int IDC_SETTINGSDIALOG_CANCEL = 101;
constexpr int IDC_SETTINGSDIALOG_CMDYEAREND = 102;
constexpr int IDC_SETTINGSDIALOG_UPDATECHECK = 103;
constexpr int IDC_SETTINGSDIALOG_PORTFOLIOVALUE = 104;
constexpr int IDC_SETTINGSDIALOG_NUMBERFORMAT_USA = 105;
constexpr int IDC_SETTINGSDIALOG_NUMBERFORMAT_EU = 106;
constexpr int IDC_SETTINGSDIALOG_COSTBASIS_AVERAGE = 107;
constexpr int IDC_SETTINGSDIALOG_COSTBASIS_FIFO = 108;


int SettingsDialog_Show(HWND hWndParent);
