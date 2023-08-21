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

#include "SideMenu/SideMenu.h"
#include "CustomLabel/CustomLabel.h"

#include "Config.h"


const std::wstring dbConfig = AfxGetExePath() + L"\\IB-Tracker-config.txt";

const std::wstring idMagic = L"IB-TRACKER-CONFIG";

bool StartupConnect = false;
int StartupWidth = 0;
int StartupHeight = 0;
int StartupRightPanelWidth = 0;


std::unordered_map<int, std::wstring> mapCategoryDescriptions {
    { 0, L"Category 0"},
    { 1, L"Category 1" },
    { 2, L"Category 2" },
    { 3, L"Category 3" },
    { 4, L"Category 4" },
    { 5, L"Category 5" },
    { 6, L"Category 6" },
    { 7, L"Category 7" },
    {99, L"All Categories" }
};

std::unordered_map<std::string, std::string> mapFuturesExchanges {
    { "/AUD", "CME" },
    { "/ES",  "CME" },
    { "/MES", "CME" }, 
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
    { L"/ES",  L"50" },
    { L"/MES", L"5" },
    { L"/CL",  L"1000" },
    { L"/ZB",  L"1000" }
};

std::unordered_map<std::wstring, int> mapTickerDecimals {
    { L"/AUD", 5 },
    { L"/ZB", 3 },
    { L"/ZC", 3 },
    { L"/ZS", 4 }
};



// ========================================================================================
// Return the application's startup width value as returned from the Config file.
// If no value exits then return default value.
// ========================================================================================
int GetStartupWidth()
{
    if (StartupWidth > 0) return StartupWidth;

    // Size the main window to encompass 75% of screen width.
    int InitalMainWidth = AfxUnScaleX(AfxGetWorkAreaWidth() * 0.75f);

    // Impose a maximum size on the initial height/width in order
    // to ensure that the main window does not display exceptionally
    // large especially when run on very large monitors.
    // Target a minimum 720p screen resolution size (1280 x 720).
    if (InitalMainWidth > 1280) InitalMainWidth = 1280;

    return InitalMainWidth;
}


// ========================================================================================
// Return the application's startup height value as returned from the Config file.
// If no value exits then return default value.
// ========================================================================================
int GetStartupHeight()
{
    if (StartupHeight > 0) return StartupHeight;

    // Size the main window to encompass 85% of screen height.
    int InitalMainHeight = AfxUnScaleY(AfxGetWorkAreaHeight() * 0.85f);

    // Impose a maximum size on the initial height/width in order
    // to ensure that the main window does not display exceptionally
    // large especially when run on very large monitors.
    // Target a minimum 720p screen resolution size (1280 x 720).
    if (InitalMainHeight > 720) InitalMainHeight = 720;

    return InitalMainHeight;
}


// ========================================================================================
// Set the application's startup height value.
// ========================================================================================
void SetStartupHeight(int height)
{
    StartupHeight = AfxUnScaleY((float)height);
}


// ========================================================================================
// Set the application's startup width value.
// ========================================================================================
void SetStartupWidth(int width)
{
    StartupWidth = AfxUnScaleX((float)width);
}


// ========================================================================================
// Return the application's Right Panel startup width value as returned from the Config file.
// If no value exits then return default value.
// ========================================================================================
int GetStartupRightPanelWidth()
{
    if (StartupRightPanelWidth > 0) return StartupRightPanelWidth;
    return 440;
}


// ========================================================================================
// Set the application's Right Panel startup width value.
// ========================================================================================
void SetStartupRightPanelWidth(int width)
{
    StartupRightPanelWidth = AfxUnScaleX((float)width);
}


// ========================================================================================
// Determine if the incoming ticker symbol is a Future.
// ========================================================================================
bool IsFuturesTicker(const std::wstring& wszTicker)
{
    return (wszTicker.substr(0, 1) == L"/");
}



// ========================================================================================
// Get the Ticker Decimals for the incoming underlying.
// ========================================================================================
int GetTickerDecimals(std::wstring wszUnderlying)
{
    if (mapTickerDecimals.count(wszUnderlying)) {
        return mapTickerDecimals.at(wszUnderlying);
    }
    else {
        return 2;
    }
}


// ========================================================================================
// Set the Ticker Decimals for the incoming underlying.
// ========================================================================================
void SetTickerDecimals(std::wstring wszUnderlying, int numDecimals)
{
    mapTickerDecimals[wszUnderlying] = numDecimals;
}


// ========================================================================================
// Get the Futures Multiplier for the incoming underlying.
// ========================================================================================
std::wstring GetMultiplier(std::wstring wszUnderlying)
{
    if (mapMultipliers.count(wszUnderlying)) {
        return mapMultipliers.at(wszUnderlying);
    }
    else {
        return L"100";
    }
}


// ========================================================================================
// Set the Futures Multiplier for the incoming underlying.
// ========================================================================================
void SetMultiplier(std::wstring wszUnderlying, std::wstring wszMultiplier)
{
    mapMultipliers[wszUnderlying] = wszMultiplier;
}


// ========================================================================================
// Get the Futures Exchanges for the incoming underlying.
// ========================================================================================
std::string GetFuturesExchange(std::string szUnderlying)
{
    if (mapFuturesExchanges.count(szUnderlying)) {
        return mapFuturesExchanges.at(szUnderlying);
    }
    else {
        return "CME";
    }
}


// ========================================================================================
// Set the Futures Exchanges for the incoming underlying.
// ========================================================================================
void SetFuturesExchange(std::string szUnderlying, std::string szExchange)
{
    mapFuturesExchanges[szUnderlying] = szExchange;
}


// ========================================================================================
// Get the Category Description for the incoming category number.
// ========================================================================================
std::wstring GetCategoryDescription(int idxCategory)
{
    return mapCategoryDescriptions.at(idxCategory);
}


// ========================================================================================
// Set the Category Description for the incoming category number.
// ========================================================================================
void SetCategoryDescription(int idxCategory, std::wstring wszDescription)
{
    mapCategoryDescriptions[idxCategory] = wszDescription;
}


// ========================================================================================
// Get the true/false to try to automatically connect on program startup.
// ========================================================================================
bool GetStartupConnect()
{
    return StartupConnect;
}


// ========================================================================================
// Get the true/false to try to automatically connect on program startup.
// ========================================================================================
void SetStartupConnect(bool bConnect)
{
    StartupConnect = bConnect;
}


// ========================================================================================
// Save the Config values to a simple text file.
// ========================================================================================
bool SaveConfig()
{
    std::wofstream db;

    db.open(dbConfig, std::ios::out | std::ios::trunc);

    if (!db.is_open()) {
        int msgboxID = MessageBox(
            NULL,
            (LPCWSTR)(L"Could not save configuration file"),
            (LPCWSTR)L"Warning",
            MB_ICONWARNING
        );
        return false;
    }


    db << idMagic << "|" << version << "\n"
        << "STARTUPCONNECT|" << (GetStartupConnect() ? L"true" : L"false") << "\n";

    db << "STARTUPWIDTH" << "|" << StartupWidth << "\n";

    db << "STARTUPHEIGHT" << "|" << StartupHeight << "\n";

    db << "STARTUPRIGHTPANELWIDTH" << "|" << StartupRightPanelWidth << "\n";

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

    if (!db.is_open())
        return false;

    std::wstring configVersion;

    bool isFirstline = true;

    std::wstring line;

    while (!db.eof()) {
        std::getline(db, line);

        if (line.length() == 0) continue;

        // If this is a Comment line then simply iterate to next line.
        if (line.compare(1, 3, L"// ") == 0) continue;

        // Tokenize the line into a vector based on the pipe delimiter
        std::vector<std::wstring> st = AfxSplit(line, L"|");

        if (st.empty()) continue;

        // First line must be the database identifier and version
        if (isFirstline) {
            if (st.at(0) != idMagic) {
                int msgboxID = MessageBox(
                    NULL,
                    (LPCWSTR)(L"Invalid configuration file"),
                    (LPCWSTR)L"Warning",
                    MB_ICONWARNING
                );
                db.close();
                return false;
            }
            configVersion = st.at(1);
            isFirstline = false;
            continue;
        }


        std::wstring arg = AfxTrim(st.at(0));


        // Check for configuration identifiers
        if (arg == L"STARTUPCONNECT") {
            std::wstring wszConnect;
            
            try {wszConnect = AfxTrim(st.at(1)); }
            catch (...) { continue; }
        
            bool bConnect = AfxWStringCompareI(wszConnect, L"true");
            SetStartupConnect(bConnect);
            continue;
        }


        // Check for StartupWidth
        if (arg == L"STARTUPWIDTH") {
            std::wstring wszWidth;

            try { wszWidth = AfxTrim(st.at(1)); }
            catch (...) { continue; }

            StartupWidth = AfxValInteger(wszWidth);
            continue;
        }


        // Check for StartupHeight
        if (arg == L"STARTUPHEIGHT") {
            std::wstring wszHeight;

            try { wszHeight = AfxTrim(st.at(1)); }
            catch (...) { continue; }

            StartupHeight = AfxValInteger(wszHeight);
            continue;
        }


        // Check for category descriptions
        if (arg == L"CATEGORY") {
            int idxCategory;
            std::wstring wszDescription;
            
            try {idxCategory = stoi(st.at(1));}
            catch (...) {continue;}
            
            try {wszDescription = st.at(2);}
            catch (...) {continue;}
            
            SetCategoryDescription(idxCategory, wszDescription);
            continue;
        }


        // Check for StartupRightPanelWidth
        if (arg == L"STARTUPRIGHTPANELWIDTH") {
            std::wstring wszWidth;

            try { wszWidth = AfxTrim(st.at(1)); }
            catch (...) { continue; }

            StartupRightPanelWidth = AfxValInteger(wszWidth);
            continue;
        }


        // Check for Futures Exchanges
        if (arg == L"EXCHANGE") {
            std::wstring wszUnderlying;
            std::wstring wszExchange;
            
            try {wszUnderlying = st.at(1);}
            catch (...) {continue;}
            
            try {wszExchange = st.at(2);}
            catch (...) {continue;}
            
            std::string szUnderlying = unicode2ansi(wszUnderlying);
            std::string szExchange = unicode2ansi(wszExchange);
            SetFuturesExchange(szUnderlying, szExchange);

            continue;
        }


        // Check for Multipliers
        if (arg == L"MULTIPLIER") {
            std::wstring wszUnderlying;
            std::wstring wszMultiplier;
            
            try {wszUnderlying = st.at(1);}
            catch (...) {continue;}
            
            try { wszMultiplier = st.at(2);}
            catch (...) {continue;}
            
            SetMultiplier(wszUnderlying, wszMultiplier);

            continue;
        }


        // Check for Ticker Decimals
        if (arg == L"TICKERDECIMALS") {
            std::wstring wszUnderlying;
            int numDecimals;
            
            try {wszUnderlying = st.at(1);}
            catch (...) {continue;}
            
            try { numDecimals = stoi(st.at(2));}
            catch (...) {continue;}
            
            SetTickerDecimals(wszUnderlying, numDecimals);

            continue;
        }

    }

    db.close();

    return true;
}
