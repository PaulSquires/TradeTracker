#pragma once
#include <Windows.h>
#include <commctrl.h>
#include <versionhelpers.h>
#include <string>
#include <winver.h>

void AfxRedrawWindow(HWND hwnd);
std::wstring AfxGetWindowText(HWND hwnd);
bool AfxSetWindowText(HWND hwnd, const std::wstring& wszText);

float AfxScaleRatioX();
float AfxScaleRatioY();
int AfxScaleX(float cx);
int AfxScaleY(float cy);
int AfxUnScaleX(float cx);
int AfxUnScaleY(float cy);

int AfxGetWindowWidth(HWND hwnd);
int AfxGetWindowHeight(HWND hwnd);
void AfxCenterWindow(HWND hwnd = NULL, HWND hwndParent = NULL);
int AfxGetWorkAreaWidth();
int AfxGetWorkAreaHeight();
int AfxComCtlVersion();
int AfxGetFileVersion(std::wstring pwszFileName);
HWND AfxAddTooltip(HWND hwnd, std::wstring wszText, bool bBalloon = FALSE, bool bCentered = FALSE);
void AfxSetTooltipText(HWND hTooltip, HWND hwnd, std::wstring wszText);
std::wstring AfxGetListBoxText(HWND hListBox, int nIndex);
std::wstring AfxGetExePath();
std::wstring AfxGetUserName();
std::wstring AfxCurrentDate();

std::wstring AfxShortDate(std::wstring wszDate);
std::wstring AfxGetShortDayName(std::wstring wszDate);
int AfxGetYear(std::wstring wszDate);
int AfxGetMonth(std::wstring wszDate);
int AfxGetDay(std::wstring wszDate);
int AfxLocalYear();
int AfxLocalMonth();
int AfxDaysBetween(std::wstring date1, std::wstring date2);
std::wstring AfxDateAddDays(std::wstring wszDate, int numDaysToAdd);

bool isWineActive();
std::wstring AfxGetDefaultFont();
std::string unicode2ansi(const std::wstring& wstr);
std::wstring ansi2unicode(const std::string& str);
std::wstring AfxMoney(double value, bool UseMinusSign = false);
int Listbox_ItemFromPoint(HWND hListBox, SHORT x, SHORT y);
bool AfxStringCompareI(const std::string& s1, const std::string& s2);
bool AfxWStringCompareI(const std::wstring& s1, const std::wstring& s2);
std::vector<std::wstring> AfxSplit(std::wstring str, std::wstring delimiter);
bool AfxFileExists(const std::wstring& wszFileSpec);
std::wstring& AfxLTrim(std::wstring& s);
std::wstring& AfxRTrim(std::wstring& s);
std::wstring& AfxTrim(std::wstring& s);
DWORD AfxRemoveWindowStyle(HWND hwnd, DWORD dwStyle);
DWORD AfxRemoveWindowExStyle(HWND hwnd, DWORD dwExStyle);
bool Header_SetItemWidth(HWND hwndHD, int nItem, int nWidth);
bool Header_SetItemText(HWND hwndHD, int nItem, LPCWSTR pwszText);
std::wstring Header_GetItemText(HWND hwndHD, int nItem);
int Header_GetItemAlignment(HWND hwndHD, int nItem);
bool Header_InsertNewItem(HWND hwndHD, int iInsertAfter, int nWidth, LPCWSTR pwszText, int Alignment = HDF_LEFT);
std::wstring AfxUpper(const std::wstring& wszText);
std::wstring AfxLower(const std::wstring& wszText);
void AfxSetDateTimePickerDate(HWND hCtl, std::wstring wszDate);
std::wstring AfxGetDateTimePickerDate(HWND hCtl);


