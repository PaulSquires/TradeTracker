
#include "pch.h"
#include "Themes.h"


// ========================================================================================
// Initialize the elements of the clr array based on the specific theme.
// Pass in the 2-dimensional color array.
// ========================================================================================
void Initialize_DarkPlus_Theme(DWORD clr[][(int)Themes::Count])
{
	int theme = (int)Themes::DarkPlus;

	clr[(int)ThemeElement::MenuPanelBack][theme] = Color::MakeARGB(255, 38, 38, 43);
	clr[(int)ThemeElement::MenuPanelBackHot][theme] = Color::MakeARGB(255, 20, 138, 228);
	clr[(int)ThemeElement::MenuPanelText][theme] = Color::MakeARGB(255, 255, 255, 255);
	clr[(int)ThemeElement::MenuPanelTextDim][theme] = Color::MakeARGB(255, 158, 205, 241);
	clr[(int)ThemeElement::MenuPanelTextHot][theme] = Color::MakeARGB(255, 255, 255, 255);
	clr[(int)ThemeElement::MenuPanelSeparator][theme] = Color::MakeARGB(255, 61, 156, 228);
	clr[(int)ThemeElement::MenuPanelBackSelected][theme] = Color::MakeARGB(255, 20, 138, 228);
	clr[(int)ThemeElement::MenuPanelTextSelected][theme] = Color::MakeARGB(255, 255, 255, 255);
	clr[(int)ThemeElement::MenuPanelSelector][theme] = Color::MakeARGB(255, 30, 30, 38);

	clr[(int)ThemeElement::TradesPanelBack][theme] = Color::MakeARGB(255, 30, 30, 38);
	
	clr[(int)ThemeElement::HistoryPanelBack][theme] = Color::MakeARGB(255, 38, 38, 43);
}



