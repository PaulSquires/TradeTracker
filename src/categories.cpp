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
#include "categories.h"


// ========================================================================================
// Show Categories modal popup dialog
// ========================================================================================
void ShowCategoriesDialogPopup(AppState& state) {
	if (!state.show_categoriesdialog_popup) return;

    static std::vector<std::string> categories;
    static bool is_first_open = true;
    bool close_dialog = false;

    float x_window_padding = state.dpi(40.0f);
    float y_window_padding = state.dpi(10.0f);
    ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, clrPopupBg(state));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(x_window_padding, y_window_padding));
    
    // Trigger the popup
    if (!ImGui::IsPopupOpen(state.id_categoriesdialog_popup.c_str())) {
        ImVec2 size{state.dpi(570.0f),state.dpi(470.0f)};
        ImGui::SetNextWindowSize(size);
    	ImGui::OpenPopup(state.id_categoriesdialog_popup.c_str());
    }

    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal(state.id_categoriesdialog_popup.c_str(), NULL, ImGuiWindowFlags_NoDecoration)) {
        DialogTitleBar(state, "CATEGORIES", state.show_categoriesdialog_popup, close_dialog);

        if (is_first_open) {
            is_first_open = false;
            categories.clear();
            for (int i = 0; i < 16; ++i) {
                categories.push_back(state.config.GetCategoryDescription(i));
            }
        }

        float x_offset = x_window_padding + state.dpi(20);

        // Define the size of the scrollable area 
        ImVec2 child_size = ImVec2(state.dpi(500), state.dpi(300)); // Width and height

        ImGui::BeginChild("ScrollableChild", child_size, true, ImGuiWindowFlags_HorizontalScrollbar);
        
        std::string id;
        for (int i = 0; i < 16; ++i) {
            id = std::to_string(i);
            ImGui::Text("%s", id.c_str());
            ImGui::SameLine();
            id = "##" + id;
            ImGui::InputText(id.c_str(), &categories.at(i));
        }

        ImGui::EndChild();

        ImVec2 button_size{state.dpi(80.0f), 0};
        ImGui::NewLine();
        ImGui::NewLine(); 
        ImGui::NewLine(); 
        if (ColoredButton(state, "SAVE", 360.0f, button_size, clrTextBrightWhite(state), clrGreen(state))) {
            for (int i = 0; i < 16; ++i) {
                state.config.SetCategoryDescription(i, categories.at(i));
            }
            state.config.SaveConfig(state);
            close_dialog = true;

            // If we are saving new Category labels then they will need to be redisplayed if
            // the ActiveTrades is active and we are sorted by Category.
            if (state.show_activetrades && 
                state.activetrades_filter_type == ActiveTradesFilterType::Category) {
                state.is_activetrades_data_loaded = false;
            }
            // Likewise, we need to redisplay the ClosedTrades and Transactions because they
            // also display Category descriptions.
            state.is_closedtrades_data_loaded = false;
            state.is_transactions_data_loaded = false;
        }

        ImGui::SameLine(); 
        if (ColoredButton(state, "Cancel", 450.0f, button_size, clrTextBrightWhite(state), clrRed(state))) {
            close_dialog = true;
        }

        if (close_dialog) {
            ImGui::CloseCurrentPopup();
	        state.show_categoriesdialog_popup = false;
        }

        ImGui::EndPopup();
    }
    ImGui::PopStyleVar(); 
    ImGui::PopStyleColor(); 
}
 
