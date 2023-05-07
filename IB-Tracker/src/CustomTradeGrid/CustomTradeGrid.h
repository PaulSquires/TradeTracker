#pragma once

#include "..\Themes\Themes.h"
#include "..\Utilities\AfxWin.h"
#include "..\Utilities\UserMessages.h"
#include "..\CustomTextBox\CustomTextBox.h"
#include "..\CustomLabel\CustomLabel.h"


enum class GridColType
{
	Label,
	TextBox,
	DatePicker,
	PutCallCombo,
	ActionCombo
};



class GridColInfo
{
public:
	HWND hCtl = NULL;
	int idCtrl = 0;
	GridColType colType = GridColType::TextBox;
	std::wstring colValue;
};


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

	std::vector<GridColInfo*> gridCols;


	~CustomTradeGrid()
	{
		for (const auto& col : gridCols) {
			DestroyWindow(col->hCtl);
		}
	}

};


CustomTradeGrid* CustomTradeGrid_GetOptions(HWND hCtrl);
int CustomTradeGrid_SetOptions(HWND hCtrl, CustomTradeGrid* pData);
void CustomTradeGrid_SetText(GridColInfo* col, std::wstring wszText);

HWND CreateCustomTradeGrid(HWND hWndParent, LONG_PTR CtrlId, int nLeft, int nTop, int nWidth, int nHeight);

