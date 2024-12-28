/*
 *
 * MIT License
 *
 * Copyright(c) 2023-2024 Paul Squires
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files(the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions :
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include "imgui.h"

#include "text_input_popup.h"
#include "appstate.h"
#include "main_window.h"
#include "active_trades.h"
#include "closed_trades.h"
#include "trade_history.h"
#include "transaction_panel.h"
#include "transaction_edit.h"
#include "journal_notes.h"
#include "tab_panel.h"
#include "reconcile.h"
#include "settings_dialog.h"
#include "yearend_dialog.h"
#include "trade_dialog.h"
#include "import_dialog.h"
#include "questionbox.h"
#include "assignment.h"

// #include "iostream"


void ShowOpenSourceLicensePopup(AppState& state) {
    static bool first_run = true;

    if (first_run) {
        std::string license_text = "  \
        \r\n \
         MIT License\r\n  \
        \r\n \
         Copyright(c) 2023 - 2024 Paul Squires\r\n  \
        \r\n  \
        Permission is hereby granted, free of charge, to any person obtaining a copy\r\n  \
        of this software and associated documentation files(the 'Software'), to deal\r\n  \
        in the Software without restriction, including without limitation the rights\r\n  \
        to use, copy, modify, merge, publish, distribute, sublicense, and /or sell\r\n  \
        copies of the Software, and to permit persons to whom the Software is\r\n  \
        furnished to do so, subject to the following conditions :\r\n  \
        \r\n  \
        The above copyright notice and this permission notice shall be included in all\r\n  \
        copies or substantial portions of the Software.\r\n  \
        \r\n  \
        THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\r\n  \
        IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\r\n  \
        FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE\r\n  \
        AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\r\n  \
        LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\r\n  \
        OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\r\n  \
        SOFTWARE.\r\n  \
        ";
        if (state.db.trades.empty()) {
            CustomQuestionBox(state, "MIT License", license_text, QuestionCallback::None, true);
        }
        first_run = false;
    }
}


void ShowMainWindow(bool* p_open, AppState& state) {
    // Prevent the imgui.ini from being written
    ImGui::GetIO().IniFilename = nullptr;
    ImGui::GetIO().LogFilename = nullptr;

    ImGui::PushFont(state.gui_font);

    // Set window flags for full-screen ImGui window
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings |
                                    ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
                                    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

    // Get the size of the current viewport
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGui::Begin("##Full Screen Window", p_open, window_flags);

    // Get the available content region size inside the child window
    ImVec2 client_size = ImGui::GetContentRegionAvail();
    state.client_width = client_size.x;
    state.client_height = client_size.y;

    float splitter_width = state.dpi(4.0f);

    state.bottom_panel_height = state.dpi(40.0f);
    if (state.right_panel_width == 0) state.right_panel_width = state.dpi(440.0f);

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec2 item_spacing = style.ItemSpacing;
    state.top_panel_height = state.client_height - state.bottom_panel_height;
    state.left_panel_width = state.client_width - state.right_panel_width - (splitter_width  + state.dpi(item_spacing.x));

    // Ensure windows sizes remain reasonable
    float min_size = state.dpi(50.0f);
    if (state.top_panel_height < min_size) state.top_panel_height = min_size;
    if (state.left_panel_width < min_size) state.left_panel_width = min_size;
    if (state.right_panel_width < min_size) state.right_panel_width = min_size;

    ShowOpenSourceLicensePopup(state);

    // Popup windows need to be rendered every frame so do checks to see if the
    // following should continue to be displayed.
    ShowReconciliationResultsPopup(state);
    ShowAssignmentPopup(state);
    ShowTradeDialogPopup(state);
    ShowSettingsDialogPopup(state);
    ShowYearEndDialogPopup(state);
    ShowActiveTradesRightClickPopup(state);
    ShowJournalFoldersRightClickPopup(state);
    ShowJournalNotesRightClickPopup(state);
    ShowTabPanelConnectRightClickPopup(state);
    ShowGetTextPopup(state);

    // Modal popups handle their own rendering of the QuestionPopup so bypass
    // it here if a modal is active. These QuestionPopups are more suited to
    // situations like when a connection fails and a message is needed to be
    // displayed.
    if (!state.is_modal_active()) {
        ShowQuestionPopup(state);
    }

    // Handle situation where no trades exist but we have connected to TWS and
    // now show Import Dialog.
    ShowImportDialogPopup(state);


    // Top panel (which is divided into the left and right panel)
    ImGui::BeginChild("##Top Panel", ImVec2(state.client_width, state.top_panel_height));

    // Journal Notes will take up the entire window whereas the other options will
    // create a split window.
    if (state.show_journalnotes) {
        ShowJournalNotes(state);

    } else {

        // Left top panel
        ShowActiveTrades(state);
        ShowClosedTrades(state);
        ShowTransPanel(state);


        // Adjust the splitter between left and right top panels
        ImGui::SameLine();
        ImGui::Button("##TopSplitter", ImVec2(splitter_width, state.dpi(-1.0f)));
        if (ImGui::IsItemActive()) {
            state.left_panel_width += ImGui::GetIO().MouseDelta.x;
            state.right_panel_width -= ImGui::GetIO().MouseDelta.x;
        }

        if (ImGui::IsItemActive() || ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        }


        // Right top panel
        ImGui::SameLine();
        ShowTradeHistory(state);
        ShowTransEdit(state);
    }

    ImGui::EndChild();  // top panel

    // Bottom panel
    ShowTabPanel(state);

    ImGui::End();       // full screen window
    ImGui::PopFont();   // gui_font
}

