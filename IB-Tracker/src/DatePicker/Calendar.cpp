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

#include "CustomLabel/CustomLabel.h"
#include "Calendar.h"


HWND HWND_CALENDAR = NULL;

CCalendar Calendar;

std::wstring wszTheSelectedDate;

// Control on parent window that new selected date will be stored in and displayed.
// That control must be a CustomLabel because we store the full ISO date in that
// control's UserData string.
HWND hTheUpdateParentCtl = NULL;
CalendarPickerReturnType TheUpdateDateReturnType = CalendarPickerReturnType::ISODate;



// ========================================================================================
// Process WM_SIZE message for window/dialog: Calendar
// ========================================================================================
void Calendar_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    SetWindowPos(GetDlgItem(hwnd, IDC_CALENDAR_CALENDAR), 0, 0, 0, cx, cy, SWP_NOZORDER | SWP_SHOWWINDOW);
    return;
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: Calendar
// ========================================================================================
BOOL Calendar_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    // Handle all of the painting in WM_PAINT
    return TRUE;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: Calendar
// ========================================================================================
void Calendar_OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    Graphics graphics(hdc);

    // Create the background brush
    Color backColor(COLOR_GRAYDARK);
    SolidBrush backBrush(backColor);

    // Paint the background using brush.
    int nWidth = (ps.rcPaint.right - ps.rcPaint.left);
    int nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&backBrush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

    EndPaint(hwnd, &ps);
}



// ========================================================================================
// Process WM_CREATE message for window/dialog: Calendar
// ========================================================================================
BOOL Calendar_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    HWND_CALENDAR = hwnd;

    HWND hCtl = CreateWindowEx(0,
        MONTHCAL_CLASS,
        L"",
        WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        0, 0, 0, 0, // resize it later
        Calendar.WindowHandle(),
        (HMENU)IDC_CALENDAR_CALENDAR,
        Calendar.hInst(),
        NULL);

    if (hCtl != NULL) {
        // If visual styles are active, this message has no effect except when wParam is MCSC_BACKGROUND.
        MonthCal_SetColor(hCtl, MCSC_BACKGROUND, Color(COLOR_GRAYMEDIUM).ToCOLORREF());

        SYSTEMTIME st{};
        st.wYear = AfxGetYear(wszTheSelectedDate);
        st.wMonth = AfxGetMonth(wszTheSelectedDate);
        st.wDay = AfxGetDay(wszTheSelectedDate);

        MonthCal_SetCurSel(hCtl, &st);
    }

    return TRUE;
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CCalendar::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Prevent a recursive calling of the WM_NCACTIVATE message as DestroyWindow deactivates the window.
    static bool destroyed = false;

    switch (msg)
    {
        HANDLE_MSG(m_hwnd, WM_CREATE, Calendar_OnCreate);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, Calendar_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, Calendar_OnPaint);
        HANDLE_MSG(m_hwnd, WM_SIZE, Calendar_OnSize);


    case WM_NOTIFY:
    {
        switch (((LPNMHDR)lParam)->code)
        {
        case MCN_SELECT:
            if (((LPNMHDR)lParam)->idFrom == IDC_CALENDAR_CALENDAR)
            {
                LPNMSELCHANGE lpNMSelChange = (LPNMSELCHANGE)lParam;

                wszTheSelectedDate = AfxMakeISODate(
                    lpNMSelChange->stSelStart.wYear,
                    lpNMSelChange->stSelStart.wMonth,
                    lpNMSelChange->stSelStart.wDay);

                std::wstring wszText;
                switch (TheUpdateDateReturnType)
                {
                case CalendarPickerReturnType::ShortDate:
                    wszText = AfxShortDate(wszTheSelectedDate);
                    break;
                case CalendarPickerReturnType::LongDate:
                    wszText = AfxLongDate(wszTheSelectedDate);
                    break;
                case CalendarPickerReturnType::ISODate:
                    wszText = wszTheSelectedDate;
                    break;
                }

                CustomLabel_SetUserData(hTheUpdateParentCtl, wszTheSelectedDate);
                CustomLabel_SetText(hTheUpdateParentCtl, wszText);
                SendMessage(GetParent(hTheUpdateParentCtl), MSG_DATEPICKER_DATECHANGED,
                    GetDlgCtrlID(hTheUpdateParentCtl), (LPARAM)hTheUpdateParentCtl);
                DestroyWindow(m_hwnd);


                return TRUE;
            }
            break;
        }
        return 0;
    }
    break;


    case WM_DESTROY:
    {
        // Reset our destroyed variable for future use of the DatePicker
        destroyed = false;
        HWND_CALENDAR = NULL;
        return 0;
    }
    break;


    case WM_NCACTIVATE:
    {
        // Detect that we have clicked outside the popup DatePicker and will now close it.
        if (wParam == false) {
            // Set our static flag to prevent recursion
            if (destroyed == false) {
                destroyed = true;
                DestroyWindow(m_hwnd);
            }
            return TRUE;
        }
        return 0;
    }
    break;


    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}


// ========================================================================================
// Create Calendar control and move it into position under the specified incoming control.
// ========================================================================================
HWND Calendar_CreateDatePicker(
    HWND hParent, HWND hParentCtl, std::wstring wszDate, 
    CalendarPickerReturnType DateReturnType, int NumCalendars)
{
    if (wszDate.length() == 0)
        wszDate = AfxCurrentDate();

    wszTheSelectedDate = wszDate;


    Calendar.Create(hParent, L"", 0, 0, 0, 0,
        WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);


    RECT rc; GetWindowRect(hParentCtl, &rc);
    SetWindowPos(Calendar.WindowHandle(), HWND_TOP,
        rc.left, rc.bottom,
        AfxScaleX(CALENDAR_WIDTH),
        AfxScaleY(CALENDAR_HEIGHT * (float)NumCalendars),
        SWP_SHOWWINDOW);


    // Set the module global hUpdateParentCtl after the above Calendar is created in
    // to ensure the variable address is correct.
    hTheUpdateParentCtl = hParentCtl;
    TheUpdateDateReturnType = DateReturnType;

    return Calendar.WindowHandle();
}

