//'
//' SUPER LABEL CONTROL
//' Copyright (C) 2023 Paul Squires, PlanetSquires Software
//' This control should be able to handle all different types of labels and will act
//' like a button, hot tracking, pictures, etc.
//'

#include "framework.h"
#include "CWindow.h"
#include "SuperLabel.h"


 
//'------------------------------------------------------------------------------ 
//bool SuperLabel_LoadImageFromResource(
//    HDC memDC,
//    std::wstring wszResourceName,
//    byval pImage as GpImage Ptr, _
//    byref rcDraw as RECT )
//{
//
//}
//   ' Loads an image from a resource file using GDI+
//   if pImage then 
//      GdipDisposeImage(pImage)
//      pImage = null
//   end if
//
//   IF LEN(wszResourceName) = 0 THEN RETURN E_INVALIDARG
//   dim hInstance as HINSTANCE = GetModuleHandle(NULL)
//   DIM hStatus AS LONG
//   ' // Find the resource and lock it
//   DIM hResource AS HRSRC = FindResource(hInstance, wszResourceName, CAST(LPCWSTR, RT_RCDATA))
//   IF hResource = NULL THEN RETURN E_INVALIDARG
//   DIM imageSize AS DWORD = SizeofResource(hInstance, hResource)
//   IF imageSize = 0 THEN RETURN E_INVALIDARG
//   DIM pResourceData AS LPVOID = LockResource(LoadResource(hInstance, hResource))
//   IF pResourceData = NULL THEN RETURN E_INVALIDARG
//   ' // Allocate memory to hold the image
//   DIM hGlobal AS HGLOBAL = GlobalAlloc(GMEM_MOVEABLE, imageSize)
//   IF hGlobal THEN
//      ' // Lock the memory
//      DIM pGlobalBuffer AS LPVOID = GlobalLock(hGlobal)
//      IF pGlobalBuffer THEN
//         ' // Copy the image from the resource file to global memory
//         memcpy pGlobalBuffer, pResourceData, imageSize
//         ' // Create an stream in global memory
//         DIM pImageStream AS LPSTREAM, pGraphics AS GpGraphics PTR
//         IF CreateStreamOnHGlobal(hGlobal, FALSE, @pImageStream) = S_OK THEN
//            ' // Create a bitmap from the data contained in the stream
//            hStatus = GdipCreateBitmapFromStream(pImageStream, cast(GpBitmap PTR PTR, @pImage))
//            IF hStatus = 0 THEN
//               ' // Creates a graphics object from it
//               hStatus = GdipCreateFromHDC(memDC, @pGraphics)
//               ' // Draws the image (required to keep it in memory, since we are
//               ' // going to unlock and free the resource)
//               IF pGraphics THEN 
//                  GdipSetInterpolationMode( pGraphics, InterpolationModeHighQualityBicubic )
//                  hStatus = _
//                     GdipDrawImageRectI( pGraphics, pImage, rcDraw.Left, rcDraw.Top, rcDraw.Right, rcDraw.Bottom )
//               end if
//               ' // Deletes the graphics object
//               IF pGraphics THEN GdipDeleteGraphics(pGraphics)
//            END IF
//            pImageStream->lpVtbl->Release(pImageStream)
//         END IF
//         ' // Unlock the memory
//         GlobalUnlock pGlobalBuffer
//      END IF
//      ' // Free the memory
//      GlobalFree hGlobal
//   END IF
//   FUNCTION = hStatus
//END FUNCTION


//'------------------------------------------------------------------------------ 
LRESULT CALLBACK SuperLabelProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    SUPERLABEL_DATA* pData = nullptr;

    if (uMsg != WM_CREATE) {
        pData = (SUPERLABEL_DATA*)GetWindowLongPtr(hWnd, 0);
    }
                 

    switch (uMsg)
    {

    case WM_CREATE:
    {
    }
    break;


    case WM_COMMAND:
    {
    }
    break;


    case WM_ERASEBKGND:
    {
        // Handle all of the painting in WM_PAINT
        return TRUE;
    }
    break;


    case WM_SETCURSOR:
    {
        if (pData == nullptr) return 0;
        if (pData->HotTestEnable) {
            SetCursor(LoadCursor(NULL, (LPCWSTR)IDC_HAND));
        }
        return 0;
    }
    break;


    case WM_MOUSEMOVE:
    {
        if (pData == nullptr) return 0;
        SendMessage(pData->hParent, MSG_SUPERLABEL_MOUSEMOVE, (WPARAM)hWnd, 0);

        // Tracks the mouse movement and stores the hot state

        TrackMouseEvent trackMouse;

        if (GetProp(hWnd, L"HOT") == 0) {
            trackMouse.cbSize = sizeof(trackMouse);
            trackMouse.dwFlags = TME_LEAVE;
            trackMouse.hwndTrack = hWnd;
            trackMouse.dwHoverTime = 1;
            TrackMouseEvent(@trackMouse);
            SetProp(hWnd, L"HOT", (HANDLE)TRUE);
            AfxRedrawWindow(hWnd);
        }
        return 0;
    }
    break;


    case WM_MOUSELEAVE:
    {
        //  Removes the hot state and redraws the label
        RemoveProp(hWnd, L"HOT");
        AfxRedrawWindow(hWnd);
        return 0;
    }
    break;


    case WM_LBUTTONUP:
    {
        PostMessage(pData->hParent, MSG_SUPERLABEL_CLICK, (WPARAM)hWnd, 0);
        return 0;
    }
    break;

                
    case WM_PAINT:
    {
        HDC _hdc = 0;
        PAINTSTRUCT ps;
        HDC memDC;           // Double buffering
        HBITMAP hbit;        // Double buffering
            dim bIsHot   as Boolean

            dim as HBRUSH   hBrush
            dim as HPEN     hPen
            dim as HFONT    hFont

            dim as RECT rcClient, rcDraw
            dim as long nLeft, nTop
            dim as single rx = 1, ry = 1

            if (pData == nullptr) {
                BeginPaint(hWnd, &ps);
                EndPaint(hWnd, &ps)
                return 0;
            |
        
        Dim pWindow As CWindow Ptr = AfxCWindowPtr(pData->hParent)
            if pWindow then
                rx = pWindow->rxRatio
                ry = pWindow->ryRatio
                end if


                _hdc = BeginPaint(hwnd, @ps)

                SaveDC _hDC

                GetClientRect(hwnd, @rcClient)

                memDC = CreateCompatibleDC(_hdc)
                hbit = CreateCompatibleBitmap(_hdc, rcClient.right, rcClient.bottom)
                If hbit Then hbit = SelectObject(memDC, hbit)


                ' Determine if we are in a Hot mouseover state
                if pData->HotTestEnable then
                    If GetProp(hwnd, "HOT") Then bIsHot = true
                    end if


                    ' Create the background brush
                    hBrush = CreateSolidBrush(iif(bIsHot, pData->BackColorHot, pData->BackColor))
                    SelectObject(memDC, hBrush)

                    ' If border does not exists then create hollow/null pen
                    if pData->BorderVisible = false then
                        hPen = CreatePen(PS_NULL, pData->BorderWidth * rx, pData->BackColor)
                    else
                        hPen = CreatePen(pData->BorderStyle, _
                            pData->BorderWidth * rx, _
                            iif(bIsHot, pData->BorderColorHot, pData->BorderColor))
                        end if
                        SelectObject(memDC, hPen)


                        ' Paint the background using current brush and pen. Use RoundRect because 
                        ' we may want to have rounded corners.
                        ' Need to paint the whole rectangle otherwise the corners will be unpainted
                        ' once the rounded edges are removed or a PS_NULL pen is used. 
                        ' Need to use the brush of the parent form's background in order to
                        ' simulate the transparency.
                        if pWindow then FillRect(memDC, @rcClient, pWindow->Brush)
                            RoundRect(memDC, 0, 0, rcClient.Right, rcClient.Bottom, _
                                pData->BorderRoundWidth * rx, pData->BorderRoundHeight * ry)


                            ' Create the different label types
                            select case pData->CtrlType
                            case SuperLabelType.ImageOnly, _
                            SuperLabelType.ImageAndText

                            nLeft = (pData->MarginLeft + pData->ImageOffsetLeft) * rx
                            nTop = (pData->MarginTop + pData->ImageOffsetTop) * ry
                            SetRect(@rcDraw, nLeft, nTop, pData->ImageWidth * rx, pData->ImageHeight * ry)
                            SuperLabel_LoadImageFromResource(memDC, _
                                iif(bIsHot, pData->wszImageHot, pData->wszImage), _
                                iif(bIsHot, pData->pImageHot, pData->pImage), _
                                rcDraw)
                            end select


                            select case pData->CtrlType
                            case SuperLabelType.TextOnly, _
                            SuperLabelType.ImageAndText

                            dim as long wsStyle
                            Select Case pData->TextAlignment
                            Case SuperLabelAlignment.BottomCenter: wsStyle = DT_CENTER Or DT_BOTTOM or DT_SINGLELINE
                            Case SuperLabelAlignment.BottomLeft : wsStyle = DT_LEFT   Or DT_BOTTOM or DT_SINGLELINE
                            Case SuperLabelAlignment.BottomRight : wsStyle = DT_RIGHT  Or DT_BOTTOM or DT_SINGLELINE
                            Case SuperLabelAlignment.MiddleCenter : wsStyle = DT_CENTER Or DT_VCENTER or DT_SINGLELINE
                            Case SuperLabelAlignment.MiddleLeft : wsStyle = DT_LEFT   Or DT_VCENTER or DT_SINGLELINE
                            Case SuperLabelAlignment.MiddleRight : wsStyle = DT_RIGHT  Or DT_VCENTER or DT_SINGLELINE
                            Case SuperLabelAlignment.TopCenter : wsStyle = DT_CENTER Or DT_TOP or DT_WORDBREAK
                            Case SuperLabelAlignment.TopLeft : wsStyle = DT_LEFT   Or DT_TOP or DT_WORDBREAK
                            Case SuperLabelAlignment.TopRight : wsStyle = DT_RIGHT  Or DT_TOP or DT_WORDBREAK
                            End Select
                            wsStyle = wsStyle or DT_EXPANDTABS or DT_END_ELLIPSIS

                            if pWindow then
                                if pData->IsSelected then
                                    hFont = pWindow->CreateFont(pData->wszFontName, pData->FontSize, FW_BOLD)
                                else
                                    hFont = pWindow->CreateFont(_
                                        iif(bIsHot, pData->wszFontNameHot, pData->wszFontName), _
                                        iif(bIsHot, pData->FontSizeHot, pData->FontSize), _
                                        iif(bIsHot, pData->FontWeightHot, pData->FontWeight), _
                                        iif(bIsHot, pData->FontItalicHot, pData->FontItalic), _
                                        iif(bIsHot, pData->FontUnderlineHot, pData->FontUnderline), _
                                        iif(bIsHot, pData->FontStrikeOutHot, pData->FontStrikeOut))
                                    end if
                                    end if

                                    SetBkColor(memDC, iif(bIsHot, pData->BackColorHot, pData->BackColor))
                                    SetTextColor(memDC, iif(bIsHot, pData->TextColorHot, pData->TextColor))
                                    SelectObject(memDC, hFont)

                                    nLeft = (pData->MarginLeft + pData->TextOffsetLeft) * rx
                                    nTop = (pData->MarginTop + pData->TextOffsetTop) * ry
                                    SetRect(@rcDraw, nLeft, nTop, _
                                        rcClient.Right - pData->MarginRight, _
                                        rcClient.Bottom - pData->MarginBottom)
                                    if pData->TextCharacterExtra then
                                        SetTextCharacterExtra(memDC, pData->TextCharacterExtra)
                                        end if
                                        DrawText(memDC, pData->wszText, -1, @rcDraw, wsStyle)
                                        'ExtTextOut( memDC, 0, 0, ETO_OPAQUE, @rcDraw, pData->wszText, len(pData->wszText), 0 )            

                                        case SuperLabelType.LineHorizontal, SuperLabelType.LineVertical
                                        ' Delete any existing pen (border) and create a new one for the line.
                                        if hPen then DeleteObject(hPen)
                                            hPen = CreatePen(pData->LineStyle, _
                                                pData->LineWidth * rx, _
                                                iif(bIsHot, pData->LineColorHot, pData->LineColor))
                                            SelectObject(memDC, hPen)

                                            if pData->CtrlType = SuperLabelType.LineHorizontal then
                                                ' Draw the horizontal line vertically centered
                                                nTop = (rcClient.Bottom - rcClient.Top) \ 2
                                                MoveToEx(memDC, pData->MarginLeft * rx, nTop, null)
                                                LineTo(memDC, rcClient.Right - (pData->MarginLeft * rx), nTop)
                                                end if

                                                if pData->CtrlType = SuperLabelType.LineVertical then
                                                    ' Draw the vertical line 
                                                    nLeft = (rcClient.Right - rcClient.Left) \ 2
                                                    MoveToEx(memDC, nLeft, pData->MarginTop * rx, null)
                                                    LineTo(memDC, nLeft, rcClient.Bottom - (pData->MarginBottom * rx))
                                                    end if

                                                    end select


                                                    ' If selection mode is enabled then draw the little right hand side notch
                                                    if (pData->IsSelected) or (bIsHot and pData->SelectionMode) then
                                                        if hBrush then DeleteObject(hBrush)
                                                            if hPen then DeleteObject(hPen)
                                                                hBrush = CreateSolidBrush(pData->SelectorColor)
                                                                hPen = CreatePen(PS_SOLID, 1 * ry, pData->SelectorColor)
                                                                SelectObject(memDC, hBrush)
                                                                SelectObject(memDC, hPen)
                                                                ' Need to center the notch vertically
                                                                dim as long nNotchHalfHeight = (10 * ry) / 2
                                                                nTop = (rcClient.Bottom / 2) - nNotchHalfHeight
                                                                dim as POINT vertices(2)
                                                                vertices(0).x = rcClient.Right            : vertices(0).y = nTop
                                                                vertices(1).x = rcClient.Right - (6 * rx) : vertices(1).y = nTop + nNotchHalfHeight
                                                                vertices(2).x = rcClient.Right : vertices(2).y = nTop + (nNotchHalfHeight * 2)
                                                                Polygon(memDC, @vertices(0), 3)
                                                                end if

                                                                ' Copy the entire memory bitmap to the main display
                                                                BitBlt _hdc, 0, 0, rcClient.right, rcClient.bottom, memDC, 0, 0, SRCCOPY

                                                                ' Cleanup
                                                                If hbit  Then DeleteObject SelectObject(memDC, hbit)
                                                                If memDC Then DeleteDC memDC

                                                                RestoreDC _hDC, -1

                                                                if hBrush then DeleteObject(hBrush)
                                                                    if hPen   then DeleteObject(hPen)
                                                                        if hFont  then DeleteObject(hFont)

                                                                            EndPaint(hwnd, @ps)

                                                                            Exit Function

    }
    break;

    case WM_DESTROY:
        If pData Then
            if pData->pImage    then GdipDisposeImage(pData->pImage)
                if pData->pImageHot then GdipDisposeImage(pData->pImageHot)
                    Delete pData
                    end if
                    break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;

}



         
         

         

   End Select 

   Function = DefWindowProc( hWnd, uMsg, wParam, lParam ) 

 End Function
  

'------------------------------------------------------------------------------ 
function SuperLabel_GetOptions( byval hCtrl as HWND ) as SUPERLABEL_DATA ptr
   dim pData As SUPERLABEL_DATA Ptr = CAST( SUPERLABEL_DATA PTR, GetWindowLongPtr( hCtrl, 0 ))
   function = pData 
end function


'------------------------------------------------------------------------------ 
function SuperLabel_SetOptions( byval hCtrl as HWND, byval pData as SUPERLABEL_DATA ptr ) as Long
   if pData = 0 then exit function

   if len( pData->wszToolTip ) then
      if pData->hToolTip then
         AfxSetTooltipText( pData->hToolTip, hCtrl, pData->wszToolTip )
      end if
   end if

   SetWindowLongPtr( hCtrl, 0, CAST(LONG_PTR, pData) )
   AfxRedrawWindow( hCtrl )
   function = 0 
end function


'------------------------------------------------------------------------------ 
Function SuperLabel( ByVal hWndParent As hwnd, _
                     ByVal CtrlId     As LONG_PTR, _ 
                     byval nCtrlType  as SuperLabelType, _
                     Byref wszText    As wString, _
                     ByVal nLeft      As Long, _
                     ByVal nTop       As Long, _
                     ByVal nWidth     As Long, _
                     ByVal nHeight    As Long _
                     ) As hwnd

   dim wszClassName As wstring * MAX_PATH = "SUPERLABEL_CONTROL"
   dim wcex         As WNDCLASSEX
    
   dim as HINSTANCE hInst = cast( HINSTANCE, GetWindowLongPtr( hWndParent, GWLP_HINSTANCE ))

   If GetClassInfoEx( hInst, @wszClassName, @wcex ) = 0 Then
      with wcex
         .cbSize        = SizeOf(wcex)
         .Style         = CS_DBLCLKS OR CS_HREDRAW OR CS_VREDRAW
         .lpfnWndProc   = @SuperLabelProc
         .cbClsExtra    = 0
         .cbWndExtra    = SIZEOF(HANDLE)   ' make room to store a pointer to the class
         .hInstance     = hInst
         .hCursor       = LoadCursor( NULL, CAST(LPCWSTR, IDC_ARROW) )
         .hIcon         = Null
         .hIconSm       = NULL
         .hbrBackground = CAST(HBRUSH, WHITE_BRUSH)
         .lpszMenuName  = Null
         .lpszClassName = @wszClassName 
      end with
      If RegisterClassEx(@wcex) = 0 Then Exit Function
   End If

    
   dim as single rx = AfxScaleRatioX()
   dim as single ry = AfxScaleRatioY()
   
   dim lpParam AS LONG_PTR = 0

   dim as HWND hCtrl = _
               CreateWindowEx( 0, wszClassName, "", _
                               WS_CHILD or WS_VISIBLE or WS_CLIPCHILDREN or WS_CLIPSIBLINGS, _
                               nLeft * rx, nTop * ry, nWidth * rx, nHeight * ry, _
                               hwndParent, CAST(HMENU, CtrlId), hInst, CAST(LPVOID, lpParam) )

   IF hCtrl THEN
      dim pData As SUPERLABEL_DATA Ptr = new SUPERLABEL_DATA                         
      pData->hWindow    = hCtrl
      pData->hParent    = hWndParent
      pData->hInst      = hInst
      pData->CtrlId     = CtrlId
      pData->wszText    = wszText
      pData->wszTextHot = wszText
      pData->CtrlType   = nCtrlType 
      
      pData->hToolTip = AfxAddTooltip( hCtrl, "", _
                                       FALSE, _   ' balloon 
                                       FALSE _    ' centered
                                       )

      if nCtrlType = SuperLabelType.LineHorizontal then 
         pData->HotTestEnable = false
         pData->BorderVisible = false
      end if
      if nCtrlType = SuperLabelType.LineVertical then 
         pData->HotTestEnable = false
         pData->BorderVisible = false
      end if

      Dim pWindow As CWindow Ptr = AfxCWindowPtr( hWndParent )
      if pWindow then 
         'pData->wszFontName    = pWindow->DefaultFontName
         pData->wszFontName    = "Arial"
         pData->FontSize       = pWindow->DefaultFontSize
         pData->wszFontNameHot = pData->wszFontName
         pData->FontSizeHot    = pData->FontSize
      end if
      
      SuperLabel_SetOptions( hCtrl, pData )
   end if
   
   return = hCtrl
}

 


