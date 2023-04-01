#pragma once

#include "AfxWin.h"

enum class Controls
{
    None = 0,
    Button, OptionButton, CheckBox, Label, Custom, Frame, Line, TextBox,
    MultilineTextBox, ComboBox, ListBox, ProgressBar, TreeView, ListView,
    DateTimePicker, MonthCalendar, TabControl, StatusBar, SizeGrip,
    HScrollBar, VScrollBar, Slider, UpDown, RichEdit
};


template <class DERIVED_TYPE>
class CWindowBase
{
public:

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        DERIVED_TYPE* pThis = NULL;

        if (uMsg == WM_NCCREATE)
        {

            CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
            pThis = (DERIVED_TYPE*)pCreate->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);

            pThis->m_hwnd = hwnd;
        }
        else
        {
            pThis = (DERIVED_TYPE*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        }
        if (pThis)
        {
            return pThis->HandleMessage(uMsg, wParam, lParam);
        }
        else
        {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }


    CWindowBase() : m_hwnd(NULL)
    {
        // Instance handle
        m_hInstance = GetModuleHandle(NULL);

        // Default font name and size
        m_wszDefaultFontName = AfxGetDefaultFont();
        m_DefaultFontSize = 9;

        HDC hDC = GetDC(HWND_DESKTOP);
        m_rx = (float)(GetDeviceCaps(hDC, LOGPIXELSX) / 96.0f);
        m_ry = (float)(GetDeviceCaps(hDC, LOGPIXELSY) / 96.0f);
        ReleaseDC(HWND_DESKTOP, hDC);

        m_hAccel = NULL;

    }


    ~CWindowBase()
    {
        if (m_hFont) DeleteObject(m_hFont);
        if (m_wszClassName.length()) UnregisterClass(m_wszClassName.c_str(), m_hInstance);
    }


    HWND Create(
        HWND hWndParent = NULL,
        std::wstring wszTitle = L"",
        int x = CW_USEDEFAULT,
        int y = CW_USEDEFAULT,
        int nWidth = CW_USEDEFAULT,
        int nHeight = CW_USEDEFAULT,
        DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        DWORD dwExStyle = WS_EX_CONTROLPARENT | WS_EX_WINDOWEDGE)
    {
        // Generate class name based on unique memory address of class
        m_wszClassName = L"CWindowClass:" + std::to_wstring((unsigned long long)this);

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
        wcex.lpszClassName = m_wszClassName.c_str();
        wcex.hIcon = 0;
        wcex.hIconSm = 0;

        // Register the class
        RegisterClassEx(&wcex);

        m_hwnd = CreateWindowEx(
            dwExStyle,
            m_wszClassName.c_str(),
            wszTitle.c_str(),
            dwStyle,
            (x == CW_USEDEFAULT) ? CW_USEDEFAULT : (int)(x * m_rx),
            (y == CW_USEDEFAULT) ? CW_USEDEFAULT : (int)(y * m_ry),
            (nWidth == CW_USEDEFAULT) ? CW_USEDEFAULT : (int)(nWidth * m_rx),
            (nHeight == CW_USEDEFAULT) ? CW_USEDEFAULT : (int)(nHeight * m_ry),
            hWndParent, NULL, m_hInstance, (HANDLE)this);


        // Create a default font
        if (m_hFont == NULL)
            m_hFont = this->CreateFont(m_wszDefaultFontName, m_DefaultFontSize, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET);

        // Set the font
        if (m_hFont) SendMessage(m_hwnd, WM_SETFONT, (WPARAM)m_hFont, false);

        return m_hwnd;
    }


    HFONT CreateFont(
        std::wstring wszFaceName,        // __in Typeface name of font
        int lPointSize,                  // __in Point size
        int lWeight,                     // __in Font weight(bold etc.)
        BYTE bItalic,                   // __in CTRUE = italic
        BYTE bUnderline,                // __in CTRUE = underline
        BYTE bStrikeOut,                // __in CTRUE = strikeout
        BYTE bCharSet)                  // __in character set
    {
        LOGFONT tlfw;
        if (wszFaceName.empty()) wszFaceName = m_wszDefaultFontName;

        memset(&tlfw, 0x00, sizeof(LOGFONT));

        // lfFaceName must be at most 32 characters maximum
        int numChars = wszFaceName.length();
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
        std::wstring wszClassName,             // Control class
        HWND hParent,                          // Parent window handle
        LONG_PTR cID,                          // Control identifier
        std::wstring wszTitle,                 // Control caption
        int x,                                 // Horizontal position
        int y,                                 // Vertical position
        int nWidth,                            // Control width
        int nHeight,                           // Control height
        int dwStyle,                           // Control style
        int dwExStyle,                         // Extended style
        bool bSetFont,                         // SetFont flag
        LONG_PTR lpParam)                      // Pointer to custom data
    {
        // Don't allow negative values for the styles
        if (dwStyle == -1) dwStyle = 0;
        if (dwExStyle == -1) dwExStyle = 0;

        // Make sure that the control has the WS_CHILD style
        dwStyle = dwStyle | WS_CHILD;

        // Create the control
        HWND hCtl = CreateWindowEx(dwExStyle, wszClassName.c_str(), wszTitle.c_str(), dwStyle,
            (int)(x * m_rx), (int)(y * m_ry), (int)(nWidth * m_rx), (int)(nHeight * m_ry),
            hParent, (HMENU)cID, m_hInstance, (LPVOID)lpParam);
        if (hCtl == NULL) return NULL;
        // Set the font
        if (m_hFont) {
            if (bSetFont) SendMessage(hCtl, WM_SETFONT, (WPARAM)m_hFont, TRUE);
        }
        return hCtl;
    }


    HWND AddControl(
        Controls control,                             // Control type (Controls Enum)
        HWND hParent = NULL,                          // Parent window handle
        LONG_PTR cID = 0,                             // Control identifier
        std::wstring wszTitle = L"",                  // Control caption
        int x = 0,                                    // Horizontal position
        int y = 0,                                    // Vertical position
        int nWidth = 0,                               // Control width
        int nHeight = 0,                              // Control height
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
        std::wstring wszClassName;


        switch (control) {
        case Controls::None:
        {
            return NULL;
        }
        break;

        case Controls::Button:
        {
            if (dwStyle == BS_FLAT) dwStyle = WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER | BS_FLAT;
            if (dwStyle == BS_DEFPUSHBUTTON) dwStyle = WS_VISIBLE | WS_TABSTOP | BS_CENTER | BS_VCENTER | BS_DEFPUSHBUTTON;
            if (dwStyle == BS_OWNERDRAW) dwStyle = WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW;
#if _WIN32_WINNT == x0602
            if (dwStyle == BS_SPLITBUTTON) dwStyle = WS_VISIBLE | WS_TABSTOP | BS_CENTER | BS_VCENTER | BS_SPLITBUTTON;
            if (dwStyle == BS_DEFSPLITBUTTON) dwStyle = WS_VISIBLE | WS_TABSTOP | BS_CENTER | BS_VCENTER | BS_DEFSPLITBUTTON;
#endif
            if (dwStyle == -1) dwStyle = WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER | BS_FLAT;
            wszClassName = L"Button";
            hCtl = CreateControl(wszClassName, hParent, cID, wszTitle, x, y, nWidth, nHeight, dwStyle, dwExStyle, bSetFont, lpParam);
        }
        break;

        case Controls::OptionButton:
        {

        }
        break;

        case Controls::CheckBox:
        {

        }
        break;

        case Controls::Label:
        {
            if (dwStyle == -1) dwStyle = WS_VISIBLE | SS_LEFT | WS_GROUP | SS_NOTIFY;
            wszClassName = L"Static";
            hCtl = CreateControl(wszClassName, hParent, cID, wszTitle, x, y, nWidth, nHeight, dwStyle, dwExStyle, bSetFont, lpParam);
        }
        break;

        case Controls::Custom:
        {
            if (dwStyle == -1) dwStyle = WS_VISIBLE | SS_NOTIFY;
            wszClassName = L"Static";
            hCtl = CreateControl(wszClassName, hParent, cID, wszTitle, x, y, nWidth, nHeight, dwStyle, dwExStyle, bSetFont, lpParam);
        }
        break;

        case Controls::Frame:
        {

        }
        break;

        case Controls::Line:
        {

        }
        break;

        case Controls::TextBox:
        {

        }
        break;

        case Controls::MultilineTextBox:
        {

        }
        break;

        case Controls::ComboBox:
        {

        }
        break;

        case Controls::ListBox:
        {
            if (dwStyle == -1) dwStyle = WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | WS_BORDER | WS_TABSTOP | LBS_STANDARD | LBS_HASSTRINGS | LBS_SORT | LBS_NOTIFY;
            wszClassName = L"Listbox";
            hCtl = CreateControl(wszClassName, hParent, cID, wszTitle, x, y, nWidth, nHeight, dwStyle, dwExStyle, bSetFont, lpParam);
            // Adjust the height of the control so that the integral height
            // is based on the new font rather than the default SYSTEM_FONT
            SetWindowPos(hCtl, NULL, x, y, nWidth, nHeight, SWP_NOZORDER);
        }
        break;

        case Controls::ProgressBar:
        {

        }
        break;

        case Controls::TreeView:
        {

        }
        break;

        case Controls::ListView:
        {

        }
        break;

        case Controls::DateTimePicker:
        {

        }
        break;

        case Controls::MonthCalendar:
        {

        }
        break;

        case Controls::TabControl:
        {

        }
        break;

        case Controls::StatusBar:
        {

        }
        break;

        case Controls::SizeGrip:
        {

        }
        break;

        case Controls::HScrollBar:
        {

        }
        break;

        case Controls::VScrollBar:
        {

        }
        break;

        case Controls::Slider:
        {

        }
        break;

        case Controls::UpDown:
        {

        }
        break;

        case Controls::RichEdit:
        {

        }
        break;

        default:
        {
            return NULL;
        }
        break;
        }

        // Subclass the control if pWndProc is not null
        if (hCtl) {
            if (pWndProc != nullptr) {
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

    HWND  m_hwnd;
    HFONT m_hFont = NULL;

    HINSTANCE m_hInstance;
    HACCEL m_hAccel;
    std::wstring m_wszClassName;
    std::wstring m_wszDefaultFontName;
    int m_DefaultFontSize;
    float m_rx = 1;
    float m_ry = 1;
};
