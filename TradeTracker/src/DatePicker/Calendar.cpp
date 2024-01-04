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

#include "pch.h"

#include "MainWindow/MainWindow.h"
#include "Utilities/UserMessages.h"

#include "Calendar.h"


HWND HWND_CALENDAR = NULL;

CalendarReturn calendar_result{};

CCalendar Calendar;

HHOOK hCalanderPopupMouseHook = nullptr;


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: Calendar
// ========================================================================================
bool Calendar_OnEraseBkgnd(HWND hwnd, HDC hdc) {
    // Handle all of the painting in WM_PAINT
    return true;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: Calendar
// ========================================================================================
void Calendar_OnPaint(HWND hwnd) {
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    Graphics graphics(hdc);

    // Create the background brush
    Color back_color(COLOR_GRAYDARK);
    SolidBrush back_brush(back_color);

    // Paint the background using brush.
    int width = (ps.rcPaint.right - ps.rcPaint.left);
    int height = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, width, height);

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: Calendar
// ========================================================================================
bool Calendar_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct) {
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

    return true;
}


// ========================================================================================
// Global mouse hook.
// ========================================================================================
LRESULT CALLBACK CalanderPopupHook(int Code, WPARAM wParam, LPARAM lParam) {
    // messages are defined in a linear way the first being WM_LBUTTONUP up to WM_MBUTTONDBLCLK
    // this subset does not include WM_MOUSEMOVE, WM_MOUSEWHEEL and a few others
    // (Don't handle WM_LBUTTONUP here because the mouse is most likely outside the menu popup
    // at the point this hook is called).
    if (wParam == WM_LBUTTONDOWN) {
        if (HWND_CALENDAR) {
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
LRESULT CCalendar::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        HANDLE_MSG(m_hwnd, WM_CREATE, Calendar_OnCreate);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, Calendar_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, Calendar_OnPaint);

    case WM_NOTIFY: {
        switch (((LPNMHDR)lParam)->code) {
        case MCN_SELECT:
            if (((LPNMHDR)lParam)->idFrom == IDC_CALENDAR_CALENDAR) {
                LPNMSELCHANGE lpNMSelChange = (LPNMSELCHANGE)lParam;

                calendar_result.iso_date = AfxMakeISODate(
                    lpNMSelChange->stSelStart.wYear,
                    lpNMSelChange->stSelStart.wMonth,
                    lpNMSelChange->stSelStart.wDay);
                
                calendar_result.exit_code = 0;

                DestroyWindow(m_hwnd);
                return true;
            }
            break;
        }
        return 0;
    }

    case WM_DESTROY: {
        // unhook and remove our global mouse hook
        UnhookWindowsHookEx(hCalanderPopupMouseHook);
        hCalanderPopupMouseHook = nullptr;

        // Reset our destroyed variable for future use of the popup
        HWND_CALENDAR = NULL;
        PostQuitMessage(0);
        return 0;
    }

    case WM_MOUSEACTIVATE: {
        return MA_NOACTIVATE;
    }

    }
    
    return DefWindowProc(m_hwnd, msg, wParam, lParam);
}


// ========================================================================================
// Create Calendar control and move it into position under the specified incoming control.
// ========================================================================================
CalendarReturn Calendar_CreateDatePicker(
    HWND hParent, HWND hParentCtl, std::wstring initial_selected_date, int NumCalendars)
{
    if (initial_selected_date.length() == 0) initial_selected_date = AfxCurrentDate();

    calendar_result.iso_date = L"";
    calendar_result.exit_code = -1;

    HWND hWindow = Calendar.Create(hParent, L"", 0, 0, 0, 0,
        WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR |
        WS_EX_NOACTIVATE);

    HWND hCtl = GetDlgItem(hWindow, IDC_CALENDAR_CALENDAR);
    if (hCtl) {
        // If visual styles are active, this message has no effect except when wParam is MCSC_BACKGROUND.
        MonthCal_SetColor(hCtl, MCSC_BACKGROUND, Color(COLOR_GRAYMEDIUM).ToCOLORREF());

        SYSTEMTIME st{};
        st.wYear = (WORD)AfxGetYear(initial_selected_date);
        st.wMonth = (WORD)AfxGetMonth(initial_selected_date);
        st.wDay = (WORD)AfxGetDay(initial_selected_date);

        MonthCal_SetCurSel(hCtl, &st);

        int calendar_width = AfxScaleX(200);
        int calendar_height = AfxScaleY(164);

        if (NumCalendars == 2) calendar_height = AfxScaleY(310);

        int margin = AfxScaleX(1);
        RECT rc; GetWindowRect(hParentCtl, &rc);
        SetWindowPos(hWindow, 0,
            rc.left, rc.bottom + margin,
            calendar_width, calendar_height,
            SWP_NOZORDER | SWP_SHOWWINDOW | SWP_NOACTIVATE);

        // Size the Calendar control to fit the full size of the window.
        GetClientRect(hWindow, &rc);
        SetWindowPos(hCtl, 0, 0, 0, rc.right, rc.bottom, SWP_NOZORDER | SWP_SHOWWINDOW);
    }

    // Set our hook and store the handle in the global variable
    hCalanderPopupMouseHook = SetWindowsHookEx(WH_MOUSE, CalanderPopupHook, 0, GetCurrentThreadId());

    // Call modal message pump and wait for it to end.
    MSG msg{};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!IsWindow(MainWindow.hWindow)) break;

        if (msg.message == WM_KEYUP &&  msg.wParam == VK_ESCAPE) {
            calendar_result.exit_code = -1;
            DestroyWindow(hWindow);
        }

        // Determines whether a message is intended for the specified
        // dialog box and, if it is, processes the message.
        if (!IsDialogMessage(hWindow, &msg)) {
            // Translates virtual-key messages into character messages.
            TranslateMessage(&msg);
            // Dispatches a message to a window procedure.
            DispatchMessage(&msg);
        }
    }

    return calendar_result;
}

