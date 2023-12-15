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
    switch (text_alignment)
    {
    case CustomLabelAlignment::bottom_center:
        stringF->SetAlignment(StringAlignmentCenter);
        stringF->SetLineAlignment(StringAlignmentFar);
        break;

    case CustomLabelAlignment::bottom_left:
        stringF->SetAlignment(StringAlignmentNear);
        stringF->SetLineAlignment(StringAlignmentFar);
        break;

    case CustomLabelAlignment::bottom_right:
        stringF->SetAlignment(StringAlignmentFar);
        stringF->SetLineAlignment(StringAlignmentFar);
        break;

    case CustomLabelAlignment::middle_center:
        stringF->SetAlignment(StringAlignmentCenter);
        stringF->SetLineAlignment(StringAlignmentCenter);
        break;

    case CustomLabelAlignment::middle_left:
        stringF->SetAlignment(StringAlignmentNear);
        stringF->SetLineAlignment(StringAlignmentCenter);
        break;

    case CustomLabelAlignment::middle_right:
        stringF->SetAlignment(StringAlignmentFar);
        stringF->SetLineAlignment(StringAlignmentCenter);
        break;

    case CustomLabelAlignment::top_center:
        stringF->SetAlignment(StringAlignmentCenter);
        stringF->SetLineAlignment(StringAlignmentNear);
        break;

    case CustomLabelAlignment::top_left:
        stringF->SetAlignment(StringAlignmentNear);
        stringF->SetLineAlignment(StringAlignmentNear);
        break;

    case CustomLabelAlignment::top_right:
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
    if (hot_test_enable) m_is_hot = GetProp(hWindow, L"HOT") ? true : false;

    m_memDC = CreateCompatibleDC(hdc);
    m_hbit = CreateCompatibleBitmap(hdc, m_rcClient.right, m_rcClient.bottom);
    SelectBitmap(m_memDC, m_hbit);

    Graphics graphics(m_memDC);
    graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

    DWORD nback_color = (m_is_hot ? back_color_hot : back_color);
    if (LButtonDown) nback_color = back_color_button_down;
    if (is_selected) nback_color = back_color_selected;

    // Create the background brush
    SolidBrush back_brush(nback_color);

    // Paint the background using brush and default pen. 
    int nWidth = (m_rcClient.right - m_rcClient.left);
    int nHeight = (m_rcClient.bottom - m_rcClient.top);
    graphics.FillRectangle(&back_brush, 0, 0, nWidth, nHeight);
}


// ========================================================================================
// Draw the image in graphic buffer if required.
// ========================================================================================
void CustomLabel::DrawImageInBuffer()
{
    switch (CtrlType)
    {
    case CustomLabelType::image_only:
    case CustomLabelType::image_and_text:
        REAL nLeft = (image_offset_left * m_rx);
        REAL nTop = (image_offset_top * m_ry);
        REAL nRight = nLeft + (image_width * m_rx);
        REAL nBottom = nTop + (image_height * m_ry);
        RectF rcImage(nLeft, nTop, nRight - nLeft, nBottom - nTop);

        Graphics graphics(m_memDC);
        graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
        graphics.DrawImage(m_is_hot ? pImageHot : pImage, rcImage);
    }
}


// ========================================================================================
// Draw the text in graphic buffer if required.
// ========================================================================================
void CustomLabel::DrawTextInBuffer()
{
    switch (CtrlType)
    {
    case CustomLabelType::text_only:
    case CustomLabelType::image_and_text:

        font_name = AfxGetDefaultFont();
        if (text.substr(0, 2) == L"\\u") {
            font_name = L"Segoe UI Symbol";
        }

        font_name_hot = font_name;
        FontFamily fontFamily(m_is_hot ? font_name_hot.c_str() : font_name.c_str());

        REAL fontSize = 0;
        int fontStyle = FontStyleRegular;

        if (m_is_hot || is_selected) {
            if (font_bold_hot) fontStyle |= FontStyleBold;
            if (font_italic_hot) fontStyle |= FontStyleItalic;
            if (font_underline_hot) fontStyle |= FontStyleUnderline;
            fontSize = font_size_hot;
        }
        else {
            if (font_bold) fontStyle |= FontStyleBold;
            if (font_italic) fontStyle |= FontStyleItalic;
            if (font_underline) fontStyle |= FontStyleUnderline;
            fontSize = font_size;
        }

        Font         font(&fontFamily, fontSize, fontStyle, Unit::UnitPoint);
        SolidBrush   text_brush((m_is_hot || is_selected) ? text_color_hot : text_color);

        StringFormat stringF(StringFormatFlagsNoWrap);
        stringF.SetTrimming(StringTrimmingEllipsisWord);
        SetTextAlignment(&stringF);

        REAL nLeft = (image_width + image_offset_left + text_offset_left) * m_rx;
        REAL nTop = (text_offset_top * m_ry);
        REAL nRight = (REAL)m_rcClient.right;
        REAL nBottom = (REAL)m_rcClient.bottom;

        RectF rcText(nLeft, nTop, nRight - nLeft, nBottom - nTop);

        Graphics graphics(m_memDC);
        graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
        graphics.DrawString(text.c_str(), -1, &font, rcText, &stringF, &text_brush);

        if (is_selected && is_selected_underline) {
            RectF boundRect;
            RectF layoutRect(0.0f, 0.0f, 1000.0f, 50.0f);
            graphics.MeasureString(text.c_str(), (int)text.length(),
                &font, layoutRect, &stringF, &boundRect);
            REAL textLength = boundRect.Width;

            nLeft = (nRight - nLeft - textLength) * 0.5f;
            nRight = nLeft + textLength;
            nTop = nBottom - (6 * m_ry);
            
            ARGB clrPen = selected_underline_color;
            Pen pen(clrPen, (REAL)selected_underline_width);
            // Draw the horizontal line
            graphics.DrawLine(&pen, nLeft, nTop, nRight, nTop);
        }
    }

}


// ========================================================================================
// Draw the horizontal or vertical line labels in graphic buffer if required.
// ========================================================================================
void CustomLabel::DrawLabelInBuffer()
{
    switch (CtrlType)
    {
    case CustomLabelType::line_horizontal:
    {

        REAL nLeft = 0;
        REAL nTop = (text_offset_top * m_ry);
        REAL nRight = (REAL)m_rcClient.right;
        REAL nBottom = nTop;
        ARGB clrPen = line_color;
        Pen pen(clrPen, line_width);
        // Draw the horizontal line
        Graphics graphics(m_memDC);
        graphics.DrawLine(&pen, nLeft, nTop, nRight, nBottom);
    }
    break;

    case CustomLabelType::line_vertical:
    {
        REAL nLeft = (REAL)m_rcClient.right / 2;
        REAL nTop = (text_offset_top * m_ry);
        REAL nRight = nLeft;
        REAL nBottom = (REAL)m_rcClient.bottom;
        ARGB clrPen = line_color;
        Pen pen(clrPen, line_width);
        // Draw the vertical line
        Graphics graphics(m_memDC);
        graphics.DrawLine(&pen, nLeft, nTop, nRight, nBottom);
    }
    break;

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
    case CustomLabelType::image_only:
    case CustomLabelType::image_and_text:
    case CustomLabelType::text_only:
    {
        //if (BorderVisible == true) {
        //    ARGB clrPen = (m_is_hot ? BorderColorHot : BorderColor);
        //    Pen pen(back_color, BorderWidth);
        //    RectF rectF(0, 0, (REAL)m_rcClient.right - BorderWidth, (REAL)m_rcClient.bottom - BorderWidth);
        //    Graphics graphics(m_memDC);
        //    graphics.DrawRectangle(&pen, rectF);
        //}
        if (allow_tab_stop == true) {
            ARGB clrPen = (GetFocus() == hWindow ? border_color_hot : back_color);
            Pen pen(clrPen, 1);
            int nWidth = (m_rcClient.right - m_rcClient.left) - 1;
            int nHeight = (m_rcClient.bottom - m_rcClient.top) - 1;
            Graphics graphics(m_memDC);
            graphics.DrawRectangle(&pen, 0, 0, nWidth, nHeight);
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
    static CustomLabel* pData = nullptr;

    if (uMsg != WM_CREATE) {
        pData = (CustomLabel*)GetWindowLongPtr(hWnd, 0);
    }
                 

    switch (uMsg)
    {

    case WM_KEYDOWN:
    {
        // Parent to handle the TAB navigation key to move amongst constrols.
        if (wParam == VK_TAB) {
            if (SendMessage(pData->hParent, uMsg, wParam, lParam) == TRUE)
                return 0;
        }

        if (wParam == VK_SPACE) {
            SendMessage(hWnd, WM_LBUTTONDOWN, wParam, lParam);
            SendMessage(hWnd, WM_LBUTTONUP, wParam, lParam);
            return 0;
        }
    }
    break;


    case WM_SETCURSOR:
    {
        LPWSTR IDCPointer = IDC_HAND;

        if (pData) {
            if (pData->hot_test_enable) {
                IDCPointer = (pData->pointer_hot == CustomLabelPointer::hand) ? IDC_HAND : IDC_ARROW;
            }
            else {
                IDCPointer = (pData->pointer == CustomLabelPointer::hand) ? IDC_HAND : IDC_ARROW;
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
        TRACKMOUSEEVENT trackMouse{};

        if (pData->hot_test_enable) {
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
        if (pData->hot_test_enable) {
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

    if (pData->tooltip_text.length()) {
        if (pData->hToolTip) {
            AfxSetTooltipText(pData->hToolTip, hCtrl, pData->tooltip_text);
        }
    }

    AfxRemoveWindowStyle(hCtrl, WS_TABSTOP);
    if (pData->allow_tab_stop) AfxAddWindowStyle(hCtrl, WS_TABSTOP);
    
    SetWindowLongPtr(hCtrl, 0, (LONG_PTR)pData);
    AfxRedrawWindow(hCtrl);

    return 0;
}


// ========================================================================================
// Set enable/disable hot testing for the custom control.
// ========================================================================================
void CustomLabel_SetHotTesting(HWND hCtrl, bool enable_hot_testing)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->hot_test_enable = enable_hot_testing;
        pData->font_name_hot = pData->font_name;
        pData->font_size_hot = pData->font_size;
        pData->font_bold_hot = pData->font_bold;
        pData->back_color_hot = pData->back_color;
        pData->text_color_hot = pData->text_color;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Set enable/disable Selected status for the custom control.
// ========================================================================================
void CustomLabel_SetSelected(HWND hCtrl, bool is_selected)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->is_selected = is_selected;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Get the Selected status for the custom control.
// ========================================================================================
bool CustomLabel_GetSelected(HWND hCtrl)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        return pData->is_selected;
    }
    return false;
}


// ========================================================================================
// Set enable/disable Selected Underline and Color status for the custom control.
// ========================================================================================
void CustomLabel_SetSelectedUnderline(HWND hCtrl, bool enable_underline, DWORD underline_color, int line_width)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->is_selected_underline = enable_underline;
        pData->selected_underline_color = underline_color;
        pData->selected_underline_width = line_width;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}




// ========================================================================================
// Set the text for the custom control.
// ========================================================================================
void CustomLabel_SetText(HWND hCtrl, std::wstring text)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->text = text;
        pData->text_hot = text;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Set the text foreground color for the custom control.
// ========================================================================================
void CustomLabel_SetTextColor(HWND hCtrl, DWORD text_color)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->text_color = text_color;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Set the text foreground hot color for the custom control.
// ========================================================================================
void CustomLabel_SetTextColorHot(HWND hCtrl, DWORD text_color_hot)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->text_color_hot = text_color_hot;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Set the text background color for the custom control.
// ========================================================================================
void CustomLabel_SetBackColor(HWND hCtrl, DWORD back_color)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->back_color = back_color;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Set the text background hot color for the custom control.
// ========================================================================================
void CustomLabel_SetBackColorHot(HWND hCtrl, DWORD back_color_hot)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->back_color_hot = back_color_hot;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Set the text background Selected color for the custom control.
// ========================================================================================
void CustomLabel_SetBackColorSelected(HWND hCtrl, DWORD back_color_selected)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->back_color_selected = back_color_selected;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Set the border color for the custom control.
// ========================================================================================
void CustomLabel_SetBorderColor(HWND hCtrl, DWORD border_color)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->border_color = border_color;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Set the border hot color for the custom control.
// ========================================================================================
void CustomLabel_SetBorderColorHot(HWND hCtrl, DWORD border_color_hot)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->border_color_hot = border_color_hot;
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
        return pData->back_color;
    }
    return COLOR_BLACK;
}


// ========================================================================================
// Set the tooltip text for the custom control.
// ========================================================================================
void CustomLabel_SetToolTip(HWND hCtrl, std::wstring text)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->tooltip_text = text;
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
        return pData->text;
    }
    return L"";
}


// ========================================================================================
// Set the border (width and colors)  for the custom control.
// ========================================================================================
void CustomLabel_SetBorder(HWND hCtrl, REAL border_width, DWORD border_color, DWORD border_color_hot)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->border_visible = true;
        pData->border_width = border_width;
        pData->border_color = border_color;
        pData->border_color_hot = border_color_hot;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Set the mouse pointer (normal and hot) for the custom control.
// ========================================================================================
void CustomLabel_SetMousePointer(HWND hCtrl, CustomLabelPointer normal_pointer, CustomLabelPointer hot_pointer)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->pointer = normal_pointer;
        pData->pointer_hot = hot_pointer;
        CustomLabel_SetOptions(hCtrl, pData);
    }

}


// ========================================================================================
// Set the text offset for the custom control (useful to ensure text is not jammed
// up against a border).
// ========================================================================================
void CustomLabel_SetTextOffset(HWND hCtrl, int offset_left, int offset_top)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->text_offset_left = offset_left;
        pData->text_offset_top = offset_top;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Set the image offset for the custom control (useful for centering image).
// ========================================================================================
void CustomLabel_SetImageOffset(HWND hCtrl, int offset_left, int offset_top)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->image_offset_left = offset_left;
        pData->image_offset_top = offset_top;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Set the user defined text data (wstring) for the custom control.
// ========================================================================================
void CustomLabel_SetUserData(HWND hCtrl, std::wstring text)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->user_data = text;
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
        return pData->user_data;
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
        pData->user_data_int = value;
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
        return pData->user_data_int;
    }
    return 0;
}


// ========================================================================================
// Set the font for the custom control.
// ========================================================================================
void CustomLabel_SetFont(HWND hCtrl, std::wstring font_name, int font_size, bool font_bold)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->font_name = font_name;
        pData->font_name_hot = font_name;
        pData->font_size = (REAL)font_size;
        pData->font_size_hot = (REAL)font_size;
        pData->font_bold = font_bold;
        pData->font_bold_hot = pData->font_bold;
        pData->pointer_hot = CustomLabelPointer::arrow;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}

// ========================================================================================
// Set the hot font for the custom control.
// ========================================================================================
void CustomLabel_SetFontHot(HWND hCtrl, std::wstring font_name, int font_size, bool font_bold)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->font_name_hot = font_name;
        pData->font_size_hot = (REAL)font_size;
        pData->font_bold_hot = pData->font_bold;
        pData->hot_test_enable = true;
        pData->back_color_hot = pData->back_color;
        pData->text_color_hot = pData->text_color;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Set the normal and hot image for an Image label.
// ========================================================================================
void CustomLabel_SetImages(HWND hCtrl, int image_id, int image_hot_id)
{
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->pImage = LoadImageFromResource(pData->hInst, MAKEINTRESOURCE(image_id), L"PNG");
        pData->pImageHot = LoadImageFromResource(pData->hInst, MAKEINTRESOURCE(image_hot_id), L"PNG");
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Creates a simple "dumb" label that basically just makes it easier to deal
// with coloring. 
// ========================================================================================
HWND CustomLabel_SimpleLabel(HWND hParent, int CtrlId, std::wstring text,
    DWORD text_color, DWORD back_color, CustomLabelAlignment alignment, 
    int nLeft, int nTop, int nWidth, int nHeight)
{
    CustomLabel* pData = nullptr;

    HWND hCtl = CreateCustomLabel(
        hParent, CtrlId, CustomLabelType::text_only,
        nLeft, nTop, nWidth, nHeight);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->text = text;
        pData->hot_test_enable = false;
        pData->back_color = back_color;
        pData->text_color = text_color;
        pData->back_color_button_down = back_color;
        pData->text_alignment = alignment;
        pData->allow_tab_stop = false;
        CustomLabel_SetOptions(hCtl, pData);
    }

    return hCtl;
}


// ========================================================================================
// Creates a simple label that acts like a push button.
// ========================================================================================
HWND CustomLabel_ButtonLabel(HWND hParent, int CtrlId, std::wstring text,
    DWORD text_color, DWORD back_color, DWORD back_color_hot, DWORD back_color_button_down, DWORD focus_border_color,
    CustomLabelAlignment alignment, int nLeft, int nTop, int nWidth, int nHeight)
{
    // Creates a simple button type of label
    CustomLabel* pData = nullptr;

    HWND hCtl = CreateCustomLabel(
        hParent, CtrlId, CustomLabelType::text_only,
        nLeft, nTop, nWidth, nHeight);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->text = text;
        pData->text_hot = text;
        pData->hot_test_enable = true;
        pData->back_color = back_color;
        pData->back_color_hot = back_color_hot;
        pData->back_color_button_down = back_color_button_down;
        pData->text_color = text_color;
        pData->text_color_hot = text_color;
        pData->text_alignment = alignment;
        pData->allow_tab_stop = true;
        pData->border_color_hot = focus_border_color;
        pData->border_color = back_color;
        CustomLabel_SetOptions(hCtl, pData);
    }

    return hCtl;
}


// ========================================================================================
// Creates a simple label that only displays an image.
// ========================================================================================
HWND CustomLabel_SimpleImageLabel(HWND hParent, int CtrlId,
    int image_id, int image_hot_id, 
    int image_width, int image_height,
    int nLeft, int nTop, int nWidth, int nHeight)
{
    // Creates a simple "dumb" label that basically just makes it easier to deal
    // with coloring. No hot tracking or click notifications.
    CustomLabel* pData = nullptr;
    
    HWND hCtl = CreateCustomLabel(
    hParent, CtrlId,
    CustomLabelType::image_only,
    nLeft, nTop, nWidth, nHeight);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->hot_test_enable = true;
        pData->back_color = COLOR_BLACK;
        pData->back_color_hot = pData->back_color;
        pData->back_color_button_down = pData->back_color;
        pData->image_width = image_width;
        pData->image_height = image_height;
        pData->pImage = LoadImageFromResource(pData->hInst, MAKEINTRESOURCE(image_id), L"PNG");
        pData->pImageHot = LoadImageFromResource(pData->hInst, MAKEINTRESOURCE(image_hot_id), L"PNG");
        CustomLabel_SetOptions(hCtl, pData);
    }
    return hCtl;
}


// ========================================================================================
// Creates a simple horizontal line label.
// ========================================================================================
HWND CustomLabel_HorizontalLine(HWND hParent, int CtrlId, DWORD line_color, DWORD back_color,
    int nLeft, int nTop, int nWidth, int nHeight)
{
    // Creates a simple "dumb" label that basically just makes it easier to deal
    // with coloring. No hot tracking or click notifications.
    CustomLabel* pData = nullptr;

    HWND hCtl = CreateCustomLabel(
        hParent, CtrlId,
        CustomLabelType::line_horizontal,
        nLeft, nTop, nWidth, nHeight);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->hot_test_enable = false;
        pData->back_color = back_color;
        pData->line_color = line_color;
        pData->back_color_button_down = back_color;
        pData->allow_tab_stop = false;
        CustomLabel_SetOptions(hCtl, pData);
    }
    return hCtl;
}

// ========================================================================================
// Creates a simple vertical line label.
// ========================================================================================
HWND CustomLabel_VerticalLine(HWND hParent, int CtrlId, DWORD line_color, DWORD back_color,
    int nLeft, int nTop, int nWidth, int nHeight)
{
    // Creates a simple "dumb" label that basically just makes it easier to deal
    // with coloring. No hot tracking or click notifications.
    CustomLabel* pData = nullptr;

    HWND hCtl = CreateCustomLabel(
        hParent, CtrlId,
        CustomLabelType::line_vertical,
        nLeft, nTop, nWidth, nHeight);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->hot_test_enable = false;
        pData->back_color = back_color;
        pData->line_color = line_color;
        pData->back_color_button_down = back_color;
        pData->allow_tab_stop = false;
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
    std::wstring class_name_text(L"CUSTOMLABEL_CONTROL");

    WNDCLASSEX wcex{};
    
    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hWndParent, GWLP_HINSTANCE);

    if (GetClassInfoEx(hInst, class_name_text.c_str(), &wcex) == 0) {
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
        wcex.lpszClassName = class_name_text.c_str();
        if(RegisterClassEx(&wcex) == 0) return 0;
    }
        
    float rx = AfxScaleRatioX();
    float ry = AfxScaleRatioY();
   

    HWND hCtl =
        CreateWindowEx(0, class_name_text.c_str(), L"",
            WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
            (int)(nLeft * rx), (int)(nTop * ry), (int)(nWidth * rx), (int)(nHeight * ry),
            hWndParent, (HMENU)CtrlId, hInst, (LPVOID)NULL);

    if (hCtl) {
        CustomLabel* pData = new CustomLabel;

        pData->hWindow = hCtl;
        pData->hParent = hWndParent;
        pData->hInst = hInst;
        pData->CtrlId = (int)CtrlId;
        pData->CtrlType = nCtrlType;

        pData->font_name = AfxGetDefaultFont();
        pData->font_size = 9;
        pData->font_name_hot = pData->font_name;
        pData->font_size_hot = pData->font_size;

        pData->hToolTip = AfxAddTooltip(hCtl, L"", FALSE, FALSE);

        if (nCtrlType == CustomLabelType::line_horizontal || 
            nCtrlType == CustomLabelType::line_vertical) {
            pData->hot_test_enable = false;
            pData->border_visible = false;
        }

        pData->pointer = CustomLabelPointer::arrow;
        pData->pointer_hot = CustomLabelPointer::arrow;

        CustomLabel_SetOptions(hCtl, pData);
    }
   
    return hCtl;
}


