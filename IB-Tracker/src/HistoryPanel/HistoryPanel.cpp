
#include "pch.h"
#include "..\Utilities\SuperLabel.h"
#include "..\MainWindow\MainWindow.h"
#include "..\MainWindow\tws-client.h"
#include "..\Themes\Themes.h"
#include "HistoryPanel.h"

HWND HWND_HISTORYPANEL = NULL;

const int LISTBOX_ROWHEIGHT = 24;
const int VSCROLLBAR_WIDTH = 14;
const int VSCROLLBAR_MINTHUMBSIZE = 20;


class VScrollBar2
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

VScrollBar2 vsb;



// ========================================================================================
// Constructor
// ========================================================================================
CHistoryPanel::CHistoryPanel(HWND hWndParent)
{
    Show(hWndParent);
}


// ========================================================================================
// Destructor
// ========================================================================================
CHistoryPanel::~CHistoryPanel()
{
    if (m_pWindow) delete(m_pWindow);
}


// ========================================================================================
// Calculate the RECT that holds the client coordinates of the scrollbar's vertical thumb
// Will return TRUE if RECT is not empty. 
// ========================================================================================
bool CHistoryPanel::calcVThumbRect()
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


// ========================================================================================
// Vertical scrollBar subclass Window procedure
// ========================================================================================
LRESULT CALLBACK CHistoryPanel::VScrollBar_SubclassProc(
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
            }
            else {
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
        RemoveWindowSubclass(hWnd, (SUBCLASSPROC)VScrollBar_SubclassProc, uIdSubclass);
        break;
    }


    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);

}


// ========================================================================================
// Listbox subclass Window procedure
// ========================================================================================
LRESULT CALLBACK CHistoryPanel::ListBox_SubclassProc(
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
        //DestroyListBoxDisplayData();

        // REQUIRED: Remove control subclassing
        RemoveWindowSubclass(hWnd, ListBox_SubclassProc, uIdSubclass);
        break;


    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);

}


// ========================================================================================
// Process WM_DESTROY message for window/dialog: HistoryPanel
// ========================================================================================
void CHistoryPanel::OnDestroy(HWND hwnd)
{
    // TODO: Add your message processing code here...
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: HistoryPanel
// ========================================================================================
BOOL CHistoryPanel::OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    // Handle all of the painting in WM_PAINT
    return TRUE;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: HistoryPanel
// ========================================================================================
void CHistoryPanel::OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    Graphics graphics(hdc);

    DWORD nBackColor = GetThemeColor(ThemeElement::HistoryPanelBack);

    // Create the background brush
    SolidBrush backBrush(nBackColor);

    // Paint the background using brush.
    int nWidth = (ps.rcPaint.right - ps.rcPaint.left);
    int nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&backBrush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Process WM_SIZE message for window/dialog: HistoryPanel
// ========================================================================================
void CHistoryPanel::OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    // TODO: Add your message processing code here...
}



// ========================================================================================
// HistoryPanel  Window Procedure
// ========================================================================================
LRESULT CALLBACK CHistoryPanel::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
        HANDLE_MSG(hwnd, WM_ERASEBKGND, OnEraseBkgnd);
        HANDLE_MSG(hwnd, WM_PAINT, OnPaint);
        HANDLE_MSG(hwnd, WM_SIZE, OnSize);

        //// TODO: Add window message crackers here...

    default: return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}



// ========================================================================================
// HistoryPanel_Show
// ========================================================================================
void CHistoryPanel::Show(HWND hWndParent)
{
    // Create the window and child controls
    CWindow* m_pWindow = new CWindow;
   
    HWND_HISTORYPANEL =
        m_pWindow->Create(hWndParent, L"", WndProc, 0, 0, HISTORYPANEL_WIDTH, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
            WS_EX_CONTROLPARENT | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);
    
    // This is a child window of the main application parent so treat it like child
    // control and assign it a ControlID.
    SetWindowLongPtr(HWND_HISTORYPANEL, GWLP_ID, IDC_HISTORYPANEL);

    // Can only set the brush after the window is created
    m_pWindow->SetBrush(GetStockBrush(NULL_BRUSH));

    SuperLabel* pData = nullptr;

    HWND hCtl = CreateSuperLabel(
        HWND_HISTORYPANEL,
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
        pData->wszText = L"Trade History";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }


    // Create an Ownerdraw fixed row sized listbox that we will use to custom
    // paint our various open trades.
    vsb.hListBox =
        m_pWindow->AddControl(Controls::ListBox, HWND_HISTORYPANEL, IDC_LISTBOX, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_EXTENDEDSEL | LBS_MULTIPLESEL |
            LBS_NODATA | LBS_OWNERDRAWFIXED | LBS_NOTIFY,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)ListBox_SubclassProc,
            IDC_LISTBOX, (DWORD_PTR)m_pWindow);


    vsb.hWnd =
        m_pWindow->AddControl(Controls::Custom, HWND_HISTORYPANEL, IDC_VSCROLLBAR, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | SS_NOTIFY,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)VScrollBar_SubclassProc,
            IDC_VSCROLLBAR, (DWORD_PTR)m_pWindow);

}

