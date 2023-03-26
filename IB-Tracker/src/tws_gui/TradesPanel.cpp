
#include "pch.h"
#include "ib-tracker.h"
#include "TradesPanel.h"
#include "Themes.h"

const int LISTBOX_ROWHEIGHT = 20;



//' ========================================================================================
//' Process WM_DRAWITEM message 
//' ========================================================================================
int OnDrawItem(HWND hWnd, DRAWITEMSTRUCT* lpdis)
{
    if (lpdis == nullptr) return 0;

    if (lpdis->itemAction == ODA_DRAWENTIRE ||
        lpdis->itemAction == ODA_SELECT ||
        lpdis->itemAction == ODA_FOCUS) {

        int nWidth = (lpdis->rcItem.right - lpdis->rcItem.left);
        int nHeight = (lpdis->rcItem.bottom - lpdis->rcItem.top);

        bool bIsHot = false;

        SaveDC(lpdis->hDC);

        HDC memDC = NULL;         // Double buffering
        HBITMAP hbit = NULL;      // Double buffering

        memDC = CreateCompatibleDC(lpdis->hDC);
        hbit = CreateCompatibleBitmap(lpdis->hDC, nWidth, nHeight);
        if (hbit) SelectObject(memDC, hbit);

        if (ListBox_GetSel(lpdis->hwndItem, lpdis->itemID)) bIsHot = true;


        Graphics graphics(memDC);
        graphics.SetTextRenderingHint(TextRenderingHintAntiAlias);


        DWORD nBackColor = (bIsHot 
            ? GetThemeColor(ThemeElement::TradesPanelBackHot) 
            : GetThemeColor(ThemeElement::TradesPanelBack));

        DWORD nTextColor = (bIsHot
            ? GetThemeColor(ThemeElement::TradesPanelTextHot)
            : GetThemeColor(ThemeElement::TradesPanelText));

        // Create the background brush
        SolidBrush backBrush(nBackColor);

        // Paint the background using brush and default pen. Use RoundRect because 
        // we may want to have rounded corners.
        graphics.FillRectangle(&backBrush, 0, 0, nWidth, nHeight);


        std::wstring wszFontName = L"Tahoma";
        FontFamily fontFamily(wszFontName.c_str());

        REAL fontSize = 9;
        int fontStyle = FontStyleRegular;

            //if (FontBoldHot) fontStyle |= FontStyleBold;
            //if (FontItalicHot) fontStyle |= FontStyleItalic;
            //if (FontUnderlineHot) fontStyle |= FontStyleUnderline;

        Font         font(&fontFamily, fontSize, fontStyle, Unit::UnitPoint);
        SolidBrush   textBrush(nTextColor);

		// MiddleLeft
        StringFormat stringF(StringFormatFlagsNoWrap);
        stringF.SetAlignment(StringAlignmentNear);
        stringF.SetLineAlignment(StringAlignmentCenter);
            
        //REAL nLeft = (MarginLeft + TextOffsetLeft) * m_rx;
        //REAL nTop = (MarginTop + TextOffsetTop) * m_ry;
        //REAL nRight = m_rcClient.right - (MarginRight * m_rx);
        //REAL nBottom = m_rcClient.bottom - (MarginBottom * m_ry);

        //RectF rcText(nLeft, nTop, nRight - nLeft, nBottom - nTop);
        RectF rcText((REAL)0, (REAL)0, (REAL)nWidth, (REAL)nHeight);

        std::wstring wszText = AfxGetListBoxText(lpdis->hwndItem, lpdis->itemID);
        graphics.DrawString(wszText.c_str(), -1, &font, rcText, &stringF, &textBrush);

        BitBlt(lpdis->hDC, lpdis->rcItem.left, lpdis->rcItem.top, nWidth, nHeight, memDC, 0, 0, SRCCOPY);
        

        // Cleanup
        RestoreDC(lpdis->hDC, -1);
        if (hbit) DeleteObject(hbit);
        if (memDC) DeleteDC(memDC);

    }

    return 0;
}





//' ========================================================================================
//' TradesPanel Listbox subclass Window procedure
//' ========================================================================================
LRESULT CALLBACK TradesPanelListBox_SubclassProc(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{

//dim pWindow as CWindow ptr = AfxCWindowPtr(HWND_FRMEXPLORER)

    // Keep track of last index we were over so that we only issue a 
    // repaint if the cursor has moved off of the line.
    static int nLastIdx = -1;

    // Create static accumulation variable to collect the data from
    // a series of middle mouse wheel scrolls.
    static int accumDelta = 0;

    switch (uMsg)
    {

    case WM_MOUSEWHEEL:
//' accumulate delta until scroll one line (up +120, down -120). 
//' 120 is the Microsoft default delta
//dim as long zDelta = GET_WHEEL_DELTA_WPARAM(_wParam)
//dim as long nTopIndex = SendMessage(hWin, LB_GETTOPINDEX, 0, 0)
//accumDelta = accumDelta + zDelta
//if accumDelta >= 120 then       ' scroll up 3 lines
//nTopIndex = nTopIndex - 3
//nTopIndex = max(0, nTopIndex)
//SendMessage(hWin, LB_SETTOPINDEX, nTopIndex, 0)
//accumDelta = 0
//frmPanelVScroll_PositionWindows(SW_SHOWNA)
//elseif accumDelta <= -120 then  ' scroll down 3 lines
//nTopIndex = nTopIndex + 3
//SendMessage(hWin, LB_SETTOPINDEX, nTopIndex, 0)
//accumDelta = 0
//frmPanelVScroll_PositionWindows(SW_SHOWNA)
//end if
        break;


    case WM_MOUSEMOVE:
//' Track that we are over the control in order to catch the 
//' eventual WM_MOUSEHOVER and WM_MOUSELEAVE events
//dim tme as TrackMouseEvent
//tme.cbSize = sizeof(TrackMouseEvent)
//tme.dwFlags = TME_HOVER or TME_LEAVE
//tme.hwndTrack = hWin
//TrackMouseEvent(@tme)
//
//' get the item rect that the mouse is over and only invalidate
//' that instead of the entire listbox
//dim as RECT rc
//dim as long idx = Listbox_ItemFromPoint(hWin, GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam))
//' The return value contains the index of the nearest item in the LOWORD. The HIWORD is zero 
//' if the specified point is in the client area of the list box, or one if it is outside the 
//' client area.
//if hiword(idx) < > 1 then
//if idx <> nLastIdx then
//ListBox_GetItemRect(hWin, idx, @rc)
//InvalidateRect(hWin, @rc, true)
//ListBox_GetItemRect(hWin, nLastIdx, @rc)
//InvalidateRect(hWin, @rc, true)
//nLastIdx = idx
//end if
//end if
        break;


    case WM_MOUSEHOVER:
//dim as CWSTR wszTooltip
//if IsWindow(hTooltip) = 0 then hTooltip = AfxAddTooltip(hWin, "", false, false)
//dim as long idx = Listbox_ItemFromPoint(hWin, GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam))
//' The return value contains the index of the nearest item in the LOWORD. The HIWORD is zero 
//' if the specified point is in the client area of the list box, or one if it is outside the 
//' client area.
//if hiword(idx) < > 1 then
//dim as clsDocument ptr pDoc = cast(clsDocument ptr, ListBox_GetItemData(hWin, idx))
//if pDoc then wszTooltip = pDoc->DiskFilename
//' Display the tooltip
//AfxSetTooltipText(hTooltip, hWin, wszTooltip)
//AfxRedrawWindow(hWin)
//end if
        break;


    case WM_MOUSELEAVE:
//nLastIdx = -1
//AfxRedrawWindow(hWin)
        break;


    case WM_RBUTTONDOWN:
//' Create the popup menu
//dim as long idx = Listbox_ItemFromPoint(hWin, GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam))
//' The return value contains the index of the nearest item in the LOWORD. The HIWORD is zero 
//' if the specified point is in the client area of the list box, or one if it is outside the 
//' client area.
//if hiword(idx) < > 1 then
//dim as clsDocument ptr pDoc = cast(clsDocument ptr, ListBox_GetItemData(hWin, idx))
//if pDoc then
//ListBox_SetSel(hWin, true, idx)
//dim as HMENU hPopupMenu = CreateExplorerContextMenu(pDoc)
//dim as POINT pt : GetCursorPos(@pt)
//dim as long id = TrackPopupMenu(hPopUpMenu, TPM_RETURNCMD, pt.x, pt.y, 0, HWND_FRMMAIN, byval null)
//
        break;


    case WM_LBUTTONUP:
//' Prevent this programmatic selection if Ctrl or Shift is active
//' because we want the listbox to select the listbox item rather for
//' us to mess with that selection via SetTabIndexByDocumentPtr().
//dim as boolean isCtrl = (GetAsyncKeyState(VK_CONTROL) and &H8000)
//dim as boolean isShift = (GetAsyncKeyState(VK_SHIFT) and &H8000)
//if (isCtrl = false) andalso(isShift = false) then
//' determine if we clicked on a regular file or a node header
//dim as RECT rc
//dim as long idx = Listbox_ItemFromPoint(hWin, GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam))
//' The return value contains the index of the nearest item in the LOWORD. The HIWORD is zero 
//' if the specified point is in the client area of the list box, or one if it is outside the 
//' client area.
//if hiword(idx) < > 1 then
//dim as CWSTR wszCaption = AfxGetListBoxText(hWin, idx)
//if left(wszCaption, 1) = "%" then
//' Toggle the show/hide of files under this node
//dim as long idxArray = getExplorerNodeHeaderIndex(wszCaption)
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

        if (NumItems > 0) {
            ItemsPerPage = (rc.bottom - rc.top) / itemHeight;
            bottom_index = (nTopIndex + ItemsPerPage);
            if (bottom_index >= NumItems)
                bottom_index = NumItems - 1;
            visible_rows = (bottom_index - nTopIndex) + 1;
            rc.top = visible_rows * itemHeight;
        }

        if (rc.top < rc.bottom) {
            HDC hDC = (HDC)wParam;
            HBRUSH hBrush = CreateSolidBrush(GetThemeCOLORREF(ThemeElement::TradesPanelBack));
            FillRect(hDC, &rc, hBrush);
            DeleteBrush(hBrush);
        }

        ValidateRect(hWnd, &rc);
        return TRUE;
        break;

    }


    case WM_DESTROY:
        // REQUIRED: Remove control subclassing
        RemoveWindowSubclass(hWnd, &TradesPanelListBox_SubclassProc, uIdSubclass);
        break;


        }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);

}


//' ========================================================================================
//' TradesPanel Window procedure
//' ========================================================================================
LRESULT CALLBACK TradesPanel_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {

    case WM_COMMAND:
    {
        HWND hCtl = GET_WM_COMMAND_HWND(wParam, lParam);
        int cmd = GET_WM_COMMAND_CMD(wParam, lParam);
        int id = GET_WM_COMMAND_ID(wParam, lParam);

        if (id == IDC_LISTBOX && cmd == LBN_SELCHANGE) {
            // update the highlighting of the current line
            AfxRedrawWindow(hCtl);
            //    ' update the scrollbar position if necessary
            //    frmExplorer_PositionWindows()
        }

    }
    break;


    case WM_MEASUREITEM:
    {
        // Set the height of the list box items. 
        MEASUREITEMSTRUCT* lpmis = (MEASUREITEMSTRUCT*)lParam;
        lpmis->itemHeight = (UINT)AfxScaleY(LISTBOX_ROWHEIGHT);
    }
    break;


    case WM_DRAWITEM:
        OnDrawItem(hWnd, (DRAWITEMSTRUCT*)lParam);


    case WM_SIZE:
    {
        // Fill the entire client area with our listbox.
        // Get the entire client area
        RECT rcClient;
        GetClientRect(hWnd, &rcClient);

        HWND hListBox = GetDlgItem(hWnd, IDC_LISTBOX);
        SetWindowPos(hListBox, 0, 
            rcClient.left, rcClient.top,
            rcClient.right - rcClient.left, 
            rcClient.bottom - rcClient.top, 
            SWP_NOZORDER | SWP_SHOWWINDOW);
    }
    break;


    case WM_ERASEBKGND:
    {
        // Handle all of the painting in WM_PAINT
        return TRUE;
        break;
    }


    case WM_PAINT:
    {
        PAINTSTRUCT ps;

        HDC hdc = BeginPaint(hWnd, &ps);

        //Graphics graphics(hdc);

        //// The ListBox covers the entire client area so currently there is no need
        //// to paint the background. This may change in the future if we add more controls.
        //
        //DWORD nBackColor = GetThemeColor(ThemeElement::TradesPanelBack);

        //// Create the background brush
        //SolidBrush backBrush(nBackColor);

        //// Paint the background using brush.
        //graphics.FillRectangle(&backBrush, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom);

        EndPaint(hWnd, &ps);
        break;
    }



    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;

}


//' ========================================================================================
//' TradesPanel_Show
//' ========================================================================================
CWindow* TradesPanel_Show(HWND hWndParent)
{
    // Create the window and child controls
    CWindow* pWindow = new CWindow;

    HWND HWND_FRMTRADESPANEL =
        pWindow->Create(hWndParent, L"", &TradesPanel_WndProc, 0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
            WS_EX_CONTROLPARENT | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);

    // This is a child window of the main application parent so treat it like child
    // control and assign it a ControlID.
    SetWindowLongPtr(HWND_FRMTRADESPANEL, GWLP_ID, IDC_FRMTRADESPANEL);

    // Can only set the brush after the window is created
    pWindow->SetBrush(GetStockBrush(NULL_BRUSH));

    // Create an Ownerdraw fixed row sized listbox that we will use to custom
    // paint our various open trades.
    HWND hListBox = 
        pWindow->AddControl(Controls::ListBox, HWND_FRMTRADESPANEL, IDC_LISTBOX, L"", 
            0, 0, 0, 0, 
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP | 
            LBS_NOINTEGRALHEIGHT | LBS_EXTENDEDSEL | LBS_MULTIPLESEL |
            LBS_OWNERDRAWFIXED | LBS_HASSTRINGS | LBS_NOTIFY, 
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL, 
            (SUBCLASSPROC)&TradesPanelListBox_SubclassProc, 
            IDC_LISTBOX, (DWORD_PTR)pWindow);

    for (int i = 0; i < 20; i++) {
        std::wstring wszTemp = L"My test string " + std::to_wstring(i);
        ListBox_AddString(hListBox, wszTemp.c_str());
    }

    return pWindow;

}


