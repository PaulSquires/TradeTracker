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

#include "appstate.h"
#include "utilities.h"
#include "journal_notes.h"
#include "text_input_popup.h"


void ShowGetTextPopup(AppState& state) {
    if (!state.show_gettext_popup) return;

    ImU32 back_color = clrBackMediumGray(state);
    ImU32 text_color_white = clrTextLightWhite(state);
    ImU32 text_color_gray = clrTextDarkWhite(state);

    static bool is_first_open = true;
    static std::string input_text;

    ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, clrPopupBg(state));

    if (!ImGui::IsPopupOpen(state.gettextpopup_caption.c_str())) {
        ImGui::OpenPopup(state.gettextpopup_caption.c_str());
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{state.dpi(26.0f), state.dpi(26.0f)});


    if (ImGui::BeginPopupModal(state.gettextpopup_caption.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {

        ImGui::BeginGroup();

        TextLabel(state, state.gettextpopup_message.c_str(), 0.0f, text_color_white, back_color);
        ImGui::NewLine();

        if (is_first_open) {
            ImGui::SetKeyboardFocusHere();
            input_text = state.gettextpopup_result_string;
        }
        TextInput(state, "##textpopup", &input_text, ImGuiInputTextFlags_None, 0.0f, 275.0f, text_color_white, back_color); 
        ImGui::NewLine();
        ImGui::NewLine();
        is_first_open = false;

        // Check if the Escape key is pressed
        bool is_esc_pressed = ImGui::IsKeyPressed(ImGuiKey_Escape);
        bool is_enter_pressed = ImGui::IsKeyPressed(ImGuiKey_Enter);

        ImVec2 button_size = ImVec2(state.dpi(80.0f), 0);
        if (is_enter_pressed || ColoredButton(state, "SAVE", 110.0f, button_size, clrTextBrightWhite(state), clrGreen(state))) {
            input_text = AfxTrim(input_text);
            if (input_text.length()) {
                state.show_gettext_popup = false;
                state.gettextpopup_cancelled = false;
                state.gettextpopup_result_string = input_text;
                input_text = "";
                is_first_open = true;
                ImGui::CloseCurrentPopup();
                if (state.gettextpopup_callback == QuestionCallback::AddJournalFolder) {
                    AddJournalFolder(state);
                }
                if (state.gettextpopup_callback == QuestionCallback::RenameJournalFolder) {
                    RenameJournalFolder(state);
                }
            }
        }

        ImGui::SameLine(); 

        if (is_esc_pressed || ColoredButton(state, "Cancel", 200.0f, button_size, clrTextBrightWhite(state), clrRed(state))) {
            state.show_gettext_popup = false;
            state.gettextpopup_cancelled = true;
            state.gettextpopup_result_string = "";
            input_text = "";
            is_first_open = true;
            ImGui::CloseCurrentPopup();
        }




        ImGui::EndGroup();

        ImGui::EndPopup();
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

void CustomGetTextPopup(AppState& state, const std::string& caption, const std::string& message, const std::string& default_text, QuestionCallback callback) {
    if (state.show_gettext_popup) return;
    state.gettextpopup_caption = caption;
    state.gettextpopup_message = message;
    state.gettextpopup_cancelled = false;
    state.gettextpopup_result_string = default_text;
    state.gettextpopup_callback = callback;
    state.show_gettext_popup = true;
}
