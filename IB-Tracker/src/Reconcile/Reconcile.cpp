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
#include "Utilities/IntelDecimal.h"
#include "Utilities/UserMessages.h"
#include "Config/Config.h"
#include "Reconcile.h"


std::vector<positionStruct> IBKRPositions;    // stays persistent
std::vector<positionStruct> local_positions;  // will get cleared


HWND HWND_RECONCILE = NULL;

CReconcile Reconcile;

std::wstring results_text;


// ========================================================================================
// Information received from TwsClient::position callback
// ========================================================================================
void Reconcile_position(const Contract& contract, Decimal position)
{
	// This callback is initiated by the reqPositions() call via the clicking on Reconcile button.
	// This will receive every open position as reported by IBKR. We take these positions and
	// store them in the IBKRPositions vector for later processing.
	
	// If position is zero then the contract has been closed so find it in our existing
	// vector and remove it.
	if (position == 0) {
		auto end = std::remove_if(IBKRPositions.begin(),
			IBKRPositions.end(),
			[contract](positionStruct const& p) {
				return (p.contract_id == contract.conId) ? true : false;
			});

		IBKRPositions.erase(end, IBKRPositions.end());
		return;
	}


	// This is a new position so add it to the vector
	positionStruct p{};
	p.contract_id   = contract.conId;
	p.open_quantity = (int)intelDecimalToDouble(position);
	p.ticker_symbol = ansi2unicode(contract.symbol);
	p.underlying    = ansi2unicode(contract.secType);
	p.expiry_date   = ansi2unicode(contract.lastTradeDateOrContractMonth);   // YYYYMMDD
	p.strike_price  = contract.strike;
	// If this is a Lean Hog Futures contract then we multiply the strike by 100 b/c IBKR
	// stores it as cents but we placed the trade as "dollars".  eg. .85 vs. 85
	if (p.ticker_symbol == L"HE" && p.underlying == L"FOP") {
		p.strike_price *= 100;
	}
	p.PutCall       = ansi2unicode(contract.right);
	IBKRPositions.push_back(p);
}


// ========================================================================================
// Test if IBKR and LOCAL position are equal
// ========================================================================================
bool Reconcile_ArePositionsEqual(positionStruct ibkr, positionStruct local)
{
	bool equal = false;

	if (ibkr.underlying == L"OPT" ||
		ibkr.underlying == L"FOP") {
		if (ibkr.strike_price == local.strike_price &&
			ibkr.open_quantity == local.open_quantity &&
			ibkr.ticker_symbol == local.ticker_symbol &&
			ibkr.expiry_date == local.expiry_date &&
			ibkr.PutCall == local.PutCall &&
			ibkr.underlying == local.underlying) {
			equal = true;
		}
	}
	if (ibkr.underlying == L"STK" ||
		ibkr.underlying == L"FUT") {
		if (ibkr.open_quantity == local.open_quantity &&
			ibkr.ticker_symbol == local.ticker_symbol &&
			ibkr.underlying == local.underlying) {
			equal = true;
		}
	}

	return equal;
}



// ========================================================================================
// Information received from TwsClient::positionEnd callback and when Reconciliation
// is requested to be run.
// ========================================================================================
void Reconcile_doPositionMatching()
{
	// Add all of the current LOCAL "Open" positions to the local_positions vector
	for (const auto& trade : trades) {
		if (!trade->is_open) continue;
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
					if (p.strike_price  == local.strike_price &&
						p.ticker_symbol == local.ticker_symbol &&
						p.expiry_date   == local.expiry_date &&
						p.PutCall      == local.PutCall) {
						found = true;
						local.open_quantity += p.open_quantity;
						local.legs.push_back(leg);
						break;
					}
				}
				if (p.underlying == L"STK" ||
					p.underlying == L"FUT") {
					if (p.ticker_symbol == local.ticker_symbol &&
						p.underlying   == local.underlying) {
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


	std::wstring text = L"";

	// (1) Determine what IBKR "real" positions do not exist in the Local database.
	results_text = L"IBKR that do not exist in Local:\r\n";

	for (const auto& ibkr : IBKRPositions) {
		if (ibkr.open_quantity == 0) continue;
		bool found = false;
		for (auto& local : local_positions) {
			found = Reconcile_ArePositionsEqual(ibkr, local);
			// If found, then update the Local vector to point to the contract_id of the 
			// actual IBKR psoition. We use this when dealing with UpdatePortfolio() callbacks.
			if (found == true) {
				for (auto& leg : local.legs) {
					leg->contract_id = ibkr.contract_id;
				}
				break;
			}
		}
		if (!found) {
			text += L"   " + std::to_wstring(ibkr.open_quantity) + L" " +
				ibkr.ticker_symbol + L" " + ibkr.underlying;
			if (ibkr.underlying == L"OPT" || ibkr.underlying == L"FOP") {
				text += L" " + ibkr.expiry_date + L" " + std::to_wstring(ibkr.strike_price) + L" " + ibkr.PutCall;
			}
			text += L"\r\n";
		}
	}
	if (text.length() == 0) text = L"** Everything matches correctly **";
	results_text += text;

	results_text += L"\r\n\r\n";   // blank lines

	// (2) Determine what Local positions do not exist in the IBKR "real" database.
	results_text += L"Local that do not exist in IBKR:\r\n";
	text = L"";
	for (const auto& local : local_positions) {
		bool found = false;
		for (const auto& ibkr : IBKRPositions) {
			found = Reconcile_ArePositionsEqual(ibkr, local);
			if (found == true) break;
		}
		if (!found) {
			if (local.open_quantity != 0) {   // test b/c local may aggregate to zero and may already disappeard from IB
				text += L"   " + std::to_wstring(local.open_quantity) + L" " +
					local.ticker_symbol + L" " + local.underlying;
				if (local.underlying == L"OPT" || local.underlying == L"FOP") {
					text += L" " + local.expiry_date + L" " + std::to_wstring(local.strike_price) + L" " + local.PutCall;
				}
				text += L"\r\n";
			}
		}
	}
	if (text.length() == 0) text = L"** Everything matches correctly **";
	results_text += text;

	// Clear the local vector in case we run reconcile again. The IBKR vector will
	// be dynamically updated as positions change in TWS and it callback to our program.
	local_positions.clear();

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
	EnableWindow(HWND_MAINWINDOW, TRUE);
	DestroyWindow(hwnd);
}


// ========================================================================================
// Process WM_DESTROY message for window/dialog: Reconcile
// ========================================================================================
void Reconcile_OnDestroy(HWND hwnd)
{
	PostQuitMessage(0);
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: Reconcile
// ========================================================================================
BOOL Reconcile_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    HWND_RECONCILE = hwnd;

    HWND hCtl =
        Reconcile.AddControl(Controls::MultilineTextBox, hwnd, IDC_RECONCILE_TEXTBOX);

    return TRUE;
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CReconcile::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(m_hwnd, WM_CREATE, Reconcile_OnCreate);
		HANDLE_MSG(m_hwnd, WM_DESTROY, Reconcile_OnDestroy);
		HANDLE_MSG(m_hwnd, WM_CLOSE, Reconcile_OnClose);
        HANDLE_MSG(m_hwnd, WM_SIZE, Reconcile_OnSize);

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

	HBRUSH hbrBackground = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
	SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)hbrBackground);

	HANDLE hIconSmall = LoadImage(Reconcile.hInst(), MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON, 16, 16, LR_SHARED);
	SendMessage(hwnd, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIconSmall);


	AfxCenterWindow(hwnd, HWND_MAINWINDOW);

	EnableWindow(HWND_MAINWINDOW, FALSE);

	// Apply fixed width font for better readability
	HFONT hFont = Reconcile.CreateFont(L"Courier New", 10, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET);
	SendMessage(GetDlgItem(hwnd, IDC_RECONCILE_TEXTBOX), WM_SETFONT, (WPARAM)hFont, 0);
	AfxSetWindowText(GetDlgItem(hwnd, IDC_RECONCILE_TEXTBOX), L"Hold on a second. Waiting for reconciliation data...");

	// Do the reconciliation matching
	Reconcile_doPositionMatching();
	AfxSetWindowText(GetDlgItem(hwnd, IDC_RECONCILE_TEXTBOX), results_text.c_str());


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

