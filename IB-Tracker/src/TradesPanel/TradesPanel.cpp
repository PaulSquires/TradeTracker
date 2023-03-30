
#include "pch.h"
#include "..\Utilities\SuperLabel.h"
#include "..\MainWindow\MainWindow.h"
#include "..\MainWindow\tws-client.h"
#include "..\Database\trade.h"
#include "..\Themes\Themes.h"
#include "TradesPanel.h"


HWND HWND_TRADESPANEL = NULL;

const int LISTBOX_ROWHEIGHT = 24;
const int VSCROLLBAR_WIDTH = 14;
const int VSCROLLBAR_MINTHUMBSIZE = 20;

const int NUM_COLUMNS = 8;
int nMinColWidth[NUM_COLUMNS] =
{
    25,     /* dropdown arrow*/
    45,     /* ticker symbol */
    50,     /* ITM */
    50,     /* position quantity */
    50,     /* expiry date */
    40,     /* DTE */
    45,     /* strike price */
    40      /* put/call */
};
int nColWidth[NUM_COLUMNS] = {};


class VScrollBar
{
public:
    HWND hWnd = NULL;
    HWND hListBox = NULL;
    bool bDragActive = false;
    int listBoxHeight = 0;
    int itemHeight = 0;
    int numItems = 0;
    int itemsPerPage = 0;
    int thumbHeight = 0;
    RECT rc{};
};


std::vector<LineData*> vec;

VScrollBar vsb;



//' ========================================================================================
// Destroy all manually allocated ListBox display data that is held in the vector.
//' ========================================================================================
void DestroyListBoxDisplayData()
{
    for (LineData* ld : vec) {
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
    // *** TRADE HEADER LINE ***
    LineData* ld = new LineData;
    ld->trade = trade;
    ld->isTickerLine = true;

    SetColumnData(ld, 0, L"\u23F7", StringAlignmentCenter, ThemeElement::TradesPanelBack,
        ThemeElement::TradesPanelTextDim, 8, FontStyleRegular);
    SetColumnData(ld, 1, trade->tickerSymbol, StringAlignmentNear, ThemeElement::TradesPanelBack,
        ThemeElement::TradesPanelText, 9, FontStyleRegular | FontStyleBold);
    // Col 1 to 6 are set based on incoming TWS price data 
    SetColumnData(ld, COLUMN_TICKER_ITM, L"", StringAlignmentNear, ThemeElement::TradesPanelBack,
        ThemeElement::TradesPanelText, 8, FontStyleRegular);   // ITM
    SetColumnData(ld, 3, L"", StringAlignmentNear, ThemeElement::TradesPanelBack,
        ThemeElement::TradesPanelText, 8, FontStyleRegular);
    SetColumnData(ld, 4, L"", StringAlignmentNear, ThemeElement::TradesPanelBack,
        ThemeElement::TradesPanelText, 8, FontStyleRegular);
    SetColumnData(ld, COLUMN_TICKER_CHANGE, L"", StringAlignmentFar, ThemeElement::TradesPanelBack,
        ThemeElement::TradesPanelTextDim, 8, FontStyleRegular);   // price change
    SetColumnData(ld, COLUMN_TICKER_CURRENTPRICE, L"0.00", StringAlignmentCenter, ThemeElement::TradesPanelBack,
        ThemeElement::TradesPanelText, 9, FontStyleRegular | FontStyleBold);   // current price
    SetColumnData(ld, COLUMN_TICKER_PERCENTAGE, L"", StringAlignmentNear, ThemeElement::TradesPanelBack,
        ThemeElement::TradesPanelTextDim, 8, FontStyleRegular);   // price percentage change
    vec.push_back(ld);


    std::wstring wszDot = L"\u23FA";   // dot character
    ThemeElement WarningDTE = ThemeElement::TradesPanelNormalDTE;


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

    if (aggregate) {
        ld = new LineData;
        ld->trade = trade;

        SetColumnData(ld, 0, L"", StringAlignmentNear, ThemeElement::TradesPanelBack,
            ThemeElement::TradesPanelText, 8, FontStyleRegular);

        SetColumnData(ld, 1, wszDot, StringAlignmentFar, ThemeElement::TradesPanelBack,
            WarningDTE, 7, FontStyleRegular);   // dot (magenta/yellow)

        SetColumnData(ld, 2, L"", StringAlignmentNear, ThemeElement::TradesPanelBack,
            ThemeElement::TradesPanelText, 8, FontStyleRegular);  // empty column

        SetColumnData(ld, 3, textShares, StringAlignmentNear, ThemeElement::TradesPanelColBackDark,
            ThemeElement::TradesPanelTextDim, 8, FontStyleRegular);  // SHARES/FUTURES

        SetColumnData(ld, 4, L"", StringAlignmentCenter, ThemeElement::TradesPanelColBackDark,
            ThemeElement::TradesPanelText, 8, FontStyleRegular);   // empty

        SetColumnData(ld, 5, L"", StringAlignmentCenter, ThemeElement::TradesPanelColBackDark,
            ThemeElement::TradesPanelTextDim, 8, FontStyleRegular);   // empty

        SetColumnData(ld, 6, L"11.58", StringAlignmentCenter, ThemeElement::TradesPanelColBackDark,
            ThemeElement::TradesPanelTextDim, 8, FontStyleRegular);   // strike price

        SetColumnData(ld, 7, L"200", StringAlignmentCenter, ThemeElement::TradesPanelColBackDark,
            ThemeElement::TradesPanelText, 8, FontStyleRegular);   // PutCall

        vec.push_back(ld);
    }


    // *** OPTION LEGS ***
    for (const auto& leg : trade->openLegs) {
        if (leg->underlying == L"OPTIONS") {
            ld = new LineData;
            ld->trade = trade;

            std::wstring currentDate = AfxCurrentDate();
            std::wstring expiryDate = leg->expiryDate;
            std::wstring wszShortDate = AfxShortDate(expiryDate);
            std::wstring wszDTE;


            // If the ExpiryDate is 2 days or less then display in Yellow, otherwise Magenta.
            int DTE = AfxDaysBetween(currentDate, expiryDate);
            wszDTE = std::to_wstring(DTE) + L"d";

            if (DTE < 3) {
                WarningDTE = ThemeElement::TradesPanelWarningDTE;
            }

            // If the expiry year is greater than current year + 1 then add
            // the year to the display string. Useful for LEAP options.
            if (AfxGetYear(expiryDate) > AfxGetYear(currentDate) + 1) {
                wszShortDate.append(L"/"); 
                wszShortDate.append(std::to_wstring(AfxGetYear(expiryDate)));
            }
                        
            SetColumnData(ld, 0, L"", StringAlignmentNear, ThemeElement::TradesPanelBack,
                ThemeElement::TradesPanelText, 8, FontStyleRegular);

            SetColumnData(ld, 1, wszDot, StringAlignmentFar, ThemeElement::TradesPanelBack,
                WarningDTE, 7, FontStyleRegular);   // dot (magenta/yellow)

            SetColumnData(ld, 2, L"", StringAlignmentNear, ThemeElement::TradesPanelBack,
                ThemeElement::TradesPanelText, 8, FontStyleRegular);  // empty column

            SetColumnData(ld, 3, std::to_wstring(leg->openQuantity), StringAlignmentFar, ThemeElement::TradesPanelColBackDark,
                ThemeElement::TradesPanelText, 8, FontStyleRegular);  // position quantity

            SetColumnData(ld, 4, wszShortDate, StringAlignmentCenter, ThemeElement::TradesPanelColBackLight,
                ThemeElement::TradesPanelText, 8, FontStyleRegular);   // expiry date

            SetColumnData(ld, 5, wszDTE, StringAlignmentCenter, ThemeElement::TradesPanelColBackDark,
                ThemeElement::TradesPanelTextDim, 8, FontStyleRegular);   // DTE

            SetColumnData(ld, 6, leg->strikePrice, StringAlignmentCenter, ThemeElement::TradesPanelColBackLight,
                ThemeElement::TradesPanelText, 8, FontStyleRegular);   // strike price

            SetColumnData(ld, 7, L"  " + leg->PutCall, StringAlignmentNear, ThemeElement::TradesPanelColBackDark,
                ThemeElement::TradesPanelTextDim, 8, FontStyleRegular);   // PutCall

            vec.push_back(ld);
        }
    }


    // *** BLANK SEPARATION LINE ***
    ld = new LineData;
    vec.push_back(ld);
}



//' ========================================================================================
//' Calculate the actual column widths based on the size of the strings in
//' the vector while respecting the minimum values as defined in nMinColWidth[].
//' This function is also called when receiving new price data from TWS because
//' that data may need the column width to be wider.
//' ========================================================================================
void CalculateColumnWidths(int nIndex)
{
    HDC hdc = GetDC(vsb.hListBox);

    Graphics graphics(hdc);
    graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

    std::wstring wszFontName = AfxGetDefaultFont();
    FontFamily fontFamily(wszFontName.c_str());
    REAL fontSize = 10;
    int fontStyle = FontStyleRegular;
    RectF boundRect;
    RectF layoutRect(0.0f, 0.0f, 100.0f, 50.0f);
    StringFormat format;
    format.SetAlignment(StringAlignmentFar);

    bool bRedrawListBox = false;

    for (auto& ld : vec) {
        // If a specific line number was passed intot his function then we only
        // test for that line rather than all lines (like when the arrays are first loaded).
        if (nIndex != -1) ld = vec.at(nIndex);
        for (int i = 0; i < NUM_COLUMNS; i++) {
            fontSize = ld->col[i].fontSize;
            fontStyle = ld->col[i].fontStyle;
            Font font(&fontFamily, fontSize, fontStyle, Unit::UnitPoint);
            graphics.MeasureString(ld->col[i].wszText.c_str(), ld->col[i].wszText.length(), 
                &font, layoutRect, &format, &boundRect);

            nColWidth[i] = max(nColWidth[i], nMinColWidth[i]);
            int textLength = (int)AfxUnScaleX(boundRect.Width) + 6;  // add a bit for padding
            if (textLength > nColWidth[i]) {  
                nColWidth[i] = textLength;
                bRedrawListBox = true;
            }
        }
        if (nIndex != -1) break;
    }
    ReleaseDC(vsb.hListBox, hdc);

    if (bRedrawListBox)
        AfxRedrawWindow(vsb.hListBox);
}


//' ========================================================================================
//' Populate the Trades ListBox with the current active/open trades
//' ========================================================================================
void ShowActiveTrades()
{
    tws_PauseTWS();

    vec.reserve(100);    // reserve space for 100 lines

    // TODO: optimize this by sorting after initial database load and after
    // newly added/deleted data, rather than here every time we display the
    // active trades.
    // Sort the trades vector based on ticker symbol
    std::sort(trades.begin(), trades.end(),
        [](const Trade* trade1, const Trade* trade2) {
            return (trade1->tickerSymbol < trade2->tickerSymbol) ? true : false;
        });

    
    // Cancel any previous market data requests
    for (const auto& ld: vec) {
        if (ld->isTickerLine) {
            tws_cancelMktData(ld->tickerId);
        }
    }


    // Clear the current trades list
    ListBox_ResetContent(GetDlgItem(HWND_TRADESPANEL, IDC_LISTBOX));

    // Destroy any existing ListBox line data
    // This will also clear the vector
    DestroyListBoxDisplayData();

    // Create the new ListBox line data
    for (const auto& trade : trades) {
        // We are displaying only all open trades
        if (trade->isOpen) {
            CreateListBoxData(trade);
        }
    }


    // Calculate the actual column widths based on the size of the strings in
    // the vector while respecting the minimum values as defined in nMinColWidth[].
    // This function is also called when receiving new price data from TWS because
    // that data may need the column width to be wider.
    CalculateColumnWidths();


    // Display the new ListBox data
    for (auto& ld : vec) {
        int nIndex = ListBox_AddString(GetDlgItem(HWND_TRADESPANEL, IDC_LISTBOX), L"");
        // Request price market data for all non-listbox blank lines. 
        // Blank lines will have nullptr trade and this is tested for
        // in the called routine.
        if (ld->isTickerLine) {
            ld->tickerId = nIndex + TICKER_NUMBER_OFFEST;
            tws_requestMktData(ld);
        }
    }
    AfxRedrawWindow(GetDlgItem(HWND_TRADESPANEL, IDC_LISTBOX));
    AfxRedrawWindow(vsb.hWnd);

    tws_ResumeTWS();
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
        graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
        

        // Set some defaults in case there is no valid ListBox line number
        std::wstring wszText;
        
        DWORD nBackColor = (bIsHot)
            ? GetThemeColor(ThemeElement::TradesPanelBackHot)
            : GetThemeColor(ThemeElement::TradesPanelBack);
        DWORD nTextColor = GetThemeColor(ThemeElement::TradesPanelText);
        
        std::wstring wszFontName = AfxGetDefaultFont();
        FontFamily   fontFamily(wszFontName.c_str());
        REAL fontSize = 10;
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
                int nLeft = 0;

                // Draw each of the columns
                for (int i = 0; i < NUM_COLUMNS; i++) {
                    wszText = ld->col[i].wszText;
                    
                    alignment = ld->col[i].alignment;
                    nBackColor = (bIsHot) 
                        ? GetThemeColor(ThemeElement::TradesPanelBackHot) 
                        : GetThemeColor(ld->col[i].backTheme);
                    nTextColor = GetThemeColor(ld->col[i].textTheme);
                    fontSize = ld->col[i].fontSize;
                    fontStyle = ld->col[i].fontStyle;

                    int colWidth = (int)AfxScaleX((float)nColWidth[i]);

                    backBrush.SetColor(nBackColor);
                    graphics.FillRectangle(&backBrush, nLeft, 0, colWidth, nHeight);
                    
                    Font         font(&fontFamily, fontSize, fontStyle, Unit::UnitPoint);
                    SolidBrush   textBrush(nTextColor);
                    StringFormat stringF(StringFormatFlagsNoWrap);
                    stringF.SetAlignment(alignment);
                    stringF.SetLineAlignment(StringAlignmentCenter);

                    RectF rcText((REAL)nLeft, (REAL)0, (REAL)colWidth, (REAL)nHeight);
                    graphics.DrawString(wszText.c_str(), -1, &font, rcText, &stringF, &textBrush);
                    
                    nLeft += colWidth;
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
//' Calculate the RECT that holds the client coordinates of the scrollbar's vertical thumb
//' Will return TRUE if RECT is not empty. 
//' ========================================================================================
bool calcVThumbRect()
{
    // calculate the vertical scrollbar in client coordinates
    SetRectEmpty(&vsb.rc);
    int nTopIndex = SendMessage(vsb.hListBox, LB_GETTOPINDEX, 0, 0);

    RECT rc{};
    GetClientRect(vsb.hListBox, &rc);
    vsb.listBoxHeight = (rc.bottom - rc.top);
    vsb.itemHeight = ListBox_GetItemHeight(vsb.hListBox, 0);
    vsb.numItems = ListBox_GetCount(vsb.hListBox);
    
    // If no items exist then exit to avoid division by zero GPF's.
    if (vsb.numItems == 0) return FALSE;
    
    vsb.itemsPerPage = (int)(std::round(vsb.listBoxHeight / (float)vsb.itemHeight));
    vsb.thumbHeight = (int)(((float)vsb.itemsPerPage / (float)vsb.numItems) * (float)vsb.listBoxHeight);

    vsb.rc.left = rc.left;
    vsb.rc.top = (int)(rc.top + (((float)nTopIndex / (float)vsb.numItems) * (float)vsb.listBoxHeight));
    vsb.rc.right = rc.right;
    vsb.rc.bottom = (vsb.rc.top + vsb.thumbHeight);
    
    // If the number of items in the listbox is less than what could display
    // on the screen then there is no need to show the scrollbar.
    return (vsb.numItems < vsb.itemsPerPage) ? FALSE : TRUE;

}


//' ========================================================================================
//' Vertical scrollBar subclass Window procedure
//' ========================================================================================
LRESULT CALLBACK VScrollBar_SubclassProc(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{

    static POINT prev_pt;             // screen pt.y cursor position

    switch (uMsg)
    {
    case WM_LBUTTONDOWN:
        {
        POINT pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        calcVThumbRect();           
        if (PtInRect(&vsb.rc, pt)) {
            prev_pt = pt;
            vsb.bDragActive = true;
            SetCapture(hWnd);
        }
        else {
            // we have clicked on a PageUp or PageDn
            int nTopIndex = SendMessage(vsb.hListBox, LB_GETTOPINDEX, 0, 0);
            if (pt.y < vsb.rc.top) {
                nTopIndex = max(nTopIndex - vsb.itemsPerPage, 0);
                SendMessage(vsb.hListBox, LB_SETTOPINDEX, nTopIndex, 0);
                calcVThumbRect();   
                AfxRedrawWindow(vsb.hWnd);
            } else {
                if (pt.y > vsb.rc.bottom) {
                    int nMaxTopIndex = vsb.numItems - vsb.itemsPerPage;
                    nTopIndex = min(nTopIndex + vsb.itemsPerPage, nMaxTopIndex);
                    SendMessage(vsb.hListBox, LB_SETTOPINDEX, nTopIndex, 0);
                    calcVThumbRect(); 
                    AfxRedrawWindow(vsb.hWnd);
                }
            }

        }
        break;
        }


    case WM_MOUSEMOVE:
        {
        if (vsb.bDragActive) {
            POINT pt;
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            if (pt.y != prev_pt.y) {
                int delta = (pt.y - prev_pt.y);

                RECT rc; GetClientRect(hWnd, &rc);
                vsb.rc.top = max(0, vsb.rc.top + delta);
                vsb.rc.top = min(vsb.rc.top, rc.bottom - vsb.thumbHeight);
                vsb.rc.bottom = vsb.rc.top + vsb.thumbHeight;

                prev_pt = pt;

                int nPrevTopLine = SendMessage(vsb.hListBox, LB_GETTOPINDEX, 0, 0);
                int nTopLine = (int)std::round(vsb.rc.top / (float)rc.bottom * vsb.numItems);
                if (nTopLine != nPrevTopLine)
                    SendMessage(vsb.hListBox, LB_SETTOPINDEX, (WPARAM)nTopLine, 0);

                //std::cout << nPrevTopLine << "  " << nTopLine << std::endl;

                AfxRedrawWindow(hWnd);
            }
        }
        break;
        }


    case WM_LBUTTONUP:
        {
        vsb.bDragActive = false;
        prev_pt.x = 0;
        prev_pt.y = 0;
        ReleaseCapture();
        break;
        }

    case WM_ERASEBKGND:
        return TRUE;
        break;
        
        
    case WM_PAINT:
        {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        SaveDC(hdc);

        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP hbit = CreateCompatibleBitmap(hdc, ps.rcPaint.right, ps.rcPaint.bottom);
        SelectBitmap(memDC, hbit);

        Graphics graphics(memDC);
        int nWidth = (ps.rcPaint.right - ps.rcPaint.left);
        int nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);

        SolidBrush backBrush(GetThemeColor(ThemeElement::TradesPanelScrollBarBack));
        graphics.FillRectangle(&backBrush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

        backBrush.SetColor(GetThemeColor(ThemeElement::TradesPanelScrollBarThumb));
        graphics.FillRectangle(&backBrush, vsb.rc.left, vsb.rc.top, nWidth, vsb.thumbHeight);
        
        Pen pen(GetThemeColor(ThemeElement::TradesPanelScrollBarLine), 1);
        graphics.DrawLine(&pen, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.left, ps.rcPaint.bottom);

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

                                               

    case WM_DESTROY:
    {
        // REQUIRED: Remove control subclassing
        RemoveWindowSubclass(hWnd, &VScrollBar_SubclassProc, uIdSubclass);
        break;
    }


    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);

}



//' ========================================================================================
//' Listbox subclass Window procedure
//' ========================================================================================
LRESULT CALLBACK ListBox_SubclassProc(
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
        calcVThumbRect();   
        AfxRedrawWindow(vsb.hWnd);
        break;
    }

    case WM_MOUSEMOVE:
        break;

    case WM_MOUSEHOVER:
        break;

    case WM_MOUSELEAVE:
        break;


    case WM_RBUTTONDOWN:
    {
        // Create the popup menu
        int idx = Listbox_ItemFromPoint(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        // The return value contains the index of the nearest item in the LOWORD. The HIWORD is zero 
        // if the specified point is in the client area of the list box, or one if it is outside the 
        // client area.
        if (HIWORD(idx) != 1) {
            //ListBox_SetSel(hWnd, TRUE, idx);
            //dim as HMENU hPopupMenu = CreateExplorerContextMenu(pDoc)
            //dim as POINT pt : GetCursorPos(@pt)
            //dim as long id = TrackPopupMenu(hPopUpMenu, TPM_RETURNCMD, pt.x, pt.y, 0, HWND_FRMMAIN, byval null)
            //AfxRedrawWindow(hWnd);
        }
        break;
    }


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
            HDC hDC = (HDC)wParam;
            Graphics graphics(hDC);
            SolidBrush backBrush(GetThemeColor(ThemeElement::TradesPanelBack));
            graphics.FillRectangle(&backBrush, rc.left, rc.top, nWidth, nHeight);
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
        RemoveWindowSubclass(hWnd, &ListBox_SubclassProc, uIdSubclass);
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
            // TODO: Handle the repaint in the subclass (WM_LBUTTONDOWN) rather than this notification because
            // of the delay in repainting the switched lines.
            AfxRedrawWindow(hCtl);
            //  update the scrollbar position if necessary
            calcVThumbRect();
            AfxRedrawWindow(vsb.hWnd);
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
        // Move ListBox leaving a top and bottom margin.
        RECT rcClient;
        GetClientRect(hWnd, &rcClient);

        // Do not call the calcVThumbRect() function during a scrollbar move. This WM_SIZE
        // gets triggered when the ListBox WM_DRAWITEM fires. If we do another calcVThumbRect()
        // calcualtion then the scrollbar will appear "jumpy" under the user's mouse cursor.
        bool bShowScrollBar = false;
        if (vsb.bDragActive) {
            bShowScrollBar = true;
        }
        else {
            bShowScrollBar = calcVThumbRect();
        }
        int VScrollBarWidth = bShowScrollBar ? (int)AfxScaleX(VSCROLLBAR_WIDTH) : 0;

        int margin = (int)AfxScaleY(LISTBOX_ROWHEIGHT);

        int nLeft = rcClient.left;
        int nTop = rcClient.top + margin;
        int nWidth = rcClient.right - rcClient.left - VScrollBarWidth;
        int nHeight = rcClient.bottom - rcClient.top - (margin * 2);
        SetWindowPos(GetDlgItem(hWnd, IDC_LISTBOX), 0, 
            nLeft, nTop, nWidth, nHeight, SWP_NOZORDER | SWP_SHOWWINDOW);

        nLeft = nLeft + nWidth;   // right edge of ListBox
        nWidth = VScrollBarWidth;
        SetWindowPos(GetDlgItem(hWnd, IDC_VSCROLLBAR), 0,
            nLeft, nTop, nWidth, nHeight, 
            SWP_NOZORDER | (bShowScrollBar ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));
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

        Graphics graphics(hdc);

        DWORD nBackColor = GetThemeColor(ThemeElement::NavPanelBack);

        // Create the background brush
        SolidBrush backBrush(nBackColor);

        // Paint the background using brush.
        int nWidth = (ps.rcPaint.right - ps.rcPaint.left);
        int nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);
        graphics.FillRectangle(&backBrush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

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

    
    SuperLabel* pData = nullptr;
    
    HWND hCtl = CreateSuperLabel(
        HWND_TRADESPANEL,
        IDC_LABEL,
        SuperLabelType::TextOnly,
        0, 0, 200, LISTBOX_ROWHEIGHT);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = false;
        pData->BackColor = ThemeElement::NavPanelBack;
        pData->TextColor = ThemeElement::NavPanelText;
        pData->FontSize = 9;
        pData->TextAlignment = SuperLabelAlignment::MiddleLeft;
        pData->wszText = L"Active Trades";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }


    // Create an Ownerdraw fixed row sized listbox that we will use to custom
    // paint our various open trades.
    vsb.hListBox = 
        pWindow->AddControl(Controls::ListBox, HWND_TRADESPANEL, IDC_LISTBOX, L"", 
            0, 0, 0, 0, 
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP | 
            LBS_NOINTEGRALHEIGHT | LBS_EXTENDEDSEL | LBS_MULTIPLESEL |
            LBS_NODATA | LBS_OWNERDRAWFIXED | LBS_NOTIFY, 
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL, 
            (SUBCLASSPROC)&ListBox_SubclassProc, 
            IDC_LISTBOX, (DWORD_PTR)pWindow);

    vsb.hWnd =
        pWindow->AddControl(Controls::Custom, HWND_TRADESPANEL, IDC_VSCROLLBAR, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | SS_NOTIFY,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)&VScrollBar_SubclassProc,
            IDC_VSCROLLBAR, (DWORD_PTR)pWindow);

    
    ShowActiveTrades();
    
    // Set focus to ListBox so we get a correct repaint via WM_DRAWITEM
    SetFocus(vsb.hListBox);

    return pWindow;
}


