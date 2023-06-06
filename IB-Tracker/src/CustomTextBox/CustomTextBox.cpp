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
#include "Utilities\CWindowBase.h"
#include "MainWindow/MainWindow.h"

#include "CustomTextBox.h"


extern CMainWindow Main;



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
        if (pData->BorderStyle != CustomTextBoxBorder::BorderNone && pData->BorderWidth) {
            int BorderWidth = AfxScaleX((float)pData->BorderWidth);
            InflateRect(&rc, -BorderWidth, -BorderWidth);
        }
        if (pData->HTextMargin) {
            int HMarginWidth = AfxScaleX((float)pData->HTextMargin);
            InflateRect(&rc, -HMarginWidth, 0);
        }
        if (pData->VTextMargin) {
            int VMarginWidth = AfxScaleY((float)pData->VTextMargin);
            rc.top += VMarginWidth;
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
    if (pData->isNumeric == false) return;
    if (pData->AllowFormatting == CustomTextBoxFormatting::Disallow) return;

    if (pData->NumDecimals > 0) {
        static std::wstring DecimalSep = L".";
        static std::wstring ThousandSep = L",";

        static NUMBERFMTW num{};
        num.NumDigits = pData->NumDecimals;
        num.LeadingZero = true;
        num.Grouping = 0;
        num.lpDecimalSep = (LPWSTR)DecimalSep.c_str();
        num.lpThousandSep = (LPWSTR)ThousandSep.c_str();
        num.NegativeOrder = 1;   // Negative sign, number; for example, -1.1

        std::wstring money = AfxGetWindowText(pData->hTextBox);

        std::wstring buffer(256, 0);
        int j = GetNumberFormatEx(LOCALE_NAME_USER_DEFAULT, 0, money.c_str(), &num, (LPWSTR)buffer.c_str(), 256);

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
    CustomTextBox* pData = CustomTextBox_GetOptions(GetParent(hWnd));

    switch (uMsg)
    {

    case WM_CHAR:
    {
        // Prevent the TAB character causing a BEEP. We handle TAB key navigation
        // ourselves in WM_KEYDOW. Likewise for ENTER key.
        if (wParam == VK_TAB) return 0;
        if (wParam == VK_RETURN) return 0;

        if (pData == nullptr) break;

        // Allow any character for non-numeric these TextBoxes
        if (!pData->isNumeric) break;

        // Handle Numeric textboxes

        // Allow backspace
        if (wParam == VK_BACK) break;

        
        // Allow 0 to 9 and Decimal
        if (wParam >= 48 && wParam <= 57 || wParam == 46) {
            // If decimal places are allowed then only allow the digit
            // if room exists after the decimal point.
            if (pData->NumDecimals == 0) {
                if (wParam == 46) return 0;
            }
            
            int nPos = SendMessage(hWnd, EM_GETSEL, 0, 0);
            int nStartPos = LOWORD(nPos);
            int nEndPos = HIWORD(nPos);

            // Allow decimal (but only once), and digit if limit not reached
            std::wstring wszText = AfxGetWindowText(hWnd);
            // Remove any selected text that will be replaced
            if (nStartPos != nEndPos) {   // we have selected text
                wszText.erase(nStartPos, (nEndPos - nStartPos + 1));
            }

            int nFoundAt = wszText.find(L".");
            if (nFoundAt != std::wstring::npos) {
                // Period already exists then don't allow another one.
                if (wParam == 46) {
                    return 0;   // do not allow
                }
                else {
                    // Allow the digit is being entered before the decimal point
                    if (nStartPos <= nFoundAt) break;
                    std::wstring wszFractional = wszText.substr(nFoundAt + 1);
                    if (wszFractional.length() == pData->NumDecimals)
                        return 0;
                }
            }

            PostMessage(pData->hParent, WM_COMMAND, MAKEWPARAM(pData->CtrlId, EN_CHANGE), (LPARAM)pData->hWindow);

            // Allow the number or decimal
            break;
        }



        // Negative sign 
        if (wParam == 45) {
            if (pData->AllowNegative == CustomTextBoxNegative::Disallow) return 0;

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


    case WM_KEYDOWN:
    {
        // Handle up/down arrows by sending notification to parent. TradeGrid will
        // act on up/down arrows by moving the focus amongst the grid textboxes.
        if (wParam == VK_UP || wParam == VK_DOWN) {
            if (SendMessage(pData->hParent, uMsg, wParam, lParam) == TRUE)
                return 0;
        }

        // Handle the TAB navigation key to move amongst cells in the table rather
        // than move away from the table itself.
        if (wParam == VK_TAB) {
            if (SendMessage(pData->hParent, uMsg, wParam, lParam) == TRUE)
                return 0;
        }
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
        Edit_SetSel(hWnd, 0, -1);
    
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
        SetTextColor(hdc, pData->TextColor);
        SetBkColor(hdc, pData->BackColor);
        SetBkMode(hdc, OPAQUE);
        return (LRESULT)pData->hBackBrush;
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

            Color backColor;
            backColor.SetFromCOLORREF(pData->BackColor);
            SolidBrush backBrush(backColor);
            graphics.FillRectangle(&backBrush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

            if (pData->BorderWidth > 0 && pData->BorderStyle != CustomTextBoxBorder::BorderNone) {
                Color clrPen; 
                if (GetFocus() == pData->hTextBox) {
                    clrPen.SetFromCOLORREF(pData->BorderColorFocus);
                }
                else {
                    clrPen.SetFromCOLORREF(pData->BorderColor);
                }

                int nTop = ps.rcPaint.top;
                if (pData->BorderStyle == CustomTextBoxBorder::BorderUnderline)
                    nTop = ps.rcPaint.bottom - pData->BorderWidth;
                nWidth = nWidth - pData->BorderWidth;
                nHeight = nHeight - pData->BorderWidth;
                Pen pen(clrPen, (REAL)pData->BorderWidth);
                RectF rectF((REAL)ps.rcPaint.left, (REAL)nTop, (REAL)nWidth, (REAL)nHeight);
                graphics.DrawRectangle(&pen, rectF);
            }

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


    case WM_NCDESTROY:
        if (pData) {
            DeleteBrush(pData->hBackBrush);
            delete(pData);
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
void CustomTextBox_SetText(HWND hCtrl, std::wstring wszText)
{
    CustomTextBox* pData = CustomTextBox_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->wszText = wszText;
        AfxSetWindowText(pData->hTextBox, wszText);
        CustomTextBox_SetOptions(hCtrl, pData);
        CustomTextBox_FormatDisplayDecimalPlaces(pData);
        AfxRedrawWindow(hCtrl);
    }
}


// ========================================================================================
// Set the user defined text data (wstring) for the custom control.
// ========================================================================================
void CustomTextBox_SetUserData(HWND hCtrl, std::wstring UserData)
{
    CustomTextBox* pData = CustomTextBox_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->UserData = UserData;
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
        return pData->UserData;
    }
}


// ========================================================================================
// Set the margins for the custom control (horizontal and vertical).
// ========================================================================================
void CustomTextBox_SetMargins(HWND hCtrl, int HTextMargin, int VTextMargin)
{
    CustomTextBox* pData = CustomTextBox_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->HTextMargin = HTextMargin;
        pData->VTextMargin = VTextMargin;
        CustomTextBox_SetOptions(hCtrl, pData);
        CustomTextBox_OnSize(hCtrl);
        AfxRedrawWindow(hCtrl);
    }
}


// ========================================================================================
// Set the text colors for the custom control (foreground and background).
// ========================================================================================
void CustomTextBox_SetColors(HWND hCtrl, COLORREF TextColor, COLORREF BackColor)
{
    CustomTextBox* pData = CustomTextBox_GetOptions(hCtrl);
    if (pData != nullptr) {
        if (pData->hBackBrush) DeleteBrush(pData->hBackBrush);
        pData->hBackBrush = CreateSolidBrush(BackColor);
        pData->BackColor = BackColor;
        pData->TextColor = TextColor;
        CustomTextBox_SetOptions(hCtrl, pData);
        AfxRedrawWindow(hCtrl);
    }
}


// ========================================================================================
// Set the font for the custom control.
// ========================================================================================
void CustomTextBox_SetFont(HWND hCtrl, std::wstring wszFontName, int FontSize)
{
    CustomTextBox* pData = CustomTextBox_GetOptions(hCtrl);
    if (pData != nullptr) {
        if (pData->hFontText) DeleteFont(pData->hFontText);
        pData->wszFontName = wszFontName;
        pData->FontSize = FontSize;
        pData->hFontText = Main.CreateFont(wszFontName, FontSize, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET);
        if (pData->hFontText) SendMessage(pData->hTextBox, WM_SETFONT, (WPARAM)pData->hFontText, false);
        CustomTextBox_SetOptions(hCtrl, pData);
        AfxRedrawWindow(hCtrl);
    }
}


// ========================================================================================
// Set the border attributes for the custom control (width, colors, style).
// ========================================================================================
void CustomTextBox_SetBorderAttributes(
    HWND hCtrl, int BorderWidth, COLORREF clrGotFocus, COLORREF clrLostFocus, CustomTextBoxBorder BorderStyle)
{
    CustomTextBox* pData = CustomTextBox_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->BorderWidth = BorderWidth;
        pData->BorderColorFocus = clrGotFocus;
        pData->BorderColor = clrLostFocus;
        pData->BorderStyle = BorderStyle;
        CustomTextBox_SetOptions(hCtrl, pData);
        AfxRedrawWindow(hCtrl);
    }
}


// ========================================================================================
// Set the numeric attributes for the custom control.
// ========================================================================================
void CustomTextBox_SetNumericAttributes(
    HWND hCtrl, int NumDecimals, CustomTextBoxNegative AllowNegative, CustomTextBoxFormatting AllowFormatting)
{
    CustomTextBox* pData = CustomTextBox_GetOptions(hCtrl);
    if (pData != nullptr) {
        pData->isNumeric = true;
        pData->NumDecimals = NumDecimals;
        pData->AllowNegative = AllowNegative;
        pData->AllowFormatting = AllowFormatting;
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
    int Alignment,
    std::wstring wszText,
    int nLeft,
    int nTop,
    int nWidth,
    int nHeight)
{
    std::wstring wszClassName(L"CUSTOMTEXTBOX_CONTROL");

    WNDCLASSEX wcex{};

    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hWndParent, GWLP_HINSTANCE);

    if (GetClassInfoEx(hInst, wszClassName.c_str(), &wcex) == 0) {
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
        wcex.lpszClassName = wszClassName.c_str();
        if (RegisterClassEx(&wcex) == 0) return 0;
    }

    float rx = AfxScaleRatioX();
    float ry = AfxScaleRatioY();


    HWND hCtl =
        CreateWindowEx(0, wszClassName.c_str(), L"",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
            (int)(nLeft * rx), (int)(nTop * ry), (int)(nWidth * rx), (int)(nHeight * ry),
            hWndParent, (HMENU)CtrlId, hInst, (LPVOID)NULL);

    if (hCtl) {
        CustomTextBox* pData = new CustomTextBox;

        pData->hWindow = hCtl;
        pData->hParent = hWndParent;
        pData->hInst = hInst;
        pData->CtrlId = CtrlId;
        pData->wszFontName = L"Segoe UI";
        pData->FontSize = 9;
        pData->BackColor = GetThemeCOLORREF(ThemeElement::GrayDark);
        pData->hBackBrush = CreateSolidBrush(pData->BackColor);
        pData->TextColor = GetThemeCOLORREF(ThemeElement::WhiteLight);
        pData->HTextMargin = 0;
        pData->VTextMargin = 0;
        pData->BorderWidth = 0;
        pData->BorderColorFocus = GetThemeCOLORREF(ThemeElement::WhiteLight);
        pData->BorderColor = GetThemeCOLORREF(ThemeElement::WhiteDark);
        pData->Alignment = Alignment;
        pData->BorderStyle = CustomTextBoxBorder::BorderNone;


        // Create the child embedded TextBox control
        pData->hTextBox =
            CreateWindowEx(0, L"Edit", L"",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | Alignment | ES_AUTOHSCROLL,
                0, 0, 0, 0, hCtl, (HMENU)100, hInst, (LPVOID)NULL);
        SetWindowTheme(pData->hTextBox, L"", L"");

        // Subclass the child TextBox if pWndProc is not null
        if (pData->hTextBox)
            SetWindowSubclass(pData->hTextBox, CustomTextBox_SubclassProc, 100, NULL);

        AfxSetWindowText(pData->hTextBox, wszText);

        CustomTextBox_SetOptions(hCtl, pData);

        // Create and set the text font
        CustomTextBox_SetFont(hCtl, pData->wszFontName, pData->FontSize);

        // If numeric then format and display the text to correct number of decimal places
        CustomTextBox_FormatDisplayDecimalPlaces(pData);

        // Move the child textbox into place based on border width
        CustomTextBox_OnSize(hCtl);
    }

    return hCtl;
}


