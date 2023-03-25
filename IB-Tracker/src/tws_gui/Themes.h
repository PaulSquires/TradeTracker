#pragma once

#include "framework.h"

enum class Themes {
	Light,
	Dark,
	Blue,
	Red,
	Yellow,
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
	Count     // Returns the number of defined theme elements
};



DWORD GetThemeColor(ThemeElement element);
COLORREF GetThemeCOLORREF(ThemeElement element);
void SetTheme(Themes theme);
void ApplyActiveTheme();
void SetThemeMainWindow(HWND hWndMain);
Themes GetTheme();

