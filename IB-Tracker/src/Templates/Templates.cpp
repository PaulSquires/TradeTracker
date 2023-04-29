#include "pch.h"

#include "..\Utilities\AfxWin.h"

#include "Templates.h"


const std::wstring dbTemplates = AfxGetExePath() + L"\\IB-Tracker-templates.txt";
const std::wstring dbTemplatesUser = AfxGetExePath() + L"\\IB-Tracker-templates-user.txt";


std::vector<CTradeTemplate> TradeTemplates;



// ========================================================================================
// Load the Trade Templates from file.
// ========================================================================================
bool LoadTemplates_Internal(const std::wstring& dbFilename)
{
    std::wifstream db;

    db.open(dbFilename, std::ios::in);

    if (!db.is_open())
        return false;

    std::wstring line;
    bool readingTemplate = false;


    CTradeTemplate ctrade;


    while (!db.eof()) {
        std::getline(db, line);

        if (line.length() == 0) continue;

        // If this is a Comment line then simply iterate to next line.
        if (line.compare(1, 3, L"// ") == 0) continue;

        // Tokenize the line into a vector based on the colon delimiter
        std::vector<std::wstring> st = AfxSplit(line, L":");

        if (st.empty()) continue;

        std::wstring arg = AfxTrim(st.at(0));


        if (arg == L"TEMPLATE") {
            // If already reading a template then commit the current one to the
            // vector and start a new one.
            if (readingTemplate) {
                TradeTemplates.push_back(ctrade);
                
                ctrade.name.clear();
                ctrade.legs.clear();
                ctrade.menu = false;
            }
            readingTemplate = true;
            continue;
        }

    
        if (arg == L"NAME") {
            if (!readingTemplate) continue;
            ctrade.name = AfxTrim(st.at(1));
            // Also default the description to be the same as the name in the event
            // that the user does not specify a description.
            ctrade.description = ctrade.name;
            continue;
        }

        
        if (arg == L"DESCRIPTION") {
            if (!readingTemplate) continue;
            ctrade.description = AfxTrim(st.at(1));
            continue;
        }


        if (arg == L"TICKER") {
            if (!readingTemplate) continue;
            ctrade.ticker = AfxTrim(st.at(1));
            continue;
        }


        if (arg == L"COMPANY") {
            if (!readingTemplate) continue;
            ctrade.company = AfxTrim(st.at(1));
            continue;
        }


        if (arg == L"MAINMENU") {
            if (!readingTemplate) continue;
            ctrade.menu = AfxWStringCompareI(AfxTrim(st.at(1)), L"true");
            continue;
        }


        if (arg == L"MULTIPLIER") {
            if (!readingTemplate) continue;
            ctrade.multiplier = AfxTrim(st.at(1));
            continue;
        }


        if (arg == L"LEG") {
            if (!readingTemplate) continue;
            // Tokenize the leg attributes into a vector based on the comma delimiter
            std::vector<std::wstring> attr = AfxSplit(st.at(1), L",");

            CTradeTemplateLeg leg;
            leg.quantity = AfxTrim(attr.at(0));
            leg.PutCall = AfxTrim(attr.at(1));
            leg.action = AfxTrim(attr.at(2));

            ctrade.legs.push_back(leg);
            continue;
        }


    }

    if (readingTemplate) {
        TradeTemplates.push_back(ctrade);
    }

    db.close();

    return true;
}



// ========================================================================================
// Load the Trade Templates (pre-defined and user) from files.
// ========================================================================================
bool LoadTemplates()
{
    LoadTemplates_Internal(dbTemplates);
    LoadTemplates_Internal(dbTemplatesUser);
    return true;
}

