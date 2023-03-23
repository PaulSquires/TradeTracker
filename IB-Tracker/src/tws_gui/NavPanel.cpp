' ========================================================================================
'
' inControl - Accounting and Invoicing for Freelancers and Small Business
' Copyright (C) 2019-2020 Paul Squires, PlanetSquires Software
'
' ========================================================================================

enum NavBarMenu
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
type NAVBAR_TYPE
   hCtrl(NavBarMenu.Header_Logo to NavBarMenu.Accounting) as hwnd
   idSelected       as Long
   hWndSidePanel    as HWND
   hBottomSeparator as HWND
   hAppName         as HWND
end type
dim shared gNavBar as NAVBAR_TYPE


declare function frmSideFrame_PositionWindows() As LRESULT



' ========================================================================================
' frmNavBar Window procedure
' ========================================================================================
private Function frmNavBar_WndProc( ByVal HWnd   As HWnd, _
                                    ByVal uMsg   As UINT, _
                                    ByVal wParam As WPARAM, _
                                    ByVal lParam As LPARAM _
                                    ) As LRESULT

   Select Case uMsg
      
      case WM_SIZE
         ' Move the bottom separator and application name into place
         dim as long nLeft, nTop
         
         dim as RECT rcClient
         GetClientRect( hwnd, @rcClient )
         
         AfxSetWindowLocation( gNavBar.hBottomSeparator, 0, rcClient.bottom - AfxScaleY(50) )
         AfxSetWindowLocation( gNavBar.hAppName, 0, rcClient.bottom - AfxScaleY(30) )

      
      case MSG_SUPERLABEL_MOUSEMOVE

      
      case MSG_SUPERLABEL_CLICK
         dim as HWND hCtrl = cast(HWND, wParam)
         dim pData As SUPERLABEL_DATA Ptr 
         
         ' Determine if a menu item was clicked on or if a member
         ' of the header area was clicked on.
         dim as Boolean bIsHeaderControl, bIsMenuControl
         
         for i as long = NavBarMenu.Header_Logo to NavBarMenu.Header_Company
            if gNavBar.hCtrl(i) = hCtrl then
               bIsHeaderControl = true: exit for
            end if
         next
         for i as long = NavBarMenu.DashBoard to NavBarMenu.Accounting
            if gNavBar.hCtrl(i) = hCtrl then
               bIsMenuControl = true: exit for
            end if
         next
         
         if bIsHeaderControl then
            ' Activate the Gear icon
            frmSettingsMenu_Show( HWND_FRMNAVBAR )
            
         elseif bIsMenuControl then
            ' Set the selected menu item
            for i as long = NavBarMenu.DashBoard to NavBarMenu.Accounting
               pData = SuperLabel_GetOptions( gNavBar.hCtrl(i) )
               if pData then
                  ' default to not selected
                  pData->IsSelected = false
                  
                  ' if hwnd of control matches then select it
                  if gNavBar.hCtrl(i) = hCtrl then
                     pData->IsSelected = true
                     gNavBar.idSelected = pData->CtrlId
                  end if
                  
                  SuperLabel_SetOptions( gNavBar.hCtrl(i), pData )
                  AfxRedrawWindow( gNavBar.hCtrl(i) )
               end if   
            next
            frmSideFrame_PositionWindows()
         end if
         
   End Select

   ' for messages that we don't deal with
   Function = DefWindowProc( HWnd, uMsg, wParam, lParam )

End Function


' ========================================================================================
' frmNavBar_Show
' ========================================================================================
public Function frmNavBar_Show( ByVal hWndParent As HWnd ) as LRESULT

   '  Create the window and child controls
   Dim pWindow As CWindow Ptr = New CWindow
   pWindow->DPI = AfxCWindowPtr(hwndParent)->DPI
   
   HWND_FRMNAVBAR = _
         pWindow->Create( hWndParent, "", @frmNavBar_WndProc, 0, 0, NAVBAR_WIDTH, 0, _
         WS_CHILD Or WS_CLIPSIBLINGS Or WS_CLIPCHILDREN, _
         WS_EX_CONTROLPARENT Or WS_EX_LEFT Or WS_EX_LTRREADING Or WS_EX_RIGHTSCROLLBAR)

   ' Can only set the brush after the window is created
   dim as COLORREF nBackColor = BGR(13, 131, 221)
   dim as COLORREF nBackColorHot = BGR(20, 138, 228)  
   pWindow->Brush = CreateSolidBrush( nBackColor )
   
   dim as COLORREF nTextColor = BGR(255, 255, 255)
   dim as long nTop, nLeft, nLeftOffset 
   dim as long nItemHeight = 40
   dim as HWND hCtrl
   
   dim pData As SUPERLABEL_DATA Ptr 
   

   ' HEADER CONTROLS
   nLeft = (NAVBAR_WIDTH - 68) / 2
   hCtrl = SuperLabel( HWND_FRMNAVBAR, _
                       navBarMenu.Header_Logo, _
                       SuperLabelType.ImageOnly, _
                       "", nLeft, 20, 68, 68 )
   gNavBar.hCtrl(navBarMenu.Header_Logo) = hCtrl
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


   nLeft = NAVBAR_WIDTH - 40
   hCtrl = SuperLabel( HWND_FRMNAVBAR, _
                       navBarMenu.Header_Gear, _
                       SuperLabelType.ImageOnly, _
                       "", nLeft, 60, 24, 24 )
   gNavBar.hCtrl(navBarMenu.Header_Gear) = hCtrl
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


   hCtrl = SuperLabel( HWND_FRMNAVBAR, _
                       navBarMenu.Header_User, _
                       SuperLabelType.TextOnly, _
                       "Paul Squires", 0, 100, NAVBAR_WIDTH, 18 )
   gNavBar.hCtrl(navBarMenu.Header_User) = hCtrl
   pData = SuperLabel_GetOptions( hCtrl )
   if pData then
      pData->HotTestEnable   = true
      pData->BackColor       = nBackColor
      pData->BackColorHot    = nBackColor
      pData->TextColor       = BGR( 158, 205, 241 )
      pData->TextColorHot    = pData->TextColor
      pData->FontSize        = 10
      pData->FontWeight      = FW_MEDIUM
      pData->FontSizeHot     = pData->FontSize
      pData->FontWeightHot   = pData->FontWeight
      pData->TextAlignment   = SuperLabelAlignment.MiddleCenter
      SuperLabel_SetOptions( hCtrl, pData )
   end if

   hCtrl = SuperLabel( HWND_FRMNAVBAR, _
                       navBarMenu.Header_Company, _
                       SuperLabelType.TextOnly, _
                       "Insight Web Design", 0, 118, NAVBAR_WIDTH, 18 )
   gNavBar.hCtrl(navBarMenu.Header_Company) = hCtrl
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
   hCtrl = SuperLabel( HWND_FRMNAVBAR, -1, SuperLabelType.LineHorizontal, "", 0, 150, NAVBAR_WIDTH, 2 )
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
   hCtrl = SuperLabel( HWND_FRMNAVBAR, _
                       navBarMenu.DashBoard, _
                       SuperLabelType.TextOnly, _
                       "DashBoard", 0, nTop, NAVBAR_WIDTH, nItemHeight )
   gNavBar.hCtrl(navBarMenu.DashBoard) = hCtrl
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
   hCtrl = SuperLabel( HWND_FRMNAVBAR, _
                       navBarMenu.Clients, _
                       SuperLabelType.TextOnly, _
                       "Clients", 0, nTop, NAVBAR_WIDTH, nItemHeight )
   gNavBar.hCtrl(navBarMenu.Clients) = hCtrl
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
   hCtrl = SuperLabel( HWND_FRMNAVBAR, _
                       navBarMenu.Invoices, _
                       SuperLabelType.TextOnly, _
                       "Invoices", 0, nTop, NAVBAR_WIDTH, nItemHeight )
   gNavBar.hCtrl(navBarMenu.Invoices) = hCtrl
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
   hCtrl = SuperLabel( HWND_FRMNAVBAR, _
                       navBarMenu.Expenses, _
                       SuperLabelType.TextOnly, _
                       "Expenses", 0, nTop, NAVBAR_WIDTH, nItemHeight )
   gNavBar.hCtrl(navBarMenu.Expenses) = hCtrl
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
   hCtrl = SuperLabel( HWND_FRMNAVBAR, _
                       navBarMenu.Estimates, _
                       SuperLabelType.TextOnly, _
                       "Estimates", 0, nTop, NAVBAR_WIDTH, nItemHeight )
   gNavBar.hCtrl(navBarMenu.Estimates) = hCtrl
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
   hCtrl = SuperLabel( HWND_FRMNAVBAR, _
                       navBarMenu.TimeTracking, _
                       SuperLabelType.TextOnly, _
                       "Time Tracking", 0, nTop, NAVBAR_WIDTH, nItemHeight )
   gNavBar.hCtrl(navBarMenu.TimeTracking) = hCtrl
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
   hCtrl = SuperLabel( HWND_FRMNAVBAR, _
                       navBarMenu.Projects, _
                       SuperLabelType.TextOnly, _
                       "Projects", 0, nTop, NAVBAR_WIDTH, nItemHeight )
   gNavBar.hCtrl(navBarMenu.Projects) = hCtrl
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
   hCtrl = SuperLabel( HWND_FRMNAVBAR, _
                       navBarMenu.MyTeam, _
                       SuperLabelType.TextOnly, _
                       "My Team", 0, nTop, NAVBAR_WIDTH, nItemHeight )
   gNavBar.hCtrl(navBarMenu.MyTeam) = hCtrl
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
   hCtrl = SuperLabel( HWND_FRMNAVBAR, -1, SuperLabelType.LineHorizontal, "", 0, nTop, NAVBAR_WIDTH, 10 )
   pData = SuperLabel_GetOptions( hCtrl )
   if pData then
      pData->BackColor   = nBackColor
      pData->LineWidth   = 2
      pData->MarginLeft  = 10 
      pData->MarginRight = 10 
      SuperLabel_SetOptions( hCtrl, pData )
   end if


   nTop = nTop + 16
   hCtrl = SuperLabel( HWND_FRMNAVBAR, _
                       NavBarMenu.Reports, _
                       SuperLabelType.TextOnly, _
                       "", 0, nTop, NAVBAR_WIDTH, nItemHeight )
   gNavBar.hCtrl(NavBarMenu.Reports) = hCtrl
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
      pData->wszText         = "Reports"
      pData->wszTextHot      = *pData->wszText
      SuperLabel_SetOptions( hCtrl, pData )
   end if


   nTop = nTop + nItemHeight
   hCtrl = SuperLabel( HWND_FRMNAVBAR, _
                       NavBarMenu.Accounting, _
                       SuperLabelType.TextOnly, _
                       "", 0, nTop, NAVBAR_WIDTH, nItemHeight )
   gNavBar.hCtrl(NavBarMenu.Accounting) = hCtrl
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
      pData->wszText         = "Accounting"
      pData->wszTextHot      = *pData->wszText
      SuperLabel_SetOptions( hCtrl, pData )
   end if


   ' SEPARATOR
   hCtrl = SuperLabel( HWND_FRMNAVBAR, -1, SuperLabelType.LineHorizontal, "", 0, 0, NAVBAR_WIDTH, 10 )
   gNavBar.hBottomSeparator = hCtrl
   pData = SuperLabel_GetOptions( hCtrl )
   if pData then
      pData->BackColor   = nBackColor
      pData->LineWidth   = 2
      pData->MarginLeft  = 10 
      pData->MarginRight = 10 
      SuperLabel_SetOptions( hCtrl, pData )
   end if

   hCtrl = SuperLabel( HWND_FRMNAVBAR, _
                       navBarMenu.Header_User, _
                       SuperLabelType.TextOnly, _
                       "inControl Software v1.0", 0, 0, NAVBAR_WIDTH, 18 )
   gNavBar.hAppName = hCtrl
   pData = SuperLabel_GetOptions( hCtrl )
   if pData then
      pData->HotTestEnable   = false
      pData->BackColor       = nBackColor
      pData->BackColorHot    = nBackColor
      pData->TextColor       = BGR( 158, 205, 241 )
      pData->TextColorHot    = pData->TextColor
      pData->FontSize        = 9
      pData->FontWeight      = FW_MEDIUM
      pData->FontSizeHot     = pData->FontSize
      pData->FontWeightHot   = pData->FontWeight
      pData->TextAlignment   = SuperLabelAlignment.MiddleCenter
      SuperLabel_SetOptions( hCtrl, pData )
   end if

   UpdateWindow( pWindow->hWindow )

   function = 0

End Function
