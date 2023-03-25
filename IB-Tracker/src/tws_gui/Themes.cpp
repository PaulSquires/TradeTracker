
#include "framework.h"
#include "Themes.h"
#include "CWindow.h"


HWND hWndMainWindow = NULL;
bool IsInitialized = false;

DWORD clr[(int)ThemeElement::Count][(int)Themes::Count];
Themes ActiveTheme = Themes::Dark;


//' ========================================================================================
//' Initialize all theme and all theme element colors
//' ========================================================================================
void InitializeThemeColors()
{
	IsInitialized = true;

	clr[(int)ThemeElement::NavPanelBack][(int)Themes::Dark] = Color::MakeARGB(255, 33, 37, 43);
	clr[(int)ThemeElement::NavPanelBackHot][(int)Themes::Dark] = Color::MakeARGB(255, 20, 138, 228);
	clr[(int)ThemeElement::NavPanelText][(int)Themes::Dark] = Color::MakeARGB(255, 255, 255, 255);
	clr[(int)ThemeElement::NavPanelTextDim][(int)Themes::Dark] = Color::MakeARGB(255, 158, 205, 241);
	clr[(int)ThemeElement::NavPanelTextHot][(int)Themes::Dark] = Color::MakeARGB(255, 255, 255, 255);
	clr[(int)ThemeElement::NavPanelSeparator][(int)Themes::Dark] = Color::MakeARGB(255, 61, 156, 228);
	clr[(int)ThemeElement::NavPanelBackSelected][(int)Themes::Dark] = Color::MakeARGB(255, 100, 100, 100);
	clr[(int)ThemeElement::NavPanelTextSelected][(int)Themes::Dark] = Color::MakeARGB(255, 255, 255, 255);



	clr[(int)ThemeElement::NavPanelBack][(int)Themes::Blue] = Color::MakeARGB(255, 13, 131, 221);
	clr[(int)ThemeElement::NavPanelBack][(int)Themes::Red] = Color::PaleVioletRed;
	clr[(int)ThemeElement::NavPanelBack][(int)Themes::Yellow] = Color::LightYellow;  
				
}


//' ========================================================================================
//' Get the specified Theme element Color based on the current active theme.
//' ========================================================================================
DWORD GetThemeColor(ThemeElement element)
{
	if (!IsInitialized) InitializeThemeColors();

	// Retrieve the ARGB color value for the specified element
	// based on the current active theme.
	return clr[(int)element][(int)ActiveTheme];
}


//' ========================================================================================
//' Get the specified Theme element COLORREF based on the current active theme.
//' Used for interfacing to GDI based api. 
//' ========================================================================================
COLORREF GetThemeCOLORREF(ThemeElement element)
{
	if (!IsInitialized) InitializeThemeColors();

	// Retrieve the COLORREF color value for the specified element
	// based on the current active theme.
	Color color(GetThemeColor(element));
	return color.ToCOLORREF();
}


//' ========================================================================================
//' Set the internal structures to use the incoming Theme.
//' ========================================================================================
void SetTheme(Themes theme)
{
	if (!IsInitialized) InitializeThemeColors();
	ActiveTheme = theme;
}


//' ========================================================================================
//' Return the current set Theme (Themes enum).
//' ========================================================================================
Themes GetTheme()
{
	if (!IsInitialized) InitializeThemeColors();
	return ActiveTheme;
}


//' ========================================================================================
//' Save the Window handle of the application's main window so that ApplyActiveTheme
//' can enumerate all child windows in order to apply Theme changes.
//' ========================================================================================
void SetThemeMainWindow(HWND hWndMain)
{
	hWndMainWindow = hWndMain;
}


//' ========================================================================================
//' Apply the active/current Theme to all visual windows in real time.
//' ========================================================================================
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
