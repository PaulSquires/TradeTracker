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

enum class ConnectionType {
    tws_data_live,
    tws_data_paper,
    scraped_data,
    disconnect
};


class CTabPanel : public CWindowBase<CTabPanel> {
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

extern CTabPanel TabPanel;
extern HWND HWND_TABPANEL;


constexpr int TABPANEL_HEIGHT = 36;

constexpr int IDC_TABPANEL_CONNECT      = 100;
constexpr int IDC_TABPANEL_RECONCILE    = 101;
constexpr int IDC_TABPANEL_SETTINGS     = 102;
constexpr int IDC_TABPANEL_SEPARATOR    = 103;
constexpr int IDC_TABPANEL_ACTIVETRADES = 104;
constexpr int IDC_TABPANEL_CLOSEDTRADES = 105;
constexpr int IDC_TABPANEL_TRANSACTIONS = 106;
constexpr int IDC_TABPANEL_TICKERTOTALS = 107;
constexpr int IDC_TABPANEL_JOURNALNOTES = 108;
constexpr int IDC_TABPANEL_TRADEPLAN    = 109;

void TabPanel_SelectPanelItem(HWND hwnd, int ctrl_id);
