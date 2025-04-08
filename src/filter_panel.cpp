/*

MIT License

Copyright(c) 2023-2025 Paul Squires

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
#include "imgui_stdlib.h"

#include "appstate.h"
#include "icons_material_design.h"
#include "trade_dialog.h"    // CalendarComboBox and CategoriesComboBox
#include "categories.h"
#include "utilities.h"
#include "filter_panel.h"


// ========================================================================================
// Set the StartDate and EndDate based on the current value of the Date Filter.
// ========================================================================================
void SetFilterPanelStartEndDates(AppState& state) {
    // Do not modify dates if Custom has been set.
    if (state.datefilter_current_item == (int)TransDateFilterType::Custom) return;

    std::string end_date = AfxCurrentDate();   // ISO format YYYY-MM-DD
    std::string start_date = end_date;       // ISO format
    int adjust_days = 0;

    if (state.datefilter_current_item == (int)TransDateFilterType::YearToDate) {
        start_date = end_date.substr(0, 4) + "-01-01";
    }
    else if (state.datefilter_current_item == (int)TransDateFilterType::MonthToDate) {
        start_date = end_date.substr(0, 4) + "-" + end_date.substr(5, 2) + "-01";
    }
    else if (state.datefilter_current_item == (int)TransDateFilterType::Yesterday) {
        start_date = AfxDateAddDays(end_date, -1);
        end_date = start_date;
    }
    else if (state.datefilter_current_item == (int)TransDateFilterType::Today) {
        // Dates are already set to current date
    }
    else {
        switch (state.datefilter_current_item) {
        case (int)TransDateFilterType::Days7: adjust_days = 7; break;
        case (int)TransDateFilterType::Days14: adjust_days = 14; break;
        case (int)TransDateFilterType::Days30: adjust_days = 30; break;
        case (int)TransDateFilterType::Days60: adjust_days = 60; break;
        case (int)TransDateFilterType::Days120: adjust_days = 120; break;
        }
        start_date = AfxDateAddDays(end_date, -(adjust_days));
    }

    state.filterpanel_start_date = start_date;
    state.filterpanel_end_date = end_date;
}


void ShowDateFilterComboBox(AppState& state) {
    static ImU32 text_color = clrTextDarkWhite(state);
    ImGui::SetNextItemWidth(state.dpi(133.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, clrBackMediumGray(state));  // Custom background color
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, clrBackMediumGray(state));  // Custom background color
    ImGui::PushStyleColor(ImGuiCol_PopupBg, clrBackMediumGray(state));  // Custom popup background color
    ImGui::PushStyleColor(ImGuiCol_Text, text_color);

    const char* datefilter_items[] = { 
        "Today",
        "Yesterday",
        "7 days",
        "14 days",
        "30 days",
        "60 days",
        "120 days",
        "Month to Date",
        "Year to Date",
        "Custom"
    };

    if (ImGui::BeginCombo("##DateFilter", datefilter_items[state.datefilter_current_item], ImGuiComboFlags_HeightLarge)) {

        // Loop through items and create a selectable for each
        ImGui::PushStyleColor(ImGuiCol_Text, clrTextLightWhite(state));
        for (int i = 0; i < IM_ARRAYSIZE(datefilter_items); i++) {
            bool is_selected = (state.datefilter_current_item == i);

            // Change background color when the item is hot tracked
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, clrSelection(state));

            // Indent by using spaces
            std::string text = datefilter_items[i];
            text = "      " + text;
            if (is_selected) text.replace(0,5,ICON_MD_CIRCLE_SMALL);
            if (ImGui::Selectable(text.c_str(), is_selected)) {
                state.datefilter_current_item = i;
                SetFilterPanelStartEndDates(state);
                state.is_closedtrades_data_loaded = false;
                state.is_transactions_data_loaded = false;
            }
            
            ImGui::PopStyleColor();  // pop hot tracking

            // Set the initial focus when opening the combo box
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::PopStyleColor();
        ImGui::EndCombo();
    }
    ImGui::PopStyleColor(4);

    text_color = (ImGui::IsItemHovered()) ? clrTextLightWhite(state) : clrTextDarkWhite(state);
}


std::string FormatFilterDate(std::string& iso_date) {
    // Return iso_date in mmm dd, yyyy format
    // YYYY-MM-DD
    // 0123456789
    std::string result = 
            AfxGetShortMonthName(iso_date) + " " +
            iso_date.substr(8, 2) + ", " +
            iso_date.substr(0, 4);
    return result;    
}


// ========================================================================================
// Show Filter Panel used for ClosedTrades and Transactions
// ========================================================================================
void ShowFilterPanel(AppState& state) {
    ImU32 back_color = clrBackMediumGray(state);
    ImU32 text_color_white = clrTextLightWhite(state);
    ImU32 text_color_gray = clrTextDarkWhite(state);

    static bool is_first_time = true;

    if (is_first_time) {
        state.datefilter_current_item = (int)TransDateFilterType::MonthToDate;
        state.filterpanel_selected_category = CATEGORY_END + 2;  // ALL CATEGORIES
        SetFilterPanelStartEndDates(state);
        is_first_time = false;
    }

    TextLabel(state, "Ticker Filter", 0.0f, text_color_gray, back_color);
    TextLabel(state, "Date Filter", 120.0f, text_color_gray, back_color);
    TextLabel(state, "Start Date", 270.0f, text_color_gray, back_color);
    TextLabel(state, "End Date", 405.0f, text_color_gray, back_color);
    TextLabel(state, "Category Filter", 540.0f, text_color_gray, back_color);

    ImGui::NewLine();
    TextInput(state, "##ticker_symbol", &state.filterpanel_ticker_symbol, ImGuiInputTextFlags_CharsUppercase, 0.0f, 70.0f, text_color_white, back_color);
    ImVec2 button_size = ImVec2{state.dpi(30.0f), 0}; 
    if (ColoredButton(state, "GO", 72.0f, button_size, clrBackDarkBlack(state), clrBlue(state))) {
        state.is_closedtrades_data_loaded = false;
        state.is_transactions_data_loaded = false;
    }

    ImGui::SameLine(state.dpi(120));
    ShowDateFilterComboBox(state);   // 133.0f wide

    std::string start_date_display = FormatFilterDate(state.filterpanel_start_date);
    std::string end_date_display = FormatFilterDate(state.filterpanel_end_date);
    if (CalendarComboBox(state, "StartDate", state.filterpanel_start_date, start_date_display, ImGuiComboFlags_HeightLarge, 270.0f, 120.0f)) {
        state.datefilter_current_item = (int)TransDateFilterType::Custom;
        state.is_closedtrades_data_loaded = false;
        state.is_transactions_data_loaded = false;
    }
    if (CalendarComboBox(state, "EndDate", state.filterpanel_end_date, end_date_display, ImGuiComboFlags_HeightLarge, 405.0f, 120.0f)) {
        state.datefilter_current_item = (int)TransDateFilterType::Custom;
        state.is_closedtrades_data_loaded = false;
        state.is_transactions_data_loaded = false;
    }

    CategoriesComboBox(state, "Categories", 540.0f, 210.0f);
    if (ColoredButton(state, ICON_MD_EDIT, 754.0f, button_size, clrTextDarkWhite(state), back_color)) {
        state.show_categoriesdialog_popup = true;
    }
    Tooltip(state, "Edit Categories", clrTextLightWhite(state), clrBackMediumGray(state));

    ImGui::BeginGroup();
    ImGui::EndGroup();

    // If a secondary popup modal must be shown (eg Categories), then we need to display it
    // here prior to the call to the EndPopup()
    ShowCategoriesDialogPopup(state);

}
 
