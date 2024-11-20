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
#include "journal_notes.h"
#include "yearend_dialog.h"
#include "active_trades_actions.h"
#include "transaction_edit.h"
#include "questionbox.h"


void ShowQuestionPopup(AppState& state) {
    if (!state.show_questionbox_popup) return;

    if (!ImGui::IsPopupOpen(state.questionbox_caption.c_str())) {
        ImGui::OpenPopup(state.questionbox_caption.c_str());
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{state.dpi(26.0f), state.dpi(26.0f)});

    if (ImGui::BeginPopupModal(state.questionbox_caption.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {

        ImGui::BeginGroup();

        ImGui::Text("%s", state.questionbox_message.c_str());
        ImGui::Spacing();

        ImVec2 button_size(ImGui::GetFontSize() * 7.0f, 0.0f);

        if (state.questionbox_display_ok) {
            if (ImGui::Button("OK", button_size)) {
                state.show_questionbox_popup = false;
                ImGui::CloseCurrentPopup();
            }

        } else {
            if (ImGui::Button("Yes", button_size)) {
                state.show_questionbox_popup = false;
                ImGui::CloseCurrentPopup();

                if (state.questionbox_callback == QuestionCallback::ExpireSelectedLegs) {
                    ExpireSelectedLegs(state);
                }
                if (state.questionbox_callback == QuestionCallback::DeleteTransaction) {
                    DeleteTransaction(state);
                }
                if (state.questionbox_callback == QuestionCallback::ProcessYearEnd) {
                    ProcessYearEnd(state);
                }
                if (state.questionbox_callback == QuestionCallback::PermanentDeleteNote) {
                    PermanentDeleteNote(state);
                }
                if (state.questionbox_callback == QuestionCallback::DeleteJournalFolder) {
                    DeleteJournalFolder(state);
                }
            }

            ImGui::SameLine();
            if (ImGui::Button("No", button_size)) {
                state.show_questionbox_popup = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();
            if (ImGui::Button("Cancel", button_size)) {
                state.show_questionbox_popup = false;
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndGroup();

        ImGui::EndPopup();
    }

    ImGui::PopStyleVar();
}

void CustomQuestionBox(AppState& state, const std::string& caption, const std::string& message, QuestionCallback callback, bool display_ok) {
    if (state.show_questionbox_popup) return;
    state.questionbox_caption = caption;
    state.questionbox_message = message;
    state.questionbox_display_ok = display_ok;
    state.questionbox_callback = callback;
    state.show_questionbox_popup = true;
}
