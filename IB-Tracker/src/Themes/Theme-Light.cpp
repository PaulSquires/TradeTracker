
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

	clr[(int)ThemeElement::HistoryPanelBack][theme] = Color::MakeARGB(255, 243, 243, 243);

}



