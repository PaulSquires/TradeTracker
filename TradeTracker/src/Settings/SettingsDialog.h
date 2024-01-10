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
#include "Config/Config.h"


class CSettingsDialog : public CWindowBase<CSettingsDialog> {
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

    // Settings that require a restart
    NumberFormatType orig_number_format_type = NumberFormatType::American;
    CostingMethod orig_costing_method = CostingMethod::AverageCost;
    // Settings that trigger an immediate action when Settings dialog is closed
    bool orig_allow_update_check = true;
};


constexpr int IDC_SETTINGSDIALOG_SAVE = 100;
constexpr int IDC_SETTINGSDIALOG_CANCEL = 101;
constexpr int IDC_SETTINGSDIALOG_CMDYEAREND = 102;
constexpr int IDC_SETTINGSDIALOG_UPDATECHECK = 103;
constexpr int IDC_SETTINGSDIALOG_PORTFOLIOVALUE = 104;
constexpr int IDC_SETTINGSDIALOG_NUMBERFORMAT = 105;
constexpr int IDC_SETTINGSDIALOG_NUMBERFORMAT_USA = 106;
constexpr int IDC_SETTINGSDIALOG_NUMBERFORMAT_EU = 107;
constexpr int IDC_SETTINGSDIALOG_COSTBASIS = 108;
constexpr int IDC_SETTINGSDIALOG_COSTBASIS_AVERAGE = 109;
constexpr int IDC_SETTINGSDIALOG_COSTBASIS_FIFO = 110;
constexpr int IDC_SETTINGSDIALOG_STARTWEEKDAY = 111;
constexpr int IDC_SETTINGSDIALOG_STARTWEEKDAY_SUNDAY = 112;
constexpr int IDC_SETTINGSDIALOG_STARTWEEKDAY_MONDAY = 113;


int SettingsDialog_Show(HWND hWndParent);
