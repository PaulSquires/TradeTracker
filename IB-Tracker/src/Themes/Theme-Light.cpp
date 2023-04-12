
#include "pch.h"
#include "Themes.h"


// ========================================================================================
// Initialize the elements of the clr array based on the specific theme.
// Pass in the 2-dimensional color array.
// ========================================================================================
void Initialize_Light_Theme(DWORD clr[][(int)Themes::Count])
{
	int theme = (int)Themes::Light;

	clr[(int)ThemeElement::MenuPanelBack][theme] = Color::MakeARGB(255, 243, 243, 243);
	clr[(int)ThemeElement::MenuPanelBackHot][theme] = Color::MakeARGB(255, 232, 232, 232);
	clr[(int)ThemeElement::MenuPanelText][theme] = Color::MakeARGB(255, 97, 97, 97);
	clr[(int)ThemeElement::MenuPanelTextDim][theme] = Color::MakeARGB(255, 157, 165, 180);
	clr[(int)ThemeElement::MenuPanelTextHot][theme] = Color::MakeARGB(255, 97, 97, 97);
	clr[(int)ThemeElement::MenuPanelSeparator][theme] = Color::MakeARGB(255, 157, 165, 180);
	clr[(int)ThemeElement::MenuPanelBackSelected][theme] = Color::MakeARGB(255, 232, 232, 232);
	clr[(int)ThemeElement::MenuPanelTextSelected][theme] = Color::MakeARGB(255, 97, 97, 97);
	clr[(int)ThemeElement::MenuPanelSelector][theme] = Color::MakeARGB(255, 255, 255, 255);

	clr[(int)ThemeElement::TradesPanelBack][theme] = Color::MakeARGB(255, 255, 255, 255);
	clr[(int)ThemeElement::TradesPanelBackHot][theme] = Color::MakeARGB(255, 44, 49, 58);
	clr[(int)ThemeElement::TradesPanelText][theme] = Color::MakeARGB(255, 212, 212, 212);
	clr[(int)ThemeElement::TradesPanelTextDim][theme] = Color::Gray;
	clr[(int)ThemeElement::TradesPanelColBackLight][theme] = Color::MakeARGB(255, 68, 68, 68);
	clr[(int)ThemeElement::TradesPanelColBackDark][theme] = Color::MakeARGB(255, 51, 51, 51);
	clr[(int)ThemeElement::TradesPanelNormalDTE][theme] = Color::Magenta;
	clr[(int)ThemeElement::TradesPanelWarningDTE][theme] = Color::Yellow;
	clr[(int)ThemeElement::TradesPanelScrollBarBack][theme] = Color::MakeARGB(255, 38, 38, 38);
	clr[(int)ThemeElement::TradesPanelScrollBarLine][theme] = Color::MakeARGB(255, 64, 67, 73);
	clr[(int)ThemeElement::TradesPanelScrollBarThumb][theme] = Color::MakeARGB(255, 51, 51, 51);
	clr[(int)ThemeElement::TradesPanelHistoryText][theme] = Color::MakeARGB(255, 193, 98, 24);  // burnt orange

	clr[(int)ThemeElement::valuePositive][theme] = Color::MakeARGB(255, 72, 151, 13);
	clr[(int)ThemeElement::valueNegative][theme] = Color::MakeARGB(255, 255, 30, 0);

}



