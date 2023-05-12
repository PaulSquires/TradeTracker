//
// CUSTOM LABEL CONTROL
// Copyright (C) 2023 Paul Squires, PlanetSquires Software
// This control should be able to handle all different types of labels and will act
// like a button, hot tracking, pictures, etc.
//

#include "pch.h"
#include "..\Utilities\AfxWin.h"
#include "CustomLabel.h"



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
LRESULT CALLBACK CustomLabelProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static CustomLabel* pDataSelected = nullptr;
    CustomLabel* pData = nullptr;

    if (uMsg != WM_CREATE) {
        pData = (CustomLabel*)GetWindowLongPtr(hWnd, 0);
    }
                 

    switch (uMsg)
    {

    case WM_SETCURSOR:
    {
        LPWSTR IDCPointer = IDC_HAND;

        if (pData) {
            if (pData->HotTestEnable) {
                IDCPointer = (pData->PointerHot == CustomLabelPointer::Hand) ? IDC_HAND : IDC_ARROW;
            }
            else {
                IDCPointer = (pData->Pointer == CustomLabelPointer::Hand) ? IDC_HAND : IDC_ARROW;
            }
        }
        SetCursor(LoadCursor(NULL, (LPCWSTR)IDCPointer));
        return 0;
    }
    break;


    case WM_MOUSEMOVE:
    {
        if (pData == nullptr) return 0;
        SendMessage(pData->hParent, MSG_CUSTOMLABEL_MOUSEMOVE, (WPARAM)pData->CtrlId, (LPARAM)hWnd);

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
        if (pData == nullptr) return 0;
        SendMessage(pData->hParent, MSG_CUSTOMLABEL_MOUSELEAVE, (WPARAM)pData->CtrlId, (LPARAM)hWnd);

        //  Removes the hot state and redraws the label
        if (pData->HotTestEnable) {
            RemoveProp(hWnd, L"HOT");
            AfxRedrawWindow(hWnd);
        }
        return 0;
        break;


    case WM_LBUTTONDOWN:
        if (pData) {
            // Allow the label to repaint in case different color has been specified
            // for button down (eg. if label is acting like a button).
            AfxRedrawWindow(hWnd);

            // We do not set the button IsSelected to true here because we leave it to 
            // the user to decide if after clicking the label whether to set the label
            // to selected or not.
            SendMessage(pData->hParent, MSG_CUSTOMLABEL_CLICK, (WPARAM)pData->CtrlId, (LPARAM)hWnd);
        }
        return 0;
        break;


    case WM_LBUTTONUP:
        if (pData) {
            // Allow the label to repaint in case different color has been specified
            // for button down (eg. if label is acting like a button).
            AfxRedrawWindow(hWnd);
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
CustomLabel* CustomLabel_GetOptions(HWND hCtrl)
{
    CustomLabel* pData = (CustomLabel*)GetWindowLongPtr(hCtrl, 0);
    return pData;
}


//------------------------------------------------------------------------------ 
int CustomLabel_SetOptions(HWND hCtrl, CustomLabel* pData)
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
void CustomLabel_SetText(HWND hCtrl, std::wstring wszText)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->wszText = wszText;
        pData->wszTextHot = wszText;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


//------------------------------------------------------------------------------ 
void CustomLabel_SetTextColor(HWND hCtrl, ThemeElement TextColor)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->TextColor = TextColor;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


//------------------------------------------------------------------------------ 
void CustomLabel_SetBackColor(HWND hCtrl, ThemeElement BackColor)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->BackColor = BackColor;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


//------------------------------------------------------------------------------ 
ThemeElement CustomLabel_GetBackColor(HWND hCtrl)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        return pData->BackColor;
    }
    return ThemeElement::Black;
}


//------------------------------------------------------------------------------ 
std::wstring CustomLabel_GetText(HWND hCtrl)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        return pData->wszText;
    }
    return L"";
}


//------------------------------------------------------------------------------ 
void CustomLabel_SetBorder(HWND hCtrl, REAL BorderWidth, ThemeElement BorderColor, ThemeElement BorderColorHot)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->BorderVisible = true;
        pData->BorderWidth = BorderWidth;
        pData->BorderColor = BorderColor;
        pData->BorderColorHot = BorderColorHot;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


//------------------------------------------------------------------------------ 
void CustomLabel_SetMousePointer(HWND hCtrl, CustomLabelPointer NormalPointer, CustomLabelPointer HotPointer)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->Pointer = NormalPointer;
        pData->PointerHot = HotPointer;
        CustomLabel_SetOptions(hCtrl, pData);
    }

}


//------------------------------------------------------------------------------ 
void CustomLabel_SetTextOffset(HWND hCtrl, int OffsetLeft, int OffsetTop)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->TextOffsetLeft = OffsetLeft;
        pData->TextOffsetTop = OffsetTop;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}

//------------------------------------------------------------------------------ 
void CustomLabel_SetUserData(HWND hCtrl, std::wstring UserData)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->UserData = UserData;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


//------------------------------------------------------------------------------ 
std::wstring CustomLabel_GetUserData(HWND hCtrl)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        return pData->UserData;
    }
    return L"";
}


//------------------------------------------------------------------------------ 
void CustomLabel_SetFont(HWND hCtrl, std::wstring wszFontName, int FontSize)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->wszFontName = wszFontName;
        pData->wszFontNameHot = wszFontName;
        pData->FontSize = (REAL)FontSize;
        pData->FontSizeHot = (REAL)FontSize;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


//------------------------------------------------------------------------------ 
void CustomLabel_Select(HWND hCtrl, bool IsSelected)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        bool redraw = pData->IsSelected != IsSelected ? true : false;
        pData->IsSelected = IsSelected;
        CustomLabel_SetOptions(hCtrl, pData);
        if (redraw) AfxRedrawWindow(hCtrl);
    }
}



//------------------------------------------------------------------------------ 
HWND CustomLabel_SimpleLabel(HWND hParent, int CtrlId, std::wstring wszText,
    ThemeElement TextColor, ThemeElement BackColor, CustomLabelAlignment alignment, 
    int nLeft, int nTop, int nWidth, int nHeight)
{
    // Creates a simple "dumb" label that basically just makes it easier to deal
    // with theme coloring. No hot tracking or click notifications.
    CustomLabel* pData = nullptr;

    HWND hCtl = CreateCustomLabel(
        hParent, CtrlId, CustomLabelType::TextOnly,
        nLeft, nTop, nWidth, nHeight);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->wszText = wszText;
        pData->HotTestEnable = false;
        pData->BackColor = BackColor;
        pData->TextColor = TextColor;
        pData->BackColorButtonDown = BackColor;
        pData->TextAlignment = alignment;
        CustomLabel_SetOptions(hCtl, pData);
    }

    return hCtl;
}



//------------------------------------------------------------------------------ 
HWND CustomLabel_ButtonLabel(HWND hParent, int CtrlId, std::wstring wszText,
    ThemeElement TextColor, ThemeElement BackColor, ThemeElement BackColorHot, ThemeElement BackColorButtonDown,
    CustomLabelAlignment alignment, int nLeft, int nTop, int nWidth, int nHeight)
{
    // Creates a simple button type of label
    CustomLabel* pData = nullptr;

    HWND hCtl = CreateCustomLabel(
        hParent, CtrlId, CustomLabelType::TextOnly,
        nLeft, nTop, nWidth, nHeight);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->wszText = wszText;
        pData->wszTextHot = wszText;
        pData->HotTestEnable = true;
        pData->BackColor = BackColor;
        pData->BackColorHot = BackColorHot;
        pData->BackColorButtonDown = BackColorButtonDown;
        pData->TextColor = TextColor;
        pData->TextColorHot = TextColor;
        pData->TextAlignment = alignment;
        CustomLabel_SetOptions(hCtl, pData);
    }

    return hCtl;
}




//------------------------------------------------------------------------------ 
HWND CustomLabel_SimpleImageLabel(HWND hParent, int CtrlId, 
    std::wstring wszImage, std::wstring wszImageHot, 
    int ImageWidth, int ImageHeight,
    int nLeft, int nTop, int nWidth, int nHeight)
{
    // Creates a simple "dumb" label that basically just makes it easier to deal
    // with theme coloring. No hot tracking or click notifications.
    CustomLabel* pData = nullptr;
    
    HWND hCtl = CreateCustomLabel(
    hParent, CtrlId,
    CustomLabelType::ImageOnly,
    nLeft, nTop, nWidth, nHeight);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = false;
        pData->BackColor = ThemeElement::Black;
        pData->BackColorButtonDown = pData->BackColor;
        pData->ImageWidth = 68;
        pData->ImageHeight = 68;
        pData->pImage = LoadImageFromResource(pData->hInst, MAKEINTRESOURCE(IDB_LOGO), L"PNG");
        pData->pImageHot = LoadImageFromResource(pData->hInst, MAKEINTRESOURCE(IDB_LOGO), L"PNG");
        CustomLabel_SetOptions(hCtl, pData);
    }
    return hCtl;
}


//------------------------------------------------------------------------------ 
HWND CreateCustomLabel( 
    HWND hWndParent, 
    LONG_PTR CtrlId,
    CustomLabelType nCtrlType,
    int nLeft, 
    int nTop, 
    int nWidth, 
    int nHeight )
{
    std::wstring wszClassName(L"CUSTOMLABEL_CONTROL");

    WNDCLASSEX wcex{};
    
    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hWndParent, GWLP_HINSTANCE);

    if (GetClassInfoEx(hInst, wszClassName.c_str(), &wcex) == 0) {
        wcex.cbSize = sizeof(wcex);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = CustomLabelProc;
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
        CustomLabel* pData = new CustomLabel;

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

        if (nCtrlType == CustomLabelType::LineHorizontal || 
            nCtrlType == CustomLabelType::LineVertical) {
            pData->HotTestEnable = false;
            pData->BorderVisible = false;
        }

        pData->Pointer = CustomLabelPointer::Arrow;
        pData->PointerHot = CustomLabelPointer::Hand;

        CustomLabel_SetOptions(hCtl, pData);
    }
   
    return hCtl;
}


