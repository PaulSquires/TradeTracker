#pragma once

#include "pch.h"

enum class Themes {
	Light,
	Dark,
	DarkPlus,
	Blue,
	Count     // Returns the number of defined themes
};

enum class ThemeElement {
	NavPanelBack,
	NavPanelBackHot,
	NavPanelText,
	NavPanelTextDim,
	NavPanelTextHot,
	NavPanelBackSelected,
	NavPanelTextSelected,
	NavPanelSeparator,
	NavPanelSelector,

	TradesPanelBack,
	TradesPanelBackHot,
	TradesPanelText,
	TradesPanelTextDim,
	TradesPanelColBackLight,
	TradesPanelColBackDark,
	TradesPanelNormalDTE,
	TradesPanelWarningDTE,

	HistoryPanelBack,

	Count     // Returns the number of defined theme elements
};



DWORD GetThemeColor(ThemeElement element);
COLORREF GetThemeCOLORREF(ThemeElement element);
void SetTheme(Themes theme);
void ApplyActiveTheme();
void SetThemeMainWindow(HWND hWndMain);
Themes GetTheme();
std::wstring GetThemeName();
std::wstring GetTraderName();
void SetTraderName(std::wstring wszName);
void SetThemeName(std::wstring wszTheme);
void Initialize_Light_Theme(DWORD clr[][(int)Themes::Count]);
void Initialize_Dark_Theme(DWORD clr[][(int)Themes::Count]);
void Initialize_DarkPlus_Theme(DWORD clr[][(int)Themes::Count]);
void Initialize_Blue_Theme(DWORD clr[][(int)Themes::Count]);


