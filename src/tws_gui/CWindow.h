#pragma once

#include "framework.h"
#include <string>

using namespace std::string_literals; // enables 'L' macro


enum class Controls
{
    None = 0,
    Button = 1,
    CustomButton, OwnerDrawButton = 2,
    RadioButton, OptionButton = 3,
    CheckBox = 4,
    Check3State = 5,
    Label = 6,
    BitmapLabel = 7,
    IconLabel = 8,
    BitmapButton = 9,
    IconButton = 10,
    CustomLabel = 11,
    Frame = 12,
    GroupBox = 13,
    Line = 14,
    Edit, TextBox = 15,
    EditMultiline, MultilineTextBox = 16,
    ComboBox = 17,
    ComboBoxEx = 18,
    ListBox = 19,
    ProgressBar = 20,
    Header = 21,
    TreeView = 22,
    ListView = 23,
    ToolBar = 24,
    Rebar = 25,
    DateTimePicker = 26,
    MonthCalendar = 27,
    IPAddress = 28,
    HotKey = 29,
    Animate = 30,
    Pager = 31,
    TabControl = 32,
    StatusBar = 33,
    SizeBox, SizeGrip = 34,
    HScrollBar = 35,
    VScrollBar = 36,
    TrackBar, Slider = 37,
    UpDown = 38,
    RichEdit = 39
};



//' ========================================================================================
//' CWindow class
//' ========================================================================================
class CWindow
{

private:
    HWND            m_hwnd = NULL;              // Window handle
    HINSTANCE       m_hInstance = NULL;         // Instance handle
    HFONT           m_hFont = NULL;             // Default font handle
    HACCEL          m_hAccel = NULL;            // Accelerator table handle
    int             m_DPI = 96;                 // Design-time DPI
    float           m_rx = 1;                   // Horizontal scaling ratio
    float           m_ry = 1;                   // Vertical scaling ratio
    ATOM            m_wAtom;                    // Class atom
    std::wstring    m_wszClassName;             // Class name
    int             m_DefaultFontSize = 0;      // Default font size
    std::wstring    m_wszDefaultFontName;       // Default font name
//DIM m_rgAccelEntries(ANY) AS ACCEL
    HMODULE     m_hRichEditLib = NULL;                 // Rich Edit moudle handle

public:
    CWindow();
    virtual ~CWindow();

    HWND Create(
        HWND hParent = NULL,
        std::wstring wszTitle = L"",
        WNDPROC lpfnWndProc = nullptr,
        int x = CW_USEDEFAULT,
        int y = CW_USEDEFAULT,
        int nWidth = CW_USEDEFAULT,
        int nHeight = CW_USEDEFAULT,
        DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        DWORD dwExStyle = WS_EX_CONTROLPARENT | WS_EX_WINDOWEDGE);
        
    HFONT CreateFont(
        std::wstring wszFaceName,             // __in Typeface name of font
        int lPointSize,                       // __in Point size
        int lWeight = 0,                      // __in Font weight(bold etc.)
        BYTE bItalic = FALSE,                // __in CTRUE = italic
        BYTE bUnderline = FALSE,             // __in CTRUE = underline
        BYTE bStrikeOut = FALSE,             // __in CTRUE = strikeout
        BYTE bCharSet = DEFAULT_CHARSET);    // __in character set
        
    bool SetFont(
        std::wstring wszFaceName,
        int lPointSize,
        int lWeight = 0,
        BYTE bItalic = FALSE,
        BYTE bUnderline = FALSE,
        BYTE bStrikeOut = FALSE,
        BYTE bCharSet = DEFAULT_CHARSET);

    HWND CreateControl(                        // INTERNAL called by AddControl() 
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
        LONG_PTR lpParam);                     // Pointer to custom data
        
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
        DWORD_PTR dwRefData = NULL);                  // Pointer to reference data

    WPARAM DoEvents(int nCmdShow = 0);

    const HWND& hWindow() const { return m_hwnd; }
    void hWindow(const HWND& hwnd) { if(m_hwnd == NULL) m_hwnd = hwnd; } 

    const HINSTANCE& hInst() const { return m_hInstance; }
    void hInst(const HINSTANCE& hInstance) { if (m_hInstance == NULL) m_hInstance = hInstance; }

    const float& rxRatio() const { return m_rx; }
    void rxRatio(const float& rx) { m_rx = rx; }

    const float& ryRatio() const { return m_ry; }
    void ryRatio(const float& ry) { m_ry = ry; }

    const float& ScaleX(const float& cx) const { return cx * m_rx; }
    const float& ScaleY(const float& cy) const { return cy * m_ry; }

    const float& UnScaleX(const float& cx) const { return cx / m_rx; }
    const float& UnScaleY(const float& cy) const { return cy / m_ry; }


/*
    DECLARE SUB GetWorkArea OVERLOAD(BYVAL lpRect AS LPRECT)
    DECLARE FUNCTION GetWorkArea OVERLOAD() AS RECT
    DECLARE FUNCTION AddControl(BYREF wszClassName AS WSTRING, BYVAL hParent AS HWND = NULL, BYVAL cID AS LONG_PTR = 0, _
        BYREF wszTitle AS WSTRING = "", BYVAL x AS LONG = 0, BYVAL y AS LONG = 0, BYVAL nWidth AS LONG = 0, _
        BYVAL nHeight AS LONG = 0, BYVAL dwStyle AS LONG = -1, BYVAL dwExStyle AS LONG = -1, BYVAL lpParam AS LONG_PTR = 0, _
        BYVAL pWndProc AS SUBCLASSPROC = NULL, BYVAL uIdSubclass AS UINT_PTR = &HFFFFFFFF, BYVAL dwRefData AS DWORD_PTR = NULL) AS HWND
    DECLARE PROPERTY AccelHandle() AS HACCEL
    DECLARE PROPERTY AccelHandle(BYVAL hAccel AS HACCEL)
    DECLARE SUB AddAccelerator OVERLOAD(BYVAL fvirt AS UBYTE, BYVAL wKey AS WORD, BYVAL cmd AS WORD)
    DECLARE SUB AddAccelerator OVERLOAD(BYVAL fvirt AS UBYTE, BYREF wszKey AS WSTRING, BYVAL cmd AS WORD)
    DECLARE FUNCTION CreateAcceleratorTable() AS HACCEL
    DECLARE SUB DestroyAcceleratorTable()
    DECLARE PROPERTY UserData(BYVAL idx AS LONG) AS LONG_PTR
    DECLARE PROPERTY UserData(BYVAL idx AS LONG, BYVAL newValue AS LONG_PTR)
*/

};


//' Helper functions
CWindow* AfxCWindowPtr(HWND hwnd);
CWindow* AfxCWindowOwnerPtr(HWND hwnd);

void dp(std::wstring msg);
void dp(std::string msg);
void dp(int msg);
void dp(HWND msg);




