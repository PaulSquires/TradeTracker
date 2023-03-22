#include "pch.h"
#include "CWindow.h"
#include <iostream>



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
    // Resolution ratio = current resolution / 96
    m_rx = (float)(GetDeviceCaps(hDC, LOGPIXELSX) / 96);
    m_ry = (float)(GetDeviceCaps(hDC, LOGPIXELSY) / 96);
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
//' Remarks: As the last parameter we are passing a pointer to the class to allow its use
//' in the WM_CREATE message, e.g.
//'    CASE WM_CREATE
//'       DIM pCreateStruct AS CREATESTRUCT PTR = CAST(CREATESTRUCT PTR, lParam)
//'       DIM pWindow AS CWindow PTR = CAST(CWindow PTR, pCreateStruct->lpCreateParams)
//'       IF pWindow THEN pWindow->AddControl("Button", hwnd, IDCANCEL, "&Close", 350, 250, 75, 23)
//' -or-
//'    CASE WM_CREATE
//'       DIM pWindow AS CWindow PTR = AfxCWindowPtr(CAST(CREATESTRUCT PTR, lParam))
//'       IF pWindow THEN pWindow->AddControl("Button", hwnd, IDCANCEL, "&Close", 350, 250, 75, 23)
//' -or-
//'    CASE WM_CREATE
//'       DIM pWindow AS CWindow PTR = AfxCWindowPtr(lParam)
//'       IF pWindow THEN pWindow->AddControl("Button", hwnd, IDCANCEL, "&Close", 350, 250, 75, 23)
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
        
    case Controls::CustomButton:  // OwnerDrawButton:
        {

        }
        break;

    case Controls::RadioButton:  // OptionButton:
        {

        }
        break;

    case Controls::CheckBox:
        {

        }
        break;

    case Controls::Check3State:
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

    case Controls::BitmapLabel:
        {

        }
        break;

    case Controls::IconLabel:
        {

        }
        break;

    case Controls::BitmapButton:
        {

        }
        break;

    case Controls::IconButton:
        {

        }
        break;

    case Controls::CustomLabel:
        {

        }
        break;

    case Controls::Frame:
        {

        }
        break;

    case Controls::GroupBox:
        {

        }
        break;
        
    case Controls::Line:
        {

        }
        break;

    case Controls::Edit:    // TextBox:
        {

        }
        break;

    case Controls::EditMultiline:   // MultilineTextBox:
        {

        }
        break;

    case Controls::ComboBox:
        {

        }
        break;

    case Controls::ComboBoxEx:
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

    case Controls::Header:
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

    case Controls::ToolBar:
        {

        }
        break;

    case Controls::Rebar:
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

    case Controls::IPAddress:
        {

        }
        break;

    case Controls::HotKey:
        {

        }
        break;

    case Controls::Animate:
        {

        }
        break;

    case Controls::Pager:
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

    case Controls::SizeBox:  // SizeGrip:
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

    case Controls::TrackBar:  // Slider:
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


    /*
    CASE "CUSTOMBUTTON", "OWNERDRAWBUTTON"
    ' Adds an ownerdraw button to the window.
    wsClassName = "Button"
    IF dwStyle = -1 THEN dwStyle = WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW
    CASE "RADIOBUTTON", "OPTION"
    ' Adds a radio button to the window.
    ' Note: In PowerBASIC this control is called "Option".
    wsClassName = "Button"
    IF dwStyle = -1 THEN dwStyle = WS_VISIBLE | WS_TABSTOP | BS_AUTORADIOBUTTON | BS_LEFT | BS_VCENTER
    IF dwStyle = WS_GROUP THEN dwStyle = WS_VISIBLE | WS_TABSTOP | BS_AUTORADIOBUTTON | BS_LEFT | BS_VCENTER | WS_GROUP
    CASE "CHECKBOX"
    ' Adds a checkbox to the window.
    wsClassName = "Button"
    IF dwStyle = -1 THEN dwStyle = WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX | BS_LEFT | BS_VCENTER
    CASE "CHECK3STATE"
    ' Adds a 3 state checkbox to the window.
    wsClassName = "Button"
    IF dwStyle = -1 THEN dwStyle = WS_VISIBLE | WS_TABSTOP | BS_AUTO3STATE | BS_LEFT | BS_VCENTER
    CASE "BITMAPLABEL"
    ' Adds an image label to the window.
    ' You must delete the bitmap before the application ends.
    wsClassName = "Static"
    IF dwStyle = -1 THEN dwStyle = WS_VISIBLE | WS_GROUP | SS_BITMAP
    IF dwExStyle = -1 THEN dwExStyle = WS_EX_TRANSPARENT
    bSetFont = FALSE
    CASE "ICONLABEL"
    ' Adds an image label to the window.
    ' You must delete the icon before the application ends.
    wsClassName = "Static"
    IF dwStyle = -1 THEN dwStyle = WS_VISIBLE | WS_GROUP | SS_ICON
    IF dwExStyle = -1 THEN dwExStyle = WS_EX_TRANSPARENT
    bSetFont = FALSE
    CASE "BITMAPBUTTON"
    ' Adds an image button to the window.
    ' You must delete the bitmap before the application ends.
    wsClassName = "Button"
    IF dwStyle = -1 THEN dwStyle = WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_BITMAP
    CASE "ICONBUTTON"
    ' Adds an image button to the window.
    ' You must delete the icon before the application ends.
    wsClassName = "Button"
    IF dwStyle = -1 THEN dwStyle = WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_ICON
    CASE "CUSTOMLABEL"
    ' Adds an ownerdraw label to the window.
    wsClassName = "Static"
    IF dwStyle = -1 THEN dwStyle = WS_VISIBLE | WS_GROUP | SS_OWNERDRAW
    bSetFont = FALSE
    CASE "FRAME", "FRAMEWINDOW"
    ' Adds a frame to the window.
    ' Note: This is not the same that PowerBASIC DDT's Frame control, that in fact is a Group Box.
    wsClassName = "Static"
    IF dwStyle = -1 THEN dwStyle = WS_VISIBLE | WS_CLIPSIBLINGS | WS_GROUP | SS_BLACKFRAME
    IF dwExStyle = -1 THEN dwExStyle = WS_EX_TRANSPARENT
    bSetFont = FALSE
    CASE "GROUPBOX"
    ' Adds a group box to the window.
    ' Note: This is the same that DDT's frame control.
    wsClassName = "Button"
    IF dwStyle = -1 THEN dwStyle = WS_VISIBLE | WS_CLIPSIBLINGS | WS_GROUP | BS_GROUPBOX
    IF dwExStyle = -1 THEN dwExStyle = WS_EX_TRANSPARENT
    CASE "LINE"
    ' Adds an horizontal line to the window
    wsClassName = "Static"
    IF dwStyle = -1 THEN dwStyle = WS_VISIBLE | SS_ETCHEDFRAME
    IF dwExStyle = -1 THEN dwExStyle = WS_EX_TRANSPARENT
    bSetFont = FALSE
    CASE "EDIT", "TEXTBOX"
    ' Adds an edit control to the window.
    wsClassName = "Edit"
    IF dwStyle = -1 THEN dwStyle = WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL
    IF dwExStyle = -1 THEN dwExStyle = WS_EX_CLIENTEDGE
    CASE "EDITMULTILINE", "MULTILINETEXTBOX"
    ' Adds an edit control to the window.
    wsClassName = "Edit"
    IF dwStyle = -1 THEN dwStyle = WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | ES_LEFT | ES_AUTOHSCROLL | ES_MULTILINE | ES_NOHIDESEL | ES_WANTRETURN
    IF dwExStyle = -1 THEN dwExStyle = WS_EX_CLIENTEDGE
    CASE "COMBOBOX"
    ' Adds a combo box to the window.
    IF dwStyle = -1 THEN dwStyle = WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER | WS_TABSTOP | CBS_DROPDOWN | CBS_HASSTRINGS | CBS_SORT
    IF dwExStyle = -1 THEN dwExStyle = WS_EX_CLIENTEDGE
    CASE "COMBOBOXEX", "COMBOBOXEX32"
    ' Adds a combo box ex to the window.
    wsClassName = "ComboBoxEx32"
    IF dwStyle = -1 THEN dwStyle = WS_VISIBLE | WS_BORDER | WS_TABSTOP | CBS_DROPDOWNLIST
    CASE "LISTBOX"
    ' Adds a list box to the window.
    IF dwStyle = -1 THEN dwStyle = WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | WS_BORDER | WS_TABSTOP | LBS_STANDARD | LBS_HASSTRINGS | LBS_SORT | LBS_NOTIFY
    IF dwExStyle = -1 THEN dwExStyle = WS_EX_CLIENTEDGE
    CASE "PROGRESSBAR", "MSCTLS_PROGRESS32"
    ' Adds a progress bar to the window.
    wsClassName = "msctls_progress32"
    IF dwStyle = -1 THEN dwStyle = WS_VISIBLE
    bSetFont = FALSE
    CASE "HEADER", "SYSHEADER32"
    ' Adds an header control to the window.
    wsClassName = "SysHeader32"
    IF dwStyle = -1 THEN dwStyle = WS_VISIBLE | CCS_TOP | HDS_HORZ | HDS_BUTTONS
    CASE "TREEVIEW", "SYSTREEVIEW32"
    ' Adds a tree view control to the window.
    wsClassName = "SysTreeView32"
    IF dwStyle = -1 THEN dwStyle = WS_VISIBLE | WS_BORDER | WS_TABSTOP | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS
    IF dwExStyle = -1 THEN dwExStyle = WS_EX_CLIENTEDGE
    CASE "LISTVIEW", "SYSLISTVIEW32"
    ' Adds a list view control to the window.
    wsClassName = "SysListView32"
    IF dwStyle = -1 THEN dwStyle = WS_VISIBLE | WS_CLIPCHILDREN | WS_TABSTOP | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS | LVS_AUTOARRANGE | LVS_EDITLABELS | LVS_ALIGNTOP
    IF dwExStyle = -1 THEN dwExStyle = WS_EX_CLIENTEDGE
    CASE "TOOLBAR", "TOOLBARWINDOW32"
    ' Adds a toolbar control to the window.
    wsClassName = "ToolbarWindow32"
    IF dwStyle = -1 THEN dwStyle = WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | CCS_TOP | WS_BORDER | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS
    CASE "REBAR", "REBARWINDOW32"
    ' Adds a rebar control to the window.
    wsClassName = "ReBarWindow32"
    IF dwStyle = -1 THEN dwStyle = WS_VISIBLE | WS_BORDER | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | CCS_NODIVIDER | RBS_VARHEIGHT | RBS_BANDBORDERS
    CASE "DATETIMEPICKER", "SYSDATETIMEPICK32"
    ' Adds a date time picker control to the window.
    wsClassName = "SysDateTimePick32"
    IF dwStyle = -1 THEN dwStyle = WS_VISIBLE | WS_TABSTOP | DTS_SHORTDATEFORMAT
    CASE "MONTHCALENDAR", "MONTHCAL", "SYSMONTHCAL32"
    ' Adds a month calendar control to the window.
    wsClassName = "SysMonthCal32"
    IF dwStyle = -1 THEN dwStyle = WS_VISIBLE | WS_TABSTOP
    IF dwExStyle = -1 THEN dwExStyle = WS_EX_CLIENTEDGE
    CASE "IPADDRESS", "SYSIPADDRESS32"
    ' Adds an IPAddress control to the window.
    wsClassName = "SysIPAddress32"
    IF dwStyle = -1 THEN dwStyle = WS_VISIBLE | WS_TABSTOP
    IF dwExStyle = -1 THEN dwExStyle = WS_EX_CLIENTEDGE
    CASE "HOTKEY", "MSCTLS_HOTKEY32"
    ' Adds an hotkey control to the window.
    wsClassName = "msctls_hotkey32"
    IF dwStyle = -1 THEN dwStyle = WS_VISIBLE | WS_TABSTOP
    IF dwExStyle = -1 THEN dwExStyle = WS_EX_CLIENTEDGE
    CASE "ANIMATE", "ANIMATION", "SYSANIMATE32"
    ' Adds an animation control to the window.
    wsClassName = "SysAnimate32"
    IF dwStyle = -1 THEN dwStyle = WS_VISIBLE | ACS_TRANSPARENT
    CASE "SYSLINK"
    ' Adds a SysLink control to the window.
    ' Note: The SysLink control is defined in the ComCtl32.dll version 6 and requires a manifest
    ' | directive that specifies that version 6 of the DLL should be used if it is available.
    IF dwStyle = -1 THEN dwStyle = WS_VISIBLE | WS_TABSTOP
    bSetFont = FALSE
    CASE "PAGER", "SYSPAGER"
    ' Adds a Pager control to the window.
    wsClassName = "SysPager"
    IF dwStyle = -1 THEN dwStyle = WS_VISIBLE | WS_TABSTOP | PGS_HORZ
    bSetFont = FALSE
    CASE "TAB", "TABCONTROL", "SYSTABCONTROL32"
    ' Adds a Tab control to the window.
    wsClassName = "SysTabControl32"
    IF dwStyle = -1 THEN dwStyle = WS_VISIBLE | WS_GROUP | WS_TABSTOP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | TCS_TABS | TCS_SINGLELINE | TCS_RAGGEDRIGHT
    IF dwExStyle = -1 THEN dwExStyle = 0
    dwExStyle = dwExStyle | WS_EX_CONTROLPARENT
    CASE "STATUSBAR", "MSCTLS_STATUSBAR32"
    ' Adds a StatusBar control to the window.
    wsClassName = "msctls_statusbar32"
    IF dwStyle = -1 THEN dwStyle = WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | CCS_BOTTOM | SBARS_SIZEGRIP
    CASE "SIZEBAR", "SIZEBOX", "SIZEGRIP"
    ' Adds a size box to the window.
    wsClassName = "Scrollbar"
    IF dwStyle = -1 THEN dwStyle = WS_VISIBLE | SBS_SIZEGRIP | SBS_SIZEBOXBOTTOMRIGHTALIGN
    bSetFont = FALSE
    nWidth = GetSystemMetrics(SM_CXVSCROLL)
    nHeight = GetSystemMetrics(SM_CYHSCROLL)
    DIM rcClient AS RECT = this.GetClientRect
    x = rcClient.Right - nWidth
    y = rcClient.Bottom - nHeight
    CASE "HSCROLLBAR"
    ' Adds an horizontal scroll bar to the window.
    wsClassName = "Scrollbar"
    IF dwStyle = -1 THEN dwStyle = WS_VISIBLE | WS_TABSTOP | SBS_HORZ
    bSetFont = FALSE
    CASE "VSCROLLBAR"
    ' Adds a vertical scroll bar to the window.
    wsClassName = "Scrollbar"
    IF dwStyle = -1 THEN dwStyle = WS_VISIBLE | WS_TABSTOP | SBS_VERT
    bSetFont = FALSE
    CASE "TRACKBAR", "MSCTLS_TRACKBAR32", "SLIDER"
    wsClassName = "msctls_trackbar32"
    IF dwStyle = -1 THEN dwStyle = WS_VISIBLE | WS_TABSTOP | TBS_AUTOTICKS | TBS_HORZ | TBS_BOTTOM | TBS_TOOLTIPS
    bSetFont = FALSE
    CASE "UPDOWN", "MSCTLS_UPDOWN32"
    wsClassName = "msctls_updown32"
    IF dwStyle = -1 THEN dwStyle = WS_VISIBLE | UDS_WRAP | UDS_ARROWKEYS | UDS_ALIGNRIGHT | UDS_SETBUDDYINT
    bSetFont = FALSE
    CASE "RICHEDIT", "RichEdit50W"
    IF dwStyle = -1 THEN dwStyle = WS_VISIBLE | WS_TABSTOP | ES_LEFT | WS_HSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_WANTRETURN | ES_NOHIDESEL | ES_SAVESEL
    IF dwExStyle = -1 THEN dwExStyle = WS_EX_CLIENTEDGE
    wsClassName = "RichEdit50W"
    m_hRichEditLib = CAST(HMODULE, LoadLibraryW("MSFTEDIT.DLL"))
    END SELECT






    SELECT CASE UCASE(wszClassName)
    CASE "LISTBOX"
    ' // Adjust the height of the control so that the integral height
    ' // is based on the new font rather than the default SYSTEM_FONT
    SetWindowPos hCtl, NULL, x, y, nWidth, nHeight, SWP_NOZORDER
    CASE "DATETIMEPICKER", "SYSDATETIMEPICK32"
    ' // Sets the font to be used by the date and time picker control's child month calendar control.
    IF m_hFont THEN SendMessageW hCtl, DTM_SETMCFONT, CAST(WPARAM, m_hFont), CTRUE
    CASE "PROGRESSBAR", "MSCTLS_PROGRESS32"
    ' // Set the default range
    .SendMessageW hCtl, PBM_SETRANGE32, 0, 100
    ' // Set the default initial value
    .SendMessageW hCtl, PBM_SETPOS, 0, 0
    CASE "TRACKBAR", "MSCTLS_TRACKBAR32"
    ' // Set the default range values
    .SendMessageW hCtl, TBM_SETRANGEMIN, CTRUE, 0
    .SendMessageW hCtl, TBM_SETRANGEMAX, CTRUE, 100
    ' // Set the default page size
    .SendMessageW hCtl, TBM_SETPAGESIZE, 0, 10
    CASE "UPDOWN", "MSCTLS_UPDOWN32"
    ' // Set the default base
    .SendMessageW hCtl, UDM_SETBASE, 10, 0
    ' // Set the default range values
    .SendMessageW hCtl, UDM_SETRANGE32, 100, 0
    ' // Set the default initial value
    .SendMessageW hCtl, UDM_SETPOS32, 0, 0
    ' // Correct for Windows using a default size for the updown control
    this.SetWindowPos hCtl, NULL, x, y, nWidth, nHeight, SWP_NOZORDER
    CASE "HSCROLLBAR", "VSCROLLBAR"
    ' // Initialize the scroll bar with default values
    DIM tsi AS SCROLLINFO
    tsi.cbSize = SIZEOF(tsi)
    tsi.fMask = SIF_PAGE | SIF_POS | SIF_RANGE
    tsi.nMin = 0
    tsi.nMax = 100
    tsi.nPage = 0
    tsi.nPos = 0
    .SetScrollInfo hCtl, SB_CTL, @tsi, CTRUE
    CASE "TOOLBAR", "TOOLBARWINDOW32"
    ' // Set the button size
    DIM AS LONG nButtonWidth, nButtonHeight
    nButtonWidth = LOWORD(.SendMessageW(hCtl, TB_GETBUTTONSIZE, 0, 0)) * m_rx
    nButtonHeight = HIWORD(.SendMessageW(hCtl, TB_GETBUTTONSIZE, 0, 0)) * m_ry
    .SendMessageW hCtl, TB_SETBUTTONSIZE, 0, MAKELONG(nButtonWidth, nButtonHeight)
    ' // Send this message for backward compatibility
    .SendMessageW hCtl, TB_BUTTONSTRUCTSIZE, SIZEOF(TBBUTTON), 0
    CASE "BITMAPLABEL"
    ' // Loads the image
    DIM hImage AS HANDLE, wID AS WORD, dwID AS DWORD
    IF LEFT(wszTitle, 1) = "#" THEN
    wID = VAL(MID(wszTitle, 2))
    dwID = MAKELONG(wID, 0)
    hImage = .LoadImageW(m_hInstance, CAST(LPCWSTR, CAST(ULONG_PTR, dwID)), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR)
    ELSE
    hImage = .LoadImageW(m_hInstance, wszTitle, IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR)
    END IF
    IF hImage THEN.SendMessageW(hCtl, STM_SETIMAGE, IMAGE_BITMAP, CAST(LPARAM, hImage))
    CASE "ICONLABEL"
    ' // Loads the image
    DIM hImage AS HANDLE, wID AS WORD, dwID AS DWORD
    IF LEFT(wszTitle, 1) = "#" THEN
    wID = VAL(MID(wszTitle, 2))
    dwID = MAKELONG(wID, 0)
    hImage = .LoadImageW(m_hInstance, CAST(LPCWSTR, CAST(ULONG_PTR, dwID)), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR)
    ELSE
    hImage = .LoadImageW(m_hInstance, wszTitle, IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR)
    END IF
    IF hImage THEN.SendMessageW(hCtl, STM_SETIMAGE, IMAGE_ICON, CAST(LPARAM, hImage))
    CASE "BITMAPBUTTON"
    ' // Loads the image
    DIM hImage AS HANDLE, wID AS WORD, dwID AS DWORD
    IF LEFT(wszTitle, 1) = "#" THEN
    wID = VAL(MID(wszTitle, 2))
    dwID = MAKELONG(wID, 0)
    hImage = .LoadImageW(m_hInstance, CAST(LPCWSTR, CAST(ULONG_PTR, dwID)), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR)
    ELSE
    hImage = .LoadImageW(m_hInstance, wszTitle, IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR)
    END IF
    IF hImage THEN.SendMessageW(hCtl, BM_SETIMAGE, IMAGE_BITMAP, CAST(LPARAM, hImage))
    CASE "ICONBUTTON"
    ' // Loads the image
    DIM hImage AS HANDLE, wID AS WORD, dwID AS DWORD
    IF LEFT(wszTitle, 1) = "#" THEN
    wID = VAL(MID(wszTitle, 2))
    dwID = MAKELONG(wID, 0)
    hImage = .LoadImageW(m_hInstance, CAST(LPCWSTR, CAST(ULONG_PTR, dwID)), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR)
    ELSE
    hImage = .LoadImageW(m_hInstance, wszTitle, IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR)
    END IF
    IF hImage THEN.SendMessageW(hCtl, BM_SETIMAGE, IMAGE_ICON, CAST(LPARAM, hImage))
    END SELECT
    FUNCTION = hCtl
    END FUNCTION
    */
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
//' Simple debug message output to console
//' ========================================================================================
void dp(std::wstring msg)
{
    std::wcout << msg << std::endl;
}

void dp(std::string msg)
{
    std::cout << msg << std::endl;
}

void dp(int msg)
{
    std::cout << msg << std::endl;
}

void dp(HWND msg)
{
    std::cout << msg << std::endl;
}


    /*

' =====================================================================================
' Retrieves the size of the work area on the primary display monitor. The work area is the
' portion of the screen not obscured by the system taskbar | by application desktop toolbars.
' =====================================================================================
PRIVATE SUB CWindow.GetWorkArea OVERLOAD(BYVAL lpRect AS LPRECT)
IF lpRect = NULL THEN EXIT SUB
SystemParametersInfoW(SPI_GETWORKAREA, 0, lpRect, 0)
' // Divide by m_rx and m_ry to make the result High DPI aware
lpRect->Left /= m_rx
lpRect->Right /= m_rx
lpRect->Top /= m_ry
lpRect->Bottom /= m_ry
END SUB
' =====================================================================================
' =====================================================================================
PRIVATE FUNCTION CWindow.GetWorkArea OVERLOAD() AS RECT
DIM rc AS RECT
SystemParametersInfoW(SPI_GETWORKAREA, 0, @rc, 0)
' // Divide by m_rx and m_ry to make the result High DPI aware
rc.Left /= m_rx
rc.Right /= m_rx
rc.Top /= m_ry
rc.Bottom /= m_ry
FUNCTION = rc
END FUNCTION
' =====================================================================================

' =====================================================================================
    ' Gets/Sets the accelerator table handle
    ' =====================================================================================
    PRIVATE PROPERTY CWindow.AccelHandle() AS HACCEL
    PROPERTY = m_hAccel
    END PROPERTY
    ' =====================================================================================
    ' =====================================================================================
    PRIVATE PROPERTY CWindow.AccelHandle(BYVAL hAccel AS HACCEL)
    IF m_hAccel THEN.DestroyAcceleratorTable(m_hAccel)
    IF UBOUND(m_rgAccelEntries) - LBOUND(m_rgAccelEntries) > -1 THEN ERASE m_rgAccelEntries
    m_hAccel = hAccel
    END PROPERTY
    ' =====================================================================================
    ' =====================================================================================
    ' Adds an accelerator key to the table.
    ' =====================================================================================
    PRIVATE SUB CWindow.AddAccelerator OVERLOAD(BYVAL fvirt AS UBYTE, BYVAL wKey AS WORD, BYVAL cmd AS WORD)
    REDIM PRESERVE m_rgAccelEntries(UBOUND(m_rgAccelEntries) + 1) AS ACCEL
    m_rgAccelEntries(UBOUND(m_rgAccelEntries)).fvirt = fvirt
    m_rgAccelEntries(UBOUND(m_rgAccelEntries)).key = wKey
    m_rgAccelEntries(UBOUND(m_rgAccelEntries)).cmd = cmd
    END SUB
    ' =====================================================================================
    ' =====================================================================================
    PRIVATE SUB CWindow.AddAccelerator OVERLOAD(BYVAL fvirt AS UBYTE, BYREF wszKey AS WSTRING, BYVAL cmd AS WORD)
    REDIM PRESERVE m_rgAccelEntries(UBOUND(m_rgAccelEntries) + 1) AS ACCEL
    m_rgAccelEntries(UBOUND(m_rgAccelEntries)).fvirt = fvirt
    m_rgAccelEntries(UBOUND(m_rgAccelEntries)).key = ASC(wszKey)
    m_rgAccelEntries(UBOUND(m_rgAccelEntries)).cmd = cmd
    END SUB
    ' =====================================================================================
    ' =====================================================================================
    ' Creates the accelerator table.
    ' =====================================================================================
    PRIVATE FUNCTION CWindow.CreateAcceleratorTable() AS HACCEL
    IF UBOUND(m_rgAccelEntries) - LBOUND(m_rgAccelEntries) = -1 THEN EXIT FUNCTION
    IF m_hAccel THEN.DestroyAcceleratorTable(m_hAccel)
    m_hAccel = .CreateAcceleratorTableW(CAST(LPACCEL, @m_rgAccelEntries(0)), UBOUND(m_rgAccelEntries) - LBOUND(m_rgAccelEntries) + 1)
    FUNCTION = m_hAccel
    END FUNCTION
    ' =====================================================================================
    ' =====================================================================================
    ' Destroys the accelerator table.
    ' =====================================================================================
    PRIVATE SUB CWindow.DestroyAcceleratorTable
    IF m_hAccel THEN.DestroyAcceleratorTable(m_hAccel)
    IF UBOUND(m_rgAccelEntries) - LBOUND(m_rgAccelEntries) > -1 THEN ERASE m_rgAccelEntries
    m_hAccel = NULL
    END SUB
    ' =====================================================================================


    ' ########################################################################################
    '                                *** HELPER FUNCTIONS ***
    ' ########################################################################################

    ' ========================================================================================
    ' Returns a pointer to the CWindow class given the handle of its associated window handle.
    ' To retrieve it from the handle of any of its child windows | controls, use AfxCWindowOwnerPtr.
    ' ========================================================================================
    PRIVATE FUNCTION AfxCWindowPtr OVERLOAD(BYVAL hwnd AS HWND) AS CWindow PTR
    FUNCTION = CAST(CWindow PTR, .GetWindowLongPtrW(hwnd, 0))
    END FUNCTION
    ' ========================================================================================

    ' ========================================================================================
    ' Returns a pointer to the CWindow class given the handle of the window created with it
    ' | the handle of any of it's children.
    ' ========================================================================================
    PRIVATE FUNCTION AfxCWindowOwnerPtr OVERLOAD(BYVAL hwnd AS HWND) AS CWindow PTR
    IF hwnd = NULL THEN EXIT FUNCTION
    DIM hRootOwner AS.HWND = .GetAncestor(hwnd, GA_ROOTOWNER)
    IF hRootOwner = NULL THEN EXIT FUNCTION
    FUNCTION = CAST(CWindow PTR, .GetWindowLongPtrW(hRootOwner, 0))
    END FUNCTION
    ' ========================================================================================

*/
