
#include "pch.h"
#include "Themes.h"
#include "..\Utilities\AfxWin.h"
#include "..\Utilities\CWindowBase.h"


HWND hWndMainWindow = NULL;
bool IsInitialized = false;

std::wstring wszThemeName = L"Dark";
bool IsThemeDark = true;
ActiveThemeColor ActiveTheme = ActiveThemeColor::Dark;

std::vector<DWORD> clr;


// ========================================================================================
// Initialize DARK theme element colors
// ========================================================================================
void InitializeDarkThemeColors()
{
	IsInitialized = true;

	clr.clear();
	clr.reserve((int)ThemeElement::Count);

	SetIsThemeDark(true);

	ActiveTheme = ActiveThemeColor::Dark;

	clr[(int)ThemeElement::Black] = Color::MakeARGB(255, 0, 0, 0);
	clr[(int)ThemeElement::GrayDark] = Color::MakeARGB(255, 38, 38, 38);
	clr[(int)ThemeElement::GrayMedium] = Color::MakeARGB(255, 51, 51, 51);
	clr[(int)ThemeElement::GrayLight] = Color::MakeARGB(255, 68, 68, 68);

	clr[(int)ThemeElement::WhiteDark] = Color::Gray;
	clr[(int)ThemeElement::WhiteMedium] = Color::MakeARGB(255, 157, 165, 180);
	clr[(int)ThemeElement::WhiteLight] = Color::MakeARGB(255, 212, 212, 212);

	clr[(int)ThemeElement::Magenta] = Color::Magenta;
	clr[(int)ThemeElement::Yellow] = Color::Yellow;
	clr[(int)ThemeElement::Green] = Color::MakeARGB(255, 72, 151, 13);
	clr[(int)ThemeElement::Red] = Color::Firebrick;  // Color::MakeARGB(255, 255, 30, 0);
	clr[(int)ThemeElement::Orange] = Color::MakeARGB(255, 193, 98, 24);  // burnt orange
	clr[(int)ThemeElement::Purple] = Color::MediumOrchid;
	clr[(int)ThemeElement::Blue] = Color::CornflowerBlue;

	clr[(int)ThemeElement::MenuNotch] = Color::MakeARGB(255, 38, 38, 38);
	clr[(int)ThemeElement::Separator] = Color::MakeARGB(255, 53, 59, 69);
	clr[(int)ThemeElement::Selection] = Color::MakeARGB(255, 44, 49, 58);

	clr[(int)ThemeElement::ScrollBarBack] = Color::MakeARGB(255, 38, 38, 38);
	clr[(int)ThemeElement::ScrollBarDivider] = Color::MakeARGB(255, 64, 67, 73);
	clr[(int)ThemeElement::ScrollBarThumb] = Color::MakeARGB(255, 51, 51, 51);

}


// ========================================================================================
// Initialize LIGHT theme element colors
// ========================================================================================
void InitializeLightThemeColors()
{
	IsInitialized = true;

	clr.clear();
	clr.reserve((int)ThemeElement::Count);

	SetIsThemeDark(false);

	ActiveTheme = ActiveThemeColor::Light;

	clr[(int)ThemeElement::Black] = Color::MakeARGB(255, 243, 243, 243);
	clr[(int)ThemeElement::GrayDark] = Color::MakeARGB(255, 255, 255, 255);
	clr[(int)ThemeElement::GrayMedium] = Color::MakeARGB(255, 224, 224, 224);
	clr[(int)ThemeElement::GrayLight] = Color::MakeARGB(255, 238, 238, 238);

	clr[(int)ThemeElement::WhiteDark] = Color::MakeARGB(255, 158, 158, 158);
	clr[(int)ThemeElement::WhiteMedium] = Color::MakeARGB(255, 157, 165, 180);
	clr[(int)ThemeElement::WhiteLight] = Color::MakeARGB(255, 97, 97, 97);

	clr[(int)ThemeElement::MenuNotch] = Color::MakeARGB(255, 255, 255, 255);
	clr[(int)ThemeElement::Selection] = Color::MakeARGB(255, 232, 232, 232);
	clr[(int)ThemeElement::Separator] = Color::MakeARGB(255, 157, 165, 180);

	clr[(int)ThemeElement::Magenta] = Color::MakeARGB(255, 255, 0, 255);
	clr[(int)ThemeElement::Yellow] = Color::MakeARGB(255, 255, 214, 0);
	clr[(int)ThemeElement::Green] = Color::MakeARGB(255, 72, 151, 13);
	clr[(int)ThemeElement::Red] = Color::MakeARGB(255, 255, 30, 0);
	clr[(int)ThemeElement::Orange] = Color::MakeARGB(255, 193, 98, 24);  // burnt orange

	clr[(int)ThemeElement::ScrollBarBack] = Color::MakeARGB(255, 38, 38, 38);
	clr[(int)ThemeElement::ScrollBarDivider] = Color::MakeARGB(255, 64, 67, 73);
	clr[(int)ThemeElement::ScrollBarThumb] = Color::MakeARGB(255, 51, 51, 51);
}



// ========================================================================================
// Set the current Theme based on it's string name (std::wstring).
// This value will have been read from the Config file or set via clicking
// on the Dark Mode label on the MainWindow.
// ========================================================================================
void SetThemeName(std::wstring wszTheme)
{
	if (AfxWStringCompareI(wszTheme, L"LIGHT")) {
		ActiveTheme = ActiveThemeColor::Light;
		wszThemeName = L"Light";
		InitializeLightThemeColors();
	}
	else {
		wszThemeName = L"Dark";
		InitializeDarkThemeColors();
	}
}



// ========================================================================================
// Get the current Theme name.
// ========================================================================================
std::wstring GetThemeName()
{
	return AfxTrim(wszThemeName);
}



// ========================================================================================
// Set the current IsThemeDark variable. This is used to determine whether
// to color the application's caption bar using light or dark Windows theme.
// ========================================================================================
void SetIsThemeDark(bool isDark)
{
	IsThemeDark = isDark;
}



// ========================================================================================
// Set the current IsThemeDark variable. This is used to determine whether
// to color th application's caption bar using light or dark Windows theme.
// ========================================================================================
bool GetIsThemeDark()
{
	return IsThemeDark;
}



// ========================================================================================
// Get the specified Theme element Color based on the current active theme.
// ========================================================================================
DWORD GetThemeColor(ThemeElement element)
{
	if (!IsInitialized) InitializeDarkThemeColors();

	// Retrieve the ARGB color value for the specified element
	// based on the current active theme.
	return clr[(int)element];
}



// ========================================================================================
// Get the specified Theme element COLORREF based on the current active theme.
// Used for interfacing to GDI based api. 
// ========================================================================================
COLORREF GetThemeCOLORREF(ThemeElement element)
{
	if (!IsInitialized) InitializeDarkThemeColors();

	// Retrieve the COLORREF color value for the specified element
	// based on the current active theme.
	Color color(GetThemeColor(element));
	return color.ToCOLORREF();
}



// ========================================================================================
// Save the Window handle of the application's main window so that ApplyActiveTheme
// can enumerate all child windows in order to apply Theme changes.
// ========================================================================================
void SetThemeMainWindow(HWND hWndMain)
{
	hWndMainWindow = hWndMain;
}



// ========================================================================================
// Apply the active/current Theme to all visual windows in real time.
// ========================================================================================
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
	if (IsWindowVisible(hwnd)) AfxRedrawWindow(hwnd);
	return TRUE;
}

void ApplyActiveTheme()
{
	if (!IsInitialized) InitializeDarkThemeColors();

	// Enumerate through all top level and child windows to invalidate
	// and repaint them with the new current active theme.
	//if (hWndMainWindow != NULL) {
//		AfxRedrawWindow(hWndMainWindow);
	//	EnumChildWindows(hWndMainWindow, EnumWindowsProc, 0);
	//}
	
	if (hWndMainWindow != NULL) {
		// If we are using a dark theme then attempt to apply the standard Windows dark theme
		// to the non-client areas of the main form.
		BOOL value = GetIsThemeDark();
		::DwmSetWindowAttribute(hWndMainWindow, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));

		// The normal methods to invalidate the non-client area does not seem to force the
		// theme titlebar to repaint. Need to hide/show the window (not ideal). However, one
		// good benefit is that the child windows will be forced to repaint when shown thereby
		// eliminating our need to EnumChildWindows.
		ShowWindow(hWndMainWindow, SW_HIDE);
		ShowWindow(hWndMainWindow, SW_SHOW);

		//RedrawWindow(hWndMainWindow, NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);
		//SetWindowPos(hWndMainWindow, 0, 0, 0, 0, 0,
		//	SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
	}
}



