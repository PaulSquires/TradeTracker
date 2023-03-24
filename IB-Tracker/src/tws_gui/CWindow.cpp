#include "pch.h"
#include "CWindow.h"



//' ========================================================================================
//' CWindow class constructor
//' Usage:
//'    DIM pWindow AS CWindow
//' Remarks:
//' CWindow will use "FBWindowClass:" and a number as the window class.
//' ========================================================================================
CWindow::CWindow()
{
    // Class name
    m_wszClassName = L"";

    // Instance handle
    m_hInstance = GetModuleHandle(NULL);

    // Scale windows according to the DPI setting.
    HDC hDC = GetDC(NULL);

    // Resolution ratio = current resolution / 96.0 (using floating point math)
    m_rx = (float)(GetDeviceCaps(hDC, LOGPIXELSX) / 96.0);
    m_ry = (float)(GetDeviceCaps(hDC, LOGPIXELSY) / 96.0);
    ReleaseDC(NULL, hDC);

    // Default font name and size
    m_wszDefaultFontName = L"Segoe UI";
    m_DefaultFontSize = 9;

    // Initialize the common controls library
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


//' ========================================================================================
//' CWindow class destructor
//' ========================================================================================
CWindow::~CWindow()
{
    if (m_hFont) DeleteObject(m_hFont);
    if (m_hAccel) DestroyAcceleratorTable(m_hAccel);
    if (m_wszClassName.length()) UnregisterClass(m_wszClassName.c_str(), m_hInstance);
    if (m_hRichEditLib) FreeLibrary(m_hRichEditLib);
}


//' ========================================================================================
//' Default CWindow callback function.
//' ========================================================================================
LRESULT CALLBACK CWindow_WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {

    case WM_CREATE:
    {
    }
    break;

    case WM_COMMAND:
    {
    }
    break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: Add any drawing code that uses hdc here...
        EndPaint(hWnd, &ps);
    }
    break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;

}


//' ========================================================================================
//' Window creation
//' Parameters:
//' - hParent     = Parent window handle
//' - wszTitle    = Window caption
//' - lpfnWndProc = Address of the callback function
//' - x           = Horizontal position
//' - y           = Vertical position
//' - nWidth      = Window width
//' - nHeight     = Window height
//' - dwStyle     = Window style
//' - dwExStyle   = Extended style
//' ========================================================================================
HWND CWindow::Create(HWND hParent, std::wstring wszTitle, WNDPROC lpfnWndProc,
    int x, int y, int nWidth, int nHeight, DWORD dwStyle, DWORD dwExStyle)
{
    if (m_hwnd) return NULL;

    static int nCount;

    m_wszClassName = L"FBWindowClass:" + nCount;

    // Default handler
    if (lpfnWndProc == nullptr) lpfnWndProc = CWindow_WindowProc;

    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = lpfnWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = sizeof(HANDLE);
    wcex.hInstance = m_hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = m_wszClassName.c_str();
    wcex.hIcon = 0;
    wcex.hIconSm = 0;

    // Register the class
    m_wAtom = RegisterClassEx(&wcex);

    // Increment the class counter
    if (m_wAtom) nCount += 1;

    // Create a default font
    if (m_hFont == NULL) 
        m_hFont = this->CreateFont(m_wszDefaultFontName, m_DefaultFontSize, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET);
    
    // Create the window
    m_hwnd = CreateWindowEx(
        dwExStyle,
        (LPCWSTR)(ULONG_PTR)(WORD)m_wAtom,
        wszTitle.c_str(),
        dwStyle,
        (x == CW_USEDEFAULT) ? CW_USEDEFAULT : (int)(x * m_rx),
        (y == CW_USEDEFAULT) ? CW_USEDEFAULT : (int)(y * m_ry),
        (nWidth == CW_USEDEFAULT) ? CW_USEDEFAULT : (int)(nWidth * m_rx),
        (nHeight == CW_USEDEFAULT) ? CW_USEDEFAULT : (int)(nHeight * m_ry),
        hParent, NULL, m_hInstance, (HANDLE)this);

    if (m_hwnd == NULL) return 0;


    // Set the font
    if (m_hFont) SendMessage(m_hwnd, WM_SETFONT, (WPARAM)m_hFont, false);

    // Store the class pointer
    SetWindowLongPtr(m_hwnd, 0, (LONG_PTR)this);

    return m_hwnd;

}


//' ========================================================================================
//' Processes window messages
//' Uses IsDialogMessage in the message pump
//' Note: To process arrow keys, characters, enter, insert, backspace | delete keys, set USEDLGMSG = 0.
//' | you can leave it as is and process the WM_GETDLGCODE message:
//' CASE WM_GETDLGCODE
//'    FUNCTION = DLGC_WANTALLKEYS
//' If you are only interested in arrow keys and characters...
//' CASE WM_GETDLGCODE
//'    FUNCTION = DLGC_WANTARROWS | DLGC_WANTCHARS
#ifndef USEDLGMSG
#define USEDLGMSG 1
#endif
//' ========================================================================================
WPARAM CWindow::DoEvents(int nCmdShow)
{
    MSG uMsg;
    if (m_hwnd == NULL) return 0;

    // Show the window and update its client area
    ShowWindow(m_hwnd, (nCmdShow == 0) ? SW_SHOW : nCmdShow);
    UpdateWindow(m_hwnd);

    // Message loop
    while (GetMessage(&uMsg, NULL, 0, 0))
    {
        // Processes accelerator keys for menu commands
        if (m_hAccel == NULL || (!TranslateAccelerator(m_hwnd, m_hAccel, &uMsg))) {
#if (USEDLGMSG >= 1)
            // Determines whether a message is intended for the specified
            // dialog box and, if it is, processes the message.
            if (!IsDialogMessage(m_hwnd, &uMsg)) {
                // Translates virtual-key messages into character messages.
                TranslateMessage(&uMsg);
                // Dispatches a message to a window procedure.
                DispatchMessage(&uMsg);
            }
#else
            // Translates virtual-key messages into character messages.
            TranslateMessage(uMsg);
            // Dispatches a message to a window procedure.
            DispatchMessage(uMsg);
#endif
        }
    }
    return uMsg.wParam;
}


//' ========================================================================================
//' Creates a High DPI aware logical font.
//' - wszFaceName = The typeface name.
//' - lPointSize = The point size.
//' - lWeight = The weight of the font in the range 0 through 1000. For example, 400 is normal
//'      and 700 is bold. If this value is zero, a default weight is used.
//'      The following values are defined for convenience.
//'      FW_DONTCARE (0), FW_THIN (100), FW_EXTRALIGHT (200), FW_ULTRALIGHT (200), FW_LIGHT (300),
//'      FW_NORMAL (400), FW_REGULAR (400), FW_MEDIUM (500), FW_SEMIBOLD (600), FW_DEMIBOLD (600),
//'      FW_BOLD (700), FW_EXTRABOLD (800), FW_ULTRABOLD (800), FW_HEAVY (900), FW_BLACK (900)
//' - bItalic = Italic flag. CTRUE | FALSE
//' - bUnderline = Underline flag. CTRUE | FALSE
//' - bStrikeOut = StrikeOut flag. CTRUE | FALSE
//' - bCharset = Charset.
//'      The following values are predefined: ANSI_CHARSET, BALTIC_CHARSET, CHINESEBIG5_CHARSET,
//'      DEFAULT_CHARSET, EASTEUROPE_CHARSET, GB2312_CHARSET, GREEK_CHARSET, HANGUL_CHARSET,
//'      MAC_CHARSET, OEM_CHARSET, RUSSIAN_CHARSET, SHIFTJIS_CHARSET, SYMBOL_CHARSET, TURKISH_CHARSET,
//'      VIETNAMESE_CHARSET, JOHAB_CHARSET (Korean language edition of Windows), ARABIC_CHARSET and
//'      HEBREW_CHARSET (Middle East language edition of Windows), THAI_CHARSET (Thai language
//'      edition of Windows).
//'      The OEM_CHARSET value specifies a character set that is operating-system dependent.
//'      DEFAULT_CHARSET is set to a value based on the current system locale. For example, when
//'      the system locale is English (United States), it is set as ANSI_CHARSET.
//'      Fonts with other character sets may exist in the operating system. If an application uses
//'      a font with an unknown character set, it should not attempt to translate | interpret
//'      strings that are rendered with that font.
//'      This parameter is important in the font mapping process. To ensure consistent results,
//'      specify a specific character set. If you specify a typeface name in the lfFaceName member,
//'      make sure that the lfCharSet value matches the character set of the typeface specified in lfFaceName.
//' Return value: The handle of the font | NULL on failure.
//' Remarks: The returned font must be destroyed with DeleteObject | the macro DeleteFont
//' when no longer needed to prevent memory leaks.
//' Usage examples:
//'   hFont = CWindow.CreateFont("MS Sans Serif", 8, FW_NORMAL, , , , DEFAULT_CHARSET)
//'   hFont = CWindow.CreateFont("Courier New", 10, FW_BOLD, , , , DEFAULT_CHARSET)
//'   hFont = CWindow.CreateFont("Marlett", 8, FW_NORMAL, , , , SYMBOL_CHARSET)
//' ========================================================================================
HFONT CWindow::CreateFont(
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
//' Creates a High DPI aware logical font and sets it as the default font.
//' Usage examples:
//'   CWindow.SetFont("MS Sans Serif", 8, FW_NORMAL, , , , DEFAULT_CHARSET)
//'   CWindow.SetFont("Courier New", 10, FW_BOLD, , , , DEFAULT_CHARSET)
//'   CWindow.SetFont("Marlett", 8, FW_NORMAL, , , , SYMBOL_CHARSET)
//' Return Value = TRUE | FALSE.
//' =====================================================================================
bool CWindow::SetFont(
    std::wstring wszFaceName,
    int lPointSize,
    int lWeight,
    BYTE bItalic,
    BYTE bUnderline,
    BYTE bStrikeOut,
    BYTE bCharSet)
{
    HFONT hFont = this->CreateFont(wszFaceName, lPointSize, lWeight, bItalic, bUnderline, bStrikeOut, bCharSet);
    if (hFont) {
        if (m_hFont) DeleteObject(m_hFont);
        m_hFont = hFont;
        m_wszDefaultFontName = wszFaceName;
        m_DefaultFontSize = lPointSize;
        return true;
    }
    else {
        return false;
    }
}


//' =====================================================================================
//' Gets the background brush.
//' =====================================================================================
HBRUSH CWindow::GetBrush()
{
    if (m_hwnd == NULL) return NULL;
    return (HBRUSH)GetClassLongPtr(m_hwnd, GCLP_HBRBACKGROUND);
}


//' =====================================================================================
//' Sets the background brush.
//' Handle to the class background brush. This member can be a handle to the physical
//' brush to be used for painting the background, or it can be a color value. A color
//' value must be one of the standard system colors (the value 1 must be added
//' to the chosen color), e.g. COLOR_WINDOW + 1.
//' You can also use CreateSolidBrush to create a logical brush with a solid color, e.g.
//' CreateSolidBrush(RGB(0, 0, 255)
//' =====================================================================================
void CWindow::SetBrush(HBRUSH hbrBackground)
{
    if (m_hwnd == NULL) return;
    SetClassLongPtr(m_hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)hbrBackground);
    InvalidateRect(m_hwnd, NULL, TRUE);
    UpdateWindow(m_hwnd);
}


//' =====================================================================================
//' Internal function called by AddControl()
//' =====================================================================================
HWND CWindow::CreateControl(
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
        hParent, (HMENU) cID, m_hInstance, (LPVOID)lpParam);

    if (hCtl == NULL) return NULL;

    // Set the font
    if (m_hFont) {
        if (bSetFont) SendMessage(hCtl, WM_SETFONT, (WPARAM)m_hFont, TRUE);
    }

    return hCtl;
}


    
//' =====================================================================================
//' Adds a control to the window
//' =====================================================================================
HWND CWindow::AddControl(
    Controls control,                      // Control type (Controls Enum)
    HWND hParent,                          // Parent window handle
    LONG_PTR cID,                          // Control identifier
    std::wstring wszTitle,                 // Control caption
    int x,                                 // Horizontal position
    int y,                                 // Vertical position
    int nWidth,                            // Control width
    int nHeight,                           // Control height
    int dwStyle,                           // Control style
    int dwExStyle,                         // Extended style
    LONG_PTR lpParam,                      // Pointer to custom data
    SUBCLASSPROC pWndProc,                 // Address of the window callback procedure
    UINT_PTR uIdSubclass,                  // The subclass ID
    DWORD_PTR dwRefData)                   // Pointer to reference data
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


//' ========================================================================================
//' Returns a pointer to the CWindow class given the handle of its associated window handle.
//' To retrieve it from the handle of any of its child windows | controls, use AfxCWindowOwnerPtr.
//' ========================================================================================
CWindow* AfxCWindowPtr(HWND hwnd)
{
    return (CWindow*)GetWindowLongPtr(hwnd, 0);
}


//' ========================================================================================
//' Returns a pointer to the CWindow class given the handle of the window created with it
//' | the handle of any of it's children.
//' ========================================================================================
CWindow* AfxCWindowOwnerPtr(HWND hwnd)
{
    if (hwnd == NULL) return nullptr;
    HWND hRootOwner = GetAncestor(hwnd, GA_ROOTOWNER);
    if (hRootOwner == NULL) return nullptr;
    return (CWindow*)GetWindowLongPtr(hRootOwner, 0);
}


//' ========================================================================================
//' Redraws the specified window.
//' Do not use it from within a WM_PAINT message.
//' ========================================================================================
void AfxRedrawWindow(HWND hwnd)
{
    InvalidateRect(hwnd, NULL, true);
    UpdateWindow(hwnd);
}


//' ========================================================================================
//' Retrieves the desktop horizontal scaling ratio.
//' ========================================================================================
float AfxScaleRatioX()
{
    HDC hDC = GetDC(HWND_DESKTOP);
    float res = (float)(GetDeviceCaps(hDC, LOGPIXELSX) / 96.0f);
    ReleaseDC(HWND_DESKTOP, hDC);
    return res;
}


//' ========================================================================================
//' Retrieves the desktop vertical scaling ratio.
//' ========================================================================================
float AfxScaleRatioY()
{
    HDC hDC = GetDC(HWND_DESKTOP);
    float res = (float)(GetDeviceCaps(hDC, LOGPIXELSY) / 96.0f);
    ReleaseDC(HWND_DESKTOP, hDC);
    return res;
}


//' ========================================================================================
//' Scales an horizontal coordinate according the DPI (dots per pixel) being used by the desktop.
//' ========================================================================================
float AfxScaleX(float cx)
{
    HDC hDC = GetDC(HWND_DESKTOP);
    float res = (float)(cx * (GetDeviceCaps(hDC, LOGPIXELSX) / 96.0f));
    ReleaseDC( HWND_DESKTOP, hDC);
    return res;
}


//' ========================================================================================
//' Scales a vertical coordinate according the DPI (dots per pixel) being used by the desktop.
//' ========================================================================================
float AfxScaleY(float cy)
{
    HDC hDC = GetDC(HWND_DESKTOP);
    float res = (float)(cy * (GetDeviceCaps(hDC, LOGPIXELSY) / 96.0f));
    ReleaseDC(HWND_DESKTOP, hDC);
    return res;
}


//' ========================================================================================
//' Unscales an horizontal coordinate according the DPI (dots per pixel) being used by the desktop.
//' ========================================================================================
float AfxUnScaleX(float cx)
{
    HDC hDC = GetDC(HWND_DESKTOP);
    float res = (float)(cx / (GetDeviceCaps(hDC, LOGPIXELSX) / 96.0f));
    ReleaseDC(HWND_DESKTOP, hDC);
    return res;
}


//' ========================================================================================
//' Unscales a vertical coordinate according the DPI (dots per pixel) being used by the desktop.
//' ========================================================================================
float AfxUnScaleY(float cy)
{
    HDC hDC = GetDC(HWND_DESKTOP);
    float res = (float)(cy / (GetDeviceCaps(hDC, LOGPIXELSY) / 96.0f));
    ReleaseDC(HWND_DESKTOP, hDC);
    return res;
}


//' ========================================================================================
//' Retrieve text from the specified window
//' ========================================================================================
std::wstring AfxGetWindowText(HWND hwnd)
{
    std::wstring wszTemp;;
    wszTemp.reserve(SendMessage(hwnd, WM_GETTEXTLENGTH, 0, 0) + 1);
    GetWindowText(hwnd, const_cast<WCHAR*>(wszTemp.c_str()), wszTemp.capacity());
    //return std::move(wszTemp);
    return wszTemp;
}


//' ========================================================================================
//' Gets the width in pixels of a window.
//' Note: To retrieve the height of the desktop window pass the handle returned by the
//' API function GetDesktopWindow.
//' ========================================================================================
int AfxGetWindowWidth(HWND hwnd)
{
    RECT rc;
    GetWindowRect(hwnd, &rc);
    return rc.right - rc.left;
}


//' ========================================================================================
//' Gets the height in pixels of a window.
//' Note: To retrieve the height of the desktop window pass the handle returned by the
//' API function GetDesktopWindow.
//' ========================================================================================
int AfxGetWindowHeight(HWND hwnd)
{
    RECT rc;
    GetWindowRect(hwnd, &rc);
    return rc.bottom - rc.top;
}


//' ========================================================================================
//' Centers a window on the screen or over another window.
//' It also ensures that the placement is done within the work area.
//' Parameters:
//' - hwnd = Handle of the window.
//' - hwndParent = [optional] Handle of the parent window.
//' ========================================================================================
void AfxCenterWindow(HWND hwnd, HWND hwndParent)
{
    RECT rc;            // Window coordinates
    RECT rcParent;      // Parent window coordinates
    RECT rcWorkArea;    // Work area coordinates
    POINT pt;           // x and y coordinates of centered window

    // Get the coordinates of the window
    GetWindowRect(hwnd, &rc);
    
    // Calculate the width and height of the window
    int nWidth = rc.right - rc.left;
    int nHeight = rc.bottom - rc.top;

    // Get the coordinates of the work area
    if (SystemParametersInfo(SPI_GETWORKAREA, sizeof(rcWorkArea), &rcWorkArea, 0) == 0) {
        rcWorkArea.right = GetSystemMetrics(SM_CXSCREEN);
        rcWorkArea.bottom = GetSystemMetrics(SM_CYSCREEN);
    }
    
    // Get the coordinates of the parent window
    if (hwndParent) {
        GetWindowRect(hwndParent, &rcParent);
    }
    else {
        rcParent.left = rcWorkArea.left;
        rcParent.top = rcWorkArea.top;
        rcParent.right = rcWorkArea.right;
        rcParent.bottom = rcWorkArea.bottom;
    }

        // Calculate the width and height of the parent window
    int nParentWidth = rcParent.right - rcParent.left;
    int nParentHeight = rcParent.bottom - rcParent.top;

    // Calculate the new x coordinate and adjust for work area
    pt.x = rcParent.left + ((nParentWidth - nWidth) / 2);
    if (pt.x < rcWorkArea.left) {
        pt.x = rcWorkArea.left;
    } else if ((pt.x + nWidth) > rcWorkArea.right) {
        pt.x = rcWorkArea.right - nWidth;
    }

    // Calculate the new y coordinate and adjust for work area
    pt.y = rcParent.top + ((nParentHeight - nHeight) / 2);
    if (pt.y < rcWorkArea.top) {
        pt.y = rcWorkArea.top;
    } else if ((pt.y + nHeight) > rcWorkArea.bottom) {
        pt.y = rcWorkArea.bottom - nHeight;
    }

    // Convert screen coordinates to client area coordinates
    if ((int)(GetWindowLongPtr(hwnd, GWL_STYLE) && WS_CHILD) == (int)WS_CHILD)
        ScreenToClient(hwndParent, &pt);

    // Reposition the window retaining its size and Z order
    SetWindowPos(hwnd, NULL, pt.x, pt.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}


//' ========================================================================================
//' Retrieves the coordinates of the work area on the primary display monitor expressed in
//' virtual screen coordinates. The work area is the portion of the screen not obscured by
//' the system taskbar or by application desktop toolbars. To get the work area of a monitor
//' other than the primary display monitor, call the GetMonitorInfo function.
//' ========================================================================================
int AfxGetWorkAreaWidth()
{
    RECT rcWrk{};
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWrk, 0);
    return (rcWrk.right - rcWrk.left);
}

//' ========================================================================================
int AfxGetWorkAreaHeight()
{
    RECT rcWrk{};
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWrk, 0);
    return (rcWrk.bottom - rcWrk.top);
}


//========================================================================================
//' Creates a tooltip for a control.
//' Parameters:
//' - hwnd      = Handle of the window or control
//' - wszText   = Tooltip text
//' - bBalloon  = Ballon tip (TRUE or FALSE)
//' - bCentered = Centered (TRUE or FALSE)
//' Return Value:
//'   The handle of the tooltip control
//' ========================================================================================
HWND AfxAddTooltip(HWND hwnd, std::wstring wszText, bool bBalloon, bool bCentered)
{
    if (IsWindow(hwnd) == 0) return 0;

    // Creates the tooltip control
    DWORD dwStyle = WS_POPUP | TTS_ALWAYSTIP;
    if (bBalloon) dwStyle = dwStyle | TTS_BALLOON;

    HWND hTooltip = CreateWindowEx(0, L"tooltips_class32", L"", dwStyle, 0, 0, 0, 0, NULL, NULL, NULL, NULL);
    if (hTooltip == NULL) return 0;

    // You must explicitly define a tooltip control as topmost. Otherwise, it might be covered by the parent window.
    SetWindowPos(hTooltip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

    // Registers the window with the tooltip control
    // 32-bit: The size of the TOOLINFOW structure is 48 bytes in
    // version 6 of comctl32.dll, and 44 bytes in lower versions.
    // 64-bit: The size of the TOOLINFOW structure is 72 bytes in
    // version 6 of comctl32.dll, and 64 bytes in lower versions.
    TOOLINFO tti{};
#ifdef _win64
    tti.cbSize = (AfxComCtlVersion() < 600) ? 64 : 72;
#else
    tti.cbSize = (AfxComCtlVersion() < 600) ? 44 : 48;
#endif

    if ((int)(GetWindowLongPtr(hwnd, GWL_STYLE) && WS_CHILD) == (int)WS_CHILD) {
        tti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
        tti.hwnd = GetParent(hwnd);
        tti.uId = (UINT_PTR)hwnd;
    }
    else {
        tti.uFlags = TTF_SUBCLASS;
        tti.hwnd = hwnd;
        tti.uId = 0;
        GetClientRect(hwnd, &tti.rect);
    }
    if (bCentered) 
        tti.uFlags = tti.uFlags | TTF_CENTERTIP;
    tti.hinst = GetModuleHandle(NULL);
    tti.lpszText = (LPWSTR)&wszText;
    SendMessage(hTooltip, TTM_ADDTOOL, 0, (LPARAM)&tti);

    return hTooltip;
}


//' ========================================================================================
//' Sets/replaces the text of a tooltip control
//' Parameters:
//' - hTooltip = Handle of the tooltip control
//' - hwnd     = Handle of the window or control
//' - wszText  = Tooltip text
//' ========================================================================================
void AfxSetTooltipText(HWND hTooltip, HWND hwnd, std::wstring wszText)
{
    if ((hTooltip == NULL) || (hwnd == NULL)) return;
    // 32-bit: The size of the TOOLINFOW structure is 48 bytes in
    // version 6 of comctl32.dll, and 44 bytes in lower versions.
    // 64-bit: The size of the TOOLINFOW structure is 72 bytes in
    // version 6 of comctl32.dll, and 64 bytes in lower versions.
    TOOLINFO tti{};
#ifdef _win64
    tti.cbSize = (AfxComCtlVersion() < 600) ? 64 : 72;
#else
    tti.cbSize = (AfxComCtlVersion() < 600) ? 44 : 48;
#endif

    if ((int)(GetWindowLongPtr(hwnd, GWL_STYLE) && WS_CHILD) == (int)WS_CHILD) {
        tti.hwnd = GetParent(hwnd);
        tti.uId = (UINT_PTR)hwnd;
    }
    else {
        tti.hwnd = hwnd;
        tti.uId = 0;
    }
    // Retrieve the tooltip information
    SendMessage(hTooltip, TTM_GETTOOLINFO, 0, (LPARAM)&tti);
    // Set the new tooltip text
    tti.lpszText = &wszText[0];
    SendMessage(hTooltip, TTM_UPDATETIPTEXT, 0, (LPARAM)&tti);
}

    
    //' ========================================================================================
//' Returns the version of specified file multiplied by 100, e.g. 601 for version 6.01.
//' Example: DIM ver AS LONG = AfxGetFileVersion("COMCTL32.DLL")
//' ========================================================================================
int AfxGetFileVersion(std::wstring wszFileName)
{
    VS_FIXEDFILEINFO* pvsffi = nullptr;
    DWORD dwHandle;
    int res = 0;

    DWORD cbLen = GetFileVersionInfoSize(wszFileName.c_str(), &dwHandle);
    if (cbLen == 0) return 0;

    HANDLE pVerInfo = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbLen);
    if (pVerInfo == NULL) return 0;
    if (GetFileVersionInfo(wszFileName.c_str(), dwHandle, cbLen, pVerInfo)) {
        if (VerQueryValue(pVerInfo, L"\\", (LPVOID*) &pvsffi, (PUINT)&cbLen)) {
            WORD wMajor = HIWORD(pvsffi->dwFileVersionMS);
            WORD wMinor = LOWORD(pvsffi->dwFileVersionMS);
            res = (wMajor + wMinor / 100) * 100;
        }
    }

    HeapFree(GetProcessHeap(), 0, pVerInfo);
    return res;
}

//' ========================================================================================
//' Returns the version of CommCtl32.dll multiplied by 100, e.g. 582 for version 5.82.
//' ========================================================================================
int AfxComCtlVersion()
{
    return AfxGetFileVersion(L"COMCTL32.DLL");
}
