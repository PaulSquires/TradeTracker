
#include "pch.h"
#include "Themes.h"
#include "..\Utilities\CWindow.h"


HWND hWndMainWindow = NULL;
bool IsInitialized = false;

DWORD clr[(int)ThemeElement::Count][(int)Themes::Count];
Themes ActiveTheme = Themes::Dark;

std::wstring wszTraderName;



// ========================================================================================
// Initialize all theme and all theme element colors
// ========================================================================================
void InitializeThemeColors()
{
	IsInitialized = true;

	Initialize_Light_Theme(clr);
	Initialize_Dark_Theme(clr);
	Initialize_DarkPlus_Theme(clr);
	Initialize_Blue_Theme(clr);

}


// ========================================================================================
// Get the specified Theme element Color based on the current active theme.
// ========================================================================================
DWORD GetThemeColor(ThemeElement element)
{
	if (!IsInitialized) InitializeThemeColors();

	// Retrieve the ARGB color value for the specified element
	// based on the current active theme.
	return clr[(int)element][(int)ActiveTheme];
}


// ========================================================================================
// Get the specified Theme element COLORREF based on the current active theme.
// Used for interfacing to GDI based api. 
// ========================================================================================
COLORREF GetThemeCOLORREF(ThemeElement element)
{
	if (!IsInitialized) InitializeThemeColors();

	// Retrieve the COLORREF color value for the specified element
	// based on the current active theme.
	Color color(GetThemeColor(element));
	return color.ToCOLORREF();
}


// ========================================================================================
// Set the internal structures to use the incoming Theme.
// ========================================================================================
void SetTheme(Themes theme)
{
	if (!IsInitialized) InitializeThemeColors();
	ActiveTheme = theme;
}


// ========================================================================================
// Return the current set Theme (Themes enum).
// ========================================================================================
Themes GetTheme()
{
	if (!IsInitialized) InitializeThemeColors();
	return ActiveTheme;
}


// ========================================================================================
// Return the current set Theme (std::wstring).
// ========================================================================================
std::wstring GetThemeName()
{
	if (!IsInitialized) InitializeThemeColors();

	switch (ActiveTheme)
	{
	case Themes::Light:
		return L"Light";
		break;
	case Themes::Dark:
		return L"Dark";
		break;
	case Themes::DarkPlus:
		return L"DarkPlus";
		break;
	case Themes::Blue:
		return L"Blue";
		break;
	default:
		return L"Dark";
	}
}


// ========================================================================================
// Set the current Theme based on it's string name (std::wstring).
// ========================================================================================
void SetThemeName(std::wstring wszTheme)
{
	if (!IsInitialized) InitializeThemeColors();

	// Set a default theme here in case the incoming string is invalid
	SetTheme(Themes::Light);

	if (wszTheme == L"Light") {
		SetTheme(Themes::Light);
		return;
	}

	if (wszTheme == L"Dark") {
		SetTheme(Themes::Dark);
		return;
	}

	if (wszTheme == L"DarkPlus" || wszTheme == L"Dark+") {
		SetTheme(Themes::DarkPlus);
		return;
	}

	if (wszTheme == L"Blue") {
		SetTheme(Themes::Blue);
		return;
	}
}


// ========================================================================================
// Get the Trader's name that displays in the Navigation Panel.
// This value is saved and restored from database. 
// ========================================================================================
std::wstring GetTraderName()
{
	return wszTraderName;
}


// ========================================================================================
// Set the Trader's name that displays in the Navigation Panel.
// This value is saved and restored from database. 
// ========================================================================================
void SetTraderName(std::wstring wszName)
{
	wszTraderName = wszName;
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

	if (IsWindowVisible(hwnd)) {
		AfxRedrawWindow(hwnd);
	}

	return TRUE;
}

void ApplyActiveTheme()
{
	if (!IsInitialized) InitializeThemeColors();

	// Enumerate through all top level and child windows to invalidate
	// and repaint them with the new current active theme.
	if (hWndMainWindow != NULL)
		EnumChildWindows(hWndMainWindow, EnumWindowsProc, 0);
}
