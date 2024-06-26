/*

MIT License

Copyright(c) 2023-2024 Paul Squires

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


enum class GridColType {
	Label,
	TextBox,
	DatePicker,
	PutCallCombo,
	ActionCombo,
	LineReset
};

class GridColInfo {
public:
	HWND hCtl = NULL;
	int ctrl_id = 0;
	GridColType colType = GridColType::TextBox;
	int colData = 0;  // holds LineReset line number

	// The quantity and date cells of the first row in the grid are considered "trigger cells"
	// meaning that if they are changed then their values will populate through the other rows
	// in the grid template. This makes it faster for the user to change all of the leg quantities
	// and/or expiry dates at once rather than manually line by line.
	bool isTriggerCell = false;
};

class TradeGrid {
private:
	float m_rx = 0;
	float m_ry = 0;

public:
	HWND hWindow = NULL;
	HWND hParent = NULL;
	HINSTANCE hInst = NULL;

	int ctrl_id = 0;

	COLORREF back_color{};
	HBRUSH hback_brush = NULL;
	HFONT hFont = NULL;
	bool show_original_quantity = false;

	std::vector<GridColInfo*> gridCols;

	~TradeGrid() {
		for (const auto& col : gridCols) {
			DestroyWindow(col->hCtl);
		}
	}
};


constexpr int IDC_TRADEGRID_FIRSTCONTROL = 100;

constexpr int TRADEGRID_LINERESETICONWIDTH = 24;

TradeGrid* TradeGrid_GetOptions(HWND hCtrl);
int TradeGrid_SetOptions(HWND hCtrl, TradeGrid* pData);
void TradeGrid_SetText(GridColInfo* col, std::wstring text);
std::wstring TradeGrid_GetText(HWND hCtl, int row, int col);
void TradeGrid_CalculateDTE(HWND hwnd);
void TradeGrid_SetColData(HWND hGrid, int row, int col, const std::wstring& text);

HWND CreateTradeGrid(HWND hWndParent, LONG_PTR ctrl_id, int left, int top, int width, int height, bool show_original_quantity);

