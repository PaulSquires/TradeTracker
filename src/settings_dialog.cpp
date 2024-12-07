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

#include "imgui.h"

#include "appstate.h"
#include "active_trades_actions.h"
#include "utilities.h"
#include "settings_dialog.h"


// ========================================================================================
// Show Settings modal popup dialog
// ========================================================================================
void ShowSettingsDialogPopup(AppState& state) {
	if (!state.show_settingsdialog_popup) return;

    static bool is_first_open = true;
    bool close_dialog = false;

    float x_window_padding = state.dpi(40.0f);
    float y_window_padding = state.dpi(10.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, clrTextLightWhite(state));
    ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, clrPopupBg(state));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(x_window_padding, y_window_padding));
    
    // Trigger the popup
    if (!ImGui::IsPopupOpen(state.id_settingsdialog_popup.c_str())) {
        ImVec2 size{state.dpi(570.0f),state.dpi(470.0f)};
        ImGui::SetNextWindowSize(size);
    	ImGui::OpenPopup(state.id_settingsdialog_popup.c_str(), ImGuiPopupFlags_NoOpenOverExistingPopup);
    }

    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal(state.id_settingsdialog_popup.c_str(), NULL, ImGuiWindowFlags_NoDecoration)) {
        DialogTitleBar(state, "SETTINGS", state.show_settingsdialog_popup, close_dialog);

        if (is_first_open) is_first_open = false;

        float x_offset = x_window_padding + state.dpi(20);

        // Change the background color of the checkbox/radiobutton
        ImGui::PushStyleColor(ImGuiCol_FrameBg, clrCheckBox(state));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, clrCheckBoxHover(state));

        static bool is_allow_update = state.config.allow_update_check;
        static bool is_show_portfolio = state.config.show_portfolio_value;
        static bool is_show_45_trade_date = state.config.show_45day_trade_date;
        static bool is_dark_theme = (state.config.color_theme == ColorThemeType::Dark);
        ImGui::Checkbox("Check for newer available TradeTracker versions", &is_allow_update);
        ImGui::Checkbox("Display Net and Excess portfolio liquidity amounts", &is_show_portfolio);
        ImGui::Checkbox("Display closest 45 day trade expiration date information", &is_show_45_trade_date);
        // ImGui::Checkbox("Use dark color theme", &is_dark_theme);
        
        ImGui::Spacing();
        static int selected_number_format = (int)state.config.number_format_type;
        ImGui::Text("Number format:");
        ImGui::NewLine(); ImGui::SameLine(x_offset);
        ImGui::RadioButton("American (US) 1,234.00", &selected_number_format, (int)NumberFormatType::American);
        ImGui::NewLine(); ImGui::SameLine(x_offset);
        ImGui::RadioButton("European (EU) 1.234,00", &selected_number_format, (int)NumberFormatType::European);
    
        ImGui::Spacing();
        static int selected_costing_method = (int)state.config.costing_method;
        static bool is_exclude_nonstock = state.config.exclude_nonstock_costs;
        ImGui::Text("Stock costing method:");
        ImGui::NewLine(); ImGui::SameLine(x_offset);
        ImGui::RadioButton("Average Cost Basis (ACB)", &selected_costing_method, (int)CostingMethodType::AverageCost);
        ImGui::NewLine(); ImGui::SameLine(x_offset);
        ImGui::RadioButton("First-In, First-Out (FIFO)", &selected_costing_method, (int)CostingMethodType::fifo);
        ImGui::NewLine(); ImGui::SameLine(x_offset);
        ImGui::Checkbox("Exclude non-stock costs (eg. dividends, options premium)", &is_exclude_nonstock);

        ImGui::Spacing();
        static float gui_font_size = state.config.font_size;
        const char* gui_font_sizes[] = {"12","13","14","15","16","17","18"};
        std::string font_string = AfxDoubleToString(gui_font_size, 0);
        ImGui::Text("Font size (requires program restart):");
        ImGui::NewLine(); ImGui::SameLine(x_offset);
        ImGui::PushItemWidth(state.dpi(100));
        if (ImGui::BeginCombo("##FontSize", font_string.c_str(), ImGuiComboFlags_HeightLargest)) {
            for (int i = 0; i < IM_ARRAYSIZE(gui_font_sizes); i++) {
                // Change background color when the item is hot tracked
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, clrSelection(state));
                if (ImGui::Selectable(gui_font_sizes[i])) {
                    gui_font_size = AfxValDouble(gui_font_sizes[i]);
                }
              ImGui::PopStyleColor();  // pop hot tracking
            }
            ImGui::EndCombo();
        }
        ImGui::PopItemWidth();
        
        ImGui::NewLine(); ImGui::NewLine();
        ImVec2 button_size{state.dpi(180.0f), 0};
        if (ColoredButton(state, "Year End Procedure", 40, button_size, clrTextBrightWhite(state), clrRed(state))) {
            state.show_yearenddialog_popup = true;
            close_dialog = true;
        }

        // NOTE: The ImPlot calendar widget does not seem to have a built in method to change
        // it from displaying Sunday as the first day of the week to Monday.
        // ImGui::Spacing();
        static int selected_start_weekday = (int)state.config.start_weekday;
        // ImGui::Text("First day of the week:");
        // ImGui::NewLine(); ImGui::SameLine(x_offset);
        // ImGui::RadioButton("Sunday", &selected_start_weekday, (int)StartWeekdayType::Sunday);
        // ImGui::NewLine(); ImGui::SameLine(x_offset);
        // ImGui::RadioButton("Monday", &selected_start_weekday, (int)StartWeekdayType::Monday);

        ImGui::PopStyleColor(2);

        button_size = ImVec2(state.dpi(80.0f), 0);
        if (ColoredButton(state, "SAVE", 380.0f, button_size, clrTextBrightWhite(state), clrGreen(state))) {
            // If the costing method or exclude criteria has changed then we need to recalculate all Trade's ACB.
            if (state.config.costing_method != (CostingMethodType)selected_costing_method ||
                state.config.exclude_nonstock_costs != is_exclude_nonstock ) {
                for (const auto& trade : state.db.trades) {
                    trade->CalculateAdjustedCostBase(state);
                }
            }

            // Save the Settings
            state.config.allow_update_check = is_allow_update;
            state.config.show_portfolio_value = is_show_portfolio;
            state.config.number_format_type = (NumberFormatType)selected_number_format;
            state.config.costing_method = (CostingMethodType)selected_costing_method;
            state.config.exclude_nonstock_costs = is_exclude_nonstock;
            state.config.start_weekday = (StartWeekdayType)selected_start_weekday;
            state.config.show_45day_trade_date = is_show_45_trade_date;
            state.config.color_theme = (is_dark_theme ? ColorThemeType::Dark : ColorThemeType::Light);
            state.config.font_size = gui_font_size;

            // Save the configuration/settings file to disk
            state.config.SaveConfig(state);
            ReloadAppState(state);
            close_dialog = true;
        }

        ImGui::SameLine(); 
        if (ColoredButton(state, "Cancel", 470.0f, button_size, clrTextBrightWhite(state), clrRed(state))) {
            close_dialog = true;
        }

        if (close_dialog) {
            ImGui::CloseCurrentPopup();
	        state.show_settingsdialog_popup = false;
        }

        ImGui::EndPopup();
    }
    ImGui::PopStyleVar(); 
    ImGui::PopStyleColor(2); 
}
 
