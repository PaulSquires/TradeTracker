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
    NewIronCondor,
    NewShortLT112,
    NewShortStrangle,
    NewShortPut,
    NewShortCall,
    EditTransaction,
    NoAction
};



// User defined messages
constexpr int MSG_CUSTOMLABEL_CLICK			= WM_USER + 1000;
constexpr int MSG_CUSTOMLABEL_MOUSEMOVE		= WM_USER + 1001;
constexpr int MSG_CUSTOMLABEL_MOUSELEAVE	= WM_USER + 1002;
constexpr int MSG_TWS_CONNECT_START			= WM_USER + 1003;
constexpr int MSG_tws_Connect_SUCCESS		= WM_USER + 1004;
constexpr int MSG_tws_Connect_FAILURE		= WM_USER + 1005;
constexpr int MSG_tws_Connect_DISCONNECT	= WM_USER + 1006;
constexpr int MSG_TWS_WARNING_EXCEPTION     = WM_USER + 1007;
constexpr int MSG_tws_Connect_WAIT_RECONNECTION = WM_USER + 1008;
constexpr int MSG_STARTUP_SHOWTRADES        = WM_USER + 1009;
constexpr int MSG_DATEPICKER_DATECHANGED    = WM_USER + 1010;
constexpr int MSG_RECONCILIATION_READY      = WM_USER + 1011;
constexpr int MSG_CATEGORY_CATEGORYCHANGED  = WM_USER + 1012;
constexpr int MSG_POSITIONS_READY           = WM_USER + 1013;

constexpr int DIALOG_RETURN_OK = 0;
constexpr int DIALOG_RETURN_CANCEL = 1;