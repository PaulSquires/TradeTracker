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

#include "pch.h"

#include "AfxWin.h"
#include "Config/Config.h"


//
// Clean a numeric string from European to American format so that regular
// library functions can operate on them.
//
std::wstring AfxClean(const std::wstring& text) {
    std::wstring temp = text;
    if (config.GetNumberFormatType() == NumberFormatType::European) {
        // convert commas to periods
        temp = AfxReplace(temp, L",", L".");
    }
    return temp;
}


//
// Create a nested folder structure if it does not already exist.
//
bool AfxCreateNestedFolder(const std::wstring& folderPath) {
    if (CreateDirectory(folderPath.c_str(), nullptr) || ERROR_ALREADY_EXISTS == GetLastError()) {
        std::wcout << L"Folder created or already exists: " << folderPath << std::endl;
        return true;
    }
    else {
        std::cerr << "Error creating folder: " << GetLastError() << std::endl;
        return false;
    }
}


//
// Retrieve the Windows system Program Files folder name
//
std::wstring AfxGetProgramFilesFolder() {
    wchar_t* knownFolderPath = nullptr;

    // Get the path of the Program Files folder
    HRESULT result = SHGetKnownFolderPath(FOLDERID_ProgramFilesX64, 0, nullptr, &knownFolderPath);

    if (SUCCEEDED(result)) {
        // Compare the provided folder path with the Program Files folder path
        std::wstring folder_name(knownFolderPath);

        // Free the allocated memory for the known folder path
        CoTaskMemFree(knownFolderPath);

        return folder_name;
    }
    else {
        std::cout << "Error getting Program Files folder path: 0x" << std::hex << result << std::endl;
        return L"";
    }
}


//
// Retrieve the Windows system Local AppData folder name
//
std::wstring AfxGetLocalAppDataFolder() {
    wchar_t* knownFolderPath = nullptr;

    // Get the path of the Program Files folder
    HRESULT result = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &knownFolderPath);

    if (SUCCEEDED(result)) {
        // Compare the provided folder path with the Program Files folder path
        std::wstring folder_name(knownFolderPath);

        // Free the allocated memory for the known folder path
        CoTaskMemFree(knownFolderPath);

        return folder_name;
    }
    else {
        std::cout << "Error getting Program Files folder path: 0x" << std::hex << result << std::endl;
        return L"";
    }
}


//
// Execute a command and get the results. (Only standard output)
//
std::wstring AfxExecCmd(const std::wstring& cmd) {
    // [in] command to execute 
    std::wstring strResult;
    HANDLE hPipeRead, hPipeWrite;

    SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES) };
    saAttr.bInheritHandle = true; // Pipe handles are inherited by child process.
    saAttr.lpSecurityDescriptor = NULL;

    // Create a pipe to get results from child's stdout.
    if (!CreatePipe(&hPipeRead, &hPipeWrite, &saAttr, 0)) {
        return strResult;
    }

    STARTUPINFOW si = { sizeof(STARTUPINFOW) };
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.hStdOutput = hPipeWrite;
    si.hStdError = hPipeWrite;
    si.wShowWindow = SW_HIDE; // Prevents cmd window from flashing.
    // Requires STARTF_USESHOWWINDOW in dwFlags.

    PROCESS_INFORMATION pi = { 0 };

    bool fSuccess = CreateProcessW(NULL, (LPWSTR)cmd.c_str(), NULL, NULL, true, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
    if (!fSuccess)
    {
        CloseHandle(hPipeWrite);
        CloseHandle(hPipeRead);
        return strResult;
    }

    bool bProcessEnded = false;
    for (; !bProcessEnded;)
    {
        // Give some timeslice (50 ms), so we won't waste 100% CPU.
        bProcessEnded = WaitForSingleObject(pi.hProcess, 50) == WAIT_OBJECT_0;

        // Even if process exited - we continue reading, if
        // there is some data available over pipe.
        for (;;)
        {
            char buf[1024];
            DWORD dwRead = 0;
            DWORD dwAvail = 0;

            if (!::PeekNamedPipe(hPipeRead, NULL, 0, NULL, &dwAvail, NULL))
                break;

            if (!dwAvail) // No data available, return
                break;

            if (!::ReadFile(hPipeRead, buf, min(sizeof(buf) - 1, dwAvail), &dwRead, NULL) || !dwRead)
                // Error, the child process might ended
                break;

            buf[dwRead] = 0;

            const size_t WCHARBUF = 1024;
            wchar_t  wszDest[WCHARBUF];
            MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, buf, -1, wszDest, WCHARBUF);
            strResult += wszDest;
        }
    }

    CloseHandle(hPipeWrite);
    CloseHandle(hPipeRead);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return strResult;
}


// ========================================================================================
// Redraws the specified window.
// Do not use it from within a WM_PAINT message.
// ========================================================================================
void AfxRedrawWindow(HWND hwnd) {
    InvalidateRect(hwnd, NULL, true);
    UpdateWindow(hwnd);
}


// ========================================================================================
// Retrieves the desktop horizontal scaling ratio.
// ========================================================================================
float AfxScaleRatioX() {
    HDC hDC = GetDC(HWND_DESKTOP);
    float res = (float)(GetDeviceCaps(hDC, LOGPIXELSX) / 96.0f);
    ReleaseDC(HWND_DESKTOP, hDC);
    return res;
}


// ========================================================================================
// Retrieves the desktop vertical scaling ratio.
// ========================================================================================
float AfxScaleRatioY() {
    HDC hDC = GetDC(HWND_DESKTOP);
    float res = (float)(GetDeviceCaps(hDC, LOGPIXELSY) / 96.0f);
    ReleaseDC(HWND_DESKTOP, hDC);
    return res;
}


// ========================================================================================
// Scales an horizontal coordinate according the DPI (dots per pixel) being used by the desktop.
// ========================================================================================
int AfxScaleX(float cx) {
    return (int)round((cx * AfxScaleRatioX()));
}
int AfxScaleX(int cx) {
    return (int)round((cx * AfxScaleRatioX()));
}
int AfxScaleX(long cx) {
    return (int)round((cx * AfxScaleRatioX()));
}


// ========================================================================================
// Scales a vertical coordinate according the DPI (dots per pixel) being used by the desktop.
// ========================================================================================
int AfxScaleY(float cy) {
    return (int)round((cy * AfxScaleRatioX()));
}
int AfxScaleY(int cy) {
    return (int)round((cy * AfxScaleRatioX()));
}
int AfxScaleY(long cy) {
    return (int)round((cy * AfxScaleRatioX()));
}


// ========================================================================================
// Unscales an horizontal coordinate according the DPI (dots per pixel) being used by the desktop.
// ========================================================================================
int AfxUnScaleX(float cx) {
    return (int)round((cx / AfxScaleRatioX()));
}
int AfxUnScaleX(int cx) {
    return (int)round((cx / AfxScaleRatioX()));
}
int AfxUnScaleX(long cx) {
    return (int)round((cx / AfxScaleRatioX()));
}


// ========================================================================================
// Unscales a vertical coordinate according the DPI (dots per pixel) being used by the desktop.
// ========================================================================================
int AfxUnScaleY(float cy) {
    return (int)round((cy / AfxScaleRatioY()));
}
int AfxUnScaleY(int cy) {
    return (int)round((cy / AfxScaleRatioY()));
}
int AfxUnScaleY(long cy) {
    return (int)round((cy / AfxScaleRatioY()));
}


// ========================================================================================
// Retrieve text from the specified window
// ========================================================================================
std::wstring AfxGetWindowText(HWND hwnd) {
    DWORD dwBufLen = GetWindowTextLength(hwnd) + 1; 
    std::wstring buffer;
    buffer.resize(dwBufLen);
    GetWindowText(hwnd, &buffer[0], dwBufLen);
    buffer.resize(dwBufLen - 1);
    return buffer;
}


// ========================================================================================
// Set text for the specified window
// ========================================================================================
bool AfxSetWindowText(HWND hwnd, const std::wstring& text) {
    return SetWindowText(hwnd, text.c_str());
}


// ========================================================================================
// Gets the client width in pixels of a window.
// ========================================================================================
int AfxGetClientWidth(HWND hwnd) {
    RECT rc{};
    GetClientRect(hwnd, &rc);
    return rc.right;
}


// ========================================================================================
// Gets the client height in pixels of a window.
// ========================================================================================
int AfxGetClientHeight(HWND hwnd) {
    RECT rc{};
    GetClientRect(hwnd, &rc);
    return rc.bottom;
}


// ========================================================================================
// Gets the width in pixels of a window.
// ========================================================================================
int AfxGetWindowWidth(HWND hwnd) {
    RECT rc{};
    GetWindowRect(hwnd, &rc);
    return rc.right - rc.left;
}


// ========================================================================================
// Gets the height in pixels of a window.
// ========================================================================================
int AfxGetWindowHeight(HWND hwnd) {
    RECT rc{};
    GetWindowRect(hwnd, &rc);
    return rc.bottom - rc.top;
}


#define MONITOR_CENTER     0x0001        // center rect to monitor 
#define MONITOR_CLIP     0x0000        // clip rect to monitor 
#define MONITOR_WORKAREA 0x0002        // use monitor work area 
#define MONITOR_AREA     0x0000        // use monitor entire area 

// 
//  ClipOrCenterRectToMonitor 
// 
//  The most common problem apps have when running on a 
//  multimonitor system is that they "clip" or "pin" windows 
//  based on the SM_CXSCREEN and SM_CYSCREEN system metrics. 
//  Because of app compatibility reasons these system metrics 
//  return the size of the primary monitor. 
// 
//  This shows how you use the multi-monitor functions 
//  to do the same thing. 
// 
void ClipOrCenterRectToMonitor(LPRECT prc, UINT flags)
{
    HMONITOR hMonitor;
    MONITORINFO mi;
    RECT        rc;
    int         w = prc->right - prc->left;
    int         h = prc->bottom - prc->top;

    // 
    // get the nearest monitor to the passed rect. 
    // 
    hMonitor = MonitorFromRect(prc, MONITOR_DEFAULTTONEAREST);

    // 
    // get the work area or entire monitor rect. 
    // 
    mi.cbSize = sizeof(mi);
    GetMonitorInfo(hMonitor, &mi);

    if (flags & MONITOR_WORKAREA)
        rc = mi.rcWork;
    else
        rc = mi.rcMonitor;

    // 
    // center or clip the passed rect to the monitor rect 
    // 
    if (flags & MONITOR_CENTER)
    {
        prc->left = rc.left + (rc.right - rc.left - w) / 2;
        prc->top = rc.top + (rc.bottom - rc.top - h) / 2;
        prc->right = prc->left + w;
        prc->bottom = prc->top + h;
    }
    else
    {
        prc->left = max(rc.left, min(rc.right - w, prc->left));
        prc->top = max(rc.top, min(rc.bottom - h, prc->top));
        prc->right = prc->left + w;
        prc->bottom = prc->top + h;
    }
}

void AfxCenterWindowMonitorAware(HWND hwnd, HWND hwndParent)
{
    // Monitor aware code to center child popup window in its parent.
    // Do not use this for centering on the desktop,
    if (hwndParent == NULL) {
        AfxCenterWindow(hwnd, hwndParent);
        return;
    }

    RECT rc;
    GetWindowRect(hwndParent, &rc);
    ClipOrCenterRectToMonitor(&rc, 0);
    int parent_left = rc.left;
    int parent_top = rc.top;
    int parent_width = (rc.right - rc.left);
    int parent_height = (rc.bottom - rc.top);

    // The values returned
    GetWindowRect(hwnd, &rc);
    int child_width = (rc.right - rc.left);
    int child_height = (rc.bottom - rc.top);
    int left = parent_left + ((parent_width - child_width) / 2);
    int top = parent_top + ((parent_height - child_height) / 2);
    SetWindowPos(hwnd, NULL, left, top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}


// ========================================================================================
// Centers a window on the screen or over another window.
// It also ensures that the placement is done within the work area.
// Parameters:
// - hwnd = Handle of the window.
// - hwndParent = [optional] Handle of the parent window.
// ========================================================================================
void AfxCenterWindow(HWND hwnd, HWND hwndParent) {
    RECT rc{};            // Window coordinates
    RECT rcParent{};      // Parent window coordinates
    RECT rcWorkArea{};    // Work area coordinates
    POINT pt{};           // x and y coordinates of centered window

    // Get the coordinates of the window
    GetWindowRect(hwnd, &rc);

    // Calculate the width and height of the window
    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;

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
    int parent_width = rcParent.right - rcParent.left;
    int parent_height = rcParent.bottom - rcParent.top;

    // Calculate the new x coordinate and adjust for work area
    pt.x = rcParent.left + ((parent_width - width) / 2);
    if (pt.x < rcWorkArea.left) {
        pt.x = rcWorkArea.left;
    }
    else if ((pt.x + width) > rcWorkArea.right) {
        pt.x = rcWorkArea.right - width;
    }

    // Calculate the new y coordinate and adjust for work area
    pt.y = rcParent.top + ((parent_height - height) / 2);
    if (pt.y < rcWorkArea.top) {
        pt.y = rcWorkArea.top;
    }
    else if ((pt.y + height) > rcWorkArea.bottom) {
        pt.y = rcWorkArea.bottom - height;
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
int AfxGetWorkAreaWidth() {
    RECT rcWrk{};
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWrk, 0);
    return (rcWrk.right - rcWrk.left);
}


// ========================================================================================
int AfxGetWorkAreaHeight() {
    RECT rcWrk{};
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWrk, 0);
    return (rcWrk.bottom - rcWrk.top);
}


// ========================================================================================
// Returns the version of specified file multiplied by 100, e.g. 601 for version 6.01.
// Example: DIM ver AS LONG = AfxGetFileVersion("COMCTL32.DLL")
// ========================================================================================
int AfxGetFileVersion(const std::wstring& file_name) {
    VS_FIXEDFILEINFO* pvsffi = nullptr;
    DWORD dwHandle = 0;
    int res = 0;

    DWORD cbLen = GetFileVersionInfoSize(file_name.c_str(), &dwHandle);
    if (cbLen == 0) return 0;

    HANDLE pVerInfo = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbLen);
    if (pVerInfo == NULL) return 0;
    if (GetFileVersionInfo(file_name.c_str(), 0L, cbLen, pVerInfo)) {
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
int AfxComCtlVersion() {
    return AfxGetFileVersion(L"COMCTL32.DLL");
}


//========================================================================================
// Creates a tooltip for a control.
// Parameters:
// - hwnd      = Handle of the window or control
// - text   = Tooltip text
// - bBalloon  = Ballon tip (true or false)
// - bCentered = Centered (true or false)
// Return Value:
//   The handle of the tooltip control
// ========================================================================================
HWND AfxAddTooltip(HWND hwnd, const std::wstring& text, bool bBalloon, bool bCentered) {
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
    tti.lpszText = (LPWSTR)text.c_str();
    SendMessage(hTooltip, TTM_ADDTOOL, 0, (LPARAM)&tti);

    return hTooltip;
}


// ========================================================================================
// Sets/replaces the text of a tooltip control
// Parameters:
// - hTooltip = Handle of the tooltip control
// - hwnd     = Handle of the window or control
// - text  = Tooltip text
// ========================================================================================
void AfxSetTooltipText(HWND hTooltip, HWND hwnd, const std::wstring& text) {
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
    tti.lpszText = (LPWSTR)text.c_str();
    SendMessage(hTooltip, TTM_UPDATETIPTEXT, 0, (LPARAM)&tti);
}


// ========================================================================================
// Gets a string from a list box.
// - hListBox: A handle to the list box.
// - index: The zero-based index of the item.
// ========================================================================================
std::wstring AfxGetListBoxText(HWND hListBox, int index) {
    DWORD dwBufLen = ListBox_GetTextLen(hListBox, index) + 1;
    std::wstring buffer;
    buffer.resize(dwBufLen);
    ListBox_GetText(hListBox, index, &buffer[0]);
    buffer.resize(dwBufLen - 1);
    return buffer;
}


// ========================================================================================
// Returns the path of the program which is currently executing.
// The path name will not contain a trailing backslash.
// ========================================================================================
std::wstring AfxGetExePath() {
    // The following retrieves the full path *and* exe name and extension.
    std::wstring buffer(MAX_PATH, NULL);
    GetModuleFileName(NULL, (LPWSTR)buffer.c_str(), MAX_PATH);

    // Remove everything after the last trailing backslash
    std::size_t found = buffer.find_last_of(L"/\\");
    return buffer.substr(0, found);
}


// ========================================================================================
// Convert wstring to integer catching any exceptions
// ========================================================================================
int AfxValInteger(const std::wstring& st)  {
    try {
        size_t pos;
        int result = std::stoi(st, &pos);

        // Check if the entire string was used for conversion
        return (pos == st.length()) ? result : 0;
    }
    catch (...) {
        return 0;
    }
}


// ========================================================================================
// Convert wstring to double catching any exceptions
// ========================================================================================
double AfxValDouble(const std::wstring& st) {
    try {
        size_t pos;
        double result = std::stod(st, &pos);

        // Check if the entire string was used for conversion
        return (pos == st.length()) ? result : 0.0f;
    }
    catch (...) {
        return 0.0f;
    }
}


// ========================================================================================
// Insert embedded hyphen "-" into a date string.
// e.g.  20230728 would be returned as 2023-07-28
// ========================================================================================
std::wstring AfxInsertDateHyphens(const std::wstring& date_string) {
    if (date_string.length() != 8) return L"";

    std::wstring new_date = date_string;
    // YYYYMMDD
    // 01234567

    new_date.insert(4, L"-");
    // YYYY-MMDD
    // 012345678

    new_date.insert(7, L"-");
    // YYYY-MM-DD
    // 0123456789

    return new_date;
}


// ========================================================================================
// Remove any embedded hyphen "-" from a date string.
// e.g.  2023-07-28 would be returned as 20230728
// ========================================================================================
std::wstring AfxRemoveDateHyphens(const std::wstring& date_string) {
    std::wstring new_date = date_string;
    new_date.erase(remove(new_date.begin(), new_date.end(), L'-'), new_date.end());
    return new_date;
}


// ========================================================================================
// Determines if a given year is a leap year or not.
// Parameters:
// - year: A four digit year, e.g. 2011.
// Return Value:
// - true or false.
// Note: A leap year is defined as all years divisible by 4, except for years divisible by
// 100 that are not also divisible by 400. Years divisible by 400 are leap years. 2000 is a
// leap year. 1900 is not a leap year.
// ========================================================================================
bool AfxIsLeapYear(int year) {
    return (year % 4 == 0) ? ((year % 100 == 0) ? ((year % 400 == 0) ? true : false) : true) : false;
}


// ========================================================================================
// Returns the number of days in the specified month.
// Parameters:
// - month: A two digit month (1-12).
// - year: A four digit year, e.g. 2011.
// Return Value:
// - The number of days.
//' ========================================================================================
int AfxDaysInMonth(int month, int year) {
    switch (month) {
    case 2:
        return AfxIsLeapYear(year) ? 29 : 28;
        break;
    case 4: case 6: case 9: case 11:
        return 30;
        break;
    case 1: case 3: case 5: case 7:
    case 8: case 10: case 12:
        return 31;
        break;
    default:
        return 0;
    }
}

// ========================================================================================
// Return weekday number of the specified date. (0 based)
// ========================================================================================
int AfxDateWeekday(int day, int month, int year) {
    static int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
    year -= month < 3;
    return (year + year / 4 - year / 100 + year / 400 + t[month - 1] + day) % 7;
}

int day(int d, int m, int y)
{
    // The following, invented by Mike Keith and published in Journal of Recreational Mathematics,
    // Vol. 22, No. 4, 1990, p. 280, is conjectured to be the shortest expression of a 
    // day-of-the-week algorithm: 

    return  (d += m < 3 ? y-- : y - 2, 23 * m / 9 + d + 4 + y / 4 - y / 100 + y / 400) % 7;
}

// ========================================================================================
// Return the days in month from an ISO specified date.
// ========================================================================================
int AfxDaysInMonthISODate(const std::wstring& date_text) {
    // YYYY-MM-DD
    // 0123456789
    if (date_text.length() != 10) return 0;
    int year = std::stoi(date_text.substr(0, 4));
    int month = std::stoi(date_text.substr(5, 2));
    return AfxDaysInMonth(month, year);
}


// ========================================================================================
// Adds the specified number of days to the incoming date and returns the new date.
// ========================================================================================
std::wstring AfxDateAddDays(const std::wstring& date_text, int num_days_to_add) {
    // YYYY-MM-DD
    // 0123456789
    if (date_text.length() != 10) return L"";

    SYSTEMTIME st = { 0 };
    FILETIME ft = { 0 };

    st.wYear = (WORD)std::stoi(date_text.substr(0, 4));
    st.wMonth = (WORD)std::stoi(date_text.substr(5, 2));
    st.wDay = (WORD)std::stoi(date_text.substr(8, 2));

    SystemTimeToFileTime(&st, &ft);

    ULARGE_INTEGER u = { 0 };
    u.LowPart = ft.dwLowDateTime;
    u.HighPart = ft.dwHighDateTime;

    // Add seconds to our large integer
    u.QuadPart += (60 * 60 * 24 * (LONGLONG)num_days_to_add) * 10000000LLU;

    ft.dwLowDateTime = u.LowPart;
    ft.dwHighDateTime = u.HighPart;

    FileTimeToSystemTime(&ft, &st);

    std::wstring buffer(260, NULL);
    int bytes_written = GetDateFormat(LOCALE_USER_DEFAULT, NULL, &st, L"yyyy-MM-dd", (LPWSTR)buffer.c_str(), 260);
    return buffer.substr(0, bytes_written - 1); // remove terminating null
}


// ========================================================================================
// Return the number of days between two dates (YYYY-MM-DD)
// ========================================================================================
int AfxDaysBetween(const std::wstring& date1, const std::wstring& date2) {
    if (date1.length() != 10) return 0;
    if (date2.length() != 10) return 0;
    static const ULONGLONG FT_SECOND = ((ULONGLONG)10000000);
    static const ULONGLONG FT_MINUTE = (60 * FT_SECOND);
    static const ULONGLONG FT_HOUR = (60 * FT_MINUTE);
    static const ULONGLONG FT_DAY = (24 * FT_HOUR);

    SYSTEMTIME st1 = { 0 };
    FILETIME ft1 = { 0 };
    st1.wYear = (WORD)std::stoi(date1.substr(0, 4));
    st1.wMonth = (WORD)std::stoi(date1.substr(5, 2));
    st1.wDay = (WORD)std::stoi(date1.substr(8, 2));

    SystemTimeToFileTime(&st1, &ft1);

    ULARGE_INTEGER u1 = { 0 };
    u1.LowPart = ft1.dwLowDateTime;
    u1.HighPart = ft1.dwHighDateTime;

    SYSTEMTIME st2 = { 0 };
    FILETIME ft2 = { 0 };
    st2.wYear = (WORD)std::stoi(date2.substr(0, 4));
    st2.wMonth = (WORD)std::stoi(date2.substr(5, 2));
    st2.wDay = (WORD)std::stoi(date2.substr(8, 2));

    SystemTimeToFileTime(&st2, &ft2);

    ULARGE_INTEGER u2 = { 0 };
    u2.LowPart = ft2.dwLowDateTime;
    u2.HighPart = ft2.dwHighDateTime;

    // Subtract the two dates.
    // FILETIME's measure the number of 100-nanosecond intervals. Convert this to days.
    ULONGLONG ullNanoDiff = 0;
    if (u2.QuadPart > u1.QuadPart) {
        ullNanoDiff = u2.QuadPart - u1.QuadPart;
        return (int)(ullNanoDiff / FT_DAY);
    }
    else {
        ullNanoDiff = u1.QuadPart - u2.QuadPart;
        return -(int)(ullNanoDiff / FT_DAY);
    }
}


// ========================================================================================
// Returns the current date in ISO format (YYYY-MM-DD)
// ========================================================================================
std::wstring AfxCurrentDate() {
    std::wstring buffer(260, NULL);
    int bytes_written = GetDateFormat(LOCALE_USER_DEFAULT, NULL, NULL, L"yyyy-MM-dd", (LPWSTR)buffer.c_str(), 260);
    return buffer.substr(0, bytes_written - 1); // remove terminating null
}


// ========================================================================================
// Returns the year from a date in ISO format (YYYY-MM-DD)
// ========================================================================================
int AfxGetYear(const std::wstring& date_text) {
    // YYYY-MM-DD
    // 0123456789
    if (date_text.length() != 10) return 0;
    return std::stoi(date_text.substr(0, 4));
}


// ========================================================================================
// Returns the month from a date in ISO format (YYYY-MM-DD)
// ========================================================================================
int AfxGetMonth(const std::wstring& date_text) {
    // YYYY-MM-DD
    // 0123456789
    if (date_text.length() != 10) return 0;
    return std::stoi(date_text.substr(5, 2));
}


// ========================================================================================
// Returns the day from a date in ISO format (YYYY-MM-DD)
// ========================================================================================
int AfxGetDay(const std::wstring& date_text) {
    // YYYY-MM-DD
    // 0123456789
    if (date_text.length() != 10) return 0;
    return std::stoi(date_text.substr(8, 2));
}


// ========================================================================================
// Returns the current local year. The valid values are 1601 through 30827.
// ========================================================================================
int AfxLocalYear() {
    SYSTEMTIME st{};
    GetLocalTime(&st);
    return st.wYear;
}


// ========================================================================================
// Returns the current local month. The valid values are 1 through 12 (1 = January, etc.).
// ========================================================================================
int AfxLocalMonth() {
    SYSTEMTIME st{};
    GetLocalTime(&st);
    return st.wMonth;
}


// ========================================================================================
// Returns the current local day. The valid values are 1 through 31.
// ========================================================================================
int AfxLocalDay() {
    SYSTEMTIME st{};
    GetLocalTime(&st);
    return st.wDay;
}


// ========================================================================================
// Returns the current day of the week.
// It is a numeric value in the range of 0-6 (representing Sunday through Saturday).
// ========================================================================================
int AfxLocalDayOfWeek() {
    SYSTEMTIME st{};
    GetLocalTime(&st);
    return st.wDayOfWeek;
}


// ========================================================================================
// Returns the UNIX (Epoch) time given the incoming ISO date (YYYY-MM-DD).
// ========================================================================================
unsigned int AfxUnixTime(const std::wstring& date_text) {
    // YYYY-MM-DD
    // 0123456789
    if (date_text.length() != 10) return 0;

    SYSTEMTIME st = { 0 };
    FILETIME ft = { 0 };

    st.wYear = (WORD)std::stoi(date_text.substr(0, 4));
    st.wMonth = (WORD)std::stoi(date_text.substr(5, 2));
    st.wDay = (WORD)std::stoi(date_text.substr(8, 2));

    SystemTimeToFileTime(&st, &ft);

    //convert FILETIME to ULARGE_INTEGER
    //then QuadPart is 64bit timestamp
    ULARGE_INTEGER ul{ ft.dwLowDateTime, ft.dwHighDateTime };
    return (unsigned int)((ul.QuadPart - 116444736000000000ULL) / 10000000);
}


// ========================================================================================
// Returns the Futures Contract date MMMDD from a date in ISO format (YYYY-MM-DD)
// This is used for display purposes on the Trade Management screen.
// ========================================================================================
std::wstring AfxFormatFuturesDate(const std::wstring& date_text) {
    if (date_text.length() == 0) return L"";
    SYSTEMTIME st{};
    st.wYear = (WORD)std::stoi(date_text.substr(0, 4));
    st.wMonth = (WORD)std::stoi(date_text.substr(5, 2));
    st.wDay = (WORD)std::stoi(date_text.substr(8, 2));

    std::wstring buffer(260, NULL);
    int bytes_written = GetDateFormat(LOCALE_USER_DEFAULT, NULL, &st, L"MMMdd", (LPWSTR)buffer.c_str(), 260);
    std::wstring text = buffer.substr(0, bytes_written - 1); // remove terminating null
    return AfxUpper(text);
}


// ========================================================================================
// Returns the Futures Contract date YYYYMM from a date in ISO format (YYYY-MM-DD)
// This is used for retrieveing market data. (uses ansi strings rather than unicode).
// ========================================================================================
std::string AfxFormatFuturesDateMarketData(const std::wstring& date_text) {
    if (date_text.length() == 0) return "";
    // Date enters as YYYY-MM-DD so we simply need to remove the hyphens    
    std::string new_date = unicode2ansi(date_text);
    new_date.erase(remove(new_date.begin(), new_date.end(), '-'), new_date.end());
    return new_date;  //YYYYMMDD
}


// ========================================================================================
// Returns the short format day based on the specified date in ISO format (YYYY-MM-DD)
// ========================================================================================
std::wstring AfxGetShortDayName(const std::wstring& date_text) {
    if (date_text.length() == 0) return L"";
    // YYYY-MM-DD
    // 0123456789

    SYSTEMTIME st{};
    st.wYear = (WORD)std::stoi(date_text.substr(0, 4));
    st.wMonth = (WORD)std::stoi(date_text.substr(5, 2));
    st.wDay = (WORD)std::stoi(date_text.substr(8, 2));

    std::wstring buffer(260, NULL);
    int bytes_written = GetDateFormat(LOCALE_USER_DEFAULT, NULL, &st, L"ddd", (LPWSTR)buffer.c_str(), 260);
    return buffer.substr(0, bytes_written - 1); // remove terminating null
}


// ========================================================================================
// Returns the long format day based on the specified date in ISO format (YYYY-MM-DD)
// ========================================================================================
std::wstring AfxGetLongDayName(const std::wstring& date_text) {
    if (date_text.length() == 0) return L"";
    // YYYY-MM-DD
    // 0123456789

    SYSTEMTIME st{};
    st.wYear = (WORD)std::stoi(date_text.substr(0, 4));
    st.wMonth = (WORD)std::stoi(date_text.substr(5, 2));
    st.wDay = (WORD)std::stoi(date_text.substr(8, 2));

    std::wstring buffer(260, NULL);
    int bytes_written = GetDateFormat(LOCALE_USER_DEFAULT, NULL, &st, L"dddd", (LPWSTR)buffer.c_str(), 260);
    return buffer.substr(0, bytes_written - 1); // remove terminating null
}


// ========================================================================================
// Returns the short format month based on the specified date in ISO format (YYYY-MM-DD)
// ========================================================================================
std::wstring AfxGetShortMonthName(const std::wstring& date_text) {
    if (date_text.length() == 0) return L"";
    // YYYY-MM-DD
    // 0123456789

    SYSTEMTIME st{};
    st.wYear = (WORD)std::stoi(date_text.substr(0, 4));
    st.wMonth = (WORD)std::stoi(date_text.substr(5, 2));
    st.wDay = (WORD)std::stoi(date_text.substr(8, 2));

    std::wstring buffer(260, NULL);
    int bytes_written = GetDateFormat(LOCALE_USER_DEFAULT, NULL, &st, L"MMM", (LPWSTR)buffer.c_str(), 260);
    return buffer.substr(0, bytes_written - 1); // remove terminating null
}


// ========================================================================================
// Returns the long format month based on the specified date in ISO format (YYYY-MM-DD)
// ========================================================================================
std::wstring AfxGetLongMonthName(const std::wstring& date_text) {
    if (date_text.length() == 0) return L"";
    // YYYY-MM-DD
    // 0123456789

    SYSTEMTIME st{};
    st.wYear = (WORD)std::stoi(date_text.substr(0, 4));
    st.wMonth = (WORD)std::stoi(date_text.substr(5, 2));
    st.wDay = (WORD)std::stoi(date_text.substr(8, 2));

    std::wstring buffer(260, NULL);
    int bytes_written = GetDateFormat(LOCALE_USER_DEFAULT, NULL, &st, L"MMMM", (LPWSTR)buffer.c_str(), 260);
    return buffer.substr(0, bytes_written - 1); // remove terminating null
}


// ========================================================================================
// Returns the short date MMM DD from a date in ISO format (YYYY-MM-DD)
// We use this when dealing with Option expiration dates to display.
// ========================================================================================
std::wstring AfxShortDate(const std::wstring& date_text) {
    if (date_text.length() == 0) return L"";

    SYSTEMTIME st{};
    st.wYear = (WORD)std::stoi(date_text.substr(0, 4));
    st.wMonth = (WORD)std::stoi(date_text.substr(5, 2));
    st.wDay = (WORD)std::stoi(date_text.substr(8, 2));

    std::wstring buffer(260, NULL);
    int bytes_written = GetDateFormat(LOCALE_USER_DEFAULT, NULL, &st, L"MMM dd", (LPWSTR)buffer.c_str(), 260);
    return buffer.substr(0, bytes_written - 1); // remove terminating null
}


// ========================================================================================
// Returns the long date MMM DD, yyyy from a date in ISO format (YYYY-MM-DD)
// ========================================================================================
std::wstring AfxLongDate(const std::wstring& date_text) {
    if (date_text.length() == 0) return L"";

    SYSTEMTIME st{};
    st.wYear = (WORD)std::stoi(date_text.substr(0, 4));
    st.wMonth = (WORD)std::stoi(date_text.substr(5, 2));
    st.wDay = (WORD)std::stoi(date_text.substr(8, 2));

    std::wstring buffer(260, NULL);
    int bytes_written = GetDateFormat(LOCALE_USER_DEFAULT, NULL, &st, L"MMM dd, yyyy", (LPWSTR)buffer.c_str(), 260);
    return buffer.substr(0, bytes_written - 1); // remove terminating null
}


// ========================================================================================
// Returns the date in ISO format (YYYY-MM-DD) based on incoming year, month, day.
// ========================================================================================
std::wstring AfxMakeISODate(int year, int month, int day) {
    SYSTEMTIME st{};
    st.wYear = (WORD)year;
    st.wMonth = (WORD)month;
    st.wDay = (WORD)day;
    std::wstring buffer(260, NULL);
    int bytes_written = GetDateFormat(LOCALE_USER_DEFAULT, NULL, &st, L"yyyy-MM-dd", (LPWSTR)buffer.c_str(), 260);
    return buffer.substr(0, bytes_written - 1); // remove terminating null
}


// ========================================================================================
// Retrieve name of font to use for GUI elements
// ========================================================================================
std::wstring AfxGetDefaultFont() {
    // Tahoma seems to exist on base Windows.
    std::wstring font_string = L"Tahoma";

    // Use macro from versionhelpers.h
    if (IsWindowsVistaOrGreater()) font_string = L"Segoe UI";
    
    // Segoe has been default Windows font since Vista
    // User should install Segoe UI font under Linux Wine if used on that platform.

    return font_string;
}


// ========================================================================================
// Convert an wide Unicode string to ANSI string
// ========================================================================================
std::string unicode2ansi(const std::wstring& wstr) {
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
std::wstring ansi2unicode(const std::string& str) {
    int size_needed = MultiByteToWideChar(CP_ACP, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_ACP, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}


// ========================================================================================
// Format a numeric (double) string with two decimal places.
// Decimal places = 2 unless specified otherwise.
// Negative values will be encloses in parenthesis.
// ========================================================================================
std::wstring AfxMoney(double value, bool use_minus_sign, int num_decimal_places) {
    static std::wstring decimal_sep = L".";
    static std::wstring thousand_sep = L",";

    static NUMBERFMTW num{};
    num.NumDigits = num_decimal_places;
    num.LeadingZero = true;
    num.Grouping = 3;

    if (config.GetNumberFormatType() == NumberFormatType::American) {
        num.lpDecimalSep = (LPWSTR)decimal_sep.c_str();
        num.lpThousandSep = (LPWSTR)thousand_sep.c_str();
    }
    if (config.GetNumberFormatType() == NumberFormatType::European) {
        num.lpDecimalSep = (LPWSTR)thousand_sep.c_str();
        num.lpThousandSep = (LPWSTR)decimal_sep.c_str(); 
    }

    num.NegativeOrder = 0;   // Left parenthesis, number, right parenthesis; for example, (1.1)

    if (use_minus_sign)
        num.NegativeOrder = 1;   // Negative sign, number; for example, -1.1

    std::wstring money(std::to_wstring(value));
    std::wstring buffer(256, 0);
    int j = GetNumberFormatEx(LOCALE_NAME_USER_DEFAULT, 0, money.c_str(), &num, (LPWSTR)buffer.c_str(), 256);

    money = buffer.substr(0, j - 1);

    // If value is negative then add a space after the negative parenthesis for visual purposes
    if (value < 0) {
        if (use_minus_sign == false) money.insert(1, L" ");
    }

    return money;
}


// ========================================================================================
// Gets the zero-based index of the item nearest the specified point in a list box.
// The return value contains the index of the nearest item in the LOWORD. The HIWORD is zero
// if the specified point is in the client area of the list box, or one if it is outside
// the client area.
// ========================================================================================
int Listbox_ItemFromPoint(HWND hListBox, SHORT x, SHORT y) {
    return (int)SendMessage(hListBox, LB_ITEMFROMPOINT, 0, (LPARAM)MAKELONG(x, y));
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

bool AfxStringCompareI(const std::string& s1, const std::string& s2) {
    return ((s1.size() == s2.size()) &&
    std::equal (s1.begin(), s1.end(), s2.begin(), caseInsCharCompareN));
}

bool AfxWStringCompareI(const std::wstring& s1, const std::wstring& s2) {
    return ((s1.size() == s2.size()) &&
    std::equal(s1.begin(), s1.end(), s2.begin(), caseInsCharCompareW));
}


// ========================================================================================
// Function to split the string to words in a vector separated by the delimiter
// ========================================================================================
std::vector<std::wstring> AfxSplit(const std::wstring& input, wchar_t delimiter) 
{
    std::wistringstream stream(input);
    std::wstring token;
    std::vector<std::wstring> result;

    while (std::getline(stream, token, delimiter)) {
        result.push_back(token);
    }

    return result;
}

//std::vector<std::wstring> AfxSplit(const std::wstring& input, wchar_t delimiter) {
//    std::wistringstream stream(input);
//    std::vector<std::wstring> result;
//
//    // Reserve space to avoid unnecessary reallocations
//    result.reserve(std::count(input.begin(), input.end(), delimiter) + 1);
//
//    for (std::wstring token; std::getline(stream, token, delimiter); ) {
//        result.push_back(std::move(token));
//    }
//
//    return result;
//}


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
//   Returns true if the specified file exists or false otherwise.
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
bool AfxFileExists(const std::wstring& wszFileSpec) {
    WIN32_FIND_DATAW fd{};
    if (wszFileSpec.c_str() == NULL) return false;

    HANDLE hFind = FindFirstFile(wszFileSpec.c_str(), &fd);
    if (hFind == INVALID_HANDLE_VALUE) return false;
    FindClose(hFind);
    return true;
}


// ========================================================================================
// Remove all leading whitespace characters from a string
// ========================================================================================
std::wstring AfxLTrim(const std::wstring& input) {
    // Find the position of the first non-whitespace character
    auto firstNonWhitespace = std::find_if_not(input.begin(), input.end(), std::iswspace);

    // Create a new string starting from the first non-whitespace character
    return std::wstring(firstNonWhitespace, input.end());
}


// ========================================================================================
// Remove all trailing whitespace characters from a string
// ========================================================================================
std::wstring AfxRTrim(const std::wstring& input) {
    if (input.empty()) {
        return input;  // Nothing to remove if the string is empty
    }

    // Find the position of the last non-whitespace character
    auto lastNonWhitespace = input.find_last_not_of(L" \t\r\n");

    if (lastNonWhitespace == std::wstring::npos) {
        // The string contains only whitespaces; return an empty string
        return L"";
    }

    // Extract the substring without trailing whitespaces
    return input.substr(0, lastNonWhitespace + 1);
}


// ========================================================================================
// Remove all leading and trailing whitespace characters from a string
// ========================================================================================
std::wstring AfxTrim(const std::wstring& input) {
    // Find the first non-whitespace character
    size_t startPos = input.find_first_not_of(L" \t\n\r");

    // If the string is all whitespace, return an empty string
    if (startPos == std::wstring::npos) {
        return L"";
    }

    // Find the last non-whitespace character
    size_t endPos = input.find_last_not_of(L" \t\n\r");

    // Extract the substring without leading and trailing whitespaces
    return input.substr(startPos, endPos - startPos + 1);
}


// ========================================================================================
// Right align string in of string of size width
// ========================================================================================
std::wstring AfxRSet(const std::wstring& text, int width) {
    if (text.length() > width) return text;
    size_t num_chars = width - text.length();
    if (num_chars <= 0) return text;
    std::wstring spaces_string(num_chars, L' ');
    return spaces_string + text;
}


// ========================================================================================
// Left align string in of string of size width
// ========================================================================================
std::wstring AfxLSet(const std::wstring& text, int width) {
    if (text.length() > width) return text;
    size_t num_chars = width - text.length();
    if (num_chars <= 0) return text;
    std::wstring spaces_string(num_chars, L' ');
    return text + spaces_string;
}


// ========================================================================================
// Replace one char/string another char/string. Return a copy.
// ========================================================================================
std::wstring AfxReplace(const std::wstring& str, const std::wstring& from, const std::wstring& to) {
    std::wstring text_string = str;
    if (str.empty()) return text_string;
    if (from.empty()) return text_string;
    size_t start_pos = 0;
    const size_t to_length = to.length();
    while ((start_pos = text_string.find(from, start_pos)) != std::wstring::npos) {
        text_string.replace(start_pos, from.length(), to);
        start_pos += to_length;     // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
    return text_string;
}


// ========================================================================================
// Remove char/string from string. Return a copy.
// ========================================================================================
std::wstring AfxRemove(const std::wstring& text, const std::wstring& repl) {
    std::wstring text_string = text;
    std::string::size_type i = text_string.find(repl);
    while (i != std::string::npos) {
        text_string.erase(i, repl.length());
        i = text_string.find(repl, i);
    }
    return text_string;
}


// ========================================================================================
// Removes a style from the specified window.
// - hwnd  = Window handle
// - dwStyle = Style to remove
// Return value:
//   The previous window styles
// ========================================================================================
DWORD AfxRemoveWindowStyle(HWND hwnd, DWORD dwStyle) {
    DWORD dwOldStyle = (DWORD)GetWindowLongPtr(hwnd, GWL_STYLE);
    DWORD dwNewStyle = dwOldStyle & ~(dwStyle);
    SetWindowLongPtr(hwnd, GWL_STYLE, dwNewStyle);
    SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER);
    InvalidateRect(hwnd, NULL, true);
    return dwOldStyle;
}


// ========================================================================================
// Adds a style from the specified window.
// - hwnd  = Window handle
// - dwStyle = Style to add
// Return value:
//   The previous window styles
// ========================================================================================
DWORD AfxAddWindowStyle(HWND hwnd, DWORD dwStyle) {
    DWORD dwOldStyle = (DWORD)GetWindowLongPtr(hwnd, GWL_STYLE);
    DWORD dwNewStyle = dwOldStyle | (dwStyle);
    SetWindowLongPtr(hwnd, GWL_STYLE, dwNewStyle);
    SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER);
    InvalidateRect(hwnd, NULL, true);
    return dwOldStyle;
}


// ========================================================================================
// Removes an extended style from the specified window.
// - hwnd  = Window handle
// - dwExStyle = Style to remove
// Return value:
//   The previous window styles
// ========================================================================================
DWORD AfxRemoveWindowExStyle(HWND hwnd, DWORD dwExStyle) {
    DWORD dwOldExStyle = (DWORD)GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    DWORD dwNewExStyle = dwOldExStyle & ~(dwExStyle);
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, dwNewExStyle);
    return dwOldExStyle;
}


// ========================================================================================
// Adds an extended  style from the specified window.
// - hwnd  = Window handle
// - dwExStyle = Style to add
// Return value:
//   The previous window EX styles
// ========================================================================================
DWORD AfxAddWindowExStyle(HWND hwnd, DWORD dwExStyle) {
    DWORD dwOldStyle = (DWORD)GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    DWORD dwNewStyle = dwOldStyle | (dwExStyle);
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, dwNewStyle);
    return dwOldStyle;
}


// ========================================================================================
// Sets the width of the specified item of a header control.
// Returns nonzero upon success, or zero otherwise.
// ========================================================================================
bool Header_SetItemWidth(HWND hwndHD, int item_index, int width) {
    HDITEM hdi{};
    hdi.mask = HDI_WIDTH;
    hdi.cxy = width;
    return SendMessage(hwndHD, HDM_SETITEM, (WPARAM)item_index, (LPARAM)(HDITEMW*)&hdi);
}


// ========================================================================================
// Gets the width of the specified item of a header control.
// Returns nonzero upon success, or zero otherwise.
// ========================================================================================
int Header_GetItemWidth(HWND hwndHD, int item_index) {
    HDITEM hdi{};
    hdi.mask = HDI_WIDTH;
    SendMessage(hwndHD, HDM_GETITEM, (WPARAM)item_index, (LPARAM)(HDITEMW*)&hdi);
    return hdi.cxy;
}


// ========================================================================================
// Sets the Header text of the specified item. Returns true or false.
// ========================================================================================
bool Header_SetItemText(HWND hwndHD, int item_index, LPCWSTR ptext) {
    if (ptext == nullptr) return 0;
    HDITEM hdi{};
    hdi.mask = HDI_TEXT;
    hdi.cchTextMax = lstrlenW(ptext);
    hdi.pszText = (LPWSTR)ptext;
    return SendMessage(hwndHD, HDM_SETITEM, (WPARAM)item_index, (LPARAM)(HDITEMW*)&hdi);
}


// ========================================================================================
// Gets the Header text of the specified item. 
// ========================================================================================
std::wstring Header_GetItemText(HWND hwndHD, int item_index) {
    std::wstring buffer(MAX_PATH, NULL);

    HDITEM hdi{};
    hdi.mask = HDI_TEXT;
    hdi.cchTextMax = MAX_PATH;
    hdi.pszText = (LPWSTR)buffer.c_str();
    SendMessage(hwndHD, HDM_GETITEM, (WPARAM)item_index, (LPARAM)(HDITEMW*)&hdi);

    return buffer.substr(0, lstrlenW(buffer.c_str()));
}


// ========================================================================================
// Gets the Header alignment of the specified item. 
// ========================================================================================
int Header_GetItemAlignment(HWND hwndHD, int item_index) {
    HDITEM hdi{};
    hdi.mask = HDI_FORMAT;
    SendMessage(hwndHD, HDM_GETITEM, (WPARAM)item_index, (LPARAM)(HDITEMW*)&hdi);

    if ((hdi.fmt & ~HDF_STRING) == HDF_LEFT) return HDF_LEFT;
    if ((hdi.fmt & ~HDF_STRING) == HDF_CENTER) return HDF_CENTER;
    if ((hdi.fmt & ~HDF_STRING) == HDF_RIGHT) return HDF_RIGHT;

    return HDF_CENTER;
}


// ========================================================================================
// Inserts an item into a header control. Returns the index of the new item.
// hwndHeader - handle to the header control. 
// iInsertAfter - index of the previous item. 
// width - width of the new item. 
// lpsz - address of the item string. 
// ========================================================================================
bool Header_InsertNewItem(HWND hwndHD, int iInsertAfter, int width, LPCWSTR ptext, int alignment) {
    HDITEM hdi{};
    hdi.mask = HDI_TEXT | HDI_FORMAT | HDI_WIDTH;
    hdi.cxy = width;
    hdi.pszText = (LPWSTR)ptext; // lpsz;
    hdi.cchTextMax = lstrlenW(ptext);
    hdi.fmt = alignment | HDF_STRING;

    return SendMessage(hwndHD, HDM_INSERTITEM, (WPARAM)iInsertAfter, (LPARAM)&hdi);
}


// ========================================================================================
// Convert a string to uppercase or lowercase. 
// ========================================================================================
std::wstring AfxUpper(const std::wstring& text) {
    // using transform() function and ::toupper in STL
    std::wstring s = text;
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
    return s;
}

std::wstring AfxLower(const std::wstring& text) {
    // using transform() function and ::tolower in STL
    std::wstring s = text;
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}


//' ========================================================================================
//  Determine if program is running under Wine in Linux
//' ========================================================================================
bool isWineActive() {
    HMODULE hntdll = GetModuleHandle(L"ntdll.dll");
    if (!hntdll) return false;
    FARPROC pwine_get_version = GetProcAddress(hntdll, "wine_get_version");
    return pwine_get_version ? true : false;
}


