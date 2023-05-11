//
// CUSTOM TEXTBOX CONTROL
// Copyright (C) 2023 Paul Squires, PlanetSquires Software
// This control should be able to handle all different types of labels and will act
// like a button, hot tracking, pictures, etc.
//

#include "pch.h"
#include "..\Utilities\CWindowBase.h"
#include "MainWindow/MainWindow.h"

#include "CustomCalendar.h"


extern CMainWindow Main;



//------------------------------------------------------------------------------ 
LRESULT CALLBACK CustomCalendarProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CustomCalendar* pData = nullptr;

    if (uMsg != WM_CREATE) {
        pData = (CustomCalendar*)GetWindowLongPtr(hWnd, 0);
    }


    switch (uMsg)
    {


    case WM_GETTEXT:
    case WM_SETTEXT:
    case WM_GETTEXTLENGTH:
    {
        //return SendMessage(pData->hTextBox, uMsg, wParam, lParam);
    }
    break;


    case WM_SETFOCUS:
    {
//        SetFocus(pData->hTextBox);
    }
    break;


    case WM_KILLFOCUS:
    {
        DestroyWindow(pData->hWindow);
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
            delete(pData);
        }
        break;


    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;

}



//------------------------------------------------------------------------------ 
CustomCalendar* CustomCalendar_GetOptions(HWND hCtrl)
{
    CustomCalendar* pData = (CustomCalendar*)GetWindowLongPtr(hCtrl, 0);
    return pData;
}


//------------------------------------------------------------------------------ 
int CustomCalendar_SetOptions(HWND hCtrl, CustomCalendar* pData)
{
    if (pData == nullptr) return 0;

    SetWindowLongPtr(hCtrl, 0, (LONG_PTR)pData);
    AfxRedrawWindow(hCtrl);

    return 0;
}


//------------------------------------------------------------------------------ 
HWND CreateCustomCalendar(
    HWND hWndParent,
    LONG_PTR CtrlId,
    std::wstring wszDate)
{
    std::wstring wszClassName(L"CUSTOMCALENDAR_CONTROL");

    WNDCLASSEX wcex{};

    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hWndParent, GWLP_HINSTANCE);

    if (GetClassInfoEx(hInst, wszClassName.c_str(), &wcex) == 0) {
        wcex.cbSize = sizeof(wcex);
        wcex.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = CustomCalendarProc;
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
            WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
            0, 0, 0, 0,
            hWndParent, (HMENU)NULL, hInst, (LPVOID)NULL);

    if (hCtl) {
        CustomCalendar* pData = new CustomCalendar;

        pData->hWindow = hCtl;
        pData->hParent = hWndParent;
        pData->hInst = hInst;
        pData->CtrlId = CtrlId;
        pData->BackColor = GetThemeCOLORREF(ThemeElement::GrayDark);
        pData->hBackBrush = CreateSolidBrush(pData->BackColor);

        // Create the child embedded month calendar control
        pData->hMonthCal =
            CreateWindowEx(0,
            MONTHCAL_CLASS,
            L"",
            WS_BORDER | WS_CHILD | WS_VISIBLE | MCS_DAYSTATE,
            0, 0, 0, 0, // resize it later
            hCtl,
            (HMENU)100,
            hInst,
            NULL);        
        
        SetWindowTheme(pData->hMonthCal , L"", L"");

        // Subclass the child TextBox if pWndProc is not null
        //if (pData->hTextBox)
        //    SetWindowSubclass(pData->hTextBox, CustomTextBox_SubclassProc, 100, NULL);

        if (pData->hMonthCal != NULL) {
             // Get the size required to show an entire month.
            RECT rc;
            MonthCal_GetMinReqRect(pData->hMonthCal, &rc);

            // Resize the control now that the size values have been obtained.
            SetWindowPos(pData->hMonthCal, 0, 0, 0, rc.right, rc.bottom, SWP_NOZORDER);

            // Resize the parent window to the dimensions of the calendar and display
            // immediately under the parent control.
            RECT rcParent;
            GetWindowRect(hWndParent, &rcParent);
            SetWindowPos(hCtl, 0, rcParent.left, rcParent.bottom, rc.right, rc.bottom,
                SWP_NOZORDER | SWP_SHOWWINDOW);
        }

        CustomCalendar_SetOptions(hCtl, pData);

    }

    return hCtl;
}


