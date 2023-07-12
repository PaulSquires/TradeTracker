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


// Various actions that can be performed on selected Trade or Legs.
// TODO: Add default multipliers for known futures contracts.
enum class TradeAction
{
    NewOptionsTrade,
    NewSharesTrade,
    NewFuturesTrade,
    ManageShares,
    ManageFutures,
    RollLeg,
    CloseLeg,
    ExpireLeg,
    Assignment,
    AddOptionsToTrade,
    AddSharesToTrade,
    AddFuturesToTrade,
    AddPutToTrade,
    AddCallToTrade,
    NewShortLT112,
    NewShortStrangle,
    NewShortPut,
    NewShortCall,
    EditTransaction,
    NoAction
};



// User defined messages
const int MSG_CUSTOMLABEL_CLICK			= WM_USER + 1000;
const int MSG_CUSTOMLABEL_MOUSEMOVE		= WM_USER + 1001;
const int MSG_CUSTOMLABEL_MOUSELEAVE	= WM_USER + 1002;
const int MSG_TWS_CONNECT_START			= WM_USER + 1003;
const int MSG_TWS_CONNECT_SUCCESS		= WM_USER + 1004;
const int MSG_TWS_CONNECT_FAILURE		= WM_USER + 1005;
const int MSG_TWS_CONNECT_DISCONNECT	= WM_USER + 1006;
const int MSG_TWS_WARNING_EXCEPTION     = WM_USER + 1007;
const int MSG_STARTUP_SHOWTRADES        = WM_USER + 1008;
const int MSG_DATEPICKER_DATECHANGED    = WM_USER + 1009;
const int MSG_CATEGORY_CHANGED          = WM_USER + 1010;

const int DIALOG_RETURN_OK = 0;
const int DIALOG_RETURN_CANCEL = 1;