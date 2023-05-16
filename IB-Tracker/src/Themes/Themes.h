#pragma once

enum class ThemeElement {
	Black,
	GrayDark,
	GrayMedium,
	GrayLight,
	
	WhiteLight,
	WhiteMedium,
	WhiteDark,
	
	Red,   
	Yellow,
	Green,   
	Magenta,
	Orange,
	Purple,
	Blue,
	Pink,

	MenuNotch,
	Separator,
	Selection,

	ScrollBarBack,
	ScrollBarDivider,
	ScrollBarThumb,

	Count     // Returns the number of defined theme elements
};


enum class ActiveThemeColor {
	Dark,
	Light
};


DWORD GetThemeColor(ThemeElement element);
COLORREF GetThemeCOLORREF(ThemeElement element);

void ApplyActiveTheme();
void SetThemeMainWindow(HWND hWndMain);
void SetThemeName(std::wstring wszTheme);
std::wstring GetThemeName();
void SetIsThemeDark(bool isDark);
bool GetIsThemeDark();
void InitializeDarkThemeColors();
void InitializeLightThemeColors();

extern std::vector<DWORD> clr;
