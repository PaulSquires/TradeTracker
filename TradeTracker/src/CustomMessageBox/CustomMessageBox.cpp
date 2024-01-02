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
#include "CustomMessageBox.h"



// ========================================================================================
// Process WM_CLOSE message for window/dialog: CustomMessageBox
// ========================================================================================
void CCustomMessageBox::OnClose(HWND hwnd) {
	MainWindow.BlurPanels(false);
	EnableWindow(MainWindow.hWindow, true);
	DestroyWindow(hwnd);
}


// ========================================================================================
// Process WM_DESTROY message for window/dialog: CustomMessageBox
// ========================================================================================
void CCustomMessageBox::OnDestroy(HWND hwnd) {
	PostQuitMessage(0);
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: CustomMessageBox
// ========================================================================================
bool CCustomMessageBox::OnEraseBkgnd(HWND hwnd, HDC hdc) {
	// Handle all of the painting in WM_PAINT
	return true;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: CustomMessageBox
// ========================================================================================
void CCustomMessageBox::OnPaint(HWND hwnd) {
	PAINTSTRUCT ps;

	HDC hdc = BeginPaint(hwnd, &ps);

	Graphics graphics(hdc);

	// Create the background brush
	Color bk_clr(back_color);
	SolidBrush back_brush(bk_clr);

	// Paint the background using brush.
	int width = (ps.rcPaint.right - ps.rcPaint.left);
	int height = (ps.rcPaint.bottom - ps.rcPaint.top);
	graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, width, height);

	// Paint the area to the bottom where the buttons reside.
	ps.rcPaint.top = ps.rcPaint.bottom - button_area_height;

	// Set the background brush
	bk_clr.SetValue(back_color_button_area);
	back_brush.SetColor(bk_clr);
	height = (ps.rcPaint.bottom - ps.rcPaint.top);
	graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, width, height);

	EndPaint(hwnd, &ps);
}


// ========================================================================================
// Calculate the RECT needed to hold the message text.
// ========================================================================================
SIZEL CCustomMessageBox::CalculateMessageBoxSize(HWND hwnd) {
	int cx = GetSystemMetrics(SM_CXBORDER) * 2;
	int cy = GetSystemMetrics(SM_CYCAPTION) + (GetSystemMetrics(SM_CYBORDER) * 2);
	
	HDC hdc = GetDC(hwnd);
	SelectFont(hdc, m_hFont);

	RECT rc{};
	DrawText(hdc, message_text.c_str(), -1, &rc, DT_CALCRECT);
	text_width = rc.right;
	text_height = rc.bottom;
	
	// Check if icon is being used
	if (flags & MB_ICONMASK) {
		text_width = max(text_width, icon_width + image_text_margin);
		text_height = max(text_height, icon_height);
	}

	cx += (left_margin + text_width + right_margin);
	cy += (text_height +image_top_margin + image_bottom_margin + button_area_height);

	ReleaseDC(hwnd, hdc);

	return { AfxUnScaleX(cx), AfxUnScaleY(cy) };
}


// ========================================================================================
// Process WM_SIZE message for window/dialog: CustomMessageBox
// ========================================================================================
void CCustomMessageBox::OnSize(HWND hwnd, UINT state, int cx, int cy) {

	// Calculate the message box width and height based on the values set by the user.
	// Minimum width is the number of buttons + margins (will always have at least MB_OK)
	// Maximum message text width will be 350.

	
	int message_height = AfxScaleY(50);
	int message_width_max = AfxScaleY(350);

	int left = left_margin;
	int top = AfxScaleX(32);

	if (hStaticIcon) {
		SetWindowPos(hStaticIcon, 0, left, top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_SHOWWINDOW);
		left += (icon_width + AfxScaleX(10));
	}

	int button_left = 0;
	int button_top = 0;
	int button_width = 80;
	int button_height = 23;

}


// ========================================================================================
// Process WM_CREATE message for window/dialog: CustomMessageBox
// ========================================================================================
bool CCustomMessageBox::OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct) {
	hWindow = hwnd;

	// PLAY MESSAGE BEEP
	int mb_icon = (flags & MB_ICONMASK);
	MessageBeep(mb_icon);

	
	// CREATE AND DISPLAY BUTTONS
	// Buttons are created from right to left
	//    {Button3} {Button2} {Button1}

	int mb_type = (flags & MB_TYPEMASK);
	
	switch (mb_type) {
	case MB_OK:
		hButton1 = CustomLabel_ButtonLabel(hwnd, IDC_MESSAGEBOX_OK, L"OK",
			text_color, back_color, back_color, back_color_down, focus_border_color,
			CustomLabelAlignment::middle_center, 0, 0, 0, 0);
		button_count = 1;
		break;

	case MB_OKCANCEL:
		hButton2 = CustomLabel_ButtonLabel(hwnd, IDC_MESSAGEBOX_OK, L"OK",
			text_color, back_color, back_color, back_color_down, focus_border_color,
			CustomLabelAlignment::middle_center, 0, 0, 0, 0);
		hButton1 = CustomLabel_ButtonLabel(hwnd, IDC_MESSAGEBOX_CANCEL, L"Cancel",
			text_color, back_color, back_color, back_color_down, focus_border_color,
			CustomLabelAlignment::middle_center, 0, 0, 0, 0);
		button_count = 2;
		break;

	case MB_YESNO:
		hButton2 = CustomLabel_ButtonLabel(hwnd, IDC_MESSAGEBOX_YES, L"Yes",
			text_color, back_color, back_color, back_color_down, focus_border_color,
			CustomLabelAlignment::middle_center, 0, 0, 0, 0);
		hButton1 = CustomLabel_ButtonLabel(hwnd, IDC_MESSAGEBOX_NO, L"No",
			text_color, back_color, back_color, back_color_down, focus_border_color,
			CustomLabelAlignment::middle_center, 0, 0, 0, 0);
		button_count = 2;
		break;

	case MB_YESNOCANCEL:
		hButton3 = CustomLabel_ButtonLabel(hwnd, IDC_MESSAGEBOX_CANCEL, L"Cancel",
			text_color, back_color, back_color, back_color_down, focus_border_color,
			CustomLabelAlignment::middle_center, 0, 0, 0, 0);
		hButton2 = CustomLabel_ButtonLabel(hwnd, IDC_MESSAGEBOX_YES, L"Yes",
			text_color, back_color, back_color, back_color_down, focus_border_color,
			CustomLabelAlignment::middle_center, 0, 0, 0, 0);
		hButton1 = CustomLabel_ButtonLabel(hwnd, IDC_MESSAGEBOX_NO, L"No",
			text_color, back_color, back_color, back_color_down, focus_border_color,
			CustomLabelAlignment::middle_center, 0, 0, 0, 0);
		button_count = 3;
		break;
	}

	// CREATE THE MULTILINE STATIC CONTROL THAT DISPLAYS THE MESSAGE TEXT
	hStaticMessage = AddControl(Controls::Label, hwnd, IDC_MESSAGEBOX_STATIC, message_text, 0, 0, 0, 0, 
						WS_VISIBLE | SS_LEFT | WS_GROUP | SS_NOPREFIX | SS_EDITCONTROL, 0);

	// CREATE THE ICON
	HICON hIcon = NULL;
	switch (mb_icon) {
	case MB_ICONERROR:  // same as MB_ICONSTOP, MB_ICONHAND
		LoadIconWithScaleDown(0, IDI_ERROR, icon_width, icon_height, &hIcon);
		break;
	case MB_ICONWARNING:   // same as MB_ICONEXCLAMATION
		LoadIconWithScaleDown(0, IDI_EXCLAMATION, icon_width, icon_height, &hIcon);
		break;
	case MB_ICONINFORMATION:   // same as MB_ICONASTERISK
		LoadIconWithScaleDown(0, IDI_INFORMATION, icon_width, icon_height, &hIcon);
		break;
	case MB_ICONQUESTION:
		LoadIconWithScaleDown(0, IDI_QUESTION, icon_width, icon_height, &hIcon);
		break;
	}
	hStaticIcon = AddControl(Controls::IconLabel, hwnd, IDC_MESSAGEBOX_ICON, L"", 0, 0, icon_width, icon_height);
	if (hIcon) SendMessage(hStaticIcon, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);

	return true;
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CCustomMessageBox::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
	static HBRUSH hBackBrush = CreateSolidBrush(Color(back_color).ToCOLORREF());

	switch (msg) {
		HANDLE_MSG(m_hwnd, WM_CREATE, OnCreate);
		HANDLE_MSG(m_hwnd, WM_DESTROY, OnDestroy);
		HANDLE_MSG(m_hwnd, WM_CLOSE, OnClose);
		HANDLE_MSG(m_hwnd, WM_SIZE, OnSize);
		HANDLE_MSG(m_hwnd, WM_ERASEBKGND, OnEraseBkgnd);
		HANDLE_MSG(m_hwnd, WM_PAINT, OnPaint);

	case WM_CTLCOLORSTATIC: {
		HDC hdc = (HDC)wParam;
		SetTextColor(hdc, Color(text_color).ToCOLORREF());
		SetBkColor(hdc, Color(back_color).ToCOLORREF());
		SetBkMode(hdc, OPAQUE);
		return (LRESULT)hBackBrush;
	}

	case WM_NCDESTROY: {
		if (hBackBrush) DeleteBrush(hBackBrush);
		break;
	}

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

	}
	return DefWindowProc(m_hwnd, msg, wParam, lParam);
}


// ========================================================================================
// Create and show the Reconcile modal dialog.
// ========================================================================================
int CCustomMessageBox::Show(
	HWND hwndParent, const std::wstring& message, const std::wstring& caption, int option_flags)
{
	hParent = hwndParent;
	message_text = message;
	caption_text = caption.length() ? caption : L"TradeTracker";
	flags = option_flags;

	// Need to calculate the width and height needed based on message size, number of
	// buttons and if an icon will be displayed.
	SIZEL size = CalculateMessageBoxSize(hwndParent);

std::cout << "calc: " << size.cx << " " << size.cy << std::endl;
	
	HWND hwnd = Create(hParent, caption_text, 0, 0, size.cx, size.cy,
		WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		WS_EX_DLGMODALFRAME | WS_EX_LEFT | WS_EX_LTRREADING | 
		WS_EX_RIGHTSCROLLBAR | WS_EX_WINDOWEDGE | WS_EX_CONTROLPARENT);

	// Attempt to apply the standard Windows dark theme to the non-client areas of the main form.
	BOOL value = TRUE;
	::DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));

	HBRUSH hbrBackground = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
	SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)hbrBackground);

	MainWindow.BlurPanels(true);

	AfxCenterWindow(hwnd, HWND_DESKTOP);

	EnableWindow(MainWindow.hWindow, false);


	// Fix Windows 10 white flashing
	BOOL cloak = TRUE;
	DwmSetWindowAttribute(hwnd, DWMWA_CLOAK, &cloak, sizeof(cloak));

	ShowWindow(hWindow, SW_SHOWNORMAL);
	UpdateWindow(hWindow);

	cloak = FALSE;
	DwmSetWindowAttribute(hwnd, DWMWA_CLOAK, &cloak, sizeof(cloak));

	SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);


	// SET FOCUS TO ANY DEFINED DEFAULT BUTTON
	int mb_default = (flags & MB_DEFMASK);

	switch (mb_default) {
	case MB_DEFBUTTON1: 
		SetFocus(hButton3); 
		break;
	case MB_DEFBUTTON2: 
		SetFocus(hButton2); 
		break;
	case MB_DEFBUTTON3: 
		SetFocus(hButton1); 
		break;
	default: SetFocus(hButton1);
	}

		
	// Call modal message pump and wait for it to end.
	MSG msg{};
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (msg.message == WM_KEYUP && msg.wParam == VK_ESCAPE) {
			result_code = IDCANCEL;
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

	return result_code;
}

