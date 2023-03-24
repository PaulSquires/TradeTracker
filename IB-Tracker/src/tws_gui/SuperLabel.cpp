//'
//' SUPER LABEL CONTROL
//' Copyright (C) 2023 Paul Squires, PlanetSquires Software
//' This control should be able to handle all different types of labels and will act
//' like a button, hot tracking, pictures, etc.
//'

#include "framework.h"
#include "CWindow.h"
#include "SuperLabel.h"



// Stick a new line into your.rc file :
// IDI_MY_IMAGE_FILE    PNG      "foo.png"
// And make sure IDI_MY_IMAGE_FILE is defined as an integer in your resource.h header file.
// #define IDI_MY_IMAGE_FILE               131
// Then to load the image at runtime :
// Gdiplus::Bitmap * pBmp = LoadImageFromResource(hInstance, MAKEINTRESOURCE(IDI_MY_IMAGE_FILE), L"PNG");

//'------------------------------------------------------------------------------ 
Gdiplus::Bitmap* LoadImageFromResource(HMODULE hMod, const wchar_t* resid, const wchar_t* restype)
{
    IStream* pStream = nullptr;
    Gdiplus::Bitmap* pBmp = nullptr;
    HGLOBAL hGlobal = nullptr;

    HRSRC hrsrc = FindResourceW(GetModuleHandle(NULL), resid, restype);     // get the handle to the resource
    if (hrsrc)
    {
        DWORD dwResourceSize = SizeofResource(hMod, hrsrc);
        if (dwResourceSize > 0)
        {
            HGLOBAL hGlobalResource = LoadResource(hMod, hrsrc); // load it
            if (hGlobalResource)
            {
                void* imagebytes = LockResource(hGlobalResource); // get a pointer to the file bytes

                // copy image bytes into a real hglobal memory handle
                hGlobal = GlobalAlloc(GHND, dwResourceSize);
                if (hGlobal)
                {
                    void* pBuffer = GlobalLock(hGlobal);
                    if (pBuffer)
                    {
                        memcpy(pBuffer, imagebytes, dwResourceSize);
                        HRESULT hr = CreateStreamOnHGlobal(hGlobal, TRUE, &pStream);
                        if (SUCCEEDED(hr))
                        {
                            // pStream now owns the global handle and will invoke GlobalFree on release
                            hGlobal = nullptr;
                            pBmp = new Gdiplus::Bitmap(pStream);
                        }
                    }
                }
            }
        }
    }

    if (pStream)
    {
        pStream->Release();
        pStream = nullptr;
    }

    if (hGlobal)
    {
        GlobalFree(hGlobal);
        hGlobal = nullptr;
    }

    return pBmp;
}


//'------------------------------------------------------------------------------ 
void SuperLabel_SetTextAlignment(SUPERLABEL_DATA* pData, StringFormat& stringF)
{
    switch (pData->TextAlignment)
    {
    case SuperLabelAlignment::BottomCenter:
        stringF.SetAlignment(StringAlignmentCenter);
        stringF.SetLineAlignment(StringAlignmentFar);
        break;

    case SuperLabelAlignment::BottomLeft:
        stringF.SetAlignment(StringAlignmentNear);
        stringF.SetLineAlignment(StringAlignmentFar);
        break;

    case SuperLabelAlignment::BottomRight:
        stringF.SetAlignment(StringAlignmentFar);
        stringF.SetLineAlignment(StringAlignmentFar);
        break;

    case SuperLabelAlignment::MiddleCenter:
        stringF.SetAlignment(StringAlignmentCenter);
        stringF.SetLineAlignment(StringAlignmentCenter);
        break;

    case SuperLabelAlignment::MiddleLeft:
        stringF.SetAlignment(StringAlignmentNear);
        stringF.SetLineAlignment(StringAlignmentCenter);
        break;

    case SuperLabelAlignment::MiddleRight:
        stringF.SetAlignment(StringAlignmentFar);
        stringF.SetLineAlignment(StringAlignmentCenter);
        break;

    case SuperLabelAlignment::TopCenter:
        stringF.SetAlignment(StringAlignmentCenter);
        stringF.SetLineAlignment(StringAlignmentNear);
        break;

    case SuperLabelAlignment::TopLeft:
        stringF.SetAlignment(StringAlignmentNear);
        stringF.SetLineAlignment(StringAlignmentNear);
        break;

    case SuperLabelAlignment::TopRight:
        stringF.SetAlignment(StringAlignmentFar);
        stringF.SetLineAlignment(StringAlignmentNear);
        break;

    default:
        stringF.SetAlignment(StringAlignmentCenter);
        stringF.SetLineAlignment(StringAlignmentCenter);
    }
}


//'------------------------------------------------------------------------------ 
LRESULT CALLBACK SuperLabelProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    SUPERLABEL_DATA* pData = nullptr;

    if (uMsg != WM_CREATE) {
        pData = (SUPERLABEL_DATA*)GetWindowLongPtr(hWnd, 0);
    }
                 

    switch (uMsg)
    {

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

        if (pData->HotTestEnable) {
            if (GetProp(hWnd, L"HOT") == 0) {
                trackMouse.cbSize = sizeof(trackMouse);
                trackMouse.dwFlags = TME_LEAVE;
                trackMouse.hwndTrack = hWnd;
                trackMouse.dwHoverTime = 1;
                TrackMouseEvent(&trackMouse);
                SetProp(hWnd, L"HOT", (HANDLE)TRUE);
                AfxRedrawWindow(hWnd);
            }
        }
        return 0;
    }
    break;


    case WM_MOUSELEAVE:
    {
        //  Removes the hot state and redraws the label
        if (pData->HotTestEnable) {
            RemoveProp(hWnd, L"HOT");
            AfxRedrawWindow(hWnd);
        }
        return 0;
    }
    break;


    case WM_LBUTTONUP:
    {
        if (pData)
            PostMessage(pData->hParent, MSG_SUPERLABEL_CLICK, (WPARAM)pData->CtrlId, (LPARAM)hWnd);

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
            graphics.SetTextRenderingHint(TextRenderingHintAntiAlias);

            // Create the background brush
            SolidBrush backBrush(bIsHot ? pData->BackColorHot : pData->BackColor);


            // Paint the background using brush and default pen. Use RoundRect because 
            // we may want to have rounded corners.
            graphics.FillRectangle(&backBrush, 0, 0, rcClient.right, rcClient.bottom);
            //    RoundRect(memDC, 0, 0, rcClient.Right, rcClient.Bottom, _
            //        pData->BorderRoundWidth * rx, pData->BorderRoundHeight * ry)


            REAL nLeft = 0;
            REAL nTop = 0;
            REAL nRight = 0;
            REAL nBottom = 0;

            // Create the different label types
            switch (pData->CtrlType)
            {
            case SuperLabelType::ImageOnly:
            case SuperLabelType::ImageAndText:
            {
                nLeft = (pData->MarginLeft + pData->ImageOffsetLeft) * rx;
                nTop = (pData->MarginTop + pData->ImageOffsetTop) * ry;
                nRight = rcClient.right - (pData->MarginRight * rx);
                nBottom = rcClient.bottom - (pData->MarginBottom * ry);

                RectF rcImage(nLeft, nTop, nRight - nLeft, nBottom - nTop);

                graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
                graphics.DrawImage(bIsHot ? pData->pImageHot : pData->pImage, rcImage);
            }
            break;

            default: {}
            }


            switch (pData->CtrlType)
            {
            case SuperLabelType::TextOnly:
            case SuperLabelType::ImageAndText:
            {
                FontFamily   fontFamily(bIsHot ? pData->wszFontNameHot.c_str() : pData->wszFontName.c_str());

                REAL fontSize = (bIsHot ? pData->FontSizeHot : pData->FontSize);
                int fontStyle = FontStyleRegular;

                if (bIsHot) {
                    if (pData->FontBoldHot) fontStyle |= FontStyleBold;
                    if (pData->FontItalicHot) fontStyle |= FontStyleItalic;
                    if (pData->FontUnderlineHot) fontStyle |= FontStyleUnderline;
                }
                else {
                    if (pData->FontBold) fontStyle |= FontStyleBold;
                    if (pData->FontItalic) fontStyle |= FontStyleItalic;
                    if (pData->FontUnderline) fontStyle |= FontStyleUnderline;
                }

                Font         font(&fontFamily, fontSize, fontStyle, Unit::UnitPoint);
                SolidBrush   textBrush(bIsHot ? pData->TextColorHot : pData->TextColor);

                StringFormat stringF(0);
                SuperLabel_SetTextAlignment(pData, stringF);

                if (pData->TextCharacterExtra)
                    SetTextCharacterExtra(memDC, pData->TextCharacterExtra);

                nLeft = (pData->MarginLeft + pData->TextOffsetLeft) * rx;
                nTop = (pData->MarginTop + pData->TextOffsetTop) * ry;
                nRight = rcClient.right - (pData->MarginRight * rx);
                nBottom = rcClient.bottom - (pData->MarginBottom * ry);

                RectF rcText(nLeft, nTop, nRight - nLeft, nBottom - nTop);

                graphics.DrawString(pData->wszText.c_str(), -1, &font, rcText, &stringF, &textBrush);
            }

            case SuperLabelType::LineHorizontal:
            {
                nLeft = pData->MarginLeft * rx;
                nTop = (pData->MarginTop + pData->TextOffsetTop) * ry;
                nRight = rcClient.right - (pData->MarginRight * rx);
                nBottom = nTop;
                ARGB clrPen = (bIsHot ? pData->LineColorHot : pData->LineColor);
                Pen pen(clrPen, pData->LineWidth);
                // Draw the horizontal line centered taking margins into account
                graphics.DrawLine(&pen, nLeft, nTop, nRight, nBottom);
            }

            case SuperLabelType::LineVertical:
            {
                ARGB clrPen = (bIsHot ? pData->LineColorHot : pData->LineColor);
                Pen pen(clrPen, pData->LineWidth);
                // Draw the vertical line centered taking margins into account
                //graphics.DrawLine(&pen, rcDraw.GetLeft(), rcDraw.GetTop(), rcDraw.GetRight(), rcDraw.GetBottom());
            }

            default: {}


            }


            // If selection mode is enabled then draw the little right hand side notch
            if ((pData->IsSelected) || (bIsHot && pData->SelectionMode)) {
                // Create the background brush
                SolidBrush backBrush(pData->SelectorColor);
                // Need to center the notch vertically
                REAL nNotchHalfHeight = (10 * ry) / 2;
                REAL nTop = (rcClient.bottom / 2) - nNotchHalfHeight;
                PointF point1((REAL)rcClient.right, nTop);
                PointF point2((REAL)rcClient.right - (6 * rx), nTop + nNotchHalfHeight);
                PointF point3((REAL)rcClient.right, nTop + (nNotchHalfHeight * 2));
                PointF points[3] = { point1, point2, point3 };
                PointF* pPoints = points;
                graphics.FillPolygon(&backBrush, pPoints, 3);
            }
        

            // Finally, draw any applicable border around the control after everything
            // else has been painted.
            switch (pData->CtrlType)
            {
            case SuperLabelType::ImageOnly:
            case SuperLabelType::ImageAndText:
            case SuperLabelType::TextOnly:
            {
                ARGB clrPen = (bIsHot ? pData->BorderColorHot : pData->BorderColor);
                if (!pData->BorderVisible) clrPen = pData->BackColor;
                Pen pen(clrPen, pData->BorderWidth);

                RectF rectF(0, 0, (REAL)rcClient.right, (REAL)rcClient.bottom);
                graphics.DrawRectangle(&pen, rectF);
            }
            break;
            
            default: {}
            }


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
            if (pData->pImage) delete(pData->pImage);
            if (pData->pImageHot) delete(pData->pImageHot);
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
            AfxSetTooltipText(pData->hToolTip, hCtrl, pData->wszToolTip);
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
   

    HWND hCtl =
        CreateWindowEx(0, wszClassName.c_str(), L"",
            WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
            (int)(nLeft * rx), (int)(nTop * ry), (int)(nWidth * rx), (int)(nHeight * ry),
            hWndParent, (HMENU)CtrlId, hInst, (LPVOID)NULL);

    if (hCtl) {
        SUPERLABEL_DATA* pData = new SUPERLABEL_DATA;

        pData->hWindow = hCtl;
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

        pData->hToolTip = AfxAddTooltip(hCtl, L"", FALSE, FALSE);

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


        SuperLabel_SetOptions(hCtl, pData);
    }
   
    return hCtl;
}

 
