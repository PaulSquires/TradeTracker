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

#include "utilities.h"

#include "appstate.h"
#include "messagebox.h"

#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include "icons_material_design.h"


CConfig::CConfig() {
    dbConfig = GetDataFilesFolder() + "/tt-config.txt";
}


// ========================================================================================
// Return the application's startup width value as returned from the Config file.
// If no value exits then return default value.
// ========================================================================================
int CConfig::GetStartupWidth(AppState& state) {
    // Size the main window to encompass 75% of screen width.
    int inital_main_width = state.undpi(state.display_width * 0.75f);

    // Impose a maximum size on the initial height/width in order
    // to ensure that the main window does not display exceptionally
    // large especially when run on very large monitors.
    // Target a minimum 1200 x 800p screen resolution size.
    if (inital_main_width > 1200) inital_main_width = 1200;

    // Get the previously saved Startup Width and restrict its maximum
    // to the size of the width of the work area.
   if (startup_width == 0 ||
       startup_width > state.undpi(state.display_width))
       startup_width = inital_main_width;
    
    return startup_width;
}


// ========================================================================================
// Return the application's startup height value as returned from the Config file.
// If no value exits then return default value.
// ========================================================================================
int CConfig::GetStartupHeight(AppState& state) {
    // Size the main window to encompass 85% of screen height.
    int inital_main_height = state.undpi(state.display_height * 0.85f);

    // Impose a maximum size on the initial height/width in order
    // to ensure that the main window does not display exceptionally
    // large especially when run on very large monitors.
    // Target a minimum 1200 x 800p screen resolution size.
   if (inital_main_height > 800) inital_main_height = 800;

    // Get the previously saved Startup Height and restrict its maximum
    // to the size of the height of the work area.
   if (startup_height == 0 ||
       startup_height > state.undpi(state.display_height))
       startup_height = inital_main_height;

    return startup_height;
}


// ========================================================================================
// Set the application's startup height value.
// ========================================================================================
void CConfig::SetStartupHeight(AppState& state, int height) {
   startup_height = state.undpi(height);
}


// ========================================================================================
// Set the application's startup width value.
// ========================================================================================
void CConfig::SetStartupWidth(AppState& state, int width) {
   startup_width = state.undpi(width);
}


// ========================================================================================
// Return the application's Right Panel startup width value as returned from the Config file.
// If no value exits then return default value.
// ========================================================================================
int CConfig::GetStartupRightPanelWidth(AppState& state) {
    if (startup_right_panel_width > 0) return state.dpi(startup_right_panel_width);
    return state.dpi(440);
}


// ========================================================================================
// Set the application's Right Panel startup width value.
// ========================================================================================
void CConfig::SetStartupRightPanelWidth(AppState& state, int width) {
   startup_right_panel_width = state.undpi(width);
}


// ========================================================================================
// Determine if the incoming ticker symbol is a Future.
// ========================================================================================
bool CConfig::IsFuturesTicker(const std::string& ticker) {
    return (ticker.substr(0, 1) == "/");
}


// ========================================================================================
// Determine if the incoming ticker symbol is an Index.
// ========================================================================================
bool CConfig::IsIndexTicker(const std::string& ticker) {
    if (mapIndexTickers.count(ticker)) return true;
    return false;
}


// ========================================================================================
// Set the Ticker Decimals for the incoming underlying.
// ========================================================================================
void CConfig::SetIndexTicker(const std::string& ticker) {
    mapIndexTickers[ticker] = true;
}


// ========================================================================================
// Get the Ticker Decimals for the incoming underlying.
// ========================================================================================
int CConfig::GetTickerDecimals(const std::string& underlying) {
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
void CConfig::SetTickerDecimals(const std::string& underlying, int decimals) {
    mapTickerDecimals[underlying] = decimals;
}


// ========================================================================================
// Get the Futures Multiplier for the incoming underlying.
// ========================================================================================
std::string CConfig::GetMultiplier(const std::string& underlying) {
    if (mapMultipliers.count(underlying)) {
        return mapMultipliers.at(underlying);
    }
    else {
        return "100";
    }
}


// ========================================================================================
// Set the Futures Multiplier for the incoming underlying.
// ========================================================================================
void CConfig::SetMultiplier(const std::string& underlying, const std::string& multiplier) {
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
std::string CConfig::GetCategoryDescription(int index) {
    return mapCategoryDescriptions.at(index);
}


// ========================================================================================
// Set the Category Description for the incoming category number.
// ========================================================================================
void CConfig::SetCategoryDescription(int index, const std::string& description) {
    mapCategoryDescriptions[index] = description;
}


// ========================================================================================
// Save the Config values to a simple text file.
// ========================================================================================
bool CConfig::SaveConfig(AppState& state) {
    std::ofstream db(dbConfig);
    if (!db) {
        CustomMessageBox(state, "Warning", "Could not save configuration file.");
        return false;
    }

    std::ostringstream text;

    text << "COLORTHEME|" << (color_theme == ColorThemeType::Dark ? "Dark" : "Light") << "\n";
    
    text << "STARTWEEKDAY|" << (start_weekday == StartWeekdayType::Sunday ? "Sunday" : "Monday") << "\n";

    text << "NUMBERFORMAT|" << (number_format_type == NumberFormatType::European ? "European" : "American") << "\n";

    text << "COSTINGMETHOD|" << (costing_method == CostingMethodType::fifo ? "fifo" : "AverageCost") << "\n";
    
    text << "EXCLUDENONSTOCKCOSTS|" << (exclude_nonstock_costs ? "true" : "false") << "\n";
        
    text << "SHOWPORTFOLIOVALUE|" << (show_portfolio_value ? "true" : "false") << "\n";

    text << "SHOW45TRADEDATE|" << (show_45day_trade_date ? "true" : "false") << "\n";
    
    text << "ALLOWUPDATECHECK|" << (allow_update_check ? "true" : "false") << "\n";
   
    text << "DISPLAYLICENSE|" << (display_open_source_license ? "true" : "false") << "\n";

    text << "STARTUPWIDTH" << "|" << startup_width << "\n";

    text << "STARTUPHEIGHT" << "|" << startup_height << "\n";

    text << "STARTUPRIGHTPANELWIDTH" << "|" << startup_right_panel_width << "\n";
    
    text << "GUIFONTSIZE" << "|" << AfxDoubleToString(font_size, 0) << "\n";

    for (auto item : mapCategoryDescriptions) {
        text << "CATEGORY|" << item.first << "|" << item.second << "\n";
    }

    for (auto item : mapFuturesExchanges) {
        text << "EXCHANGE|" << std::string(item.first) << "|" << std::string(item.second) << "\n";
    }

    for (auto item : mapMultipliers) {
        text << "MULTIPLIER|" << item.first << "|" << item.second << "\n";
    }

    for (auto item : mapTickerDecimals) {
        text << "TICKERDECIMALS|" << item.first << "|" << item.second << "\n";
    }

    for (auto item : mapIndexTickers) {
        text << "INDEXTICKER|" << item.first << "\n";
    }

    db << text.str();
    db.close();

    return true;
}


// ========================================================================================
// Load the Config values from file.
// ========================================================================================
bool CConfig::LoadConfig(AppState& state) {

    // If config file does not exist then simply exit because default config values
    // will be used and then saved to disk.
    if (!AfxFileExists(dbConfig)) return true;

    std::ifstream db(dbConfig);
    if (!db) {
        CustomMessageBox(state, "Warning", "Could not load configuration file.");
        return false;
    }

    std::string config_version;
    std::string line;

    while (getline(db, line)) {
        // Trim leading and trailing white space
        line = AfxTrim(line);

        if (line.length() == 0) continue;

        // If this is a Comment line then simply iterate to next line.
        if (line.compare(1, 3, "// ") == 0) continue;

        // Tokenize the line into a vector based on the pipe delimiter
        std::vector<std::string> st = AfxSplit(line, L'|');

        if (st.empty()) continue;

        std::string arg = AfxTrim(st.at(0));

        // Determine the Starting day of th week
        if (arg == "STARTWEEKDAY") {
            std::string value;
            
            try {value = AfxTrim(st.at(1)); }
            catch (...) { continue; }
            start_weekday = StartWeekdayType::Sunday;
            if (value == "Monday") start_weekday = StartWeekdayType::Monday;
            continue;
        }

        // Determine the Number Format to use
        if (arg == "NUMBERFORMAT") {
            std::string value;
            
            try {value = AfxTrim(st.at(1)); }
            catch (...) { continue; }
            number_format_type = NumberFormatType::American;
            if (value == "European") number_format_type = NumberFormatType::European;
            continue;
        }

        // Determine the Color Theme
        if (arg == "COLORTHEME") {
            std::string value;
            
            try {value = AfxTrim(st.at(1)); }
            catch (...) { continue; }
            color_theme = ColorThemeType::Dark;
            if (value == "Light") color_theme = ColorThemeType::Light;
            continue;
        }

        // Determine the Costing Method to use for stocks
        if (arg == "COSTINGMETHOD") {
            std::string value;
            
            try {value = AfxTrim(st.at(1)); }
            catch (...) { continue; }
            costing_method = CostingMethodType::AverageCost;
            if (value == "fifo") costing_method = CostingMethodType::fifo;
            continue;
        }

        // Check if should include/exclude non stock costs in cost calculation (e.g dividends)
        if (arg == "EXCLUDENONSTOCKCOSTS") {
            std::string value;
            
            try {value = AfxTrim(st.at(1)); }
            catch (...) { continue; }
        
            exclude_nonstock_costs = (value == "true") ? true : false;
            continue;
        }

        // Check if should show/hide Total Portfolio dollar amount on main screen
        if (arg == "SHOWPORTFOLIOVALUE") {
            std::string value;
            
            try {value = AfxTrim(st.at(1)); }
            catch (...) { continue; }
        
            show_portfolio_value = (value == "true") ? true : false;
            continue;
        }

        // Check if should show/hide the closest 45 day expiration to place trades (on main screen)
        if (arg == "SHOW45TRADEDATE") {
            std::string value;
            
            try {value = AfxTrim(st.at(1)); }
            catch (...) { continue; }
        
            show_45day_trade_date = (value == "true") ? true : false;
            continue;
        }

        // Check if should allow checking for available program update
        if (arg == "ALLOWUPDATECHECK") {
            std::string value;
            
            try {value = AfxTrim(st.at(1)); }
            catch (...) { continue; }
        
            allow_update_check = (value == "true") ? true : false;
            continue;
        }

        // Check if should allow checking for available program update
        if (arg == "DISPLAYLICENSE") {
            std::string value;
            
            try {value = AfxTrim(st.at(1)); }
            catch (...) { continue; }
        
            display_open_source_license = (value == "true") ? true : false;
            continue;
        }

        // Check for startup_width
        if (arg == "STARTUPWIDTH") {
            std::string width;

            try { width = AfxTrim(st.at(1)); }
            catch (...) { continue; }

            startup_width = AfxValInteger(width);
            continue;
        }

        // Check for startup_height
        if (arg == "STARTUPHEIGHT") {
            std::string height;

            try { height = AfxTrim(st.at(1)); }
            catch (...) { continue; }

            startup_height = AfxValInteger(height);
            continue;
        }

        // Get the main GUI font size
        if (arg == "GUIFONTSIZE") {
            std::string value;

            try { value = AfxTrim(st.at(1)); }
            catch (...) { continue; }

            font_size = AfxValDouble(value);
            continue;
        }

        // Check for category descriptions
        if (arg == "CATEGORY") {
            int category_index{0};
            std::string description;

            try {
                category_index = AfxValInteger(st.at(1));
            }
            catch (...) {continue;}
            
            try {description = st.at(2);}
            catch (...) {continue;}
            SetCategoryDescription(category_index, description);
            continue;
        }

        // Check for startup_right_panel_width
        if (arg == "STARTUPRIGHTPANELWIDTH") {
            std::string width;

            try { width = AfxTrim(st.at(1)); }
            catch (...) { continue; }

            startup_right_panel_width = AfxValInteger(width);
            continue;
        }

        // Check for Futures Exchanges
        if (arg == "EXCHANGE") {
            std::string text1;
            std::string text2;
            
            try {text1 = st.at(1);}
            catch (...) {continue;}
            
            try {text2 = st.at(2);}
            catch (...) {continue;}
            
            // underlying = text1
            // exchange = text2
            SetFuturesExchange(text1, text2);

            continue;
        }

        // Check for Multipliers
        if (arg == "MULTIPLIER") {
            std::string underlying;
            std::string multiplier;
            
            try {underlying = st.at(1);}
            catch (...) {continue;}
            
            try { multiplier = st.at(2);}
            catch (...) {continue;}
            
            SetMultiplier(underlying, multiplier);

            continue;
        }

        // Check for Ticker Decimals
        if (arg == "TICKERDECIMALS") {
            std::string underlying;
            std::string temp;

            int decimals{0};
            
            try {underlying = st.at(1);}
            catch (...) {continue;}
            
            try { 
                temp = AfxTrim(st.at(2));
                decimals = AfxValInteger(temp);
            }
            catch (...) {continue;}
            
            SetTickerDecimals(underlying, decimals);

            continue;
        }

        // Check for Index Ticker Symbols
        if (arg == "INDEXTICKER") {
            std::string underlying;

            try {underlying = st.at(1);}
            catch (...) {continue;}
            
            SetIndexTicker(underlying);

            continue;
        }

    }

    db.close();

    return true;
}


// ========================================================================================
// Load the Config values from file.
// ========================================================================================
void CConfig::CreateAppFonts(AppState& state) {
        ImFont* gui_font = nullptr;
        ImFont* gui_font_mono = nullptr;

        // Add custom fonts 
        float baseFontSize = state.config.font_size; 
        float iconFontSize = state.config.font_size;

        // Load the embedded font
        //io.Fonts->AddFontFromMemoryCompressedTTF(segoeui_compressed_data, segoeui_compressed_size, baseFontSize * dpi_scale);

        // Merging two memory compressed fonts does not seem to work so load the fonts from disk instead.
        // ImFont* font = io.Fonts->AddFontFromMemoryCompressedTTF(materialicons_compressed_data, materialicons_compressed_size, baseFontSize * dpi_scale);
        // if (font == NULL) {
        //     std::cerr << "Failed to load embedded font" << std::endl;
        // }

        ImGuiIO& io = ImGui::GetIO();

        std::string fontfile = AfxGetExePath() + "/gui-font.ttf";
        gui_font = io.Fonts->AddFontFromFileTTF(fontfile.c_str(), state.dpi(baseFontSize));

        // Define and merge into the base font the Unicode ranges for Material Design Icons
        static const ImWchar icon_ranges[] = { ICON_MIN_MD, ICON_MAX_16_MD, 0 };
        ImFontConfig config;
        config.MergeMode = true;
        config.PixelSnapH = true;
        config.GlyphMinAdvanceX = iconFontSize; 
        config.GlyphOffset.y = state.dpi(3.0f); 
        fontfile = AfxGetExePath() + "/icon-font.ttf";
        io.Fonts->AddFontFromFileTTF(fontfile.c_str(), state.dpi(iconFontSize), &config, icon_ranges);

        fontfile = AfxGetExePath() + "/gui-font-mono.ttf";
        gui_font_mono = io.Fonts->AddFontFromFileTTF(fontfile.c_str(), state.dpi(baseFontSize));

        state.gui_font = gui_font;
        state.gui_font_mono = gui_font_mono;

        ImGui::GetStyle().ScaleAllSizes(state.dpi_scale);
}
