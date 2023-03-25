
#include "framework.h"
#include "NavPanel.h"
#include "tws-client.h"
#include "SuperLabel.h"
#include "Themes.h"


//' ========================================================================================
//' NavPanel Window procedure
//' ========================================================================================
LRESULT CALLBACK NavPanel_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {

    case WM_SIZE:
    {
        // Move the bottom separator and application name into place, but only do so
        // if the vertical height of NavBar window has not become less than the minimum
        // otherwise the two controls will bleed into the ones above them.
        RECT rcClient;
        GetClientRect(hWnd, &rcClient);
        float MinHeight = AfxScaleY(645);

        if (rcClient.bottom < MinHeight) return 0;

        SetWindowPos(GetDlgItem(hWnd, IDC_NAVPANEL_BOTTOMSEP), 0,
            0, rcClient.bottom - (int)AfxScaleY(50), 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        SetWindowPos(GetDlgItem(hWnd, IDC_NAVPANEL_APPNAME), 0,
            0, rcClient.bottom - (int)AfxScaleY(30), 0, 0, SWP_NOSIZE | SWP_NOZORDER);
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
        graphics.FillRectangle(&backBrush, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom);

        EndPaint(hWnd, &ps);
        break;
    }


    case MSG_SUPERLABEL_MOUSEMOVE:
        break;

      
    case MSG_SUPERLABEL_CLICK:
    {   
        HWND hCtl = (HWND)lParam;
        int CtrlId = (int)wParam;

        if (hCtl == NULL) return 0;
        SuperLabel* pData = (SuperLabel*)GetWindowLongPtr(hWnd, 0);

        if (pData) {

            switch (CtrlId) {
            
            case IDC_NAVPANEL_GEARICON:
            {
                bool res = tws_connect();
                printf("Connect: %ld\n", res);
                printf("isConnected: %ld\n", tws_isConnected());
                break;
            }

            case IDC_NAVPANEL_LIGHTTHEME:
            {
                SetTheme(Themes::Light);
                ApplyActiveTheme();
                break;
            }

            case IDC_NAVPANEL_DARKTHEME:
            {
                SetTheme(Themes::Dark);
                ApplyActiveTheme();
                break;
            }

            case IDC_NAVPANEL_BLUETHEME:
            {
                SetTheme(Themes::Blue);
                ApplyActiveTheme();
                break;
            }

            case IDC_NAVPANEL_REDTHEME:
            {
                SetTheme(Themes::Red);
                ApplyActiveTheme();
                break;
            }

            case IDC_NAVPANEL_YELLOWTHEME:
            {
                SetTheme(Themes::Yellow);
                ApplyActiveTheme();
                break;
            }


            }

        }

    }
    break;


    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;

}


//' ========================================================================================
//' NavPanel_Show
//' ========================================================================================
CWindow* NavPanel_Show(HWND hWndParent)
{
    // Create the window and child controls
    CWindow* pWindow = new CWindow;
   
    HWND HWND_FRMNAVPANEL =
        pWindow->Create(hWndParent, L"", & NavPanel_WndProc, 0, 0, NAVPANEL_WIDTH, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
            WS_EX_CONTROLPARENT | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);
    
    // This is a child window of the main application parent so treat it like child
    // control and assign it a ControlID.
    SetWindowLongPtr(HWND_FRMNAVPANEL, GWLP_ID, IDC_FRMNAVPANEL);

    // Can only set the brush after the window is created
    DWORD nBackColor = GetThemeColor(ThemeElement::NavPanelBack);
    DWORD nBackColorHot = GetThemeColor(ThemeElement::NavPanelBackHot);

    COLORREF clrBack = GetThemeCOLORREF(ThemeElement::NavPanelBack);
    pWindow->SetBrush(GetStockBrush(NULL_BRUSH));

    DWORD nTextColor = GetThemeColor(ThemeElement::NavPanelText);
    DWORD nTextColorDim = GetThemeColor(ThemeElement::NavPanelTextDim);
    DWORD nSeparatorColor = GetThemeColor(ThemeElement::NavPanelSeparator);
    DWORD nBackColorSelected = GetThemeColor(ThemeElement::NavPanelBackSelected);

    int nTop, nLeft, nLeftOffset;
    int nItemHeight = 40;
        
    HWND hCtl;

    SuperLabel* pData = nullptr;

   

   // HEADER CONTROLS
    nLeft = (NAVPANEL_WIDTH - 68) / 2;
    hCtl = CreateSuperLabel(
        HWND_FRMNAVPANEL,
        IDC_NAVPANEL_LOGO,
        SuperLabelType::ImageOnly,
        nLeft, 20, 68, 68);
   pData = SuperLabel_GetOptions(hCtl);
   if (pData) {
       pData->HotTestEnable = false;
       pData->BackColor = nBackColor;
       pData->BackColorHot = nBackColor;
       pData->ImageWidth = 68;
       pData->ImageHeight = 68;
       pData->pImage = LoadImageFromResource(pData->hInst, MAKEINTRESOURCE(IDB_LOGO), L"PNG");
       pData->pImageHot = LoadImageFromResource(pData->hInst, MAKEINTRESOURCE(IDB_LOGO), L"PNG");
       SuperLabel_SetOptions(hCtl, pData);
   }


   nLeft = NAVPANEL_WIDTH - 40;
   hCtl = CreateSuperLabel(
       HWND_FRMNAVPANEL,
       IDC_NAVPANEL_GEARICON,
       SuperLabelType::ImageOnly,
       nLeft, 60, 20, 20);
   pData = SuperLabel_GetOptions(hCtl);
   if (pData) {
       pData->HotTestEnable = true;
       pData->BackColor = nBackColor;
       pData->BackColorHot = nBackColor;
       pData->wszToolTip = L"Connect to TWS";
       pData->ImageWidth = 20;
       pData->ImageHeight = 20;
       pData->pImage = LoadImageFromResource(pData->hInst, MAKEINTRESOURCE(IDB_GEAR), L"PNG");
       pData->pImageHot = LoadImageFromResource(pData->hInst, MAKEINTRESOURCE(IDB_GEARHOT), L"PNG");
       SuperLabel_SetOptions(hCtl, pData);
   }

   
    hCtl = CreateSuperLabel(
        HWND_FRMNAVPANEL,
        IDC_NAVPANEL_USERNAME,
        SuperLabelType::TextOnly,
        0, 100, NAVPANEL_WIDTH, 18);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = false;
        pData->BackColor = nBackColor;
        pData->BackColorHot = nBackColor;
        pData->TextColor = nTextColorDim;
        pData->TextColorHot = pData->TextColor;
        pData->FontSize = 10;
        pData->FontSizeHot = pData->FontSize;
        pData->TextAlignment = SuperLabelAlignment::MiddleCenter;
        pData->wszText = L"Paul Squires";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }


    hCtl = CreateSuperLabel(
        HWND_FRMNAVPANEL, 
        IDC_NAVPANEL_COMPANY,
        SuperLabelType::TextOnly, 
        0, 118, NAVPANEL_WIDTH, 18);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = false;
        pData->BackColor = nBackColor;
        pData->BackColorHot = nBackColor;
        pData->TextColor = nTextColor;
        pData->TextColorHot = pData->TextColor;
        pData->FontSize = 10;
        pData->FontSizeHot = pData->FontSize;
        pData->TextAlignment = SuperLabelAlignment::MiddleCenter;
        pData->wszText = L"Insight Web Design";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }
      
   
    // SEPARATOR
    hCtl = CreateSuperLabel(
        HWND_FRMNAVPANEL, -1, 
        SuperLabelType::LineHorizontal, 
        0, 150, NAVPANEL_WIDTH, 10);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->BackColor = nBackColor;
        pData->LineColor = nSeparatorColor;
        pData->LineColorHot = pData->LineColor;
        pData->LineWidth = 6;
        pData->MarginLeft = 10;
        pData->MarginRight = 10;
        SuperLabel_SetOptions(hCtl, pData);
    }

   
    // MENU ITEMS
    nLeftOffset = 0;
    nTop = 155;
    hCtl = CreateSuperLabel(
        HWND_FRMNAVPANEL,
        IDC_NAVPANEL_LIGHTTHEME,
        SuperLabelType::TextOnly,
        0, nTop, NAVPANEL_WIDTH, nItemHeight);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = true;
        pData->AllowSelect = true;
        pData->BackColor = nBackColor;
        pData->BackColorHot = nBackColorHot;
        pData->BackColorSelected = nBackColorSelected;
        pData->TextColor = nTextColor;
        pData->TextColorHot = nTextColor;
        pData->TextOffsetLeft = nLeftOffset;
        pData->FontSize = 11;
        pData->FontSizeHot = 11;
        pData->wszText = L"Light Theme";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }
   
   
    nTop = nTop + nItemHeight;
    hCtl = CreateSuperLabel(
        HWND_FRMNAVPANEL,
        IDC_NAVPANEL_DARKTHEME,
        SuperLabelType::TextOnly,
        0, nTop, NAVPANEL_WIDTH, nItemHeight);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = true;
        pData->AllowSelect = true;
        pData->BackColor = nBackColor;
        pData->BackColorHot = nBackColorHot;
        pData->BackColorSelected = nBackColorSelected;
        pData->TextColor = nTextColor;
        pData->TextColorHot = nTextColor;
        pData->TextOffsetLeft = nLeftOffset;
        pData->FontSize = 11;
        pData->FontSizeHot = 11;
        pData->wszText = L"Dark Theme";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }


    nTop = nTop + nItemHeight;
    hCtl = CreateSuperLabel(
        HWND_FRMNAVPANEL,
        IDC_NAVPANEL_BLUETHEME,
        SuperLabelType::TextOnly,
        0, nTop, NAVPANEL_WIDTH, nItemHeight);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = true;
        pData->AllowSelect = true;
        pData->BackColor = nBackColor;
        pData->BackColorHot = nBackColorHot;
        pData->BackColorSelected = nBackColorSelected;
        pData->TextColor = nTextColor;
        pData->TextColorHot = nTextColor;
        pData->TextOffsetLeft = nLeftOffset;
        pData->FontSize = 11;
        pData->FontSizeHot = 11;
        pData->wszText = L"Blue Theme";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }


    nTop = nTop + nItemHeight;
    hCtl = CreateSuperLabel(
        HWND_FRMNAVPANEL,
        IDC_NAVPANEL_REDTHEME,
        SuperLabelType::TextOnly,
        0, nTop, NAVPANEL_WIDTH, nItemHeight);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = true;
        pData->AllowSelect = true;
        pData->BackColor = nBackColor;
        pData->BackColorHot = nBackColorHot;
        pData->BackColorSelected = nBackColorSelected;
        pData->TextColor = nTextColor;
        pData->TextColorHot = nTextColor;
        pData->TextOffsetLeft = nLeftOffset;
        pData->FontSize = 11;
        pData->FontSizeHot = 11;
        pData->wszText = L"Red Theme";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }


    nTop = nTop + nItemHeight;
    hCtl = CreateSuperLabel(
        HWND_FRMNAVPANEL,
        IDC_NAVPANEL_YELLOWTHEME,
        SuperLabelType::TextOnly,
        0, nTop, NAVPANEL_WIDTH, nItemHeight);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = true;
        pData->AllowSelect = true;
        pData->BackColor = nBackColor;
        pData->BackColorHot = nBackColorHot;
        pData->BackColorSelected = nBackColorSelected;
        pData->TextColor = nTextColor;
        pData->TextColorHot = nTextColor;
        pData->TextOffsetLeft = nLeftOffset;
        pData->FontSize = 11;
        pData->FontSizeHot = 11;
        pData->wszText = L"Yellow Theme";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }


    nTop = nTop + nItemHeight;
    hCtl = CreateSuperLabel(
        HWND_FRMNAVPANEL,
        IDC_NAVPANEL_TIMETRACKING,
        SuperLabelType::TextOnly,
        0, nTop, NAVPANEL_WIDTH, nItemHeight);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = true;
        pData->AllowSelect = true;
        pData->BackColor = nBackColor;
        pData->BackColorHot = nBackColorHot;
        pData->TextColor = nTextColor;
        pData->TextColorHot = nTextColor;
        pData->BackColorSelected = nBackColorSelected;
        pData->TextOffsetLeft = nLeftOffset;
        pData->FontSize = 11;
        pData->FontSizeHot = 11;
        pData->wszText = L"Time Tracking";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }


    nTop = nTop + nItemHeight;
    hCtl = CreateSuperLabel(
        HWND_FRMNAVPANEL,
        IDC_NAVPANEL_PROJECTS,
        SuperLabelType::TextOnly,
        0, nTop, NAVPANEL_WIDTH, nItemHeight);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = true;
        pData->AllowSelect = true;
        pData->BackColor = nBackColor;
        pData->BackColorHot = nBackColorHot;
        pData->TextColor = nTextColor;
        pData->TextColorHot = nTextColor;
        pData->BackColorSelected = nBackColorSelected;
        pData->TextOffsetLeft = nLeftOffset;
        pData->FontSize = 11;
        pData->FontSizeHot = 11;
        pData->wszText = L"Projects";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }


    nTop = nTop + nItemHeight;
    hCtl = CreateSuperLabel(
        HWND_FRMNAVPANEL,
        IDC_NAVPANEL_MYTEAM,
        SuperLabelType::TextOnly,
        0, nTop, NAVPANEL_WIDTH, nItemHeight);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = true;
        pData->AllowSelect = true;
        pData->BackColor = nBackColor;
        pData->BackColorHot = nBackColorHot;
        pData->TextColor = nTextColor;
        pData->TextColorHot = nTextColor;
        pData->BackColorSelected = nBackColorSelected;
        pData->TextOffsetLeft = nLeftOffset;
        pData->FontSize = 11;
        pData->FontSizeHot = 11;
        pData->wszText = L"My Team";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }


    // SEPARATOR
    nTop = nTop + nItemHeight + 6;
    hCtl = CreateSuperLabel(
        HWND_FRMNAVPANEL,
        -1, SuperLabelType::LineHorizontal,
        0, nTop, NAVPANEL_WIDTH, 10);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->BackColor = nBackColor;
        pData->LineColor = nSeparatorColor;
        pData->LineColorHot = pData->LineColor;
        pData->LineWidth = 6;
        pData->MarginLeft = 10;
        pData->MarginRight = 10;
        SuperLabel_SetOptions(hCtl, pData);
    }


    nTop = nTop + 16;
    hCtl = CreateSuperLabel(
        HWND_FRMNAVPANEL,
        IDC_NAVPANEL_REPORTS,
        SuperLabelType::TextOnly,
        0, nTop, NAVPANEL_WIDTH, nItemHeight);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = true;
        pData->AllowSelect = true;
        pData->BackColor = nBackColor;
        pData->BackColorHot = nBackColorHot;
        pData->TextColor = nTextColor;
        pData->TextColorHot = nTextColor;
        pData->BackColorSelected = nBackColorSelected;
        pData->TextOffsetLeft = nLeftOffset;
        pData->FontSize = 11;
        pData->FontSizeHot = 11;
        pData->wszText = L"Reports";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }


    nTop = nTop + nItemHeight;
    hCtl = CreateSuperLabel(
        HWND_FRMNAVPANEL,
        IDC_NAVPANEL_ACCOUNTING,
        SuperLabelType::TextOnly, 
        0, nTop, NAVPANEL_WIDTH, nItemHeight);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = true;
        pData->AllowSelect = true;
        pData->BackColor = nBackColor;
        pData->BackColorHot = nBackColorHot;
        pData->BackColorSelected = nBackColorSelected;
        pData->TextColor = nTextColor;
        pData->TextColorHot = nTextColor;
        pData->TextOffsetLeft = nLeftOffset;
        pData->FontSize = 11;
        pData->FontSizeHot = 11;
        pData->wszText = L"Accounting";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }


    // SEPARATOR
    hCtl = CreateSuperLabel(
        HWND_FRMNAVPANEL, 
        IDC_NAVPANEL_BOTTOMSEP,
        SuperLabelType::LineHorizontal, 
        0, 0, NAVPANEL_WIDTH, 10);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->BackColor = nBackColor;
        pData->LineColor = nSeparatorColor;
        pData->LineColorHot = pData->LineColor;
        pData->LineWidth = 6;
        pData->MarginLeft = 10;
        pData->MarginRight = 10; 
        SuperLabel_SetOptions(hCtl, pData);
    }


    hCtl = CreateSuperLabel(
        HWND_FRMNAVPANEL,
        IDC_NAVPANEL_APPNAME,
        SuperLabelType::TextOnly,
        0, 0, NAVPANEL_WIDTH, 18);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = false;
        pData->BackColor = nBackColor;
        pData->BackColorHot = nBackColor;
        pData->TextColor = nTextColorDim;
        pData->TextColorHot = pData->TextColor;
        pData->FontSize = 9;
        pData->FontSizeHot = pData->FontSize;
        pData->TextAlignment = SuperLabelAlignment::MiddleCenter;
        pData->wszText = L"inControl Software v1.0";
        pData->wszTextHot = pData->wszText;
        SuperLabel_SetOptions(hCtl, pData);
    }

    return pWindow;

}

