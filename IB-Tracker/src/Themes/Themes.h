#pragma once

enum class ThemeElement {
	BaseBlack,
	MenuPanelBackHot,
	MenuPanelText,
	MenuPanelTextDim,
	MenuPanelTextHot,
	MenuPanelBackSelected,
	MenuPanelTextSelected,
	MenuPanelSeparator,
	MenuPanelSelector,

	GrayDark,
	TradesPanelBackHot,
	TradesPanelText,
	TradesPanelTextDim,
	GrayLight,
	GrayMedium,
	TradesPanelNormalDTE,
	TradesPanelWarningDTE,
	TradesPanelScrollBarBack,
	TradesPanelScrollBarLine,
	TradesPanelScrollBarThumb,
	TradesPanelHistoryText,

	ListHeaderBack,
	ListHeaderText,
	ListHeaderBorder,

	valuePositive,    // usually green
	valueNegative,    // usually red

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
