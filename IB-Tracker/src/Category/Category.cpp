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
#include "Config/Config.h"

#include "Category.h"
#include "CategoryDialog.h"



// ========================================================================================
// Get the control id of the selected Category in the group.
// ========================================================================================
int CategoryControl_GetSelectedId(HWND hwnd)
{
    CustomLabel* pData = nullptr;

    for (int i = IDC_CATEGORYCONTROL_FIRST; i <= IDC_CATEGORYCONTROL_LAST; ++i) {
        HWND hCtl = GetDlgItem(hwnd, i);
        pData = CustomLabel_GetOptions(hCtl);

        if (pData != nullptr) {
            if (pData->IsSelected)
                return i;
        }
    }
    return -1;
}


// ========================================================================================
// Get the index of the selected Category in the group.
// ========================================================================================
int CategoryControl_GetSelectedIndex(HWND hwnd)
{
    return CategoryControl_GetSelectedId(hwnd) - IDC_CATEGORYCONTROL_FIRST;
}


// ========================================================================================
// Select the incoming control id in the Category group.
// ========================================================================================
void CategoryControl_SetSelectedId(HWND hwnd, int CtrlId)
{
    CustomLabel* pData = nullptr;

    for (int i = IDC_CATEGORYCONTROL_FIRST; i <= IDC_CATEGORYCONTROL_LAST; ++i) {
        HWND hCtl = GetDlgItem(hwnd, i);
        pData = CustomLabel_GetOptions(hCtl);

        if (pData != nullptr) {
            pData->IsSelected = false;
            CustomLabel_SetOptions(hCtl, pData);
        }
    }

    HWND hCtl = GetDlgItem(hwnd, CtrlId);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData != nullptr) {
        pData->IsSelected = true;
        CustomLabel_SetOptions(hCtl, pData);
    }

    AfxRedrawWindow(hwnd);
}


// ========================================================================================
// Select the incoming index in the Category group.
// ========================================================================================
void CategoryControl_SetSelectedIndex(HWND hwnd, int index)
{
    int CtrlId = index + IDC_CATEGORYCONTROL_FIRST;
    CategoryControl_SetSelectedId(hwnd, CtrlId);
}


// ========================================================================================
// Create child controls for the Window.
// ========================================================================================
void CategoryControl_OnCreate(HWND hwnd, bool AllowAllButton)
{
    HWND hCtl = NULL;

    int margin = 1;
    int nLeft = 0;

    CustomLabel* pData = nullptr;

    if (AllowAllButton == true) {
        hCtl = CreateCustomLabel(
            hwnd,
            IDC_CATEGORYCONTROL_ALL,
            CustomLabelType::TextOnly,
            nLeft, 0, 24, 24);
        pData = CustomLabel_GetOptions(hCtl);
        if (pData) {
            pData->BackColor = COLOR_GRAYMEDIUM;
            pData->SelectorColor = pData->BackColor;
            pData->TextColor = COLOR_WHITEDARK;
            pData->TextColorHot = COLOR_WHITEMEDIUM;
            pData->BackColorHot = COLOR_GRAYLIGHT;
            pData->BackColorButtonDown = COLOR_GRAYMEDIUM;
            pData->wszText = L"ALL";
            pData->wszToolTip = L"All Categories";
            pData->HotTestEnable = true;
            pData->PointerHot = CustomLabelPointer::Hand;
            CustomLabel_SetOptions(hCtl, pData);
        }
        nLeft += 24 + margin;
    }

    hCtl = CreateCustomLabel(
        hwnd,
        IDC_CATEGORYCONTROL_GRAY,
        CustomLabelType::TextOnly,
        nLeft, 0, 24, 24);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->BackColor = COLOR_GRAYMEDIUM;
        pData->SelectorColor = pData->BackColor;
        pData->TextColor = COLOR_WHITEDARK;
        pData->TextColorHot = COLOR_WHITEMEDIUM;
        pData->BackColorHot = COLOR_GRAYLIGHT;
        pData->BackColorButtonDown = COLOR_GRAYMEDIUM;
        pData->wszText = GLYPH_CIRCLE; 
        pData->wszToolTip = GetCategoryDescription(0);
        pData->HotTestEnable = true;
        pData->PointerHot = CustomLabelPointer::Hand;
        CustomLabel_SetOptions(hCtl, pData);
    }
    nLeft += 24 + margin;

    hCtl = CreateCustomLabel(
        hwnd,
        IDC_CATEGORYCONTROL_BLUE,
        CustomLabelType::TextOnly,
        nLeft, 0, 24, 24);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->BackColor = COLOR_GRAYMEDIUM;
        pData->SelectorColor = pData->BackColor;
        pData->TextColor = COLOR_BLUE;
        pData->TextColorHot = pData->TextColor;
        pData->BackColorHot = COLOR_GRAYLIGHT;
        pData->BackColorButtonDown = COLOR_GRAYMEDIUM;
        pData->wszText = GLYPH_CIRCLE; 
        pData->wszToolTip = GetCategoryDescription(1);
        pData->HotTestEnable = true;
        pData->PointerHot = CustomLabelPointer::Hand;
        CustomLabel_SetOptions(hCtl, pData);
    }
    nLeft += 24 + margin;

    hCtl = CreateCustomLabel(
        hwnd,
        IDC_CATEGORYCONTROL_PINK,
        CustomLabelType::TextOnly,
        nLeft, 0, 24, 24);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->BackColor = COLOR_GRAYMEDIUM;
        pData->SelectorColor = pData->BackColor;
        pData->TextColor = COLOR_PINK;
        pData->TextColorHot = pData->TextColor;
        pData->BackColorHot = COLOR_GRAYLIGHT;
        pData->BackColorButtonDown = COLOR_GRAYMEDIUM;
        pData->wszText = GLYPH_CIRCLE; 
        pData->wszToolTip = GetCategoryDescription(2);
        pData->HotTestEnable = true;
        pData->PointerHot = CustomLabelPointer::Hand;
        CustomLabel_SetOptions(hCtl, pData);
    }
    nLeft += 24 + margin;

    hCtl = CreateCustomLabel(
        hwnd,
        IDC_CATEGORYCONTROL_GREEN,
        CustomLabelType::TextOnly,
        nLeft, 0, 24, 24);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->BackColor = COLOR_GRAYMEDIUM;
        pData->SelectorColor = pData->BackColor;
        pData->TextColor = COLOR_GREEN;
        pData->TextColorHot = pData->TextColor;
        pData->BackColorHot = COLOR_GRAYLIGHT;
        pData->BackColorButtonDown = COLOR_GRAYMEDIUM;
        pData->wszText = GLYPH_CIRCLE; 
        pData->wszToolTip = GetCategoryDescription(3);
        pData->HotTestEnable = true;
        pData->PointerHot = CustomLabelPointer::Hand;
        CustomLabel_SetOptions(hCtl, pData);
    }
    nLeft += 24 + margin;

    hCtl = CreateCustomLabel(
        hwnd,
        IDC_CATEGORYCONTROL_ORANGE,
        CustomLabelType::TextOnly,
        nLeft, 0, 24, 24);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->BackColor = COLOR_GRAYMEDIUM;
        pData->SelectorColor = pData->BackColor;
        pData->TextColor = COLOR_ORANGE;
        pData->TextColorHot = pData->TextColor;
        pData->BackColorHot = COLOR_GRAYLIGHT;
        pData->BackColorButtonDown = COLOR_GRAYMEDIUM;
        pData->wszText = GLYPH_CIRCLE; 
        pData->wszToolTip = GetCategoryDescription(4);
        pData->HotTestEnable = true;
        pData->PointerHot = CustomLabelPointer::Hand;
        CustomLabel_SetOptions(hCtl, pData);
    }
    nLeft += 24 + margin;

    if (AllowAllButton == true) {
        hCtl = CreateCustomLabel(
            hwnd,
            IDC_CATEGORYCONTROL_SETUP,
            CustomLabelType::TextOnly,
            nLeft, 0, 24, 24);
        pData = CustomLabel_GetOptions(hCtl);
        if (pData) {
            pData->BackColor = COLOR_GRAYMEDIUM;
            pData->SelectorColor = pData->BackColor;
            pData->TextColor = COLOR_WHITEDARK;
            pData->TextColorHot = COLOR_WHITEMEDIUM;
            pData->BackColorHot = COLOR_GRAYLIGHT;
            pData->BackColorButtonDown = COLOR_GRAYMEDIUM;
            pData->wszText = GLYPH_SETUP;
            pData->wszToolTip = L"Configure Categories";
            pData->HotTestEnable = true;
            pData->PointerHot = CustomLabelPointer::Hand;
            CustomLabel_SetOptions(hCtl, pData);
        }
    }
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

    case MSG_CUSTOMLABEL_CLICK:
    {
        HWND hCtl = (HWND)lParam;
        int CtrlId = (int)wParam;

        if (hCtl == NULL) return 0;

        if (CtrlId == IDC_CATEGORYCONTROL_SETUP) {
            if (CategoryDialog_Show() == DIALOG_RETURN_OK) {
                CustomLabel_SetToolTip(GetDlgItem(hWnd, IDC_CATEGORYCONTROL_GRAY), GetCategoryDescription(0));
                CustomLabel_SetToolTip(GetDlgItem(hWnd, IDC_CATEGORYCONTROL_BLUE), GetCategoryDescription(1));
                CustomLabel_SetToolTip(GetDlgItem(hWnd, IDC_CATEGORYCONTROL_PINK), GetCategoryDescription(2));
                CustomLabel_SetToolTip(GetDlgItem(hWnd, IDC_CATEGORYCONTROL_GREEN), GetCategoryDescription(3));
                CustomLabel_SetToolTip(GetDlgItem(hWnd, IDC_CATEGORYCONTROL_ORANGE), GetCategoryDescription(4));
            }
        }
        else {
            CategoryControl_SetSelectedId(hWnd, CtrlId);
            SendMessage(pData->hParent, MSG_CATEGORY_CHANGED, CtrlId, 0);
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

        int selectId = CategoryControl_GetSelectedId(hWnd);
        RECT rc; GetWindowRect(GetDlgItem(hWnd, selectId), &rc);
        MapWindowPoints(HWND_DESKTOP, hWnd, (LPPOINT)&rc, 2);

        ARGB clrPen = pData->UnderlineColor;
        Pen pen(clrPen, (REAL)pData->UnderlineHeight);

        RectF rectF((REAL)rc.left- ps.rcPaint.left, (REAL)(ps.rcPaint.bottom - pData->UnderlineHeight),
            (REAL)rc.right-rc.left, (REAL)pData->UnderlineHeight);
        graphics.DrawRectangle(&pen, rectF);

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
// Create the Category control.
// ========================================================================================
HWND CreateCategoryControl(
    HWND hWndParent,
    int CtrlId,
    int nLeft, int nTop, int nWidth, int nHeight, bool AllowAllButton)
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

    HWND hCtl =
        CreateWindowEx(0, wszClassName.c_str(), L"",
            WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
            (int)(nLeft * rx), (int)(nTop * ry), 
            (int)(nWidth * rx), (int)((nHeight + 4) * ry),   // add room for underline
            hWndParent, (HMENU)CtrlId, hInst, (LPVOID)NULL);

    if (hCtl) {
        CategoryControl* pData = new CategoryControl;

        pData->hwnd = hCtl;
        pData->hParent = hWndParent;
        pData->CtrlId = CtrlId;
        pData->BackColor = (AllowAllButton == true) ? COLOR_BLACK : COLOR_GRAYDARK;

        CategoryControl_SetOptions(hCtl, pData);

        CategoryControl_OnCreate(hCtl, AllowAllButton);

        if (AllowAllButton) {
            CategoryControl_SetSelectedId(hCtl, IDC_CATEGORYCONTROL_ALL);
        }
        else {
            CategoryControl_SetSelectedId(hCtl, IDC_CATEGORYCONTROL_GRAY);
        }
    }

    return hCtl;
}


