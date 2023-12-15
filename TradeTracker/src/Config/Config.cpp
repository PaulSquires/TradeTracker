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

#include "pch.h"

#include <unordered_map>
#include "Utilities/AfxWin.h"

#include "CustomLabel/CustomLabel.h"
#include "MainWindow/MainWindow.h"

#include "Config.h"


const std::wstring dbConfig = AfxGetExePath() + L"\\IB-Tracker-config.txt";
const std::wstring dbJournalNotes = AfxGetExePath() + L"\\IB-Tracker-journalnotes.txt";
const std::wstring dbTradePlan = AfxGetExePath() + L"\\IB-Tracker-tradeplan.txt";

const std::wstring idMagic = L"IB-TRACKER-CONFIG";

std::wstring journal_notes_text = L"";
std::wstring trade_plan_text = L"";

int startup_width = 0;
int startup_height = 0;
int startup_right_panel_width = 0;

bool startup_paper_trading = false;
bool show_portfolio_value = true;
bool allow_update_check = true;
 

std::unordered_map<int, std::wstring> mapCategoryDescriptions {
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

std::unordered_map<std::string, std::string> mapFuturesExchanges {
    { "/AUD", "CME" },
    { "/EUR", "CME" },
    { "/GBP", "CME" },
    { "/ES",  "CME" },
    { "/MES", "CME" }, 
    { "/HE",  "CME" }, 
    { "/LE",  "CME" }, 
    { "/GC",  "COMEX" }, 
    { "/NG",  "NYMEX" },
    { "/CL",  "NYMEX" },
    { "/MCL", "NYMEX" },
    { "/ZB",  "CBOT" },
    { "/ZC",  "CBOT" },
    { "/ZS",  "CBOT" }
};

std::unordered_map<std::wstring, std::wstring> mapMultipliers {
    { L"/AUD", L"100000" },
    { L"/EUR", L"125000" },
    { L"/GBP", L"62500" },
    { L"/ES",  L"50" },
    { L"/MES", L"5" },
    { L"/CL",  L"1000" },
    { L"/HE",  L"400" },
    { L"/LE",  L"400" },
    { L"/ZB",  L"1000" }
};

std::unordered_map<std::wstring, int> mapTickerDecimals {
    { L"/AUD", 5 },
    { L"/EUR", 5 },
    { L"/GBP", 4 },
    { L"/ZB", 3 },
    { L"/ZC", 3 },
    { L"/LE", 3 },
    { L"/HE", 4 },
    { L"/ZS", 4 }
};



// ========================================================================================
// Display Paper Trading warning message if port is enabled. 
// Normal Trading 7496;   7497 is paper trading account.
// ========================================================================================
void DisplayPaperTradingWarning()
{
    if (!startup_paper_trading) return;

    CustomLabel_SetText(GetDlgItem(HWND_MAINWINDOW, IDC_MAINWINDOW_WARNING),
        L"*** USING PAPER TRADING ACCOUNT ***");
    ShowWindow(GetDlgItem(HWND_MAINWINDOW, IDC_MAINWINDOW_WARNING), SW_SHOWNORMAL);
}


// ========================================================================================
// Determine if show/hide the Portfolio dollar value on the main screen.
// ========================================================================================
bool GetShowPortfolioValue()
{
    return show_portfolio_value;
}


// ========================================================================================
// Determine if allow to check for available program update.
// ========================================================================================
bool GetAllowUpdateCheck()
{
    return allow_update_check;
}


// ========================================================================================
// Return the TWS connect Port. 
// Normal Trading 7496;   7497 is paper trading account.
// ========================================================================================
int GetStartupPort()
{
    return (startup_paper_trading ? 7497 : 7496);   // paper trading port is 7497
}


// ========================================================================================
// Return the application's startup width value as returned from the Config file.
// If no value exits then return default value.
// ========================================================================================
int GetStartupWidth()
{

    // Size the main window to encompass 75% of screen width.
    int inital_main_width = AfxUnScaleX(AfxGetWorkAreaWidth() * 0.75f);

    // Impose a maximum size on the initial height/width in order
    // to ensure that the main window does not display exceptionally
    // large especially when run on very large monitors.
    // Target a minimum 720p screen resolution size (1280 x 720).
    if (inital_main_width > 1280) inital_main_width = 1280;

    // Get the previously saved Startup Width and restrict its maximum
    // to the size of the width of the work area.
    if (startup_width == 0 ||
        startup_width > AfxUnScaleX(AfxGetWorkAreaWidth()))
        startup_width = inital_main_width;
    
    return startup_width;
}


// ========================================================================================
// Return the application's startup height value as returned from the Config file.
// If no value exits then return default value.
// ========================================================================================
int GetStartupHeight()
{
    // Size the main window to encompass 85% of screen height.
    int inital_main_height = AfxUnScaleY(AfxGetWorkAreaHeight() * 0.85f);

    // Impose a maximum size on the initial height/width in order
    // to ensure that the main window does not display exceptionally
    // large especially when run on very large monitors.
    // Target a minimum 720p screen resolution size (1280 x 720).
    if (inital_main_height > 720) inital_main_height = 720;

    // Get the previously saved Startup Height and restrict its maximum
    // to the size of the height of the work area.
    if (startup_height == 0 ||
        startup_height > AfxUnScaleY(AfxGetWorkAreaHeight()))
        startup_height = inital_main_height;

    return startup_height;
}


// ========================================================================================
// Set the application's startup height value.
// ========================================================================================
void SetStartupHeight(int height)
{
    startup_height = AfxUnScaleY(height);
}


// ========================================================================================
// Set the application's startup width value.
// ========================================================================================
void SetStartupWidth(int width)
{
    startup_width = AfxUnScaleX(width);
}


// ========================================================================================
// Return the application's Right Panel startup width value as returned from the Config file.
// If no value exits then return default value.
// ========================================================================================
int GetStartupRightPanelWidth()
{
    if (startup_right_panel_width > 0) return startup_right_panel_width;
    return 440;
}


// ========================================================================================
// Set the application's Right Panel startup width value.
// ========================================================================================
void SetStartupRightPanelWidth(int width)
{
    startup_right_panel_width = AfxUnScaleX(width);
}


// ========================================================================================
// Determine if the incoming ticker symbol is a Future.
// ========================================================================================
bool IsFuturesTicker(const std::wstring& ticker)
{
    return (ticker.substr(0, 1) == L"/");
}


// ========================================================================================
// Get the Ticker Decimals for the incoming underlying.
// ========================================================================================
int GetTickerDecimals(const std::wstring& underlying)
{
    if (mapTickerDecimals.count(underlying)) {
        return mapTickerDecimals.at(underlying);
    }
    else {
        return 2;
    }
}


// ========================================================================================
// Set the Ticker Decimals for the incoming underlying.
// ========================================================================================
void SetTickerDecimals(const std::wstring& underlying, int decimals)
{
    mapTickerDecimals[underlying] = decimals;
}


// ========================================================================================
// Get the Futures Multiplier for the incoming underlying.
// ========================================================================================
std::wstring GetMultiplier(const std::wstring& underlying)
{
    if (mapMultipliers.count(underlying)) {
        return mapMultipliers.at(underlying);
    }
    else {
        return L"100";
    }
}


// ========================================================================================
// Set the Futures Multiplier for the incoming underlying.
// ========================================================================================
void SetMultiplier(const std::wstring& underlying, const std::wstring& multiplier)
{
    mapMultipliers[underlying] = multiplier;
}


// ========================================================================================
// Get the Futures Exchanges for the incoming underlying.
// ========================================================================================
std::string GetFuturesExchange(const std::string& underlying)
{
    if (mapFuturesExchanges.count(underlying)) {
        return mapFuturesExchanges.at(underlying);
    }
    else {
        return "CME";
    }
}


// ========================================================================================
// Set the Futures Exchanges for the incoming underlying.
// ========================================================================================
void SetFuturesExchange(const std::string& underlying, const std::string& exchange)
{
    mapFuturesExchanges[underlying] = exchange;
}


// ========================================================================================
// Get the Category Description for the incoming category number.
// ========================================================================================
std::wstring GetCategoryDescription(int category_index)
{
    return mapCategoryDescriptions.at(category_index);
}


// ========================================================================================
// Set the Category Description for the incoming category number.
// ========================================================================================
void SetCategoryDescription(int category_index, const std::wstring& wszDescription)
{
    mapCategoryDescriptions[category_index] = wszDescription;
}


// ========================================================================================
// Get the JournalNotes text.
// ========================================================================================
std::wstring GetJournalNotesText()
{
    static bool is_JournalNotes_loaded = false;

    if (!is_JournalNotes_loaded) {
        std::wifstream db;

        db.open(dbJournalNotes, std::ios::in);

        if (db.is_open()) {
            std::wostringstream ss;
            ss << db.rdbuf();
            journal_notes_text = ss.str();

            is_JournalNotes_loaded = true;
        }
    }

    return journal_notes_text;
}


// ========================================================================================
// Set and save the JournalNotes text.
// ========================================================================================
void SetJournalNotesText(const std::wstring& wszText)
{
    std::wofstream db;

    db.open(dbJournalNotes, std::ios::out | std::ios::trunc);

    if (!db.is_open()) {
        MessageBox(
            NULL,
            (LPCWSTR)(L"Could not save JournalNotes text to file"),
            (LPCWSTR)L"Warning",
            MB_ICONWARNING
        );
        return;
    }

    db << wszText;
    db.close();

    journal_notes_text = wszText;
}


// ========================================================================================
// Get the TradePlan text.
// ========================================================================================
std::wstring GetTradePlanText()
{
    static bool is_TradePlan_loaded = false;

    if (!is_TradePlan_loaded) {
        std::wifstream db;

        db.open(dbTradePlan, std::ios::in);

        if (db.is_open()) {
            std::wostringstream ss;
            ss << db.rdbuf();
            trade_plan_text = ss.str();

            is_TradePlan_loaded = true;
        }
    }

    return trade_plan_text;
}


// ========================================================================================
// Set and save the TradePlan text.
// ========================================================================================
void SetTradePlanText(const std::wstring& wszText)
{
    std::wofstream db;

    db.open(dbTradePlan, std::ios::out | std::ios::trunc);

    if (!db.is_open()) {
        MessageBox(
            NULL,
            (LPCWSTR)(L"Could not save TradePlan text to file"),
            (LPCWSTR)L"Warning",
            MB_ICONWARNING
        );
        return;
    }

    db << wszText;
    db.close();

    trade_plan_text = wszText;
}


// ========================================================================================
// Save the Config values to a simple text file.
// ========================================================================================
bool SaveConfig()
{
    std::wofstream db;

    db.open(dbConfig, std::ios::out | std::ios::trunc);

    if (!db.is_open()) {
        MessageBox(
            NULL,
            (LPCWSTR)(L"Could not save configuration file"),
            (LPCWSTR)L"Warning",
            MB_ICONWARNING
        );
        return false;
    }


    db << idMagic << "|" << version << "\n";

    db << "ENABLEPAPERTRADING|" << (startup_paper_trading ? L"true" : L"false") << "\n";
    
    db << "SHOWPORTFOLIOVALUE|" << (show_portfolio_value ? L"true" : L"false") << "\n";
    
    db << "ALLOWUPDATECHECK|" << (allow_update_check ? L"true" : L"false") << "\n";

    db << "STARTUPWIDTH" << "|" << startup_width << "\n";

    db << "STARTUPHEIGHT" << "|" << startup_height << "\n";

    db << "STARTUPRIGHTPANELWIDTH" << "|" << startup_right_panel_width << "\n";

    for (auto item : mapCategoryDescriptions) {
        db << "CATEGORY|" << item.first << "|" << item.second << "\n";
    }

    for (auto item : mapFuturesExchanges) {
        db << "EXCHANGE|" << ansi2unicode(item.first) << "|" << ansi2unicode(item.second) << "\n";
    }

    for (auto item : mapMultipliers) {
        db << "MULTIPLIER|" << item.first << "|" << item.second << "\n";
    }

    for (auto item : mapTickerDecimals) {
        db << "TICKERDECIMALS|" << item.first << "|" << item.second << "\n";
    }

    db.close();

    return true;
}


// ========================================================================================
// Load the Config values from file.
// ========================================================================================
bool LoadConfig()
{
    // If the Config does not exist then create a new one.
    if (!AfxFileExists(dbConfig)) {
        SaveConfig();
    }

    std::wifstream db;

    db.open(dbConfig, std::ios::in);

    if (!db.is_open()) {
        db.close();
        return false;
    }

    std::wstring config_version;

    bool isFirstline = true;

    std::wstring line;

    while (!db.eof()) {
        std::getline(db, line);

        if (line.length() == 0) continue;

        // If this is a Comment line then simply iterate to next line.
        if (line.compare(1, 3, L"// ") == 0) continue;

        // Tokenize the line into a vector based on the pipe delimiter
        std::vector<std::wstring> st = AfxSplit(line, L'|');

        if (st.empty()) continue;

        // First line must be the database identifier and version
        if (isFirstline) {
            if (st.at(0) != idMagic) {
                MessageBox(
                    NULL,
                    (LPCWSTR)(L"Invalid configuration file"),
                    (LPCWSTR)L"Warning",
                    MB_ICONWARNING
                );
                db.close();
                return false;
            }
            config_version = st.at(1);
            isFirstline = false;
            continue;
        }


        std::wstring arg = AfxTrim(st.at(0));


        // Check for paper trading
        if (arg == L"ENABLEPAPERTRADING") {
            std::wstring paper;
            
            try {paper = AfxTrim(st.at(1)); }
            catch (...) { continue; }
        
            startup_paper_trading = AfxWStringCompareI(paper, L"true");
            continue;
        }


        // Check if should show/hide Total Portfolio dollar amount on main screen
        if (arg == L"SHOWPORTFOLIOVALUE") {
            std::wstring value;
            
            try {value = AfxTrim(st.at(1)); }
            catch (...) { continue; }
        
            show_portfolio_value = AfxWStringCompareI(value, L"true");
            continue;
        }


        // Check if should allow checking for available program update
        if (arg == L"ALLOWUPDATECHECK") {
            std::wstring value;
            
            try {value = AfxTrim(st.at(1)); }
            catch (...) { continue; }
        
            allow_update_check = AfxWStringCompareI(value, L"true");
            continue;
        }


        // Check for startup_width
        if (arg == L"STARTUPWIDTH") {
            std::wstring width;

            try { width = AfxTrim(st.at(1)); }
            catch (...) { continue; }

            startup_width = AfxValInteger(width);
            continue;
        }


        // Check for startup_height
        if (arg == L"STARTUPHEIGHT") {
            std::wstring height;

            try { height = AfxTrim(st.at(1)); }
            catch (...) { continue; }

            startup_height = AfxValInteger(height);
            continue;
        }


        // Check for category descriptions
        if (arg == L"CATEGORY") {
            int category_index;
            std::wstring description;
            
            try {category_index = stoi(st.at(1));}
            catch (...) {continue;}
            
            try {description = st.at(2);}
            catch (...) {continue;}
            
            SetCategoryDescription(category_index, description);
            continue;
        }


        // Check for startup_right_panel_width
        if (arg == L"STARTUPRIGHTPANELWIDTH") {
            std::wstring width;

            try { width = AfxTrim(st.at(1)); }
            catch (...) { continue; }

            startup_right_panel_width = AfxValInteger(width);
            continue;
        }


        // Check for Futures Exchanges
        if (arg == L"EXCHANGE") {
            std::wstring text1;
            std::wstring text2;
            
            try {text1 = st.at(1);}
            catch (...) {continue;}
            
            try {text2 = st.at(2);}
            catch (...) {continue;}
            
            std::string underlying = unicode2ansi(text1);
            std::string exchange = unicode2ansi(text2);
            SetFuturesExchange(underlying, exchange);

            continue;
        }


        // Check for Multipliers
        if (arg == L"MULTIPLIER") {
            std::wstring underlying;
            std::wstring multiplier;
            
            try {underlying = st.at(1);}
            catch (...) {continue;}
            
            try { multiplier = st.at(2);}
            catch (...) {continue;}
            
            SetMultiplier(underlying, multiplier);

            continue;
        }


        // Check for Ticker Decimals
        if (arg == L"TICKERDECIMALS") {
            std::wstring underlying;
            int numDecimals;
            
            try {underlying = st.at(1);}
            catch (...) {continue;}
            
            try { numDecimals = stoi(st.at(2));}
            catch (...) {continue;}
            
            SetTickerDecimals(underlying, numDecimals);

            continue;
        }

    }

    db.close();

    return true;
}
