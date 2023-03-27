
#include "pch.h"
#include "ib-tracker.h"
#include "TradesPanel.h"
#include "Themes.h"
#include "trade.h"


HWND HWND_TRADESPANEL = NULL;

const int LISTBOX_ROWHEIGHT = 20;

std::vector<LineData*> vec;


//' ========================================================================================
// Destroy all manually allocated ListBox display data that is held in the vector.
//' ========================================================================================
void DestroyListBoxDisplayData()
{
    for (const auto& ld : vec) {
        delete(ld);
    }
    vec.clear();
}


//' ========================================================================================
//' Helper function to more easily set the data for an individual ListBox display column.
//' ========================================================================================
void SetColumnData(LineData* ld, int index, std::wstring wszText, StringAlignment alignment,
    ThemeElement backTheme, ThemeElement textTheme, REAL fontSize, int fontStyle)
{
    ld->col[index].wszText = wszText;
    ld->col[index].alignment = alignment;
    ld->col[index].backTheme = backTheme;
    ld->col[index].textTheme = textTheme;
    ld->col[index].fontSize = fontSize;
    ld->col[index].fontStyle = fontStyle;
}



//' ========================================================================================
//' Create and populate the display data for the Trades ListBox
//' ========================================================================================
void CreateListBoxData(Trade* trade)
{
    LineData* ld = new LineData;

    ld->trade = trade;
    
    
    // *** TRADE HEADER LINE ***
    SetColumnData(ld, 0, trade->tickerSymbol, StringAlignmentNear, ThemeElement::TradesPanelBack,
        ThemeElement::TradesPanelText, 10, FontStyleRegular | FontStyleBold);
    // Col 1 to 6 are set based on incoming TWS price data 
    SetColumnData(ld, 1, L"", StringAlignmentNear, ThemeElement::TradesPanelBack,
        ThemeElement::TradesPanelText, 8, FontStyleRegular);   // ITM
    SetColumnData(ld, 2, L"", StringAlignmentNear, ThemeElement::TradesPanelBack,
        ThemeElement::TradesPanelText, 8, FontStyleRegular);
    SetColumnData(ld, 3, L"", StringAlignmentNear, ThemeElement::TradesPanelBack,
        ThemeElement::TradesPanelText, 8, FontStyleRegular);
    SetColumnData(ld, 4, L"", StringAlignmentFar, ThemeElement::TradesPanelBack,
        ThemeElement::TradesPanelTextDim, 8, FontStyleRegular);   // price change
    SetColumnData(ld, 5, L"0.00", StringAlignmentCenter, ThemeElement::TradesPanelBack,
        ThemeElement::TradesPanelText, 10, FontStyleRegular | FontStyleBold);   // current price
    SetColumnData(ld, 6, L"", StringAlignmentNear, ThemeElement::TradesPanelBack,
        ThemeElement::TradesPanelTextDim, 8, FontStyleRegular);   // price percentage change
    vec.push_back(ld);


    // *** SHARES ***
    // Roll up all of the SHARES or FUTURES transactions and display the aggregate rather than the individual legs.
    std::wstring textShares;
    int aggregate = 0;
    for (const auto& leg : trade->openLegs) {
        if (leg->underlying == L"SHARES") {
            textShares = L"SHARES";
            aggregate = aggregate + leg->openQuantity;
        }
        else if (leg->underlying == L"FUTURES") {
            textShares = L"FUTURES";
            aggregate = aggregate + leg->openQuantity;
        }
    }

//    if (aggregate) {

//        int col = 1;
//        text = QChar(0x23FA);     // filled circle
//        SetItemAttributes(legItem, 0, text, Qt::AlignCenter, font, Qt::magenta, baseGrayBack);
//        col++;

    //    text = textShares;
    //    SetItemAttributes(legItem, col, text, Qt::AlignLeft | Qt::AlignVCenter, font, gray, darkGrayBack);

    //    text = QString::number(std::abs(trade->ACB / aggregate), 'f', 2);
    //    SetItemAttributes(legItem, 5, text, Qt::AlignCenter, font, gray, darkGrayBack);

    //    text = QString::number(aggregate);
    //    SetItemAttributes(legItem, 6, text, Qt::AlignRight | Qt::AlignVCenter, font, white, darkGrayBack);

    //}


    // *** OPTION LEGS ***
    for (const auto& leg : trade->openLegs) {
        if (leg->underlying == L"OPTIONS") {
            ld = new LineData;
            ld->trade = trade;

            std::wstring expiryDate; // = QDate::fromString(leg->expiryDate, "yyyy-MM-dd");
            std::wstring wszShortDate = L"Mar 24"; // = expiryDate.toString("MMM dd");
            std::wstring wszDTE = L"6d";

            // If the ExpiryDate is 2 days or less then display in Yellow, otherwise Magenta.
            ThemeElement WarningDTE = ThemeElement::TradesPanelNormalDTE;
            //if (QDate::currentDate().daysTo(expiryDate) < 3) 
            //    WarningDTE = ThemeElement::TradesPanelwarningDTE;

            // If the expiry year is greater than current year + 1 then add
            // the year to the display string. Useful for LEAP options.
            //if (expiryDate.year() > QDate::currentDate().year() + 1) {
            //    strShortDate = strShortDate + "/" + QString::number(expiryDate.year());
            //}


            // wszDTE = QString::number(QDate::currentDate().daysTo(expiryDate)) + "d";
                        
                        
            SetColumnData(ld, 0, L"", StringAlignmentNear, ThemeElement::TradesPanelBack,
            ThemeElement::TradesPanelText, 8, FontStyleRegular );  // empty column

            SetColumnData(ld, 1, std::to_wstring(0x23FA), StringAlignmentNear, ThemeElement::TradesPanelBack,
                ThemeElement::TradesPanelText, 8, FontStyleRegular);   // dot (magenta/yellow)

            SetColumnData(ld, 2, std::to_wstring(leg->openQuantity), StringAlignmentFar, ThemeElement::TradesPanelColBackDark,
                ThemeElement::TradesPanelText, 8, FontStyleRegular);  // position quantity

            SetColumnData(ld, 3, wszShortDate, StringAlignmentCenter, ThemeElement::TradesPanelColBackLight,
                ThemeElement::TradesPanelText, 8, FontStyleRegular);   // expiry date

            SetColumnData(ld, 4, wszDTE, StringAlignmentCenter, ThemeElement::TradesPanelColBackDark,
                ThemeElement::TradesPanelTextDim, 8, FontStyleRegular);   // DTE

            SetColumnData(ld, 5, leg->strikePrice, StringAlignmentCenter, ThemeElement::TradesPanelColBackLight,
                ThemeElement::TradesPanelText, 8, FontStyleRegular);   // strike price

            SetColumnData(ld, 6, leg->PutCall, StringAlignmentNear, ThemeElement::TradesPanelColBackDark,
                ThemeElement::TradesPanelTextDim, 8, FontStyleRegular);   // PutCall

            vec.push_back(ld);
        }
    }


    // *** BLANK SEPARATION LINE ***
    ld = new LineData;
    for (int i = 0; i < 7; i++) {
        SetColumnData(ld, i, L"", StringAlignmentNear, ThemeElement::TradesPanelBack,
            ThemeElement::TradesPanelTextDim, 8, FontStyleRegular); 
    }
    vec.push_back(ld);
}



//' ========================================================================================
//' Populate the Trades ListBox with the current active/open trades
//' ========================================================================================
void ShowActiveTrades()
{
  //  PauseTWS();

    //ui->labelTicker->setText("Active Trades");

    // TODO: optimize this by sorting after initial database load and after
    // newly added/deleted data, rather than here every time we display the
    // active trades.
    // Sort the trades vector based on ticker symbol
    std::sort(trades.begin(), trades.end(),
        [](const Trade* trade1, const Trade* trade2) {
            return (trade1->tickerSymbol < trade2->tickerSymbol) ? true : false;
        });


    // Clear the current trades list
    ListBox_ResetContent(GetDlgItem(HWND_TRADESPANEL, IDC_LISTBOX));

    // Destroy any existing ListBox line data
    DestroyListBoxDisplayData();

    // Create the new ListBox line data
    for (const auto& trade : trades) {
        // We are displaying only all open trades
        if (trade->isOpen) {
            CreateListBoxData(trade);
        }
    }

    // Display the new ListBox data
    for (int i = 0; i < (int)vec.size(); i++) {
        ListBox_AddString(GetDlgItem(HWND_TRADESPANEL, IDC_LISTBOX), L""); 
    }
    AfxRedrawWindow(GetDlgItem(HWND_TRADESPANEL, IDC_LISTBOX));

    //ResumeTWS();
}


//' ========================================================================================
//' Process WM_DRAWITEM message 
//' ========================================================================================
int OnDrawItem(HWND hWnd, DRAWITEMSTRUCT* lpdis)
{
    if (lpdis == nullptr) return 0;

    if (lpdis->itemAction == ODA_DRAWENTIRE ||
        lpdis->itemAction == ODA_SELECT ||
        lpdis->itemAction == ODA_FOCUS) {

        int linenum = lpdis->itemID;
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

        // Set some defaults in case there is no valid ListBox line number
        std::wstring wszText;
        
        DWORD nBackColor = (bIsHot)
            ? GetThemeColor(ThemeElement::TradesPanelBackHot)
            : GetThemeColor(ThemeElement::TradesPanelBack);
        DWORD nTextColor = GetThemeColor(ThemeElement::TradesPanelText);
        
        std::wstring wszFontName = L"Arial";
        FontFamily   fontFamily(wszFontName.c_str());
        REAL fontSize = 8;
        int fontStyle = FontStyleRegular;
        
        StringAlignment alignment = StringAlignmentNear;


        // Paint the full width background using brush 
        SolidBrush backBrush(nBackColor);
        graphics.FillRectangle(&backBrush, 0, 0, nWidth, nHeight);


        // Get the current ListBox line data should a valid line exist
        // Paint the individual columns with the specific data.
        if (linenum > -1) {
            LineData* ld = vec.at(linenum);
            if (ld != nullptr) {
                int nColWidth = (int)(nWidth / 7);  // (int)AfxScaleX(120);
                int nLeft = 0;

                // Draw each of the columns
                for (int i = 0; i < 7; i++) {
                    wszText = ld->col[i].wszText;
                    
                    alignment = ld->col[i].alignment;
                    nBackColor = (bIsHot) 
                        ? GetThemeColor(ThemeElement::TradesPanelBackHot) 
                        : GetThemeColor(ld->col[i].backTheme);
                    nTextColor = GetThemeColor(ld->col[i].textTheme);
                    fontSize = ld->col[i].fontSize;
                    fontStyle = ld->col[i].fontStyle;

                    backBrush.SetColor(nBackColor);
                    graphics.FillRectangle(&backBrush, nLeft, 0, nColWidth, nHeight);
                    
                    Font         font(&fontFamily, fontSize, fontStyle, Unit::UnitPoint);
                    SolidBrush   textBrush(nTextColor);
                    StringFormat stringF(StringFormatFlagsNoWrap);
                    stringF.SetAlignment(alignment);
                    stringF.SetLineAlignment(StringAlignmentCenter);

                    RectF rcText((REAL)nLeft, (REAL)0, (REAL)nColWidth, (REAL)nHeight);
                    graphics.DrawString(wszText.c_str(), -1, &font, rcText, &stringF, &textBrush);
                    
                    nLeft += nColWidth;
                }

            }
        }

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
        // Destroy all manually allocated ListBox display data that is held
        // in the vector.
        DestroyListBoxDisplayData();

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

    HWND_TRADESPANEL =
        pWindow->Create(hWndParent, L"", &TradesPanel_WndProc, 0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
            WS_EX_CONTROLPARENT | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);

    // This is a child window of the main application parent so treat it like child
    // control and assign it a ControlID.
    SetWindowLongPtr(HWND_TRADESPANEL, GWLP_ID, IDC_TRADESPANEL);

    // Can only set the brush after the window is created
    pWindow->SetBrush(GetStockBrush(NULL_BRUSH));

    // Create an Ownerdraw fixed row sized listbox that we will use to custom
    // paint our various open trades.
    HWND hListBox = 
        pWindow->AddControl(Controls::ListBox, HWND_TRADESPANEL, IDC_LISTBOX, L"", 
            0, 0, 0, 0, 
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP | 
            LBS_NOINTEGRALHEIGHT | LBS_EXTENDEDSEL | LBS_MULTIPLESEL |
            LBS_NODATA | LBS_OWNERDRAWFIXED | LBS_NOTIFY, 
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL, 
            (SUBCLASSPROC)&TradesPanelListBox_SubclassProc, 
            IDC_LISTBOX, (DWORD_PTR)pWindow);

    return pWindow;

}


