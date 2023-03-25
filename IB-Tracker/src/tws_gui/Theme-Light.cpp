
#include "framework.h"
#include "Themes.h"


//' ========================================================================================
//' Initialize the elements of the clr array based on the specific theme.
//' Pass in the 2-dimensional color array.
//' ========================================================================================
void Initialize_Light_Theme(DWORD clr[][(int)Themes::Count])
{
	int theme = (int)Themes::Light;

	clr[(int)ThemeElement::NavPanelBack][theme] = Color::MakeARGB(255, 243, 243, 243);
	clr[(int)ThemeElement::NavPanelBackHot][theme] = Color::MakeARGB(255, 232, 232, 232);
	clr[(int)ThemeElement::NavPanelText][theme] = Color::MakeARGB(255, 97, 97, 97);
	clr[(int)ThemeElement::NavPanelTextDim][theme] = Color::MakeARGB(255, 157, 165, 180);
	clr[(int)ThemeElement::NavPanelTextHot][theme] = Color::MakeARGB(255, 97, 97, 97);
	clr[(int)ThemeElement::NavPanelSeparator][theme] = Color::MakeARGB(255, 157, 165, 180);
	clr[(int)ThemeElement::NavPanelBackSelected][theme] = Color::MakeARGB(255, 232, 232, 232);
	clr[(int)ThemeElement::NavPanelTextSelected][theme] = Color::MakeARGB(255, 97, 97, 97);
	clr[(int)ThemeElement::NavPanelSelector][theme] = Color::MakeARGB(255, 255, 255, 255);

	clr[(int)ThemeElement::TradesPanelBack][theme] = Color::MakeARGB(255, 255, 255, 255);

}



