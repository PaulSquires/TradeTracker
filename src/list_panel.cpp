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
#include "imgui_internal.h" // For internal access (use with caution)
#include <string>

#include "appstate.h"
#include "tab_panel.h"
#include "list_panel.h"
#include "list_panel_data.h"

// #include <iostream>


void SetSelectedGridRow(AppState& state, CListPanel& lp, CListPanelData& ld) {
    // Save the selected row for ActiveTrades table in order to easily position it again after a database reload
    if (lp.table_id == TableType::active_trades) state.activetrades_current_row_index = ImGui::TableGetRowIndex();
    if (lp.table_id == TableType::closed_trades) state.closedtrades_current_row_index = ImGui::TableGetRowIndex();
    if (lp.table_id == TableType::trans_panel)   state.transactions_current_row_index = ImGui::TableGetRowIndex();

    // If CTRL (or SHIFT) is held then:
    // - Select the line -OR- toggle deselect an already selected line.
    ImGuiIO& io = ImGui::GetIO();
#if defined(__APPLE__)
    if ((io.KeySuper && io.MouseClicked[0]) || io.KeyShift && io.MouseClicked[0]) {
#else
    if ((io.KeyCtrl && io.MouseClicked[0]) || io.KeyShift && io.MouseClicked[0]) {
#endif
        // Deselect all selected rows of all other Trades
        for (auto& item : *lp.vec) {
            if (item.trade != ld.trade) item.is_selected = false;
        }
        // Toggle the current line in the Trade
        ld.is_selected = (ld.is_selected) ? false : true;
    } else {
        // If CTRL or SHIFT is not held then:
        // - Deselect all lines and select the chosen line.
        for (auto& item : *lp.vec) {
            item.is_selected = false;
        }
        ld.is_selected = true;
    }
}


void DrawTableRow(AppState& state, CListPanel& lp, CListPanelData& ld) {
    ImGui::TableNextRow(0, state.dpi(lp.row_height));

    // Draw the columns cells
    int colnum = 0;
    for (colnum = 0; colnum < lp.column_count; ++colnum) {
        ImGui::TableSetColumnIndex(colnum);

        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ld.col[colnum].back_color);

        std::string& text = ld.col[colnum].text;

        if (ld.line_type == LineType::category_header ||
            ld.line_type == LineType::none) {

            ImGui::PushStyleColor(ImGuiCol_Text, ld.col[colnum].text_color);

            // Render the text spanning across the entire row
            ImGui::Text("%s", text.c_str());

            // Skip the remaining columns in this row
            ImGui::PopStyleColor();
            break;
        }

        float cell_width = ImGui::GetColumnWidth();
        float text_width = ImGui::CalcTextSize(text.c_str()).x;
        float offset = 0;

        if (ld.col[colnum].Alignment == StringAlignment::center) {
            offset = std::abs((cell_width - text_width) / 2);
        }
        if (ld.col[colnum].Alignment == StringAlignment::right) {
            offset = std::abs(cell_width - text_width);
        }

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);

        // If this is the TradeHistory grid then show the trade Edit icon if the mouse is over
        // the header line. Show/Hide by manipulating the text color.
        ImU32 text_color = ld.col[colnum].text_color;
        if (colnum == COLUMN_EDIT_ICON ) {
            if (lp.table_id == TableType::trade_history) {
                text_color = ImGui::IsItemHovered() ? ld.col[colnum].text_color : ld.col[colnum].back_color;
            }
            if (lp.table_id == TableType::trans_edit) {
                text_color = ld.col[colnum].back_color;
            }
        }

        ImGui::PushStyleColor(ImGuiCol_Text, text_color);
        ImGui::PushStyleColor(ImGuiCol_Header, clrSelection(state));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, clrSelection(state));
        if (ld.line_type == LineType::history_roi ||
            ld.line_type == LineType::nonselectable ) {
            ImGui::Text("%s", text.c_str());
        } else {
            // Handle row selection
            ImGui::PushID(colnum);
            ImGui::Selectable(text.c_str(), ld.is_selected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap);
            ImGui::PopID();
        }
        ImGui::PopStyleColor(3);
    }


    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        SetSelectedGridRow(state, lp, ld);

        if (lp.table_id == TableType::active_trades) {
            state.activetrades_selected_trade = ld.trade;
            state.selected_tabpanelitem = TabPanelItem::ActiveTrades;
            SelectTabPanelItem(state);
        }

        if (lp.table_id == TableType::closed_trades) {
            state.closedtrades_selected_trade = ld.trade;
            state.selected_tabpanelitem = TabPanelItem::ClosedTrades;
            SelectTabPanelItem(state);
        }

        if (lp.table_id == TableType::trans_panel) {
            state.transactions_selected_trade = ld.trade;
            state.selected_tabpanelitem = TabPanelItem::Transactions;
            SelectTabPanelItem(state);
        }

        if (lp.table_id == TableType::trade_history) {
            // Clicked on a TradeHistory header line to invoke edit/delete of the transaction
            ld.is_selected = false;
            state.trans_edit_trade = ld.trade;
            state.trans_edit_transaction = ld.trans;
            state.show_tradehistory = false;
            state.show_transedit = true;
        }
    }

    // Check for right-click
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        if (!ld.is_selected) return;
        if (!state.show_activetrades_rightclickmenu && lp.table_id == TableType::active_trades) {
            // Set flag for MainWindow to handle right-click event
            state.activetrades_selected_trade = ld.trade;
            state.show_activetrades_rightclickmenu = true;
            state.activetrades_rightclickmenu_shares_count = 0;
            state.activetrades_rightclickmenu_futures_count = 0;
            state.activetrades_rightclickmenu_shares_exist = false;
            state.activetrades_rightclickmenu_futures_exist = false;
            state.activetrades_rightclickmenu_options_exist = false;
            state.activetrades_rightclickmenu_assignment_leg = nullptr;

            // If the selected Leg is an Option Leg with SHORT negative quantity then
            // we save its Leg here to be used for possible ASSIGNMENT.
            if (ld.leg && ld.leg->underlying == Underlying::Options && ld.leg->open_quantity < 0) {
                state.activetrades_rightclickmenu_assignment_leg = ld.leg;
            }

            // Create vector holding all of the selected lines.
            // determine what exists within the trade such as shares, futures, options.
            state.activetrades_selected_legs.clear();
            int numselected = 0;
            for (const auto& item : *lp.vec) {
                if (item.is_selected) {
                    numselected++;
                    if (item.leg) {
                        state.activetrades_selected_legs.push_back(item.leg);
                    }
                }
            }

            // Determine what exists within the trade such as shares, futures, options.
            for (const auto& trans : ld.trade->transactions) {
                for (const auto& leg : trans->legs) {
                    if (!leg->isOpen()) continue;
                    switch (trans->underlying) {
                    case Underlying::Options:
                        state.activetrades_rightclickmenu_options_exist = true;
                        break;
                    case Underlying::Shares:
                        state.activetrades_rightclickmenu_shares_count += leg->open_quantity;
                        break;
                    case Underlying::Futures:
                        state.activetrades_rightclickmenu_futures_count += leg->open_quantity;
                        break;
                    case Underlying::Dividend:
                    case Underlying::Other:
                    case Underlying::Nothing:
                      break;
                    }
                }
                state.activetrades_rightclickmenu_shares_exist = (bool)state.activetrades_rightclickmenu_shares_count;
                state.activetrades_rightclickmenu_futures_exist = (bool)state.activetrades_rightclickmenu_futures_count;
            }

            state.activetrades_rightclickmenu_multiselected = (numselected > 1) ? true : false;

            if (ld.is_selected && (ld.line_type == LineType::ticker_line)) {
               state.activetrades_rightclickmenu_linetype = RightClickMenuLineType::trade_header_line;
            }
            if (ld.is_selected && (ld.line_type == LineType::options_leg)) {
               state.activetrades_rightclickmenu_linetype = RightClickMenuLineType::options_leg_line;
            }
            if (ld.is_selected && (ld.line_type == LineType::shares)) {
               state.activetrades_rightclickmenu_linetype = RightClickMenuLineType::shares_line;
            }
            if (ld.is_selected && (ld.line_type == LineType::futures)) {
               state.activetrades_rightclickmenu_linetype = RightClickMenuLineType::futures_line;
            }
        }
    }
}


void SetupTableColumns(AppState& state, CListPanel& lp) {
    if (lp.vecHeader) {
        ImGui::TableSetupScrollFreeze(0, 1); // Make header row always visible
        for (auto& ld : *lp.vecHeader) {     // Should only be one item in the vector
            for (int colnum = 0; colnum < lp.column_count; ++colnum) {
                float column_width = state.dpi(lp.min_col_widths[colnum]);
                ImGui::TableSetupColumn(ld.col[colnum].text.c_str(), ImGuiTableColumnFlags_WidthFixed, column_width);
            }
        }
        ImGui::TableHeadersRow();
    } else {
        for (int colnum = 0; colnum < lp.column_count; ++colnum) {
            float column_width = state.dpi(lp.min_col_widths[colnum]);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, column_width);
        }
    }
}


void DrawListPanel(AppState& state, CListPanel& lp) {
    std::string table_id = std::to_string((int)lp.table_id);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, clrBackDarkGray(state));

    ImGui::BeginChild(table_id.c_str(), ImVec2(lp.panel_width, lp.panel_height));

    if (ImGui::BeginTable("##TableWithFullRowSelection", lp.column_count, lp.table_flags)) {
        SetupTableColumns(state, lp);

        // Fill table rows
        int id_gridline = 0;

        for (auto& ld : *lp.vec) {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(state.dpi(6), state.dpi(6)));
            ImGui::PushID(id_gridline);
            DrawTableRow(state, lp, ld);
            ImGui::PopID();
            ImGui::PopStyleVar();
            id_gridline++;
        }

        ImGui::EndTable();
    }

    ImGui::EndChild();

    ImGui::PopStyleColor(1);
}


