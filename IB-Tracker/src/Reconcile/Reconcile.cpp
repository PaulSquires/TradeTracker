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

#include "MainWindow/MainWindow.h"
#include "Database/trade.h"
#include "ActiveTrades/ActiveTrades.h"
#include "tws-api/IntelDecimal/IntelDecimal.h"
#include "Utilities/UserMessages.h"
#include "Config/Config.h"
#include "Reconcile.h"


std::vector<positionStruct> ibkr_positions;    // persistent 
std::vector<positionStruct> local_positions;   // persistent 


HWND HWND_RECONCILE = NULL;

CReconcile Reconcile;

std::wstring results_text;



// ========================================================================================
// Load one open trades into the local_positions vector
// ========================================================================================
void Reconcile_LoadOneLocalPosition(const auto& trade)
{
	for (const auto& leg : trade->open_legs) {

		positionStruct p{};
		p.open_quantity = leg->open_quantity;
		p.ticker_symbol = trade->ticker_symbol;

		if (leg->underlying == L"FUTURES") p.underlying = L"FUT";
		if (leg->underlying == L"OPTIONS") p.underlying = L"OPT";
		if (leg->underlying == L"SHARES") p.underlying = L"STK";

		p.strike_price = AfxValDouble(leg->strike_price);
		p.expiry_date = AfxRemoveDateHyphens(leg->expiry_date);
		p.PutCall = leg->PutCall;

		// Check if the ticker is a future
		if (IsFuturesTicker(p.ticker_symbol)) {
			p.ticker_symbol = trade->ticker_symbol.substr(1);
			if (p.underlying == L"OPT") p.underlying = L"FOP";
		}


		// Check to see if the LOCAL position already exists in the vector. If it
		// does then simply update the Local quantity. We need to do this because IBKR
		// aggregates all similar positions.
		bool found = false;
		for (auto& local : local_positions) {
			if (p.underlying == L"OPT" ||
				p.underlying == L"FOP") {
				if (p.strike_price == local.strike_price &&
					p.ticker_symbol == local.ticker_symbol &&
					p.expiry_date == local.expiry_date &&
					p.PutCall == local.PutCall) {
					found = true;
					local.open_quantity += p.open_quantity;
					local.legs.push_back(leg);
					break;
				}
			}
			if (p.underlying == L"STK" ||
				p.underlying == L"FUT") {
				if (p.ticker_symbol == local.ticker_symbol &&
					p.underlying == local.underlying) {
					found = true;
					local.open_quantity += p.open_quantity;
					local.legs.push_back(leg);
					break;
				}
			}
		}

		if (found == false) {
			p.legs.push_back(leg);
			local_positions.push_back(p);
		}

	}
}


// ========================================================================================
// Load all open trades into the local_positions vector
// ========================================================================================
void Reconcile_LoadAllLocalPositions()
{
	// Add all of the current LOCAL "Open" positions to the local_positions vector
	local_positions.clear();
	for (const auto& trade : trades) {
		if (!trade->is_open) continue;
		Reconcile_LoadOneLocalPosition(trade);
	}
}


// ========================================================================================
// Information received from TwsClient::position callback
// ========================================================================================
void Reconcile_position(const Contract& contract, Decimal position)
{
	// This callback is initiated by the reqPositions() call via the clicking on Reconcile button.
	// This will receive every open position as reported by IBKR. We take these positions and
	// store them in the IBKRPositions vector for later processing.
	// This function is also called whenever the position data on IBKR server changes. We update
	// positions here as they are received.
	
	// Remove any existing version of the contract before adding it again
	auto end = std::remove_if(ibkr_positions.begin(),
		ibkr_positions.end(),
		[contract](positionStruct const& p) {
			return (p.contract_id == contract.conId) ? true : false;
		});
	ibkr_positions.erase(end, ibkr_positions.end());

	// Add the position the vector
	positionStruct p{};
	p.contract_id   = contract.conId;
	p.open_quantity = (int)intelDecimalToDouble(position);
	p.ticker_symbol = ansi2unicode(contract.symbol);
	p.underlying    = ansi2unicode(contract.secType);
	p.expiry_date   = ansi2unicode(contract.lastTradeDateOrContractMonth);   // YYYYMMDD
	p.strike_price  = contract.strike;
	p.PutCall       = ansi2unicode(contract.right);
	// If this is a Lean Hog Futures contract then we multiply the strike by 100 b/c IBKR
	// stores it as cents but we placed the trade as "dollars".  eg. .85 vs. 85
	if (p.ticker_symbol == L"HE" && p.underlying == L"FOP") {
		p.strike_price *= 100;
	}
	if (p.ticker_symbol == L"LE" && p.underlying == L"FOP") {
		p.strike_price *= 100;
	}
	ibkr_positions.push_back(p);
}


// ========================================================================================
// Test if IBKR and LOCAL position are equal
// ========================================================================================
bool Reconcile_ArePositionsEqual(positionStruct ibkr, positionStruct local)
{
	if (ibkr.underlying == L"OPT" ||
		ibkr.underlying == L"FOP") {
		if (ibkr.strike_price == local.strike_price &&
			ibkr.open_quantity == local.open_quantity &&
			ibkr.ticker_symbol == local.ticker_symbol &&
			ibkr.expiry_date == local.expiry_date &&
			ibkr.PutCall == local.PutCall &&
			ibkr.underlying == local.underlying) {
			return true;
		}
	}
	if (ibkr.underlying == L"STK" ||
		ibkr.underlying == L"FUT") {
		if (ibkr.open_quantity == local.open_quantity &&
			ibkr.ticker_symbol == local.ticker_symbol &&
			ibkr.underlying == local.underlying) {
			return true;
		}
	}

	return false;
}



// ========================================================================================
// Information received from TwsClient::positionEnd callback and when Reconciliation
// is requested to be run.
// ========================================================================================
void Reconcile_doPositionMatching()
{
	for (const auto& ibkr : ibkr_positions) {
		if (ibkr.open_quantity == 0) continue;
		bool found = false;
		for (auto& local : local_positions) {
			// Check if the contract has previously already been assigned.
			if (local.contract_id == ibkr.contract_id) break;
			found = Reconcile_ArePositionsEqual(ibkr, local);
			// If found, then update the Local vector to point to the contract_id of the 
			// actual IBKR position. We use this when dealing with UpdatePortfolio() callbacks.
			if (found) {
				local.contract_id = ibkr.contract_id;
				for (auto& leg : local.legs) {
					leg->contract_id = ibkr.contract_id;
				}
				break;
			}
		}
	}
}


// ========================================================================================
// Perform a reconciliation between IBKR positions and local positions.
// ========================================================================================
void Reconcile_doReconciliation()
{
	std::wstring text = L"";
	std::wstring sp = L"  ";

	// (1) Determine what IBKR "real" positions do not exist in the Local database.
	results_text = sp + L"IBKR that do not exist in Local:\r\n";

	for (const auto& ibkr : ibkr_positions) {
		if (ibkr.open_quantity == 0) continue;
		bool found = false;
		for (auto& local : local_positions) {
			found = Reconcile_ArePositionsEqual(ibkr, local);
			if (found) break;
		}
		if (!found) {
			text += sp + 
				AfxRSet(std::to_wstring(ibkr.open_quantity), 8) +
				L"  " +
				AfxLSet(ibkr.ticker_symbol, 8) + 
				AfxLSet(ibkr.underlying, 5);
			if (ibkr.underlying == L"OPT" || ibkr.underlying == L"FOP") {
				text += sp + 
					AfxLSet(AfxInsertDateHyphens(ibkr.expiry_date), 12) + 
					AfxRSet(std::to_wstring(ibkr.strike_price), 16) +
					AfxRSet(ibkr.PutCall, 3);
			}
			text += L"\r\n";
		}
	}
	if (text.length() == 0) text = sp + L"** Everything matches correctly **";
	results_text += text;

	results_text += L"\r\n\r\n";   // blank lines

	// (2) Determine what Local positions do not exist in the IBKR "real" database.
	results_text += sp + L"Local that do not exist in IBKR:\r\n";
	text = L"";
	for (const auto& local : local_positions) {
		bool found = false;
		for (const auto& ibkr : ibkr_positions) {
			found = Reconcile_ArePositionsEqual(ibkr, local);
			if (found == true) break;
		}
		if (!found) {
			if (local.open_quantity != 0) {   // test b/c local may aggregate to zero and may already disappeard from IB
				text += sp +
					AfxRSet(std::to_wstring(local.open_quantity), 8) +
					L"  " +
					AfxLSet(local.ticker_symbol, 8) +
					AfxLSet(local.underlying, 5);
				if (local.underlying == L"OPT" || local.underlying == L"FOP") {
					text += sp +
						AfxLSet(AfxInsertDateHyphens(local.expiry_date), 12) +
						AfxRSet(std::to_wstring(local.strike_price), 16) +
						AfxRSet(local.PutCall, 3);
				}
				text += L"\r\n";
			}
		}
	}
	if (text.length() == 0) text = sp + L"** Everything matches correctly **";
	results_text += text;
	results_text = L"\r\n" + results_text;
}


// ========================================================================================
// Process WM_SIZE message for window/dialog: Reconcile
// ========================================================================================
void Reconcile_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    // Move and size the TextBox into place
    SetWindowPos(
        GetDlgItem(HWND_RECONCILE, IDC_RECONCILE_TEXTBOX), 
        0, 0, 0, cx, cy, SWP_NOZORDER | SWP_SHOWWINDOW);
}


// ========================================================================================
// Process WM_CLOSE message for window/dialog: Reconcile
// ========================================================================================
void Reconcile_OnClose(HWND hwnd)
{
	MainWindow_BlurPanels(false);
	EnableWindow(HWND_MAINWINDOW, TRUE);
	DestroyWindow(hwnd);
}


// ========================================================================================
// Process WM_DESTROY message for window/dialog: Reconcile
// ========================================================================================
void Reconcile_OnDestroy(HWND hwnd)
{
	HFONT hFont = (HFONT)SendMessage(GetDlgItem(hwnd, IDC_RECONCILE_TEXTBOX), WM_GETFONT, 0, 0);
	DeleteFont(hFont);
	PostQuitMessage(0);
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: Reconcile
// ========================================================================================
BOOL Reconcile_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    HWND_RECONCILE = hwnd;

    HWND hCtl =
        Reconcile.AddControl(Controls::MultilineTextBox, hwnd, IDC_RECONCILE_TEXTBOX, 
			L"", 0, 0 ,0 ,0,
			WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | ES_LEFT | ES_AUTOHSCROLL | ES_MULTILINE | ES_NOHIDESEL | ES_WANTRETURN, 
			0);
    return TRUE;
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CReconcile::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HBRUSH hBackBrush = CreateSolidBrush(Color(COLOR_GRAYDARK).ToCOLORREF());

    switch (msg)
    {
        HANDLE_MSG(m_hwnd, WM_CREATE, Reconcile_OnCreate);
		HANDLE_MSG(m_hwnd, WM_DESTROY, Reconcile_OnDestroy);
		HANDLE_MSG(m_hwnd, WM_CLOSE, Reconcile_OnClose);
        HANDLE_MSG(m_hwnd, WM_SIZE, Reconcile_OnSize);


	case WM_CTLCOLOREDIT:
	{
		HDC hdc = (HDC)wParam;
		SetTextColor(hdc, Color(COLOR_WHITELIGHT).ToCOLORREF());
		SetBkColor(hdc, Color(COLOR_GRAYDARK).ToCOLORREF());
		SetBkMode(hdc, OPAQUE);
		return (LRESULT)hBackBrush;
	}
	break;


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
	return 0;


    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}


// ========================================================================================
// Create and show the Reconcile modal dialog.
// ========================================================================================
void Reconcile_Show()
{
	HWND hwnd = Reconcile.Create(HWND_MAINWINDOW, L"Reconcile Local Data to IBKR TWS", 0, 0, 600, 390,
		WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		WS_EX_CONTROLPARENT | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);

	// Attempt to apply the standard Windows dark theme to the non-client areas of the main form.
	BOOL value = true;
	::DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));

	HBRUSH hbrBackground = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
	SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)hbrBackground);

	HANDLE hIconSmall = LoadImage(Reconcile.hInst(), MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON, 16, 16, LR_SHARED);
	SendMessage(hwnd, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIconSmall);

	MainWindow_BlurPanels(true);

	AfxCenterWindow(hwnd, HWND_MAINWINDOW);

	EnableWindow(HWND_MAINWINDOW, FALSE);


	// Apply fixed width font for better readability
	HFONT hFont = (HFONT)SendMessage(GetDlgItem(hwnd, IDC_RECONCILE_TEXTBOX), WM_GETFONT, 0, 0);
	DeleteFont(hFont);
	hFont = Reconcile.CreateFont(L"Courier New", 10, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET);
	SendMessage(GetDlgItem(hwnd, IDC_RECONCILE_TEXTBOX), WM_SETFONT, (WPARAM)hFont, 0);
	AfxSetWindowText(GetDlgItem(hwnd, IDC_RECONCILE_TEXTBOX), L"Hold on a second. Waiting for reconciliation data...");

	// Do the reconciliation
	HWND hTextBox = GetDlgItem(hwnd, IDC_RECONCILE_TEXTBOX);
	Reconcile_LoadAllLocalPositions();   // in case some have been closed/expired/rolled since last time
	Reconcile_doReconciliation();
	AfxSetWindowText(hTextBox, results_text.c_str());
	
	// Hide the vertical scrollbar if < 25 lines
	if (Edit_GetLineCount(hTextBox) <= 25) {
		ShowScrollBar(hTextBox, SB_VERT, false);
	}

	// Fix Windows 10 white flashing
	BOOL cloak = TRUE;
	DwmSetWindowAttribute(hwnd, DWMWA_CLOAK, &cloak, sizeof(cloak));

	ShowWindow(HWND_RECONCILE, SW_SHOWNORMAL);
	UpdateWindow(HWND_RECONCILE);

	cloak = FALSE;
	DwmSetWindowAttribute(hwnd, DWMWA_CLOAK, &cloak, sizeof(cloak));

	SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	// Call modal message pump and wait for it to end.
	MSG msg{};
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (msg.message == WM_KEYUP && msg.wParam == VK_ESCAPE) {
			SendMessage(hwnd, WM_CLOSE, 0, 0);
		}

		// Determines whether a message is intended for the specified
		// dialog box and, if it is, processes the message.
		if (!IsDialogMessage(hwnd, &msg)) {
			// Translates virtual-key messages into character messages.
			TranslateMessage(&msg);
			// Dispatches a message to a window procedure.
			DispatchMessage(&msg);
		}
	}

	results_text = L"";

	DeleteFont(hFont);

}

