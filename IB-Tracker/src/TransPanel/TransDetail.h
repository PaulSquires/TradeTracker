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

#include "Utilities\CWindowBase.h"


class CTransDetail: public CWindowBase<CTransDetail>
{
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

};


const int IDC_TRANSDETAIL_LISTBOX = 110;
const int IDC_TRANSDETAIL_LABEL1 = 111;
const int IDC_TRANSDETAIL_LBLCOST = 112;
const int IDC_TRANSDETAIL_SYMBOL = 113;
const int IDC_TRANSDETAIL_CATEGORY = 114;
const int IDC_TRANSDETAIL_CMDEDIT = 115;
const int IDC_TRANSDETAIL_CMDDELETE = 116;
const int IDC_TRANSDETAIL_CUSTOMVSCROLLBAR = 117;

const int TRANSDETAIL_LISTBOX_ROWHEIGHT = 20;
const int TRANSDETAIL_WIDTH = 400;
const int TRANSDETAIL_MARGIN = 24;

