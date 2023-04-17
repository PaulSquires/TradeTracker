
#include "pch.h"

#include "..\Themes\Themes.h"
#include "..\MainWindow\MainWindow.h"
#include "..\Database\database.h"
#include "..\Database\trade.h"

#include "Reconcile.h"

extern double intelDecimalToDouble(Decimal decimal);

extern HWND HWND_MAINWINDOW;
extern std::vector<Trade*> trades;



// Structure & vector to hold all positions returned from connection to IBKR (TWS).
// These are used for the reconciliation between IB-Tracker and IBKR.
struct positionStruct {
    int openQuantity = 0;
    std::wstring tickerSymbol;
    std::wstring underlying;
	std::wstring expiryDate;
    double strikePrice = 0;
    std::wstring PutCall;
};
std::vector<positionStruct> IBKRPositions;
std::vector<positionStruct> LocalPositions;


HWND HWND_RECONCILE = NULL;

CReconcile Reconcile;




// ========================================================================================
// Information received from TwsClient::position callback
// ========================================================================================
void Reconcile_position(const Contract& contract, Decimal position)
{
	// This callback is initiated by the reqPositions() call via the clicking on Reconcile button.
	if (position == 0) return;

	positionStruct p;
	p.openQuantity = (int)intelDecimalToDouble(position);
	p.tickerSymbol = ansi2unicode(contract.symbol);
	p.underlying = ansi2unicode(contract.secType);
	p.expiryDate = ansi2unicode(contract.lastTradeDateOrContractMonth);   // YYYYMMDD
	p.strikePrice = contract.strike;
	p.PutCall = ansi2unicode(contract.right);
	IBKRPositions.push_back(p);
}


// ========================================================================================
// Information received from TwsClient::positionEnd callback
// ========================================================================================
void Reconcile_positionEnd()
{
	// Add all of the current "Open" positions to the ReconcilePositions vector
	for (const auto& t : trades) {
		if (!t->isOpen) continue;
		for (const auto& leg : t->openLegs) {
			positionStruct p;
			p.openQuantity = leg->openQuantity;
			p.tickerSymbol = t->tickerSymbol;

			if (leg->underlying == L"OPTIONS") p.underlying = L"OPT";
			if (leg->underlying == L"SHARES") p.underlying = L"STK";
			p.strikePrice = stod(leg->strikePrice);
			p.expiryDate = RemoveDateHyphens(leg->expiryDate);
			p.PutCall = leg->PutCall;


			// If this is a STK then check to see if it already exists in the vector. If it
			// does then simply update the Local quantity. We need to do this because IBKR
			// aggregates all similar stock trades.
			if (p.underlying == L"STK") {
				bool found = false;
				for (auto& l : LocalPositions) {
					if (l.tickerSymbol == p.tickerSymbol) {
						found = true;
						l.openQuantity += p.openQuantity;
						break;
					}
				}
				if (found == false) {
					LocalPositions.push_back(p);
				}
			}
			else {
				LocalPositions.push_back(p);
			}
		}
	}

	std::wstring text;

	// Determine what IBKR "real" positions do not exist in the Local database.
	text = L"IBKR that do not exist in Local:\r\n";
	for (const auto& i : IBKRPositions) {
		if (i.openQuantity == 0) continue;
		bool found = false;
		for (const auto& l : LocalPositions) {
			if (i.underlying == L"OPT") {
				if (i.strikePrice == l.strikePrice &&
					i.openQuantity == l.openQuantity &&  
					i.tickerSymbol == l.tickerSymbol &&
					i.expiryDate == l.expiryDate &&
					i.PutCall == l.PutCall &&
					i.underlying == l.underlying) {
					found = true;
					break;
				}
			}
			if (i.underlying == L"STK") {
				if (i.openQuantity == l.openQuantity &&
					i.tickerSymbol == l.tickerSymbol &&
					i.underlying == l.underlying) {
					found = true;
					break;
				}
			}
		}
		if (!found) {
			text = text + L"   " + std::to_wstring(i.openQuantity) + L" " +
				i.tickerSymbol + L" " + i.underlying;
			if (i.underlying == L"OPT") {
				text = text + L" " + i.expiryDate + L" " + std::to_wstring(i.strikePrice) + L" " + i.PutCall;
			}
			text = text + L"\r\n";
		}
	}

	text = text + L"\r\n";   // blank line

	// Determine what Local positions do not exist in the IBKR "real" database.
	text = text + L"Local that do not exist in IBKR:\r\n";
	for (const auto& l : LocalPositions) {
		bool found = false;
		for (const auto& i : IBKRPositions) {
			if (i.underlying == L"OPT") {
				if (i.strikePrice == l.strikePrice &&
					i.openQuantity == l.openQuantity &&
					i.tickerSymbol == l.tickerSymbol &&
					i.expiryDate == l.expiryDate &&
					i.PutCall == l.PutCall &&
					i.underlying == l.underlying) {
					found = true;
					break;
				}
			}
			if (i.underlying == L"STK") {
				if (i.openQuantity == l.openQuantity &&
					i.tickerSymbol == l.tickerSymbol &&
					i.underlying == l.underlying) {
					found = true;
					break;
				}
			}
		}
		if (!found) {
			if (l.openQuantity > 0) {   // test b/c local may aggregate to zero and may already disappeard from IB
				text = text + L"   " + std::to_wstring(l.openQuantity) + L" " +
					l.tickerSymbol + L" " + l.underlying;
				if (l.underlying == L"OPT") {
					text = text + L" " + l.expiryDate + L" " + std::to_wstring(l.strikePrice) + L" " + l.PutCall;
				}
				text = text + L"\r\n";
			}
		}
	}

	// Clear the vectors in case we run reconcile again
	LocalPositions.clear();
	IBKRPositions.clear();

	Reconcile_Show(text);
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
void Reconcile_Show(std::wstring& wszText)
{
	HWND hwnd = Reconcile.Create(HWND_MAINWINDOW, L"Reconcile Local Data to IBKR TWS", 0, 0, 600, 390,
		WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		WS_EX_CONTROLPARENT | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);

	HANDLE hIconSmall = LoadImage(Reconcile.hInst(), MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON, 16, 16, LR_SHARED);
	SendMessage(hwnd, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIconSmall);

	AfxCenterWindow(hwnd, HWND_MAINWINDOW);

	EnableWindow(HWND_MAINWINDOW, FALSE);

	AfxSetWindowText(GetDlgItem(hwnd, IDC_RECONCILE_TEXTBOX), wszText.c_str());

	// Apply fixed width font for better readability
	HFONT hFont = Reconcile.CreateFont(L"Courier New", 10, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET);
	SendMessage(GetDlgItem(hwnd, IDC_RECONCILE_TEXTBOX), WM_SETFONT, (WPARAM)hFont, 0);


	ShowWindow(hwnd, SW_SHOWNORMAL);

	// Call modal message pump and wait for it to end.
	MSG msg{};
	while (GetMessage(&msg, NULL, 0, 0))
	{
		// Determines whether a message is intended for the specified
		// dialog box and, if it is, processes the message.
		if (!IsDialogMessage(hwnd, &msg)) {
			// Translates virtual-key messages into character messages.
			TranslateMessage(&msg);
			// Dispatches a message to a window procedure.
			DispatchMessage(&msg);
		}
	}

	DeleteFont(hFont);

}

