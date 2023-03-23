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
        LPWSTR IDCPointer = IDC_HAND;

        if (pData) {
            if (pData->HotTestEnable) {
                IDCPointer = (pData->PointerHot == SuperLabelPointer::Hand) ? IDC_HAND : IDC_ARROW;
            } else {
                IDCPointer = (pData->Pointer == SuperLabelPointer::Hand) ? IDC_HAND : IDC_ARROW;
            }
        }
        SetCursor(LoadCursor(NULL, (LPCWSTR)IDCPointer));
        return 0;
    }
    break;


    case WM_MOUSEMOVE:
    {
        if (pData == nullptr) return 0;
        SendMessage(pData->hParent, MSG_SUPERLABEL_MOUSEMOVE, (WPARAM)hWnd, 0);

        // Tracks the mouse movement and stores the hot state
        TRACKMOUSEEVENT trackMouse;

        if (GetProp(hWnd, L"HOT") == 0) {
            trackMouse.cbSize = sizeof(trackMouse);
            trackMouse.dwFlags = TME_LEAVE;
            trackMouse.hwndTrack = hWnd;
            trackMouse.dwHoverTime = 1;
            TrackMouseEvent(&trackMouse);
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
        HDC hdc = NULL;
        PAINTSTRUCT ps;
        HDC memDC;           // Double buffering
        HBITMAP hbit;        // Double buffering

        if (pData == nullptr) {
            BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
             return 0;
        }

                
        RECT rcClient{}, rcDraw{};
        int  nLeft = 0, nTop = 0;
        float rx = 1, ry = 1;

        bool bIsHot = false;

        CWindow* pWindow = AfxCWindowPtr(pData->hParent);

        if (pWindow != nullptr)
        {

            rx = pWindow->rxRatio();
            ry = pWindow->ryRatio();

            hdc = BeginPaint(hWnd, &ps);

            SaveDC(hdc);

            GetClientRect(hWnd, &rcClient);

            memDC = CreateCompatibleDC(hdc);
            hbit = CreateCompatibleBitmap(hdc, rcClient.right, rcClient.bottom);
            SelectBitmap(memDC, hbit);


            // Determine if we are in a Hot mouseover state
            if (pData->HotTestEnable) {
                if (GetProp(hWnd, L"HOT")) bIsHot = true;
            }

            Graphics graphics(memDC);


            // If border does not exist then create hollow/null pen
            ARGB clrPen = (bIsHot ? pData->BorderColorHot : pData->BorderColor);
            if (pData->BorderVisible == false) clrPen = pData->BackColor;
            Pen pen(clrPen, pData->BorderWidth);

            // Create the background brush
            SolidBrush backBrush(bIsHot ? pData->BackColorHot : pData->BackColor);
            SolidBrush textBrush(bIsHot ? pData->TextColorHot : pData->TextColor);


            // Paint the background using current brush and pen. Use RoundRect because 
            // we may want to have rounded corners.
            graphics.FillRectangle(&backBrush, 0, 0, rcClient.right, rcClient.bottom);
            //    RoundRect(memDC, 0, 0, rcClient.Right, rcClient.Bottom, _
            //        pData->BorderRoundWidth * rx, pData->BorderRoundHeight * ry)

            FontFamily   fontFamily(pData->wszFontName.c_str());
            Font         font(&fontFamily, pData->FontSize, FontStyleRegular, Unit::UnitPoint);
            PointF       pointF(0.0f, 0.0f);
            RectF        rectF(0, 0, (Gdiplus::REAL)rcClient.right, (Gdiplus::REAL)rcClient.bottom);

            StringFormat stringF(0);
            stringF.SetAlignment(StringAlignmentCenter);
            stringF.SetLineAlignment(StringAlignmentCenter);

            graphics.DrawString(L"Hello World!", -1, &font, rectF, &stringF, &textBrush);
            rectF.Inflate(-(pData->BorderWidth), -(pData->BorderWidth));
            graphics.DrawRectangle(&pen, rectF);


                        //    // Create the different label types
                        //    select case pData->CtrlType
                        //    case SuperLabelType.ImageOnly, _
                        //    SuperLabelType.ImageAndText

                        //    nLeft = (pData->MarginLeft + pData->ImageOffsetLeft) * rx
                        //    nTop = (pData->MarginTop + pData->ImageOffsetTop) * ry
                        //    SetRect(@rcDraw, nLeft, nTop, pData->ImageWidth * rx, pData->ImageHeight * ry)
                        //    SuperLabel_LoadImageFromResource(memDC, _
                        //        iif(bIsHot, pData->wszImageHot, pData->wszImage), _
                        //            iif(bIsHot, pData->pImageHot, pData->pImage), _
                        //            rcDraw)
                        //            end select


                        //            select case pData->CtrlType
                        //            case SuperLabelType.TextOnly, _
                        //            SuperLabelType.ImageAndText

                        //            dim as long wsStyle
                        //            Select Case pData->TextAlignment
                        //            Case SuperLabelAlignment.BottomCenter: wsStyle = DT_CENTER Or DT_BOTTOM or DT_SINGLELINE
                        //            Case SuperLabelAlignment.BottomLeft : wsStyle = DT_LEFT   Or DT_BOTTOM or DT_SINGLELINE
                        //            Case SuperLabelAlignment.BottomRight : wsStyle = DT_RIGHT  Or DT_BOTTOM or DT_SINGLELINE
                        //            Case SuperLabelAlignment.MiddleCenter : wsStyle = DT_CENTER Or DT_VCENTER or DT_SINGLELINE
                        //            Case SuperLabelAlignment.MiddleLeft : wsStyle = DT_LEFT   Or DT_VCENTER or DT_SINGLELINE
                        //            Case SuperLabelAlignment.MiddleRight : wsStyle = DT_RIGHT  Or DT_VCENTER or DT_SINGLELINE
                        //            Case SuperLabelAlignment.TopCenter : wsStyle = DT_CENTER Or DT_TOP or DT_WORDBREAK
                        //            Case SuperLabelAlignment.TopLeft : wsStyle = DT_LEFT   Or DT_TOP or DT_WORDBREAK
                        //            Case SuperLabelAlignment.TopRight : wsStyle = DT_RIGHT  Or DT_TOP or DT_WORDBREAK
                        //            End Select
                        //            wsStyle = wsStyle or DT_EXPANDTABS or DT_END_ELLIPSIS

                        //            if pWindow then
                        //                if pData->IsSelected then
                        //                    hFont = pWindow->CreateFont(pData->wszFontName, pData->FontSize, FW_BOLD)
                        //                else
                        //                    hFont = pWindow->CreateFont(_
                        //                        iif(bIsHot, pData->wszFontNameHot, pData->wszFontName), _
                        //                        iif(bIsHot, pData->FontSizeHot, pData->FontSize), _
                        //                        iif(bIsHot, pData->FontWeightHot, pData->FontWeight), _
                        //                        iif(bIsHot, pData->FontItalicHot, pData->FontItalic), _
                        //                        iif(bIsHot, pData->FontUnderlineHot, pData->FontUnderline), _
                        //                        iif(bIsHot, pData->FontStrikeOutHot, pData->FontStrikeOut))
                        //                    end if
                        //                    end if

                        //                    SetBkColor(memDC, iif(bIsHot, pData->BackColorHot, pData->BackColor))
                        //                    SetTextColor(memDC, iif(bIsHot, pData->TextColorHot, pData->TextColor))
                        //                    SelectObject(memDC, hFont)

                        //                    nLeft = (pData->MarginLeft + pData->TextOffsetLeft) * rx
                        //                    nTop = (pData->MarginTop + pData->TextOffsetTop) * ry
                        //                    SetRect(@rcDraw, nLeft, nTop, _
                        //                        rcClient.Right - pData->MarginRight, _
                        //                        rcClient.Bottom - pData->MarginBottom)
                        //                    if pData->TextCharacterExtra then
                        //                        SetTextCharacterExtra(memDC, pData->TextCharacterExtra)
                        //                        end if
                        //                        DrawText(memDC, pData->wszText, -1, @rcDraw, wsStyle)
                        //                        'ExtTextOut( memDC, 0, 0, ETO_OPAQUE, @rcDraw, pData->wszText, len(pData->wszText), 0 )            

                        //                        case SuperLabelType.LineHorizontal, SuperLabelType.LineVertical
                        //                        // Delete any existing pen (border) and create a new one for the line.
                        //                        if hPen then DeleteObject(hPen)
                        //                            hPen = CreatePen(pData->LineStyle, _
                        //                                pData->LineWidth * rx, _
                        //                                iif(bIsHot, pData->LineColorHot, pData->LineColor))
                        //                            SelectObject(memDC, hPen)

                        //                            if pData->CtrlType = SuperLabelType.LineHorizontal then
                        //                                // Draw the horizontal line vertically centered
                        //                                nTop = (rcClient.Bottom - rcClient.Top) \ 2
                        //                                MoveToEx(memDC, pData->MarginLeft * rx, nTop, null)
                        //                                LineTo(memDC, rcClient.Right - (pData->MarginLeft * rx), nTop)
                        //                                end if

                        //                                if pData->CtrlType = SuperLabelType.LineVertical then
                        //                                    // Draw the vertical line 
                        //                                    nLeft = (rcClient.Right - rcClient.Left) \ 2
                        //                                    MoveToEx(memDC, nLeft, pData->MarginTop * rx, null)
                        //                                    LineTo(memDC, nLeft, rcClient.Bottom - (pData->MarginBottom * rx))
                        //                                    end if

                        //                                    end select


                        //                                    // If selection mode is enabled then draw the little right hand side notch
                        //                                    if (pData->IsSelected) or (bIsHot and pData->SelectionMode) then
                        //                                        if hBrush then DeleteObject(hBrush)
                        //                                            if hPen then DeleteObject(hPen)
                        //                                                hBrush = CreateSolidBrush(pData->SelectorColor)
                        //                                                hPen = CreatePen(PS_SOLID, 1 * ry, pData->SelectorColor)
                        //                                                SelectObject(memDC, hBrush)
                        //                                                SelectObject(memDC, hPen)
                        //                                                // Need to center the notch vertically
                        //                                                dim as long nNotchHalfHeight = (10 * ry) / 2
                        //                                                nTop = (rcClient.Bottom / 2) - nNotchHalfHeight
                        //                                                dim as POINT vertices(2)
                        //                                                vertices(0).x = rcClient.Right            : vertices(0).y = nTop
                        //                                                vertices(1).x = rcClient.Right - (6 * rx) : vertices(1).y = nTop + nNotchHalfHeight
                        //                                                vertices(2).x = rcClient.Right : vertices(2).y = nTop + (nNotchHalfHeight * 2)
                        //                                                Polygon(memDC, @vertices(0), 3)
                        //                                                end if

            // Copy the entire memory bitmap to the main display
            BitBlt(hdc, 0, 0, rcClient.right, rcClient.bottom, memDC, 0, 0, SRCCOPY);

            // Restore the original state of the DC
            RestoreDC(hdc, -1);

            // Cleanup
            DeleteObject(hbit);
            DeleteDC(memDC);

            EndPaint(hWnd, &ps);

        }
         
    }
    break;

    case WM_DESTROY:
        if (pData) {
        //    if (pData->pImage) GdipDisposeImage(pData->pImage);
        //    if (pData->pImageHot) GdipDisposeImage(pData->pImageHot);
        }
        break;

    case WM_NCDESTROY:
        if (pData) delete(pData);
        break;

    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;

}



//'------------------------------------------------------------------------------ 
SUPERLABEL_DATA* SuperLabel_GetOptions(HWND hCtrl)
{
    SUPERLABEL_DATA* pData = (SUPERLABEL_DATA*)GetWindowLongPtr(hCtrl, 0);
    return pData;
}


//'------------------------------------------------------------------------------ 
int SuperLabel_SetOptions(HWND hCtrl, SUPERLABEL_DATA* pData)
{
    if (pData == nullptr) return 0;

    if (pData->wszToolTip.length()) {
        if (pData->hToolTip) {
            //AfxSetTooltipText(pData->hToolTip, hCtrl, pData->wszToolTip)
        }
    }

    SetWindowLongPtr(hCtrl, 0, (LONG_PTR)pData);
    AfxRedrawWindow(hCtrl);

    return 0;
}


//'------------------------------------------------------------------------------ 
HWND CreateSuperLabel( 
    HWND hWndParent, 
    LONG_PTR CtrlId,
    SuperLabelType nCtrlType,
    std::wstring wszText, 
    int nLeft, 
    int nTop, 
    int nWidth, 
    int nHeight )
{
    std::wstring wszClassName(L"SUPERLABEL_CONTROL");

    WNDCLASSEX wcex{};
    
    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hWndParent, GWLP_HINSTANCE);

    if (GetClassInfoEx(hInst, wszClassName.c_str(), &wcex) == 0) {
        wcex.cbSize = sizeof(wcex);
        wcex.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = &SuperLabelProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = sizeof(HANDLE);    //' make room to store a pointer to the class
        wcex.hInstance = hInst;
        wcex.hCursor = LoadCursor(NULL, (LPCWSTR)IDC_ARROW);
        wcex.hIcon = NULL;
        wcex.hIconSm = NULL;
        wcex.hbrBackground = (HBRUSH)WHITE_BRUSH;
        wcex.lpszMenuName = NULL;
        wcex.lpszClassName = wszClassName.c_str();
        if(RegisterClassEx(&wcex) == 0) return 0;
    }
        
    float rx = AfxScaleRatioX();
    float ry = AfxScaleRatioY();
   

    HWND hCtrl =
        CreateWindowEx(0, wszClassName.c_str(), L"",
            WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
            (int)(nLeft * rx), (int)(nTop * ry), (int)(nWidth * rx), (int)(nHeight * ry),
            hWndParent, (HMENU)CtrlId, hInst, (LPVOID)NULL);

    if (hCtrl) {
        SUPERLABEL_DATA* pData = new SUPERLABEL_DATA;

        pData->hWindow = hCtrl;
        pData->hParent = hWndParent;
        pData->hInst = hInst;
        pData->CtrlId = CtrlId;
        pData->wszText = wszText;
        pData->wszTextHot = wszText;
        pData->CtrlType = nCtrlType;

        pData->wszFontName = L"Segoe UI";
        pData->FontSize = 9;
        pData->wszFontNameHot = pData->wszFontName;
        pData->FontSizeHot = pData->FontSize;

        //pData->hToolTip = AfxAddTooltip( hCtrl, "", _
        //                                 FALSE, _   ' balloon 
        //                                 FALSE _    ' centered
        //                                 )

        if (nCtrlType == SuperLabelType::LineHorizontal) {
            pData->HotTestEnable = false;
            pData->BorderVisible = false;
        }
        if (nCtrlType == SuperLabelType::LineVertical) {
            pData->HotTestEnable = false;
            pData->BorderVisible = false;
        }

        pData->Pointer = SuperLabelPointer::Arrow;
        pData->PointerHot = SuperLabelPointer::Hand;


        SuperLabel_SetOptions(hCtrl, pData);
    }
   
    return hCtrl;
}

 
