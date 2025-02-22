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
#include "imgui_internal.h"

#include "messagebox.h"
#include "trade_history.h"
#include "icons_material_design.h"
#include "appstate.h"
#include "reconcile.h"
#include "tws-client.h"
#include "tab_panel.h"

//#include <iostream>


void ShowTabPanelConnectRightClickPopup(AppState& state) {
    if (state.show_connect_rightclickmenu) {
        ImGui::OpenPopup("ConnectRightClickMenuPopup");
        state.show_connect_rightclickmenu = false;
    }

    if (ImGui::BeginPopup("ConnectRightClickMenuPopup")) {

        ImGui::PushStyleColor(ImGuiCol_Text, clrTextLightWhite(state));
        // Change background color when the item is hot tracked
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, clrSelection(state));

        if (ImGui::MenuItem("Connect to TWS (live trading)")) {
            state.show_connect_rightclickmenu = false;
            state.is_paper_trading = false;
            tws_Connect(state);
        }
        if (ImGui::MenuItem("Connect to TWS (paper trading)")) {
            state.show_connect_rightclickmenu = false;
            state.is_paper_trading = true;
            tws_Connect(state);
        }

        ImGui::PopStyleColor(2);
        ImGui::EndPopup();
    }
}


bool ShowVerticalLine(AppState &state) {
    ImGui::SameLine();

    // Get the current ImGui window
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    float button_width = state.dpi(3.0f);
    float button_height = state.bottom_panel_height;
    ImVec2 size{button_width, button_height};

    // Calculate the button's bounding box
    ImGuiContext& g = *ImGui::GetCurrentContext();
    ImGuiStyle& style = g.Style;
    ImVec2 pos = window->DC.CursorPos;
    ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));

    // Register the button's region for interaction
    ImGui::ItemSize(bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, 0))
        return false;

    ImU32 buttonColor = clrBackDarkBlack(state);
    ImU32 textColor = clrTextMediumWhite(state);

    // Get the current ImDrawList
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // Draw the vertical line
    draw_list->AddRectFilled(bb.Min, bb.Max, buttonColor, style.FrameRounding);
    ImVec2 p1{bb.GetCenter().x, bb.GetCenter().y - state.dpi(7.0f)};
    ImVec2 p2{p1.x, p1.y + state.dpi(17.0f)};
    draw_list->AddLine(p1, p2, clrTextMediumWhite(state), state.dpi(1.0f));

    return true;
}


bool ShowTabPanelItem(AppState &state, TabPanelItem id, const char* label) {
    // Get the current ImGui window
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    bool is_selected = (state.selected_tabpanelitem == id);

    float padding = state.dpi(10.0f);   // 10 left and 10 right
    float button_width = padding * 2 + ImGui::CalcTextSize(label).x;
    float button_height = ImGui::GetContentRegionAvail().y;
    ImVec2 size{button_width, button_height};

    // Calculate the button's bounding box
    ImGuiContext& g = *ImGui::GetCurrentContext();
    ImGuiStyle& style = g.Style;
    ImVec2 pos = window->DC.CursorPos;
    ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));

    // Register the button's region for interaction
    ImGui::ItemSize(bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, 0))
        return false;

    // Check if the button is hovered or pressed
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, ImGui::GetID(label), &hovered, &held, ImGuiButtonFlags_PressedOnClick);

    // Choose the color based on the button's state
    ImU32 buttonColor = is_selected ? clrBackDarkGray(state) : clrBackDarkBlack(state);
    ImU32 textColor = hovered || is_selected ? clrTextBrightWhite(state) : clrTextMediumWhite(state);

    // Get the current ImDrawList
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // Draw the button
    draw_list->AddRectFilled(bb.Min, bb.Max, buttonColor, 0);
    ImVec2 text_size{ImGui::CalcTextSize(label)};
    pos.x = bb.GetCenter().x - text_size.x * 0.5f;
    pos.y = bb.GetCenter().y - text_size.y * 0.5f;
    draw_list->AddText(pos, textColor, label);

    // If this item is selected then draw the green underline
    if (is_selected) {
        ImVec2 p1{pos.x, pos.y + text_size.y + state.dpi(3.0f)};
        ImVec2 p2{pos.x + text_size.x, p1.y};
        draw_list->AddLine(p1, p2, clrGreen(state), state.dpi(1.0f));
    }

    ShowVerticalLine(state);

    if (pressed) {
        state.selected_tabpanelitem = id;
        SelectTabPanelItem(state);
    }

    return pressed;
}


void SelectTabPanelItem(AppState& state) {
    switch (state.selected_tabpanelitem) {
    case TabPanelItem::ActiveTrades:
        state.show_activetrades = true;
        state.show_closedtrades = false;
        state.show_transpanel = false;
        state.show_tradehistory = true;
        state.show_journalnotes = false;
        SetTradeHistoryTrade(state, state.activetrades_selected_trade);
        break;
    case TabPanelItem::ClosedTrades:
        state.show_activetrades = false;
        state.show_closedtrades = true;
        state.show_transpanel = false;
        state.show_tradehistory = true;
        state.show_journalnotes = false;
        SetTradeHistoryTrade(state, state.closedtrades_selected_trade);
        break;
    case TabPanelItem::Transactions:
        state.show_activetrades = false;
        state.show_closedtrades = false;
        state.show_transpanel = true;
        state.show_tradehistory = true;
        state.show_journalnotes = false;
        SetTradeHistoryTrade(state, state.transactions_selected_trade);
        break;
    case TabPanelItem::JournalNotes:
        state.show_tradehistory = false;
        state.show_journalnotes = true;
        break;
    }
}


void DisplayIconButtons(AppState& state) {
    ImGui::BeginGroup();
    static ImVec2 icon_button_size{state.dpi(28.0f), state.dpi(28.0f)};

    ImGui::Dummy(ImVec2(0.0f, state.dpi(0.1f)));   // Add vertical spacing

    ImGui::PushStyleColor(ImGuiCol_Button, clrBackDarkBlack(state));

    bool is_connected = tws_IsConnected(state);
    ImGui::PushStyleColor(ImGuiCol_Text, is_connected ? clrGreen(state) : clrRed(state));
    if (ImGui::Button(ICON_MD_LAN, icon_button_size)) {
        if (is_connected) {
            tws_Disconnect(state);
        } else {
            state.show_connect_rightclickmenu = true;
        }
    }
    ImGui::PopStyleColor();
    Tooltip(state, is_connected ? "Click to Disconnect" : "Click to Connect",
        clrTextLightWhite(state), clrBackMediumGray(state));

    ImGui::SameLine(state.dpi(34.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, clrGreen(state));
    if (ImGui::Button(ICON_MD_CHECK_CIRCLE, icon_button_size)) {
        if (!tws_IsConnected(state)) {
            CustomMessageBox(state, "Error", "Must be connected to TWS to perform a reconciliation.");
        } else {
            // Do the reconciliation
            if (!state.is_pause_market_data) {
                Reconcile_LoadAllLocalPositions(state);   // in case some have been closed/expired/rolled since last time
                Reconcile_doReconciliation(state);
                state.show_reconciliation_popup = true;
            }
        }
    }
    ImGui::PopStyleColor();
    Tooltip(state, "Reconcile", clrTextLightWhite(state), clrBackMediumGray(state));

    ImGui::SameLine(state.dpi(68.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, clrTextMediumWhite(state));
    if (ImGui::Button(ICON_MD_SETTINGS, icon_button_size)) {
        state.show_settingsdialog_popup = true;
    }
    ImGui::PopStyleColor();
    Tooltip(state, "Settings", clrTextLightWhite(state), clrBackMediumGray(state));

    ImGui::PopStyleColor();  // icon button back color
    ImGui::EndGroup();
}


void DisplayUpdateAvailable(AppState& state) {
    // Compare the versions
    if (!state.version_available_display.empty() && (state.version_current_display != state.version_available_display)) {
        std::string text = "TradeTracker v" + state.version_available_display + std::string(" available");
        const char* link = "https://www.tradetracker.planetsquires.com/";
        float window_width = ImGui::GetWindowSize().x;
        float text_width = ImGui::CalcTextSize(text.c_str()).x;
        ImGui::SameLine(window_width - text_width - ImGui::GetStyle().ItemSpacing.x);
        ImGui::TextLinkOpenURL(text.c_str(), link);     // hyperlink text button, automatically open file/url when clicked
    }
}


void DisplayLinkButtons(AppState& state) {
    ImGui::BeginGroup();
    ImGui::SameLine(state.dpi(110));
    ShowTabPanelItem(state, TabPanelItem::ActiveTrades, "Active Trades");
    ImGui::SameLine();
    ShowTabPanelItem(state, TabPanelItem::ClosedTrades, "Closed Trades");
    ImGui::SameLine();
    ShowTabPanelItem(state, TabPanelItem::Transactions, "Transactions");
    ImGui::SameLine();
    ShowTabPanelItem(state, TabPanelItem::JournalNotes, "Journal Notes");
    ImGui::SameLine();
    DisplayUpdateAvailable(state);
    ImGui::EndGroup();
}


void ShowTabPanel(AppState& state) {
    // Push the custom padding
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, clrBackDarkBlack(state));
    ImGui::BeginChild("TabPanel", ImVec2(state.client_width, state.bottom_panel_height));
    ImGui::PopStyleColor();

    DisplayIconButtons(state);
    DisplayLinkButtons(state);

    // Do not process the MessageBox Popup here if the TradeDialog is already
    // showing otherwise it will close the TradeDialog.
    if (!state.show_tradedialog_popup) {
        ShowMessageBoxPopup(state);
    }

    ImGui::EndChild();
    ImGui::PopStyleVar();
}


