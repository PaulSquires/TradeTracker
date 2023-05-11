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
	ActionCombo,
	LineReset
};



class GridColInfo
{
public:
	HWND hCtl = NULL;
	int idCtrl = 0;
	GridColType colType = GridColType::TextBox;
	int colData = 0;  // holds LineReset line number
	std::wstring wszDate;   // holds the ISO date (whereas short date is displayed)
};


class TradeGrid
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


	~TradeGrid()
	{
		for (const auto& col : gridCols) {
			DestroyWindow(col->hCtl);
		}
	}

};


const int IDC_TRADEGRID_FIRSTCONTROL = 100;

const int TRADEGRID_LINERESETICONWIDTH = 24;

TradeGrid* TradeGrid_GetOptions(HWND hCtrl);
int TradeGrid_SetOptions(HWND hCtrl, TradeGrid* pData);
void TradeGrid_SetText(GridColInfo* col, std::wstring wszText);
std::wstring TradeGrid_GetText(HWND hCtl, int row, int col);

HWND CreateTradeGrid(HWND hWndParent, LONG_PTR CtrlId, int nLeft, int nTop, int nWidth, int nHeight);

