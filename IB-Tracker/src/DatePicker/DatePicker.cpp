#include "pch.h"
#include "..\CustomLabel\CustomLabel.h"

#include "DatePicker.h"


HWND HWND_DATEPICKER = NULL;

CDatePicker DatePicker;

HWND hListBoxActive = NULL;
std::wstring wszSelectedDate;

bool SystemListBoxSmoothScrolling = FALSE;



// ========================================================================================
// Load Month ListBox data
// ========================================================================================
void DatePicker_LoadMonthListBox(HWND hListBox)
{
    int idx = 0;

    ListBox_ResetContent(hListBox);

    for (int i = 0; i < 3; ++i) {

        idx = ListBox_AddString(hListBox, L"January");
        ListBox_SetItemData(hListBox, idx, 1);
    
        idx = ListBox_AddString(hListBox, L"February");
        ListBox_SetItemData(hListBox, idx, 2);
    
        idx = ListBox_AddString(hListBox, L"March");
        ListBox_SetItemData(hListBox, idx, 3);
    
        idx = ListBox_AddString(hListBox, L"April");
        ListBox_SetItemData(hListBox, idx, 4);
    
        idx = ListBox_AddString(hListBox, L"May");
        ListBox_SetItemData(hListBox, idx, 5);
    
        idx = ListBox_AddString(hListBox, L"June");
        ListBox_SetItemData(hListBox, idx, 6);
    
        idx = ListBox_AddString(hListBox, L"July");
        ListBox_SetItemData(hListBox, idx, 7);
    
        idx = ListBox_AddString(hListBox, L"August");
        ListBox_SetItemData(hListBox, idx, 8);
    
        idx = ListBox_AddString(hListBox, L"September");
        ListBox_SetItemData(hListBox, idx, 9);
    
        idx = ListBox_AddString(hListBox, L"October");
        ListBox_SetItemData(hListBox, idx, 10);
    
        idx = ListBox_AddString(hListBox, L"November");
        ListBox_SetItemData(hListBox, idx, 11);
    
        idx = ListBox_AddString(hListBox, L"December");
        ListBox_SetItemData(hListBox, idx, 12);
    }

    int month = AfxGetMonth(wszSelectedDate);
    for (int i = 12; i < ListBox_GetCount(hListBox); ++i) {
        if (ListBox_GetItemData(hListBox, i) == month) {
            idx = i; break;
        }
    }

    ListBox_SetCurSel(hListBox, 0);
    ListBox_SetTopIndex(hListBox, idx - 4);
}


// ========================================================================================
// Load Day ListBox data
// ========================================================================================
void DatePicker_LoadDayListBox(HWND hListBox)
{
    int idx = 0;

    ListBox_ResetContent(hListBox);

    for (int ii = 0; ii < 3; ++ii) {
        for (int i = 1; i <= 31; ++i) {
            idx = ListBox_AddString(hListBox, std::to_wstring(i).c_str());
            ListBox_SetItemData(hListBox, idx, i);
        }
    }

    int day = AfxGetDay(wszSelectedDate);
    for (int i = 32; i < ListBox_GetCount(hListBox); ++i) {
        if (ListBox_GetItemData(hListBox, i) == day) {
            idx = i; break;
        }
    }

    ListBox_SetTopIndex(hListBox, idx - 4);
}


// ========================================================================================
// Load Year ListBox data
// ========================================================================================
void DatePicker_LoadYearListBox(HWND hListBox)
{
    int idx = 0;

    ListBox_ResetContent(hListBox);

    // 100 years before and after the current year
    int curYear = AfxLocalYear();
    int startYear = curYear - 100;
    int endYear = curYear + 100;

    for (int i = startYear; i <= endYear; ++i) {
        idx = ListBox_AddString(hListBox, std::to_wstring(i).c_str());
        ListBox_SetItemData(hListBox, idx, i);
    }

    int year = AfxGetYear(wszSelectedDate);
    for (int i = 0; i < ListBox_GetCount(hListBox); ++i) {
        if (ListBox_GetItemData(hListBox, i) == year) {
            idx = i; break;
        }
    }

    ListBox_SetTopIndex(hListBox, idx - 4);
}


// ========================================================================================
// Process LBN_SELCHANGE for window/dialog: DatePicker
// ========================================================================================
void DatePicker_OnSelChange(HWND hwnd)
{
    HWND hListBox = NULL;
    int cursel = -1;

    hListBox = GetDlgItem(hwnd, IDC_DATEPICKER_MONTHLISTBOX);
    cursel = ListBox_GetTopIndex(hListBox) + 4;
    int month = ListBox_GetItemData(hListBox, cursel);

    hListBox = GetDlgItem(hwnd, IDC_DATEPICKER_DAYLISTBOX);
    cursel = ListBox_GetTopIndex(hListBox) + 4;
    int day = ListBox_GetItemData(hListBox, cursel);

    hListBox = GetDlgItem(hwnd, IDC_DATEPICKER_YEARLISTBOX);
    cursel = ListBox_GetTopIndex(hListBox) + 4;
    int year = ListBox_GetItemData(hListBox, cursel);

    std::wstring wszSelectedDate = AfxMakeISODate(year, month, day);

    AfxRedrawWindow(hListBoxActive);
    
    std::wcout << wszSelectedDate << std::endl;

    //DatePicker_LoadMonthListBox(GetDlgItem(hwnd, IDC_DATEPICKER_MONTHLISTBOX));
    //DatePicker_LoadDayListBox(GetDlgItem(hwnd, IDC_DATEPICKER_DAYLISTBOX));
    //DatePicker_LoadYearListBox(GetDlgItem(hwnd, IDC_DATEPICKER_YEARLISTBOX));
}


// ========================================================================================
// Process WM_SIZE message for window/dialog: DatePicker
// ========================================================================================
void DatePicker_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    int nLeft = 0;
    int nTop = 0;
    int nWidth = 0;
    int nHeightListBox = AfxScaleY(DATEPICKER_LISTBOX_ROWHEIGHT * 9);
    int nHeightButton = AfxScaleY(DATEPICKER_LISTBOX_ROWHEIGHT);

    
    HDWP hdwp = BeginDeferWindowPos(10);

    nWidth = AfxScaleX(DATEPICKER_PANEL_MONTHWIDTH);
    hdwp = DeferWindowPos(
        hdwp, GetDlgItem(hwnd, IDC_DATEPICKER_MONTHLISTBOX), 0,
        nLeft, 0, nWidth, nHeightListBox, SWP_NOZORDER | SWP_SHOWWINDOW);

    nLeft += nWidth;
    nWidth = AfxScaleX(DATEPICKER_PANEL_DAYWIDTH);
    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_DATEPICKER_DAYLISTBOX), 0,
        nLeft, 0, nWidth, nHeightListBox, SWP_NOZORDER | SWP_SHOWWINDOW);

    nLeft += nWidth;
    nWidth = AfxScaleX(DATEPICKER_PANEL_YEARWIDTH);
    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_DATEPICKER_YEARLISTBOX), 0,
        nLeft, 0, nWidth, nHeightListBox, SWP_NOZORDER | SWP_SHOWWINDOW);


    int nShowHide = SWP_HIDEWINDOW;
    RECT rc{ 0,0,0,0 };

    if (hListBoxActive != NULL) {
        nShowHide = SWP_SHOWWINDOW;
        GetWindowRect(hListBoxActive, &rc);
        MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&rc, 2);
    }

    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_DATEPICKER_MOVEUP), HWND_TOP,
        rc.left, rc.top, rc.right-rc.left, nHeightButton, nShowHide | SWP_NOACTIVATE);

    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_DATEPICKER_MOVEDOWN), HWND_TOP,
        rc.left , rc.bottom - nHeightButton, rc.right - rc.left, nHeightButton, nShowHide | SWP_NOACTIVATE);
    

    // ACCEPT
    nTop = cy - nHeightButton;
    nWidth = AfxScaleX(DATEPICKER_PANEL_WIDTH / 2);
    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_DATEPICKER_ACCEPT),
        0, 0, nTop, nWidth, nHeightButton, SWP_NOZORDER | SWP_SHOWWINDOW);

    // CANCEL
    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_DATEPICKER_CANCEL),
        0, nWidth, nTop, nWidth, nHeightButton, SWP_NOZORDER | SWP_SHOWWINDOW);

    EndDeferWindowPos(hdwp);


    // Resize the DatePicker popup window to fit the client size
    rc.left = 0; rc.top = 0;
    rc.right = AfxScaleX(DATEPICKER_PANEL_WIDTH);
    rc.bottom = AfxScaleY(DATEPICKER_LISTBOX_ROWHEIGHT * DATEPICKER_LISTBOX_VISIBLELINES) + nHeightButton;

    DWORD dwStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
    DWORD dwExStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    AdjustWindowRectEx(&rc, dwStyle, FALSE, dwExStyle);
    SetWindowPos(hwnd, NULL, 0, 0, rc.right, rc.bottom, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
}


// ========================================================================================
// Process WM_DRAWITEM message for window/dialog 
// ========================================================================================
void DatePicker_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT* lpDrawItem)
{
    if (lpDrawItem->itemID == -1) return;

    if (lpDrawItem->itemAction == ODA_DRAWENTIRE ||
        lpDrawItem->itemAction == ODA_SELECT) {

        int nWidth = (lpDrawItem->rcItem.right - lpDrawItem->rcItem.left);
        int nHeight = (lpDrawItem->rcItem.bottom - lpDrawItem->rcItem.top);

        bool bIsHot = false;

        SaveDC(lpDrawItem->hDC);

        HDC memDC = NULL;         // Double buffering
        HBITMAP hbit = NULL;      // Double buffering

        memDC = CreateCompatibleDC(lpDrawItem->hDC);
        hbit = CreateCompatibleBitmap(lpDrawItem->hDC, nWidth, nHeight);
        if (hbit) SelectObject(memDC, hbit);

        // The "middle line" displayed is considered the selected line. This
        // would be the line 4 lines after the listbox top line because there
        // would be 4 lines before , selected line, 4 lines after. The listboxes
        // do not use SetCurSel because it has the LBS_NOSEL style set.
        if (lpDrawItem->itemID == ListBox_GetTopIndex(lpDrawItem->hwndItem) + 4) {
            bIsHot = true;
        }

        Graphics graphics(memDC);
        graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

        std::wstring wszText = AfxGetListBoxText(lpDrawItem->hwndItem, lpDrawItem->itemID);

        DWORD nBackColor = (bIsHot)
            ? GetThemeColor(ThemeElement::Selection)
            : GetThemeColor(ThemeElement::GrayDark);
        DWORD nBackColorHot = GetThemeColor(ThemeElement::Selection);
        DWORD nTextColor = GetThemeColor(ThemeElement::WhiteLight);

        std::wstring wszFontName = AfxGetDefaultFont();
        FontFamily   fontFamily(wszFontName.c_str());
        REAL fontSize = 10;
        int fontStyle = FontStyleRegular;

        StringAlignment HAlignment = StringAlignmentCenter;
        StringAlignment VAlignment = StringAlignmentCenter;

        // Paint the full width background using brush 
        SolidBrush backBrush(nBackColor);
        graphics.FillRectangle(&backBrush, 0, 0, nWidth, nHeight);

        Font         font(&fontFamily, fontSize, fontStyle, Unit::UnitPoint);
        SolidBrush   textBrush(nTextColor);
        StringFormat stringF(StringFormatFlagsNoWrap);
        stringF.SetAlignment(HAlignment);
        stringF.SetLineAlignment(VAlignment);

        RectF rcText((REAL)0, (REAL)0, (REAL)nWidth, (REAL)nHeight);
        graphics.DrawString(wszText.c_str(), -1, &font, rcText, &stringF, &textBrush);


        BitBlt(lpDrawItem->hDC, lpDrawItem->rcItem.left,
            lpDrawItem->rcItem.top, nWidth, nHeight, memDC, 0, 0, SRCCOPY);


        // Cleanup
        RestoreDC(lpDrawItem->hDC, -1);
        if (hbit) DeleteObject(hbit);
        if (memDC) DeleteDC(memDC);
    }
}


// ========================================================================================
// Listbox subclass Window procedure
// ========================================================================================
LRESULT CALLBACK DatePicker_ListBox_SubclassProc(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    // Create static accumulation variable to collect the data from
    // a series of middle mouse wheel scrolls.
    static int accumDelta = 0;

    switch (uMsg)
    {

    case WM_MOUSEWHEEL:
    {
        // Accumulate delta until scroll one line (up +120, down -120). 
        // 120 is the Microsoft default delta
        int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        int nTopIndex = SendMessage(hWnd, LB_GETTOPINDEX, 0, 0);
        accumDelta += zDelta;
        if (accumDelta >= 120) {     // scroll up 1 line
            nTopIndex -= 1;
            nTopIndex = max(0, nTopIndex);
            SendMessage(hWnd, LB_SETTOPINDEX, nTopIndex, 0);
            accumDelta = 0;
            DatePicker_OnSelChange(HWND_DATEPICKER);
        }
        else {
            if (accumDelta <= -120) {     // scroll down 1 line
                nTopIndex += 1;
                nTopIndex = min(ListBox_GetCount(hWnd) - 9, nTopIndex);
                SendMessage(hWnd, LB_SETTOPINDEX, nTopIndex, 0);
                accumDelta = 0;
                DatePicker_OnSelChange(HWND_DATEPICKER);
            }
        }
        return 0;
    }
    break;



    case WM_LBUTTONDOWN:
    {
        int idx = Listbox_ItemFromPoint(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        // The return value contains the index of the nearest item in the LOWORD. The HIWORD is zero 
        // if the specified point is in the client area of the list box, or one if it is outside the 
        // client area.
        if (HIWORD(idx) == -1) break;

        int nTopIndex = SendMessage(hWnd, LB_GETTOPINDEX, 0, 0);
        int cursel = nTopIndex + 4;
        SendMessage(hWnd, LB_SETTOPINDEX, nTopIndex + (idx - cursel), 0);
        DatePicker_OnSelChange(HWND_DATEPICKER);
    }
    break;



    case WM_MOUSEMOVE:
    {
        // Track that we are over the control in order to catch the 
        // eventual WM_MOUSEHOVER and WM_MOUSELEAVE events
        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = hWnd;
        TrackMouseEvent(&tme);

        // We are over a ListBox so ensure that the correct Up/Down buttons are shown.
        hListBoxActive = hWnd;
        RECT rc; GetClientRect(HWND_DATEPICKER, &rc);
        DatePicker_OnSize(HWND_DATEPICKER, 0, rc.right, rc.bottom);
        return 0;
    }
    break;


    case WM_MOUSELEAVE:
    {
        // We are leaving a ListBox so ensure that the correct Up/Down buttons are shown.
        // Do not call if we are over an up or down button.
        POINT pt; GetCursorPos(&pt);
        RECT rc; GetWindowRect(GetDlgItem(HWND_DATEPICKER, IDC_DATEPICKER_MOVEUP), &rc);
        if (PtInRect(&rc, pt)) return 0;
        GetWindowRect(GetDlgItem(HWND_DATEPICKER, IDC_DATEPICKER_MOVEDOWN), &rc);
        if (PtInRect(&rc, pt)) return 0;

        hListBoxActive = NULL;
        GetClientRect(HWND_DATEPICKER, &rc);
        DatePicker_OnSize(HWND_DATEPICKER, 0, rc.right, rc.bottom);
        return 0;
    }
    break;


    case WM_ERASEBKGND:
    {
        // If the number of lines in the listbox maybe less than the number per page then 
        // calculate from last item to bottom of listbox, otherwise calculate based on
        // the mod of the lineheight to listbox height so we can color the partial line
        // that won't be displayed at the bottom of the list.
        RECT rc; GetClientRect(hWnd, &rc);

        RECT rcItem{};
        SendMessage(hWnd, LB_GETITEMRECT, 0, (LPARAM)&rcItem);
        int itemHeight = (rcItem.bottom - rcItem.top);
        int NumItems = ListBox_GetCount(hWnd);
        int nTopIndex = SendMessage(hWnd, LB_GETTOPINDEX, 0, 0);
        int visible_rows = 0;
        int ItemsPerPage = 0;
        int bottom_index = 0;
        int nWidth = (rc.right - rc.left);
        int nHeight = (rc.bottom - rc.top);

        if (NumItems > 0) {
            ItemsPerPage = (nHeight) / itemHeight;
            bottom_index = (nTopIndex + ItemsPerPage);
            if (bottom_index >= NumItems)
                bottom_index = NumItems - 1;
            visible_rows = (bottom_index - nTopIndex) + 1;
            rc.top = visible_rows * itemHeight;
        }

        if (rc.top < rc.bottom) {
            nHeight = (rc.bottom - rc.top);
            HDC hDC = (HDC)wParam;
            Graphics graphics(hDC);
            SolidBrush backBrush(GetThemeColor(ThemeElement::GrayDark));
            graphics.FillRectangle(&backBrush, rc.left, rc.top, nWidth, nHeight);
        }

        ValidateRect(hWnd, &rc);
        return TRUE;
    }
    break;


    case WM_DESTROY:
        // Destroy all manually allocated ListBox display data that is held
        // in the LineData structures..
        //DatePicker_DestroyItemData(hWnd);

        // REQUIRED: Remove control subclassing
        RemoveWindowSubclass(hWnd, DatePicker_ListBox_SubclassProc, uIdSubclass);
        break;


    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);

}



// ========================================================================================
// Process WM_MEASUREITEM message for window/dialog: DatePicker
// ========================================================================================
void DatePicker_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT* lpMeasureItem)
{
    lpMeasureItem->itemHeight = AfxScaleY(DATEPICKER_LISTBOX_ROWHEIGHT);
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: DatePicker
// ========================================================================================
BOOL DatePicker_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    // Handle all of the painting in WM_PAINT
    return TRUE;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: DatePicker
// ========================================================================================
void DatePicker_OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    Graphics graphics(hdc);

    DWORD nBackColor = GetThemeColor(ThemeElement::GrayDark);

    // Create the background brush
    SolidBrush backBrush(nBackColor);

    // Paint the background using brush.
    int nWidth = (ps.rcPaint.right - ps.rcPaint.left);
    int nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&backBrush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Process WM_COMMAND message for window/dialog: DatePicker
// ========================================================================================
void DatePicker_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: DatePicker
// ========================================================================================
BOOL DatePicker_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{

    // We will save the current ListBox SmoothScrolling system setting, disable it, and
    // set it back to its original value when the DatePicker is closed. This prevents
    // the annoying first selection animation delay that screws up our selection painting.
    
    // Determines whether the smooth - scrolling effect for list boxes is enabled.
    // The pvParam parameter must point to a BOOL variable that receives TRUE for enabled, or FALSE for disabled.
    // Save the value to be restore later in WM_DESTROY
    SystemParametersInfo(SPI_GETLISTBOXSMOOTHSCROLLING, 0, &SystemListBoxSmoothScrolling, 0);
    // Turn off smooth scrolling.
    SystemParametersInfo(SPI_SETLISTBOXSMOOTHSCROLLING, 0, FALSE, 0);

    //SystemParametersInfo(SPI_SETLISTBOXSMOOTHSCROLLING, 0, FALSE,
    //    SPIF_SENDCHANGE);

    HWND_DATEPICKER = hwnd;

    HWND hCtl = NULL;

    // Create Button Up/Down and Ownerdraw listboxes that we will use to custom paint our date values.

    // MONTH
    hCtl =
        DatePicker.AddControl(Controls::ListBox, hwnd, IDC_DATEPICKER_MONTHLISTBOX, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWFIXED | LBS_NOTIFY | LBS_HASSTRINGS | LBS_NOSEL,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)DatePicker_ListBox_SubclassProc,
            IDC_DATEPICKER_MONTHLISTBOX, NULL);
    SetWindowTheme(hCtl, L" ", L" ");
    DatePicker_LoadMonthListBox(hCtl);

    // DAY
    hCtl =
        DatePicker.AddControl(Controls::ListBox, hwnd, IDC_DATEPICKER_DAYLISTBOX, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWFIXED | LBS_NOTIFY | LBS_HASSTRINGS | LBS_NOSEL,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)DatePicker_ListBox_SubclassProc,
            IDC_DATEPICKER_DAYLISTBOX, NULL);
    SetWindowTheme(hCtl, L" ", L" ");
    DatePicker_LoadDayListBox(hCtl);

    // YEAR
    hCtl =
        DatePicker.AddControl(Controls::ListBox, hwnd, IDC_DATEPICKER_YEARLISTBOX, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWFIXED | LBS_NOTIFY | LBS_HASSTRINGS | LBS_NOSEL,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)DatePicker_ListBox_SubclassProc,
            IDC_DATEPICKER_YEARLISTBOX, NULL);
    SetWindowTheme(hCtl, L" ", L" ");
    DatePicker_LoadYearListBox(hCtl);

    // UP DOWN BUTTONS
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_DATEPICKER_MOVEUP, L"\u23F6",
        ThemeElement::WhiteDark, ThemeElement::GrayDark, ThemeElement::GrayDark, ThemeElement::GrayDark,
        CustomLabelAlignment::MiddleCenter, 0, 0, 0, 0);
    CustomLabel_SetMousePointer(hCtl, CustomLabelPointer::Arrow, CustomLabelPointer::Arrow);
    ShowWindow(hCtl, SW_HIDE);

    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_DATEPICKER_MOVEDOWN, L"\u23F7",
        ThemeElement::WhiteDark, ThemeElement::GrayDark, ThemeElement::GrayDark, ThemeElement::GrayDark,
        CustomLabelAlignment::MiddleCenter, 0, 0, 0, 0);
    CustomLabel_SetMousePointer(hCtl, CustomLabelPointer::Arrow, CustomLabelPointer::Arrow);
    ShowWindow(hCtl, SW_HIDE);


    // ACCEPT
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_DATEPICKER_ACCEPT, L"\u2713",
        ThemeElement::WhiteDark, ThemeElement::GrayMedium, ThemeElement::GrayLight, ThemeElement::GrayMedium,
        CustomLabelAlignment::MiddleCenter, 0, 0, 0, 0);
    CustomLabel_SetMousePointer(hCtl, CustomLabelPointer::Arrow, CustomLabelPointer::Arrow);

    // CANCEL
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_DATEPICKER_CANCEL, L"\u2715", 
        ThemeElement::WhiteDark, ThemeElement::GrayMedium, ThemeElement::GrayLight, ThemeElement::GrayMedium,
        CustomLabelAlignment::MiddleCenter, 0, 0, 0, 0);
    CustomLabel_SetMousePointer(hCtl, CustomLabelPointer::Arrow, CustomLabelPointer::Arrow);

    return TRUE;
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CDatePicker::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Prevent a recursive calling of the WM_NCACTIVATE message as DestroyWindow deactivates the window.
    static bool destroyed = false;

    switch (msg)
    {
        HANDLE_MSG(m_hwnd, WM_CREATE, DatePicker_OnCreate);
        HANDLE_MSG(m_hwnd, WM_COMMAND, DatePicker_OnCommand);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, DatePicker_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, DatePicker_OnPaint);
        HANDLE_MSG(m_hwnd, WM_SIZE, DatePicker_OnSize);
        HANDLE_MSG(m_hwnd, WM_MEASUREITEM, DatePicker_OnMeasureItem);
        HANDLE_MSG(m_hwnd, WM_DRAWITEM, DatePicker_OnDrawItem);
   

    case WM_DESTROY:
    {
        // Reset our destroyed variable for future use of the DatePicker
        destroyed = false;

        // Reset the ListBox smooth scrolling to whatever the original system value was.
        SystemParametersInfo(SPI_SETLISTBOXSMOOTHSCROLLING, 0, &SystemListBoxSmoothScrolling, 0);
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


    case MSG_CUSTOMLABEL_MOUSELEAVE:
    {
        if (wParam == IDC_DATEPICKER_MOVEUP || wParam == IDC_DATEPICKER_MOVEDOWN) {
            hListBoxActive = NULL;
            RECT rc; GetClientRect(HWND_DATEPICKER, &rc);
            DatePicker_OnSize(HWND_DATEPICKER, 0, rc.right, rc.bottom);
        }
        return 0;
    }
    break;


    case MSG_CUSTOMLABEL_CLICK:
    {
        if (wParam == IDC_DATEPICKER_MOVEUP) {
            int nTopIndex = ListBox_GetTopIndex(hListBoxActive) - 1;
            nTopIndex = max(0, nTopIndex);
            SendMessage(hListBoxActive, LB_SETTOPINDEX, nTopIndex, 0);
            DatePicker_OnSelChange(m_hwnd);
        }
        if (wParam == IDC_DATEPICKER_MOVEDOWN) {
            int nTopIndex = ListBox_GetTopIndex(hListBoxActive) + 1;
            nTopIndex = min(ListBox_GetCount(hListBoxActive) - 9, nTopIndex);
            SendMessage(hListBoxActive, LB_SETTOPINDEX, nTopIndex, 0);
            DatePicker_OnSelChange(m_hwnd);
        }
        return 0;
    }
    break;


    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}


// ========================================================================================
// Create DatePicker contro and move it into position under the specified incoming control.
// ========================================================================================
HWND DatePicker_CreateDatePicker(HWND hParent, HWND hParentCtl, std::wstring wszDate)
{
    if (wszDate.length() == 0)
        wszDate = AfxCurrentDate();

    wszSelectedDate = wszDate;

    DatePicker.Create(hParent, L"", 0, 0, 0, 0,
        WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);


    RECT rc; GetWindowRect(hParentCtl, &rc);
    SetWindowPos(DatePicker.WindowHandle(), HWND_TOP,
        rc.left, rc.bottom, 
        AfxScaleX(DATEPICKER_PANEL_WIDTH), 
        AfxScaleY(DATEPICKER_LISTBOX_ROWHEIGHT * (DATEPICKER_LISTBOX_VISIBLELINES + 1)),
        SWP_NOSIZE | SWP_SHOWWINDOW);

    return DatePicker.WindowHandle();
}

