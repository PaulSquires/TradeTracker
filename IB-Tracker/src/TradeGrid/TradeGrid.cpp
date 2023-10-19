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


// ========================================================================================
// Helper function to set a column's data and update display 
// ========================================================================================
void TradeGrid_SetColData(HWND hGrid, int row, int col, const std::wstring& text)
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
        CustomLabel_SetText(pCol->hCtl, text);
        break;

    case GridColType::TextBox:
        AfxSetWindowText(pCol->hCtl, text);
        break;

    case GridColType::DatePicker:
        CustomLabel_SetUserData(pCol->hCtl, text);
        CustomLabel_SetText(pCol->hCtl, AfxShortDate(text));
        break;

    case GridColType::PutCallCombo:
        CustomLabel_SetText(pCol->hCtl, text);
        break;

    case GridColType::ActionCombo:
        CustomLabel_SetText(pCol->hCtl, text);
        if (pCol->colType == GridColType::ActionCombo) {
            DWORD clr = COLOR_RED;
            if (text == L"BTO" || text == L"BTC") clr = COLOR_GREEN;
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
        
    std::wstring expiry_date;
    std::wstring wszDTE;

    for (int i = 0; i < 4; ++i) {
        int colStart = 0;
        int expiry_dateCol = 0;
        int dteCol = 0;
        
        if (pData->bShowOriginalQuantity) {
            colStart = i * 8;
            expiry_dateCol = colStart + 2;
            dteCol = colStart + 3;
        }
        else {
            colStart = i * 7;
            expiry_dateCol = colStart + 1;
            dteCol = colStart + 2;
        }

        expiry_date = CustomLabel_GetUserData(pData->gridCols.at(expiry_dateCol)->hCtl);
        
        if (transDate.length() != 10) continue;
        if (expiry_date.length() != 10) continue;
        
        int days = AfxDaysBetween(transDate, expiry_date);
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

    DWORD light_back_color = COLOR_GRAYLIGHT;
    DWORD light_text_color = COLOR_WHITELIGHT;

    DWORD dark_back_color = COLOR_GRAYMEDIUM;
    DWORD dark_text_color = COLOR_WHITEDARK;

    int nTop = 0;
    int nLeft = 0;
    int nWidth = 50;
    int nHeight = 23;
    
    int vsp = 2;
    int hsp = 2;
    
    int horiz_text_margin = 0;
    int vert_text_margin = 3;
    
    int nTotalWidth = 0;
    int nTotalHeight = 0;

    CustomLabel* pLabelData = nullptr;

    for (int row = 0; row < 4; ++row) {
        // All are center except Quantity which is right align.
        GridColInfo* col;


        // QUANTITY (ORIGINAL)
        if (pData->bShowOriginalQuantity == true) {
            nWidth = 25;
            hCtl = CreateCustomTextBox(pData->hWindow, idCtrl, false, ES_RIGHT, L"", nLeft, nTop, nWidth, nHeight);
            CustomTextBox_SetNumericAttributes(hCtl, 0, CustomTextBoxNegative::allow, CustomTextBoxFormatting::disallow);
            CustomTextBox_SetColors(hCtl, light_text_color, dark_back_color);
            CustomTextBox_SetMargins(hCtl, horiz_text_margin, vert_text_margin);
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
        hCtl = CreateCustomTextBox(pData->hWindow, idCtrl, false, ES_RIGHT, L"", nLeft, nTop, nWidth, nHeight);
        CustomTextBox_SetNumericAttributes(hCtl, 0, CustomTextBoxNegative::allow, CustomTextBoxFormatting::disallow);
        CustomTextBox_SetColors(hCtl, light_text_color, dark_back_color);
        CustomTextBox_SetMargins(hCtl, horiz_text_margin, vert_text_margin);
        col = new GridColInfo;
        col->hCtl = hCtl;
        col->idCtrl = idCtrl;
        col->colType = GridColType::TextBox;
        pData->gridCols.push_back(col);
        idCtrl++;
        nLeft = nLeft + nWidth + hsp;

        nWidth = 50;

        // EXPIRY DATE
        hCtl = CustomLabel_SimpleLabel(pData->hWindow, idCtrl, 
            L"", COLOR_WHITELIGHT, COLOR_GRAYLIGHT,
            CustomLabelAlignment::middle_center, nLeft, nTop, nWidth, nHeight);
        CustomLabel_SetMousePointer(hCtl, CustomLabelPointer::hand, CustomLabelPointer::hand);
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
            CustomLabelAlignment::middle_center, nLeft, nTop, nWidth, nHeight);
        col = new GridColInfo;
        col->hCtl = hCtl;
        col->idCtrl = idCtrl;
        col->colType = GridColType::Label;
        pData->gridCols.push_back(col);
        idCtrl++;
        nLeft = nLeft + nWidth + hsp;

        // STRIKE PRICE
        hCtl = CreateCustomTextBox(pData->hWindow, idCtrl, false, ES_CENTER, L"", nLeft, nTop, nWidth, nHeight);
        CustomTextBox_SetNumericAttributes(hCtl, 5, CustomTextBoxNegative::disallow, CustomTextBoxFormatting::disallow);
        CustomTextBox_SetColors(hCtl, light_text_color, light_back_color);
        CustomTextBox_SetMargins(hCtl, horiz_text_margin, vert_text_margin);
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
            CustomLabelAlignment::middle_center, nLeft, nTop, (nWidth/2), nHeight);
        CustomLabel_SetMousePointer(hCtl, CustomLabelPointer::hand, CustomLabelPointer::hand);
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
            CustomLabelAlignment::middle_center, nLeft, nTop, nWidth, nHeight);
        CustomLabel_SetMousePointer(hCtl, CustomLabelPointer::hand, CustomLabelPointer::hand);
        col = new GridColInfo;
        col->hCtl = hCtl;
        col->idCtrl = idCtrl;
        col->colType = GridColType::ActionCombo;
        pData->gridCols.push_back(col);
        idCtrl++;
        nLeft = nLeft + nWidth + hsp;


        // RESET LINE INDICATOR
        hCtl = CreateCustomLabel(pData->hWindow, idCtrl,
            CustomLabelType::text_only, nLeft, nTop, TRADEGRID_LINERESETICONWIDTH, nHeight);
        pLabelData = CustomLabel_GetOptions(hCtl);
        if (pLabelData) {
            pLabelData->text = GLYPH_RESETLINE;
            pLabelData->text_hot = GLYPH_RESETLINE;
            pLabelData->hot_test_enable = true;
            pLabelData->back_color = COLOR_GRAYDARK; 
            pLabelData->back_color_hot = COLOR_GRAYMEDIUM;
            pLabelData->back_color_button_down = COLOR_GRAYLIGHT; 
            pLabelData->text_color = COLOR_WHITEDARK;
            pLabelData->text_color_hot = COLOR_WHITELIGHT;
            pLabelData->text_alignment = CustomLabelAlignment::middle_center;
            pLabelData->tooltip_text = L"Reset line";
            pLabelData->pointer_hot = CustomLabelPointer::arrow;
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
void TradeGrid_SetText(GridColInfo* col, std::wstring text)
{
    if (col == nullptr) return;

    CustomLabel_SetText(col->hCtl, text);

    if (col->colType == GridColType::ActionCombo) {
        DWORD clr = COLOR_RED;
        if (text == L"BTO" || text == L"BTC") clr = COLOR_GREEN;
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
        CustomTextBox_SetText(pData->gridCols.at(colStart)->hCtl, L"");     // original_quantity
        CustomTextBox_SetText(pData->gridCols.at(colStart+1)->hCtl, L"");   // open_quantity
        CustomLabel_SetText(pData->gridCols.at(colStart+2)->hCtl, L"");     // Expiry Date
        CustomLabel_SetUserData(pData->gridCols.at(colStart+2)->hCtl, L"");  // ISO Date 
        CustomLabel_SetText(pData->gridCols.at(colStart+3)->hCtl, L"");     // DTE
        CustomTextBox_SetText(pData->gridCols.at(colStart+4)->hCtl, L"");   // Strike Price
        CustomLabel_SetText(pData->gridCols.at(colStart+5)->hCtl, L"");     // Put/Call
        CustomLabel_SetText(pData->gridCols.at(colStart+6)->hCtl, L"");     // Action
    }
    else {
        CustomTextBox_SetText(pData->gridCols.at(colStart)->hCtl, L"");     // original_quantity
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

    std::wstring date_text = CustomLabel_GetUserData(col->hCtl);
    Calendar_CreateDatePicker(pData->hParent, col->hCtl, date_text, CalendarPickerReturnType::short_date, 2);
}


// ========================================================================================
// Get text from a trade grid cell (row, cell).
// ========================================================================================
std::wstring TradeGrid_GetText(HWND hCtl, int row, int col)
{
    std::wstring text;
    TradeGrid* pData = TradeGrid_GetOptions(hCtl);
    if (pData == nullptr) return text;

    if (row < 0 || row > 3) return text;
    if (col < 0 || col > 6) return text;

    // Determine the cell index
    int numCols = (pData->bShowOriginalQuantity) ? 8 : 7;
    int idx = (row * numCols) + col;

    if (pData->gridCols.at(idx)->colType == GridColType::TextBox) {
        text = AfxGetWindowText(GetDlgItem(pData->gridCols.at(idx)->hCtl,100));
    }
    else if (pData->gridCols.at(idx)->colType == GridColType::DatePicker) {
        // If the column is a DatePicker then return the ISO date rather
        // the text which would be the short display date.
        text = CustomLabel_GetUserData(pData->gridCols.at(idx)->hCtl);
    } 
    else {
        text = CustomLabel_GetText(pData->gridCols.at(idx)->hCtl);
    }

    return text;
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
    
    int offset = (pData->bShowOriginalQuantity) ? 1 : 0;

    std::wstring wszISODate = CustomLabel_GetUserData(pData->gridCols.at(1+offset)->hCtl);
    std::wstring text;

    for (int i = 1; i < 4; ++i) {
        if (col->colType == GridColType::DatePicker) {
            text = TradeGrid_GetText(hWnd, i, 1);
            if (text.length() != 0) {
                TradeGrid_SetColData(hWnd, i, 1, wszISODate);
            }
        }
    }
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

            Color back_color(pData->back_color);
            SolidBrush back_brush(back_color);
            graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

            // Portion that displays behind the grid reset icons must be the same color
            // as the main TradeDialog.
            int nIconWidth = AfxScaleX(TRADEGRID_LINERESETICONWIDTH);
            back_color.SetValue(COLOR_GRAYDARK);
            back_brush.SetColor(back_color);
            graphics.FillRectangle(&back_brush, ps.rcPaint.right - nIconWidth, ps.rcPaint.top, nIconWidth, nHeight);

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


    case WM_DESTROY:
    {
        if (pData) {
            for (auto& col : pData->gridCols) {
                delete(col);
            }
        }
    }
    break;


    case WM_NCDESTROY:
    {
        if (pData) {
            DeleteBrush(pData->hback_brush);
            delete(pData);
        }
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
    std::wstring class_name_text(L"TRADEGRID_CONTROL");

    WNDCLASSEX wcex{};

    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hWndParent, GWLP_HINSTANCE);

    if (GetClassInfoEx(hInst, class_name_text.c_str(), &wcex) == 0) {
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
        wcex.lpszClassName = class_name_text.c_str();
        if (RegisterClassEx(&wcex) == 0) return 0;
    }

    float rx = AfxScaleRatioX();
    float ry = AfxScaleRatioY();


    HWND hCtl =
        CreateWindowEx(0, class_name_text.c_str(), L"",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
            (int)(nLeft * rx), (int)(nTop * ry), (int)(nWidth * rx), (int)(nHeight * ry),
            hWndParent, (HMENU)CtrlId, hInst, (LPVOID)NULL);

    if (hCtl) {
        TradeGrid* pData = new TradeGrid;

        pData->hWindow = hCtl;
        pData->hParent = hWndParent;
        pData->hInst = hInst;
        pData->CtrlId = (int)CtrlId;
        pData->back_color = COLOR_BLACK;
        pData->hback_brush = CreateSolidBrush(pData->back_color);
        pData->bShowOriginalQuantity = bShowOriginalQuantity;

        TradeGrid_PopulateColumns(pData);
        TradeGrid_SetOptions(hCtl, pData);
    }

    return hCtl;
}


