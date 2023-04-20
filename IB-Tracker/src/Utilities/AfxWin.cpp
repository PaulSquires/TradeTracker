
#include "pch.h"



// ========================================================================================
// Redraws the specified window.
// Do not use it from within a WM_PAINT message.
// ========================================================================================
void AfxRedrawWindow(HWND hwnd)
{
    InvalidateRect(hwnd, NULL, true);
    UpdateWindow(hwnd);
}


// ========================================================================================
// Retrieves the desktop horizontal scaling ratio.
// ========================================================================================
float AfxScaleRatioX()
{
    HDC hDC = GetDC(HWND_DESKTOP);
    float res = (float)(GetDeviceCaps(hDC, LOGPIXELSX) / 96.0f);
    ReleaseDC(HWND_DESKTOP, hDC);
    return res;
}


// ========================================================================================
// Retrieves the desktop vertical scaling ratio.
// ========================================================================================
float AfxScaleRatioY()
{
    HDC hDC = GetDC(HWND_DESKTOP);
    float res = (float)(GetDeviceCaps(hDC, LOGPIXELSY) / 96.0f);
    ReleaseDC(HWND_DESKTOP, hDC);
    return res;
}


// ========================================================================================
// Scales an horizontal coordinate according the DPI (dots per pixel) being used by the desktop.
// ========================================================================================
int AfxScaleX(float cx)
{
    return (int)round((cx * AfxScaleRatioX()));
}


// ========================================================================================
// Scales a vertical coordinate according the DPI (dots per pixel) being used by the desktop.
// ========================================================================================
int AfxScaleY(float cy)
{
    return (int)round((cy * AfxScaleRatioX()));
}


// ========================================================================================
// Unscales an horizontal coordinate according the DPI (dots per pixel) being used by the desktop.
// ========================================================================================
int AfxUnScaleX(float cx)
{
    return (int)round((cx / AfxScaleRatioX()));
}


// ========================================================================================
// Unscales a vertical coordinate according the DPI (dots per pixel) being used by the desktop.
// ========================================================================================
int AfxUnScaleY(float cy)
{
    return (int)round((cy / AfxScaleRatioY()));
}


// ========================================================================================
// Retrieve text from the specified window
// ========================================================================================
std::wstring AfxGetWindowText(HWND hwnd)
{
    DWORD dwBufLen = SendMessage(hwnd, WM_GETTEXTLENGTH, 0, 0) + 1;
    std::wstring buffer(dwBufLen, NULL);
    GetWindowText(hwnd, (LPWSTR)buffer.c_str(), dwBufLen);
    return buffer.substr(0, dwBufLen - 1);
}


// ========================================================================================
// Set text for the specified window
// ========================================================================================
bool AfxSetWindowText(HWND hwnd, const std::wstring& wszText)
{
    return SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)wszText.c_str());
}


// ========================================================================================
// Gets the width in pixels of a window.
// Note: To retrieve the height of the desktop window pass the handle returned by the
// API function GetDesktopWindow.
// ========================================================================================
int AfxGetWindowWidth(HWND hwnd)
{
    RECT rc;
    GetWindowRect(hwnd, &rc);
    return rc.right - rc.left;
}


// ========================================================================================
// Gets the height in pixels of a window.
// Note: To retrieve the height of the desktop window pass the handle returned by the
// API function GetDesktopWindow.
// ========================================================================================
int AfxGetWindowHeight(HWND hwnd)
{
    RECT rc;
    GetWindowRect(hwnd, &rc);
    return rc.bottom - rc.top;
}


// ========================================================================================
// Centers a window on the screen or over another window.
// It also ensures that the placement is done within the work area.
// Parameters:
// - hwnd = Handle of the window.
// - hwndParent = [optional] Handle of the parent window.
// ========================================================================================
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
    }
    else if ((pt.x + nWidth) > rcWorkArea.right) {
        pt.x = rcWorkArea.right - nWidth;
    }

    // Calculate the new y coordinate and adjust for work area
    pt.y = rcParent.top + ((nParentHeight - nHeight) / 2);
    if (pt.y < rcWorkArea.top) {
        pt.y = rcWorkArea.top;
    }
    else if ((pt.y + nHeight) > rcWorkArea.bottom) {
        pt.y = rcWorkArea.bottom - nHeight;
    }

    // Convert screen coordinates to client area coordinates
    if ((int)(GetWindowLongPtr(hwnd, GWL_STYLE) && WS_CHILD) == (int)WS_CHILD)
        ScreenToClient(hwndParent, &pt);

    // Reposition the window retaining its size and Z order
    SetWindowPos(hwnd, NULL, pt.x, pt.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}


// ========================================================================================
// Retrieves the coordinates of the work area on the primary display monitor expressed in
// virtual screen coordinates. The work area is the portion of the screen not obscured by
// the system taskbar or by application desktop toolbars. To get the work area of a monitor
// other than the primary display monitor, call the GetMonitorInfo function.
// ========================================================================================
int AfxGetWorkAreaWidth()
{
    RECT rcWrk{};
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWrk, 0);
    return (rcWrk.right - rcWrk.left);
}

// ========================================================================================
int AfxGetWorkAreaHeight()
{
    RECT rcWrk{};
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWrk, 0);
    return (rcWrk.bottom - rcWrk.top);
}


// ========================================================================================
// Returns the version of specified file multiplied by 100, e.g. 601 for version 6.01.
// Example: DIM ver AS LONG = AfxGetFileVersion("COMCTL32.DLL")
// ========================================================================================
int AfxGetFileVersion(std::wstring wszFileName)
{
    VS_FIXEDFILEINFO* pvsffi = nullptr;
    DWORD dwHandle;
    int res = 0;

    DWORD cbLen = GetFileVersionInfoSize(wszFileName.c_str(), &dwHandle);
    if (cbLen == 0) return 0;

    HANDLE pVerInfo = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbLen);
    if (pVerInfo == NULL) return 0;
    if (GetFileVersionInfo(wszFileName.c_str(), 0L, cbLen, pVerInfo)) {
        if (VerQueryValue(pVerInfo, L"\\", (LPVOID*)&pvsffi, (PUINT)&cbLen)) {
            WORD wMajor = HIWORD(pvsffi->dwFileVersionMS);
            WORD wMinor = LOWORD(pvsffi->dwFileVersionMS);
            res = (wMajor + wMinor / 100) * 100;
        }
    }

    HeapFree(GetProcessHeap(), 0, pVerInfo);
    return res;
}


// ========================================================================================
// Returns the version of CommCtl32.dll multiplied by 100, e.g. 582 for version 5.82.
// ========================================================================================
int AfxComCtlVersion()
{
    return AfxGetFileVersion(L"COMCTL32.DLL");
}


//========================================================================================
// Creates a tooltip for a control.
// Parameters:
// - hwnd      = Handle of the window or control
// - wszText   = Tooltip text
// - bBalloon  = Ballon tip (TRUE or FALSE)
// - bCentered = Centered (TRUE or FALSE)
// Return Value:
//   The handle of the tooltip control
// ========================================================================================
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


// ========================================================================================
// Sets/replaces the text of a tooltip control
// Parameters:
// - hTooltip = Handle of the tooltip control
// - hwnd     = Handle of the window or control
// - wszText  = Tooltip text
// ========================================================================================
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


// ========================================================================================
// Gets a string from a list box.
// - hComboBox: A handle to the list box.
// - nIndex: The zero-based index of the item.
// ========================================================================================
std::wstring AfxGetListBoxText(HWND hListBox, int nIndex)
{
    int nLen = SendMessage(hListBox, LB_GETTEXTLEN, nIndex, 0) + 1;
    std::wstring wszText(nLen, NULL);
    SendMessage(hListBox, LB_GETTEXT, nIndex, (LPARAM)wszText.c_str());
    return wszText;
}


// ========================================================================================
// Returns the path of the program which is currently executing.
// The path name will not contain a trailing backslash.
// ========================================================================================
std::wstring AfxGetExePath()
{
    // The following retrieves the full path *and* exe name and extension.
    std::wstring buffer(MAX_PATH, NULL);
    GetModuleFileName(NULL, (LPWSTR)buffer.c_str(), MAX_PATH);

    // Remove everything after the last trailing backslash
    std::size_t found = buffer.find_last_of(L"/\\");
    return buffer.substr(0, found);
}


// ========================================================================================
// Retrieves the name of the user associated with the current thread.
// ========================================================================================
std::wstring AfxGetUserName()
{
    DWORD dwBufLen = (DWORD)MAX_PATH;
    std::wstring buffer(dwBufLen, NULL);
    GetUserName((LPWSTR)buffer.c_str(), &dwBufLen);
    return buffer.substr(0, dwBufLen - 1);
}


// ========================================================================================
// Returns the Astronomical Day for any given date.
// Parameters:
// - nDay: A number between 1-31.
// - nMonth; A number between 1-12.
// - nYear: A four digit year, e.g. 2011.
// Return Value:
// - The Astronomical Day.
// See: http://support.microsoft.com/kb/109451/en-us
// Note: Among other things, can be used to find the number of days between any two dates, e.g.:
// PRINT AfxAstroDay(1, 3, -12400) - AfxAstroDay(28, 2, -12400)  ' Prints 2
// PRINT AfxAstroDay(1, 3, 12000) - AfxAstroDay(28, 2, -12000) ' Prints 8765822
// PRINT AfxAstroDay(28, 2, 1902) - AfxAstroDay(1, 3, 1898)  ' Prints 1459 days
// ========================================================================================
int AfxAstroDay(int nDay, int nMonth, int nYear)
{
    double y = nYear + (nMonth - 2.85) / 12;
    return INT(INT(INT(367 * y) - 1.75 * INT(y) + nDay) - 0.75 * INT(0.01 * y)) + 1721119;
}


// ========================================================================================
// Returns the number of days between two ISO formatted dates.
// ========================================================================================
int AfxDaysBetween(std::wstring wszStartDate, std::wstring wszEndDate)
{
    // YYYY-MM-DD
    // 0123456789

    int nYear1 = std::stoi(wszStartDate.substr(0, 4));
    int nMonth1 = std::stoi(wszStartDate.substr(5, 2));
    int nDay1 = std::stoi(wszStartDate.substr(8, 2));

    int nYear2 = std::stoi(wszEndDate.substr(0, 4));
    int nMonth2 = std::stoi(wszEndDate.substr(5, 2));
    int nDay2 = std::stoi(wszEndDate.substr(8, 2));

    return AfxAstroDay(nDay2, nMonth2, nYear2) - AfxAstroDay(nDay1, nMonth1, nYear1);
}


// ========================================================================================
// Returns the current date in ISO format (YYYY-MM-DD)
// ========================================================================================
std::wstring AfxCurrentDate()
{
    std::wstring buffer(260, NULL);
    int bytesWritten = GetDateFormat(LOCALE_USER_DEFAULT, NULL, NULL, L"yyyy-MM-dd", (LPWSTR)buffer.c_str(), 260);
    return buffer.substr(0, bytesWritten - 1); // remove terminating null
}


// ========================================================================================
// Returns the year from a date in ISO format (YYYY-MM-DD)
// ========================================================================================
int AfxGetYear(std::wstring wszDate)
{
    // YYYY-MM-DD
    // 0123456789
    return std::stoi(wszDate.substr(0, 4));
}


// ========================================================================================
// Returns the month from a date in ISO format (YYYY-MM-DD)
// ========================================================================================
int AfxGetMonth(std::wstring wszDate)
{
    // YYYY-MM-DD
    // 0123456789
    return std::stoi(wszDate.substr(5, 2));
}


// ========================================================================================
// Returns the day from a date in ISO format (YYYY-MM-DD)
// ========================================================================================
int AfxGetDay(std::wstring wszDate)
{
    // YYYY-MM-DD
    // 0123456789
    return std::stoi(wszDate.substr(8, 2));
}


// ========================================================================================
// Returns the short format day based on the specified date in ISO format (YYYY-MM-DD)
// ========================================================================================
std::wstring AfxGetShortDayName(std::wstring wszDate)
{
    // YYYY-MM-DD
    // 0123456789

    SYSTEMTIME st;
    st.wYear = std::stoi(wszDate.substr(0, 4));
    st.wMonth = std::stoi(wszDate.substr(5, 2));
    st.wDay = std::stoi(wszDate.substr(8, 2));

    std::wstring buffer(260, NULL);
    int bytesWritten = GetDateFormat(LOCALE_USER_DEFAULT, NULL, &st, L"ddd", (LPWSTR)buffer.c_str(), 260);
    return buffer.substr(0, bytesWritten - 1); // remove terminating null
}


// ========================================================================================
// Returns the short date MMM DD from a date in ISO format (YYYY-MM-DD)
// We use this when dealing with Option expiration dates to display.
// ========================================================================================
std::wstring AfxShortDate(std::wstring wszDate)
{
    SYSTEMTIME st{};
    st.wYear = std::stoi(wszDate.substr(0, 4));
    st.wMonth = std::stoi(wszDate.substr(5, 2));
    st.wDay = std::stoi(wszDate.substr(8, 2));

    std::wstring buffer(260, NULL);
    int bytesWritten = GetDateFormat(LOCALE_USER_DEFAULT, NULL, &st, L"MMM dd", (LPWSTR)buffer.c_str(), 260);
    return buffer.substr(0, bytesWritten - 1); // remove terminating null
}


// ========================================================================================
// Check if running under Linux Wine
// ========================================================================================
bool isWineActive()
{
    bool res = false;

    //static const char* (CDECL * pwine_get_version)(void);
    HMODULE hntdll = GetModuleHandleA("ntdll.dll");
    if (!hntdll) {
        return false;
    }

    if (GetProcAddress(hntdll, "wine_get_version")) {
        res = true;
    }
    else {
        res = false;
    }
    FreeLibrary(hntdll);
    return res;
}


// ========================================================================================
// Retrieve name of font to use for GUI elements under Windows or Wine/Linux
// ========================================================================================
std::wstring AfxGetDefaultFont()
{
    // Windows 7 or Vista we'll use Tahoma. Everything above we'll use Segoe UI.
    // Wine will always use Tahoma.
    // Tahoma seems to exist on base Windows and Wine installs.
    std::wstring wszFont = L"Tahoma";

    // Use macro from versionhelpers.h
    if (IsWindows8OrGreater()) wszFont = L"Segoe UI";
    if (isWineActive()) wszFont = L"Tahoma";

    return wszFont;
}


// ========================================================================================
// Convert an wide Unicode string to ANSI string
// ========================================================================================
std::string unicode2ansi(const std::wstring& wstr)
{
    // Size, in bytes, including any terminating null character
    int size_needed = WideCharToMultiByte(CP_ACP, 0, &wstr[0], -1, NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    int bytes_written = WideCharToMultiByte(CP_ACP, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    // Remove trailing null when assigning back to the std::string
    return strTo.substr(0, bytes_written);
}


// ========================================================================================
// Convert an ANSI string to a wide Unicode String
// ========================================================================================
std::wstring ansi2unicode(const std::string& str)
{
    int size_needed = MultiByteToWideChar(CP_ACP, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_ACP, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}


// ========================================================================================
// Format a numeric (double) string with two decimal places.
// Negative values will be encloses in parenthesis.
// ========================================================================================
std::wstring AfxMoney(double value, bool UseMinusSign = false)
{
    static std::wstring DecimalSep = L".";
    static std::wstring ThousandSep = L",";

    static NUMBERFMTW num{};
    num.NumDigits = 2;
    num.LeadingZero = true;
    num.Grouping = 3;
    num.lpDecimalSep = (LPWSTR)DecimalSep.c_str();
    num.lpThousandSep = (LPWSTR)ThousandSep.c_str();

    num.NegativeOrder = 0;   // Left parenthesis, number, right parenthesis; for example, (1.1)

    if (UseMinusSign)
        num.NegativeOrder = 1;   // Negative sign, number; for example, -1.1

    std::wstring money(std::to_wstring(value));
    std::wstring buffer(256, 0);
    int j = GetNumberFormatEx(LOCALE_NAME_USER_DEFAULT, 0, money.c_str(), &num, (LPWSTR)buffer.c_str(), 256);

    money = buffer.substr(0, j - 1);

    // If value is negative then add a space after the negative parenthesis for visual purposes
    if (value < 0) {
        if (UseMinusSign == false) money.insert(1, L" ");
    }

    return money;   // -1 to remove null terminator

}


// ========================================================================================
// Gets the zero-based index of the item nearest the specified point in a list box.
// The return value contains the index of the nearest item in the LOWORD. The HIWORD is zero
// if the specified point is in the client area of the list box, or one if it is outside
// the client area.
// ========================================================================================
int Listbox_ItemFromPoint(HWND hListBox, SHORT x, SHORT y)
{
    return SendMessage(hListBox, LB_ITEMFROMPOINT, 0, (LPARAM)MAKELONG(x, y));
}


// ========================================================================================
// Compare two string or wstring for equality (case insensitive)
// ========================================================================================
inline bool caseInsCharCompareN(char a, char b) {
    return (toupper(a) == toupper(b));
}
inline bool caseInsCharCompareW(wchar_t a, wchar_t b) {
    return (towupper(a) == towupper(b));
}

bool AfxStringCompareI(const std::string& s1, const std::string& s2)
{
    return ((s1.size() == s2.size()) &&
    equal (s1.begin(), s1.end(), s2.begin(), caseInsCharCompareN));
}

bool AfxWStringCompareI(const std::wstring& s1, const std::wstring& s2) 
{
    return ((s1.size() == s2.size()) &&
    equal(s1.begin(), s1.end(), s2.begin(), caseInsCharCompareW));
}


// ========================================================================================
// Returns the current local year. The valid values are 1601 through 30827.
// ========================================================================================
int AfxLocalYear()
{
    SYSTEMTIME st;
    GetLocalTime(&st);
    return st.wYear;
}


// ========================================================================================
// Returns the current local month. The valid values are 1 thorugh 12 (1 = January, etc.).
// ========================================================================================
int AfxLocalMonth()
{
    SYSTEMTIME st;
    GetLocalTime(&st);
    return st.wMonth;
}


// ========================================================================================
// Function to split the string to words in a vector
// separated by the delimiter
// ========================================================================================
std::vector<std::wstring> AfxSplit(std::wstring str, std::wstring delimiter)
{
    std::vector<std::wstring> v;
    if (!str.empty()) {
        int start = 0;
        do {
            // Find the index of occurrence
            int idx = str.find(delimiter, start);
            if (idx == std::wstring::npos) {
                break;
            }

            // If found add the substring till that
            // occurrence in the vector
            int length = idx - start;
            v.push_back(str.substr(start, length));
            start += (length + delimiter.size());
        } while (true);
        v.push_back(str.substr(start));
    }

    return v;
}


// ========================================================================================
// Searches a directory for a file or subdirectory with a name that matches a specific name
// (or partial name if wildcards are used).
// Parameter:
// - pwszFileSpec: The directory or path, and the file name, which can include wildcard
//   characters, for example, an asterisk (*) or a question mark (?).
//   This parameter should not be NULL, an invalid string (for example, an empty string or a
//   string that is missing the terminating null character), or end in a trailing backslash (\).
//   If the string ends with a wildcard, period (.), or directory name, the user must have
//   access permissions to the root and all subdirectories on the path. To extend the limit
//   of MAX_PATH wide characters to 32,767 wide characters, prepend "\\?\" to the path.
// Return value:
//   Returns TRUE if the specified file exists or FALSE otherwise.
// Remarks:
//   Prepending the string "\\?\" does not allow access to the root directory.
//   On network shares, you can use a pwszFileSpec in the form of the following:
//   "\\server\service\*". However, you cannot use a pwszFileSpec that points to the share
//   itself; for example, "\\server\service" is not valid.
//   To examine a directory that is not a root directory, use the path to that directory,
//   without a trailing backslash. For example, an argument of "C:\Windows" returns information
//   about the directory "C:\Windows", not about a directory or file in "C:\Windows".
//   To examine the files and directories in "C:\Windows", use an pwszFileSpec of "C:\Windows\*".
//   Be aware that some other thread or process could create or delete a file with this name
//   between the time you query for the result and the time you act on the information.
//   If this is a potential concern for your application, one possible solution is to use
//   the CreateFile function with CREATE_NEW (which fails if the file exists) or OPEN_EXISTING
//   (which fails if the file does not exist).
// ========================================================================================
bool AfxFileExists(const std::wstring& wszFileSpec)
{
    WIN32_FIND_DATAW fd;
    if (wszFileSpec.c_str() == NULL) return false;

    HANDLE hFind = FindFirstFile(wszFileSpec.c_str(), &fd);
    if (hFind == INVALID_HANDLE_VALUE) return false;
    FindClose(hFind);
    return true;
}


// ========================================================================================
// Remove all leading whitespace characters from a string
// ========================================================================================
std::wstring& AfxLTrim(std::wstring& s)
{
    auto it = std::find_if(s.begin(), s.end(),
        [](wchar_t c) {
            return !std::isspace<wchar_t>(c, std::locale::classic());
        });
    s.erase(s.begin(), it);
    return s;
}


// ========================================================================================
// Remove all trailing whitespace characters from a string
// ========================================================================================
std::wstring& AfxRTrim(std::wstring& s)
{
    auto it = std::find_if(s.rbegin(), s.rend(),
        [](wchar_t c) {
            return !std::isspace<wchar_t>(c, std::locale::classic());
        });
    s.erase(it.base(), s.end());
    return s;
}


// ========================================================================================
// Remove all leading and trailing whitespace characters from a string
// ========================================================================================
std::wstring& AfxTrim(std::wstring& s) {
    return AfxLTrim(AfxRTrim(s));
}


// ========================================================================================
// Removes a style from the specified window.
// - hwnd  = Window handle
// - dwStyle = Style to remove
// Return value:
//   The previous window styles
// ========================================================================================
DWORD AfxRemoveWindowStyle(HWND hwnd, DWORD dwStyle)
{
    DWORD dwOldStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
    dwOldStyle &= ~(dwStyle);
    SetWindowLongPtr(hwnd, GWL_STYLE, dwOldStyle);
    return dwOldStyle;
}

// ========================================================================================
// Removes an extended style from the specified window.
// - hwnd  = Window handle
// - dwExStyle = Style to remove
// Return value:
//   The previous window styles
// ========================================================================================
DWORD AfxRemoveWindowExStyle(HWND hwnd, DWORD dwExStyle)
{
    DWORD dwOldExStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    dwOldExStyle &= ~(dwExStyle);
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, dwOldExStyle);
    return dwOldExStyle;
}


// ========================================================================================
// Sets the width of the specified item of a header control.
// Returns nonzero upon success, or zero otherwise.
// ========================================================================================
bool Header_SetItemWidth(HWND hwndHD, int nItem, int nWidth)
{
    HDITEM hdi{};
    hdi.mask = HDI_WIDTH;
    hdi.cxy = nWidth;
    return SendMessage(hwndHD, HDM_SETITEM, (WPARAM)nItem, (LPARAM)(HDITEMW*)&hdi);
}


// ========================================================================================
// Sets the text of the specified item. Returns TRUE or FALSE.
// ========================================================================================
bool Header_SetItemText(HWND hwndHD, int nItem, LPCWSTR pwszText)
{
    if (pwszText == nullptr) return 0;
    HDITEM hdi{};
    hdi.mask = HDI_TEXT;
    hdi.cchTextMax = lstrlenW(pwszText);
    hdi.pszText = (LPWSTR)pwszText;
    return SendMessage(hwndHD, HDM_SETITEMW, (WPARAM)nItem, (LPARAM)(HDITEMW*)&hdi);
}


// ========================================================================================
// Inserts an item into a header control. Returns the index of the new item.
// hwndHeader - handle to the header control. 
// iInsertAfter - index of the previous item. 
// nWidth - width of the new item. 
// lpsz - address of the item string. 
// ========================================================================================
bool Header_InsertNewItem(HWND hwndHD, int iInsertAfter, int nWidth, LPCWSTR pwszText, int Alignment = HDF_LEFT)
{
    HDITEM hdi{};
    hdi.mask = HDI_TEXT | HDI_FORMAT | HDI_WIDTH;
    hdi.cxy = nWidth;
    hdi.pszText = (LPWSTR)pwszText; // lpsz;
    hdi.cchTextMax = lstrlenW(pwszText);
    hdi.fmt = Alignment | HDF_STRING;

    return SendMessage(hwndHD, HDM_INSERTITEM, (WPARAM)iInsertAfter, (LPARAM)&hdi);
}

