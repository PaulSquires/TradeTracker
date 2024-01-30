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

#include "tws-api/Contract.h"


class CImportDialog : public CWindowBase<CImportDialog> {
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};


struct ImportStruct {
    Contract contract;
    Decimal  position = 0;
    double   avg_cost = 0;
};

struct DisplayStruct {
    ImportStruct* ibkr_ptr = nullptr;
    std::vector<std::string> text;
    bool is_checked = false;
    int group_id = 0;
};

extern HWND HWND_IMPORTDIALOG;

constexpr int IMPORTDIALOG_LISTBOX_ROWHEIGHT = 18;

constexpr int IDC_IMPORTDIALOG_LBLUNGROUPED = 100;
constexpr int IDC_IMPORTDIALOG_LISTBOX1 = 101;
constexpr int IDC_IMPORTDIALOG_CUSTOMVSCROLLBAR1 = 102;
constexpr int IDC_IMPORTDIALOG_HEADER1 = 103;

constexpr int IDC_IMPORTDIALOG_LBLGROUPED = 104;
constexpr int IDC_IMPORTDIALOG_LISTBOX2 = 105;
constexpr int IDC_IMPORTDIALOG_CUSTOMVSCROLLBAR2 = 106;
constexpr int IDC_IMPORTDIALOG_HEADER2 = 107;

constexpr int IDC_IMPORTDIALOG_GROUP = 130;
constexpr int IDC_IMPORTDIALOG_UNGROUP = 131;

constexpr int IDC_IMPORTDIALOG_SAVE = 140;
constexpr int IDC_IMPORTDIALOG_CANCEL = 141;

void ImportTrades_position(const Contract& contract, Decimal position, double avg_cost);
void ImportTrades_doPositionSorting();
void ImportTrades_AskImportMessage();
int ImportDialog_Show();


