//
// CustomVScrollBar CONTROL
// Copyright (C) 2023 Paul Squires, PlanetSquires Software
//

#include "pch.h"
#include "..\Utilities\AfxWin.h"
#include "CustomVScrollBar.h"
#include "..\TradeDialog\TradeDialog.h"


// ========================================================================================
// Calculate the RECT that holds the client coordinates of the scrollbar's vertical thumb
// Will return TRUE if RECT is not empty. 
// ========================================================================================
bool CustomVScrollBar::calcVThumbRect()
{
    // calculate the vertical scrollbar in client coordinates
    SetRectEmpty(&rc);
    int nTopIndex = SendMessage(hListBox, LB_GETTOPINDEX, 0, 0);
    
    RECT rcListBox{};
    GetClientRect(hListBox, &rcListBox);
    listBoxHeight = (rcListBox.bottom - rcListBox.top);
    itemHeight = ListBox_GetItemHeight(hListBox, 0);
    numItems = ListBox_GetCount(hListBox);

    // If no items exist then exit to avoid division by zero GPF's.
    if (numItems == 0) return FALSE;

    itemsPerPage = (int)(std::round(listBoxHeight / (float)itemHeight));
    thumbHeight = (int)(((float)itemsPerPage / (float)numItems) * (float)listBoxHeight);

    rc.left = rcListBox.left;
    rc.top = (int)(rcListBox.top + (((float)nTopIndex / (float)numItems) * (float)listBoxHeight));
    rc.right = rcListBox.right;
    rc.bottom = (rc.top + thumbHeight);

    // If the number of items in the listbox is less than what could display
    // on the screen then there is no need to show the scrollbar.
    return (numItems < itemsPerPage) ? FALSE : TRUE;
}


// ========================================================================================
// Windows message callback for the custom ScrollBar control.
// ========================================================================================
LRESULT CALLBACK CustomVScrollBarProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CustomVScrollBar* pData = nullptr;

    if (uMsg != WM_CREATE) {
        pData = (CustomVScrollBar*)GetWindowLongPtr(hWnd, 0);
    }

    switch (uMsg)
    {

    case WM_LBUTTONDOWN:
        {
            if (pData == nullptr) break;
            POINT pt;
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            pData->calcVThumbRect();

            if (PtInRect(&pData->rc, pt)) {
                pData->prev_pt = pt;
                pData->bDragActive = true;
                SetCapture(hWnd);
            }
            else {
                // we have clicked on a PageUp or PageDn
                int nTopIndex = SendMessage(pData->hListBox, LB_GETTOPINDEX, 0, 0);
                if (pt.y < pData->rc.top) {
                    nTopIndex = max(nTopIndex - pData->itemsPerPage, 0);
                    SendMessage(pData->hListBox, LB_SETTOPINDEX, nTopIndex, 0);
                    pData->calcVThumbRect();
                    AfxRedrawWindow(pData->hwnd);
                }
                else {
                    if (pt.y > pData->rc.bottom) {
                        int nMaxTopIndex = pData->numItems - pData->itemsPerPage;
                        nTopIndex = min(nTopIndex + pData->itemsPerPage, nMaxTopIndex);
                        SendMessage(pData->hListBox, LB_SETTOPINDEX, nTopIndex, 0);
                        pData->calcVThumbRect();
                        AfxRedrawWindow(pData->hwnd);
                    }
                }

            }
            break;
        }


        case WM_MOUSEMOVE:
        {
            if (pData == nullptr) break;
            if (pData->bDragActive) {
                POINT pt;
                pt.x = GET_X_LPARAM(lParam);
                pt.y = GET_Y_LPARAM(lParam);
                if (pt.y != pData->prev_pt.y) {
                    int delta = (pt.y - pData->prev_pt.y);

                    RECT rc; GetClientRect(hWnd, &rc);
                    pData->rc.top = max(0, pData->rc.top + delta);
                    pData->rc.top = min(pData->rc.top, rc.bottom - pData->thumbHeight);
                    pData->rc.bottom = pData->rc.top + pData->thumbHeight;

                    pData->prev_pt = pt;

                    int nPrevTopLine = SendMessage(pData->hListBox, LB_GETTOPINDEX, 0, 0);
                    int nTopLine = (int)std::round(pData->rc.top / (float)rc.bottom * pData->numItems);
                    if (nTopLine != nPrevTopLine)
                        SendMessage(pData->hListBox, LB_SETTOPINDEX, (WPARAM)nTopLine, 0);

                    AfxRedrawWindow(hWnd);
                }
            }
            break;
        }


        case WM_LBUTTONUP:
        {
            if (pData == nullptr) break;
            pData->bDragActive = false;
            pData->prev_pt.x = 0;
            pData->prev_pt.y = 0;
            ReleaseCapture();
            break;
        }

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

            SolidBrush backBrush(GetThemeColor(pData->ScrollBarBack));
            graphics.FillRectangle(&backBrush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

            if (pData->numItems > pData->itemsPerPage) {
                backBrush.SetColor(GetThemeColor(pData->ScrollBarThumb));
                graphics.FillRectangle(&backBrush, pData->rc.left, pData->rc.top, nWidth, pData->thumbHeight);

                Pen pen(GetThemeColor(pData->ScrollBarLine), 1);
                graphics.DrawLine(&pen, (INT)ps.rcPaint.left, (INT)ps.rcPaint.top, (INT)ps.rcPaint.left, (INT)ps.rcPaint.bottom);
            }

            // Copy the entire memory bitmap to the main display
            BitBlt(hdc, 0, 0, ps.rcPaint.right, ps.rcPaint.bottom, memDC, 0, 0, SRCCOPY);

            // Restore the original state of the DC
            RestoreDC(hdc, -1);

            // Cleanup
            DeleteObject(hbit);
            DeleteDC(memDC);

            EndPaint(hWnd, &ps);

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


//------------------------------------------------------------------------------ 
HWND CreateCustomVScrollBar(
    HWND hWndParent,
    LONG_PTR CtrlId,
    HWND hListBox
    )
{
    std::wstring wszClassName(L"CUSTOMVSCROLLBAR_CONTROL");

    WNDCLASSEX wcex{};

    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hWndParent, GWLP_HINSTANCE);

    if (GetClassInfoEx(hInst, wszClassName.c_str(), &wcex) == 0) {
        wcex.cbSize = sizeof(wcex);
        wcex.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = CustomVScrollBarProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = sizeof(HANDLE);    // make room to store a pointer to the class
        wcex.hInstance = hInst;
        wcex.hCursor = LoadCursor(NULL, (LPCWSTR)IDC_ARROW);
        wcex.hIcon = NULL;
        wcex.hIconSm = NULL;
        wcex.hbrBackground = (HBRUSH)WHITE_BRUSH;
        wcex.lpszMenuName = NULL;
        wcex.lpszClassName = wszClassName.c_str();
        if (RegisterClassEx(&wcex) == 0) return 0;
    }

    HWND hCtl =
        CreateWindowEx(0, wszClassName.c_str(), L"",
            WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
            0, 0, 0, 0,
            hWndParent, (HMENU)CtrlId, hInst, (LPVOID)NULL);

    if (hCtl) {
        CustomVScrollBar* pData = new CustomVScrollBar;

        pData->hwnd = hCtl;
        pData->hParent = hWndParent;
        pData->hListBox = hListBox;
        pData->CtrlId = CtrlId;
        pData->calcVThumbRect();

        SetWindowLongPtr(hCtl, 0, (LONG_PTR)pData);
    }

    return hCtl;
}


// ========================================================================================
// Retrieve the stored data pointer from the custom ScrollBar
// ========================================================================================
CustomVScrollBar* CustomVScrollBar_GetPointer(HWND hCtrl)
{
    CustomVScrollBar* pData = (CustomVScrollBar*)GetWindowLongPtr(hCtrl, 0);
    return pData;
}


// ========================================================================================
// Recalculate the ScrollBar thumb size and refresh display.
// ========================================================================================
void CustomVScrollBar_Recalculate(HWND hCtrl)
{
    CustomVScrollBar* pData = CustomVScrollBar_GetPointer(hCtrl);
    if (pData != nullptr) {
        pData->calcVThumbRect();
        AfxRedrawWindow(pData->hwnd);
    }
}


