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

#pragma once
#include <Windows.h>
#include <commctrl.h>
#include <versionhelpers.h>
#include <string>
#include <winver.h>


std::wstring AfxExecCmd(std::wstring cmd);

void AfxRedrawWindow(HWND hwnd);
std::wstring AfxGetWindowText(HWND hwnd);
bool AfxSetWindowText(HWND hwnd, const std::wstring& text);

float AfxScaleRatioX();
float AfxScaleRatioY();
int AfxScaleX(float cx);
int AfxScaleY(float cy);
int AfxUnScaleX(float cx);
int AfxUnScaleY(float cy);

int AfxGetClientWidth(HWND hwnd);
int AfxGetClientHeight(HWND hwnd);
int AfxGetWindowWidth(HWND hwnd);
int AfxGetWindowHeight(HWND hwnd);

void AfxCenterWindow(HWND hwnd = NULL, HWND hwndParent = NULL);

int AfxGetWorkAreaWidth();
int AfxGetWorkAreaHeight();

int AfxComCtlVersion();
int AfxGetFileVersion(const std::wstring& pwszFileName);

HWND AfxAddTooltip(HWND hwnd, const std::wstring& text, bool bBalloon = FALSE, bool bCentered = FALSE);
void AfxSetTooltipText(HWND hTooltip, HWND hwnd, std::wstring& text);

std::wstring AfxGetListBoxText(HWND hListBox, int index);

std::wstring AfxGetExePath();
std::wstring AfxCurrentDate();

int AfxValInteger(std::wstring st);
double AfxValDouble(std::wstring st);

std::wstring AfxInsertDateHyphens(const std::wstring& dateString);
std::wstring AfxRemoveDateHyphens(const std::wstring& dateString);
bool AfxIsLeapYear(int nYear);
int AfxDaysInMonth(int nMonth, int nYear);
int AfxDaysInMonthISODate(const std::wstring& date_text);
std::wstring AfxShortDate(const std::wstring& date_text);
std::wstring AfxGetShortMonthName(const std::wstring& date_text);
std::wstring AfxGetLongMonthName(const std::wstring& date_text);
std::wstring AfxGetShortDayName(const std::wstring& date_text);
std::wstring AfxGetLongDayName(const std::wstring& date_text);
std::wstring AfxMakeISODate(int year, int month, int day);
std::wstring AfxFormatFuturesDate(const std::wstring& date_text);
std::string AfxFormatFuturesDateMarketData(const std::wstring& date_text);
int AfxGetYear(const std::wstring& date_text);
int AfxGetMonth(const std::wstring& date_text);
int AfxGetDay(const std::wstring& date_text);
int AfxLocalYear();
int AfxLocalMonth();
int AfxLocalDay();
int AfxLocalDayOfWeek();
unsigned int AfxUnixTime(const std::wstring& date_text);
int AfxDaysBetween(const std::wstring& date1, const std::wstring& date2);
std::wstring AfxDateAddDays(const std::wstring& date_text, int numDaysToAdd);
std::wstring AfxLongDate(const std::wstring& date_text);


std::wstring AfxGetDefaultFont();
std::string unicode2ansi(const std::wstring& wstr);
std::wstring ansi2unicode(const std::string& str);
std::wstring AfxMoney(double value, bool UseMinusSign = false, int NumDecimalPlaces = 2);
int Listbox_ItemFromPoint(HWND hListBox, SHORT x, SHORT y);
bool AfxStringCompareI(const std::string& s1, const std::string& s2);
bool AfxWStringCompareI(const std::wstring& s1, const std::wstring& s2);
std::vector<std::wstring> AfxSplit(std::wstring str, std::wstring delimiter);
bool AfxFileExists(const std::wstring& wszFileSpec);
std::wstring AfxReplace(std::wstring& str, const std::wstring& from, const std::wstring& to);
std::wstring AfxRemove(std::wstring text, std::wstring repl);
std::wstring& AfxLTrim(std::wstring& s);
std::wstring& AfxRTrim(std::wstring& s);
std::wstring& AfxTrim(std::wstring& s);
DWORD AfxAddWindowStyle(HWND hwnd, DWORD dwStyle);
DWORD AfxRemoveWindowStyle(HWND hwnd, DWORD dwStyle);
DWORD AfxAddWindowExStyle(HWND hwnd, DWORD dwExStyle);
DWORD AfxRemoveWindowExStyle(HWND hwnd, DWORD dwExStyle);
bool Header_SetItemWidth(HWND hwndHD, int nItem, int nWidth);
bool Header_SetItemText(HWND hwndHD, int nItem, LPCWSTR ptext);
std::wstring Header_GetItemText(HWND hwndHD, int nItem);
int Header_GetItemAlignment(HWND hwndHD, int nItem);
bool Header_InsertNewItem(HWND hwndHD, int iInsertAfter, int nWidth, LPCWSTR ptext, int Alignment = HDF_LEFT);
std::wstring AfxUpper(const std::wstring& text);
std::wstring AfxLower(const std::wstring& text);
std::wstring AfxRSet(const std::wstring& text, int nWidth);
std::wstring AfxLSet(const std::wstring& text, int nWidth);


