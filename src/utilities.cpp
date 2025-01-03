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

#include <string>
#include <vector>

#include <chrono>
#include <array>

#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <locale>
#include <iomanip>
#include <stdio.h>
#include <filesystem>

#include "appstate.h"

#include "utilities.h"


using namespace std::chrono;
namespace fs = std::filesystem;


#if defined(_WIN32) // win32 and win64
std::wstring ansi2unicode(const std::string& str) {
    int size_needed = MultiByteToWideChar(CP_ACP, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_ACP, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
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


//
// Retrieve the Windows system Program Files folder name
//
std::string AfxGetProgramFilesFolder() {
    wchar_t* knownFolderPath = nullptr;

    // Get the path of the Program Files folder
    HRESULT result = SHGetKnownFolderPath(FOLDERID_ProgramFilesX64, 0, nullptr, &knownFolderPath);

    if (SUCCEEDED(result)) {
        // Compare the provided folder path with the Program Files folder path
        std::string folder_name = unicode2ansi(knownFolderPath);

        // Free the allocated memory for the known folder path
        CoTaskMemFree(knownFolderPath);

        return folder_name;
    }
    else {
        std::cout << "Error getting Program Files folder path: 0x" << std::hex << result << std::endl;
        return "";
    }
}


//
// Retrieve the Windows system Local AppData folder name
//
std::string AfxGetLocalAppDataFolder() {
    wchar_t* knownFolderPath = nullptr;

    // Get the path of the Program Files folder
    HRESULT result = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &knownFolderPath);

    if (SUCCEEDED(result)) {
        // Compare the provided folder path with the Program Files folder path
        std::string folder_name = unicode2ansi(knownFolderPath);

        // Free the allocated memory for the known folder path
        CoTaskMemFree(knownFolderPath);

        return folder_name;
    }
    else {
        std::cout << "Error getting Program Files folder path: 0x" << std::hex << result << std::endl;
        return "";
    }
}


//
// Execute a command and get the results. (Only standard output)
//
std::string AfxExecCmd(const std::string& cmd) {
    // [in] command to execute
    std::string strResult;
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

    std::wstring wcmd = ansi2unicode(cmd);
    bool fSuccess = CreateProcessW(NULL, (LPWSTR)wcmd.c_str(), NULL, NULL, true, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
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

            // const size_t WCHARBUF = 1024;
            // wchar_t  wszDest[WCHARBUF];
            // MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, buf, -1, wszDest, WCHARBUF);
            // strResult += wszDest;
            strResult += std::string(buf);
        }
    }

    CloseHandle(hPipeWrite);
    CloseHandle(hPipeRead);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return strResult;
}
#endif


//
// Create a nested folder structure if it does not already exist.
//
bool AfxCreateNestedFolder(const std::string& folder_path) {
    try {
        // Check if the directory already exists
        if (!fs::exists(folder_path)) {
            // Create the directory
            if (fs::create_directory(folder_path)) {
                // std::cout << "Directory created successfully: " << folder_path << std::endl;
                return true;
            } else {
                std::cout << "Failed to create directory: " << folder_path << std::endl;
                return false;
            }
        } else {
            //std::cout << "Directory already exists: " << folder_path << std::endl;
            return false;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
}


std::string GetDataFilesFolder() {
    std::string exe_folder = AfxGetExePath();  // no trailing backslash
    exe_folder = AfxReplace(exe_folder, "\\", "/");
    std::string data_files_folder = exe_folder;

    // default data files to same location as EXE. If the database exists in the
    // current exe folder then automatically use this folder (Portable version).
    std::string dbFilename = data_files_folder + "/tt-database.db";
    if (AfxFileExists(dbFilename)) return data_files_folder;

#if defined(_WIN32) // win32 and win64
    std::string program_files_folder = AfxGetProgramFilesFolder();   // no trailing backslash
    program_files_folder = AfxReplace(program_files_folder, "\\", "/");
    if (exe_folder.substr(0, program_files_folder.length()) == program_files_folder) {
        // Program is installed in the Program Files folder so we need to
        // set the data folder to the AppData Local folder.
        data_files_folder = AfxGetLocalAppDataFolder() + "/TradeTracker";
        data_files_folder = AfxReplace(data_files_folder, "\\", "/");
        // Create the data files folder in Local App Data should it not exist.
        AfxCreateNestedFolder(data_files_folder);
    }

#elif defined(__APPLE__)
    // Construct the path to Application Support
    std::string home = std::getenv("HOME");
    data_files_folder = home + "/Library/Application Support/TradeTracker";
    AfxCreateNestedFolder(data_files_folder);

#else
    // Construct the path to Application Support
    std::string home = std::getenv("HOME");
    data_files_folder = home + "/.config/tradetracker";
    AfxCreateNestedFolder(data_files_folder);
#endif

    // no trailing backslash
    return data_files_folder;
}


// ========================================================================================
// Save a string to a text file (returns True if error)
// ========================================================================================
bool AfxSaveStringToFile(const std::string& filename, const std::string& data) {
    std::ofstream file(filename);

    if (!file.is_open()) {
        std::cout << "Could not open file " + filename << std::endl;
        return true;
    }

    file << data;

    if (!file.good()) {
        std::cout << "Error occurred while writing to file " + filename << std::endl;
        return true;
    }

    return false;
}


// ========================================================================================
// Load a text file into a string
// ========================================================================================
std::string AfxLoadFileIntoString(const std::string& filename) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cout <<"Could not open file " + filename << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}


// ========================================================================================
// Convert YYYY-MM-DD string to year_month_day object
// ========================================================================================
std::chrono::year_month_day from_iso_string(const std::string& iso_string) {
    int y;
    unsigned int m, d;
    char delim1, delim2;
    std::istringstream iss(iso_string);

    iss >> y >> delim1 >> m >> delim2 >> d && delim1 == '-' && delim2 == '-';
    return year_month_day{year{y}, month{m}, day{d}};
}


// ========================================================================================
// Convert year_month_day object to YYYY-MM-DD string
// ========================================================================================
std::string to_iso_string(const year_month_day& ymd) {
    // return std::format("{:04}-{:02}-{:02}", static_cast<int>(ymd.year()),
    //                    static_cast<unsigned>(ymd.month()),
    //                    static_cast<unsigned>(ymd.day()));

    // Apple maxOS clang does not support std::format so use a workaround
    std::stringstream ss;
    ss << static_cast<int>(ymd.year()) << '-'
       << std::setfill('0') << std::setw(2) << static_cast<unsigned>(ymd.month()) << '-'  // Ensure 2 digits with leading zeros
       << std::setfill('0') << std::setw(2) << static_cast<unsigned>(ymd.day());          // Ensure 2 digits with leading zeros
    return ss.str();
}


// ========================================================================================
// Convert string to lowercase and return copy
// ========================================================================================
std::string AfxLower(const std::string& text) {
    std::string s = text;
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return s;
}


// ========================================================================================
// Convert string to uppercase and return copy
// ========================================================================================
std::string AfxUpper(const std::string& text) {
    std::string s = text;
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) { return std::toupper(c); });
    return s;
}


// ========================================================================================
// Remove all leading whitespace characters from a string
// ========================================================================================
std::string AfxLTrim(const std::string& input) {
    std::string s = input;
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    return s;
}


// ========================================================================================
// Remove all trailing whitespace characters from a string
// ========================================================================================
std::string AfxRTrim(const std::string& input) {
    std::string s = input;
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
    return s;
}


// ========================================================================================
// Remove all leading and trailing whitespace characters from a string
// ========================================================================================
std::string AfxTrim(const std::string& input) {
    return AfxLTrim(AfxRTrim(input));
}


// ========================================================================================
// Function to split the string to words in a vector separated by the delimiter
// ========================================================================================
std::vector<std::string> AfxSplit(const std::string& input, char delimiter) {
    std::istringstream stream(input);
    std::string token;
    std::vector<std::string> result;
    result.reserve(8);

    while (std::getline(stream, token, delimiter)) {
        result.emplace_back(token);
    }
    return result;
}


// ========================================================================================
// Convert string to integer catching any exceptions
// ========================================================================================
int AfxValInteger(const std::string& st)  {
    if (st == "") return 0;
    try {
        size_t pos;
        int result = std::stoi(st, &pos);

        // Check if the entire string was used for conversion
        return (pos == st.length()) ? result : 0;
    }
    catch (...) {
        std::cout << "Error: AfxValInteger " << st << std::endl;
        return 0;
    }
}


// ========================================================================================
// Convert string to double catching any exceptions
// ========================================================================================
double AfxValDouble(const std::string& st) {
    if (st == "") return 0.0f;
    try {
        size_t pos;
        double result = std::stod(st, &pos);

        // Check if the entire string was used for conversion
        return (pos == st.length()) ? result : 0.0f;
    }
    catch (...) {
        std::cout << "Error: AfxValDouble " << st << std::endl;
        return 0.0f;
    }
}


// ========================================================================================
// Convert double to string with number of decinal places
// ========================================================================================
std::string AfxDoubleToString(const double value, const int num_decimal_places) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(num_decimal_places) << value;
    return ss.str();
}


// ========================================================================================
// Insert embedded hyphen "-" into a date string.
// e.g.  20230728 would be returned as 2023-07-28
// ========================================================================================
std::string AfxInsertDateHyphens(const std::string& date_string) {
    if (date_string.length() != 8) return "";

    std::string new_date = date_string;
    // YYYYMMDD
    // 01234567

    new_date.insert(4, "-");
    // YYYY-MMDD
    // 012345678

    new_date.insert(7, "-");
    // YYYY-MM-DD
    // 0123456789

    return new_date;
}


// ========================================================================================
// Remove any embedded hyphen "-" from a date string.
// e.g.  2023-07-28 would be returned as 20230728
// ========================================================================================
std::string AfxRemoveDateHyphens(const std::string& date_string) {
    std::string new_date = date_string;
    new_date.erase(remove(new_date.begin(), new_date.end(), L'-'), new_date.end());
    return new_date;
}


// ========================================================================================
// Returns the current date in ISO format (YYYY-MM-DD)
// ========================================================================================
std::string AfxCurrentDate() {
    // Get current time point
    auto now = std::chrono::system_clock::now();

    // Convert to time_t for easy formatting
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);

    // Convert to tm structure for local time
    std::tm* now_tm = std::localtime(&now_c);

    // Convert to year_month_day object
    const std::chrono::year y{(int)now_tm->tm_year + 1900};
    const std::chrono::month m{(unsigned)now_tm->tm_mon + 1};
    const std::chrono::day d{(unsigned)now_tm->tm_mday};

    year_month_day ymd{y, m, d};
    return to_iso_string(ymd);
}


// ========================================================================================
// Returns the short date MMM DD from a date in ISO format (YYYY-MM-DD)
// We use this when dealing with Option expiration dates to display.
// ========================================================================================
std::string AfxShortDate(const std::string& date_text) {
    if (date_text.length() == 0) return "";
    return AfxGetShortMonthName(date_text) + " " + date_text.substr(8, 2);
}


// ========================================================================================
// Return the number of days between two dates (YYYY-MM-DD)
// ========================================================================================
int AfxDaysBetween(const std::string& date1, const std::string& date2) {
    if (date1.length() != 10) return 0;
    if (date2.length() != 10) return 0;

    // Convert year_month_day to sys_days
    year_month_day d1{from_iso_string(date1)};
    year_month_day d2{from_iso_string(date2)};

    auto sys_date1 = sys_days{d1};
    auto sys_date2 = sys_days{d2};

    // Calculate the difference in days
    auto diff = sys_date2 - sys_date1;

    // Print the number of days
    return diff.count();
}


// ========================================================================================
// Adds the specified number of days to the incoming date and returns the new date.
// ========================================================================================
std::string AfxDateAddDays(const std::string& date_text, int num_days_to_add) {
    // YYYY-MM-DD
    // 0123456789
    if (date_text.length() != 10) return "";

    // Convert year_month_day to sys_days
    year_month_day d1{from_iso_string(date_text)};
    auto sd = sys_days{d1} + days(num_days_to_add);

    // Convert sys_days to year_month_day
    year_month_day ymd{sys_days(sd)};

    return to_iso_string(ymd);
}


// ========================================================================================
// Returns the year from a date in ISO format (YYYY-MM-DD)
// ========================================================================================
int AfxGetYear(const std::string& date_text) {
    // YYYY-MM-DD
    // 0123456789
    if (date_text.length() != 10) return 0;
    year_month_day dt{from_iso_string(date_text)};
    return static_cast<int>(dt.year());
}


// ========================================================================================
// Returns the month from a date in ISO format (YYYY-MM-DD)
// ========================================================================================
int AfxGetMonth(const std::string& date_text) {
    // YYYY-MM-DD
    // 0123456789
    if (date_text.length() != 10) return 0;
    year_month_day dt{from_iso_string(date_text)};
    return (int)static_cast<unsigned>(dt.month());
}


// ========================================================================================
// Returns the day from a date in ISO format (YYYY-MM-DD)
// ========================================================================================
int AfxGetDay(const std::string& date_text) {
    // YYYY-MM-DD
    // 0123456789
    if (date_text.length() != 10) return 0;
    year_month_day dt{from_iso_string(date_text)};
    return (int)static_cast<unsigned>(dt.day());
}


// ========================================================================================
// Returns the current day of the week.
// It is a numeric value in the range of 0-6 (representing Sunday through Saturday).
// ========================================================================================
int AfxLocalDayOfWeek() {
    auto today = year_month_weekday{floor<days>(system_clock::now())};
    sys_days sd = sys_days{today};

    // Get the weekday
    weekday wd = weekday{sd};
    return (int)static_cast<unsigned>(wd.c_encoding());
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
// Return the days in month from an ISO specified date.
// ========================================================================================
int AfxDaysInMonthISODate(const std::string& date_text) {
    // YYYY-MM-DD
    // 0123456789
    if (date_text.length() != 10) return 0;
    year_month_day dt{from_iso_string(date_text)};
    return AfxDaysInMonth((int)static_cast<unsigned>(dt.month()), static_cast<int>(dt.year()));
}


// ========================================================================================
// Return weekday number of the specified date. (0 based)
// ========================================================================================
int AfxDateWeekday(int day, int month, int year) {
    static int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
    year -= month < 3;
    return (year + year / 4 - year / 100 + year / 400 + t[month - 1] + day) % 7;
}


// ========================================================================================
// Right align string in of string of size width
// ========================================================================================
std::string AfxRSet(const std::string& text, int width) {
    if (text.length() > width) return text;
    size_t num_chars = width - text.length();
    if (num_chars <= 0) return text;
    std::string spaces_string(num_chars, L' ');
    return spaces_string + text;
}


// ========================================================================================
// Left align string in of string of size width
// ========================================================================================
std::string AfxLSet(const std::string& text, int width) {
    if (text.length() > width) return text;
    size_t num_chars = width - text.length();
    if (num_chars <= 0) return text;
    std::string spaces_string(num_chars, L' ');
    return text + spaces_string;
}


// ========================================================================================
// Replace one char/string another char/string. Return a copy.
// ========================================================================================
std::string AfxReplace(const std::string& str, const std::string& from, const std::string& to) {
    std::string text_string = str;
    if (str.empty()) return text_string;
    if (from.empty()) return text_string;
    size_t start_pos = 0;
    const size_t to_length = to.length();
    while ((start_pos = text_string.find(from, start_pos)) != std::string::npos) {
        text_string.replace(start_pos, from.length(), to);
        start_pos += to_length;     // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
    return text_string;
}


// ========================================================================================
// Remove char/string from string. Return a copy.
// ========================================================================================
std::string AfxRemove(const std::string& text, const std::string& repl) {
    std::string text_string = text;
    std::string::size_type i = text_string.find(repl);
    while (i != std::string::npos) {
        text_string.erase(i, repl.length());
        i = text_string.find(repl, i);
    }
    return text_string;
}


// ========================================================================================
// Returns the Futures Contract date MMMDD from a date in ISO format (YYYY-MM-DD)
// This is used for display purposes on the Trade Management screen.
// ========================================================================================
std::string AfxFormatFuturesDate(const std::string& date_text) {
    if (date_text.length() == 0) return "";
    int month = AfxGetMonth(date_text);
    std::string month_text = AfxUpper(AfxGetShortMonthName(date_text));
    std::string day_text = date_text.substr(8, 2);
    std::string text = month_text + day_text;
    return text;
}


// ========================================================================================
// Returns the Futures Contract date YYYYMM from a date in ISO format (YYYY-MM-DD)
// This is used for retrieveing market data.
// ========================================================================================
std::string AfxFormatFuturesDateMarketData(const std::string& date_text) {
    if (date_text.length() == 0) return "";
    // Date enters as YYYY-MM-DD so we simply need to remove the hyphens and
    // take the first 6 digits.
    std::string newdate = AfxRemove(date_text, "-");
    return newdate.substr(0,6);
}


// ========================================================================================
// Returns the short format day based on the specified date in ISO format (YYYY-MM-DD)
// ========================================================================================
std::string AfxGetShortDayName(const std::string& date_text) {
    // Define the array of day names
    const std::array<std::string, 7> day_names =
    {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    if (date_text.length() == 0) return "";
    year_month_day ymd{from_iso_string(date_text)};
    // Convert year_month_day to sys_days
    sys_days sd = sys_days{ymd};
    // Get the weekday
    weekday wd = weekday{sd};  // weekday is already zero based
    // Return the name of the day
    return day_names[wd.c_encoding()];
}


// ========================================================================================
// Returns the long format day based on the specified date in ISO format (YYYY-MM-DD)
// ========================================================================================
std::string AfxGetLongDayName(const std::string& date_text) {
    // Define the array of day names
    const std::array<std::string, 7> day_names =
    {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    if (date_text.length() == 0) return "";
    year_month_day ymd{from_iso_string(date_text)};
    // Convert year_month_day to sys_days
    sys_days sd = sys_days{ymd};
    // Get the weekday
    weekday wd = weekday{sd};  // weekday is already zero based
    // Return the name of the day
    return day_names[wd.c_encoding()];
}


// ========================================================================================
// Returns the short format month based on the specified date in ISO format (YYYY-MM-DD)
// ========================================================================================
std::string AfxGetShortMonthName(const std::string& date_text) {
    // Define the array of day names
    const std::array<std::string, 12> month_names =
        {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
    if (date_text.length() == 0) return "";
    year_month_day ymd{from_iso_string(date_text)};
    int m = (int)static_cast<unsigned>(ymd.month()) - 1;   // convert 1 based to zero based
    return month_names[m];
}


// ========================================================================================
// Returns the long format month based on the specified date in ISO format (YYYY-MM-DD)
// ========================================================================================
std::string AfxGetLongMonthName(const std::string& date_text) {
    // Define the array of day names
    const std::array<std::string, 12> month_names =
        {"January","February","March","April","May","June","July","August","September","October","November","December"};
    if (date_text.length() == 0) return "";
    year_month_day ymd{from_iso_string(date_text)};
    int m = (int)static_cast<unsigned>(ymd.month()) - 1;   // convert 1 based to zero based
    return month_names[m];
}


// ========================================================================================
// Format a numeric (double) string with specified decimal places.
// Decimal places = 2 unless specified otherwise.
// No leading dollar sign is added.
// ========================================================================================
std::string AfxMoney(double value, int num_decimal_places, AppState& state) {
    // Set the output to show specified decimal places and use the fixed format
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(num_decimal_places);

    // Output the amount as currency with correct separators
    if (state.config.number_format_type == NumberFormatType::American) {
        oss.imbue(std::locale("en_US.UTF-8"));
    }
    if (state.config.number_format_type == NumberFormatType::European) {
        oss.imbue(std::locale("de_DE.UTF-8"));
    }
    oss << value;

    std::string res = oss.str();
    return res;
}


// ========================================================================================
// Returns the path of the program which is currently executing.
// The path name will not contain a trailing backslash.
// ========================================================================================
#ifdef _WIN32
#include <windows.h>
#elif __APPLE__

#include <mach-o/dyld.h>
#include <climits>

#else   // Linux
#include <unistd.h>
#endif

std::string AfxGetExePath() {
    #ifdef _WIN32
        // Windows specific
        wchar_t szPath[MAX_PATH];
        GetModuleFileNameW( NULL, szPath, MAX_PATH );
        std::string path(std::filesystem::path(szPath).generic_string());
        // Remove everything after the last trailing backslash
        std::size_t found = path.find_last_of("/\\");
        path = path.substr(0, found);
        return path;
    #elif __APPLE__
        uint32_t bufsize = 1024;
        char szPath[bufsize];
        if (_NSGetExecutablePath(szPath, &bufsize) == 0) {
            std::string path(std::filesystem::path(szPath).generic_string());
            // Remove everything after the last trailing backslash
            std::size_t found = path.find_last_of("/\\");
            path = path.substr(0, found);
            return path;
        }
        return {};  // some error
    #else
        // Linux specific
        char szPath[256];
        ssize_t count = readlink( "/proc/self/exe", szPath, 256 );
        if( count < 0 || count >= 256 ) return {}; // some error
        szPath[count] = '\0';
        std::string path(std::filesystem::path(szPath).generic_string());
        // Remove everything after the last trailing backslash
        std::size_t found = path.find_last_of("/\\");
        path = path.substr(0, found);
        return path;
    #endif

}


// ========================================================================================
// Searches a directory for a file or subdirectory with a name that matches a specific name
// (or partial name if wildcards are used).
// ========================================================================================
bool AfxFileExists(const std::string& path) {
    const std::filesystem::path p = path;
    return std::filesystem::exists(p);
}

