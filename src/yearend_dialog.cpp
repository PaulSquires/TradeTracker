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
#include <string>

#include "appstate.h"
#include "active_trades_actions.h"
#include "utilities.h"
#include "questionbox.h"
#include "yearend_dialog.h"


// ========================================================================================
// Process the Year End closing procedure
// ========================================================================================
bool ProcessYearEnd(AppState& state) {
    std::string close_date = state.year_to_close + "-12-31";

    // Iterate the trades and remove all closed trades before the close date.
    auto iter = state.db.trades.begin();
    while (iter != state.db.trades.end())
    {
        if (!(*iter)->is_open) {

            // Iterate to find the latest closed date
            std::string trade_close_date = "0000-00-00";
            for (auto& trans : (*iter)->transactions) {
                if (trans->trans_date > trade_close_date) {
                    trade_close_date = trans->trans_date;
                }
            }

            if (trade_close_date <= close_date) {
                iter = state.db.trades.erase(iter);
            } else {
                iter++;
            }
        }
        else
        {
            iter++;
        }
    }

    ReloadAppState(state);
    return true;
}


// ========================================================================================
// Show Year End modal popup dialog
// ========================================================================================
void ShowYearEndDialogPopup(AppState& state) {
	if (!state.show_yearenddialog_popup) return;

    static std::string year_to_close = std::to_string(AfxGetYear(AfxCurrentDate()) - 1);
    static bool is_first_open = true;
    bool close_dialog = false;

    float x_window_padding = state.dpi(40.0f);
    float y_window_padding = state.dpi(10.0f);
    ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, clrPopupBg(state));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(x_window_padding, y_window_padding));
    
    // Trigger the popup
    if (!ImGui::IsPopupOpen(state.id_yearenddialog_popup.c_str())) {
        ImVec2 size{state.dpi(610.0f),state.dpi(340.0f)};
        ImGui::SetNextWindowSize(size);
    	ImGui::OpenPopup(state.id_yearenddialog_popup.c_str(), ImGuiPopupFlags_NoOpenOverExistingPopup);
    }

    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal(state.id_yearenddialog_popup.c_str(), NULL, ImGuiWindowFlags_NoDecoration)) {
        DialogTitleBar(state, "YEAR END PROCEDURE", state.show_yearenddialog_popup, close_dialog);

        float x_offset = x_window_padding + state.dpi(20);

        ImGui::NewLine(); 
        std::string text = "*** I M P O R T A N T ***";
        TextLabel(state, text.c_str(), 225.0f, clrRed(state), clrBackDarkBlack(state));
        
        ImGui::NewLine(); 
        text = "BACKUP YOUR DATABASE PRIOR TO RUNNING THIS PROCEDURE";
        TextLabel(state, text.c_str(), 110.0f, clrRed(state), clrBackDarkBlack(state));

        ImU32 back_color = clrBackMediumGray(state);  //ColorConvertImVec4ToU32(ImVec4(0.10f, 0.10f, 0.10f, 1.00f));

        ImGui::NewLine(); 
        ImGui::NewLine(); 
        text = "All closed trades dated before December 31 of the selected year will be removed.";
        TextLabel(state, text.c_str(), 70.0f, clrTextMediumWhite(state), back_color);

        ImGui::NewLine(); 
        TextLabel(state, "Year:", 240.0f, clrTextMediumWhite(state), back_color);
        if (is_first_open) ImGui::SetKeyboardFocusHere();
        TextInput(state, "##year", &year_to_close, ImGuiInputTextFlags_CharsDecimal, 285.0f, 60.0f, clrTextLightWhite(state), back_color);

        ImGui::NewLine(); 
        ImGui::NewLine(); 
        ImGui::NewLine(); 
        ImGui::NewLine(); 
        ImVec2 button_size{state.dpi(80.0f), 0};
        if (ColoredButton(state, "PROCESS", 480.0f, button_size, clrTextBrightWhite(state), clrGreen(state))) {
            int year_to_close_int = AfxValInteger(year_to_close);
            if (year_to_close_int > 2000 && year_to_close_int < 3000) {   // some basic sanity check
                state.year_to_close = year_to_close;
                std::string text = "Remove closed Trades on or before December 31, " + year_to_close 
                    + "? \n\nDo not continue if you have not made a database backup.";
                CustomQuestionBox(state, "Remove Closed Trades", text, QuestionCallback::ProcessYearEnd);
                close_dialog = true;
            }
        }

        ImGui::NewLine(); 
        if (ColoredButton(state, "Cancel", 480.0f, button_size, clrTextBrightWhite(state), clrRed(state))) {
            close_dialog = true;
        }

        ShowQuestionPopup(state);

        if (close_dialog) {
            ImGui::CloseCurrentPopup();
	        state.show_yearenddialog_popup = false;
        }

        if (is_first_open) is_first_open = false;

        ImGui::EndPopup();
    }
    ImGui::PopStyleVar(); 
    ImGui::PopStyleColor(); 
}
 
