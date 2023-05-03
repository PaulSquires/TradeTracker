
#include "pch.h"
#include "Themes.h"
#include "..\Utilities\AfxWin.h"


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

	clr[(int)ThemeElement::MenuPanelBack] = Color::MakeARGB(255, 0, 0, 0);
	clr[(int)ThemeElement::MenuPanelBackHot] = Color::MakeARGB(255, 44, 49, 58);
	clr[(int)ThemeElement::MenuPanelText] = Color::MakeARGB(255, 212, 212, 212);
	clr[(int)ThemeElement::MenuPanelTextDim] = Color::MakeARGB(255, 157, 165, 180);
	clr[(int)ThemeElement::MenuPanelTextHot] = Color::MakeARGB(255, 212, 212, 212);
	clr[(int)ThemeElement::MenuPanelSeparator] = Color::MakeARGB(255, 53, 59, 69);
	clr[(int)ThemeElement::MenuPanelBackSelected] = Color::MakeARGB(255, 44, 49, 58);
	clr[(int)ThemeElement::MenuPanelTextSelected] = Color::MakeARGB(255, 212, 212, 212);
	clr[(int)ThemeElement::MenuPanelSelector] = Color::MakeARGB(255, 38, 38, 38);

	clr[(int)ThemeElement::TradesPanelBack] = Color::MakeARGB(255, 38, 38, 38);
	clr[(int)ThemeElement::TradesPanelBackHot] = Color::MakeARGB(255, 44, 49, 58);
	clr[(int)ThemeElement::TradesPanelText] = Color::MakeARGB(255, 212, 212, 212);
	clr[(int)ThemeElement::TradesPanelTextDim] = Color::Gray;
	clr[(int)ThemeElement::TradesPanelColBackLight] = Color::MakeARGB(255, 68, 68, 68);
	clr[(int)ThemeElement::TradesPanelColBackDark] = Color::MakeARGB(255, 51, 51, 51);
	clr[(int)ThemeElement::TradesPanelNormalDTE] = Color::Magenta;
	clr[(int)ThemeElement::TradesPanelWarningDTE] = Color::Yellow;
	clr[(int)ThemeElement::TradesPanelScrollBarBack] = Color::MakeARGB(255, 38, 38, 38);
	clr[(int)ThemeElement::TradesPanelScrollBarLine] = Color::MakeARGB(255, 64, 67, 73);
	clr[(int)ThemeElement::TradesPanelScrollBarThumb] = Color::MakeARGB(255, 51, 51, 51);
	clr[(int)ThemeElement::TradesPanelHistoryText] = Color::MakeARGB(255, 193, 98, 24);  // burnt orange

	clr[(int)ThemeElement::ListHeaderBack] = Color::MakeARGB(255, 51, 51, 51);
	clr[(int)ThemeElement::ListHeaderText] = Color::MakeARGB(255, 212, 212, 212);
	clr[(int)ThemeElement::ListHeaderBorder] = Color::MakeARGB(255, 64, 67, 73);

	clr[(int)ThemeElement::valuePositive] = Color::MakeARGB(255, 72, 151, 13);
	clr[(int)ThemeElement::valueNegative] = Color::MakeARGB(255, 255, 30, 0);
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

	clr[(int)ThemeElement::MenuPanelBack] = Color::MakeARGB(255, 243, 243, 243);
	clr[(int)ThemeElement::MenuPanelBackHot] = Color::MakeARGB(255, 232, 232, 232);
	clr[(int)ThemeElement::MenuPanelText] = Color::MakeARGB(255, 97, 97, 97);
	clr[(int)ThemeElement::MenuPanelTextDim] = Color::MakeARGB(255, 157, 165, 180);
	clr[(int)ThemeElement::MenuPanelTextHot] = Color::MakeARGB(255, 97, 97, 97);
	clr[(int)ThemeElement::MenuPanelSeparator] = Color::MakeARGB(255, 157, 165, 180);
	clr[(int)ThemeElement::MenuPanelBackSelected] = Color::MakeARGB(255, 232, 232, 232);
	clr[(int)ThemeElement::MenuPanelTextSelected] = Color::MakeARGB(255, 97, 97, 97);
	clr[(int)ThemeElement::MenuPanelSelector] = Color::MakeARGB(255, 255, 255, 255);

	clr[(int)ThemeElement::TradesPanelBack] = Color::MakeARGB(255, 255, 255, 255);
	clr[(int)ThemeElement::TradesPanelBackHot] = Color::MakeARGB(255, 232, 232, 232);
	clr[(int)ThemeElement::TradesPanelText] = Color::MakeARGB(255, 128, 128, 128);
	clr[(int)ThemeElement::TradesPanelTextDim] = Color::MakeARGB(255, 158, 158, 158);
	clr[(int)ThemeElement::TradesPanelColBackLight] = Color::MakeARGB(255, 238, 238, 238);
	clr[(int)ThemeElement::TradesPanelColBackDark] = Color::MakeARGB(255, 224, 224, 224);
	clr[(int)ThemeElement::TradesPanelNormalDTE] = Color::MakeARGB(255, 255, 0, 255);
	clr[(int)ThemeElement::TradesPanelWarningDTE] = Color::MakeARGB(255, 255, 214, 0);
	clr[(int)ThemeElement::TradesPanelScrollBarBack] = Color::MakeARGB(255, 38, 38, 38);
	clr[(int)ThemeElement::TradesPanelScrollBarLine] = Color::MakeARGB(255, 64, 67, 73);
	clr[(int)ThemeElement::TradesPanelScrollBarThumb] = Color::MakeARGB(255, 51, 51, 51);
	clr[(int)ThemeElement::TradesPanelHistoryText] = Color::MakeARGB(255, 193, 98, 24);  // burnt orange

	clr[(int)ThemeElement::ListHeaderBack] = Color::MakeARGB(255, 51, 51, 51);
	clr[(int)ThemeElement::ListHeaderText] = Color::MakeARGB(255, 212, 212, 212);
	clr[(int)ThemeElement::ListHeaderBorder] = Color::MakeARGB(255, 64, 67, 73);

	clr[(int)ThemeElement::valuePositive] = Color::MakeARGB(255, 72, 151, 13);
	clr[(int)ThemeElement::valueNegative] = Color::MakeARGB(255, 255, 30, 0);
}



// ========================================================================================
// Set the current Theme based on it's string name (std::wstring).
// This value will have been read from the Config file.
// ========================================================================================
void SetThemeName(std::wstring wszTheme)
{
	wszTheme = AfxUpper(wszTheme);

	if (wszTheme == L"LIGHT") {
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
	if (hWndMainWindow != NULL) {
		AfxRedrawWindow(hWndMainWindow);
		EnumChildWindows(hWndMainWindow, EnumWindowsProc, 0);
	}
}



