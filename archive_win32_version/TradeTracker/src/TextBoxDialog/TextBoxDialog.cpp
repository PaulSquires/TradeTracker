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

#include "MainWindow/MainWindow.h"
#include "Utilities/Colors.h"
#include "TextBoxDialog.h"


HWND HWND_TEXTBOXDIALOG = NULL;

CTextBoxDialog TextBoxDialog;


// ========================================================================================
// Process WM_SIZE message for window/dialog: TextBoxDialog
// ========================================================================================
void TextBoxDialog_OnSize(HWND hwnd, UINT state, int cx, int cy) {
    
	// Move and size the TextBox into place
	HWND hTextBox = GetDlgItem(hwnd, IDC_TEXTBOXDIALOG_TEXTBOX);

	SetWindowPos(
        GetDlgItem(hwnd, IDC_TEXTBOXDIALOG_TEXTBOX), 
        0, 0, 0, cx, cy, SWP_NOZORDER | SWP_SHOWWINDOW);
}


// ========================================================================================
// Process WM_CLOSE message for window/dialog: TextBoxDialog
// ========================================================================================
void TextBoxDialog_OnClose(HWND hwnd) {
	MainWindow.BlurPanels(false);
	EnableWindow(MainWindow.hWindow, true);
	DestroyWindow(hwnd);
}


// ========================================================================================
// Process WM_DESTROY message for window/dialog: TextBoxDialog
// ========================================================================================
void TextBoxDialog_OnDestroy(HWND hwnd) {
	HFONT hFont = (HFONT)SendMessage(GetDlgItem(hwnd, IDC_TEXTBOXDIALOG_TEXTBOX), WM_GETFONT, 0, 0);
	DeleteFont(hFont);
	PostQuitMessage(0);
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: TextBoxDialog
// ========================================================================================
bool TextBoxDialog_OnEraseBkgnd(HWND hwnd, HDC hdc) {
	// Handle all of the painting in WM_PAINT
	return true;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: TextBoxDialog
// ========================================================================================
void TextBoxDialog_OnPaint(HWND hwnd) {
	PAINTSTRUCT ps;

	HDC hdc = BeginPaint(hwnd, &ps);

	Graphics graphics(hdc);

	// Create the background brush
	SolidBrush back_brush(COLOR_GRAYDARK);

	// Paint the background using brush.
	int width = (ps.rcPaint.right - ps.rcPaint.left);
	int height = (ps.rcPaint.bottom - ps.rcPaint.top);
	graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, width, height);

	EndPaint(hwnd, &ps);
}

// ========================================================================================
// Process WM_CREATE message for window/dialog: TextBoxDialog
// ========================================================================================
bool TextBoxDialog_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct) {
    HWND_TEXTBOXDIALOG = hwnd;

	TextBoxDialog.AddControl(Controls::MultilineTextBox, hwnd, IDC_TEXTBOXDIALOG_TEXTBOX,
		L"", 0, 0, 0, 0,
		WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | ES_LEFT | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_NOHIDESEL | ES_WANTRETURN,
		0);

	return true;
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CTextBoxDialog::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
	static HBRUSH hBackBrush = NULL;

    switch (msg) {
        HANDLE_MSG(m_hwnd, WM_CREATE, TextBoxDialog_OnCreate);
		HANDLE_MSG(m_hwnd, WM_DESTROY, TextBoxDialog_OnDestroy);
		HANDLE_MSG(m_hwnd, WM_CLOSE, TextBoxDialog_OnClose);
        HANDLE_MSG(m_hwnd, WM_SIZE, TextBoxDialog_OnSize);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, TextBoxDialog_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, TextBoxDialog_OnPaint);

	case WM_CTLCOLOREDIT: {
		if (hBackBrush == NULL) hBackBrush = CreateSolidBrush(Color(COLOR_GRAYDARK).ToCOLORREF());
		HDC hdc = (HDC)wParam;
		SetTextColor(hdc, Color(COLOR_WHITELIGHT).ToCOLORREF());
		SetBkColor(hdc, Color(COLOR_GRAYDARK).ToCOLORREF());
		SetBkMode(hdc, OPAQUE);
		return (LRESULT)hBackBrush;
	}

	case WM_NCDESTROY: {
		if (hBackBrush) DeleteBrush(hBackBrush);
		hBackBrush = NULL;
		break;
	}

	case WM_SHOWWINDOW:	{
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

    }
	return DefWindowProc(m_hwnd, msg, wParam, lParam);
}


// ========================================================================================
// Create and show the Reconcile modal dialog.
// ========================================================================================
void TextBoxDialog_Show(HWND hParent, const std::wstring& caption, const std::wstring& text, int width, int height) {

	HWND hwnd = TextBoxDialog.Create(MainWindow.hWindow, caption.c_str(), 0, 0, width, height,
		WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		WS_EX_CONTROLPARENT | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);

	// Attempt to apply the standard Windows dark theme to the non-client areas of the main form.
	BOOL value = TRUE;
	::DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));

	HBRUSH hbrBackground = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
	SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)hbrBackground);

	HANDLE hIconSmall = LoadImage(TextBoxDialog.hInst(), MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON, 16, 16, LR_SHARED);
	SendMessage(hwnd, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIconSmall);

	MainWindow.BlurPanels(true);

	AfxCenterWindowMonitorAware(hwnd, MainWindow.hWindow);

	EnableWindow(MainWindow.hWindow, false);

	// Apply fixed width font for better readability
	HWND hTextBox = GetDlgItem(hwnd, IDC_TEXTBOXDIALOG_TEXTBOX);
	HFONT hFont = (HFONT)SendMessage(hTextBox, WM_GETFONT, 0, 0);
	DeleteFont(hFont);
	hFont = TextBoxDialog.CreateFont(L"Courier New", 10, FW_NORMAL, false, false, false, DEFAULT_CHARSET);
	SendMessage(hTextBox, WM_SETFONT, (WPARAM)hFont, 0);

	// Do the reconciliation
	AfxSetWindowText(hTextBox, text);
	
	// Hide the vertical scrollbar if < 24 lines
	if (Edit_GetLineCount(hTextBox) <= 24) {
		ShowScrollBar(hTextBox, SB_VERT, false);
	}


	// Fix Windows 10 white flashing
	BOOL cloak = TRUE;
	DwmSetWindowAttribute(hwnd, DWMWA_CLOAK, &cloak, sizeof(cloak));

	ShowWindow(HWND_TEXTBOXDIALOG, SW_SHOWNORMAL);
	UpdateWindow(HWND_TEXTBOXDIALOG);

	cloak = FALSE;
	DwmSetWindowAttribute(hwnd, DWMWA_CLOAK, &cloak, sizeof(cloak));

	SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	// Call modal message pump and wait for it to end.
	MSG msg{};
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!IsWindow(hParent)) break;

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

	DeleteFont(hFont);
}

