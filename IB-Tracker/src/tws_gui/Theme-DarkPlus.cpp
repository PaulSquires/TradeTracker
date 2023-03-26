
#include "framework.h"
#include "Themes.h"


//' ========================================================================================
//' Initialize the elements of the clr array based on the specific theme.
//' Pass in the 2-dimensional color array.
//' ========================================================================================
void Initialize_DarkPlus_Theme(DWORD clr[][(int)Themes::Count])
{
	int theme = (int)Themes::DarkPlus;

	clr[(int)ThemeElement::NavPanelBack][theme] = Color::MakeARGB(255, 38, 38, 43);
	clr[(int)ThemeElement::NavPanelBackHot][theme] = Color::MakeARGB(255, 20, 138, 228);
	clr[(int)ThemeElement::NavPanelText][theme] = Color::MakeARGB(255, 255, 255, 255);
	clr[(int)ThemeElement::NavPanelTextDim][theme] = Color::MakeARGB(255, 158, 205, 241);
	clr[(int)ThemeElement::NavPanelTextHot][theme] = Color::MakeARGB(255, 255, 255, 255);
	clr[(int)ThemeElement::NavPanelSeparator][theme] = Color::MakeARGB(255, 61, 156, 228);
	clr[(int)ThemeElement::NavPanelBackSelected][theme] = Color::MakeARGB(255, 20, 138, 228);
	clr[(int)ThemeElement::NavPanelTextSelected][theme] = Color::MakeARGB(255, 255, 255, 255);
	clr[(int)ThemeElement::NavPanelSelector][theme] = Color::MakeARGB(255, 30, 30, 38);

	clr[(int)ThemeElement::TradesPanelBack][theme] = Color::MakeARGB(255, 30, 30, 38);
	
	clr[(int)ThemeElement::HistoryPanelBack][theme] = Color::MakeARGB(255, 38, 38, 43);
}



