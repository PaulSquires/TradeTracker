//
// CUSTOM TEXTBOX CONTROL
// Copyright (C) 2023 Paul Squires, PlanetSquires Software
// This control should be able to handle all different types of labels and will act
// like a button, hot tracking, pictures, etc.
//

#include "pch.h"
#include "..\Utilities\CWindowBase.h"
#include "..\MainWindow\MainWindow.h"

#include "CustomTradeGrid.h"


extern CMainWindow Main;



//------------------------------------------------------------------------------ 
void CustomTradeGrid_PopulateColumns(CustomTradeGrid* pData)
{
    if (pData == nullptr) return;

    // Setup the lines and columns

    // The grid consists of 4 rows each with 6 columns, therefore
    // we need an array of 24 TextBoxes.
    pData->gridCols.reserve(24);

    HWND hCtl = NULL;
    int idCtrl = IDC_CUSTOMTRADEGRID_FIRSTCONTROL;

    COLORREF lightBackColor = GetThemeCOLORREF(ThemeElement::GrayLight);
    COLORREF lightTextColor = GetThemeCOLORREF(ThemeElement::WhiteLight);

    COLORREF darkBackColor = GetThemeCOLORREF(ThemeElement::GrayMedium);
    COLORREF darkTextColor = GetThemeCOLORREF(ThemeElement::WhiteDark);

    int nTop = 0;
    int nLeft = 0;
    int nWidth = 50;
    int nHeight = 23;
    
    int vsp = 3;
    int hsp = 2;
    
    int HTextMargin = 0;
    int VTextMargin = 4;
    
    int nTotalWidth = 0;
    int nTotalHeight = 0;

    std::wstring wszFontName = L"Segoe UI";
    int FontSize = 8;

    CustomLabel* pLabelData = nullptr;

    for (int row = 0; row < 4; ++row) {
        // All are center except Quantity which is right align.
        GridColInfo* col;

        // QUANTITY
        hCtl = CreateCustomTextBox(pData->hWindow, idCtrl, ES_RIGHT, L"-1", nLeft, nTop, nWidth, nHeight);
        CustomTextBox_SetFont(hCtl, wszFontName, FontSize);
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

        // EXPIRY DATE
        hCtl = CustomLabel_SimpleLabel(pData->hWindow, idCtrl, 
            L"May 30", ThemeElement::WhiteLight, ThemeElement::GrayLight,
            CustomLabelAlignment::MiddleCenter, nLeft, nTop, nWidth, nHeight);
        CustomLabel_SetFont(hCtl, wszFontName, FontSize);
        col = new GridColInfo;
        col->hCtl = hCtl;
        col->idCtrl = idCtrl;
        col->colType = GridColType::Label;
        pData->gridCols.push_back(col);
        idCtrl++;
        nLeft = nLeft + nWidth + hsp;

        // DTE
        hCtl = CustomLabel_SimpleLabel(pData->hWindow, idCtrl,
            L"24d", ThemeElement::WhiteDark, ThemeElement::GrayMedium,
            CustomLabelAlignment::MiddleCenter, nLeft, nTop, nWidth, nHeight);
        CustomLabel_SetFont(hCtl, wszFontName, FontSize);
        col = new GridColInfo;
        col->hCtl = hCtl;
        col->idCtrl = idCtrl;
        col->colType = GridColType::Label;
        pData->gridCols.push_back(col);
        idCtrl++;
        nLeft = nLeft + nWidth + hsp;

        // STRIKE PRICE
        hCtl = CreateCustomTextBox(pData->hWindow, idCtrl, ES_CENTER, L"140.5", nLeft, nTop, nWidth, nHeight);
        CustomTextBox_SetFont(hCtl, wszFontName, FontSize);
        CustomTextBox_SetNumericAttributes(hCtl, 2, CustomTextBoxNegative::Disallow, CustomTextBoxFormatting::Disallow);
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
            L"P", ThemeElement::WhiteDark, ThemeElement::GrayMedium,
            CustomLabelAlignment::MiddleCenter, nLeft, nTop, (nWidth/2), nHeight);
        CustomLabel_SetFont(hCtl, wszFontName, FontSize);
        col = new GridColInfo;
        col->hCtl = hCtl;
        col->idCtrl = idCtrl;
        col->colType = GridColType::PutCallCombo;
        pData->gridCols.push_back(col);
        idCtrl++;
        nLeft = nLeft + (nWidth/2) + hsp;

        // ACTION
        hCtl = CustomLabel_SimpleLabel(pData->hWindow, idCtrl,
            L"STO", ThemeElement::WhiteLight, ThemeElement::GrayLight,
            CustomLabelAlignment::MiddleCenter, nLeft, nTop, nWidth, nHeight);
        CustomLabel_SetFont(hCtl, wszFontName, FontSize);
        col = new GridColInfo;
        col->hCtl = hCtl;
        col->idCtrl = idCtrl;
        col->colType = GridColType::ActionCombo;
        pData->gridCols.push_back(col);
        idCtrl++;
        nLeft = nLeft + nWidth + hsp;


        // RESET LINE INDICATOR
        hCtl = CreateCustomLabel(pData->hWindow, idCtrl,
            CustomLabelType::TextOnly, nLeft, nTop, CUSTOMTRADEGRID_LINERESETICONWIDTH, nHeight);
        pLabelData = CustomLabel_GetOptions(hCtl);
        if (pLabelData) {
            pLabelData->wszText = L"x";
            pLabelData->wszTextHot = L"x";
            pLabelData->HotTestEnable = true;
            pLabelData->BackColor = ThemeElement::GrayDark; 
            pLabelData->TextColor = ThemeElement::WhiteDark;
            pLabelData->BackColorHot = ThemeElement::GrayDark;
            pLabelData->TextColorHot = ThemeElement::WhiteLight;
            pLabelData->TextAlignment = CustomLabelAlignment::MiddleLeft;
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
    nTotalWidth += AfxScaleX(CUSTOMTRADEGRID_LINERESETICONWIDTH);

    // Resize the control container to fit the size of the trade grid.
    SetWindowPos(pData->hWindow, 0, 0, 0, nTotalWidth, nTotalHeight, SWP_NOMOVE | SWP_NOZORDER);

}


//------------------------------------------------------------------------------ 
void CustomTradeGrid_SetText(GridColInfo* col, std::wstring wszText)
{
    if (col == nullptr) return;

    CustomLabel_SetText(col->hCtl, wszText);

    if (col->colType == GridColType::ActionCombo) {
        ThemeElement clr = ThemeElement::Red;
        if (wszText == L"BTO" || wszText == L"BTC") clr = ThemeElement::Green;
        CustomLabel_SetTextColor(col->hCtl, clr);
    }
}


//------------------------------------------------------------------------------ 
void CustomTradeGrid_OnClickPutCall(CustomTradeGrid* pData, GridColInfo* col)
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
    CustomTradeGrid_SetText(col, PutCall);
}


//------------------------------------------------------------------------------ 
void CustomTradeGrid_OnClickAction(CustomTradeGrid* pData, GridColInfo* col)
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

    CustomTradeGrid_SetText(col, action);
}


//------------------------------------------------------------------------------ 
void CustomTradeGrid_OnClickLineReset(CustomTradeGrid* pData, GridColInfo* col)
{
    if (pData == nullptr) return;
    if (col == nullptr) return;

    // Determine the line being reset
    int colStart = (col->colData * 7);

    CustomTextBox_SetText(pData->gridCols.at(colStart)->hCtl, L"");     // Quantity
    CustomLabel_SetText(pData->gridCols.at(colStart+1)->hCtl, L"");     // Expiry Date
    CustomLabel_SetText(pData->gridCols.at(colStart+2)->hCtl, L"0d");   // DTE
    CustomTextBox_SetText(pData->gridCols.at(colStart+3)->hCtl, L"");   // Strike Price
    CustomLabel_SetText(pData->gridCols.at(colStart+4)->hCtl, L"");     // Put/Call
    CustomLabel_SetText(pData->gridCols.at(colStart+5)->hCtl, L"");     // Action
}


//------------------------------------------------------------------------------ 
LRESULT CALLBACK CustomTradeGridProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CustomTradeGrid* pData = nullptr;

    if (uMsg != WM_CREATE) {
        pData = (CustomTradeGrid*)GetWindowLongPtr(hWnd, 0);
    }


    switch (uMsg)
    {

    case MSG_CUSTOMLABEL_CLICK:
    {
        // Find the data for the table cell that was clicked on
        for (const auto& col : pData->gridCols) {
            if (col->idCtrl == wParam) {

                if (col->colType == GridColType::PutCallCombo) {
                    CustomTradeGrid_OnClickPutCall(pData, col);
                    break;
                }

                if (col->colType == GridColType::ActionCombo) {
                    CustomTradeGrid_OnClickAction(pData, col);
                    break;
                }
        
                if (col->colType == GridColType::LineReset) {
                    CustomTradeGrid_OnClickLineReset(pData, col);
                    break;
                }

                break;
            }
        }
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

            Color backColor;
            backColor.SetFromCOLORREF(pData->BackColor);
            SolidBrush backBrush(backColor);
            graphics.FillRectangle(&backBrush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

            // Portion that displays behind the grid reset icons must be the same color
            // as the main TradeDialog.
            int nIconWidth = AfxScaleX(CUSTOMTRADEGRID_LINERESETICONWIDTH);
            backColor.SetFromCOLORREF(GetThemeCOLORREF(ThemeElement::GrayDark));
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


//------------------------------------------------------------------------------ 
CustomTradeGrid* CustomTradeGrid_GetOptions(HWND hCtrl)
{
    CustomTradeGrid* pData = (CustomTradeGrid*)GetWindowLongPtr(hCtrl, 0);
    return pData;
}


//------------------------------------------------------------------------------ 
int CustomTradeGrid_SetOptions(HWND hCtrl, CustomTradeGrid* pData)
{
    if (pData == nullptr) return 0;

    SetWindowLongPtr(hCtrl, 0, (LONG_PTR)pData);
    AfxRedrawWindow(hCtrl);

    return 0;
}


//------------------------------------------------------------------------------ 
HWND CreateCustomTradeGrid(
    HWND hWndParent,
    LONG_PTR CtrlId,
    int nLeft,
    int nTop,
    int nWidth,
    int nHeight)
{
    std::wstring wszClassName(L"CUSTOMTRADEGRID_CONTROL");

    WNDCLASSEX wcex{};

    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hWndParent, GWLP_HINSTANCE);

    if (GetClassInfoEx(hInst, wszClassName.c_str(), &wcex) == 0) {
        wcex.cbSize = sizeof(wcex);
        wcex.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = CustomTradeGridProc;
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
        CustomTradeGrid* pData = new CustomTradeGrid;

        pData->hWindow = hCtl;
        pData->hParent = hWndParent;
        pData->hInst = hInst;
        pData->CtrlId = CtrlId;
        pData->BackColor = GetThemeCOLORREF(ThemeElement::Black);
        pData->hBackBrush = CreateSolidBrush(pData->BackColor);

        CustomTradeGrid_PopulateColumns(pData);
        CustomTradeGrid_SetOptions(hCtl, pData);
    }

    return hCtl;
}


