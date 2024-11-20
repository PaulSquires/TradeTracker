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
#include "messagebox.h"


// ========================================================================================
// Generic MessageBox modal popup dialog
// ========================================================================================
void ShowMessageBoxPopup(AppState& state) {
	if (!state.show_messagebox_popup) return;

    // Trigger the popup
    if (!ImGui::IsPopupOpen(state.messagebox_caption.c_str())) {
        ImVec2 size{state.dpi(400.0f),state.dpi(280.0f)};
        ImGui::SetNextWindowSize(size);
    	ImGui::OpenPopup(state.messagebox_caption.c_str());
    }

    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(state.dpi(20.0f), state.dpi(20.0f)));
	if (ImGui::BeginPopupModal(state.messagebox_caption.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::PopStyleVar();

        ImGui::BeginGroup();

        ImGui::TextWrapped("%s", state.messagebox_message.c_str());
        ImGui::Dummy(ImVec2{0,state.dpi(20.0f)});

        // Center the OK button horizontally
        ImVec2 button_size{state.dpi(80.0f), 0}; 
        float windowWidth = ImGui::GetWindowSize().x;
        float buttonWidth = button_size.x + ImGui::GetStyle().FramePadding.x * 2.0f;
        float centeredX = (windowWidth - buttonWidth) / 2.0f;
        ImGui::SetCursorPosX(centeredX);

        ImGui::SetItemDefaultFocus();
        if (ImGui::Button("OK", button_size)) {
	        state.show_messagebox_popup = false;
            ImGui::CloseCurrentPopup();
        }

        // Check if the Escape key is pressed
        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            state.show_messagebox_popup = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndGroup();

        ImGui::EndPopup();
    }
}
 
 
void CustomMessageBox(AppState& state, const std::string& caption, const std::string& message) {
    if (state.show_messagebox_popup) return;
    state.messagebox_caption = caption;
    state.messagebox_message = message;
    state.show_messagebox_popup = true;
}
