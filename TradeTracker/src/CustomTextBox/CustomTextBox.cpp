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
// CUSTOM TEXTBOX CONTROL
//

#include "pch.h"
#include "Utilities/CWindowBase.h"
#include "MainWindow/MainWindow.h"
#include "CustomVScrollBar/CustomVScrollBar.h"

#include "CustomTextBox.h"


extern CMainWindow Main;


// ========================================================================================
// Empty text from the Windows clipboard
// ========================================================================================
void EmptyClipboardText()
{
    if (OpenClipboard(nullptr))
    {
        // Empty the clipboard
        EmptyClipboard();

        // Close the clipboard
        CloseClipboard();
    }
}
        
        
// ========================================================================================
// Set unicode text into the Windows clipboard
// ========================================================================================
void SetClipboardText(const std::wstring& text)
{
    if (OpenClipboard(nullptr))
    {
        // Empty the clipboard
        EmptyClipboard();

        // Allocate global memory for the Unicode text
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (text.length() + 1) * sizeof(wchar_t));
        if (hMem != nullptr)
        {
            // Lock the memory and copy the text into it
            wchar_t* pMem = static_cast<wchar_t*>(GlobalLock(hMem));
            if (pMem != nullptr)
            {
                wcscpy_s(pMem, text.length() + 1, text.c_str());
                GlobalUnlock(hMem);

                // Set the Unicode text to the clipboard
                SetClipboardData(CF_UNICODETEXT, hMem);
            }
        }

        // Close the clipboard
        CloseClipboard();
    }
}


// ========================================================================================
// Get unicode text from the Windows clipboard
// ========================================================================================
std::wstring GetClipboardText()
{
    std::wstring clipboard_text = L"";

    // Attempt to open the clipboard
    if (OpenClipboard(nullptr)) {
        // Attempt to retrieve text from the clipboard
        HANDLE hClipboardData = GetClipboardData(CF_UNICODETEXT);

        if (hClipboardData != nullptr) {
            // Lock the handle to get a pointer to the text
            wchar_t* text = static_cast<wchar_t*>(GlobalLock(hClipboardData));

            if (text != nullptr) {
                // Print or use the clipboard text
                clipboard_text.assign(text);

                // Unlock the global memory
                GlobalUnlock(hClipboardData);
            }
            else {
                std::cerr << "Failed to lock clipboard memory." << std::endl;
            }
        }
        else {
            std::cerr << "Failed to retrieve clipboard data." << std::endl;
        }

        // Close the clipboard
        CloseClipboard();
    }
    else {
        std::cerr << "Failed to open the clipboard." << std::endl;
    }

    return clipboard_text;
}


// ========================================================================================
// Process WM_SIZE message for window/dialog: CustomTextBox
// ========================================================================================
void CustomTextBox_OnSize(HWND hCtrl)
{
    // Move the child textbox within the confines of the parent
    // custom control. Take the border width into account.

    CustomTextBox* pData = CustomTextBox_GetOptions(hCtrl);
    if (pData) {
        RECT rc; GetClientRect(hCtrl, &rc);
        if (pData->horiz_text_margin) {
            int horiz_margin_width = AfxScaleX((float)pData->horiz_text_margin);
            InflateRect(&rc, -horiz_margin_width, 0);
        }
        if (pData->vert_text_margin) {
            int vert_margin_width = AfxScaleY((float)pData->vert_text_margin);
            rc.top += vert_margin_width;
        }
        SetWindowPos(pData->hTextBox, HWND_TOP,
            rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_SHOWWINDOW);
    }
}


// ========================================================================================
// Perform numeric formatting for textboxes allowing decimal places.
// ========================================================================================
void CustomTextBox_FormatDisplayDecimalPlaces(CustomTextBox* pData)
{
    if (pData == nullptr) return;
    if (pData->is_numeric == false) return;
    if (pData->allow_formatting == CustomTextBoxFormatting::disallow) return;

    if (pData->decimal_count > 0) {
        static std::wstring DecimalSep = L".";
        static std::wstring ThousandSep = L",";

        static NUMBERFMTW num{};
        num.NumDigits = pData->decimal_count;
        num.LeadingZero = true;
        num.Grouping = 0;
        num.lpDecimalSep = (LPWSTR)DecimalSep.c_str();
        num.lpThousandSep = (LPWSTR)ThousandSep.c_str();
        num.NegativeOrder = 1;   // Negative sign, number; for example, -1.1

        std::wstring money = AfxGetWindowText(pData->hTextBox);

        std::wstring buffer(256, 0);
        size_t j = GetNumberFormatEx(LOCALE_NAME_USER_DEFAULT, 0, money.c_str(), &num, (LPWSTR)buffer.c_str(), 256);

        money = buffer.substr(0, j - 1);
        AfxSetWindowText(pData->hTextBox, money.c_str());
    }
}


// ========================================================================================
// Custom Control subclass Window procedure to deal TextBoxes filtering numeric input
// and handling KILLFOCUS validation.
// ========================================================================================
LRESULT CALLBACK CustomTextBox_SubclassProc(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{

    // Create static accumulation variable to collect the data from
    // a series of middle mouse wheel scrolls.
    static int accumDelta = 0;

    CustomTextBox* pData = CustomTextBox_GetOptions(GetParent(hWnd));

    switch (uMsg)
    { 

    case WM_MOUSEWHEEL:
    {
        if (!pData->is_multiline || !pData->hScrollBar) return 0;

        // Accumulate delta until scroll one line (up +120, down -120). 
        // 120 is the Microsoft default delta
        int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        int num_lines = 0;
        accumDelta += zDelta;
        if (accumDelta >= 120) {     // scroll up 1 line
            num_lines = -1;
            SendMessage(hWnd, EM_LINESCROLL, 0, (LPARAM)num_lines);
            accumDelta = 0;
        }
        else {
            if (accumDelta <= -120) {     // scroll down 1 line
                num_lines = 1;
                SendMessage(hWnd, EM_LINESCROLL, 0, (LPARAM)num_lines);
                accumDelta = 0;
            }
        }

        CustomVScrollBar_Recalculate(pData->hScrollBar);
        return 0;
    }


    case WM_CHAR:
    {
        // Prevent the TAB and ESCAPE character causing a BEEP. We handle TAB key navigation
        // ourselves in WM_KEYDOWN. Likewise for ENTER and ESCAPE key.
        if (wParam == VK_TAB || wParam == VK_ESCAPE) return 0;
        
        if (pData == nullptr) break;

        // Allow the ENTER key for multiline textboxes but prevent it otherwise because it
        // will beep otherwise. 
        if (wParam == VK_RETURN) {
            if (!pData->is_multiline) return 0;
        }

        // Allow any character for non-numeric these TextBoxes
        if (!pData->is_numeric) {
            PostMessage(pData->hParent, WM_COMMAND, MAKEWPARAM(pData->CtrlId, EN_CHANGE), (LPARAM)pData->hWindow);
            break;
        }


        if (wParam == VK_BACK) break;   // Allow normal backspace

        // Handle Numeric textboxes
        // Allow 0 to 9 and Decimal
        if (wParam >= '0' && wParam <= '9' || wParam == '.') {
            // If decimal places are allowed then only allow the digit
            // if room exists after the decimal point.
            if (pData->decimal_count == 0) {
                if (wParam == 46) return 0;
            }
            
            int nPos = (int)SendMessage(hWnd, EM_GETSEL, 0, 0);
            size_t start_position = LOWORD(nPos);
            size_t end_position = HIWORD(nPos);

            // Allow decimal (but only once), and digit if limit not reached
            std::wstring text = AfxGetWindowText(hWnd);
            // Remove any selected text that will be replaced
            if (start_position != end_position) {   // we have selected text
                text.erase(start_position, (end_position - start_position + 1));
            }

            size_t nFoundAt = text.find(L".");
            if (nFoundAt != std::wstring::npos) {
                // Period already exists then don't allow another one.
                if (wParam == 46) {
                    return 0;   // do not allow
                }
                else {
                    // Allow the digit is being entered before the decimal point
                    if (start_position <= nFoundAt) break;
                    std::wstring wszFractional = text.substr(nFoundAt + 1);
                    if (wszFractional.length() == pData->decimal_count)
                        return 0;
                }
            }

            PostMessage(pData->hParent, WM_COMMAND, MAKEWPARAM(pData->CtrlId, EN_CHANGE), (LPARAM)pData->hWindow);

            // Allow the number or decimal
            break;
        }


        // Negative sign 
        if (wParam == 45) {
            if (pData->allow_negative == CustomTextBoxNegative::disallow) return 0;

            // Only allow the negative sign for other textboxes if it is 
            // the first character (position 0)
            DWORD nPos = LOWORD(SendMessage(hWnd, EM_GETSEL, 0, 0));
            if (nPos == 0) break;

            // Otherwise, disallow the negative sign
            return 0;
        }


        // Disallow everything else by skipping calling the DefSubclassProc
        return 0;
    }


    case WM_CUT:
    {
        // Send a postmessage to recalculate the scrollbar metrics. 
        PostMessage(hWnd, MSG_RECALCULATE_VSCROLLBAR, 0, 0);

        if (!pData->is_numeric) {
            PostMessage(pData->hParent, WM_COMMAND, MAKEWPARAM(pData->CtrlId, EN_CHANGE), (LPARAM)pData->hWindow);
            break;
        }
        return 0;
    }


    case WM_PASTE:
    {
        int start_pos = 0, end_pos = 0;
        SendMessage(hWnd, EM_GETSEL, (WPARAM)(&start_pos), (LPARAM)(&end_pos));
        std::wstring replace_text = GetClipboardText();
        SendMessage(hWnd, EM_REPLACESEL, (WPARAM)true, (LPARAM)replace_text.c_str());

        // Send a postmessage to recalculate the scrollbar metrics. 
        PostMessage(hWnd, MSG_RECALCULATE_VSCROLLBAR, 0, 0);
        PostMessage(pData->hParent, WM_COMMAND, MAKEWPARAM(pData->CtrlId, EN_CHANGE), (LPARAM)pData->hWindow);
        return 0;
    }
    break;


    case MSG_RECALCULATE_VSCROLLBAR:
    {
        if (pData->hScrollBar) {
            CustomVScrollBar_Recalculate(pData->hScrollBar);
            return 0;
        }
    }
    break;


    case WM_KEYDOWN:
    {
        if (!pData->is_numeric) {
            if (wParam == VK_DELETE || wParam == VK_INSERT) {
                PostMessage(pData->hParent, WM_COMMAND, MAKEWPARAM(pData->CtrlId, EN_CHANGE), (LPARAM)pData->hWindow);
                break;
            }
        }

        // Handle up/down arrows by sending notification to parent. TradeGrid will
        // act on up/down arrows by moving the focus amongst the grid textboxes.
        if (wParam == VK_UP || wParam == VK_DOWN) {
            if (SendMessage(pData->hParent, uMsg, wParam, lParam) == TRUE)
                return 0;
        }


        if (wParam == VK_UP || wParam == VK_DOWN ||
            wParam == VK_PRIOR || wParam == VK_NEXT ||
            wParam == VK_HOME || wParam == VK_END) {
            // Send a postmessage to recalculate the scrollbar metrics. 
            PostMessage(hWnd, MSG_RECALCULATE_VSCROLLBAR, 0, 0);
        }

        // Handle the TAB navigation key to move amongst cells in the table rather
        // than move away from the table itself.
        if (wParam == VK_TAB) {
            if (SendMessage(pData->hParent, uMsg, wParam, lParam) == TRUE)
                return 0;
        }

        // Check for Ctrl + Backspace
        if ((GetKeyState(VK_CONTROL) & 0x8000) && (wParam == VK_BACK))
        {
            // Get the current selection range
            int start_pos = 0, end_pos = 0;
            SendMessage(hWnd, EM_GETSEL, (WPARAM)(&start_pos), (LPARAM)(&end_pos));

            // If the start and end are not equal then text is selected. We do not want
            // our Ctrl+Backspace to work on selected text.
            if (start_pos != end_pos) return 0;

            // Get the current text in the Edit control
            int textLength = GetWindowTextLength(hWnd);
            if (textLength > 0)
            {
                std::wstring buffer(textLength, L'\0');
                GetWindowText(hWnd, &buffer[0], textLength + 1);

                buffer = buffer.substr(0, start_pos);

                // Find the position of the last non-space character before the cursor
                size_t lastNonSpacePos = buffer.find_last_not_of(L" \t", start_pos);
                if (lastNonSpacePos != std::wstring::npos)
                {
                    // Find the position of the previous space character before the cursor
                    static std::wstring search_for_text = L" \t!@#$%^&*()+~`-={}|[]\\:;'<>?,./\"";
                    size_t previousSpacePos = buffer.find_last_of(search_for_text, lastNonSpacePos);

                    SendMessage(hWnd, EM_SETSEL, (WPARAM)previousSpacePos, (LPARAM)end_pos);
                    SendMessage(hWnd, WM_CUT, 0, 0);
                    PostMessage(hWnd, MSG_REMOVE_BOXCHAR, 0, 0);

                    return 0;
                }
            }
        }


    }
    break;

    
    case MSG_REMOVE_BOXCHAR:
    {
        // Remove the box character that is inserted during the ctrl+backspace
        int start_pos = 0, end_pos = 0;
        SendMessage(hWnd, EM_GETSEL, (WPARAM)(&start_pos), (LPARAM)(&end_pos));
        
        start_pos = max(0, start_pos - 1);
        SendMessage(hWnd, EM_SETSEL, (WPARAM)start_pos, (LPARAM)end_pos);
        
        // Replace with blank character and set false to not allow undo
        std::wstring nullchar = L"";
        SendMessage(hWnd, EM_REPLACESEL, (WPARAM)false, (LPARAM)nullchar.c_str());
    }
    break;


    case WM_KEYUP:
    {
        // Send key information to the parent in case keys like VK_RETURN (Enter) need
        // to be processed.
        if (SendMessage(pData->hParent, uMsg, wParam, lParam) == TRUE)
            return 0;
    }
    break;

    
    case WM_KILLFOCUS:
    {
        // If this is a numeric textbox that allows decimal places then we will
        // format and pad the entered text to ensure that the required number
        // of decimal places will display.
        CustomTextBox_FormatDisplayDecimalPlaces(pData);

        // Ensure any border is colored correctly
        AfxRedrawWindow(pData->hWindow);

        SendMessage(pData->hParent, WM_COMMAND, MAKEWPARAM(pData->CtrlId, EN_KILLFOCUS), (LPARAM)pData->hWindow);
    }
    break;


    case WM_SETFOCUS:
    {
        // Highlight the text in the TextBox as it gains focus
        if (pData->allow_select_onfocus) {
            Edit_SetSel(hWnd, 0, -1);
        }
    
        // Ensure any border is colored correctly
        AfxRedrawWindow(GetParent(hWnd));

        SendMessage(pData->hParent, WM_COMMAND, MAKEWPARAM(pData->CtrlId, EN_SETFOCUS), (LPARAM)pData->hWindow);
    }
    break;

    
    case WM_DESTROY:
        // REQUIRED: Remove control subclassing
        RemoveWindowSubclass(hWnd, CustomTextBox_SubclassProc, uIdSubclass);
        break;


    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);

}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CALLBACK CustomTextBoxProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CustomTextBox* pData = nullptr;

    if (uMsg != WM_CREATE) {
        pData = (CustomTextBox*)GetWindowLongPtr(hWnd, 0);
    }


    switch (uMsg)
    {

    case WM_GETTEXT:
    case WM_SETTEXT:
    case WM_GETTEXTLENGTH:
    {
        return SendMessage(pData->hTextBox, uMsg, wParam, lParam);
    }
    break;


    case WM_CTLCOLOREDIT:
    {
        HDC hdc = (HDC)wParam;
        if (pData->back_brush) DeleteBrush(pData->back_brush);
        pData->back_brush = CreateSolidBrush(Color(pData->back_color).ToCOLORREF());
        SetTextColor(hdc, Color(pData->text_color).ToCOLORREF());
        SetBkColor(hdc, Color(pData->back_color).ToCOLORREF());
        SetBkMode(hdc, OPAQUE);
        return (LRESULT)pData->back_brush;
    }
    break;


    case WM_SIZE:
    {
        CustomTextBox_OnSize(hWnd);
        return 0;
    }
    break;


    case WM_SETFOCUS:
    {
        SetFocus(pData->hTextBox);
        return 0;
    }
    break;


    case WM_SETCURSOR:
    {
        SetCursor(LoadCursor(NULL, (LPCWSTR)IDC_IBEAM));
        return TRUE;
    }
    break;


    case WM_SETFONT:
    {
        HFONT hFont = (HFONT)SendMessage(pData->hTextBox, WM_GETFONT, 0, 0);
        DeleteFont(hFont);
        if (pData->hFontText) DeleteFont(pData->hFontText);
        pData->hFontText = (HFONT)wParam;
        SendMessage(pData->hTextBox, WM_SETFONT, (WPARAM)pData->hFontText, false);
        return 0;
    }
    break;


    case WM_ERASEBKGND:
    {
        // Handle all of the painting in WM_PAINT
        return TRUE;
    }
    break;


    case WM_PAINT:
    {
        PAINTSTRUCT ps;

        HDC hdc = BeginPaint(hWnd, &ps);
        if (pData) {
            SaveDC(hdc);

            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP hbit = CreateCompatibleBitmap(hdc, ps.rcPaint.right, ps.rcPaint.bottom);
            SelectBitmap(memDC, hbit);

            Graphics graphics(memDC);
            int nWidth = (ps.rcPaint.right - ps.rcPaint.left);
            int nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);

            Color back_color;
            SolidBrush back_brush(pData->back_color);
            graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

            // Copy the entire memory bitmap to the main display
            BitBlt(hdc, 0, 0, ps.rcPaint.right, ps.rcPaint.bottom, memDC, 0, 0, SRCCOPY);

            // Restore the original state of the DC
            RestoreDC(hdc, -1);

            // Cleanup
            DeleteObject(hbit);
            DeleteDC(memDC);

        }
        EndPaint(hWnd, &ps);
        return 0;
    }
    break;


    case WM_DESTROY:
    {
        if (pData) {
            HFONT hFont = (HFONT)SendMessage(pData->hTextBox, WM_GETFONT, 0, 0);
            DeleteFont(hFont);
            pData->hFontText = nullptr;
            if (pData->hRichEditLib) FreeLibrary(pData->hRichEditLib);
        }
    }
    break;


    case WM_NCDESTROY:
    {
        if (pData) {
            if (pData->back_brush) DeleteBrush(pData->back_brush);
            delete(pData);
        }
    }
    break;


    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;

}


// ========================================================================================
// Get the custom control data pointer.
// ========================================================================================
CustomTextBox* CustomTextBox_GetOptions(HWND hCtrl)
{
    CustomTextBox* pData = (CustomTextBox*)GetWindowLongPtr(hCtrl, 0);
    return pData;
}


// ========================================================================================
// Set the custom control data pointer.
// ========================================================================================
int CustomTextBox_SetOptions(HWND hCtrl, CustomTextBox* pData)
{
    if (pData == nullptr) return 0;

    SetWindowLongPtr(hCtrl, 0, (LONG_PTR)pData);
    AfxRedrawWindow(hCtrl);

    return 0;
}


// ========================================================================================
// Set the text for the custom control.
// ========================================================================================
void CustomTextBox_SetText(HWND hCtrl, std::wstring text)
{
    CustomTextBox* pData = CustomTextBox_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->text = text;
        AfxSetWindowText(pData->hTextBox, text);
        CustomTextBox_SetOptions(hCtrl, pData);
        CustomTextBox_FormatDisplayDecimalPlaces(pData);
        AfxRedrawWindow(hCtrl);
    }
}


// ========================================================================================
// Set the user defined text data (wstring) for the custom control.
// ========================================================================================
void CustomTextBox_SetUserData(HWND hCtrl, std::wstring user_data)
{
    CustomTextBox* pData = CustomTextBox_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->user_data = user_data;
        CustomTextBox_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Get the user defined text data (wstring) for the custom control.
// ========================================================================================
std::wstring CustomTextBox_GetUserData(HWND hCtrl)
{
    CustomTextBox* pData = CustomTextBox_GetOptions(hCtrl);
    if (pData != nullptr) {
        return pData->user_data;
    }
    return L"";
}


// ========================================================================================
// Set the margins for the custom control (horizontal and vertical).
// ========================================================================================
void CustomTextBox_SetMargins(HWND hCtrl, int horiz_text_margin, int vert_text_margin)
{
    CustomTextBox* pData = CustomTextBox_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->horiz_text_margin = horiz_text_margin;
        pData->vert_text_margin = vert_text_margin;
        CustomTextBox_SetOptions(hCtrl, pData);
        CustomTextBox_OnSize(hCtrl);
        AfxRedrawWindow(hCtrl);
    }
}


// ========================================================================================
// Set the text colors for the custom control (foreground and background).
// ========================================================================================
void CustomTextBox_SetColors(HWND hCtrl, DWORD text_color, DWORD back_color)
{
    CustomTextBox* pData = CustomTextBox_GetOptions(hCtrl);
    if (pData != nullptr) {
        if (pData->back_brush) DeleteBrush(pData->back_brush);
        pData->back_brush = CreateSolidBrush(back_color);
        pData->back_color = back_color;
        pData->text_color = text_color;
        CustomTextBox_SetOptions(hCtrl, pData);
        AfxRedrawWindow(hCtrl);
    }
}


// ========================================================================================
// Highlight or deselect all text when textbox gains focus.
// ========================================================================================
void CustomTextBox_SetSelectOnFocus(HWND hCtrl, bool allow_select_onfocus)
{
    CustomTextBox* pData = CustomTextBox_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->allow_select_onfocus = allow_select_onfocus;
        CustomTextBox_SetOptions(hCtrl, pData);
        AfxRedrawWindow(hCtrl);
    }
}


// ========================================================================================
// Attach a custom vertical scrollbar control to this textbox. 
// Used to coordinate and keep in sync when mousewheel used.
// ========================================================================================
void CustomTextBox_AttachScrollBar(HWND hCtrl, HWND hScrollBar)
{
    CustomTextBox* pData = CustomTextBox_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->hScrollBar = hScrollBar;
        CustomTextBox_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Set the font for the custom control.
// ========================================================================================
void CustomTextBox_SetFont(HWND hCtrl, std::wstring font_name, int font_size)
{
    CustomTextBox* pData = CustomTextBox_GetOptions(hCtrl);
    if (pData != nullptr) {
        if (pData->hFontText) DeleteFont(pData->hFontText);
        pData->font_name = font_name;
        pData->font_size = font_size;
        pData->hFontText = Main.CreateFont(font_name, font_size, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET);
        if (pData->hFontText) SendMessage(pData->hTextBox, WM_SETFONT, (WPARAM)pData->hFontText, false);
        CustomTextBox_SetOptions(hCtrl, pData);
        AfxRedrawWindow(hCtrl);
    }
}


// ========================================================================================
// Set the numeric attributes for the custom control.
// ========================================================================================
void CustomTextBox_SetNumericAttributes(
    HWND hCtrl, int decimal_count, CustomTextBoxNegative AllowNegative, CustomTextBoxFormatting AllowFormatting)
{
    CustomTextBox* pData = CustomTextBox_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->is_numeric = true;
        pData->decimal_count = decimal_count;
        pData->allow_negative = AllowNegative;
        pData->allow_formatting = AllowFormatting;
        CustomTextBox_FormatDisplayDecimalPlaces(pData);
        CustomTextBox_SetOptions(hCtrl, pData);
    }
}


// ========================================================================================
// Create the custom textbox control.
// ========================================================================================
HWND CreateCustomTextBox(
    HWND hWndParent,
    LONG_PTR CtrlId,
    bool isMultiLine,
    int Alignment,
    std::wstring text,
    int nLeft,
    int nTop,
    int nWidth,
    int nHeight)
{
    std::wstring class_name_text(L"CUSTOMTEXTBOX_CONTROL");

    WNDCLASSEX wcex{};

    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hWndParent, GWLP_HINSTANCE);

    if (GetClassInfoEx(hInst, class_name_text.c_str(), &wcex) == 0) {
        wcex.cbSize = sizeof(wcex);
        wcex.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = CustomTextBoxProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = sizeof(HANDLE);    // make room to store a pointer to the class
        wcex.hInstance = hInst;
        wcex.hCursor = LoadCursor(NULL, (LPCWSTR)IDC_ARROW);
        wcex.hIcon = NULL;
        wcex.hIconSm = NULL;
        wcex.hbrBackground = (HBRUSH)HOLLOW_BRUSH;
        wcex.lpszMenuName = NULL;
        wcex.lpszClassName = class_name_text.c_str();
        if (RegisterClassEx(&wcex) == 0) return 0;
    }

    float rx = AfxScaleRatioX();
    float ry = AfxScaleRatioY();


    HWND hCtl =
        CreateWindowEx(0, class_name_text.c_str(), L"",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
            (int)(nLeft * rx), (int)(nTop * ry), (int)(nWidth * rx), (int)(nHeight * ry),
            hWndParent, (HMENU)CtrlId, hInst, (LPVOID)NULL);

    if (hCtl) {
        CustomTextBox* pData = new CustomTextBox;

        pData->hWindow = hCtl;
        pData->hParent = hWndParent;
        pData->hInst = hInst;
        pData->CtrlId = (int)CtrlId;
        pData->font_name = AfxGetDefaultFont();
        pData->font_size = 9;
        pData->back_color = COLOR_GRAYDARK;
        pData->back_brush = CreateSolidBrush(pData->back_color);
        pData->text_color = COLOR_WHITELIGHT;
        pData->horiz_text_margin = 0;
        pData->vert_text_margin = 0;
        pData->alignment = Alignment;
        pData->is_multiline = isMultiLine;

        // Create the child embedded TextBox control
        pData->hTextBox =
            CreateWindowEx(0, L"Edit", L"",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | Alignment | 
                (isMultiLine ? ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL : ES_AUTOHSCROLL),
                0, 0, 0, 0, hCtl, (HMENU)100, hInst, (LPVOID)NULL);

        SetWindowTheme(pData->hTextBox, L"", L"");

        // Subclass the child TextBox if pWndProc is not null
        if (pData->hTextBox)
            SetWindowSubclass(pData->hTextBox, CustomTextBox_SubclassProc, 100, NULL);

        AfxSetWindowText(pData->hTextBox, text);

        CustomTextBox_SetOptions(hCtl, pData);

        // Create and set the text font
        CustomTextBox_SetFont(hCtl, pData->font_name, pData->font_size);

        // If numeric then format and display the text to correct number of decimal places
        CustomTextBox_FormatDisplayDecimalPlaces(pData);

        // Move the child textbox into place based on border width
        CustomTextBox_OnSize(hCtl);
    }

    return hCtl;
}


