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

std::wstring the_selected_date;

// Control on parent window that new selected date will be stored in and displayed.
// That control must be a CustomLabel because we store the full ISO date in that
// control's UserData string.
HWND hTheUpdateParentCtl = NULL;
CalendarPickerReturnType the_update_date_return_type = CalendarPickerReturnType::ISO_date;

HHOOK hCalanderPopupMouseHook = nullptr;


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
    Color back_color(COLOR_GRAYDARK);
    SolidBrush back_brush(back_color);

    // Paint the background using brush.
    int nWidth = (ps.rcPaint.right - ps.rcPaint.left);
    int nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

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
        (HMENU)(UINT_PTR)IDC_CALENDAR_CALENDAR,
        Calendar.hInst(),
        NULL);

    if (hCtl != NULL) {
        // If visual styles are active, this message has no effect except when wParam is MCSC_BACKGROUND.
        MonthCal_SetColor(hCtl, MCSC_BACKGROUND, Color(COLOR_GRAYMEDIUM).ToCOLORREF());

        SYSTEMTIME st{};
        st.wYear = (WORD)AfxGetYear(the_selected_date);
        st.wMonth = (WORD)AfxGetMonth(the_selected_date);
        st.wDay = (WORD)AfxGetDay(the_selected_date);

        MonthCal_SetCurSel(hCtl, &st);
    }

    return TRUE;
}


// ========================================================================================
// Global mouse hook.
// ========================================================================================
LRESULT CALLBACK CalanderPopupHook(int Code, WPARAM wParam, LPARAM lParam)
{
    // messages are defined in a linear way the first being WM_LBUTTONUP up to WM_MBUTTONDBLCLK
    // this subset does not include WM_MOUSEMOVE, WM_MOUSEWHEEL and a few others
    // (Don't handle WM_LBUTTONUP here because the mouse is most likely outside the menu popup
    // at the point this hook is called).
    if (wParam == WM_LBUTTONDOWN)
    {
        if (HWND_CALENDAR)
        {
            POINT pt;       GetCursorPos(&pt);
            RECT rcWindow;  GetWindowRect(HWND_CALENDAR, &rcWindow);

            // if the mouse action is outside the menu, hide it. the window procedure will also unset this hook 
            if (!PtInRect(&rcWindow, pt)) {
                DestroyWindow(HWND_CALENDAR);
            }
        }
    }

    return CallNextHookEx(NULL, Code, wParam, lParam);
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CCalendar::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(m_hwnd, WM_CREATE, Calendar_OnCreate);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, Calendar_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, Calendar_OnPaint);

    case WM_NOTIFY:
    {
        switch (((LPNMHDR)lParam)->code)
        {
        case MCN_SELECT:
            if (((LPNMHDR)lParam)->idFrom == IDC_CALENDAR_CALENDAR)
            {
                LPNMSELCHANGE lpNMSelChange = (LPNMSELCHANGE)lParam;

                the_selected_date = AfxMakeISODate(
                    lpNMSelChange->stSelStart.wYear,
                    lpNMSelChange->stSelStart.wMonth,
                    lpNMSelChange->stSelStart.wDay);

                std::wstring text;
                switch (the_update_date_return_type)
                {
                case CalendarPickerReturnType::short_date:
                    text = AfxShortDate(the_selected_date);
                    break;
                case CalendarPickerReturnType::long_date:
                    text = AfxLongDate(the_selected_date);
                    break;
                case CalendarPickerReturnType::ISO_date:
                    text = the_selected_date;
                    break;
                }

                CustomLabel_SetUserData(hTheUpdateParentCtl, the_selected_date);
                CustomLabel_SetText(hTheUpdateParentCtl, text);
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
        // unhook and remove our global mouse hook
        UnhookWindowsHookEx(hCalanderPopupMouseHook);
        hCalanderPopupMouseHook = nullptr;

        // Reset our destroyed variable for future use of the popup
        HWND_CALENDAR = NULL;
        return 0;
    }
    break;


    case WM_MOUSEACTIVATE:
    {
        return MA_NOACTIVATE;
    }
    break;


    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}


// ========================================================================================
// Create Calendar control and move it into position under the specified incoming control.
// ========================================================================================
HWND Calendar_CreateDatePicker(
    HWND hParent, HWND hParentCtl, std::wstring date_text, 
    CalendarPickerReturnType DateReturnType, int NumCalendars)
{
    if (date_text.length() == 0)
        date_text = AfxCurrentDate();

    the_selected_date = date_text;


    Calendar.Create(hParent, L"", 0, 0, 0, 0,
        WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR |
        WS_EX_NOACTIVATE);

    int calendar_width = AfxScaleX(200);
    int calendar_height = AfxScaleY(164);

    if (NumCalendars == 2) {
        calendar_height = AfxScaleY(310);
    }

    int margin = AfxScaleX(1);
    RECT rc; GetWindowRect(hParentCtl, &rc);
    SetWindowPos(Calendar.WindowHandle(), 0,
        rc.left, rc.bottom + margin,
        calendar_width, calendar_height,
        SWP_NOZORDER | SWP_SHOWWINDOW | SWP_NOACTIVATE);

    HWND hCtl = GetDlgItem(Calendar.WindowHandle(), IDC_CALENDAR_CALENDAR);
    GetClientRect(Calendar.WindowHandle(), &rc);
    SetWindowPos(hCtl, 0, 0, 0, rc.right, rc.bottom, SWP_NOZORDER | SWP_SHOWWINDOW);


    // Set the module global hUpdateParentCtl after the above Calendar is created in
    // to ensure the variable address is correct.
    hTheUpdateParentCtl = hParentCtl;
    the_update_date_return_type = DateReturnType;

    // Set our hook and store the handle in the global variable
    hCalanderPopupMouseHook = SetWindowsHookEx(WH_MOUSE, CalanderPopupHook, 0, GetCurrentThreadId());

    return Calendar.WindowHandle();
}

