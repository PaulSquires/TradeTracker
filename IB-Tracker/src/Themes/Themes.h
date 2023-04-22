#pragma once

enum class ThemeElement {
	MenuPanelBack,
	MenuPanelBackHot,
	MenuPanelText,
	MenuPanelTextDim,
	MenuPanelTextHot,
	MenuPanelBackSelected,
	MenuPanelTextSelected,
	MenuPanelSeparator,
	MenuPanelSelector,

	TradesPanelBack,
	TradesPanelBackHot,
	TradesPanelText,
	TradesPanelTextDim,
	TradesPanelColBackLight,
	TradesPanelColBackDark,
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



bool LoadTheme();
void InitializeThemeColors();

DWORD GetThemeColor(ThemeElement element);
COLORREF GetThemeCOLORREF(ThemeElement element);

void ApplyActiveTheme();
void SetThemeMainWindow(HWND hWndMain);
void SetThemeName(std::wstring wszTheme);
std::wstring GetThemeName();
void SetIsThemeDark(bool isDark);
bool GetIsThemeDark();

extern std::vector<DWORD> clr;
