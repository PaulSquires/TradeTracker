#include "pch.h"
#include "..\CustomLabel\CustomLabel.h"

#include "DatePicker.h"


HWND HWND_DATEPICKER = NULL;

CDatePicker DatePicker;

HWND hListBoxActive = NULL;


// ========================================================================================
// Load Month ListBox data
// ========================================================================================
void DatePicker_LoadMonthListBox(HWND hListBox)
{
    int idx = 0;

    ListBox_ResetContent(hListBox);
    idx = ListBox_AddString(hListBox, L"January");
    ListBox_SetItemData(hListBox, idx, 0);
    
    idx = ListBox_AddString(hListBox, L"February");
    ListBox_SetItemData(hListBox, idx, 1);
    
    idx = ListBox_AddString(hListBox, L"March");
    ListBox_SetItemData(hListBox, idx, 2);
    
    idx = ListBox_AddString(hListBox, L"April");
    ListBox_SetItemData(hListBox, idx, 3);
    
    idx = ListBox_AddString(hListBox, L"May");
    ListBox_SetItemData(hListBox, idx, 4);
    
    idx = ListBox_AddString(hListBox, L"June");
    ListBox_SetItemData(hListBox, idx, 5);
    
    idx = ListBox_AddString(hListBox, L"July");
    ListBox_SetItemData(hListBox, idx, 6);
    
    idx = ListBox_AddString(hListBox, L"August");
    ListBox_SetItemData(hListBox, idx, 7);
    
    idx = ListBox_AddString(hListBox, L"September");
    ListBox_SetItemData(hListBox, idx, 8);
    
    idx = ListBox_AddString(hListBox, L"October");
    ListBox_SetItemData(hListBox, idx, 9);
    
    idx = ListBox_AddString(hListBox, L"November");
    ListBox_SetItemData(hListBox, idx, 10);
    
    idx = ListBox_AddString(hListBox, L"December");
    ListBox_SetItemData(hListBox, idx, 11);
}


// ========================================================================================
// Load Day ListBox data
// ========================================================================================
void DatePicker_LoadDayListBox(HWND hListBox)
{
    int idx = 0;

    ListBox_ResetContent(hListBox);

    for (int i = 1; i <= 31; ++i) {
        idx = ListBox_AddString(hListBox, std::to_wstring(i).c_str());
        ListBox_SetItemData(hListBox, idx, 0);
    }
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
        ListBox_SetItemData(hListBox, idx, 0);
    }
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

        if ((lpDrawItem->itemAction | ODA_SELECT) &&
            (lpDrawItem->itemState & ODS_SELECTED)) {
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
        if (accumDelta >= 120) {     // scroll up 3 lines
            nTopIndex -= 3;
            nTopIndex = max(0, nTopIndex);
            SendMessage(hWnd, LB_SETTOPINDEX, nTopIndex, 0);
            accumDelta = 0;
        }
        else {
            if (accumDelta <= -120) {     // scroll down 3 lines
                nTopIndex += +3;
                SendMessage(hWnd, LB_SETTOPINDEX, nTopIndex, 0);
                accumDelta = 0;
            }
        }
    }
    break;


    case WM_MOUSEMOVE:
    {
        // Track that we are over the control in order to catch the 
        // eventual WM_MOUSEHOVER and WM_MOUSELEAVE events
        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        tme.dwFlags = TME_HOVER or TME_LEAVE;
        tme.hwndTrack = hWnd;
        TrackMouseEvent(&tme);

        // We are over a ListBox so ensure that the correct Up/Down buttons are shown.
        hListBoxActive = hWnd;
        RECT rc; GetClientRect(HWND_DATEPICKER, &rc);
        DatePicker_OnSize(HWND_DATEPICKER, 0, rc.right, rc.bottom);
    }
    break;


    case WM_MOUSELEAVE:
    {
        // We are leaving a ListBox so ensure that the correct Up/Down buttons are shown.
        hListBoxActive = NULL;
        RECT rc; GetClientRect(HWND_DATEPICKER, &rc);
        DatePicker_OnSize(HWND_DATEPICKER, 0, rc.right, rc.bottom);
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
// Process WM_CREATE message for window/dialog: DatePicker
// ========================================================================================
BOOL DatePicker_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    HWND_DATEPICKER = hwnd;

    HWND hCtl = NULL;

    // Create Button Up/Down and Ownerdraw listboxes that we will use to custom paint our date values.

    // MONTH
    hCtl =
        DatePicker.AddControl(Controls::ListBox, hwnd, IDC_DATEPICKER_MONTHLISTBOX, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWFIXED | LBS_NOTIFY | LBS_HASSTRINGS,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)DatePicker_ListBox_SubclassProc,
            IDC_DATEPICKER_MONTHLISTBOX, NULL);
    DatePicker_LoadMonthListBox(hCtl);

    // DAY
    hCtl =
        DatePicker.AddControl(Controls::ListBox, hwnd, IDC_DATEPICKER_DAYLISTBOX, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWFIXED | LBS_NOTIFY | LBS_HASSTRINGS,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)DatePicker_ListBox_SubclassProc,
            IDC_DATEPICKER_DAYLISTBOX, NULL);
    DatePicker_LoadDayListBox(hCtl);

    // YEAR
    hCtl =
        DatePicker.AddControl(Controls::ListBox, hwnd, IDC_DATEPICKER_YEARLISTBOX, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWFIXED | LBS_NOTIFY | LBS_HASSTRINGS,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)DatePicker_ListBox_SubclassProc,
            IDC_DATEPICKER_YEARLISTBOX, NULL);
    DatePicker_LoadYearListBox(hCtl);

    // UP DOWN BUTTONS
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_DATEPICKER_MOVEUP, L"\uE015",
        ThemeElement::WhiteDark, ThemeElement::GrayMedium, ThemeElement::GrayLight, ThemeElement::GrayDark,
        CustomLabelAlignment::MiddleCenter, 0, 0, 0, 0);
    AfxAddWindowExStyle(hCtl, WS_EX_TRANSPARENT);
    ShowWindow(hCtl, SW_HIDE);

    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_DATEPICKER_MOVEDOWN, L"\uE015",
        ThemeElement::WhiteDark, ThemeElement::GrayMedium, ThemeElement::GrayLight, ThemeElement::GrayDark,
        CustomLabelAlignment::MiddleCenter, 0, 0, 0, 0);
    AfxAddWindowExStyle(hCtl, WS_EX_TRANSPARENT);
    ShowWindow(hCtl, SW_HIDE);


    // ACCEPT
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_DATEPICKER_ACCEPT, L"\u2713",
        ThemeElement::WhiteDark, ThemeElement::GrayMedium, ThemeElement::GrayLight, ThemeElement::GrayDark,
        CustomLabelAlignment::MiddleCenter, 0, 0, 0, 0);

    // CANCEL
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_DATEPICKER_CANCEL, L"\u2715",
        ThemeElement::WhiteDark, ThemeElement::GrayMedium, ThemeElement::GrayLight, ThemeElement::GrayDark,
        CustomLabelAlignment::MiddleCenter, 0, 0, 0, 0);

    return TRUE;
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CDatePicker::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(m_hwnd, WM_CREATE, DatePicker_OnCreate);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, DatePicker_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, DatePicker_OnPaint);
        HANDLE_MSG(m_hwnd, WM_SIZE, DatePicker_OnSize);
        HANDLE_MSG(m_hwnd, WM_MEASUREITEM, DatePicker_OnMeasureItem);
        HANDLE_MSG(m_hwnd, WM_DRAWITEM, DatePicker_OnDrawItem);

    case WM_KILLFOCUS:
    {
        DestroyWindow(m_hwnd);
        return 0;
    }
    break;

    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}


// ========================================================================================
// Create DatePicker contro and move it into position under the specified incoming control.
// ========================================================================================
HWND DatePicker_CreateDatePicker(HWND hParent, HWND hParentCtl)
{
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

