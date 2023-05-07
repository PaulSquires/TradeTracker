#pragma once

#include "..\Themes\Themes.h"
#include "..\Utilities\AfxWin.h"
#include "..\Utilities\UserMessages.h"
#include "..\CustomTextBox\CustomTextBox.h"
#include "..\CustomLabel\CustomLabel.h"



class CustomTradeGrid
{
private:
	float m_rx = 0;
	float m_ry = 0;

public:
	HWND hWindow = NULL;
	HWND hParent = NULL;
	HINSTANCE hInst = NULL;

	int CtrlId = 0;

	COLORREF BackColor{};
	HBRUSH hBackBrush = NULL;
	HFONT hFont = NULL;

	// The grid consists of 4 rows each with 6 columns, therefore
	// we need an array of 24 TextBoxes.

	std::vector<HWND> gridCols;


	~CustomTradeGrid()
	{
		for (const auto& col : gridCols) {
			DestroyWindow(col);
		}
	}

};


CustomTradeGrid* CustomTradeGrid_GetOptions(HWND hCtrl);
int CustomTradeGrid_SetOptions(HWND hCtrl, CustomTradeGrid* pData);

HWND CreateCustomTradeGrid(HWND hWndParent, LONG_PTR CtrlId, int nLeft, int nTop, int nWidth, int nHeight);

