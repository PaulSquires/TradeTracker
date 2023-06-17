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

#include "Utilities/CWindowBase.h"


class CSideMenu : public CWindowBase<CSideMenu>
{
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

const int IDC_SideMenu_FIRSTITEM       = 100;
const int IDC_SideMenu_LOGO            = IDC_SideMenu_FIRSTITEM;
const int IDC_SideMenu_TRADERNAME      = 102;
const int IDC_SideMenu_APPNAME         = 103;
const int IDC_SideMenu_ACTIVETRADES    = 104;
const int IDC_SideMenu_CLOSEDTRADES    = 105;
const int IDC_SideMenu_CONNECTTWS      = 106;
const int IDC_SideMenu_TICKERTOTALS    = 107;
const int IDC_SideMenu_DAILYTOTALS     = 108;
const int IDC_SideMenu_RECONCILE       = 109;
const int IDC_SideMenu_TransDetail    = 110;
const int IDC_SideMenu_NEWSHARESTRADE  = 111;
const int IDC_SideMenu_NEWFUTURESTRADE = 112;
const int IDC_SideMenu_NEWOPTIONSTRADE = 113;

const int IDC_SideMenu_NEWIRONCONDOR   = 114;
const int IDC_SideMenu_NEWSHORTSTRANGLE= 115;
const int IDC_SideMenu_NEWSHORTPUT     = 116;
const int IDC_SideMenu_NEWSHORTCALL    = 117;

const int IDC_SideMenu_LASTITEM = IDC_SideMenu_NEWSHORTCALL;

const int IDC_SideMenu_AUTOCONNECT     = 120;

const int SideMenu_WIDTH = 180;

void SideMenu_SelectMenuItem(HWND hParent, int CtrlId);
int SideMenu_GetActiveMenuItem(HWND hParent);
