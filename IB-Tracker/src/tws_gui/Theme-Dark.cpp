
#include "pch.h"
#include "Themes.h"


//' ========================================================================================
//' Initialize the elements of the clr array based on the specific theme.
//' Pass in the 2-dimensional color array.
//' ========================================================================================
void Initialize_Dark_Theme(DWORD clr[][(int)Themes::Count])
{
	int theme = (int)Themes::Dark;

	clr[(int)ThemeElement::NavPanelBack][theme] = Color::MakeARGB(255, 0, 0, 0);
	clr[(int)ThemeElement::NavPanelBackHot][theme] = Color::MakeARGB(255, 44, 49, 58);
	clr[(int)ThemeElement::NavPanelText][theme] = Color::MakeARGB(255, 255, 255, 255);
	clr[(int)ThemeElement::NavPanelTextDim][theme] = Color::MakeARGB(255, 157, 165, 180);
	clr[(int)ThemeElement::NavPanelTextHot][theme] = Color::MakeARGB(255, 255, 255, 255);
	clr[(int)ThemeElement::NavPanelSeparator][theme] = Color::MakeARGB(255, 53, 59, 69);
	clr[(int)ThemeElement::NavPanelBackSelected][theme] = Color::MakeARGB(255, 44, 49, 58);
	clr[(int)ThemeElement::NavPanelTextSelected][theme] = Color::MakeARGB(255, 255, 255, 255);
	clr[(int)ThemeElement::NavPanelSelector][theme] = Color::MakeARGB(255, 38, 38, 38);

	clr[(int)ThemeElement::TradesPanelBack][theme] = Color::MakeARGB(255, 38, 38, 38);
	clr[(int)ThemeElement::TradesPanelBackHot][theme] = Color::MakeARGB(255, 44, 49, 58);
	clr[(int)ThemeElement::TradesPanelText][theme] = Color::MakeARGB(255, 255, 255, 255);
	clr[(int)ThemeElement::TradesPanelTextHot][theme] = Color::MakeARGB(255, 255, 255, 255);




	//clr[(int)ThemeElement::HistoryPanelBack][theme] = Color::MakeARGB(255, 0, 0, 0);
	clr[(int)ThemeElement::HistoryPanelBack][theme] = Color::ForestGreen;

}



