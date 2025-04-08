/*

MIT License

Copyright(c) 2023-2025 Paul Squires

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

#ifndef AFXHELPER_H
#define AFXHELPER_H

#include <string>
#include <vector>
#include <chrono>

#if defined(_WIN32) // win32 and win64
#include <cwctype>
#include <shlobj.h>
#include <initguid.h>
std::wstring ansi2unicode(const std::string& str);
std::string unicode2ansi(const std::wstring& wstr);
bool AfxCreateNestedFolder(const std::string& folderPath);
std::string AfxGetLocalAppDataFolder();
std::string AfxGetProgramFilesFolder();
std::string AfxExecCmd(const std::string& cmd);
#endif

#include "appstate.h"

using namespace std::chrono;

std::string GetDataFilesFolder();
std::string AfxDoubleToString(const double value, const int num_decimal_places);
int AfxValInteger(const std::string& st);
double AfxValDouble(const std::string& st);
std::string AfxGetExePath();
bool AfxFileExists(const std::string& path);
std::string AfxLoadFileIntoString(const std::string& filename);
bool AfxSaveStringToFile(const std::string& filename, const std::string& data);
std::vector<std::string> AfxSplit(const std::string& input, char delimiter);
std::string AfxLower(const std::string& text);
std::string AfxUpper(const std::string& text);
std::string AfxLTrim(const std::string& s);
std::string AfxRTrim(const std::string& s);
std::string AfxTrim(const std::string& s);
std::string AfxReplace(const std::string& str, const std::string& from, const std::string& to);
std::string AfxRemove(const std::string& text, const std::string& repl);
std::string AfxRSet(const std::string& text, int width);
std::string AfxLSet(const std::string& text, int width);
std::string AfxMoney(double value, int num_decimal_places, AppState& state);
std::chrono::year_month_day from_iso_string(const std::string& iso_string);
std::string to_iso_string(const year_month_day& ymd);
std::string AfxGetShortMonthName(const std::string& date_text);
std::string AfxGetLongMonthName(const std::string& date_text);
std::string AfxGetShortDayName(const std::string& date_text);
std::string AfxGetLongDayName(const std::string& date_text);
std::string AfxInsertDateHyphens(const std::string& date_string);
std::string AfxRemoveDateHyphens(const std::string& date_string);
std::string AfxFormatFuturesDate(const std::string& date_text);
std::string AfxFormatFuturesDateMarketData(const std::string& date_text);
int AfxGetYear(const std::string& date_text);
int AfxGetMonth(const std::string& date_text);
int AfxGetDay(const std::string& date_text);
int AfxLocalDayOfWeek();
std::string AfxCurrentDate();
std::string AfxShortDate(const std::string& date_text);
int AfxDaysBetween(const std::string& date1, const std::string& date2);
std::string AfxDateAddDays(const std::string& date_text, int num_days_to_add);
bool AfxIsLeapYear(int year);
int AfxDaysInMonth(int month, int year);
int AfxDaysInMonthISODate(const std::string& date_text);
int AfxDateWeekday(int day, int month, int year);
std::string AfxGetShortDayName(const std::string& date_text);
std::string AfxGetLongDayName(const std::string& date_text);
std::string AfxGetShortMonthName(const std::string& date_text);
std::string AfxGetLongMonthName(const std::string& date_text);

#endif //AFXHELPER_H

