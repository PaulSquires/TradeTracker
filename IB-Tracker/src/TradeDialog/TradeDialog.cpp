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

#include "pch.h"

#include "TradeDialog.h"
#include "TradeDialogControls.h"
#include "Database/trade.h"
#include "MainWindow/MainWindow.h"
#include "ActiveTrades/ActiveTrades.h"
#include "CustomLabel/CustomLabel.h"
#include "DatePicker/Calendar.h"
#include "Strategy/StrategyPopup.h"
#include "Category/CategoryPopup.h"
#include "Utilities/UserMessages.h"
#include "Config/Config.h"
#include "TradeGrid/TradeGrid.h"


HWND HWND_TRADEDIALOG = NULL;

CTradeDialog TradeDialog;
TradeDialogData tdd;

int dialog_return_code = DIALOG_RETURN_CANCEL;



// ========================================================================================
// Process WM_CLOSE message for window/dialog: TradeDialog
// ========================================================================================
void TradeDialog_OnClose(HWND hwnd)
{
    MainWindow_BlurPanels(false);
    EnableWindow(HWND_MAINWINDOW, TRUE);
    DestroyWindow(hwnd);
}


// ========================================================================================
// Process WM_DESTROY message for window/dialog: TradeDialog
// ========================================================================================
void TradeDialog_OnDestroy(HWND hwnd)
{
    PostQuitMessage(0);
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: TradeDialog
// ========================================================================================
BOOL TradeDialog_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    HWND_TRADEDIALOG = hwnd;

    TradeDialogControls_CreateControls(hwnd);

    return TRUE;
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: TradeDialog
// ========================================================================================
BOOL TradeDialog_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    // Handle all of the painting in WM_PAINT
    return TRUE;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: TradeDialog
// ========================================================================================
void TradeDialog_OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    SaveDC(hdc);

    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP hbit = CreateCompatibleBitmap(hdc, ps.rcPaint.right, ps.rcPaint.bottom);
    SelectBitmap(memDC, hbit);

    Graphics graphics(memDC);
    int nWidth = (ps.rcPaint.right - ps.rcPaint.left);
    int nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);

    // Create the background brush
    SolidBrush back_brush(COLOR_GRAYDARK);
    graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

    // Copy the entire memory bitmap to the main display
    BitBlt(hdc, 0, 0, ps.rcPaint.right, ps.rcPaint.bottom, memDC, 0, 0, SRCCOPY);

    // Restore the original state of the DC
    RestoreDC(hdc, -1);

    // Cleanup
    DeleteObject(hbit);
    DeleteDC(memDC);

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Process WM_COMMAND message for window/dialog: TradeDialog
// ========================================================================================
void TradeDialog_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
    case (IDC_TRADEDIALOG_TXTTICKER):
        if (codeNotify == EN_KILLFOCUS) {
            // Show the Futures contract date field and set the label descriptions if needed.
            TradeDialogControls_ShowFuturesContractDate(hwnd);

            // Attempt to lookup the specified Ticker and fill in the corresponding Company Name & Multiplier.
            std::wstring ticker_symbol = AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTTICKER));
            std::wstring company_name;

            auto iter = std::find_if(trades.begin(), trades.end(),
                [&](const auto t) { return (t->ticker_symbol == ticker_symbol); });

            if (iter != trades.end()) {
                auto index = std::distance(trades.begin(), iter);
                company_name = trades.at(index)->ticker_name;
            }

            AfxSetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTCOMPANY), company_name);

            std::wstring multiplier = GetMultiplier(ticker_symbol);
            if (tdd.trade_action == TradeAction::new_shares_trade ||
                tdd.trade_action == TradeAction::manage_shares ||
                tdd.trade_action == TradeAction::add_shares_to_trade) {
                multiplier = L"1";
            }
            CustomTextBox_SetText(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTMULTIPLIER), multiplier);

        }
        break;


    case (IDC_TRADEDIALOG_TXTQUANTITY):
    case (IDC_TRADEDIALOG_TXTMULTIPLIER):
    case (IDC_TRADEDIALOG_TXTPRICE):
    case (IDC_TRADEDIALOG_TXTFEES):
        if (codeNotify == EN_KILLFOCUS) {
            TradeDialog_CalculateTradeTotal(hwnd);
        }
        break;

    }
}


// ========================================================================================
// Process a change in the DR/CR button (colors and trade totals).
// ========================================================================================
void TradeDialog_SetComboDRCR(HWND hCtl, std::wstring text)
{
    CustomLabel_SetText(hCtl, text);

    DWORD clr = (text == L"CR") ? COLOR_GREEN : COLOR_RED;
    CustomLabel_SetBackColor(hCtl, clr);
    CustomLabel_SetBackColorHot(hCtl, clr);
    TradeDialog_CalculateTradeTotal(HWND_TRADEDIALOG);
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CTradeDialog::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(m_hwnd, WM_CREATE, TradeDialog_OnCreate);
        HANDLE_MSG(m_hwnd, WM_COMMAND, TradeDialog_OnCommand);
        HANDLE_MSG(m_hwnd, WM_DESTROY, TradeDialog_OnDestroy);
        HANDLE_MSG(m_hwnd, WM_CLOSE, TradeDialog_OnClose);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, TradeDialog_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, TradeDialog_OnPaint);

    
    case WM_SHOWWINDOW:
    {
        // Workaround for the Windows 11 (The cloaking solution seems to work only
        // on Windows 10 whereas this WM_SHOWWINDOW workaround seems to only work
        // on Windows 11).
        // https://stackoverflow.com/questions/69715610/how-to-initialize-the-background-color-of-win32-app-to-something-other-than-whit

        SetWindowLongPtr(m_hwnd,
            GWL_EXSTYLE,
            GetWindowLongPtr(m_hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);

        if (!GetLayeredWindowAttributes(m_hwnd, NULL, NULL, NULL))
        {
            SetLayeredWindowAttributes(m_hwnd, 0, 0, LWA_ALPHA);
            DefWindowProc(m_hwnd, WM_ERASEBKGND, (WPARAM)GetDC(m_hwnd), lParam);
            SetLayeredWindowAttributes(m_hwnd, 0, 255, LWA_ALPHA);
            AnimateWindow(m_hwnd, 1, AW_ACTIVATE | AW_BLEND);
            return 0;
        }
        SetWindowLongPtr(m_hwnd,
            GWL_EXSTYLE,
            GetWindowLong(m_hwnd, GWL_EXSTYLE) & ~WS_EX_LAYERED);

        return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
    return 0;


    case WM_KEYDOWN:
    {
        // We are handling the TAB naviagation ourselves.
        if (wParam == VK_TAB) {
            HWND hFocus = GetFocus();
            HWND hNextCtrl = NULL;
            if (GetAsyncKeyState(VK_SHIFT) & 0x8000) { 
                hNextCtrl = GetNextDlgTabItem(m_hwnd, hFocus, TRUE);
            }
            else {
                hNextCtrl = GetNextDlgTabItem(m_hwnd, hFocus, FALSE);
            }
            SetFocus(hNextCtrl);
            return TRUE;
        }
    }
    return 0;

    
    case MSG_DATEPICKER_DATECHANGED:
    {
        TradeGrid_CalculateDTE(GetDlgItem(m_hwnd, IDC_TRADEDIALOG_TABLEGRIDMAIN));
        TradeGrid_CalculateDTE(GetDlgItem(m_hwnd, IDC_TRADEDIALOG_TABLEGRIDROLL));
    }
    return 0;


    case MSG_CUSTOMLABEL_CLICK:
    {
        HWND hCtl = (HWND)lParam;
        int CtrlId = (int)wParam;

        if (hCtl == NULL) return 0;

        if (CtrlId == IDC_TRADEDIALOG_BUYSHARES) {
            TradeDialog_ToggleBuyLongShortText(hCtl);
            TradeDialog_SetLongShortback_color(hCtl);
        }

        if (CtrlId == IDC_TRADEDIALOG_BUYSHARES_DROPDOWN) {
            hCtl = GetDlgItem(m_hwnd, IDC_TRADEDIALOG_BUYSHARES);
            TradeDialog_ToggleBuyLongShortText(hCtl);
            TradeDialog_SetLongShortback_color(hCtl);
        }

        if (CtrlId == IDC_TRADEDIALOG_SELLSHARES) {
            TradeDialog_ToggleSellLongShortText(hCtl);
            TradeDialog_SetLongShortback_color(hCtl);
        }

        if (CtrlId == IDC_TRADEDIALOG_SELLSHARES_DROPDOWN) {
            hCtl = GetDlgItem(m_hwnd, IDC_TRADEDIALOG_SELLSHARES);
            TradeDialog_ToggleSellLongShortText(hCtl);
            TradeDialog_SetLongShortback_color(hCtl);
        }

        if (CtrlId == IDC_TRADEDIALOG_COMBODRCR) {
            // Clicked on the DRCR combo so cycle through the choices
            std::wstring text = CustomLabel_GetText(hCtl);
            text = (text == L"DR") ? L"CR" : L"DR";
            TradeDialog_SetComboDRCR(hCtl, text);
        }

        if (CtrlId == IDC_TRADEDIALOG_CMDTRANSDATE || CtrlId == IDC_TRADEDIALOG_LBLTRANSDATE) {
            // Clicked on the Transaction Date dropdown or label itself
            std::wstring date_text = CustomLabel_GetUserData(GetDlgItem(m_hwnd, IDC_TRADEDIALOG_LBLTRANSDATE));
            Calendar_CreateDatePicker(
                m_hwnd, GetDlgItem(m_hwnd, IDC_TRADEDIALOG_LBLTRANSDATE), date_text, 
                CalendarPickerReturnType::long_date, 2);
        }

        if (CtrlId == IDC_TRADEDIALOG_CMDCONTRACTDATE || CtrlId == IDC_TRADEDIALOG_LBLCONTRACTDATE) {
            // Clicked on the Futures Contract Date dropdown or label itself
            std::wstring date_text = CustomLabel_GetUserData(GetDlgItem(m_hwnd, IDC_TRADEDIALOG_LBLCONTRACTDATE));
            Calendar_CreateDatePicker(
                m_hwnd, GetDlgItem(m_hwnd, IDC_TRADEDIALOG_LBLCONTRACTDATE), date_text, 
                CalendarPickerReturnType::long_date, 2);
        }

        if (CtrlId == IDC_TRADEDIALOG_SAVE) {
            if (IsNewSharesTradeAction(tdd.trade_action) == true ||
                tdd.trade_action == TradeAction::add_shares_to_trade||
                tdd.trade_action == TradeAction::add_futures_to_trade||
                tdd.trade_action == TradeAction::manage_shares ||
                tdd.trade_action == TradeAction::manage_futures) {
                if (TradeDialog_ValidateSharesTradeData(m_hwnd) == true) {
                    TradeDialog_CreateSharesTradeData(m_hwnd);
                    dialog_return_code = DIALOG_RETURN_OK;
                    SendMessage(m_hwnd, WM_CLOSE, 0, 0);
                }
                SetFocus(GetDlgItem(m_hwnd, IDC_TRADEDIALOG_SAVE));
            }
            else if (tdd.trade_action == TradeAction::edit_transaction) {
                if (TradeDialog_ValidateEditTradeData(m_hwnd) == true) {
                    TradeDialog_CreateEditTradeData(m_hwnd);
                    dialog_return_code = DIALOG_RETURN_OK;
                    SendMessage(m_hwnd, WM_CLOSE, 0, 0);
                }
                SetFocus(GetDlgItem(m_hwnd, IDC_TRADEDIALOG_SAVE));
            }
            else {
                if (TradeDialog_ValidateOptionsTradeData(m_hwnd) == true) {
                    TradeDialog_CreateOptionsTradeData(m_hwnd);
                    dialog_return_code = DIALOG_RETURN_OK;
                    SendMessage(m_hwnd, WM_CLOSE, 0, 0);
                }
                SetFocus(GetDlgItem(m_hwnd, IDC_TRADEDIALOG_SAVE));
            }
        }

    return 0;
    }

    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}


// ========================================================================================
// Create and show the Trade modal dialog.
// ========================================================================================
int TradeDialog_Show(TradeAction inTradeAction)
{
    tdd.trade_action = inTradeAction;

    int nWidth = 715;
    int nHeight = 500;

    HWND hwnd = TradeDialog.Create(HWND_MAINWINDOW, L"Trade Management", 0, 0, nWidth, nHeight,
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_CONTROLPARENT | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);

    // Attempt to apply the standard Windows dark theme to the non-client areas of the main form.
    BOOL value = true;
    ::DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));

    HBRUSH hbrBackground = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)hbrBackground);

    HANDLE hIconSmall = LoadImage(TradeDialog.hInst(), MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON, 16, 16, LR_SHARED);
    SendMessage(hwnd, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIconSmall);


    // Show the legsEdit legs (if any) based on the incoming action and set the
    // Ticker and Company Name labels.
    TradeDialog_LoadEditLegsInTradeTable(hwnd);
    
    TradeDialog_CalculateTradeTotal(hwnd);

    // Blur the underlying MainWindow panels in order to reduce "visual noise" while
    // our Trade Management popup is active. The MainWindow panels are shown again
    // during our call to TradeDialog_OnClose() prior to enabling the MainWindow
    // and this popup closing.
    MainWindow_BlurPanels(true);

    AfxCenterWindow(hwnd, HWND_MAINWINDOW);

    EnableWindow(HWND_MAINWINDOW, FALSE);

    // Fix Windows 10 white flashing
    BOOL cloak = TRUE;
    DwmSetWindowAttribute(hwnd, DWMWA_CLOAK, &cloak, sizeof(cloak));

    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);

    cloak = FALSE;
    DwmSetWindowAttribute(hwnd, DWMWA_CLOAK, &cloak, sizeof(cloak));

    SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    
    if (IsNewOptionsTradeAction(tdd.trade_action) == true ||
        IsNewSharesTradeAction(tdd.trade_action) == true) {
        SetFocus(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTTICKER));
    }
    else {
        SetFocus(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTQUANTITY));
    }
     
    dialog_return_code = DIALOG_RETURN_CANCEL;
   
    // Call modal message pump and wait for it to end.
    MSG msg{};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (msg.message == WM_KEYUP && msg.wParam == VK_ESCAPE) {
            if (IsWindowVisible(HWND_CALENDAR)) {
                DestroyWindow(HWND_CALENDAR);
            }
            else if (IsWindowVisible(HWND_STRATEGYPOPUP)) {
                DestroyWindow(HWND_STRATEGYPOPUP);
            }
            else if (IsWindowVisible(HWND_CATEGORYPOPUP)) {
                DestroyWindow(HWND_CATEGORYPOPUP);
            }
            else {
                SendMessage(hwnd, WM_CLOSE, 0, 0);
            }
        }

        // We handle VK_TAB processing ourselves rather than using IsDialogMessage
        // Translates virtual-key messages into character messages.
        TranslateMessage(&msg);
        // Dispatches a message to a window procedure.
        DispatchMessage(&msg);
    }

    // Clear the tdd module global trade variable
    tdd.ResetDefaults();

    return dialog_return_code;
}

