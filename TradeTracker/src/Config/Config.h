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

#pragma once

#include <unordered_map>
#include "Utilities/AfxWin.h"


constexpr std::wstring version = L"4.2.0";

enum class NumberFormatType {
    American,
    European
};

enum class CostingMethod{
    AverageCost,
    fifo
};

enum class StartWeekdayType {
    Sunday,
    Monday
};


class CConfig {
private:
    // Filenames used on and after Version 4.0.0
    std::wstring dbConfig_new = L"\\tt-config.txt";
    std::wstring exe_new = L"\\TradeTracker.exe";

    // Filenames used prior to Version 4.0.0
    const std::wstring dbConfig_old = AfxGetExePath() + L"\\IB-Tracker-config.txt";
    const std::wstring exe_old = AfxGetExePath() + L"\\IB-Tracker.exe";


    std::wstring dbConfig;

    int startup_width = 0;
    int startup_height = 0;
    int startup_right_panel_width = 0;

    bool display_open_source_license = true;
    bool show_portfolio_value = true;
    bool allow_update_check = true;
    bool exclude_nonstock_costs = false;

    NumberFormatType number_format_type = NumberFormatType::American;
    CostingMethod costing_method = CostingMethod::AverageCost;
    StartWeekdayType start_weekday = StartWeekdayType::Monday;

    std::unordered_map<int, std::wstring> mapCategoryDescriptions{
        { 0, L"Category 0"},
        { 1, L"Category 1" },
        { 2, L"Category 2" },
        { 3, L"Category 3" },
        { 4, L"Category 4" },
        { 5, L"Category 5" },
        { 6, L"Category 6" },
        { 7, L"Category 7" },
        { 8, L"Category 8" },
        { 9, L"Category 9" },
        { 10, L"Category 10" },
        { 11, L"Category 11" },
        { 12, L"Category 12" },
        { 13, L"Category 13" },
        { 14, L"Category 14" },
        { 15, L"Category 15" },
        { 99, L"All Categories" },
        {100, L"Other Income/Expense" }
    };

    std::unordered_map<std::string, std::string> mapFuturesExchanges{
        { "/AUD", "CME" },
        { "/EUR", "CME" },
        { "/GBP", "CME" },
        { "/ES",  "CME" },
        { "/MES", "CME" },
        { "/HE",  "CME" },
        { "/LE",  "CME" },
        { "/GC",  "COMEX" },
        { "/HG",  "COMEX" },
        { "/NG",  "NYMEX" },
        { "/CL",  "NYMEX" },
        { "/MCL", "NYMEX" },
        { "/ZB",  "CBOT" },
        { "/ZC",  "CBOT" },
        { "/ZS",  "CBOT" }
    };

    std::unordered_map<std::wstring, std::wstring> mapMultipliers{
        { L"/AUD", L"100000" },
        { L"/EUR", L"125000" },
        { L"/GBP", L"62500" },
        { L"/ES",  L"50" },
        { L"/MES", L"5" },
        { L"/CL",  L"1000" },
        { L"/HE",  L"400" },
        { L"/LE",  L"400" },
        { L"/HG",  L"25000" },
        { L"/ZB",  L"1000" }
    };

    std::unordered_map<std::wstring, int> mapTickerDecimals{
        { L"/AUD", 5 },
        { L"/EUR", 5 },
        { L"/GBP", 4 },
        { L"/ZB", 3 },
        { L"/ZC", 3 },
        { L"/LE", 3 },
        { L"/HE", 4 },
        { L"/ZS", 4 }
    };
    
    void Version4UpgradeExe();
    bool Version4UpgradeConfig();

public:
	bool SaveConfig();
	bool LoadConfig();

    std::wstring GetDataFilesFolder();
    StartWeekdayType GetStartWeekday();
    void SetStartWeekday(StartWeekdayType value);
    CostingMethod GetCostingMethod();
	void SetCostingMethod(CostingMethod value);
    bool GetExcludeNonStockCosts(); 
    void SetExcludeNonStockCosts(bool value);
    NumberFormatType GetNumberFormatType();
	void SetNumberFormatType(NumberFormatType value);
	bool GetDisplayLicense();
	void SetDisplayLicense(bool value);
    void DisplayLicense();
	bool GetAllowUpdateCheck();
	void SetAllowUpdateCheck(bool value);
	bool GetAllowPortfolioDisplay();
	void SetAllowPortfolioDisplay(bool value);
	int GetTickerDecimals(const std::wstring& underlying);
	void SetTickerDecimals(const std::wstring& underlying, int decimals);
	std::wstring GetMultiplier(const std::wstring& wunderlying);
	void SetMultiplier(const std::wstring& wunderlying, const std::wstring& multiplier);
	std::string GetFuturesExchange(const std::string& underlying);
	void SetFuturesExchange(const std::string& underlying, const std::string& exchange);
	std::wstring GetCategoryDescription(int index);
	void SetCategoryDescription(int index, const std::wstring& description);
	bool IsFuturesTicker(const std::wstring& ticker);
	int GetStartupWidth();
	int GetStartupHeight();
	int GetStartupRightPanelWidth();
	void SetStartupWidth(int width);
	void SetStartupHeight(int height);
	void SetStartupRightPanelWidth(int width);
};

extern CConfig config;
