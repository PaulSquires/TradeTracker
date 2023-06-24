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

extern HWND HWND_SIDEMENU;


std::unordered_map<int, std::wstring> mapCategoryDescriptions {
    { 0, L"Gray"},
    { 1, L"Blue" },
    { 2, L"Pink" },
    { 3, L"Green" },
    { 4, L"Orange" }
};

const int CATEGORY_COUNT = 5;


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
    mapCategoryDescriptions.at(idxCategory) = wszDescription;
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

    for (int i = 0; i < CATEGORY_COUNT; ++i) {
        db << "CATEGORY|" << i << "|" << GetCategoryDescription(i) << "\n";
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

    }

    db.close();

    return true;
}
