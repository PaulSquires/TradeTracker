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


// Various actions that can be performed on selected Trade or Legs.
enum class TradeAction {
    new_options_trade,
    new_shares_trade,
    new_futures_trade,
    manage_shares,
    manage_futures,
    roll_leg,
    close_leg,
    expire_leg,
    assignment,
    add_options_to_trade,
    add_shares_to_trade,
    add_dividend_to_trade,
    add_futures_to_trade,
    add_put_to_trade,
    add_call_to_trade,
    new_iron_condor,
    new_short_put_LT112,
    new_short_strangle,
    new_short_put,
    new_short_call,
    new_short_put_vertical,
    new_short_call_vertical,
    other_income_expense,
    edit_transaction,
    no_action
};



// User defined messages
constexpr int MSG_CUSTOMLABEL_CLICK			= WM_USER + 1000;
constexpr int MSG_CUSTOMLABEL_MOUSEMOVE		= WM_USER + 1001;
constexpr int MSG_CUSTOMLABEL_MOUSELEAVE	= WM_USER + 1002;
constexpr int MSG_TWS_CONNECT_START			= WM_USER + 1003;
constexpr int MSG_TWS_CONNECT_SUCCESS		= WM_USER + 1004;
constexpr int MSG_TWS_CONNECT_FAILURE		= WM_USER + 1005;
constexpr int MSG_TWS_CONNECT_DISCONNECT	= WM_USER + 1006;
constexpr int MSG_TWS_WARNING_EXCEPTION     = WM_USER + 1007;
constexpr int MSG_TWS_CONNECT_WAIT_RECONNECTION = WM_USER + 1008;
constexpr int MSG_DATEPICKER_DATECHANGED    = WM_USER + 1009;
constexpr int MSG_RECONCILIATION_READY      = WM_USER + 1010;
constexpr int MSG_CATEGORY_CATEGORYCHANGED  = WM_USER + 1011;
constexpr int MSG_CUSTOMCOMBO_ITEMCHANGED   = WM_USER + 1012;
constexpr int MSG_POSITIONS_READY           = WM_USER + 1013;
constexpr int MSG_ACTIVETRADES_SELECTLISTBOXITEM = WM_USER + 1014;
constexpr int MSG_ACTIVETRADES_RIGHTCLICKMENU = WM_USER + 1015;
constexpr int MSG_TICKERTOTALS_SHOWTICKERCLOSEDTRADES = WM_USER + 1016;
constexpr int MSG_CLOSEDTRADES_SETSHOWTRADEDETAIL = WM_USER + 1017;
constexpr int MSG_CLOSEDTRADES_SHOWLISTBOXITEM = WM_USER + 1018;
constexpr int MSG_TRANSPANEL_SHOWLISTBOXITEM = WM_USER + 1019;
constexpr int MSG_TRANSDATEFILTER_DOSELECTED = WM_USER + 1020;

constexpr int DIALOG_RETURN_OK = 0;
constexpr int DIALOG_RETURN_CANCEL = 1;