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
#include "Utilities/AfxWin.h"
#include "Utilities/ListBoxData.h"
#include "Utilities/Colors.h"
#include "CustomLabel/CustomLabel.h"
#include "Utilities/UserMessages.h"
#include "ClosedTrades/ClosedTrades.h"
#include "MainWindow/MainWindow.h"
#include "Config/Config.h"

#include "Category.h"
#include "CategoryDialog.h"
#include "CategoryPopup.h"



// ========================================================================================
// Get the index of the selected Category in the group.
// ========================================================================================
int CategoryControl_GetSelectedIndex(HWND hwnd)
{
    return CustomLabel_GetUserDataInt(GetDlgItem(hwnd, IDC_CATEGORYCONTROL_COMBOBOX));
}


// ========================================================================================
// Select the incoming index in the Category group.
// ========================================================================================
void CategoryControl_SetSelectedIndex(HWND hwnd, int index)
{
    CustomLabel_SetUserDataInt(GetDlgItem(hwnd, IDC_CATEGORYCONTROL_COMBOBOX), index);
    CustomLabel_SetText(GetDlgItem(hwnd, IDC_CATEGORYCONTROL_COMBOBOX), GetCategoryDescription(index));
}


// ========================================================================================
// Create child controls for the Window.
// ========================================================================================
void CategoryControl_OnCreate(HWND hwnd)
{
    HWND hCtl = NULL;

    int nLeft = 0;

    std::wstring wszFontName = L"Segoe UI";
    int FontSize = 8;

    int category = CategoryControl_GetAllowAllCategories(hwnd) ? CATEGORY_ALL : CATEGORY_START;
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_CATEGORYCONTROL_COMBOBOX, GetCategoryDescription(category),
        COLOR_WHITEDARK, COLOR_GRAYMEDIUM, COLOR_GRAYMEDIUM, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::MiddleLeft, nLeft, 0, CATEGORYCONTROL_COMBOBOX_WIDTH, CATEGORYCONTROL_HEIGHT);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);
    CustomLabel_SetMousePointer(hCtl, CustomLabelPointer::Hand, CustomLabelPointer::Hand);
    CustomLabel_SetUserDataInt(hCtl, category);

    nLeft += CATEGORYCONTROL_COMBOBOX_WIDTH;
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_CATEGORYCONTROL_COMMAND, GLYPH_DROPDOWN,
        COLOR_WHITEDARK, COLOR_GRAYMEDIUM, COLOR_GRAYLIGHT, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::MiddleCenter, nLeft, 0, CATEGORYCONTROL_COMMAND_WIDTH, CATEGORYCONTROL_HEIGHT);
    CustomLabel_SetFont(hCtl, wszFontName, FontSize, true);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);

    nLeft += CATEGORYCONTROL_COMMAND_WIDTH + CATEGORYCONTROL_HMARGIN;


    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_CATEGORYCONTROL_SETUP, GLYPH_SETUP,
        COLOR_WHITEDARK, COLOR_GRAYMEDIUM, COLOR_GRAYLIGHT, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::MiddleCenter, nLeft, 0, CATEGORYCONTROL_SETUP_WIDTH, CATEGORYCONTROL_HEIGHT);
    CustomLabel_SetToolTip(hCtl, L"Configure Categories");

}


// ========================================================================================
// Windows message callback for the custom Category control.
// ========================================================================================
LRESULT CALLBACK CategoryControlProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CategoryControl* pData = nullptr;

    if (uMsg != WM_CREATE) {
        pData = (CategoryControl*)GetWindowLongPtr(hWnd, 0);
    }

    switch (uMsg)
    {

    case WM_KEYDOWN:
    {
        // Parent to handle the TAB navigation key to move amongst constrols.
        if (wParam == VK_TAB) {
            if (SendMessage(GetParent(hWnd), uMsg, wParam, lParam) == TRUE)
                return 0;
        }
    }
    break;


    case MSG_CUSTOMLABEL_CLICK:
    {
        HWND hCtl = (HWND)lParam;
        int CtrlId = (int)wParam;

        if (hCtl == NULL) return 0;

        if (CtrlId == IDC_CATEGORYCONTROL_COMBOBOX || CtrlId == IDC_CATEGORYCONTROL_COMMAND) {
            // Clicked on the combobox dropdown or label itself
            CategoryPopup_CreatePopup(hWnd, GetDlgItem(hWnd, IDC_CATEGORYCONTROL_COMBOBOX));
        }

        if (CtrlId == IDC_CATEGORYCONTROL_SETUP) {
            HWND hWndParent = GetParent(hWnd);
            if (hWndParent == HWND_CLOSEDTRADES) hWndParent = HWND_MAINWINDOW;
            if (CategoryDialog_Show(hWndParent) == DIALOG_RETURN_OK) {
                int SelectedCategory = CategoryControl_GetSelectedIndex(hWnd);   // Update the label in case text has changed
                CategoryControl_SetSelectedIndex(hWnd, SelectedCategory);
                SendMessage(GetParent(hWnd), MSG_CATEGORY_CATEGORYCHANGED, 0, 0);
            }
        }
        
        return 0;
    }
    break;


    case WM_ERASEBKGND:
        return TRUE;
        break;


    case WM_PAINT:
    {
        if (pData == nullptr) break;
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

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

        // Copy the entire memory bitmap to the main display
        BitBlt(hdc, 0, 0, ps.rcPaint.right, ps.rcPaint.bottom, memDC, 0, 0, SRCCOPY);

        // Restore the original state of the DC
        RestoreDC(hdc, -1);

        // Cleanup
        DeleteObject(hbit);
        DeleteDC(memDC);

        EndPaint(hWnd, &ps);

        return 0;
        break;
    }


    case WM_NCDESTROY:
        if (pData) delete(pData);
        break;


    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}


// ========================================================================================
// Retrieve the stored data pointer from the control
// ========================================================================================
CategoryControl* CategoryControl_GetOptions(HWND hCtrl)
{
    CategoryControl* pData = (CategoryControl*)GetWindowLongPtr(hCtrl, 0);
    return pData;
}


// ========================================================================================
// Store the data pointer into the control
// ========================================================================================
int CategoryControl_SetOptions(HWND hCtrl, CategoryControl* pData)
{
    if (pData == nullptr) return 0;

    if (pData->wszToolTip.length()) {
        if (pData->hToolTip) {
            AfxSetTooltipText(pData->hToolTip, hCtrl, pData->wszToolTip);
        }
    }

    SetWindowLongPtr(hCtrl, 0, (LONG_PTR)pData);
    AfxRedrawWindow(hCtrl);

    return 0;
}


// ========================================================================================
// Return if control will allow the "All Categories" menu option.
// ========================================================================================
bool CategoryControl_GetAllowAllCategories(HWND hwnd)
{
    CategoryControl* pData = CategoryControl_GetOptions(hwnd);
    if (pData != nullptr) {
        return pData->AllowAllCategories;
    }
    return false;
}


// ========================================================================================
// Create the Category control.
// ========================================================================================
HWND CreateCategoryControl(HWND hWndParent, int CtrlId, int nLeft, int nTop, int SelectedIndex, bool AllowAllCategories)
{
    std::wstring wszClassName(L"CATEGORYCONTROL_CONTROL");

    WNDCLASSEX wcex{};

    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hWndParent, GWLP_HINSTANCE);

    if (GetClassInfoEx(hInst, wszClassName.c_str(), &wcex) == 0) {
        wcex.cbSize = sizeof(wcex);
        wcex.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = CategoryControlProc;
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

    int nWidth = CATEGORYCONTROL_COMBOBOX_WIDTH + CATEGORYCONTROL_COMMAND_WIDTH + 
                    CATEGORYCONTROL_HMARGIN + CATEGORYCONTROL_SETUP_WIDTH;
    int nHeight = CATEGORYCONTROL_HEIGHT;

    HWND hCtl =
        CreateWindowEx(WS_EX_CONTROLPARENT, wszClassName.c_str(), L"",
            WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
            (int)(nLeft * rx), (int)(nTop * ry), 
            (int)(nWidth * rx), (int)(nHeight * ry),   
            hWndParent, (HMENU)CtrlId, hInst, (LPVOID)NULL);

    if (hCtl) {
        CategoryControl* pData = new CategoryControl;
        pData->hwnd = hCtl;
        pData->hParent = hWndParent;
        pData->CtrlId = CtrlId;
        pData->BackColor = (hWndParent == HWND_CLOSEDTRADES) ? COLOR_BLACK :COLOR_GRAYDARK;
        pData->AllowAllCategories = AllowAllCategories;

        CategoryControl_SetOptions(hCtl, pData);
        CategoryControl_OnCreate(hCtl);
        CategoryControl_SetSelectedIndex(hCtl, SelectedIndex);
    }

    return hCtl;
}


