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

extern CSideMenu SideMenu;
extern HWND HWND_SIDEMENU;


const int IDC_SIDEMENU_FIRSTITEM       = 100;
const int IDC_SIDEMENU_LOGO            = IDC_SIDEMENU_FIRSTITEM;
const int IDC_SIDEMENU_APPNAME         = 102;
const int IDC_SIDEMENU_APPVERSION      = 103;
const int IDC_SIDEMENU_ACTIVETRADES    = 104;
const int IDC_SIDEMENU_CLOSEDTRADES    = 105;
const int IDC_SIDEMENU_CONNECTTWS      = 106;
const int IDC_SIDEMENU_OTHERINCOME     = 107;
const int IDC_SIDEMENU_TICKERTOTALS    = 108;
const int IDC_SIDEMENU_RECONCILE       = 109;
const int IDC_SIDEMENU_TRANSACTIONS    = 110;
const int IDC_SIDEMENU_JOURNALNOTES    = 111;
const int IDC_SIDEMENU_NEWSHARESTRADE  = 112;
const int IDC_SIDEMENU_NEWFUTURESTRADE = 113;
const int IDC_SIDEMENU_NEWOPTIONSTRADE = 114;

const int IDC_SIDEMENU_NEWIRONCONDOR   = 115;
const int IDC_SIDEMENU_NEWSHORTLT112   = 116;
const int IDC_SIDEMENU_NEWSHORTSTRANGLE= 117;
const int IDC_SIDEMENU_NEWSHORTPUT     = 118;
const int IDC_SIDEMENU_NEWSHORTCALL    = 119;

const int IDC_SIDEMENU_LASTITEM = IDC_SIDEMENU_NEWSHORTCALL;

const int SIDEMENU_WIDTH = 180;

void SideMenu_SelectMenuItem(HWND hParent, int CtrlId);
int SideMenu_GetActiveMenuItem(HWND hParent);
