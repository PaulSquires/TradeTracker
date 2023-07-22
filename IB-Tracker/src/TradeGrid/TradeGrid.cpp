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

#include "pch.h"
#include "Utilities/CWindowBase.h"
#include "Utilities/ListBoxData.h"
#include "MainWindow/MainWindow.h"
#include "DatePicker/Calendar.h"
#include "TradeDialog/TradeDialog.h"

#include "TradeGrid.h"

extern HWND HWND_TRADEDIALOG;
extern CMainWindow Main;


// ========================================================================================
// Helper function to set a column's data and update display 
// ========================================================================================
void TradeGrid_SetColData(HWND hGrid, int row, int col, const std::wstring& wszText)
{
    TradeGrid* pData = TradeGrid_GetOptions(hGrid);
    if (pData == nullptr) return;

    int numCols = (pData->bShowOriginalQuantity) ? 8 : 7;

    int idx = (row * numCols) + col;

    GridColInfo* pCol = pData->gridCols.at(idx);
    if (pCol == nullptr) return;

    switch (pCol->colType)
    {
    case GridColType::Label:
        CustomLabel_SetText(pCol->hCtl, wszText);
        break;

    case GridColType::TextBox:
        AfxSetWindowText(pCol->hCtl, wszText);
        break;

    case GridColType::DatePicker:
        CustomLabel_SetUserData(pCol->hCtl, wszText);
        CustomLabel_SetText(pCol->hCtl, AfxShortDate(wszText));
        break;

    case GridColType::PutCallCombo:
        CustomLabel_SetText(pCol->hCtl, wszText);
        break;

    case GridColType::ActionCombo:
        CustomLabel_SetText(pCol->hCtl, wszText);
        if (pCol->colType == GridColType::ActionCombo) {
            DWORD clr = COLOR_RED;
            if (wszText == L"BTO" || wszText == L"BTC") clr = COLOR_GREEN;
            CustomLabel_SetTextColor(pCol->hCtl, clr);
        }
        break;

    }
}


// ========================================================================================
// Calculate Days To Expiration (DTE) and display it in the table
// ========================================================================================
void TradeGrid_CalculateDTE(HWND hwnd)
{
    TradeGrid* pData = TradeGrid_GetOptions(hwnd);
    if (pData == nullptr) return;

    std::wstring transDate = CustomLabel_GetUserData(GetDlgItem(HWND_TRADEDIALOG, IDC_TRADEDIALOG_LBLTRANSDATE));
        
    std::wstring expiryDate;
    std::wstring wszDTE;

    for (int i = 0; i < 4; ++i) {
        int colStart = 0;
        int expiryDateCol = 0;
        int dteCol = 0;
        
        if (pData->bShowOriginalQuantity) {
            colStart = i * 8;
            expiryDateCol = colStart + 2;
            dteCol = colStart + 3;
        }
        else {
            colStart = i * 7;
            expiryDateCol = colStart + 1;
            dteCol = colStart + 2;
        }

        expiryDate = CustomLabel_GetUserData(pData->gridCols.at(expiryDateCol)->hCtl);
        
        if (transDate.length() != 10) continue;
        if (expiryDate.length() != 10) continue;
        
        int days = AfxDaysBetween(transDate, expiryDate);
        wszDTE = std::to_wstring(days) + L"d";
        CustomLabel_SetText(pData->gridCols.at(dteCol)->hCtl, wszDTE);   // DTE
    }
}


// ========================================================================================
// Populate the trade grid (all 4 rows and columns).
// ========================================================================================
void TradeGrid_PopulateColumns(TradeGrid* pData)
{
    if (pData == nullptr) return;

    // Setup the lines and columns

    // The grid consists of 4 rows each with 6 columns, therefore
    // we need an array of 24 TextBoxes.
    pData->gridCols.reserve(24);

    HWND hCtl = NULL;
    int idCtrl = IDC_TRADEGRID_FIRSTCONTROL;

    DWORD lightBackColor = COLOR_GRAYLIGHT;
    DWORD lightTextColor = COLOR_WHITELIGHT;

    DWORD darkBackColor = COLOR_GRAYMEDIUM;
    DWORD darkTextColor = COLOR_WHITEDARK;

    int nTop = 0;
    int nLeft = 0;
    int nWidth = 50;
    int nHeight = 23;
    
    int vsp = 2;
    int hsp = 2;
    
    int HTextMargin = 0;
    int VTextMargin = 3;
    
    int nTotalWidth = 0;
    int nTotalHeight = 0;

    CustomLabel* pLabelData = nullptr;

    for (int row = 0; row < 4; ++row) {
        // All are center except Quantity which is right align.
        GridColInfo* col;


        // QUANTITY (ORIGINAL)
        if (pData->bShowOriginalQuantity == true) {
            nWidth = 25;
            hCtl = CreateCustomTextBox(pData->hWindow, idCtrl, ES_RIGHT, L"", nLeft, nTop, nWidth, nHeight);
            CustomTextBox_SetNumericAttributes(hCtl, 0, CustomTextBoxNegative::Allow, CustomTextBoxFormatting::Disallow);
            CustomTextBox_SetColors(hCtl, lightTextColor, darkBackColor);
            CustomTextBox_SetMargins(hCtl, HTextMargin, VTextMargin);
            col = new GridColInfo;
            col->hCtl = hCtl;
            col->idCtrl = idCtrl;
            col->colType = GridColType::TextBox;
            pData->gridCols.push_back(col);
            idCtrl++;
            nLeft = nLeft + nWidth + hsp;
        }

        // QUANTITY (OPEN)
        nWidth = (pData->bShowOriginalQuantity == true) ? 25 : 50;
        hCtl = CreateCustomTextBox(pData->hWindow, idCtrl, ES_RIGHT, L"", nLeft, nTop, nWidth, nHeight);
        CustomTextBox_SetNumericAttributes(hCtl, 0, CustomTextBoxNegative::Allow, CustomTextBoxFormatting::Disallow);
        CustomTextBox_SetColors(hCtl, lightTextColor, darkBackColor);
        CustomTextBox_SetMargins(hCtl, HTextMargin, VTextMargin);
        col = new GridColInfo;
        col->hCtl = hCtl;
        col->idCtrl = idCtrl;
        col->colType = GridColType::TextBox;
        if (pData->bShowOriginalQuantity == false) {
            if (row == 0) col->isTriggerCell = true;
        }
        pData->gridCols.push_back(col);
        idCtrl++;
        nLeft = nLeft + nWidth + hsp;

        nWidth = 50;

        // EXPIRY DATE
        hCtl = CustomLabel_SimpleLabel(pData->hWindow, idCtrl, 
            L"", COLOR_WHITELIGHT, COLOR_GRAYLIGHT,
            CustomLabelAlignment::MiddleCenter, nLeft, nTop, nWidth, nHeight);
        CustomLabel_SetMousePointer(hCtl, CustomLabelPointer::Hand, CustomLabelPointer::Hand);
        col = new GridColInfo;
        col->hCtl = hCtl;
        col->idCtrl = idCtrl;
        if (pData->bShowOriginalQuantity == false) {
            if (row == 0) col->isTriggerCell = true;
        }
        col->colType = GridColType::DatePicker;
        pData->gridCols.push_back(col);
        idCtrl++;
        nLeft = nLeft + nWidth + hsp;

        // DTE
        hCtl = CustomLabel_SimpleLabel(pData->hWindow, idCtrl,
            L"", COLOR_WHITEDARK, COLOR_GRAYMEDIUM,
            CustomLabelAlignment::MiddleCenter, nLeft, nTop, nWidth, nHeight);
        col = new GridColInfo;
        col->hCtl = hCtl;
        col->idCtrl = idCtrl;
        col->colType = GridColType::Label;
        pData->gridCols.push_back(col);
        idCtrl++;
        nLeft = nLeft + nWidth + hsp;

        // STRIKE PRICE
        hCtl = CreateCustomTextBox(pData->hWindow, idCtrl, ES_CENTER, L"", nLeft, nTop, nWidth, nHeight);
        CustomTextBox_SetNumericAttributes(hCtl, 5, CustomTextBoxNegative::Disallow, CustomTextBoxFormatting::Disallow);
        CustomTextBox_SetColors(hCtl, lightTextColor, lightBackColor);
        CustomTextBox_SetMargins(hCtl, HTextMargin, VTextMargin);
        col = new GridColInfo;
        col->hCtl = hCtl;
        col->idCtrl = idCtrl;
        col->colType = GridColType::TextBox;
        pData->gridCols.push_back(col);
        idCtrl++;
        nLeft = nLeft + nWidth + hsp;

        // PUT/CALL
        hCtl = CustomLabel_SimpleLabel(pData->hWindow, idCtrl,
            L"", COLOR_WHITEDARK, COLOR_GRAYMEDIUM,
            CustomLabelAlignment::MiddleCenter, nLeft, nTop, (nWidth/2), nHeight);
        CustomLabel_SetMousePointer(hCtl, CustomLabelPointer::Hand, CustomLabelPointer::Hand);
        col = new GridColInfo;
        col->hCtl = hCtl;
        col->idCtrl = idCtrl;
        col->colType = GridColType::PutCallCombo;
        pData->gridCols.push_back(col);
        idCtrl++;
        nLeft = nLeft + (nWidth/2) + hsp;

        // ACTION
        hCtl = CustomLabel_SimpleLabel(pData->hWindow, idCtrl,
            L"", COLOR_WHITELIGHT, COLOR_GRAYLIGHT,
            CustomLabelAlignment::MiddleCenter, nLeft, nTop, nWidth, nHeight);
        CustomLabel_SetMousePointer(hCtl, CustomLabelPointer::Hand, CustomLabelPointer::Hand);
        col = new GridColInfo;
        col->hCtl = hCtl;
        col->idCtrl = idCtrl;
        col->colType = GridColType::ActionCombo;
        pData->gridCols.push_back(col);
        idCtrl++;
        nLeft = nLeft + nWidth + hsp;


        // RESET LINE INDICATOR
        hCtl = CreateCustomLabel(pData->hWindow, idCtrl,
            CustomLabelType::TextOnly, nLeft, nTop, TRADEGRID_LINERESETICONWIDTH, nHeight);
        pLabelData = CustomLabel_GetOptions(hCtl);
        if (pLabelData) {
            pLabelData->wszText = GLYPH_RESETLINE;
            pLabelData->wszTextHot = GLYPH_RESETLINE;
            pLabelData->HotTestEnable = true;
            pLabelData->BackColor = COLOR_GRAYDARK; 
            pLabelData->BackColorHot = COLOR_GRAYMEDIUM;
            pLabelData->BackColorButtonDown = COLOR_GRAYLIGHT; 
            pLabelData->TextColor = COLOR_WHITEDARK;
            pLabelData->TextColorHot = COLOR_WHITELIGHT;
            pLabelData->TextAlignment = CustomLabelAlignment::MiddleCenter;
            pLabelData->wszToolTip = L"Reset line";
            pLabelData->PointerHot = CustomLabelPointer::Arrow;
            CustomLabel_SetOptions(hCtl, pLabelData);
        }
        col = new GridColInfo;
        col->hCtl = hCtl;
        col->idCtrl = idCtrl;
        col->colType = GridColType::LineReset;
        col->colData = row;
        pData->gridCols.push_back(col);
        idCtrl++;

        nTop = nTop + nHeight + vsp;
        nLeft = 0;
        
    }

    nTotalWidth = AfxScaleX((float)5 * (nWidth + hsp)) + AfxScaleX((float)nWidth/2);
    nTotalHeight = AfxScaleY((float)4 * (nHeight + vsp) - vsp);

    // Add width for the line reset indicators
    nTotalWidth += AfxScaleX(TRADEGRID_LINERESETICONWIDTH);

    // Resize the control container to fit the size of the trade grid.
    SetWindowPos(pData->hWindow, 0, 0, 0, nTotalWidth, nTotalHeight, SWP_NOMOVE | SWP_NOZORDER);

}


// ========================================================================================
// Set text in a trade grid cell.
// ========================================================================================
void TradeGrid_SetText(GridColInfo* col, std::wstring wszText)
{
    if (col == nullptr) return;

    CustomLabel_SetText(col->hCtl, wszText);

    if (col->colType == GridColType::ActionCombo) {
        DWORD clr = COLOR_RED;
        if (wszText == L"BTO" || wszText == L"BTC") clr = COLOR_GREEN;
        CustomLabel_SetTextColor(col->hCtl, clr);
    }
}


// ========================================================================================
// Handle when the PutCall button is clicked.
// ========================================================================================
void TradeGrid_OnClickPutCall(TradeGrid* pData, GridColInfo* col)
{
    if (pData == nullptr) return;
    if (col == nullptr) return;

    std::wstring PutCall = CustomLabel_GetText(col->hCtl);
    if (PutCall.length() == 0) {
        PutCall = L"P";
    }
    else {
        PutCall = (PutCall == L"P") ? L"C" : L"P";
    }
    TradeGrid_SetText(col, PutCall);
}


// ========================================================================================
// Handle when the Action trade grid cell is clicked.
// ========================================================================================
void TradeGrid_OnClickAction(TradeGrid* pData, GridColInfo* col)
{
    if (pData == nullptr) return;
    if (col == nullptr) return;

    // STO,BTO,STC,BTC

    std::wstring action = CustomLabel_GetText(col->hCtl);
    if (action == L"STO") {
        action = L"BTO";
    }
    else if (action == L"BTO") {
        action = L"STC";
    }
    else if (action == L"STC") {
        action = L"BTC";
    }
    else if (action == L"BTC") {
        action = L"STO";
    }
    else {
        action = L"STO";
    }

    TradeGrid_SetText(col, action);
}


// ========================================================================================
// Handle when the Line Reset trade grid cell is clicked.
// ========================================================================================
void TradeGrid_OnClickLineReset(TradeGrid* pData, GridColInfo* col)
{
    if (pData == nullptr) return;
    if (col == nullptr) return;

    int numCols = (pData->bShowOriginalQuantity) ? 8 : 7;

    // Determine the line being reset
    int colStart = (col->colData * numCols);

    if (pData->bShowOriginalQuantity) {
        CustomTextBox_SetText(pData->gridCols.at(colStart)->hCtl, L"");     // OrigQuantity
        CustomTextBox_SetText(pData->gridCols.at(colStart+1)->hCtl, L"");   // OpenQuantity
        CustomLabel_SetText(pData->gridCols.at(colStart+2)->hCtl, L"");     // Expiry Date
        CustomLabel_SetUserData(pData->gridCols.at(colStart+2)->hCtl, L"");  // ISO Date 
        CustomLabel_SetText(pData->gridCols.at(colStart+3)->hCtl, L"");     // DTE
        CustomTextBox_SetText(pData->gridCols.at(colStart+4)->hCtl, L"");   // Strike Price
        CustomLabel_SetText(pData->gridCols.at(colStart+5)->hCtl, L"");     // Put/Call
        CustomLabel_SetText(pData->gridCols.at(colStart+6)->hCtl, L"");     // Action
    }
    else {
        CustomTextBox_SetText(pData->gridCols.at(colStart)->hCtl, L"");     // OrigQuantity
        CustomLabel_SetText(pData->gridCols.at(colStart + 1)->hCtl, L"");     // Expiry Date
        CustomLabel_SetUserData(pData->gridCols.at(colStart + 1)->hCtl, L"");  // ISO Date 
        CustomLabel_SetText(pData->gridCols.at(colStart + 2)->hCtl, L"");     // DTE
        CustomTextBox_SetText(pData->gridCols.at(colStart + 3)->hCtl, L"");   // Strike Price
        CustomLabel_SetText(pData->gridCols.at(colStart + 4)->hCtl, L"");     // Put/Call
        CustomLabel_SetText(pData->gridCols.at(colStart + 5)->hCtl, L"");     // Action
    }
}


// ========================================================================================
// Handle when the Date Picker (Calendar) grid cell is clicked.
// ========================================================================================
void TradeGrid_OnClickDatePicker(TradeGrid* pData, GridColInfo* col)
{
    if (pData == nullptr) return;
    if (col == nullptr) return;

    std::wstring wszDate = CustomLabel_GetUserData(col->hCtl);
    Calendar_CreateDatePicker(pData->hParent, col->hCtl, wszDate, CalendarPickerReturnType::ShortDate, 2);
}


// ========================================================================================
// Get text from a trade grid cell (row, cell).
// ========================================================================================
std::wstring TradeGrid_GetText(HWND hCtl, int row, int col)
{
    std::wstring wszText;
    TradeGrid* pData = TradeGrid_GetOptions(hCtl);
    if (pData == nullptr) return wszText;

    if (row < 0 || row > 3) return wszText;
    if (col < 0 || col > 6) return wszText;

    // Determine the cell index
    int numCols = (pData->bShowOriginalQuantity) ? 8 : 7;
    int idx = (row * numCols) + col;

    if (pData->gridCols.at(idx)->colType == GridColType::TextBox) {
        wszText = AfxGetWindowText(GetDlgItem(pData->gridCols.at(idx)->hCtl,100));
    }
    else if (pData->gridCols.at(idx)->colType == GridColType::DatePicker) {
        // If the column is a DatePicker then return the ISO date rather
        // the wszText which would be the short display date.
        wszText = CustomLabel_GetUserData(pData->gridCols.at(idx)->hCtl);
    } 
    else {
        wszText = CustomLabel_GetText(pData->gridCols.at(idx)->hCtl);
    }

    return wszText;
}


// ========================================================================================
// Some trade grid cells are designated as "trigger cells" meaning that when their values
// change then that change will be populated to the other cells in the grid. For example, 
// when the first row leg quantity or leg expiry date changes then those changes get 
// populated to the other three rows in the trade grid.
// ========================================================================================
void TradeGrid_PopulateTriggerCells(HWND hWnd, auto col)
{
    TradeGrid* pData = TradeGrid_GetOptions(hWnd);
    if (pData == nullptr) return;

    std::wstring wszCellText = TradeGrid_GetText(hWnd, 0, 0);
    
    int intCellQuantity = 0;
    if (wszCellText.length()) {
        intCellQuantity = abs(stoi(wszCellText));   // will GPF if empty wszCellText string
    }

    int offset = (pData->bShowOriginalQuantity) ? 1 : 0;

    std::wstring wszISODate = CustomLabel_GetUserData(pData->gridCols.at(1+offset)->hCtl);
    std::wstring wszText;

    for (int i = 1; i < 4; ++i) {
        if (col->colType == GridColType::TextBox) {
            wszText = TradeGrid_GetText(hWnd, i, 0);
            if (wszText.length() != 0) {
                int intQuantity = 0;
                intQuantity = stoi(wszText);   // will GPF if empty wszText string
                int intNewQuantity = (intQuantity < 0) ? intCellQuantity * -1 : intCellQuantity;
                wszText = std::to_wstring(intNewQuantity);
                TradeGrid_SetColData(hWnd, i, 0, wszText);
            }
        }
        if (col->colType == GridColType::DatePicker) {
            wszText = TradeGrid_GetText(hWnd, i, 1);
            if (wszText.length() != 0) {
                TradeGrid_SetColData(hWnd, i, 1, wszISODate);
            }
        }
    }

    CustomTextBox_SetText(GetDlgItem(HWND_TRADEDIALOG, IDC_TRADEDIALOG_TXTQUANTITY), 
        std::to_wstring(intCellQuantity));
}

// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CALLBACK TradeGridProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    TradeGrid* pData = nullptr;

    if (uMsg != WM_CREATE) {
        pData = (TradeGrid*)GetWindowLongPtr(hWnd, 0);
    }


    switch (uMsg)
    {

    case WM_COMMAND:
        if (HIWORD(wParam) == EN_KILLFOCUS) {
            // A textbox in the grid has lost focus. Determine if this window has
            // the isTriggerCell flag set to true that will then populate its value
            // to the other cells in the grid table.
            HWND hCtl = (HWND)lParam;
            for (const auto& col : pData->gridCols) {
                if (col->hCtl == hCtl) {
                    if (col->isTriggerCell == true) {
                        TradeGrid_PopulateTriggerCells(hWnd, col);
                    }
                    break;
                }
            }
            return 0;
        }
        break;


    case MSG_DATEPICKER_DATECHANGED:
    {
        HWND hCtl = (HWND)lParam;
        int CtrlId = (int)wParam;

        if (hCtl == NULL) return 0;

        // If this date cell has the isTriggerCell then send a message to the parent to
        // populate the other rows in the grid with this new date data.
        for (const auto& col : pData->gridCols) {
            if (col->idCtrl == CtrlId) {
                if (col->isTriggerCell == true) {
                    TradeGrid_PopulateTriggerCells(hWnd, col);
                }
                break;
            }
        }

        TradeGrid_CalculateDTE(hWnd);
        return 0;
    }
    break;


    case MSG_CUSTOMLABEL_CLICK:
    {
        HWND hCtl = (HWND)lParam;
        int CtrlId = (int)wParam;

        if (hCtl == NULL) return 0;

        // Find the data for the table cell that was clicked on
        for (const auto& col : pData->gridCols) {
            if (col->idCtrl == CtrlId) {

                if (col->colType == GridColType::DatePicker) {
                    TradeGrid_OnClickDatePicker(pData, col);
                    break;
                }

                if (col->colType == GridColType::PutCallCombo) {
                    TradeGrid_OnClickPutCall(pData, col);
                    break;
                }

                if (col->colType == GridColType::ActionCombo) {
                    TradeGrid_OnClickAction(pData, col);
                    break;
                }
        
                if (col->colType == GridColType::LineReset) {
                    TradeGrid_OnClickLineReset(pData, col);
                    break;
                }

                break;
            }
        }
        return 0;
    }
    break;


    case WM_ERASEBKGND:
    {
        // Handle all of the painting in WM_PAINT
        return TRUE;
    }
    break;


    case WM_PAINT:
    {
        PAINTSTRUCT ps;

        HDC hdc = BeginPaint(hWnd, &ps);
        if (pData) {
            SaveDC(hdc);

            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP hbit = CreateCompatibleBitmap(hdc, ps.rcPaint.right, ps.rcPaint.bottom);
            SelectBitmap(memDC, hbit);

            Graphics graphics(memDC);
            int nWidth = (ps.rcPaint.right - ps.rcPaint.left);
            int nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);

            Color backColor(pData->BackColor);
            SolidBrush backBrush(backColor);
            graphics.FillRectangle(&backBrush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

            // Portion that displays behind the grid reset icons must be the same color
            // as the main TradeDialog.
            int nIconWidth = AfxScaleX(TRADEGRID_LINERESETICONWIDTH);
            backColor.SetValue(COLOR_GRAYDARK);
            backBrush.SetColor(backColor);
            graphics.FillRectangle(&backBrush, ps.rcPaint.right - nIconWidth, ps.rcPaint.top, nIconWidth, nHeight);

            // Copy the entire memory bitmap to the main display
            BitBlt(hdc, 0, 0, ps.rcPaint.right, ps.rcPaint.bottom, memDC, 0, 0, SRCCOPY);

            // Restore the original state of the DC
            RestoreDC(hdc, -1);

            // Cleanup
            DeleteObject(hbit);
            DeleteDC(memDC);
        }
        EndPaint(hWnd, &ps);
        return 0;
    }
    break;


    case WM_KEYDOWN:
    {
        if (wParam == VK_UP || wParam == VK_DOWN) {
            int direction = 0;
            if (wParam == VK_UP) direction = -7;   // up 1 row (minus 7 columns)
            if (wParam == VK_DOWN) direction = 7;  // down 1 row (plus 7 columns)

            HWND hFocus = GetParent(GetFocus());

            // Get the index of the current grid cell
            int idx = -1;
            for (int i = 0; i < (int)pData->gridCols.size(); ++i) {
                auto col = pData->gridCols.at(i);
                if (col->hCtl == hFocus) {
                    idx = i;
                    break;
                }
            }

            // Continue only if valid index found.
            if (idx == -1) return 0;

            idx += direction;
            if (idx >= 0 && idx < (int)pData->gridCols.size()) {
                SetFocus(pData->gridCols.at(idx)->hCtl);
            }

            return TRUE;
        }

        if (wParam == VK_TAB) {
            HWND hFocus = GetFocus();
            HWND hNextCtrl = NULL;
            if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
                hNextCtrl = GetNextDlgTabItem(hWnd, hFocus, TRUE);
            }
            else {
                hNextCtrl = GetNextDlgTabItem(hWnd, hFocus, FALSE);
            }
            SetFocus(hNextCtrl);
            return TRUE;
        }
    }
    break;


    case WM_NCDESTROY:
        if (pData) {
            DeleteBrush(pData->hBackBrush);
            delete(pData);
        }
        break;


    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;

}


// ========================================================================================
// Get the custom control data pointer.
// ========================================================================================
TradeGrid* TradeGrid_GetOptions(HWND hCtrl)
{
    TradeGrid* pData = (TradeGrid*)GetWindowLongPtr(hCtrl, 0);
    return pData;
}


// ========================================================================================
// Store the custom control data pointer.
// ========================================================================================
int TradeGrid_SetOptions(HWND hCtrl, TradeGrid* pData)
{
    if (pData == nullptr) return 0;

    SetWindowLongPtr(hCtrl, 0, (LONG_PTR)pData);
    AfxRedrawWindow(hCtrl);

    return 0;
}


// ========================================================================================
// Create the trade grid control.
// ========================================================================================
HWND CreateTradeGrid(
    HWND hWndParent,
    LONG_PTR CtrlId,
    int nLeft,
    int nTop,
    int nWidth,
    int nHeight,
    bool bShowOriginalQuantity)
{
    std::wstring wszClassName(L"TRADEGRID_CONTROL");

    WNDCLASSEX wcex{};

    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hWndParent, GWLP_HINSTANCE);

    if (GetClassInfoEx(hInst, wszClassName.c_str(), &wcex) == 0) {
        wcex.cbSize = sizeof(wcex);
        wcex.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = TradeGridProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = sizeof(HANDLE);    // make room to store a pointer to the class
        wcex.hInstance = hInst;
        wcex.hCursor = LoadCursor(NULL, (LPCWSTR)IDC_ARROW);
        wcex.hIcon = NULL;
        wcex.hIconSm = NULL;
        wcex.hbrBackground = (HBRUSH)HOLLOW_BRUSH;
        wcex.lpszMenuName = NULL;
        wcex.lpszClassName = wszClassName.c_str();
        if (RegisterClassEx(&wcex) == 0) return 0;
    }

    float rx = AfxScaleRatioX();
    float ry = AfxScaleRatioY();


    HWND hCtl =
        CreateWindowEx(0, wszClassName.c_str(), L"",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
            (int)(nLeft * rx), (int)(nTop * ry), (int)(nWidth * rx), (int)(nHeight * ry),
            hWndParent, (HMENU)CtrlId, hInst, (LPVOID)NULL);

    if (hCtl) {
        TradeGrid* pData = new TradeGrid;

        pData->hWindow = hCtl;
        pData->hParent = hWndParent;
        pData->hInst = hInst;
        pData->CtrlId = CtrlId;
        pData->BackColor = COLOR_BLACK;
        pData->hBackBrush = CreateSolidBrush(pData->BackColor);
        pData->bShowOriginalQuantity = bShowOriginalQuantity;

        TradeGrid_PopulateColumns(pData);
        TradeGrid_SetOptions(hCtl, pData);
    }

    return hCtl;
}


