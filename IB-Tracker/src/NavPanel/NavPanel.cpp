
#include "pch.h"
#include "..\Utilities\SuperLabel.h"
#include "..\Utilities\UserMessages.h"
#include "..\Themes\Themes.h"
#include "..\MainWindow\MainWindow.h"
#include "..\MainWindow\tws-client.h"
#include "..\TradesPanel\TradesPanel.h"
#include "NavPanel.h"

// The NavPanel window is exposed external because other
// areas of the application need to send messages to the
// "messages" label to display. e.g TWS connection status.
HWND HWND_NAVPANEL = NULL;


// ========================================================================================
// Constructor
// ========================================================================================
CNavPanel::CNavPanel(HWND hWndParent)
{
    Show(hWndParent);
}


// ========================================================================================
// Destructor
// ========================================================================================
CNavPanel::~CNavPanel()
{
    if (m_pWindow) delete(m_pWindow);
}


// ========================================================================================
// Process WM_SIZE message for window/dialog: NavPanel
// ========================================================================================
void CNavPanel::OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    // Move the bottom separator and application name into place, but only do so
    // if the vertical height of NavBar window has not become less than the minimum
    // otherwise the two controls will bleed into the ones above them.
    int MinHeight = AfxScaleY(540);

    if (cy < MinHeight) return;

    SetWindowPos(GetDlgItem(hwnd, IDC_NAVPANEL_GEARICON), 0,
        AfxScaleX(12), cy - AfxScaleY(46), 0, 0, SWP_NOSIZE | SWP_NOZORDER);

    SetWindowPos(GetDlgItem(hwnd, IDC_NAVPANEL_MESSAGES), 0,
        AfxScaleX(35), cy - AfxScaleY(46), 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: NavPanel
// ========================================================================================
BOOL CNavPanel::OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    // Handle all of the painting in WM_PAINT
    return TRUE;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: NavPanel
// ========================================================================================
void CNavPanel::OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    Graphics graphics(hdc);

    DWORD nBackColor = GetThemeColor(ThemeElement::NavPanelBack);

    // Create the background brush
    SolidBrush backBrush(nBackColor);

    // Paint the background using brush.
    int nWidth = (ps.rcPaint.right - ps.rcPaint.left);
    int nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&backBrush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Process WM_DESTROY message for window/dialog: NavPanel
// ========================================================================================
void CNavPanel::OnDestroy(HWND hwnd)
{
    // TODO: Add your message processing code here...
}



// ========================================================================================
// NavPanel Window procedure
// ========================================================================================
LRESULT CALLBACK CNavPanel::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    HANDLE_MSG(hwnd, WM_SIZE, OnSize);
    HANDLE_MSG(hwnd, WM_ERASEBKGND, OnEraseBkgnd);
    HANDLE_MSG(hwnd, WM_PAINT, OnPaint);


    case MSG_TWS_CONNECT_START:
    {
        SuperLabel* pData = nullptr;
        pData = SuperLabel_GetOptions(GetDlgItem(hwnd, IDC_NAVPANEL_MESSAGES));
        if (pData) {
            pData->wszText = L"Connecting to TWS";
            AfxRedrawWindow(GetDlgItem(hwnd, IDC_NAVPANEL_MESSAGES));
        }
        break;
    }


    case MSG_TWS_CONNECT_SUCCESS:
    {
        SuperLabel* pData = nullptr;
        pData = SuperLabel_GetOptions(GetDlgItem(hwnd, IDC_NAVPANEL_MESSAGES));
        if (pData) {
            pData->wszText = L"TWS Connected";
            AfxRedrawWindow(GetDlgItem(hwnd, IDC_NAVPANEL_MESSAGES));
        }
        break;
    }


    case MSG_TWS_CONNECT_FAILURE:
    {
        SuperLabel* pData = nullptr;
        pData = SuperLabel_GetOptions(GetDlgItem(hwnd, IDC_NAVPANEL_MESSAGES));
        if (pData) {
            pData->wszText = L"TWS connection failed";
            AfxRedrawWindow(GetDlgItem(hwnd, IDC_NAVPANEL_MESSAGES));
        }
        break;
    }

    case MSG_TWS_CONNECT_DISCONNECT:
    {
        SuperLabel* pData = nullptr;
        pData = SuperLabel_GetOptions(GetDlgItem(hwnd, IDC_NAVPANEL_MESSAGES));
        if (pData) {
            pData->wszText = L"TWS Disonnected";
            AfxRedrawWindow(GetDlgItem(hwnd, IDC_NAVPANEL_MESSAGES));
        }
        break;
    }


    case MSG_SUPERLABEL_MOUSEMOVE:
        break;

      
    case MSG_SUPERLABEL_CLICK:
    {   
        HWND hCtl = (HWND)lParam;
        int CtrlId = (int)wParam;

        if (hCtl == NULL) return 0;
        SuperLabel* pData = (SuperLabel*)GetWindowLongPtr(hwnd, 0);

        if (pData) {

            switch (CtrlId) {
            
            case IDC_NAVPANEL_GEARICON:
            {
                // Prevent multiple clicks of the connect button by waiting until
                // the first click is finished.
                static bool bProcessingConnectClick = false;
                if (bProcessingConnectClick) break;
                bProcessingConnectClick = true;
                bool res = tws_connect();
                printf("Connect: %ld\n", res);
                printf("isConnected: %ld\n", tws_isConnected());
                // If we connect to TWS successfully then we need to start showing
                // the price data for any active trades displaying in our table.
                if (tws_isConnected())
                    ShowActiveTrades();
                bProcessingConnectClick = false;
                break;
            }

            case IDC_NAVPANEL_ACTIVETRADES:
            {
                ShowActiveTrades();
                break;
            }

            //case IDC_NAVPANEL_DARKTHEME:
            //{
            //    SetTheme(Themes::Dark);
            //    ApplyActiveTheme();
            //    break;
            //}

            //case IDC_NAVPANEL_DARKPLUSTHEME:
            //{
            //    SetTheme(Themes::DarkPlus);
            //    ApplyActiveTheme();
            //    break;
            //}

            //case IDC_NAVPANEL_BLUETHEME:
            //{
            //    SetTheme(Themes::Blue);
            //    ApplyActiveTheme();
            //    break;
            //}


            }

        }

    }
    break;


    default: return DefWindowProc(hwnd, msg, wParam, lParam);

    }
    return 0;
}


// ========================================================================================
// NavPanel_Show
// ========================================================================================
void CNavPanel::Show(HWND hWndParent)
{
    // Create the window and child controls
    CWindow* m_pWindow = new CWindow;
   
    HWND_NAVPANEL =
        m_pWindow->Create(hWndParent, L"", WndProc, 0, 0, NAVPANEL_WIDTH, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
            WS_EX_CONTROLPARENT | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);
    
    // This is a child window of the main application parent so treat it like child
    // control and assign it a ControlID.
    SetWindowLongPtr(HWND_NAVPANEL, GWLP_ID, IDC_NAVPANEL);

    // Can only set the brush after the window is created
    m_pWindow->SetBrush(GetStockBrush(NULL_BRUSH));

    int nTop, nLeft, nLeftOffset;
    int nItemHeight = 28;
        
    HWND hCtl;

    SuperLabel* pData = nullptr;



   // HEADER CONTROLS
    nLeft = (NAVPANEL_WIDTH - 68) / 2;
    hCtl = CreateSuperLabel(
        HWND_NAVPANEL,
        IDC_NAVPANEL_LOGO,
        SuperLabelType::ImageOnly,
        nLeft, 20, 68, 68);
   pData = SuperLabel_GetOptions(hCtl);
   if (pData) {
       pData->HotTestEnable = false;
       pData->BackColor = ThemeElement::NavPanelBack;
       pData->ImageWidth = 68;
       pData->ImageHeight = 68;
       pData->pImage = LoadImageFromResource(pData->hInst, MAKEINTRESOURCE(IDB_LOGO), L"PNG");
       pData->pImageHot = LoadImageFromResource(pData->hInst, MAKEINTRESOURCE(IDB_LOGO), L"PNG");
       SuperLabel_SetOptions(hCtl, pData);
   }


    hCtl = CreateSuperLabel(
        HWND_NAVPANEL,
        IDC_NAVPANEL_USERNAME,
        SuperLabelType::TextOnly,
        0, 100, NAVPANEL_WIDTH, 18);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = false;
        pData->BackColor = ThemeElement::NavPanelBack;
        pData->TextColor = ThemeElement::NavPanelTextDim;
        pData->FontSize = 10;
        pData->TextAlignment = SuperLabelAlignment::MiddleCenter;
        pData->wszText = GetTraderName();
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }


    hCtl = CreateSuperLabel(
        HWND_NAVPANEL, 
        IDC_NAVPANEL_APPNAME,
        SuperLabelType::TextOnly, 
        0, 118, NAVPANEL_WIDTH, 18);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = false;
        pData->BackColor = ThemeElement::NavPanelBack;
        pData->TextColor = ThemeElement::NavPanelText;
        pData->FontSize = 10;
        pData->TextAlignment = SuperLabelAlignment::MiddleCenter;
        pData->wszText = L"IB-Tracker v1.0";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }
      
   
    // SEPARATOR
    nTop = 150;
    hCtl = CreateSuperLabel(
        HWND_NAVPANEL, -1, 
        SuperLabelType::LineHorizontal, 
        0, nTop, NAVPANEL_WIDTH, 10);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->BackColor = ThemeElement::NavPanelBack;
        pData->LineColor = ThemeElement::NavPanelSeparator;
        pData->LineWidth = 6;
        pData->MarginLeft = 10;
        pData->MarginRight = 10;
        SuperLabel_SetOptions(hCtl, pData);
    }

   
    // MENU ITEMS
    nLeftOffset = 0;
    nTop = nTop + 10;
    hCtl = CreateSuperLabel(
        HWND_NAVPANEL,
        IDC_NAVPANEL_ACTIVETRADES,
        SuperLabelType::TextOnly,
        0, nTop, NAVPANEL_WIDTH, nItemHeight);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = true;
        pData->AllowSelect = true;
        pData->SelectorColor = ThemeElement::TradesPanelBack;   // selector should be same color as middle panel
        pData->BackColor = ThemeElement::NavPanelBack;
        pData->BackColorHot = ThemeElement::NavPanelBackHot;
        pData->BackColorSelected = ThemeElement::NavPanelBackSelected;
        pData->TextColor = ThemeElement::NavPanelText;
        pData->TextColorHot = ThemeElement::NavPanelText;
        pData->TextOffsetLeft = nLeftOffset;
        pData->FontSize = 10;
        pData->FontSizeHot = 10;
        pData->wszText = L"Active Trades";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }
   
   
    nTop = nTop + nItemHeight;
    hCtl = CreateSuperLabel(
        HWND_NAVPANEL,
        IDC_NAVPANEL_CLOSEDTRADES,
        SuperLabelType::TextOnly,
        0, nTop, NAVPANEL_WIDTH, nItemHeight);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = true;
        pData->AllowSelect = true;
        pData->SelectorColor = ThemeElement::TradesPanelBack;   // selector should be same color as middle panel
        pData->BackColor = ThemeElement::NavPanelBack;
        pData->BackColorHot = ThemeElement::NavPanelBackHot;
        pData->BackColorSelected = ThemeElement::NavPanelBackSelected;
        pData->TextColor = ThemeElement::NavPanelText;
        pData->TextColorHot = ThemeElement::NavPanelText;
        pData->TextOffsetLeft = nLeftOffset;
        pData->FontSize = 10;
        pData->FontSizeHot = 10;
        pData->wszText = L"Closed Trades";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }


    // SEPARATOR
    nTop = nTop + nItemHeight + 6;
    hCtl = CreateSuperLabel(
        HWND_NAVPANEL, -1,
        SuperLabelType::LineHorizontal,
        0, nTop, NAVPANEL_WIDTH, 10);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->BackColor = ThemeElement::NavPanelBack;
        pData->LineColor = ThemeElement::NavPanelSeparator;
        pData->LineWidth = 6;
        pData->MarginLeft = 10;
        pData->MarginRight = 10;
        SuperLabel_SetOptions(hCtl, pData);
    }


    nTop = nTop + 10;
    hCtl = CreateSuperLabel(
        HWND_NAVPANEL,
        IDC_NAVPANEL_NEWTRADE,
        SuperLabelType::TextOnly,
        0, nTop, NAVPANEL_WIDTH, nItemHeight);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = true;
        pData->AllowSelect = true;
        pData->SelectorColor = ThemeElement::TradesPanelBack;   // selector should be same color as middle panel
        pData->BackColor = ThemeElement::NavPanelBack;
        pData->BackColorHot = ThemeElement::NavPanelBackHot;
        pData->BackColorSelected = ThemeElement::NavPanelBackSelected;
        pData->TextColor = ThemeElement::NavPanelText;
        pData->TextColorHot = ThemeElement::NavPanelText;
        pData->TextOffsetLeft = nLeftOffset;
        pData->FontSize = 10;
        pData->FontSizeHot = 10;
        pData->wszText = L"New Trade";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }


    nTop = nTop + nItemHeight;
    hCtl = CreateSuperLabel(
        HWND_NAVPANEL,
        IDC_NAVPANEL_SHORTSTRANGLE,
        SuperLabelType::TextOnly,
        0, nTop, NAVPANEL_WIDTH, nItemHeight);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = true;
        pData->AllowSelect = true;
        pData->SelectorColor = ThemeElement::TradesPanelBack;   // selector should be same color as middle panel
        pData->BackColor = ThemeElement::NavPanelBack;
        pData->BackColorHot = ThemeElement::NavPanelBackHot;
        pData->BackColorSelected = ThemeElement::NavPanelBackSelected;
        pData->TextColor = ThemeElement::NavPanelText;
        pData->TextColorHot = ThemeElement::NavPanelText;
        pData->TextOffsetLeft = nLeftOffset;
        pData->FontSize = 10;
        pData->FontSizeHot = 10;
        pData->wszText = L"Short Strangle";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }


    nTop = nTop + nItemHeight;
    hCtl = CreateSuperLabel(
        HWND_NAVPANEL,
        IDC_NAVPANEL_SHORTPUT,
        SuperLabelType::TextOnly,
        0, nTop, NAVPANEL_WIDTH, nItemHeight);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = true;
        pData->AllowSelect = true;
        pData->SelectorColor = ThemeElement::TradesPanelBack;   // selector should be same color as middle panel
        pData->BackColor = ThemeElement::NavPanelBack;
        pData->BackColorHot = ThemeElement::NavPanelBackHot;
        pData->BackColorSelected = ThemeElement::NavPanelBackSelected;
        pData->TextColor = ThemeElement::NavPanelText;
        pData->TextColorHot = ThemeElement::NavPanelText;
        pData->TextOffsetLeft = nLeftOffset;
        pData->FontSize = 10;
        pData->FontSizeHot = 10;
        pData->wszText = L"Short Put";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }


    nTop = nTop + nItemHeight;
    hCtl = CreateSuperLabel(
        HWND_NAVPANEL,
        IDC_NAVPANEL_SHORTCALL,
        SuperLabelType::TextOnly,
        0, nTop, NAVPANEL_WIDTH, nItemHeight);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = true;
        pData->AllowSelect = true;
        pData->SelectorColor = ThemeElement::TradesPanelBack;   // selector should be same color as middle panel
        pData->BackColor = ThemeElement::NavPanelBack;
        pData->BackColorHot = ThemeElement::NavPanelBackHot;
        pData->BackColorSelected = ThemeElement::NavPanelBackSelected;
        pData->TextColor = ThemeElement::NavPanelText;
        pData->TextColorHot = ThemeElement::NavPanelText;
        pData->TextOffsetLeft = nLeftOffset;
        pData->FontSize = 10;
        pData->FontSizeHot = 10;
        pData->wszText = L"Short Call";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }


    // SEPARATOR
    nTop = nTop + nItemHeight + 6;
    hCtl = CreateSuperLabel(
        HWND_NAVPANEL, -1,
        SuperLabelType::LineHorizontal,
        0, nTop, NAVPANEL_WIDTH, 10);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->BackColor = ThemeElement::NavPanelBack;
        pData->LineColor = ThemeElement::NavPanelSeparator;
        pData->LineWidth = 6;
        pData->MarginLeft = 10;
        pData->MarginRight = 10;
        SuperLabel_SetOptions(hCtl, pData);
    }


    nTop = nTop + 10;
    hCtl = CreateSuperLabel(
        HWND_NAVPANEL,
        IDC_NAVPANEL_TICKERTOTALS,
        SuperLabelType::TextOnly,
        0, nTop, NAVPANEL_WIDTH, nItemHeight);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = true;
        pData->AllowSelect = true;
        pData->SelectorColor = ThemeElement::TradesPanelBack;   // selector should be same color as middle panel
        pData->BackColor = ThemeElement::NavPanelBack;
        pData->BackColorHot = ThemeElement::NavPanelBackHot;
        pData->BackColorSelected = ThemeElement::NavPanelBackSelected;
        pData->TextColor = ThemeElement::NavPanelText;
        pData->TextColorHot = ThemeElement::NavPanelText;
        pData->TextOffsetLeft = nLeftOffset;
        pData->FontSize = 10;
        pData->FontSizeHot = 10;
        pData->wszText = L"Ticker Totals";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }


    nTop = nTop + nItemHeight;
    hCtl = CreateSuperLabel(
        HWND_NAVPANEL,
        IDC_NAVPANEL_DAILYTOTALS,
        SuperLabelType::TextOnly,
        0, nTop, NAVPANEL_WIDTH, nItemHeight);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = true;
        pData->AllowSelect = true;
        pData->SelectorColor = ThemeElement::TradesPanelBack;   // selector should be same color as middle panel
        pData->BackColor = ThemeElement::NavPanelBack;
        pData->BackColorHot = ThemeElement::NavPanelBackHot;
        pData->BackColorSelected = ThemeElement::NavPanelBackSelected;
        pData->TextColor = ThemeElement::NavPanelText;
        pData->TextColorHot = ThemeElement::NavPanelText;
        pData->TextOffsetLeft = nLeftOffset;
        pData->FontSize = 10;
        pData->FontSizeHot = 10;
        pData->wszText = L"Daily Totals";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }


    nTop = nTop + nItemHeight;
    hCtl = CreateSuperLabel(
        HWND_NAVPANEL,
        IDC_NAVPANEL_RECONCILE,
        SuperLabelType::TextOnly,
        0, nTop, NAVPANEL_WIDTH, nItemHeight);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = true;
        pData->AllowSelect = true;
        pData->SelectorColor = ThemeElement::TradesPanelBack;
        pData->BackColor = ThemeElement::NavPanelBack;
        pData->BackColorHot = ThemeElement::NavPanelBackHot;
        pData->BackColorSelected = ThemeElement::NavPanelBackSelected;
        pData->TextColor = ThemeElement::NavPanelText;
        pData->TextColorHot = ThemeElement::NavPanelText;
        pData->TextOffsetLeft = nLeftOffset;
        pData->FontSize = 10;
        pData->FontSizeHot = 10;
        pData->wszText = L"Reconcile";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }


    // SEPARATOR
    nTop = nTop + nItemHeight + 6;
    hCtl = CreateSuperLabel(
        HWND_NAVPANEL,
        -1, SuperLabelType::LineHorizontal,
        0, nTop, NAVPANEL_WIDTH, 10);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->BackColor = ThemeElement::NavPanelBack;
        pData->LineColor = ThemeElement::NavPanelSeparator;
        pData->LineWidth = 6;
        pData->MarginLeft = 10;
        pData->MarginRight = 10;
        SuperLabel_SetOptions(hCtl, pData);
    }


    nTop = nTop + 10;
    hCtl = CreateSuperLabel(
        HWND_NAVPANEL,
        IDC_NAVPANEL_CONFIGURE,
        SuperLabelType::TextOnly, 
        0, nTop, NAVPANEL_WIDTH, nItemHeight);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = true;
        pData->AllowSelect = true;
        pData->SelectorColor = ThemeElement::TradesPanelBack;
        pData->BackColor = ThemeElement::NavPanelBack;
        pData->BackColorHot = ThemeElement::NavPanelBackHot;
        pData->BackColorSelected = ThemeElement::NavPanelBackSelected;
        pData->TextColor = ThemeElement::NavPanelText;
        pData->TextColorHot = ThemeElement::NavPanelText;
        pData->TextOffsetLeft = nLeftOffset;
        pData->FontSize = 10;
        pData->FontSizeHot = 10;
        pData->wszText = L"Configuration";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }


    hCtl = CreateSuperLabel(
        HWND_NAVPANEL,
        IDC_NAVPANEL_GEARICON,
        SuperLabelType::ImageOnly,
        0, 0, 20, 20);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = true;
        pData->BackColor = ThemeElement::NavPanelBack;
        pData->BackColorHot = ThemeElement::NavPanelBack;
        pData->wszToolTip = L" Connect to TWS ";
        pData->ImageWidth = 20;
        pData->ImageHeight = 20;
        pData->pImage = LoadImageFromResource(pData->hInst, MAKEINTRESOURCE(IDB_GEAR), L"PNG");
        pData->pImageHot = LoadImageFromResource(pData->hInst, MAKEINTRESOURCE(IDB_GEARHOT), L"PNG");
        SuperLabel_SetOptions(hCtl, pData);
    }


    hCtl = CreateSuperLabel(
        HWND_NAVPANEL,
        IDC_NAVPANEL_MESSAGES,
        SuperLabelType::TextOnly,
        0, 0, NAVPANEL_WIDTH - 35, 18);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = false;
        pData->BackColor = ThemeElement::NavPanelBack;
        pData->TextColor = ThemeElement::NavPanelTextDim;
        pData->FontSize = 9;
        pData->FontSizeHot = pData->FontSize;
        pData->TextAlignment = SuperLabelAlignment::MiddleLeft;
        pData->wszText = L"TWS not connected";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }

}

