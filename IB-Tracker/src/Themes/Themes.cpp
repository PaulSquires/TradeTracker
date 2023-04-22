
#include "pch.h"
#include "Themes.h"
#include "..\Utilities\AfxWin.h"


HWND hWndMainWindow = NULL;
bool IsInitialized = false;

std::wstring wszThemeName;
bool IsThemeDark = false;

const std::wstring dbThemes = AfxGetExePath() + L"\\IB-Tracker-themes.txt";
const std::wstring dbThemesUser = AfxGetExePath() + L"\\IB-Tracker-themes-user.txt";

std::vector<DWORD> clr;



// ========================================================================================
// Initialize all theme and all theme element colors
// ========================================================================================
void InitializeThemeColors()
{
	IsInitialized = true;

	// Set default values for the clr vector. We need to do this so color values
	// will exist in the absence of an external theme text file. The values below
	// represent a standard dark theme.
	clr.reserve((int)ThemeElement::Count);

	SetIsThemeDark(true);

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
// Set the current Theme based on it's string name (std::wstring).
// This value will have been read from the Config file and will try to be matched
// to a theme in the external theme disk file(s).
// ========================================================================================
void SetThemeName(std::wstring wszTheme)
{
	wszThemeName = wszTheme;
}



// ========================================================================================
// Get the current Theme name.
// This value will have been read from the Config file and will try to be matched
// to a theme in the external theme disk file(s).
// ========================================================================================
std::wstring GetThemeName()
{
	return AfxTrim(wszThemeName);
}



// ========================================================================================
// Set the current IsThemeDark variable. This is used to determine whether
// to color th application's caption bar using light or dark Windows theme.
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
	if (!IsInitialized) InitializeThemeColors();

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
	if (!IsInitialized) InitializeThemeColors();

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
	if (hWndMainWindow != NULL) {
		AfxRedrawWindow(hWndMainWindow);
		EnumChildWindows(hWndMainWindow, EnumWindowsProc, 0);
	}
}



// ========================================================================================
// Load the Theme from file.
// ========================================================================================
bool LoadTheme_Internal(const std::wstring& dbFilename)
{
    std::wifstream db;

    db.open(dbFilename, std::ios::in);

    if (!db.is_open())
        return false;

    std::wstring line;
    bool readingTheme = false;
	bool themeFound = false;


    while (!db.eof()) {
        std::getline(db, line);

        if (line.length() == 0) continue;

        // If this is a Comment line then simply iterate to next line.
        if (line.compare(1, 3, L"// ") == 0) continue;

        // Tokenize the line into a vector based on the colon delimiter
        std::vector<std::wstring> st = AfxSplit(line, L":");

        if (st.empty()) continue;

        std::wstring arg = AfxTrim(st.at(0));


        if (arg == L"THEME") {
            readingTheme = true;
            continue;
        }


        if (arg == L"NAME") {
            if (!readingTheme) continue;
            if (AfxWStringCompareI(AfxTrim(st.at(1)), GetThemeName()) == false) {
                readingTheme = false;
                continue;
            }
			themeFound = true;
			continue;
        }


		if (arg == L"ISTHEMEDARK") {
			if (!readingTheme) continue;
			bool isDark = AfxWStringCompareI(AfxTrim(st.at(1)), L"True");
			SetIsThemeDark(isDark);
			continue;
		}


		if (!readingTheme) continue;


        // If we get this far then we are reading the Theme Elements for the matching
        // theme that was specified in the Config file. Simply replace the default data 
        // that we had already created in the clr[] vector.

		// Tokenize the argb attributes into a vector based on the comma delimiter
		if (st.size() != 2) continue;

		DWORD color = 0;
		
		std::vector<std::wstring> attr = AfxSplit(st.at(1), L",");
		if (attr.size() == 4) {
			color = Color::MakeARGB(stoi(attr.at(0)), stoi(attr.at(1)), stoi(attr.at(2)), stoi(attr.at(3)));
		}

		if (arg == L"MENUPANELBACK") {
			clr[(int)ThemeElement::MenuPanelBack] = color;
		}
		else if (arg == L"MENUPANELBACKHOT") {
			clr[(int)ThemeElement::MenuPanelBackHot] = color;
		}
		else if (arg == L"MENUPANELTEXT") {
			clr[(int)ThemeElement::MenuPanelText] = color;
		}
		else if (arg == L"MENUPANELTEXTDIM") {
			clr[(int)ThemeElement::MenuPanelTextDim] = color;
		}
		else if (arg == L"MENUPANELTEXTHOT") {
			clr[(int)ThemeElement::MenuPanelTextHot] = color;
		}
		else if (arg == L"MENUPANELSEPARATOR") {
			clr[(int)ThemeElement::MenuPanelSeparator] = color;
		}
		else if (arg == L"MENUPANELBACKSELECTED") {
			clr[(int)ThemeElement::MenuPanelBackSelected] = color;
		} 
		else if (arg == L"MENUPANELTEXTSELECTED") {
			clr[(int)ThemeElement::MenuPanelTextSelected] = color;
		}
		else if (arg == L"MENUPANELSELECTOR") {
			clr[(int)ThemeElement::MenuPanelSelector] = color;
		}
		else if (arg == L"TRADESPANELBACK") {
			clr[(int)ThemeElement::TradesPanelBack] = color;
		}
		else if (arg == L"TRADESPANELBACKHOT") {
			clr[(int)ThemeElement::TradesPanelBackHot] = color;
		}
		else if (arg == L"TRADESPANELTEXT") {
			clr[(int)ThemeElement::TradesPanelText] = color;
		}
		else if (arg == L"TRADESPANELTEXTDIM") {
			clr[(int)ThemeElement::TradesPanelTextDim] = color;
		}
		else if (arg == L"TRADESPANELCOLBACKLIGHT") {
			clr[(int)ThemeElement::TradesPanelColBackLight] = color;
		}
		else if (arg == L"TRADESPANELCOLBACKDARK") {
			clr[(int)ThemeElement::TradesPanelColBackDark] = color;
		}
		else if (arg == L"TRADESPANELNORMALDTE") {
			clr[(int)ThemeElement::TradesPanelNormalDTE] = color;
		}
		else if (arg == L"TRADESPANELWARNINGDTE") {
			clr[(int)ThemeElement::TradesPanelWarningDTE] = color;
		}
		else if (arg == L"TRADESPANELSCROLLBARBACK") {
			clr[(int)ThemeElement::TradesPanelScrollBarBack] = color;
		}
		else if (arg == L"TRADESPANELSCROLLBARLINE") {
			clr[(int)ThemeElement::TradesPanelScrollBarLine] = color;
		}
		else if (arg == L"TRADESPANELSCROLLBARTHUMB") {
			clr[(int)ThemeElement::TradesPanelScrollBarThumb] = color;
		}
		else if (arg == L"TRADESPANELHISTORYTEXT") {
			clr[(int)ThemeElement::TradesPanelHistoryText] = color;
		}
		else if (arg == L"LISTHEADERBACK") {
			clr[(int)ThemeElement::ListHeaderBack] = color;
		}
		else if (arg == L"LISTHEADERTEXT") {
			clr[(int)ThemeElement::ListHeaderText] = color;
		}
		else if (arg == L"LISTHEADERBORDER") {
			clr[(int)ThemeElement::ListHeaderBorder] = color;
		}
		else if (arg == L"VALUEPOSITIVE") {
			clr[(int)ThemeElement::valuePositive] = color;
		}
		else if (arg == L"VALUENEGATIVE") {
			clr[(int)ThemeElement::valueNegative] = color;
		}
	
    }

    db.close();

    return themeFound;
}



// ========================================================================================
// Load the Theme (pre-defined and user) from files.
// Try to match the Theme as specified by the user in the Config file. We can stop
// processing once we find the match.
// ========================================================================================
bool LoadTheme()
{
    if (LoadTheme_Internal(dbThemes) == false) {
        // Theme not found in the pre-defined file so lets try looking
        // in the user defined file.
        if (LoadTheme_Internal(dbThemesUser) == false) {
            // Theme was not found so tell the user that default dark
            // theme will be used.
			std::wstring msg = L"The Theme '" + GetThemeName() + 
				L"' specified in the Config file could not be found.\n\nDefaulting to use the standard built in Dark theme.";
			MessageBox(
				NULL,
				(LPCWSTR)(msg.c_str()),
				(LPCWSTR)L"IB-Tracker",
				MB_ICONWARNING);

        }
    }
    return true;
}

