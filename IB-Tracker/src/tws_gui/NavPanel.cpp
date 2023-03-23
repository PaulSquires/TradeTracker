
#include "framework.h"
#include "NavPanel.h"


/*
enum NavPanelMenu
   Header_Logo    = 100
   Header_Gear
   Header_User
   Header_Company       
   DashBoard
   Clients
   Invoices
   Expenses
   Estimates
   TimeTracking
   Projects
   MyTeam
   Reports
   Accounting
end enum

' Track the menu items group and selection
type NavPanel_TYPE
   hCtrl(NavPanelMenu.Header_Logo to NavPanelMenu.Accounting) as hwnd
   idSelected       as Long
   hWndSidePanel    as HWND
   hBottomSeparator as HWND
   hAppName         as HWND
end type
dim shared gNavPanel as NavPanel_TYPE


declare function frmSideFrame_PositionWindows() As LRESULT

*/



//' ========================================================================================
//' NavPanel Window procedure
//' ========================================================================================
LRESULT CALLBACK NavPanel_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {

    case WM_CREATE:
    break;


    case WM_COMMAND:
    break;


    case WM_SIZE:
         // Move the bottom separator and application name into place
         //dim as long nLeft, nTop
         //
         //dim as RECT rcClient
         //GetClientRect( hwnd, @rcClient )
         //
         //AfxSetWindowLocation( gNavPanel.hBottomSeparator, 0, rcClient.bottom - AfxScaleY(50) )
         //AfxSetWindowLocation( gNavPanel.hAppName, 0, rcClient.bottom - AfxScaleY(30) )

        break;

    case MSG_SUPERLABEL_MOUSEMOVE:
        break;

      
    case MSG_SUPERLABEL_CLICK:
         //dim as HWND hCtrl = cast(HWND, wParam)
         //dim pData As SUPERLABEL_DATA Ptr 
        //SUPERLABEL_DATA* pData = nullptr;

        //    pData = (SUPERLABEL_DATA*)GetWindowLongPtr(hWnd, 0);


    case WM_DESTROY:
        break;

    case WM_NCDESTROY:
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
        pWindow->Create(hWndParent, L"", &NavPanel_WndProc, 0, 0, NAVPANEL_WIDTH, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
            WS_EX_CONTROLPARENT | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);
    
    // This is a child window of the main application parent so treat it like child
    // control and assign it a ControlID.
    SetWindowLongPtr(HWND_FRMNAVPANEL, GWLP_ID, IDC_FRMNAVPANEL);

    // Can only set the brush after the window is created
    DWORD nBackColor = Color::MakeARGB(255, 13, 131, 221);
    DWORD nBackColorHot = Color::MakeARGB(255, 20, 138, 228);
        //pWindow->Brush = CreateSolidBrush( nBackColor )

    DWORD nTextColor = Color::MakeARGB(255, 255, 255, 255);
    //int nTop, nLeft, nLeftOffset;
    //int nItemHeight = 40;
        
    HWND hCtl;

    SUPERLABEL_DATA* pData = nullptr;

   

/*
   ' HEADER CONTROLS
   nLeft = (NAVPANEL_WIDTH - 68) / 2
   hCtrl = SuperLabel( HWND_FRMNAVPANEL, _
                       IDC_NavPanel_LOGO, _
                       SuperLabelType.ImageOnly, _
                       "", nLeft, 20, 68, 68 )
   gNavPanel.hCtrl(NavPanelMenu.Header_Logo) = hCtrl
   pData = SuperLabel_GetOptions( hCtrl )
   if pData then
      pData->HotTestEnable   = true
      pData->BackColor       = nBackColor
      pData->BackColorHot    = nBackColor
      pData->ImageWidth      = 68
      pData->ImageHeight     = 68
      pData->wszImage        = "IMAGE_LOGO"
      pData->wszImageHot     = "IMAGE_LOGO"
      SuperLabel_SetOptions( hCtrl, pData )
   end if


   nLeft = NAVPANEL_WIDTH - 40
   hCtrl = SuperLabel( HWND_FRMNAVPANEL, _
                       IDC_NavPanel_GEARICON, _
                       SuperLabelType.ImageOnly, _
                       "", nLeft, 60, 24, 24 )
   gNavPanel.hCtrl(NavPanelMenu.Header_Gear) = hCtrl
   pData = SuperLabel_GetOptions( hCtrl )
   if pData then
      pData->HotTestEnable   = true
      pData->BackColor       = nBackColor
      pData->BackColorHot    = nBackColor
      pData->wszToolTip      = "Options"
      pData->ImageWidth      = 20
      pData->ImageHeight     = 20
      pData->wszImage        = "IMAGE_GEAR"
      pData->wszImageHot     = "IMAGE_GEARHOT"
      SuperLabel_SetOptions( hCtrl, pData )
   end if
   */


    hCtl = CreateSuperLabel(
        HWND_FRMNAVPANEL,
        IDC_NAVPANEL_USERNAME,
        SuperLabelType::TextOnly,
        L"Paul Squires", 0, 100, NAVPANEL_WIDTH, 18);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = true;
        pData->BackColor = nBackColor;
        pData->BackColorHot = nBackColor;
        pData->TextColor = Color::MakeARGB(255, 158, 205, 241);
        pData->TextColorHot = pData->TextColor;
        pData->FontSize = 10;
        pData->FontSizeHot = pData->FontSize;
        pData->TextAlignment = SuperLabelAlignment::MiddleCenter;
        SuperLabel_SetOptions(hCtl, pData);
    }

    return pWindow;

}





/*
hCtrl = SuperLabel( HWND_FRMNAVPANEL, _
                       NavPanelMenu.Header_Company, _
                       SuperLabelType.TextOnly, _
                       "Insight Web Design", 0, 118, NAVPANEL_WIDTH, 18 )
   gNavPanel.hCtrl(NavPanelMenu.Header_Company) = hCtrl
   pData = SuperLabel_GetOptions( hCtrl )
   if pData then
      pData->HotTestEnable   = true
      pData->BackColor       = nBackColor
      pData->BackColorHot    = nBackColor
      pData->TextColor       = nTextColor
      pData->TextColorHot    = pData->TextColor
      pData->FontSize        = 10
      pData->FontWeight      = FW_MEDIUM
      pData->FontSizeHot     = pData->FontSize
      pData->FontWeightHot   = pData->FontWeight
      pData->TextAlignment   = SuperLabelAlignment.MiddleCenter
      SuperLabel_SetOptions( hCtrl, pData )
   end if
   
   
   
   ' SEPARATOR
   hCtrl = SuperLabel( HWND_FRMNAVPANEL, -1, SuperLabelType.LineHorizontal, "", 0, 150, NAVPANEL_WIDTH, 2 )
   pData = SuperLabel_GetOptions( hCtrl )
   if pData then
      pData->BackColor   = nBackColor
      pData->LineWidth   = 2
      pData->MarginLeft  = 10 
      pData->MarginRight = 10 
      SuperLabel_SetOptions( hCtrl, pData )
   end if

   
   ' MENU ITEMS
   nLeftOffset = 20
   nTop = 155
   hCtrl = SuperLabel( HWND_FRMNAVPANEL, _
                       NavPanelMenu.DashBoard, _
                       SuperLabelType.TextOnly, _
                       "DashBoard", 0, nTop, NAVPANEL_WIDTH, nItemHeight )
   gNavPanel.hCtrl(NavPanelMenu.DashBoard) = hCtrl
   pData = SuperLabel_GetOptions( hCtrl )
   if pData then
      pData->HotTestEnable   = true
      pData->SelectionMode   = true
      pData->BackColor       = nBackColor
      pData->BackColorHot    = nBackColorHot 
      pData->TextColor       = nTextColor
      pData->TextColorHot    = nTextColor
      pData->TextOffsetLeft  = nLeftOffset
      pData->FontSize        = 11
      pData->FontWeight      = FW_MEDIUM
      pData->FontSizeHot     = 11
      pData->FontWeightHot   = FW_MEDIUM
      pData->wszText         = "DashBoard"
      pData->wszTextHot      = *pData->wszText
      SuperLabel_SetOptions( hCtrl, pData )
   end if
   

   nTop = nTop + nItemHeight
   hCtrl = SuperLabel( HWND_FRMNAVPANEL, _
                       NavPanelMenu.Clients, _
                       SuperLabelType.TextOnly, _
                       "Clients", 0, nTop, NAVPANEL_WIDTH, nItemHeight )
   gNavPanel.hCtrl(NavPanelMenu.Clients) = hCtrl
   pData = SuperLabel_GetOptions( hCtrl )
   if pData then
      pData->HotTestEnable   = true
      pData->SelectionMode   = true
      pData->BackColor       = nBackColor
      pData->BackColorHot    = nBackColorHot
      pData->TextColor       = nTextColor
      pData->TextColorHot    = nTextColor
      pData->TextOffsetLeft  = nLeftOffset
      pData->FontSize        = 11
      pData->FontWeight      = FW_MEDIUM
      pData->FontSizeHot     = 11
      pData->FontWeightHot   = FW_MEDIUM
      pData->wszText         = "Clients"
      pData->wszTextHot      = *pData->wszText
      SuperLabel_SetOptions( hCtrl, pData )
   end if


   nTop = nTop + nItemHeight
   hCtrl = SuperLabel( HWND_FRMNAVPANEL, _
                       NavPanelMenu.Invoices, _
                       SuperLabelType.TextOnly, _
                       "Invoices", 0, nTop, NAVPANEL_WIDTH, nItemHeight )
   gNavPanel.hCtrl(NavPanelMenu.Invoices) = hCtrl
   pData = SuperLabel_GetOptions( hCtrl )
   if pData then
      pData->HotTestEnable   = true
      pData->SelectionMode   = true
      pData->BackColor       = nBackColor
      pData->BackColorHot    = nBackColorHot
      pData->TextColor       = nTextColor
      pData->TextColorHot    = nTextColor
      pData->TextOffsetLeft  = nLeftOffset
      pData->FontSize        = 11
      pData->FontWeight      = FW_MEDIUM
      pData->FontSizeHot     = 11
      pData->FontWeightHot   = FW_MEDIUM
      pData->wszText         = "Invoices"
      pData->wszTextHot      = *pData->wszText
      SuperLabel_SetOptions( hCtrl, pData )
   end if


   nTop = nTop + nItemHeight
   hCtrl = SuperLabel( HWND_FRMNAVPANEL, _
                       NavPanelMenu.Expenses, _
                       SuperLabelType.TextOnly, _
                       "Expenses", 0, nTop, NAVPANEL_WIDTH, nItemHeight )
   gNavPanel.hCtrl(NavPanelMenu.Expenses) = hCtrl
   pData = SuperLabel_GetOptions( hCtrl )
   if pData then
      pData->HotTestEnable   = true
      pData->SelectionMode   = true
      pData->BackColor       = nBackColor
      pData->BackColorHot    = nBackColorHot
      pData->TextColor       = nTextColor
      pData->TextColorHot    = nTextColor
      pData->TextOffsetLeft  = nLeftOffset
      pData->FontSize        = 11
      pData->FontWeight      = FW_MEDIUM
      pData->FontSizeHot     = 11
      pData->FontWeightHot   = FW_MEDIUM
      pData->wszText         = "Expenses"
      pData->wszTextHot      = *pData->wszText
      SuperLabel_SetOptions( hCtrl, pData )
   end if


   nTop = nTop + nItemHeight
   hCtrl = SuperLabel( HWND_FRMNAVPANEL, _
                       NavPanelMenu.Estimates, _
                       SuperLabelType.TextOnly, _
                       "Estimates", 0, nTop, NAVPANEL_WIDTH, nItemHeight )
   gNavPanel.hCtrl(NavPanelMenu.Estimates) = hCtrl
   pData = SuperLabel_GetOptions( hCtrl )
   if pData then
      pData->HotTestEnable   = true
      pData->SelectionMode   = true
      pData->BackColor       = nBackColor
      pData->BackColorHot    = nBackColorHot
      pData->TextColor       = nTextColor
      pData->TextColorHot    = nTextColor
      pData->TextOffsetLeft  = nLeftOffset
      pData->FontSize        = 11
      pData->FontWeight      = FW_MEDIUM
      pData->FontSizeHot     = 11
      pData->FontWeightHot   = FW_MEDIUM
      pData->wszText         = "Estimates"
      pData->wszTextHot      = *pData->wszText
      SuperLabel_SetOptions( hCtrl, pData )
   end if


   nTop = nTop + nItemHeight
   hCtrl = SuperLabel( HWND_FRMNAVPANEL, _
                       NavPanelMenu.TimeTracking, _
                       SuperLabelType.TextOnly, _
                       "Time Tracking", 0, nTop, NAVPANEL_WIDTH, nItemHeight )
   gNavPanel.hCtrl(NavPanelMenu.TimeTracking) = hCtrl
   pData = SuperLabel_GetOptions( hCtrl )
   if pData then
      pData->HotTestEnable   = true
      pData->SelectionMode   = true
      pData->BackColor       = nBackColor
      pData->BackColorHot    = nBackColorHot
      pData->TextColor       = nTextColor
      pData->TextColorHot    = nTextColor
      pData->TextOffsetLeft  = nLeftOffset
      pData->FontSize        = 11
      pData->FontWeight      = FW_MEDIUM
      pData->FontSizeHot     = 11
      pData->FontWeightHot   = FW_MEDIUM
      pData->wszText         = "Time Tracking"
      pData->wszTextHot      = *pData->wszText
      SuperLabel_SetOptions( hCtrl, pData )
   end if


   nTop = nTop + nItemHeight
   hCtrl = SuperLabel( HWND_FRMNAVPANEL, _
                       NavPanelMenu.Projects, _
                       SuperLabelType.TextOnly, _
                       "Projects", 0, nTop, NAVPANEL_WIDTH, nItemHeight )
   gNavPanel.hCtrl(NavPanelMenu.Projects) = hCtrl
   pData = SuperLabel_GetOptions( hCtrl )
   if pData then
      pData->HotTestEnable   = true
      pData->SelectionMode   = true
      pData->BackColor       = nBackColor
      pData->BackColorHot    = nBackColorHot
      pData->TextColor       = nTextColor
      pData->TextColorHot    = nTextColor
      pData->TextOffsetLeft  = nLeftOffset
      pData->FontSize        = 11
      pData->FontWeight      = FW_MEDIUM
      pData->FontSizeHot     = 11
      pData->FontWeightHot   = FW_MEDIUM
      pData->wszText         = "Projects"
      pData->wszTextHot      = *pData->wszText
      SuperLabel_SetOptions( hCtrl, pData )
   end if


   nTop = nTop + nItemHeight
   hCtrl = SuperLabel( HWND_FRMNAVPANEL, _
                       NavPanelMenu.MyTeam, _
                       SuperLabelType.TextOnly, _
                       "My Team", 0, nTop, NAVPANEL_WIDTH, nItemHeight )
   gNavPanel.hCtrl(NavPanelMenu.MyTeam) = hCtrl
   pData = SuperLabel_GetOptions( hCtrl )
   if pData then
      pData->HotTestEnable   = true
      pData->SelectionMode   = true
      pData->BackColor       = nBackColor
      pData->BackColorHot    = nBackColorHot
      pData->TextColor       = nTextColor
      pData->TextColorHot    = nTextColor
      pData->TextOffsetLeft  = nLeftOffset
      pData->FontSize        = 11
      pData->FontWeight      = FW_MEDIUM
      pData->FontSizeHot     = 11
      pData->FontWeightHot   = FW_MEDIUM
      pData->wszText         = "My Team"
      pData->wszTextHot      = *pData->wszText
      SuperLabel_SetOptions( hCtrl, pData )
   end if


       ' SEPARATOR
       nTop = nTop + nItemHeight + 6
       hCtrl = SuperLabel(HWND_FRMNAVPANEL, -1, SuperLabelType.LineHorizontal, "", 0, nTop, NAVPANEL_WIDTH, 10)
       pData = SuperLabel_GetOptions(hCtrl)
       if pData then
           pData->BackColor = nBackColor
           pData->LineWidth = 2
           pData->MarginLeft = 10
           pData->MarginRight = 10
           SuperLabel_SetOptions(hCtrl, pData)
           end if


           nTop = nTop + 16
           hCtrl = SuperLabel(HWND_FRMNAVPANEL, _
               NavPanelMenu.Reports, _
               SuperLabelType.TextOnly, _
               "", 0, nTop, NAVPANEL_WIDTH, nItemHeight)
           gNavPanel.hCtrl(NavPanelMenu.Reports) = hCtrl
           pData = SuperLabel_GetOptions(hCtrl)
           if pData then
               pData->HotTestEnable = true
               pData->SelectionMode = true
               pData->BackColor = nBackColor
               pData->BackColorHot = nBackColorHot
               pData->TextColor = nTextColor
               pData->TextColorHot = nTextColor
               pData->TextOffsetLeft = nLeftOffset
               pData->FontSize = 11
               pData->FontWeight = FW_MEDIUM
               pData->FontSizeHot = 11
               pData->FontWeightHot = FW_MEDIUM
               pData->wszText = "Reports"
               pData->wszTextHot = *pData->wszText
               SuperLabel_SetOptions(hCtrl, pData)
               end if


               nTop = nTop + nItemHeight
               hCtrl = SuperLabel(HWND_FRMNAVPANEL, _
                   NavPanelMenu.Accounting, _
                   SuperLabelType.TextOnly, _
                   "", 0, nTop, NAVPANEL_WIDTH, nItemHeight)
               gNavPanel.hCtrl(NavPanelMenu.Accounting) = hCtrl
               pData = SuperLabel_GetOptions(hCtrl)
               if pData then
                   pData->HotTestEnable = true
                   pData->SelectionMode = true
                   pData->BackColor = nBackColor
                   pData->BackColorHot = nBackColorHot
                   pData->TextColor = nTextColor
                   pData->TextColorHot = nTextColor
                   pData->TextOffsetLeft = nLeftOffset
                   pData->FontSize = 11
                   pData->FontWeight = FW_MEDIUM
                   pData->FontSizeHot = 11
                   pData->FontWeightHot = FW_MEDIUM
                   pData->wszText = "Accounting"
                   pData->wszTextHot = *pData->wszText
                   SuperLabel_SetOptions(hCtrl, pData)
                   end if


                   ' SEPARATOR
                   hCtrl = SuperLabel(HWND_FRMNAVPANEL, -1, SuperLabelType.LineHorizontal, "", 0, 0, NAVPANEL_WIDTH, 10)
                   gNavPanel.hBottomSeparator = hCtrl
                   pData = SuperLabel_GetOptions(hCtrl)
                   if pData then
                       pData->BackColor = nBackColor
                       pData->LineWidth = 2
                       pData->MarginLeft = 10
                       pData->MarginRight = 10
                       SuperLabel_SetOptions(hCtrl, pData)
                       end if

                       hCtrl = SuperLabel(HWND_FRMNAVPANEL, _
                           NavPanelMenu.Header_User, _
                           SuperLabelType.TextOnly, _
                           "inControl Software v1.0", 0, 0, NAVPANEL_WIDTH, 18)
                       gNavPanel.hAppName = hCtrl
                       pData = SuperLabel_GetOptions(hCtrl)
                       if pData then
                           pData->HotTestEnable = false
                           pData->BackColor = nBackColor
                           pData->BackColorHot = nBackColor
                           pData->TextColor = BGR(158, 205, 241)
                           pData->TextColorHot = pData->TextColor
                           pData->FontSize = 9
                           pData->FontWeight = FW_MEDIUM
                           pData->FontSizeHot = pData->FontSize
                           pData->FontWeightHot = pData->FontWeight
                           pData->TextAlignment = SuperLabelAlignment.MiddleCenter
                           SuperLabel_SetOptions(hCtrl, pData)
                           end if

*/