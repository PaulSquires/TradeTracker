/*

MIT License

Copyright(c) 2023 Paul Squires

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

//
// CUSTOM LABEL CONTROL
// This control should be able to handle all different types of labels and will act
// like a button, hot tracking, pictures, etc.
//

#include "pch.h"
#include "Utilities/AfxWin.h"
#include "Utilities/Colors.h"
#include "CustomLabel.h"


// ========================================================================================
// Set the vertical and horizonatal text alignment during graphic output.
// ========================================================================================
void CustomLabel::SetTextAlignment(StringFormat* stringF)
{
    switch (TextAlignment)
    {
    case CustomLabelAlignment::BottomCenter:
        stringF->SetAlignment(StringAlignmentCenter);
        stringF->SetLineAlignment(StringAlignmentFar);
        break;

    case CustomLabelAlignment::BottomLeft:
        stringF->SetAlignment(StringAlignmentNear);
        stringF->SetLineAlignment(StringAlignmentFar);
        break;

    case CustomLabelAlignment::BottomRight:
        stringF->SetAlignment(StringAlignmentFar);
        stringF->SetLineAlignment(StringAlignmentFar);
        break;

    case CustomLabelAlignment::MiddleCenter:
        stringF->SetAlignment(StringAlignmentCenter);
        stringF->SetLineAlignment(StringAlignmentCenter);
        break;

    case CustomLabelAlignment::MiddleLeft:
        stringF->SetAlignment(StringAlignmentNear);
        stringF->SetLineAlignment(StringAlignmentCenter);
        break;

    case CustomLabelAlignment::MiddleRight:
        stringF->SetAlignment(StringAlignmentFar);
        stringF->SetLineAlignment(StringAlignmentCenter);
        break;

    case CustomLabelAlignment::TopCenter:
        stringF->SetAlignment(StringAlignmentCenter);
        stringF->SetLineAlignment(StringAlignmentNear);
        break;

    case CustomLabelAlignment::TopLeft:
        stringF->SetAlignment(StringAlignmentNear);
        stringF->SetLineAlignment(StringAlignmentNear);
        break;

    case CustomLabelAlignment::TopRight:
        stringF->SetAlignment(StringAlignmentFar);
        stringF->SetLineAlignment(StringAlignmentNear);
        break;

    default:
        stringF->SetAlignment(StringAlignmentCenter);
        stringF->SetLineAlignment(StringAlignmentCenter);
    }
}


// ========================================================================================
// Start the double buffering process for graphic output.
// ========================================================================================
void CustomLabel::StartDoubleBuffering(HDC hdc)
{
    SaveDC(hdc);
    GetClientRect(hWindow, &m_rcClient);

    m_rx = AfxScaleRatioX();
    m_ry = AfxScaleRatioY();

    // Determine if we are in a Hot mouseover state
    if (HotTestEnable) m_bIsHot = GetProp(hWindow, L"HOT") ? true : false;

    m_memDC = CreateCompatibleDC(hdc);
    m_hbit = CreateCompatibleBitmap(hdc, m_rcClient.right, m_rcClient.bottom);
    SelectBitmap(m_memDC, m_hbit);

    Graphics graphics(m_memDC);
    graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

    DWORD nBackColor = (m_bIsHot ? BackColorHot : BackColor);
    if (IsSelected && AllowSelect)
        nBackColor = BackColorSelected;

    if (LButtonDown == true) {
        nBackColor = BackColorButtonDown;
    }

    // Create the background brush
    SolidBrush backBrush(nBackColor);

    // Paint the background using brush and default pen. 
    int nWidth = (m_rcClient.right - m_rcClient.left);
    int nHeight = (m_rcClient.bottom - m_rcClient.top);
    graphics.FillRectangle(&backBrush, 0, 0, nWidth, nHeight);
}


// ========================================================================================
// Draw the image in graphic buffer if required.
// ========================================================================================
void CustomLabel::DrawImageInBuffer()
{
    switch (CtrlType)
    {
    case CustomLabelType::ImageOnly:
    case CustomLabelType::ImageAndText:
        REAL nLeft = (MarginLeft + ImageOffsetLeft) * m_rx;
        REAL nTop = (MarginTop + ImageOffsetTop) * m_ry;
        REAL nRight = nLeft + (ImageWidth * m_rx);
        REAL nBottom = nTop + (ImageHeight * m_ry);

        RectF rcImage(nLeft, nTop, nRight - nLeft, nBottom - nTop);

        Graphics graphics(m_memDC);
        graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
        graphics.DrawImage(m_bIsHot ? pImageHot : pImage, rcImage);
    }
}


// ========================================================================================
// Draw the text in graphic buffer if required.
// ========================================================================================
void CustomLabel::DrawTextInBuffer()
{
    switch (CtrlType)
    {
    case CustomLabelType::TextOnly:
    case CustomLabelType::ImageAndText:

        wszFontName = AfxGetDefaultFont();
        if (wszText.substr(0, 2) == L"\\u") {
            wszFontName = L"Segoe UI Symbol";
        }

        wszFontNameHot = wszFontName;
        FontFamily fontFamily(m_bIsHot ? wszFontNameHot.c_str() : wszFontName.c_str());

        REAL fontSize = (m_bIsHot ? FontSizeHot : FontSize);
        int fontStyle = FontStyleRegular;

        if (m_bIsHot) {
            if (FontBoldHot) fontStyle |= FontStyleBold;
            if (FontItalicHot) fontStyle |= FontStyleItalic;
            if (FontUnderlineHot) fontStyle |= FontStyleUnderline;
        }
        else {
            if (FontBold) fontStyle |= FontStyleBold;
            if (FontItalic) fontStyle |= FontStyleItalic;
            if (FontUnderline) fontStyle |= FontStyleUnderline;
        }

        Font         font(&fontFamily, fontSize, fontStyle, Unit::UnitPoint);
        SolidBrush   textBrush(m_bIsHot ? TextColorHot : TextColor);

        StringFormat stringF(StringFormatFlagsNoWrap);
        stringF.SetTrimming(StringTrimmingEllipsisWord);
        SetTextAlignment(&stringF);

        if (TextCharacterExtra)
            SetTextCharacterExtra(m_memDC, TextCharacterExtra);

        REAL nLeft = (MarginLeft + ImageWidth + ImageOffsetLeft + TextOffsetLeft) * m_rx;
        REAL nTop = (MarginTop + TextOffsetTop) * m_ry;
        REAL nRight = m_rcClient.right - (MarginRight * m_rx);
        REAL nBottom = m_rcClient.bottom - (MarginBottom * m_ry);

        RectF rcText(nLeft, nTop, nRight - nLeft, nBottom - nTop);

        Graphics graphics(m_memDC);
        graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
        graphics.DrawString(wszText.c_str(), -1, &font, rcText, &stringF, &textBrush);
    }

}


// ========================================================================================
// Draw the horizontal or vertical line labels in graphic buffer if required.
// ========================================================================================
void CustomLabel::DrawLabelInBuffer()
{
    switch (CtrlType)
    {
    case CustomLabelType::LineHorizontal:
        REAL nLeft = MarginLeft * m_rx;
        REAL nTop = (MarginTop + TextOffsetTop) * m_ry;
        REAL nRight = m_rcClient.right - (MarginRight * m_rx);
        REAL nBottom = nTop;
        ARGB clrPen = (m_bIsHot ? LineColorHot : LineColor);
        Pen pen(clrPen, LineWidth);
        // Draw the horizontal line centered taking margins into account
        Graphics graphics(m_memDC);
        graphics.DrawLine(&pen, nLeft, nTop, nRight, nBottom);

        //case CustomLabelType::LineVertical:
        //	ARGB clrPen = (bIsHot ? pData->LineColorHot : pData->LineColor);
        //	Pen pen(clrPen, pData->LineWidth);
            // Draw the vertical line centered taking margins into account
            //graphics.DrawLine(&pen, rcDraw.GetLeft(), rcDraw.GetTop(), rcDraw.GetRight(), rcDraw.GetBottom());
    }
}


// ========================================================================================
// Draw the little "notch" selection indicator that appears in the main menu
// for selected menu items.
// ========================================================================================
void CustomLabel::DrawNotchInBuffer()
{
    // If selection mode is enabled then draw the little right hand side notch
    if (IsSelected && AllowNotch) {
        // Create the background brush
        SolidBrush backBrush(SelectorColor);
        // Need to center the notch vertically
        REAL nNotchHalfHeight = (16 * m_ry) / 2;
        REAL nTop = (m_rcClient.bottom / 2) - nNotchHalfHeight;
        PointF point1((REAL)m_rcClient.right, nTop);
        PointF point2((REAL)m_rcClient.right - (8 * m_rx), nTop + nNotchHalfHeight);
        PointF point3((REAL)m_rcClient.right, nTop + (nNotchHalfHeight * 2));
        PointF points[3] = { point1, point2, point3 };
        PointF* pPoints = points;
        Graphics graphics(m_memDC);
        graphics.FillPolygon(&backBrush, pPoints, 3);
    }
}


// ========================================================================================
// Draw the borders in graphic buffer if required.
// ========================================================================================
void CustomLabel::DrawBordersInBuffer()
{
    // Finally, draw any applicable border around the control after everything
    // else has been painted.
    switch (CtrlType)
    {
    case CustomLabelType::ImageOnly:
    case CustomLabelType::ImageAndText:
    case CustomLabelType::TextOnly:
    {
        if (BorderVisible == true) {
            ARGB clrPen = (m_bIsHot ? BorderColorHot : BorderColor);
            Pen pen(BackColor, BorderWidth);
            RectF rectF(0, 0, (REAL)m_rcClient.right - BorderWidth, (REAL)m_rcClient.bottom - BorderWidth);
            Graphics graphics(m_memDC);
            graphics.DrawRectangle(&pen, rectF);
        }
        if (AllowTabStop == true) {
            ARGB clrPen = (GetFocus() == hWindow ? BorderColorHot : BorderColor);
            Pen pen(BackColor, 1);
            RectF rectF(0, 0, (REAL)m_rcClient.right - 1, (REAL)m_rcClient.bottom - 1);
            Graphics graphics(m_memDC);
            graphics.DrawRectangle(&pen, rectF);
        }
        break;
    }
    }
}


// ========================================================================================
// End the double buffering process for graphic output.
// ========================================================================================
void CustomLabel::EndDoubleBuffering(HDC hdc)
{
    // Copy the entire memory bitmap to the main display
    BitBlt(hdc, 0, 0, m_rcClient.right, m_rcClient.bottom, m_memDC, 0, 0, SRCCOPY);

    // Restore the original state of the DC
    RestoreDC(hdc, -1);

    // Cleanup
    DeleteObject(m_hbit);
    DeleteDC(m_memDC);
}


// ========================================================================================
// Generic function to load a non-standard image (eg. PNG) from the resource file.
// ========================================================================================
Gdiplus::Bitmap* LoadImageFromResource(HMODULE hMod, const wchar_t* resid, const wchar_t* restype)
{
    // Stick a new line into your.rc file :
    // IDI_MY_IMAGE_FILE    PNG      "foo.png"
    // And make sure IDI_MY_IMAGE_FILE is defined as an integer in your resource.h header file.
    // #define IDI_MY_IMAGE_FILE               131
    // Then to load the image at runtime :
    // Gdiplus::Bitmap * pBmp = LoadImageFromResource(hInstance, MAKEINTRESOURCE(IDI_MY_IMAGE_FILE), L"PNG");

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

    if (pStream) {
        pStream->Release();
        pStream = nullptr;
    }

    if (hGlobal) {
        GlobalFree(hGlobal);
        hGlobal = nullptr;
    }

    return pBmp;
}


// ========================================================================================
// Windows procedure for the custom control.
// ========================================================================================
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
            SetCapture(hWnd);
            pData->LButtonDown = true;
            AfxRedrawWindow(hWnd);

            // We do not set the button IsSelected to true here because we leave it to 
            // the user to decide if after clicking the label whether to set the label
            // to selected or not.
            PostMessage(pData->hParent, MSG_CUSTOMLABEL_CLICK, (WPARAM)pData->CtrlId, (LPARAM)hWnd);
        }
        return 0;
        break;


    case WM_LBUTTONUP:
    case WM_CAPTURECHANGED:
        if (pData) {
            // Allow the label to repaint in case different color has been specified
            // for button down (eg. if label is acting like a button).
            pData->LButtonDown = false;
            AfxRedrawWindow(hWnd);
            ReleaseCapture();
        }
        return 0;
        break;


    case WM_SETFOCUS:
    case WM_KILLFOCUS:
        AfxRedrawWindow(hWnd);
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
        return 0;
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


// ========================================================================================
// Get the custom control data pointer.
// ========================================================================================
CustomLabel* CustomLabel_GetOptions(HWND hCtrl)
{
    CustomLabel* pData = (CustomLabel*)GetWindowLongPtr(hCtrl, 0);
    return pData;
}


// ========================================================================================
// Store the custom control data pointer.
// ========================================================================================
int CustomLabel_SetOptions(HWND hCtrl, CustomLabel* pData)
{
    if (pData == nullptr) return 0;

    if (pData->wszToolTip.length()) {
        if (pData->hToolTip) {
            AfxSetTooltipText(pData->hToolTip, hCtrl, pData->wszToolTip);
        }
    }

    AfxRemoveWindowStyle(hCtrl, WS_TABSTOP);
    if (pData->AllowTabStop) AfxAddWindowStyle(hCtrl, WS_TABSTOP);
    
    SetWindowLongPtr(hCtrl, 0, (LONG_PTR)pData);
    AfxRedrawWindow(hCtrl);

    return 0;
}


// ========================================================================================
// Set the text for the custom control.
// ========================================================================================
void CustomLabel_SetText(HWND hCtrl, std::wstring wszText)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->wszText = wszText;
        pData->wszTextHot = wszText;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Set the text foreground color for the custom control.
// ========================================================================================
void CustomLabel_SetTextColor(HWND hCtrl, DWORD TextColor)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->TextColor = TextColor;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Set the text foreground hot color for the custom control.
// ========================================================================================
void CustomLabel_SetTextColorHot(HWND hCtrl, DWORD TextColorHot)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->TextColorHot = TextColorHot;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Set the text background color for the custom control.
// ========================================================================================
void CustomLabel_SetBackColor(HWND hCtrl, DWORD BackColor)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->BackColor = BackColor;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Set the text background hot color for the custom control.
// ========================================================================================
void CustomLabel_SetBackColorHot(HWND hCtrl, DWORD BackColorHot)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->BackColorHot = BackColorHot;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Get the text background color for the custom control.
// ========================================================================================
DWORD CustomLabel_GetBackColor(HWND hCtrl)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        return pData->BackColor;
    }
    return COLOR_BLACK;
}


// ========================================================================================
// Set the tooltip text for the custom control.
// ========================================================================================
void CustomLabel_SetToolTip(HWND hCtrl, std::wstring wszText)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->wszToolTip = wszText;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Get the text for the custom control.
// ========================================================================================
std::wstring CustomLabel_GetText(HWND hCtrl)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        return pData->wszText;
    }
    return L"";
}


// ========================================================================================
// Set the border (width and colors)  for the custom control.
// ========================================================================================
void CustomLabel_SetBorder(HWND hCtrl, REAL BorderWidth, DWORD BorderColor, DWORD BorderColorHot)
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


// ========================================================================================
// Set the mouse pointer (normal and hot) for the custom control.
// ========================================================================================
void CustomLabel_SetMousePointer(HWND hCtrl, CustomLabelPointer NormalPointer, CustomLabelPointer HotPointer)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->Pointer = NormalPointer;
        pData->PointerHot = HotPointer;
        CustomLabel_SetOptions(hCtrl, pData);
    }

}


// ========================================================================================
// Set the text offset for the custom control (useful to ensure text is not jammed
// up against a border).
// ========================================================================================
void CustomLabel_SetTextOffset(HWND hCtrl, int OffsetLeft, int OffsetTop)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->TextOffsetLeft = OffsetLeft;
        pData->TextOffsetTop = OffsetTop;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Set the user defined text data (wstring) for the custom control.
// ========================================================================================
void CustomLabel_SetUserData(HWND hCtrl, std::wstring wszText)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->UserData = wszText;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Get the user defined text data (wstring) for the custom control.
// ========================================================================================
std::wstring CustomLabel_GetUserData(HWND hCtrl)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        return pData->UserData;
    }
    return L"";
}


// ========================================================================================
// Set the user defined integer data (int) for the custom control.
// ========================================================================================
void CustomLabel_SetUserDataInt(HWND hCtrl, int value)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->UserDataInt = value;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Get the user defined integer data (int) for the custom control.
// ========================================================================================
int CustomLabel_GetUserDataInt(HWND hCtrl)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        return pData->UserDataInt;
    }
    return 0;
}


// ========================================================================================
// Set the font for the custom control.
// ========================================================================================
void CustomLabel_SetFont(HWND hCtrl, std::wstring wszFontName, int FontSize, bool FontBold)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->wszFontName = wszFontName;
        pData->wszFontNameHot = wszFontName;
        pData->FontSize = (REAL)FontSize;
        pData->FontSizeHot = (REAL)FontSize;
        pData->FontBold = FontBold;
        pData->FontBoldHot = pData->FontBold;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Set the selected status for the custom control. Useful for toggling menu items.
// ========================================================================================
void CustomLabel_Select(HWND hCtrl, bool IsSelected)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        bool redraw = (pData->IsSelected != IsSelected) ? true : false;
        pData->IsSelected = IsSelected;
        CustomLabel_SetOptions(hCtrl, pData);
        if (redraw) AfxRedrawWindow(hCtrl);
    }
}


// ========================================================================================
// Creates a simple "dumb" label that basically just makes it easier to deal
// with coloring. No hot tracking or click notifications.
// ========================================================================================
HWND CustomLabel_SimpleLabel(HWND hParent, int CtrlId, std::wstring wszText,
    DWORD TextColor, DWORD BackColor, CustomLabelAlignment alignment, 
    int nLeft, int nTop, int nWidth, int nHeight)
{
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
        pData->AllowTabStop = false;
        CustomLabel_SetOptions(hCtl, pData);
    }

    return hCtl;
}


// ========================================================================================
// Creates a simple label that acts like a push button.
// ========================================================================================
HWND CustomLabel_ButtonLabel(HWND hParent, int CtrlId, std::wstring wszText,
    DWORD TextColor, DWORD BackColor, DWORD BackColorHot, DWORD BackColorButtonDown, DWORD FocusBorderColor,
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
        pData->AllowTabStop = true;
        pData->BorderColorHot = FocusBorderColor;
        pData->BorderColor = BackColor;
        CustomLabel_SetOptions(hCtl, pData);
    }

    return hCtl;
}


// ========================================================================================
// Creates a simple label that only displays an image.
// ========================================================================================
HWND CustomLabel_SimpleImageLabel(HWND hParent, int CtrlId,
    std::wstring wszImage, std::wstring wszImageHot, 
    int ImageWidth, int ImageHeight,
    int nLeft, int nTop, int nWidth, int nHeight)
{
    // Creates a simple "dumb" label that basically just makes it easier to deal
    // with coloring. No hot tracking or click notifications.
    CustomLabel* pData = nullptr;
    
    HWND hCtl = CreateCustomLabel(
    hParent, CtrlId,
    CustomLabelType::ImageOnly,
    nLeft, nTop, nWidth, nHeight);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->HotTestEnable = false;
        pData->BackColor = COLOR_BLACK;
        pData->BackColorButtonDown = pData->BackColor;
        pData->ImageWidth = 68;
        pData->ImageHeight = 68;
        pData->pImage = LoadImageFromResource(pData->hInst, MAKEINTRESOURCE(IDB_LOGO), L"PNG");
        pData->pImageHot = LoadImageFromResource(pData->hInst, MAKEINTRESOURCE(IDB_LOGO), L"PNG");
        CustomLabel_SetOptions(hCtl, pData);
    }
    return hCtl;
}


// ========================================================================================
// Create the custom label control.
// ========================================================================================
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


