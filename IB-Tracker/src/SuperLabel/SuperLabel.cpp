//
// SUPER LABEL CONTROL
// Copyright (C) 2023 Paul Squires, PlanetSquires Software
// This control should be able to handle all different types of labels and will act
// like a button, hot tracking, pictures, etc.
//

#include "pch.h"
#include "..\Utilities\AfxWin.h"
#include "SuperLabel.h"



// Stick a new line into your.rc file :
// IDI_MY_IMAGE_FILE    PNG      "foo.png"
// And make sure IDI_MY_IMAGE_FILE is defined as an integer in your resource.h header file.
// #define IDI_MY_IMAGE_FILE               131
// Then to load the image at runtime :
// Gdiplus::Bitmap * pBmp = LoadImageFromResource(hInstance, MAKEINTRESOURCE(IDI_MY_IMAGE_FILE), L"PNG");

//------------------------------------------------------------------------------ 
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


//------------------------------------------------------------------------------ 
LRESULT CALLBACK SuperLabelProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static SuperLabel* pDataSelected = nullptr;
    SuperLabel* pData = nullptr;

    if (uMsg != WM_CREATE) {
        pData = (SuperLabel*)GetWindowLongPtr(hWnd, 0);
    }
                 

    switch (uMsg)
    {

    case WM_SETCURSOR:
    {
        LPWSTR IDCPointer = IDC_HAND;

        if (pData) {
            if (pData->HotTestEnable) {
                IDCPointer = (pData->PointerHot == SuperLabelPointer::Hand) ? IDC_HAND : IDC_ARROW;
            }
            else {
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
        //  Removes the hot state and redraws the label
        if (pData->HotTestEnable) {
            RemoveProp(hWnd, L"HOT");
            AfxRedrawWindow(hWnd);
        }
        return 0;
        break;


    case WM_LBUTTONDOWN:
        if (pData) {
            // We do not set the button IsSelected to true here because we leave it to 
            // the user to decide if after clicking the label whether to set the label
            // to selected or not.
            PostMessage(pData->hParent, MSG_SUPERLABEL_CLICK, (WPARAM)pData->CtrlId, (LPARAM)hWnd);
        }
        return 0;
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
        if (pData) {
            pData->StartDoubleBuffering(hdc);
            pData->DrawImageInBuffer();
            pData->DrawTextInBuffer();
            pData->DrawLabelInBuffer();
            pData->DrawNotchInBuffer();
            pData->DrawBordersInBuffer();
            pData->EndDoubleBuffering(hdc);
        }
        EndPaint(hWnd, &ps);
        break;
    }


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



//------------------------------------------------------------------------------ 
SuperLabel* SuperLabel_GetOptions(HWND hCtrl)
{
    SuperLabel* pData = (SuperLabel*)GetWindowLongPtr(hCtrl, 0);
    return pData;
}


//------------------------------------------------------------------------------ 
int SuperLabel_SetOptions(HWND hCtrl, SuperLabel* pData)
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


//------------------------------------------------------------------------------ 
void SuperLabel_SetText(HWND hCtrl, std::wstring wszText)
{
    SuperLabel* pData = SuperLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->wszText = wszText;
        pData->wszTextHot = wszText;
        SuperLabel_SetOptions(hCtrl, pData);
        AfxRedrawWindow(hCtrl);
    }
}


//------------------------------------------------------------------------------ 
void SuperLabel_Select(HWND hCtrl, bool IsSelected)
{
    SuperLabel* pData = SuperLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        bool redraw = pData->IsSelected != IsSelected ? true : false;
        pData->IsSelected = IsSelected;
        SuperLabel_SetOptions(hCtrl, pData);
        if (redraw) AfxRedrawWindow(hCtrl);
    }
}



//------------------------------------------------------------------------------ 
HWND SuperLabel_SimpleLabel(HWND hParent, int CtrlId, std::wstring wszText,
    ThemeElement TextColor, ThemeElement BackColor, SuperLabelAlignment alignment, 
    int nLeft, int nTop, int nWidth, int nHeight)
{
    // Creates a simple "dumb" label that basically just makes it easier to deal
    // with theme coloring. No hot tracking or click notifications.
    SuperLabel* pData = nullptr;

    HWND hCtl = CreateSuperLabel(
        hParent, CtrlId, SuperLabelType::TextOnly,
        nLeft, nTop, nWidth, nHeight);
    pData = SuperLabel_GetOptions(hCtl);
    if (pData) {
        pData->wszText = wszText;
        pData->HotTestEnable = false;
        pData->BackColor = BackColor;
        pData->TextColor = TextColor;
        pData->TextAlignment = alignment;
        SuperLabel_SetOptions(hCtl, pData);
    }

    return hCtl;
}


//------------------------------------------------------------------------------ 
HWND CreateSuperLabel( 
    HWND hWndParent, 
    LONG_PTR CtrlId,
    SuperLabelType nCtrlType,
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
        wcex.lpfnWndProc = SuperLabelProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = sizeof(HANDLE);    // make room to store a pointer to the class
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
        SuperLabel* pData = new SuperLabel;

        pData->hWindow = hCtl;
        pData->hParent = hWndParent;
        pData->hInst = hInst;
        pData->CtrlId = CtrlId;
        pData->CtrlType = nCtrlType;

        pData->wszFontName = L"Segoe UI";
        pData->FontSize = 9;
        pData->wszFontNameHot = pData->wszFontName;
        pData->FontSizeHot = pData->FontSize;

        pData->hToolTip = AfxAddTooltip(hCtl, L"", FALSE, FALSE);

        if (nCtrlType == SuperLabelType::LineHorizontal || 
            nCtrlType == SuperLabelType::LineVertical) {
            pData->HotTestEnable = false;
            pData->BorderVisible = false;
        }

        pData->Pointer = SuperLabelPointer::Arrow;
        pData->PointerHot = SuperLabelPointer::Hand;

        SuperLabel_SetOptions(hCtl, pData);
    }
   
    return hCtl;
}


