#include "pch.h"
#include "..\CustomLabel\CustomLabel.h"

#include "DatePicker.h"


HWND HWND_DATEPICKER = NULL;

CDatePicker DatePicker;

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


        // Set some defaults in case there is no valid ListBox line number
        std::wstring wszText;

        DWORD nBackColor = (bIsHot)
            ? GetThemeColor(ThemeElement::Selection)
            : GetThemeColor(ThemeElement::GrayDark);
        DWORD nBackColorHot = GetThemeColor(ThemeElement::Selection);
        DWORD nTextColor = GetThemeColor(ThemeElement::WhiteLight);

        std::wstring wszFontName = AfxGetDefaultFont();
        FontFamily   fontFamily(wszFontName.c_str());
        REAL fontSize = 10;
        int fontStyle = FontStyleRegular;

        StringAlignment HAlignment = StringAlignmentNear;
        StringAlignment VAlignment = StringAlignmentCenter;

        // Paint the full width background using brush 
        SolidBrush backBrush(nBackColor);
        graphics.FillRectangle(&backBrush, 0, 0, nWidth, nHeight);

        // Get the current ListBox line data should a valid line exist
        // Paint the individual columns with the specific data.

/*
        ListBoxData* ld = (ListBoxData*)(lpDrawItem->itemData);
        int nLeft = 0;
        int colWidth = 0;

        // Draw each of the columns
        for (int i = 0; i < 8; i++) {
            if (ld == nullptr) break;
            if (ld->col[i].colWidth == 0) break;

            // Prepare and draw the text
            wszText = ld->col[i].wszText;

            HAlignment = ld->col[i].HAlignment;
            VAlignment = ld->col[i].VAlignment;
            nBackColor = (bIsHot)
                ? nBackColor
                : GetThemeColor(ld->col[i].backTheme);
            nTextColor = GetThemeColor(ld->col[i].textTheme);
            fontSize = ld->col[i].fontSize;
            fontStyle = ld->col[i].fontStyle;

            colWidth = AfxScaleX((float)ld->col[i].colWidth);

            backBrush.SetColor(nBackColor);
            graphics.FillRectangle(&backBrush, nLeft, 0, colWidth, nHeight);

            Font         font(&fontFamily, fontSize, fontStyle, Unit::UnitPoint);
            SolidBrush   textBrush(nTextColor);
            StringFormat stringF(StringFormatFlagsNoWrap);
            stringF.SetAlignment(HAlignment);
            stringF.SetLineAlignment(VAlignment);

            // If right alignment then add a very small amount of right side
            // padding so that text is not pushed up right against the right side.
            int rightPadding = 0;
            if (HAlignment == StringAlignmentFar) rightPadding = AfxScaleX(2);

            RectF rcText((REAL)nLeft, (REAL)0, (REAL)colWidth - rightPadding, (REAL)nHeight);
            graphics.DrawString(wszText.c_str(), -1, &font, rcText, &stringF, &textBrush);

            nLeft += colWidth;
        }
*/

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
        break;
    }


    case WM_DESTROY:
        // Destroy all manually allocated ListBox display data that is held
        // in the LineData structures..
        //ListBoxData_DestroyItemData(hWnd);

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
// Process WM_SIZE message for window/dialog: DatePicker
// ========================================================================================
void DatePicker_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    HWND hMonthListBox = GetDlgItem(hwnd, IDC_DATEPICKER_MONTHLISTBOX);
    HWND hDayListBox = GetDlgItem(hwnd, IDC_DATEPICKER_DAYLISTBOX);
    HWND hYearListBox = GetDlgItem(hwnd, IDC_DATEPICKER_YEARLISTBOX);

    HDWP hdwp = BeginDeferWindowPos(10);

    // Move and size the top label into place
//    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_HISTORY_SYMBOL), 0,
//        0, 0, cx, margin, SWP_NOZORDER | SWP_SHOWWINDOW);

    int nTop = 0;
    int nLeft = 0;
    int nWidth = AfxScaleX(DATEPICKER_PANEL_MONTHWIDTH);
    int nHeight = cy;
    hdwp = DeferWindowPos(hdwp, hMonthListBox, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nTop = 0;
    nLeft = nLeft + nWidth;
    nWidth = AfxScaleX(DATEPICKER_PANEL_DAYWIDTH);
    nHeight = cy;
    hdwp = DeferWindowPos(hdwp, hDayListBox, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

    nTop = 0;
    nLeft = nLeft + nWidth;
    nWidth = AfxScaleX(DATEPICKER_PANEL_YEARWIDTH);
    nHeight = cy;
    hdwp = DeferWindowPos(hdwp, hYearListBox, 0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);


    EndDeferWindowPos(hdwp);
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: DatePicker
// ========================================================================================
BOOL DatePicker_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    HWND_DATEPICKER = hwnd;

    HWND hCtl = NULL;

  //  HWND hCtl = CustomLabel_SimpleLabel(hwnd, IDC_HISTORY_SYMBOL, L"Trade History",
   //     ThemeElement::WhiteLight, ThemeElement::Black);

    // Create an Ownerdraw listboxes that we will use to custom paint our date values.
    hCtl =
        DatePicker.AddControl(Controls::ListBox, hwnd, IDC_DATEPICKER_MONTHLISTBOX, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWFIXED | LBS_NOTIFY,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)DatePicker_ListBox_SubclassProc,
            IDC_DATEPICKER_MONTHLISTBOX, NULL);
    ListBox_AddString(hCtl, NULL);

    hCtl =
        DatePicker.AddControl(Controls::ListBox, hwnd, IDC_DATEPICKER_DAYLISTBOX, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWFIXED | LBS_NOTIFY,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)DatePicker_ListBox_SubclassProc,
            IDC_DATEPICKER_MONTHLISTBOX, NULL);
    ListBox_AddString(hCtl, NULL);

    hCtl =
        DatePicker.AddControl(Controls::ListBox, hwnd, IDC_DATEPICKER_YEARLISTBOX, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWFIXED | LBS_NOTIFY,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)DatePicker_ListBox_SubclassProc,
            IDC_DATEPICKER_MONTHLISTBOX, NULL);
    ListBox_AddString(hCtl, NULL);

    return TRUE;
}


// ========================================================================================
// Position the DatePicker immediately under the specified incoming control.
// This is called by the TradeDialog when the DatePicker is created but not yet shown.
// This function will also show the DatePicker once it has been positioned correctly.
// ========================================================================================
void DatePicker_SetPosition(HWND hParentCtl)
{
    RECT rc; GetWindowRect(hParentCtl, &rc);
    SetWindowPos(DatePicker.WindowHandle(), HWND_TOP,
        rc.left, rc.bottom,
        AfxScaleX(DATEPICKER_PANEL_WIDTH), AfxScaleY(DATEPICKER_PANEL_HEIGHT),
        SWP_SHOWWINDOW);
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

