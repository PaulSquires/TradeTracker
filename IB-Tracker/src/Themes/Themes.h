#pragma once

enum class Themes {
	Light,
	Dark,
	DarkPlus,
	Blue,
	Count     // Returns the number of defined themes
};

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

	ConfigDialogBack,
	ConfigDialogText,
	ConfigDialogSeparator,

	valuePositive,    // usually green
	valueNegative,    // usually red

	Count     // Returns the number of defined theme elements
};



DWORD GetThemeColor(ThemeElement element);
COLORREF GetThemeCOLORREF(ThemeElement element);
void SetTheme(Themes theme);
void ApplyActiveTheme();
void SetThemeMainWindow(HWND hWndMain);
Themes GetTheme();
int GetThemeControlId();

std::wstring GetThemeName();
void SetThemeName(std::wstring wszTheme);

std::wstring GetTraderName();
void SetTraderName(std::wstring wszName);

bool GetStartupConnect();
void SetStartupConnect(bool bConnect);

void Initialize_Light_Theme(DWORD clr[][(int)Themes::Count]);
void Initialize_Dark_Theme(DWORD clr[][(int)Themes::Count]);
void Initialize_DarkPlus_Theme(DWORD clr[][(int)Themes::Count]);
void Initialize_Blue_Theme(DWORD clr[][(int)Themes::Count]);


