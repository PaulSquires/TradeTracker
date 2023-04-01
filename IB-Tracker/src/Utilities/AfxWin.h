#pragma once
#include <Windows.h>
#include <commctrl.h>
#include <versionhelpers.h>
#include <string>
#include <winver.h>

void AfxRedrawWindow(HWND hwnd);
std::wstring AfxGetWindowText(HWND hwnd);

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
int AfxGetYear(std::wstring wszDate);
int AfxDaysBetween(std::wstring wszStartDate, std::wstring wszEndDate);
std::wstring AfxShortDate(std::wstring wszDate);
bool isWineActive();
std::wstring AfxGetDefaultFont();
std::string unicode2ansi(const std::wstring& wstr);
std::wstring ansi2unicode(const std::string& str);
std::wstring AfxMoney(double value);
int Listbox_ItemFromPoint(HWND hListBox, SHORT x, SHORT y);

