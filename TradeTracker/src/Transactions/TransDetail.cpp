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

#include "pch.h"
#include "CustomLabel/CustomLabel.h"
#include "CustomVScrollBar/CustomVScrollBar.h"
#include "ActiveTrades/ActiveTrades.h"
#include "MainWindow/MainWindow.h"
#include "Transactions/TransPanel.h"
#include "MainWindow/tws-client.h"
#include "Database/database.h"
#include "Database/trade.h"
#include "TradeDialog/TradeDialog.h"
#include "Utilities/ListBoxData.h"
#include "Config/Config.h"
#include "TransDetail.h"


std::shared_ptr<Trade> tradeEditDelete = nullptr;
std::shared_ptr<Transaction> transEditDelete = nullptr;

CTransDetail TransDetail;


// ========================================================================================
// Get the HWND's of the the controls on the form.
// ========================================================================================
inline HWND CTransDetail::TransListBox() {
    return GetDlgItem(hWindow, IDC_TRANSDETAIL_LISTBOX);
}
inline HWND CTransDetail::VScrollBar() {
    return GetDlgItem(hWindow, IDC_TRANSDETAIL_CUSTOMVSCROLLBAR);
}
inline HWND CTransDetail::Label1() {
    return GetDlgItem(hWindow, IDC_TRANSDETAIL_LABEL1);
}
inline HWND CTransDetail::CostLabel() {
    return GetDlgItem(hWindow, IDC_TRANSDETAIL_LBLCOST);
}
inline HWND CTransDetail::SymbolLabel() {
    return GetDlgItem(hWindow, IDC_TRANSDETAIL_SYMBOL);
}
inline HWND CTransDetail::EditButton() {
    return GetDlgItem(hWindow, IDC_TRANSDETAIL_CMDEDIT);
}
inline HWND CTransDetail::DeleteButton() {
    return GetDlgItem(hWindow, IDC_TRANSDETAIL_CMDDELETE);
}


// ========================================================================================
// Retrieve the Leg pointer based on a leg's backPointerID.
// ========================================================================================
std::shared_ptr<Leg> CTransDetail::GetLegBackPointer(std::shared_ptr<Trade> trade, int back_pointer_id) {
    for (auto& trans : trade->transactions) {
        for (auto& leg : trans->legs) {
            if (leg->leg_id == back_pointer_id) {
                return leg;
            }
        }
    }
    return nullptr;
}
    
    
// ========================================================================================
// Display the selected Transaction detail (legs) and ability to Edit it.
// ========================================================================================
void CTransDetail::EditTransaction(HWND hwnd) {
    if (!tradeEditDelete) return;
    if (!transEditDelete) return;

    tdd.legs.clear();
    tdd.legs = transEditDelete->legs;
    tdd.trade = tradeEditDelete;
    tdd.trans = transEditDelete;
    TradeDialog_Show(TradeAction::edit_transaction);
}
    

// ========================================================================================
// Delete (after confirmation) the selected Transaction.
// ========================================================================================
void CTransDetail::DeleteTransaction(HWND hwnd) {
    if (!tradeEditDelete) return;
    if (!transEditDelete) return;

    int res = MessageBox(
        hWindow,
        (LPCWSTR)(L"Are you sure you wish to DELETE this Transaction?"),
        (LPCWSTR)L"Confirm",
        MB_ICONWARNING | MB_YESNOCANCEL | MB_DEFBUTTON2);

    if (res != IDYES) return;

    // Iterate the trades and look into each TransDetail vector to match the 
    // transaction that we need to delete.
    for (auto& trade : trades) {
        auto iter = trade->transactions.begin();
        while (iter != trade->transactions.end()) {
            // If element matches the element to be deleted then delete it
            if (*iter == transEditDelete) {
                // Cycle through the legs being deleted to see if any contains a leg backpointer.
                // If yes, then retrieve the leg related to that pointer and update its open
                // quantity amount. Backpointers exist for Transactions that modify quantity
                // amounts like CLOSE, EXPIRE, ROLL.
                for (auto& leg : transEditDelete->legs) {
                    if (leg->leg_back_pointer_id != 0) {
                        // Get the backpointer leg.
                        std::shared_ptr<Leg> LegBackPointer = nullptr;
                        LegBackPointer = GetLegBackPointer(trade, leg->leg_back_pointer_id);
                        if (LegBackPointer != nullptr) {
                            LegBackPointer->open_quantity = LegBackPointer->open_quantity - leg->original_quantity;
                        }
                    }
                }
                iter = trade->transactions.erase(iter);
            }
            else
            {
                iter++;
            }
        }

    }

    // If no more Transactions exist in the Trade then delete the Trade itself
    if (tradeEditDelete->transactions.size() == 0) {
        auto it = std::find(trades.begin(), trades.end(), tradeEditDelete);
        if (it != trades.end()) trades.erase(it);
    }
    else {
        // Calculate the Trade open status because we may have just deleted the 
        // transaction that sets the trades open_quantity to zero.
        tradeEditDelete->SetTradeOpenStatus();
    }

    // Save the modified data
    SaveDatabase();

    tradeEditDelete = nullptr;
    transEditDelete = nullptr;

    // Reset the Active Trades
    ActiveTrades.ShowActiveTrades();
}


// ========================================================================================
// Display the selected Transaction detail (legs) and ability to Edit/Delete it.
// ========================================================================================
void CTransDetail::ShowTransDetail(const std::shared_ptr<Trade> trade, const std::shared_ptr<Transaction> trans) {
    tradeEditDelete = trade;
    transEditDelete = trans;

    // Clear the current transaction history table
    ListBoxData_DestroyItemData(TransListBox());

    // Ensure that the Transaction panel is set
    MainWindow.SetRightPanel(hWindow);

    if (!trade) {
        CustomLabel_SetText(Label1(), L"");
        CustomLabel_SetText(CostLabel(), L"");
        CustomLabel_SetText(SymbolLabel(), L"");
        ShowWindow(EditButton(), SW_HIDE);
        ShowWindow(DeleteButton(), SW_HIDE);
        ListBoxData_AddBlankLine(TransListBox());
        AfxRedrawWindow(TransListBox());
        return;
    }

    ShowWindow(EditButton(), SW_SHOW);
    ShowWindow(DeleteButton(), SW_SHOW);

    ListBoxData_HistoryHeader(TransListBox(), trade, trans);

    CustomLabel_SetText(Label1(), L"Transaction Details");

    // Construct the string to display the cost details of the selected transaction
    std::wstring wszDRCR = (transEditDelete->total < 0) ? L" DR" : L" CR";
    std::wstring wszPlusMinus = (transEditDelete->total < 0) ? L" + " : L" - ";
    std::wstring text;
    text = std::to_wstring(transEditDelete->quantity) + L" @ " +
                AfxMoney(transEditDelete->price, false, GetTickerDecimals(trade->ticker_symbol)) + wszPlusMinus +
                AfxMoney(transEditDelete->fees) + L" = " +
                AfxMoney(abs(transEditDelete->total)) + wszDRCR;
    CustomLabel_SetText(CostLabel(), text);
    
    text = trade->ticker_symbol + L": " + trade->ticker_name;
    if (trade->future_expiry.length()) {
        text = text + L" ( " + AfxFormatFuturesDate(trade->future_expiry) + L" )";
    }
    CustomLabel_SetText(SymbolLabel(), text);

    // Show the detail leg information for this transaction.
    for (const auto& leg : trans->legs) {
        if (leg->underlying == L"OPTIONS") { 
            ListBoxData_HistoryOptionsLeg(TransListBox(), trade, trans, leg);
        }
        else if (leg->underlying == L"SHARES") { 
            ListBoxData_HistorySharesLeg(TransListBox(), trade, trans, leg);
        }
        else if (leg->underlying == L"FUTURES") {
            ListBoxData_HistorySharesLeg(TransListBox(), trade, trans, leg);
        }
        else if (leg->underlying == L"DIVIDEND") {
            ListBoxData_HistoryDividendLeg(TransListBox(), trade, trans, leg);
        }
    }

    ListBoxData_AddBlankLine(TransListBox());

    // Calculate the actual column widths based on the size of the strings in
    // ListBoxData while respecting the minimum values as defined in nMinColWidth[].
    // This function is also called when receiving new price data from TWS because
    // that data may need the column width to be wider.
    ListBoxData_ResizeColumnWidths(TransListBox(), TableType::trade_history);

    // Set the ListBox to the topline.
    ListBox_SetTopIndex(TransListBox(), 0);

}


// ========================================================================================
// Listbox subclass Window procedure
// ========================================================================================
LRESULT CALLBACK CTransDetail::ListBox_SubclassProc(
    HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    // Create static accumulation variable to collect the data from
    // a series of middle mouse wheel scrolls.
    static int accum_delta = 0;

    switch (uMsg) {

    case WM_MOUSEWHEEL: {
        // Accumulate delta until scroll one line (up +120, down -120). 
        // 120 is the Microsoft default delta
        int zdelta = GET_WHEEL_DELTA_WPARAM(wParam);
        int top_index = (int)SendMessage(hwnd, LB_GETTOPINDEX, 0, 0);
        accum_delta += zdelta;
        if (accum_delta >= 120) {     // scroll up 3 lines
            top_index -= 3;
            top_index = max(0, top_index);
            SendMessage(hwnd, LB_SETTOPINDEX, top_index, 0);
            accum_delta = 0;
        }
        else {
            if (accum_delta <= -120) {     // scroll down 3 lines
                top_index += +3;
                SendMessage(hwnd, LB_SETTOPINDEX, top_index, 0);
                accum_delta = 0;
            }
        }
        HWND hCustomVScrollBar = GetDlgItem(GetParent(hwnd), IDC_TRANSDETAIL_CUSTOMVSCROLLBAR);
        CustomVScrollBar_Recalculate(hCustomVScrollBar);
        return 0;
    }

    case WM_ERASEBKGND: {
        // If the number of lines in the listbox maybe less than the number per page then 
        // calculate from last item to bottom of listbox, otherwise calculate based on
        // the mod of the lineheight to listbox height so we can color the partial line
        // that won't be displayed at the bottom of the list.
        RECT rc; GetClientRect(hwnd, &rc);

        RECT rcItem{};
        SendMessage(hwnd, LB_GETITEMRECT, 0, (LPARAM)&rcItem);
        int item_height = (rcItem.bottom - rcItem.top);
        int items_count = ListBox_GetCount(hwnd);
        int top_index = (int)SendMessage(hwnd, LB_GETTOPINDEX, 0, 0);
        int visible_rows = 0;
        int items_per_page = 0;
        int bottom_index = 0;
        int width = (rc.right - rc.left);
        int height = (rc.bottom - rc.top);

        if (items_count > 0) {
            items_per_page = (height) / item_height;
            bottom_index = (top_index + items_per_page);
            if (bottom_index >= items_count)
                bottom_index = items_count - 1;
            visible_rows = (bottom_index - top_index) + 1;
            rc.top = visible_rows * item_height;
        }

        if (rc.top < rc.bottom) {
            height = (rc.bottom - rc.top);
            HDC hDC = (HDC)wParam;
            Graphics graphics(hDC);
            Color back_color(COLOR_GRAYDARK);
            SolidBrush back_brush(back_color);
            graphics.FillRectangle(&back_brush, rc.left, rc.top, width, height);
        }

        ValidateRect(hwnd, &rc);
        return true;
    }

    case WM_DESTROY: {
        // Destroy all manually allocated ListBox display data that is held
        // in the LineData structures..
        ListBoxData_DestroyItemData(hwnd);

        // REQUIRED: Remove control subclassing
        RemoveWindowSubclass(hwnd, ListBox_SubclassProc, uIdSubclass);
        break;
    }

    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}


// ========================================================================================
// Process WM_MEASUREITEM message for window/dialog: TransDetail
// ========================================================================================
void CTransDetail::OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT* lpMeasureItem) {
    lpMeasureItem->itemHeight = AfxScaleY(TRANSDETAIL_LISTBOX_ROWHEIGHT);
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: TransDetail
// ========================================================================================
bool CTransDetail::OnEraseBkgnd(HWND hwnd, HDC hdc) {
    // Handle all of the painting in WM_PAINT
    return true;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: TransDetail
// ========================================================================================
void CTransDetail::OnPaint(HWND hwnd) {
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    Graphics graphics(hdc);

    // Create the background brush
    Color back_color(COLOR_BLACK);
    SolidBrush back_brush(back_color);

    // Paint the background using brush.
    int width = (ps.rcPaint.right - ps.rcPaint.left);
    int height = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, width, height);

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Process WM_SIZE message for window/dialog: TransDetail
// ========================================================================================
void CTransDetail::OnSize(HWND hwnd, UINT state, int cx, int cy) {
    int margin = AfxScaleY(TRANSDETAIL_MARGIN);
    int top = 0;
    int left = 0;
    int button_width = AfxScaleX(80);
    int button_height = AfxScaleX(23);

    HDWP hdwp = BeginDeferWindowPos(10);

    // Move and size the top label into place
    hdwp = DeferWindowPos(hdwp, Label1(), 0,
        0, top, cx, margin, SWP_NOZORDER | SWP_SHOWWINDOW);

    top += margin;
    hdwp = DeferWindowPos(hdwp, SymbolLabel(), 0,
        0, top, cx, margin, SWP_NOZORDER | SWP_SHOWWINDOW);

    top += margin;
    hdwp = DeferWindowPos(hdwp, CostLabel(), 0,
        0, top, cx - (button_width * 2) - AfxScaleX(8), margin, SWP_NOZORDER | SWP_SHOWWINDOW);

    left = cx - (button_width * 2) - AfxScaleX(8);
    hdwp = DeferWindowPos(hdwp, EditButton(), 0,
        left, top, button_width, button_height, SWP_NOZORDER | SWP_SHOWWINDOW);

    left = cx - button_width;
    hdwp = DeferWindowPos(hdwp, DeleteButton(), 0,
        left, top, button_width, button_height, SWP_NOZORDER | SWP_SHOWWINDOW);

    // Do not call the calcVThumbRect() function during a scrollbar move. This WM_SIZE
    // gets triggered when the ListBox WM_DRAWITEM fires. If we do another calcVThumbRect()
    // calcualtion then the scrollbar will appear "jumpy" under the user's mouse cursor.
    bool bshow_scrollbar = false;
    CustomVScrollBar* pData = CustomVScrollBar_GetPointer(VScrollBar());
    if (pData) {
        if (pData->drag_active) {
            bshow_scrollbar = true;
        }
        else {
            bshow_scrollbar = pData->calcVThumbRect();
        }
    }
    int custom_scrollbar_width = bshow_scrollbar ? AfxScaleX(CUSTOMVSCROLLBAR_WIDTH) : 0;

    top = AfxScaleY(80);
    left = 0;
    int width = cx - custom_scrollbar_width;
    int height = cy - top;
    hdwp = DeferWindowPos(hdwp, TransListBox(), 0, left, top, width, height, SWP_NOZORDER | SWP_SHOWWINDOW);

    left = left + width;   // right edge of ListBox
    width = custom_scrollbar_width;
    hdwp = DeferWindowPos(hdwp, VScrollBar(), 0, left, top, width, height,
        SWP_NOZORDER | (bshow_scrollbar ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));

    EndDeferWindowPos(hdwp);
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: TransDetail
// ========================================================================================
bool CTransDetail::OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct) {
    hWindow = hwnd;

    std::wstring font_name = AfxGetDefaultFont();
    int font_size = 9;

    HWND hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TRANSDETAIL_LABEL1, L"Transaction Details",
        COLOR_WHITELIGHT, COLOR_BLACK);

    hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TRANSDETAIL_SYMBOL, L"",
        COLOR_WHITEDARK, COLOR_BLACK);

    hCtl = CustomLabel_SimpleLabel(hwnd, IDC_TRANSDETAIL_LBLCOST, L"",
        COLOR_WHITEDARK, COLOR_BLACK);

    // EDIT button
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_TRANSDETAIL_CMDEDIT, L"EDIT",
        COLOR_BLACK, COLOR_GREEN, COLOR_GREEN, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_center, 0, 0, 0, 0);
    CustomLabel_SetFont(hCtl, font_name, font_size, true);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);

    // DELETE button
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_TRANSDETAIL_CMDDELETE, L"DELETE",
        COLOR_BLACK, COLOR_RED, COLOR_RED, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_center, 0, 0, 0, 0);
    CustomLabel_SetFont(hCtl, font_name, font_size, true);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);

    // Create an Ownerdraw listbox that we will use to custom paint our transaction detail.
    hCtl =
        TransDetail.AddControl(Controls::ListBox, hwnd, IDC_TRANSDETAIL_LISTBOX, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_NOSEL |
            LBS_OWNERDRAWFIXED | LBS_NOTIFY,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)ListBox_SubclassProc,
            IDC_TRANSDETAIL_LISTBOX, NULL);
    ListBox_AddString(hCtl, NULL);

    // Create our custom vertical scrollbar and attach the ListBox to it.
    CreateCustomVScrollBar(hwnd, IDC_TRANSDETAIL_CUSTOMVSCROLLBAR, hCtl, Controls::ListBox);

    return true;
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CTransDetail::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        HANDLE_MSG(m_hwnd, WM_CREATE, OnCreate);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, OnPaint);
        HANDLE_MSG(m_hwnd, WM_SIZE, OnSize);
        HANDLE_MSG(m_hwnd, WM_MEASUREITEM, OnMeasureItem);
        HANDLE_MSG(m_hwnd, WM_DRAWITEM, ListBoxData_OnDrawItem);

    case MSG_CUSTOMLABEL_CLICK: {
        HWND hCtl = (HWND)lParam;
        int ctrl_id = (int)wParam;

        if (hCtl == NULL) return 0;

        if (ctrl_id == IDC_TRANSDETAIL_CMDEDIT) {
            EditTransaction(m_hwnd);
            return 0;
        }

        if (ctrl_id == IDC_TRANSDETAIL_CMDDELETE) {
            DeleteTransaction(m_hwnd);
        }
        return 0;
    }

    }

    return DefWindowProc(m_hwnd, msg, wParam, lParam);
}

