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

constexpr std::wstring version = L"3.3.0";

bool SaveConfig();
bool LoadConfig();

int GetTickerDecimals(std::wstring wunderlying);
void SetTickerDecimals(std::wstring wunderlying, int numDecimals);
std::wstring GetMultiplier(std::wstring wunderlying);
void SetMultiplier(std::wstring wunderlying, std::wstring wszMultiplier);
std::string GetFuturesExchange(std::string underlying);
void SetFuturesExchange(std::string underlying, std::string exchange);
std::wstring GetCategoryDescription(int category_index);
void SetCategoryDescription(int category_index, std::wstring wszDescription);
void DisplayPaperTradingWarning();
int GetStartupPort();
bool IsFuturesTicker(const std::wstring& wszTicker);
int GetStartupWidth();
int GetStartupHeight();
int GetStartupRightPanelWidth();
void SetStartupWidth(int width);
void SetStartupHeight(int height);
void SetStartupRightPanelWidth(int width);
std::wstring GetJournalNotesText();
void SetJournalNotesText(std::wstring wszText);
