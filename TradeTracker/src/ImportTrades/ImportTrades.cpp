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

#include "Database/trade.h"
#include "Database/database.h"
#include "CustomLabel/CustomLabel.h"
#include "CustomCalendar/CustomCalendar.h"
#include "CustomTextBox/CustomTextBox.h"
#include "CustomMessageBox/CustomMessageBox.h"
#include "Utilities/UserMessages.h"
#include "Config/Config.h"
#include "tws-api/IntelDecimal/IntelDecimal.h"
#include "MainWindow/MainWindow.h"
#include "MainWindow/tws-client.h"

#include "ImportTrades.h"


HWND HWND_IMPORTDIALOG = NULL;

CImportDialog ImportDialog;

extern int dialog_return_code;

bool show_import_dialog = true;

std::vector<ImportStruct> ibkr;    // persistent 



// ========================================================================================
// Information received from TwsClient::position callback
// ========================================================================================
void ImportTrades_position(const Contract& contract, Decimal position, double avg_cost) {
	// This callback is initiated by the reqPositions() call when the user requests to
	// import existing IBR positions (i.e. the database is empty).

	// Remove any existing version of the contract before adding it again
	auto end = std::remove_if(ibkr.begin(),
		ibkr.end(),
		[contract](ImportStruct const& p) {
			return (p.contract.conId == contract.conId) ? true : false;
		});
	ibkr.erase(end, ibkr.end());

	// Add the position the vector
	ImportStruct p{};
	p.contract = contract;
	p.position = position;
	p.avg_cost = avg_cost;
	ibkr.push_back(p);
}


// ========================================================================================
// Sort all IBKR positions in order to start to group similar trades together.
// ========================================================================================
void ImportTrades_doPositionSorting(){

	// Sort by Symbol, secType, lastTradeDateOrContractMonth
	// Sort based on Category and then TickerSymbol
	std::sort(ibkr.begin(), ibkr.end(),
		[](const auto& trade1, const auto& trade2) {
			{
				if (trade1.contract.symbol < trade2.contract.symbol) return true;
				if (trade2.contract.symbol < trade1.contract.symbol) return false;

				// symbol=symbol for primary condition, go to secType
				if (trade1.contract.secType < trade2.contract.secType) return true;
				if (trade2.contract.secType < trade1.contract.secType) return false;

				// secType=secType for primary condition, go to lastTradeDateOrContractMonth
				if (trade1.contract.lastTradeDateOrContractMonth < trade2.contract.lastTradeDateOrContractMonth) return true;
				if (trade2.contract.lastTradeDateOrContractMonth < trade1.contract.lastTradeDateOrContractMonth) return false;

				return false;
			}
		});

	for (const auto& p : ibkr) {
		std::cout <<
		p.contract.conId << " " <<
		p.contract.symbol << " " <<
		p.contract.secType << " " <<
		p.contract.lastTradeDateOrContractMonth << " " <<
		p.contract.strike << " " <<
		p.contract.right << " " <<
		(int)intelDecimalToDouble(p.position) << " " <<
		p.avg_cost << " " <<
		std::endl;
	}
}


// ========================================================================================
// Ask to initiate importing trades from IBKR TWS.
// ========================================================================================
void ImportTrades_AskImportMessage() {
    if (!show_import_dialog) return;

    std::wstring text =
        L"No trades exist in the local database.\n\n" \
        "Would you like to connect to TWS and import existing positions from your online IBKR account?";

    int res = CustomMessageBox.Show(
        MainWindow.hWindow, text, L"Import Trades", MB_ICONEXCLAMATION | MB_YESNOCANCEL | MB_DEFBUTTON1);

    if (res != IDYES) return;

    client.connection_type = ConnectionType::tws_data_live;
    tws_Connect();
    
    tws_RequestPositions();

    // When positions are finished being received, MSG_POSITIONS_READY is sent to ActiveTrades
    // window. This is where the ImportDialog_Show() call is made.
}


// ========================================================================================
// Process WM_CLOSE message for window/dialog: ImportDialog
// ========================================================================================
void ImportDialog_OnClose(HWND hwnd) {
    if (dialog_return_code == DIALOG_RETURN_OK) {
    }

    MainWindow.BlurPanels(false);
    EnableWindow(MainWindow.hWindow, true);
    DestroyWindow(hwnd);
}


// ========================================================================================
// Process WM_DESTROY message for window/dialog: ImportDialog
// ========================================================================================
void ImportDialog_OnDestroy(HWND hwnd) {
    PostQuitMessage(0);
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: ImportDialog
// ========================================================================================
bool ImportDialog_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct) {
    HWND_IMPORTDIALOG = hwnd;

    std::wstring font_name = AfxGetDefaultFont();
    int font_size = 9;

    ImportTrades_doPositionSorting();

    // SAVE button
    HWND hCtl = CustomLabel_ButtonLabel(hwnd, IDC_IMPORTDIALOG_SAVE, L"SAVE",
        COLOR_BLACK, COLOR_GREEN, COLOR_GREEN, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_center, 580, 390, 80, 23);
    CustomLabel_SetFont(hCtl, font_name, font_size, true);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);

    // CANCEL button
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_IMPORTDIALOG_CANCEL, L"Cancel",
        COLOR_BLACK, COLOR_RED, COLOR_RED, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_center, 580, 423, 80, 23);
    CustomLabel_SetFont(hCtl, font_name, font_size, true);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);

    return true;
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: ImportDialog
// ========================================================================================
bool ImportDialog_OnEraseBkgnd(HWND hwnd, HDC hdc) {
    // Handle all of the painting in WM_PAINT
    return true;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: ImportDialog
// ========================================================================================
void ImportDialog_OnPaint(HWND hwnd) {
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    SaveDC(hdc);

    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP hbit = CreateCompatibleBitmap(hdc, ps.rcPaint.right, ps.rcPaint.bottom);
    SelectBitmap(memDC, hbit);

    Graphics graphics(memDC);
    int width = (ps.rcPaint.right - ps.rcPaint.left);
    int height = (ps.rcPaint.bottom - ps.rcPaint.top);

    // Create the background brush
    SolidBrush back_brush(COLOR_GRAYDARK);
    graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, width, height);

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
// Process WM_COMMAND message for window/dialog: ImportDialog
// ========================================================================================
void ImportDialog_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {
    
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CImportDialog::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        HANDLE_MSG(m_hwnd, WM_CREATE, ImportDialog_OnCreate);
        HANDLE_MSG(m_hwnd, WM_COMMAND, ImportDialog_OnCommand);
        HANDLE_MSG(m_hwnd, WM_DESTROY, ImportDialog_OnDestroy);
        HANDLE_MSG(m_hwnd, WM_CLOSE, ImportDialog_OnClose);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, ImportDialog_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, ImportDialog_OnPaint);

    case WM_SHOWWINDOW: {
        // Workaround for the Windows 11 (The cloaking solution seems to work only
        // on Windows 10 whereas this WM_SHOWWINDOW workaround seems to only work
        // on Windows 11).
        // https://stackoverflow.com/questions/69715610/how-to-initialize-the-background-color-of-win32-app-to-something-other-than-whit

        SetWindowLongPtr(m_hwnd,
            GWL_EXSTYLE,
            GetWindowLongPtr(m_hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);

        if (!GetLayeredWindowAttributes(m_hwnd, NULL, NULL, NULL)) {
            HDC hdc = GetDC(m_hwnd);
            SetLayeredWindowAttributes(m_hwnd, 0, 0, LWA_ALPHA);
            DefWindowProc(m_hwnd, WM_ERASEBKGND, (WPARAM)hdc, lParam);
            SetLayeredWindowAttributes(m_hwnd, 0, 255, LWA_ALPHA);
            AnimateWindow(m_hwnd, 1, AW_ACTIVATE | AW_BLEND);
            ReleaseDC(m_hwnd, hdc);
            return 0;
        }
        SetWindowLongPtr(m_hwnd,
            GWL_EXSTYLE,
            GetWindowLong(m_hwnd, GWL_EXSTYLE) & ~WS_EX_LAYERED);

        return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }

    case WM_KEYDOWN: {
        // We are handling the TAB naviagation ourselves.
        if (wParam == VK_TAB) {
            HWND hFocus = GetFocus();
            HWND hNextCtrl = NULL;
            if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
                hNextCtrl = GetNextDlgTabItem(m_hwnd, hFocus, true);
            }
            else {
                hNextCtrl = GetNextDlgTabItem(m_hwnd, hFocus, false);
            }
            SetFocus(hNextCtrl);
            return true;
        }
        break;
    }

    case MSG_DATEPICKER_DATECHANGED: {
        //TradeGrid_CalculateDTE(GetDlgItem(m_hwnd, IDC_TRADEDIALOG_TABLEGRIDMAIN));
        //TradeGrid_CalculateDTE(GetDlgItem(m_hwnd, IDC_TRADEDIALOG_TABLEGRIDROLL));
        return 0;
    }

    case MSG_CUSTOMLABEL_CLICK: {
        HWND hCtl = (HWND)lParam;
        int ctrl_id = (int)wParam;

        if (!hCtl) return 0;

        //if (ctrl_id == IDC_TRADEDIALOG_COMBODRCR) {
        //}

        if (ctrl_id == IDC_IMPORTDIALOG_SAVE) {
            dialog_return_code = DIALOG_RETURN_OK;
            SendMessage(m_hwnd, WM_CLOSE, 0, 0);
        }

        if (ctrl_id == IDC_IMPORTDIALOG_CANCEL) {
            dialog_return_code = DIALOG_RETURN_CANCEL;
            SendMessage(m_hwnd, WM_CLOSE, 0, 0);
        }

        return 0;
    }

    }

    return DefWindowProc(m_hwnd, msg, wParam, lParam);
}


// ========================================================================================
// Create and show the Import Trades modal dialog.
// ========================================================================================
int ImportDialog_Show() {

    int width = 715;
    int height = 500;

    HWND hwnd = ImportDialog.Create(MainWindow.hWindow, L"Import Trades", 0, 0, width, height,
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_CONTROLPARENT | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);

    // Attempt to apply the standard Windows dark theme to the non-client areas of the main form.
    BOOL value = TRUE;
    ::DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));

    HBRUSH hbrBackground = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)hbrBackground);

    HANDLE hIconSmall = LoadImage(ImportDialog.hInst(), MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON, 16, 16, LR_SHARED);
    SendMessage(hwnd, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIconSmall);

    // Blur the underlying MainWindow panels in order to reduce "visual noise" while
    // our Trade Management popup is active. The MainWindow panels are shown again
    // during our call to TradeDialog_OnClose() prior to enabling the MainWindow
    // and this popup closing.
    MainWindow.BlurPanels(true);

    AfxCenterWindowMonitorAware(hwnd, MainWindow.hWindow);

    EnableWindow(MainWindow.hWindow, false);

    // Fix Windows 10 white flashing
    BOOL cloak = TRUE;
    DwmSetWindowAttribute(hwnd, DWMWA_CLOAK, &cloak, sizeof(cloak));

    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);

    cloak = FALSE;
    DwmSetWindowAttribute(hwnd, DWMWA_CLOAK, &cloak, sizeof(cloak));

    dialog_return_code = DIALOG_RETURN_CANCEL;

    show_import_dialog = false;

    // Call modal message pump and wait for it to end.
    MSG msg{};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (msg.message == WM_KEYUP && msg.wParam == VK_ESCAPE) {
            //if (IsWindowVisible(HWND_CUSTOMCALENDAR)) {
            //    DestroyWindow(HWND_CUSTOMCALENDAR);
            //}
            //else if (IsWindowVisible(HWND_STRATEGYPOPUP)) {
            //    DestroyWindow(HWND_STRATEGYPOPUP);
            //}
            //else {
                SendMessage(hwnd, WM_CLOSE, 0, 0);
            //}
        }

        // We handle VK_TAB processing ourselves rather than using IsDialogMessage
        // Translates virtual-key messages into character messages.
        TranslateMessage(&msg);
        // Dispatches a message to a window procedure.
        DispatchMessage(&msg);
    }

    return dialog_return_code;
}

