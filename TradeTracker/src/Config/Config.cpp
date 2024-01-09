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

#include "Utilities/AfxWin.h"

#include "CustomLabel/CustomLabel.h"
#include "CustomMessageBox/CustomMessageBox.h"
#include "TextBoxDialog/TextBoxDialog.h"
#include "MainWindow/MainWindow.h"
#include "ActiveTrades/ActiveTrades.h"

#include "Config.h"


// ========================================================================================
// Determine if upgrade to Version 4 filenames. 
// ========================================================================================
std::wstring CConfig::GetDataFilesFolder() {
    std::wstring exe_folder = AfxGetExePath();  // no trailing backslash
    std::wstring program_files_folder = AfxGetProgramFilesFolder();   // no trailing backslash
    std::wstring data_files_folder = exe_folder;   // default data files to same location as EXE
    
    if (AfxGetExePath().substr(0, program_files_folder.length()) == program_files_folder) {
        // Program is installed in the Program Files folder so we need to 
        // set the data folder to the AppData Local folder.
        data_files_folder = AfxGetLocalAppDataFolder() + L"\\TradeTracker";
        // Create the data files folder in Local App Data should it not exist.
        AfxCreateNestedFolder(data_files_folder);
    }

    return data_files_folder;
}

void CConfig::Version4UpgradeExe() {
    exe_new = AfxGetExePath() + exe_new;

    // Delete pre-Version4 exe
    if (AfxFileExists(exe_new) && AfxFileExists(exe_old)) {
        DeleteFile(exe_old.c_str());
    }
}

bool CConfig::Version4UpgradeConfig() {
    std::wstring dbConfig_filename = GetDataFilesFolder() + dbConfig_new;

    // If version 4 filenames already exist then we would have already upgraded the
    // files previously,
    if (AfxFileExists(dbConfig_filename) || !AfxFileExists(dbConfig_old)) {
        dbConfig = dbConfig_filename;
        return false;
    }
    else {
        // Old files will be renamed after they are first loaded into memory.
        if (AfxFileExists(dbConfig_old)) {
            dbConfig = dbConfig_old;
        }
        return true;
    }
}


// ========================================================================================
// Determine the Costing Method to be used.
// ========================================================================================
CostingMethod CConfig::GetCostingMethod() {
    return costing_method;
}

void CConfig::SetCostingMethod(CostingMethod value) {
    costing_method = value;
}


// ========================================================================================
// Determine the NumberFormatting to be used.
// ========================================================================================
NumberFormatType CConfig::GetNumberFormatType() {
    return number_format_type;
}

void CConfig::SetNumberFormatType(NumberFormatType value) {
    number_format_type = value;
}


// ========================================================================================
// Determine the week starting day (Sunday or Monday).
// ========================================================================================
StartWeekdayType CConfig::GetStartWeekday() {
    return start_weekday;
}

void CConfig::SetStartWeekday(StartWeekdayType value) {
    start_weekday = value;
}


// ========================================================================================
// Determine if show/hide the Portfolio dollar value on the main screen.
// ========================================================================================
bool CConfig::GetAllowPortfolioDisplay() {
    return show_portfolio_value;
}

void CConfig::SetAllowPortfolioDisplay(bool value) {
    show_portfolio_value = value;
    ActiveTrades.ShowHideLiquidityLabels();
}


// ========================================================================================
// Determine if should display the MIT open source license.
// ========================================================================================
bool CConfig::GetDisplayLicense() {
    return display_open_source_license;
}
void CConfig::SetDisplayLicense(bool value) {
    display_open_source_license = value;
}

void CConfig::DisplayLicense() {
    std::wstring license_text = L"  \
\r\n \
 MIT License\r\n  \
\r\n \
 Copyright(c) 2023 - 2024 Paul Squires\r\n  \
\r\n  \
Permission is hereby granted, free of charge, to any person obtaining a copy\r\n  \
of this software and associated documentation files(the 'Software'), to deal\r\n  \
in the Software without restriction, including without limitation the rights\r\n  \
to use, copy, modify, merge, publish, distribute, sublicense, and /or sell\r\n  \
copies of the Software, and to permit persons to whom the Software is\r\n  \
furnished to do so, subject to the following conditions :\r\n  \
\r\n  \
The above copyright notice and this permission notice shall be included in all\r\n  \
copies or substantial portions of the Software.\r\n  \
\r\n  \
\
THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\r\n  \
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\r\n  \
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE\r\n  \
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\r\n  \
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\r\n  \
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\r\n  \
SOFTWARE.\r\n  \
";

    TextBoxDialog_Show(MainWindow.hWindow, L"MIT License", license_text, 680, 390);
    SetDisplayLicense(false);
}


// ========================================================================================
// Determine if allow to check for available program update.
// ========================================================================================
bool CConfig::GetAllowUpdateCheck() {
    return allow_update_check;
}
void CConfig::SetAllowUpdateCheck(bool value) {
    allow_update_check = value;
}


// ========================================================================================
// Return the application's startup width value as returned from the Config file.
// If no value exits then return default value.
// ========================================================================================
int CConfig::GetStartupWidth() {
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
int CConfig::GetStartupHeight() {
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
void CConfig::SetStartupHeight(int height) {
    startup_height = AfxUnScaleY(height);
}


// ========================================================================================
// Set the application's startup width value.
// ========================================================================================
void CConfig::SetStartupWidth(int width) {
    startup_width = AfxUnScaleX(width);
}


// ========================================================================================
// Return the application's Right Panel startup width value as returned from the Config file.
// If no value exits then return default value.
// ========================================================================================
int CConfig::GetStartupRightPanelWidth() {
    if (startup_right_panel_width > 0) return startup_right_panel_width;
    return 440;
}


// ========================================================================================
// Set the application's Right Panel startup width value.
// ========================================================================================
void CConfig::SetStartupRightPanelWidth(int width) {
    startup_right_panel_width = AfxUnScaleX(width);
}


// ========================================================================================
// Determine if the incoming ticker symbol is a Future.
// ========================================================================================
bool CConfig::IsFuturesTicker(const std::wstring& ticker) {
    return (ticker.substr(0, 1) == L"/");
}


// ========================================================================================
// Get the Ticker Decimals for the incoming underlying.
// ========================================================================================
int CConfig::GetTickerDecimals(const std::wstring& underlying) {
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
void CConfig::SetTickerDecimals(const std::wstring& underlying, int decimals) {
    mapTickerDecimals[underlying] = decimals;
}


// ========================================================================================
// Get the Futures Multiplier for the incoming underlying.
// ========================================================================================
std::wstring CConfig::GetMultiplier(const std::wstring& underlying) {
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
void CConfig::SetMultiplier(const std::wstring& underlying, const std::wstring& multiplier) {
    mapMultipliers[underlying] = multiplier;
}


// ========================================================================================
// Get the Futures Exchanges for the incoming underlying.
// ========================================================================================
std::string CConfig::GetFuturesExchange(const std::string& underlying) {
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
void CConfig::SetFuturesExchange(const std::string& underlying, const std::string& exchange) {
    mapFuturesExchanges[underlying] = exchange;
}


// ========================================================================================
// Get the Category Description for the incoming category number.
// ========================================================================================
std::wstring CConfig::GetCategoryDescription(int index) {
    return mapCategoryDescriptions.at(index);
}


// ========================================================================================
// Set the Category Description for the incoming category number.
// ========================================================================================
void CConfig::SetCategoryDescription(int index, const std::wstring& description) {
    mapCategoryDescriptions[index] = description;
}


// ========================================================================================
// Save the Config values to a simple text file.
// ========================================================================================
bool CConfig::SaveConfig() {
    std::wofstream db;

    db.open(dbConfig, std::ios::out | std::ios::trunc);

    if (!db.is_open()) {
        CustomMessageBox.Show(
            NULL,
            L"Could not save configuration file",
            L"Warning",
            MB_ICONWARNING
        );
        return false;
    }

    db << "STARTWEEKDAY|" << (start_weekday == StartWeekdayType::Sunday? L"Sunday" : L"Monday") << "\n";

    db << "NUMBERFORMAT|" << (number_format_type == NumberFormatType::European ? L"European" : L"American") << "\n";

    db << "COSTINGMETHOD|" << (costing_method == CostingMethod::fifo ? L"fifo" : L"AverageCost") << "\n";
        
    db << "SHOWPORTFOLIOVALUE|" << (show_portfolio_value ? L"true" : L"false") << "\n";
    
    db << "ALLOWUPDATECHECK|" << (allow_update_check ? L"true" : L"false") << "\n";
   
    db << "DISPLAYLICENSE|" << (display_open_source_license ? L"true" : L"false") << "\n";

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
bool CConfig::LoadConfig() {
    Version4UpgradeExe();

    bool upgrade_to_version4 = Version4UpgradeConfig();

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
    std::wstring line;

    while (!db.eof()) {
        std::getline(db, line);

        if (line.length() == 0) continue;

        // If this is a Comment line then simply iterate to next line.
        if (line.compare(1, 3, L"// ") == 0) continue;

        // Tokenize the line into a vector based on the pipe delimiter
        std::vector<std::wstring> st = AfxSplit(line, L'|');

        if (st.empty()) continue;

        std::wstring arg = AfxTrim(st.at(0));

        // Determine the Starting day of th week
        if (arg == L"STARTWEEKDAY") {
            std::wstring value;
            
            try {value = AfxTrim(st.at(1)); }
            catch (...) { continue; }
            start_weekday = StartWeekdayType::Sunday;
            if (AfxWStringCompareI(value, L"Monday")) start_weekday = StartWeekdayType::Monday;
            continue;
        }

        // Determine the Number Format to use
        if (arg == L"NUMBERFORMAT") {
            std::wstring value;
            
            try {value = AfxTrim(st.at(1)); }
            catch (...) { continue; }
            number_format_type = NumberFormatType::American;
            if (AfxWStringCompareI(value, L"European")) number_format_type = NumberFormatType::European;
            continue;
        }

        // Determine the Costing Method to use for stocks
        if (arg == L"COSTINGMETHOD") {
            std::wstring value;
            
            try {value = AfxTrim(st.at(1)); }
            catch (...) { continue; }
            costing_method = CostingMethod::AverageCost;
            if (AfxWStringCompareI(value, L"fifo")) costing_method = CostingMethod::fifo;
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

        // Check if should allow checking for available program update
        if (arg == L"DISPLAYLICENSE") {
            std::wstring value;
            
            try {value = AfxTrim(st.at(1)); }
            catch (...) { continue; }
        
            display_open_source_license = AfxWStringCompareI(value, L"true");
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
            int category_index{0};
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
            int decimals{0};
            
            try {underlying = st.at(1);}
            catch (...) {continue;}
            
            try { decimals = stoi(st.at(2));}
            catch (...) {continue;}
            
            SetTickerDecimals(underlying, decimals);

            continue;
        }

    }

    db.close();

    if (upgrade_to_version4) {
        std::cout << "Upgrade config to version 4" << std::endl;
        dbConfig = GetDataFilesFolder() + dbConfig_new;
        SaveConfig();
        // Delete the older version file
        DeleteFile(dbConfig_old.c_str());
    }

    return true;
}
