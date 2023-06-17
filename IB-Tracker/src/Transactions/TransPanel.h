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


class CTransPanel : public CWindowBase<CTransPanel>
{
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

};


const int IDC_TRANS_LISTBOX = 100;
const int IDC_TRANS_LABEL = 101;
const int IDC_TRANS_CUSTOMVSCROLLBAR = 102;
const int IDC_TRANS_HEADER = 103;

const int IDC_TRANS_LBLTICKERFILTER = 110;
const int IDC_TRANS_TXTTICKER = 111;
const int IDC_TRANS_CMDTICKERGO = 112;

const int IDC_TRANS_LBLDATEFILTER = 115;
const int IDC_TRANS_TRANSDATE = 116;
const int IDC_TRANS_CMDTRANSDATE = 117;

const int IDC_TRANS_LBLSTARTDATE = 120;
const int IDC_TRANS_STARTDATE = 121;
const int IDC_TRANS_CMDSTARTDATE = 122;

const int IDC_TRANS_LBLENDDATE = 125;
const int IDC_TRANS_ENDDATE = 126;
const int IDC_TRANS_CMDENDDATE = 127;


const int TRANSDETAIL_LISTBOX_ROWHEIGHT = 18;
const int TRANSDETAIL_MARGIN = 80;

