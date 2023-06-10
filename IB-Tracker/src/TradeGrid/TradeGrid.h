/*

MIT License

Copyright(c) 2023 Paul Squires

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#pragma once

#include "Themes/Themes.h"
#include "Utilities/AfxWin.h"
#include "Utilities/UserMessages.h"
#include "CustomTextBox/CustomTextBox.h"
#include "CustomLabel/CustomLabel.h"


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

	// The quantity and date cells of the first row in the grid are considered "trigger cells"
	// meaning that if they are changed then their values will populate through the other rows
	// in the grid template. This makes it faster for the user to change all of the leg quantities
	// and/or expiry dates at once rather than manually line by line.
	bool isTriggerCell = false;
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
void TradeGrid_CalculateDTE(HWND hwnd);
void TradeGrid_SetColData(HWND hGrid, int row, int col, const std::wstring& wszText);

HWND CreateTradeGrid(HWND hWndParent, LONG_PTR CtrlId, int nLeft, int nTop, int nWidth, int nHeight);

