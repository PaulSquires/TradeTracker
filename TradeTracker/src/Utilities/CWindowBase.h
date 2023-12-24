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

#pragma once

#include "AfxWin.h"

#include "Utilities/Colors.h"

#include <dwmapi.h>

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif


enum class Controls {
    None = 0,
    Button, OptionButton, CheckBox, Label, Custom, Frame, Line, TextBox,
    MultilineTextBox, ComboBox, ListBox, ProgressBar, TreeView, ListView,
    Header, DateTimePicker, MonthCalendar, TabControl, StatusBar, SizeGrip,
    HScrollBar, CustomVScrollBar, Slider, UpDown, RichEdit
};


template <class DERIVED_TYPE>
class CWindowBase {
public:

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        DERIVED_TYPE* pThis = NULL;

        if (uMsg == WM_NCCREATE) {
            CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
            pThis = (DERIVED_TYPE*)pCreate->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);

            pThis->m_hwnd = hwnd;
        }
        else {
            pThis = (DERIVED_TYPE*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        }
        if (pThis) {
            return pThis->HandleMessage(uMsg, wParam, lParam);
        }
        else {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }

    CWindowBase() : m_hwnd(NULL) {
        // Instance handle
        m_hInstance = GetModuleHandle(NULL);

        // Default font name and size
        m_wszDefaultFontName = AfxGetDefaultFont();
        m_Defaultfont_size = 9;

        HDC hDC = GetDC(HWND_DESKTOP);
        m_rx = (float)(GetDeviceCaps(hDC, LOGPIXELSX) / 96.0f);
        m_ry = (float)(GetDeviceCaps(hDC, LOGPIXELSY) / 96.0f);
        ReleaseDC(HWND_DESKTOP, hDC);

        m_hAccel = NULL;

        // Create a default font
        if (m_hFont == NULL) {
            m_hFont = this->CreateFont(m_wszDefaultFontName, m_Defaultfont_size, FW_NORMAL, false, false, false, DEFAULT_CHARSET);
        }

        // Generate class name based on unique memory address of class
        m_class_name_text = L"CWindowClass:" + std::to_wstring((unsigned long long)this);

        INITCOMMONCONTROLSEX icc;
        icc.dwSize = sizeof(icc);
        icc.dwICC = ICC_NATIVEFNTCTL_CLASS | ICC_COOL_CLASSES | ICC_BAR_CLASSES |
            ICC_TAB_CLASSES | ICC_USEREX_CLASSES | ICC_WIN95_CLASSES |
            ICC_STANDARD_CLASSES | ICC_ANIMATE_CLASS | ICC_DATE_CLASSES |
            ICC_HOTKEY_CLASS | ICC_INTERNET_CLASSES | ICC_LISTVIEW_CLASSES | 
            ICC_PAGESCROLLER_CLASS | ICC_PROGRESS_CLASS | ICC_TREEVIEW_CLASSES |
            ICC_UPDOWN_CLASS;
        InitCommonControlsEx(&icc);
    }

    ~CWindowBase() {
        if (m_hFont) DeleteObject(m_hFont);
        if (m_hRichEditLib) FreeLibrary(m_hRichEditLib);
        if (m_class_name_text.length()) UnregisterClass(m_class_name_text.c_str(), m_hInstance);
    }

    HWND Create(
        HWND hWndParent = NULL,
        std::wstring wszTitle = L"",
        int x = CW_USEDEFAULT,
        int y = CW_USEDEFAULT,
        int width = CW_USEDEFAULT,
        int height = CW_USEDEFAULT,
        DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        DWORD dwExStyle = WS_EX_CONTROLPARENT | WS_EX_WINDOWEDGE)
    {

        WNDCLASSEXW wcex = { 0 };
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = DERIVED_TYPE::WindowProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = sizeof(HANDLE);
        wcex.hInstance = GetModuleHandle(NULL);
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszMenuName = NULL;
        wcex.lpszClassName = m_class_name_text.c_str();
        wcex.hIcon = 0;
        wcex.hIconSm = 0;

        // Register the class
        RegisterClassEx(&wcex);

        m_hwnd = CreateWindowEx(
            dwExStyle,
            m_class_name_text.c_str(),
            wszTitle.c_str(),
            dwStyle,
            (x == CW_USEDEFAULT) ? CW_USEDEFAULT : (int)(x * m_rx),
            (y == CW_USEDEFAULT) ? CW_USEDEFAULT : (int)(y * m_ry),
            (width == CW_USEDEFAULT) ? CW_USEDEFAULT : (int)(width * m_rx),
            (height == CW_USEDEFAULT) ? CW_USEDEFAULT : (int)(height * m_ry),
            hWndParent, NULL, m_hInstance, (HANDLE)this);

        // Set the font
        if (m_hFont) SendMessage(m_hwnd, WM_SETFONT, (WPARAM)m_hFont, false);

        return m_hwnd;
    }


    HFONT CreateFont(
        std::wstring wszFaceName,       // __in Typeface name of font
        int lPointSize,                 // __in Point size
        int lWeight,                    // __in Font weight(bold etc.)
        BYTE bItalic,                   // __in CTRUE = italic
        BYTE bUnderline,                // __in CTRUE = underline
        BYTE bStrikeOut,                // __in CTRUE = strikeout
        BYTE bCharSet)                  // __in character set
    {
        LOGFONT tlfw;
        if (wszFaceName.empty()) wszFaceName = m_wszDefaultFontName;

        memset(&tlfw, 0x00, sizeof(LOGFONT));

        // lfFaceName must be at most 32 characters maximum
        size_t numChars = wszFaceName.length();
        if (numChars > 32) numChars = 32;
        wmemcpy(&tlfw.lfFaceName[0], wszFaceName.c_str(), numChars);

        HDC hDC = GetDC(HWND_DESKTOP);

        tlfw.lfHeight = -MulDiv(lPointSize, GetDeviceCaps(hDC, LOGPIXELSY), 72);
        tlfw.lfWidth = 0;
        tlfw.lfEscapement = 0;
        tlfw.lfOrientation = 0;
        tlfw.lfWeight = lWeight;
        tlfw.lfItalic = bItalic;
        tlfw.lfUnderline = bUnderline;
        tlfw.lfStrikeOut = bStrikeOut;
        tlfw.lfCharSet = bCharSet;
        tlfw.lfOutPrecision = OUT_TT_PRECIS;
        tlfw.lfClipPrecision = CLIP_DEFAULT_PRECIS;
        tlfw.lfQuality = DEFAULT_QUALITY;
        tlfw.lfPitchAndFamily = FF_DONTCARE;

        ReleaseDC(HWND_DESKTOP, hDC);

        return CreateFontIndirect(&tlfw);
    }


    //' =====================================================================================
    //' Internal function called by AddControl()
    //' =====================================================================================
    HWND CreateControl(
        std::wstring class_name_text,             // Control class
        HWND hParent,                          // Parent window handle
        LONG_PTR cID,                          // Control identifier
        std::wstring wszTitle,                 // Control caption
        int x,                                 // Horizontal position
        int y,                                 // Vertical position
        int width,                            // Control width
        int height,                           // Control height
        int dwStyle,                           // Control style
        int dwExStyle,                         // Extended style
        LONG_PTR lpParam)                      // Pointer to custom data
    {
        // Don't allow negative values for the styles
        if (dwStyle == -1) dwStyle = 0;
        if (dwExStyle == -1) dwExStyle = 0;

        // Make sure that the control has the WS_CHILD style
        dwStyle = dwStyle | WS_CHILD;

        // Create the control
        HWND hCtl = CreateWindowEx(dwExStyle, class_name_text.c_str(), wszTitle.c_str(), dwStyle,
            (int)(x * m_rx), (int)(y * m_ry), (int)(width * m_rx), (int)(height * m_ry),
            hParent, (HMENU)cID, m_hInstance, (LPVOID)lpParam);
        if (hCtl == NULL) return NULL;

        return hCtl;
    }


    HWND AddControl(
        Controls control,                             // Control type (Controls Enum)
        HWND hParent = NULL,                          // Parent window handle
        LONG_PTR cID = 0,                             // Control identifier
        std::wstring wszTitle = L"",                  // Control caption
        int x = 0,                                    // Horizontal position
        int y = 0,                                    // Vertical position
        int width = 0,                               // Control width
        int height = 0,                              // Control height
        int dwStyle = -1,                             // Control style
        int dwExStyle = -1,                           // Extended style
        LONG_PTR lpParam = 0,                         // Pointer to custom data
        SUBCLASSPROC pWndProc = NULL,                 // Address of the window callback procedure
        UINT_PTR uIdSubclass = 0xFFFFFFFF,            // The subclass ID
        DWORD_PTR dwRefData = NULL)                   // Pointer to reference data
    {
        HWND hCtl = NULL;
        if (hParent == NULL) hParent = m_hwnd;

        bool bSetFont = true;
        std::wstring class_name_text;


        switch (control) {
        case Controls::None: {
            return NULL;
        }
        break;

        case Controls::Button: {
            if (dwStyle == BS_FLAT) dwStyle = WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER | BS_FLAT;
            if (dwStyle == BS_DEFPUSHBUTTON) dwStyle = WS_VISIBLE | WS_TABSTOP | BS_CENTER | BS_VCENTER | BS_DEFPUSHBUTTON;
            if (dwStyle == BS_OWNERDRAW) dwStyle = WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW;
#if _WIN32_WINNT == x0602
            if (dwStyle == BS_SPLITBUTTON) dwStyle = WS_VISIBLE | WS_TABSTOP | BS_CENTER | BS_VCENTER | BS_SPLITBUTTON;
            if (dwStyle == BS_DEFSPLITBUTTON) dwStyle = WS_VISIBLE | WS_TABSTOP | BS_CENTER | BS_VCENTER | BS_DEFSPLITBUTTON;
#endif
            if (dwStyle == -1) dwStyle = WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER | BS_FLAT;
            class_name_text = L"Button";
            hCtl = CreateControl(class_name_text, hParent, cID, wszTitle, x, y, width, height, dwStyle, dwExStyle, lpParam);
            break;
        }

        case Controls::OptionButton: {
            if (dwStyle == WS_GROUP) dwStyle = WS_VISIBLE | WS_TABSTOP | BS_AUTORADIOBUTTON | BS_LEFT | BS_VCENTER | WS_GROUP;
            if (dwStyle == -1) dwStyle = WS_VISIBLE | WS_TABSTOP | BS_AUTORADIOBUTTON | BS_LEFT | BS_VCENTER;
            class_name_text = L"Button";
            hCtl = CreateControl(class_name_text, hParent, cID, wszTitle, x, y, width, height, dwStyle, dwExStyle, lpParam);
            break;
        }

        case Controls::CheckBox: {
            if (dwStyle == -1) dwStyle = WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX | BS_LEFT | BS_VCENTER;
            class_name_text = L"Button";
            hCtl = CreateControl(class_name_text, hParent, cID, wszTitle, x, y, width, height, dwStyle, dwExStyle, lpParam);
            break;
        }

        case Controls::Label: {
            if (dwStyle == -1) dwStyle = WS_VISIBLE | SS_LEFT | WS_GROUP | SS_NOTIFY;
            class_name_text = L"Static";
            hCtl = CreateControl(class_name_text, hParent, cID, wszTitle, x, y, width, height, dwStyle, dwExStyle, lpParam);
            break;
        }

        case Controls::Custom: {
            if (dwStyle == -1) dwStyle = WS_VISIBLE | SS_NOTIFY;
            class_name_text = L"Static";
            hCtl = CreateControl(class_name_text, hParent, cID, wszTitle, x, y, width, height, dwStyle, dwExStyle, lpParam);
            break;
        }

        case Controls::Frame: {
            bSetFont = false;
            if (dwStyle == -1) dwStyle = WS_VISIBLE | WS_CLIPSIBLINGS | WS_GROUP | SS_GRAYFRAME;
            if (dwExStyle == -1) dwExStyle = WS_EX_TRANSPARENT;
            class_name_text = L"Static";
            hCtl = CreateControl(class_name_text, hParent, cID, wszTitle, x, y, width, height, dwStyle, dwExStyle, lpParam);
            break;
        }

        case Controls::Line: {
            bSetFont = false;
            if (dwStyle == -1) dwStyle = WS_VISIBLE | SS_ETCHEDFRAME;
            if (dwExStyle == -1) dwExStyle = WS_EX_TRANSPARENT;
            class_name_text = L"Static";
            hCtl = CreateControl(class_name_text, hParent, cID, wszTitle, x, y, width, height, dwStyle, dwExStyle, lpParam);
            break;
        }

        case Controls::TextBox: {
            if (dwExStyle == -1) dwExStyle = WS_EX_CLIENTEDGE;
            if (dwStyle == -1) dwStyle = WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL;
            class_name_text = L"Edit";
            hCtl = CreateControl(class_name_text, hParent, cID, wszTitle, x, y, width, height, dwStyle, dwExStyle, lpParam);
            break;
        }

        case Controls::MultilineTextBox: {
            if (dwExStyle == -1) dwExStyle = WS_EX_CLIENTEDGE;
            if (dwStyle == -1) dwStyle = WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | ES_LEFT | ES_AUTOHSCROLL | ES_MULTILINE | ES_NOHIDESEL | ES_WANTRETURN;
            class_name_text = L"Edit";
            hCtl = CreateControl(class_name_text, hParent, cID, wszTitle, x, y, width, height, dwStyle, dwExStyle, lpParam);
            break;
        }

        case Controls::RichEdit: {
            if (dwExStyle == -1) dwExStyle = WS_EX_CLIENTEDGE;
            if (dwStyle == -1) dwStyle = WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | ES_LEFT | ES_AUTOHSCROLL | ES_MULTILINE | ES_NOHIDESEL | ES_WANTRETURN;
            class_name_text = L"RichEdit50W";
            hCtl = CreateControl(class_name_text, hParent, cID, wszTitle, x, y, width, height, dwStyle, dwExStyle, lpParam);
            m_hRichEditLib = LoadLibrary(L"MSFTEDIT.DLL");
            break;
        }

        case Controls::ComboBox: {
            if (dwExStyle == -1) dwExStyle = WS_EX_CLIENTEDGE;
            if (dwStyle == -1) dwStyle = WS_VISIBLE | WS_VSCROLL | WS_TABSTOP | CBS_DROPDOWNLIST | CBS_HASSTRINGS;
            class_name_text = L"ComboBox";
            hCtl = CreateControl(class_name_text, hParent, cID, wszTitle, x, y, width, height, dwStyle, dwExStyle, lpParam);
            break;
        }

        case Controls::ListBox: {
            if (dwStyle == -1) dwStyle = WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | WS_BORDER | WS_TABSTOP | LBS_STANDARD | LBS_HASSTRINGS | LBS_SORT | LBS_NOTIFY;
            class_name_text = L"Listbox";
            hCtl = CreateControl(class_name_text, hParent, cID, wszTitle, x, y, width, height, dwStyle, dwExStyle, lpParam);
            // Adjust the height of the control so that the integral height
            // is based on the new font rather than the default SYSTEM_FONT
            SetWindowPos(hCtl, NULL, x, y, width, height, SWP_NOZORDER);
            break;
        }

        case Controls::Header: {
            if (dwStyle == -1) dwStyle = WS_VISIBLE | CCS_TOP | HDS_HORZ | HDS_NOSIZING;
            class_name_text = L"SysHeader32";
            hCtl = CreateControl(class_name_text, hParent, cID, wszTitle, x, y, width, height, dwStyle, dwExStyle, lpParam);
            break;
        }

        case Controls::ListView: {
            if (dwExStyle == -1) dwExStyle = WS_EX_CLIENTEDGE;
            if (dwStyle == -1) dwStyle = WS_VISIBLE | WS_CLIPCHILDREN | WS_TABSTOP | LVS_REPORT | 
                LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS | LVS_AUTOARRANGE | LVS_ALIGNTOP;
            class_name_text = L"SysListView32";
            hCtl = CreateControl(class_name_text, hParent, cID, wszTitle, x, y, width, height, dwStyle, dwExStyle, lpParam);
            break;
        }

        case Controls::DateTimePicker: {
            if (dwExStyle == -1) dwExStyle = 0;
            if (dwStyle == -1) dwStyle = WS_VISIBLE | WS_TABSTOP | DTS_SHORTDATEFORMAT;
            class_name_text = L"SysDateTimePick32";
            hCtl = CreateControl(class_name_text, hParent, cID, wszTitle, x, y, width, height, dwStyle, dwExStyle, lpParam);
            // Sets the font to be used by the date and time picker control's child month calendar control.
            if (m_hFont) SendMessage(hCtl, DTM_SETMCFONT, (WPARAM)m_hFont, true);
            break;
        }

        default: {
            return NULL;
        }

        }


        // Set the font
        if (hCtl) {
            if (m_hFont) {
                if (bSetFont) SendMessage(hCtl, WM_SETFONT, (WPARAM)m_hFont, true);
            }
        }

        // Subclass the control if pWndProc is not null
        if (hCtl) {
            if (pWndProc) {
                if (uIdSubclass == 0xFFFFFFFF) {
                    SetProp(hCtl, L"OLDWNDPROC", (HANDLE)SetWindowLongPtr(hCtl, GWLP_WNDPROC, (LONG_PTR)pWndProc));
                }
                else {
                    SetWindowSubclass(hCtl, pWndProc, uIdSubclass, dwRefData);
                }
            }
        }

        return hCtl;
    }

    HWND WindowHandle() const { return m_hwnd; }
    HINSTANCE hInst() const { return m_hInstance; }
    HACCEL hAccel() const { return m_hAccel; }


protected:
    virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;

    HWND  m_hwnd = NULL;
    HFONT m_hFont = NULL;
    HINSTANCE m_hInstance = NULL;
    HACCEL m_hAccel = NULL;
    HMODULE m_hRichEditLib = NULL;
    std::wstring m_class_name_text;
    std::wstring m_wszDefaultFontName;
    int m_Defaultfont_size = 9;
    float m_rx = 1;
    float m_ry = 1;
};
