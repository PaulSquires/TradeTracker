/*

MIT License

Copyright(c) 2023-2024 Paul Squires

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
void CustomLabel::SetTextAlignment(StringFormat* stringF) {
    switch (text_alignment) {
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
void CustomLabel::StartDoubleBuffering(HDC hdc) {
    SaveDC(hdc);
    GetClientRect(hWindow, &m_rcClient);

    m_rx = AfxScaleRatioX();
    m_ry = AfxScaleRatioY();

    // Determine if we are in a Hot mouseover state
    if (hot_test_enable) is_hot = GetProp(hWindow, L"HOT") ? true : false;

    m_memDC = CreateCompatibleDC(hdc);
    m_hbit = CreateCompatibleBitmap(hdc, m_rcClient.right, m_rcClient.bottom);
    SelectBitmap(m_memDC, m_hbit);

    Graphics graphics(m_memDC);
    graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

    DWORD brush_back_color = (is_hot ? back_color_hot : back_color);
    if (LButtonDown) brush_back_color = back_color_button_down;
    if (is_selected) brush_back_color = back_color_selected;

    // Create the background brush
    SolidBrush back_brush(brush_back_color);

    // Paint the background using brush and default pen. 
    int width = (m_rcClient.right - m_rcClient.left);
    int height = (m_rcClient.bottom - m_rcClient.top);
    graphics.FillRectangle(&back_brush, 0, 0, width, height);
}


// ========================================================================================
// Draw the image in graphic buffer if required.
// ========================================================================================
void CustomLabel::DrawImageInBuffer() {
    switch (ctrl_type) {
    case CustomLabelType::image_only:
    case CustomLabelType::image_and_text:
        REAL left = (image_offset_left * m_rx);
        REAL top = (image_offset_top * m_ry);
        REAL right = left + (image_width * m_rx);
        REAL bottom = top + (image_height * m_ry);
        RectF rcImage(left, top, right - left, bottom - top);

        Graphics graphics(m_memDC);
        graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);

        Bitmap* p = pImage;
        if (is_hot || is_selected) p = pImageHot;
        graphics.DrawImage(p, rcImage);
    }
}


// ========================================================================================
// Draw the checkbox in graphic buffer if required.
// ========================================================================================
void CustomLabel::DrawCheckBoxInBuffer() {
    font_name = L"Segoe UI Symbol";

    FontFamily fontFamily(font_name.c_str());

    REAL font_size = 10;
    int font_style = FontStyleRegular;

    Font         font(&fontFamily, font_size, font_style, Unit::UnitPoint);
    SolidBrush   text_brush(text_color);

    StringFormat stringF(StringFormatFlagsNoWrap);
    stringF.SetTrimming(StringTrimmingEllipsisWord);
    SetTextAlignment(&stringF);
    
    // Left-justify the checkmark.
    stringF.SetAlignment(StringAlignmentNear);

    // Center the checkmark (top to bottom) in the rectangle.
    stringF.SetLineAlignment(StringAlignmentCenter);

    REAL left = 0;
    REAL top = 0;
    REAL right = left + (14 * m_rx);
    REAL bottom = (REAL)m_rcClient.bottom;

    RectF rcText(left, top, right - left, bottom - top);

    std::wstring check_text = (is_checked) ? L"\u2611" : L"\u2610";
    text_offset_left = (int)right;

    Graphics graphics(m_memDC);
    graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
    graphics.DrawString(check_text.c_str(), -1, &font, rcText, &stringF, &text_brush);
}

// ========================================================================================
// Draw the text in graphic buffer if required.
// ========================================================================================
void CustomLabel::DrawTextInBuffer() {
    switch (ctrl_type) {
    case CustomLabelType::checkbox:
        DrawCheckBoxInBuffer();
        // fallthrough to draw the text
        [[fallthrough]];
    case CustomLabelType::text_only:
    case CustomLabelType::image_and_text:

        font_name = AfxGetDefaultFont();
        if (text.substr(0, 2) == L"\\u") {
            font_name = L"Segoe UI Symbol";
        }

        font_name_hot = font_name;
        FontFamily fontFamily(is_hot ? font_name_hot.c_str() : font_name.c_str());

        REAL font_size = 9;
        int font_style = FontStyleRegular;

        if (is_hot || is_selected) {
            if (font_bold_hot) font_style |= FontStyleBold;
            if (font_italic_hot) font_style |= FontStyleItalic;
            if (font_underline_hot) font_style |= FontStyleUnderline;
            font_size = font_size_hot;
        }
        else {
            if (font_bold) font_style |= FontStyleBold;
            if (font_italic) font_style |= FontStyleItalic;
            if (font_underline) font_style |= FontStyleUnderline;
        }

        Font         font(&fontFamily, font_size, font_style, Unit::UnitPoint);
        SolidBrush   text_brush((is_hot || is_selected) ? text_color_hot : text_color);

        StringFormat stringF(StringFormatFlagsNoWrap);
        stringF.SetTrimming(StringTrimmingEllipsisWord);
        SetTextAlignment(&stringF);

        REAL left = (image_width + image_offset_left + text_offset_left) * m_rx;
        REAL top = (text_offset_top * m_ry);
        REAL right = (REAL)m_rcClient.right;
        REAL bottom = (REAL)m_rcClient.bottom;

        RectF rcText(left, top, right - left, bottom - top);

        Graphics graphics(m_memDC);
        graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
        graphics.DrawString(text.c_str(), -1, &font, rcText, &stringF, &text_brush);

        if (is_selected && is_selected_underline) {
            RectF boundRect;
            RectF layoutRect(0.0f, 0.0f, 1000.0f, 50.0f);
            graphics.MeasureString(text.c_str(), (int)text.length(),
                &font, layoutRect, &stringF, &boundRect);
            REAL text_length = boundRect.Width;

            left = (right - left - text_length) * 0.5f;
            right = left + text_length;
            top = bottom - (6 * m_ry);
            
            ARGB clrPen = selected_underline_color;
            Pen pen(clrPen, (REAL)selected_underline_width);
            // Draw the horizontal line
            graphics.DrawLine(&pen, left, top, right, top);
        }
    }
}


// ========================================================================================
// Draw the horizontal or vertical line labels in graphic buffer if required.
// ========================================================================================
void CustomLabel::DrawLabelInBuffer() {
    switch (ctrl_type) {
    case CustomLabelType::line_horizontal: {
        REAL left = 0;
        REAL top = (text_offset_top * m_ry);
        REAL right = (REAL)m_rcClient.right;
        REAL bottom = top;
        ARGB clrPen = line_color;
        Pen pen(clrPen, line_width);
        // Draw the horizontal line
        Graphics graphics(m_memDC);
        graphics.DrawLine(&pen, left, top, right, bottom);
        break;
    }

    case CustomLabelType::line_vertical: {
        REAL left = (REAL)m_rcClient.right / 2;
        REAL top = (text_offset_top * m_ry);
        REAL right = left;
        REAL bottom = (REAL)m_rcClient.bottom;
        ARGB clrPen = line_color;
        Pen pen(clrPen, line_width);
        // Draw the vertical line
        Graphics graphics(m_memDC);
        graphics.DrawLine(&pen, left, top, right, bottom);
        break;
    }

    }
}


// ========================================================================================
// Draw the borders in graphic buffer if required.
// ========================================================================================
void CustomLabel::DrawBordersInBuffer() {
    // Finally, draw any applicable border around the control after everything
    // else has been painted.
    switch (ctrl_type) {
    case CustomLabelType::image_only:
    case CustomLabelType::image_and_text:
    case CustomLabelType::text_only: 
    case CustomLabelType::checkbox: {
        //if (BorderVisible == true) {
        //    ARGB clrPen = (is_hot ? border_color_hot : border_color);
        //    Pen pen(back_color, border_width);
        //    RectF rectF(0, 0, (REAL)m_rcClient.right - border_width, (REAL)m_rcClient.bottom - border_width);
        //    Graphics graphics(m_memDC);
        //    graphics.DrawRectangle(&pen, rectF);
        //}
        if (allow_tab_stop) {
            REAL rect_border_width = border_width;
            ARGB clrPen = border_color;
            if (GetFocus() == hWindow) {
                rect_border_width = (REAL)AfxScaleX(GetSystemMetrics(SM_CXFOCUSBORDER));
                clrPen = border_color_focus;
            }
            Pen pen(clrPen, (REAL)rect_border_width);

            if (ctrl_type == CustomLabelType::checkbox) {
                pen.SetDashStyle(DashStyleDash);
            }

            int width = (m_rcClient.right - m_rcClient.left) - (int)rect_border_width;
            int height = (m_rcClient.bottom - m_rcClient.top) - (int)rect_border_width;
            Graphics graphics(m_memDC);
            graphics.DrawRectangle(&pen, 0, 0, width, height);
        }
        break;
    }
    }
}


// ========================================================================================
// End the double buffering process for graphic output.
// ========================================================================================
void CustomLabel::EndDoubleBuffering(HDC hdc) {
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
Gdiplus::Bitmap* LoadImageFromResource(HMODULE hMod, const wchar_t* resid, const wchar_t* restype) {
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
        if (dwResourceSize) {
            HGLOBAL hGlobalResource = LoadResource(hMod, hrsrc); // load it
            if (hGlobalResource) {
                void* imagebytes = LockResource(hGlobalResource); // get a pointer to the file bytes

                // copy image bytes into a real hglobal memory handle
                hGlobal = GlobalAlloc(GHND, dwResourceSize);
                if (hGlobal) {
                    void* pBuffer = GlobalLock(hGlobal);
                    if (pBuffer) {
                        memcpy(pBuffer, imagebytes, dwResourceSize);
                        HRESULT hr = CreateStreamOnHGlobal(hGlobal, true, &pStream);
                        if (SUCCEEDED(hr)) {
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
LRESULT CALLBACK CustomLabelProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static CustomLabel* pDataSelected = nullptr;
    static CustomLabel* pData = nullptr;

    if (uMsg != WM_CREATE) {
        pData = (CustomLabel*)GetWindowLongPtr(hwnd, 0);
    }

    switch (uMsg) {

    case WM_KEYDOWN: {
        // Parent to handle the TAB navigation key to move amongst constrols.
        if (wParam == VK_TAB || wParam == VK_UP || wParam == VK_DOWN) {
            if (SendMessage(pData->hParent, uMsg, wParam, lParam))
                return 0;
        }

        if (wParam == VK_SPACE) {
            SendMessage(hwnd, WM_LBUTTONDOWN, wParam, lParam);
            SendMessage(hwnd, WM_LBUTTONUP, wParam, lParam);
            return 0;
        }
        break;
    }

    case WM_SETCURSOR: {
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

    case WM_MOUSEMOVE: {
        if (!pData) return 0;
        SendMessage(pData->hParent, MSG_CUSTOMLABEL_MOUSEMOVE, (WPARAM)pData->ctrl_id, (LPARAM)hwnd);

        // Tracks the mouse movement and stores the hot state
        TRACKMOUSEEVENT trackMouse{};

        if (pData->hot_test_enable) {
            if (GetProp(hwnd, L"HOT") == 0) {
                trackMouse.cbSize = sizeof(trackMouse);
                trackMouse.dwFlags = TME_LEAVE;
                trackMouse.hwndTrack = hwnd;
                trackMouse.dwHoverTime = 1;
                TrackMouseEvent(&trackMouse);
                SetProp(hwnd, L"HOT", (HANDLE)true);
                AfxRedrawWindow(hwnd);
            }
        }
        return 0;
    }

    case WM_MOUSELEAVE: {
        if (!pData) return 0;
        SendMessage(pData->hParent, MSG_CUSTOMLABEL_MOUSELEAVE, (WPARAM)pData->ctrl_id, (LPARAM)hwnd);

        //  Removes the hot state and redraws the label
        if (pData->hot_test_enable) {
            RemoveProp(hwnd, L"HOT");
            AfxRedrawWindow(hwnd);
        }
        return 0;
    }

    case WM_LBUTTONDOWN: {
        if (pData) {
            // Allow the label to repaint in case different color has been specified
            // for button down (eg. if label is acting like a button).
            SetCapture(hwnd);
            pData->LButtonDown = true;
            AfxRedrawWindow(hwnd);

            // We do not set the button IsSelected to true here because we leave it to 
            // the user to decide if after clicking the label whether to set the label
            // to selected or not.
            PostMessage(pData->hParent, MSG_CUSTOMLABEL_CLICK, (WPARAM)pData->ctrl_id, (LPARAM)hwnd);
        }
        return 0;
    }


    case WM_LBUTTONUP:
    case WM_CAPTURECHANGED: {
        if (pData) {
            // Allow the label to repaint in case different color has been specified
            // for button down (eg. if label is acting like a button).
            pData->LButtonDown = false;
            AfxRedrawWindow(hwnd);
            ReleaseCapture();
        }
        return 0;
    }

    case WM_SETFOCUS:
    case WM_KILLFOCUS: {
        AfxRedrawWindow(hwnd);
        break;
    }

    case WM_ERASEBKGND: {
        // Handle all of the painting in WM_PAINT
        return true;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;

        HDC hdc = BeginPaint(hwnd, &ps);
        if (pData) {
            pData->StartDoubleBuffering(hdc);
            pData->DrawImageInBuffer();
            pData->DrawTextInBuffer();
            pData->DrawLabelInBuffer();
            pData->DrawBordersInBuffer();
            pData->EndDoubleBuffering(hdc);
        }
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY: {
        if (pData) {
            if (pData->pImage) delete(pData->pImage);
            if (pData->pImageHot) delete(pData->pImageHot);
        }
        break;
    }

    case WM_NCDESTROY: {
        if (pData) delete(pData);
        break;
    }

    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


// ========================================================================================
// Get the custom control data pointer.
// ========================================================================================
CustomLabel* CustomLabel_GetOptions(HWND hCtrl) {
    CustomLabel* pData = (CustomLabel*)GetWindowLongPtr(hCtrl, 0);
    return pData;
}


// ========================================================================================
// Store the custom control data pointer.
// ========================================================================================
int CustomLabel_SetOptions(HWND hCtrl, CustomLabel* pData) {
    if (!pData) return 0;

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
void CustomLabel_SetHotTesting(HWND hCtrl, bool enable_hot_testing) {
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData) {
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
void CustomLabel_SetSelected(HWND hCtrl, bool is_selected) {
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData) {
        pData->is_selected = is_selected;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Get the Selected status for the custom control.
// ========================================================================================
bool CustomLabel_GetSelected(HWND hCtrl) {
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData) {
        return pData->is_selected;
    }
    return false;
}


// ========================================================================================
// Set enable/disable Selected Underline and Color status for the custom control.
// ========================================================================================
void CustomLabel_SetSelectedUnderline(HWND hCtrl, bool enable_underline, DWORD underline_color, int line_width) {
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData) {
        pData->is_selected_underline = enable_underline;
        pData->selected_underline_color = underline_color;
        pData->selected_underline_width = line_width;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Set the text for the custom control.
// ========================================================================================
void CustomLabel_SetText(HWND hCtrl, std::wstring text) {
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData) {
        pData->text = text;
        pData->text_hot = text;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Set the text foreground color for the custom control.
// ========================================================================================
void CustomLabel_SetTextColor(HWND hCtrl, DWORD text_color) {
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData) {
        pData->text_color = text_color;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Set the text foreground hot color for the custom control.
// ========================================================================================
void CustomLabel_SetTextColorHot(HWND hCtrl, DWORD text_color_hot) {
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData) {
        pData->text_color_hot = text_color_hot;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Set the text background color for the custom control.
// ========================================================================================
void CustomLabel_SetBackColor(HWND hCtrl, DWORD back_color) {
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData) {
        pData->back_color = back_color;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Set the text background hot color for the custom control.
// ========================================================================================
void CustomLabel_SetBackColorHot(HWND hCtrl, DWORD back_color_hot) {
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData) {
        pData->back_color_hot = back_color_hot;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Set the text background Selected color for the custom control.
// ========================================================================================
void CustomLabel_SetBackColorSelected(HWND hCtrl, DWORD back_color_selected) {
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData) {
        pData->back_color_selected = back_color_selected;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Set the border color for the custom control.
// ========================================================================================
void CustomLabel_SetBorderColor(HWND hCtrl, DWORD border_color) {
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData) {
        pData->border_color = border_color;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Set the border hot color for the custom control.
// ========================================================================================
void CustomLabel_SetBorderColorFocus(HWND hCtrl, DWORD color_focus) {
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData) {
        pData->border_color_focus = color_focus;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Get the text background color for the custom control.
// ========================================================================================
DWORD CustomLabel_GetBackColor(HWND hCtrl) {
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData) {
        return pData->back_color;
    }
    return COLOR_BLACK;
}


// ========================================================================================
// Set the tooltip text for the custom control.
// ========================================================================================
void CustomLabel_SetToolTip(HWND hCtrl, std::wstring text) {
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData) {
        pData->tooltip_text = text;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Get the text for the custom control.
// ========================================================================================
std::wstring CustomLabel_GetText(HWND hCtrl) {
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData) {
        return pData->text;
    }
    return L"";
}


// ========================================================================================
// Set the border (width and colors)  for the custom control.
// ========================================================================================
void CustomLabel_SetBorder(HWND hCtrl, REAL border_width, DWORD border_color, DWORD color_focus) {
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData) {
        pData->border_visible = true;
        pData->border_width = border_width;
        pData->border_color = border_color;
        pData->border_color_focus = color_focus;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Set the mouse pointer (normal and hot) for the custom control.
// ========================================================================================
void CustomLabel_SetMousePointer(HWND hCtrl, CustomLabelPointer normal_pointer, CustomLabelPointer hot_pointer) {
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData) {
        pData->pointer = normal_pointer;
        pData->pointer_hot = hot_pointer;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Set the text offset for the custom control (useful to ensure text is not jammed
// up against a border).
// ========================================================================================
void CustomLabel_SetTextOffset(HWND hCtrl, int offset_left, int offset_top) {
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData) {
        pData->text_offset_left = offset_left;
        pData->text_offset_top = offset_top;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Set the image offset for the custom control (useful for centering image).
// ========================================================================================
void CustomLabel_SetImageOffset(HWND hCtrl, int offset_left, int offset_top) {
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData) {
        pData->image_offset_left = offset_left;
        pData->image_offset_top = offset_top;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Set the user defined text data (wstring) for the custom control.
// ========================================================================================
void CustomLabel_SetUserData(HWND hCtrl, std::wstring text) {
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData) {
        pData->user_data = text;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Get the user defined text data (wstring) for the custom control.
// ========================================================================================
std::wstring CustomLabel_GetUserData(HWND hCtrl) {
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData) {
        return pData->user_data;
    }
    return L"";
}


// ========================================================================================
// Set the user defined integer data (int) for the custom control.
// ========================================================================================
void CustomLabel_SetUserDataInt(HWND hCtrl, int value) {
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData) {
        pData->user_data_int = value;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Get the user defined integer data (int) for the custom control.
// ========================================================================================
int CustomLabel_GetUserDataInt(HWND hCtrl) {
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData) {
        return pData->user_data_int;
    }
    return 0;
}


// ========================================================================================
// Set the font for the custom control.
// ========================================================================================
void CustomLabel_SetFont(HWND hCtrl, std::wstring font_name, int font_size, bool font_bold) {
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData) {
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
void CustomLabel_SetFontHot(HWND hCtrl, std::wstring font_name, int font_size, bool font_bold) {
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData) {
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
void CustomLabel_SetImages(HWND hCtrl, int image_id, int image_hot_id) {
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData) {
        pData->pImage = LoadImageFromResource(pData->hInst, MAKEINTRESOURCE(image_id), L"PNG");
        pData->pImageHot = LoadImageFromResource(pData->hInst, MAKEINTRESOURCE(image_hot_id), L"PNG");
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Creates a simple "dumb" label that basically just makes it easier to deal
// with coloring. 
// ========================================================================================
HWND CustomLabel_SimpleLabel(HWND hParent, int ctrl_id, std::wstring text,
    DWORD text_color, DWORD back_color, CustomLabelAlignment alignment, 
    int left, int top, int width, int height)
{
    CustomLabel* pData = nullptr;

    HWND hCtl = CreateCustomLabel(
        hParent, ctrl_id, CustomLabelType::text_only,
        left, top, width, height);
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
// Creates a simple checkbox button. 
// ========================================================================================
HWND CustomLabel_SimpleCheckBox(HWND hParent, int ctrl_id, std::wstring text,
    DWORD text_color, DWORD back_color, DWORD check_color, DWORD check_back_color, DWORD border_focus_color,
    int left, int top, int width, int height)
{
    CustomLabel* pData = nullptr;

    HWND hCtl = CreateCustomLabel(
        hParent, ctrl_id, CustomLabelType::checkbox,
        left, top, width, height);
    pData = CustomLabel_GetOptions(hCtl);
    if (pData) {
        pData->text = text;
        pData->hot_test_enable = false;
        pData->back_color = back_color;
        pData->text_color = text_color;
        pData->check_color = text_color;
        pData->check_back_color = back_color;
        pData->back_color_button_down = back_color;
        pData->border_visible = true;
        pData->border_width = 1;
        pData->border_color = back_color;
        pData->border_color_focus = border_focus_color;
        pData->is_checked = false;
        pData->text_alignment = CustomLabelAlignment::middle_left;
        pData->allow_tab_stop = true;
        CustomLabel_SetOptions(hCtl, pData);
    }

    return hCtl;
}


// ========================================================================================
// Set the checkbox state to selected (true) or unselected (false).
// ========================================================================================
void CustomLabel_SetCheckState(HWND hCtrl, bool check_state) {
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData) {
        pData->is_checked = check_state;
        CustomLabel_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Set the checkbox state to selected (true) or unselected (false).
// ========================================================================================
bool CustomLabel_GetCheckState(HWND hCtrl) {
    CustomLabel* pData = CustomLabel_GetOptions(hCtrl);
    if (pData) {
        return pData->is_checked;
    }
    return false;
}


// ========================================================================================
// Creates a simple label that acts like a push button.
// ========================================================================================
HWND CustomLabel_ButtonLabel(HWND hParent, int ctrl_id, std::wstring text,
    DWORD text_color, DWORD back_color, DWORD back_color_hot, DWORD back_color_button_down, DWORD focus_border_color,
    CustomLabelAlignment alignment, int left, int top, int width, int height)
{
    // Creates a simple button type of label
    CustomLabel* pData = nullptr;

    HWND hCtl = CreateCustomLabel(
        hParent, ctrl_id, CustomLabelType::text_only,
        left, top, width, height);
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
        pData->border_color_focus = focus_border_color;
        pData->border_color = back_color;
        CustomLabel_SetOptions(hCtl, pData);
    }

    return hCtl;
}


// ========================================================================================
// Creates a simple label that only displays an image.
// ========================================================================================
HWND CustomLabel_SimpleImageLabel(HWND hParent, int ctrl_id,
    int image_id, int image_hot_id, 
    int image_width, int image_height,
    int left, int top, int width, int height)
{
    // Creates a simple "dumb" label that basically just makes it easier to deal
    // with coloring. No hot tracking or click notifications.
    CustomLabel* pData = nullptr;
    
    HWND hCtl = CreateCustomLabel(
    hParent, ctrl_id,
    CustomLabelType::image_only,
    left, top, width, height);
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
HWND CustomLabel_HorizontalLine(HWND hParent, int ctrl_id, DWORD line_color, DWORD back_color,
    int left, int top, int width, int height)
{
    // Creates a simple "dumb" label that basically just makes it easier to deal
    // with coloring. No hot tracking or click notifications.
    CustomLabel* pData = nullptr;

    HWND hCtl = CreateCustomLabel(
        hParent, ctrl_id,
        CustomLabelType::line_horizontal,
        left, top, width, height);
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
HWND CustomLabel_VerticalLine(HWND hParent, int ctrl_id, DWORD line_color, DWORD back_color,
    int left, int top, int width, int height)
{
    // Creates a simple "dumb" label that basically just makes it easier to deal
    // with coloring. No hot tracking or click notifications.
    CustomLabel* pData = nullptr;

    HWND hCtl = CreateCustomLabel(
        hParent, ctrl_id,
        CustomLabelType::line_vertical,
        left, top, width, height);
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
    LONG_PTR ctrl_id,
    CustomLabelType ctrl_type,
    int left, 
    int top, 
    int width, 
    int height )
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
            (int)(left * rx), (int)(top * ry), (int)(width * rx), (int)(height * ry),
            hWndParent, (HMENU)ctrl_id, hInst, (LPVOID)NULL);

    if (hCtl) {
        CustomLabel* pData = new CustomLabel;

        pData->hWindow = hCtl;
        pData->hParent = hWndParent;
        pData->hInst = hInst;
        pData->ctrl_id = (int)ctrl_id;
        pData->ctrl_type = ctrl_type;

        pData->font_name = AfxGetDefaultFont();
        pData->font_size = 9;
        pData->font_name_hot = pData->font_name;
        pData->font_size_hot = pData->font_size;

        pData->hToolTip = AfxAddTooltip(hCtl, L"", false, false);

        if (ctrl_type == CustomLabelType::line_horizontal || 
            ctrl_type == CustomLabelType::line_vertical) {
            pData->hot_test_enable = false;
            pData->border_visible = false;
        }

        pData->pointer = CustomLabelPointer::arrow;
        pData->pointer_hot = CustomLabelPointer::arrow;

        CustomLabel_SetOptions(hCtl, pData);
    }
   
    return hCtl;
}


