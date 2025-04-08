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
#include "implot.h"
#include "implot_internal.h"
#include <string>
#include <vector>
#include <algorithm>

#include "appstate.h"
#include "tws-client.h"
#include "list_panel.h"
#include "utilities.h"
#include "reconcile.h"
#include "active_trades_actions.h"
#include "import_dialog.h"

#include <iostream>


std::vector<ImportStruct> ibkr;    // persistent

// From tws-client.cpp signals when all position data has been received from the callback
extern bool positionEnd_fired;


// ========================================================================================
// Create the grouped trades into actual Trade/Transactions
// ========================================================================================
void ImportTrades_CreateTradeTransactions(AppState& state) {
    // Sort the data based on group_id
    std::sort(ibkr.begin(), ibkr.end(),
        [](const auto& trade1, const auto& trade2) {
            {
                if (trade1.group_id < trade2.group_id) return true;
                if (trade2.group_id < trade1.group_id) return false;
                return false;
            }
        });

    std::shared_ptr<Trade> trade;
    std::shared_ptr<Transaction> trans;
    std::shared_ptr<Leg> leg;

    int current_group_id = 0;

    for (const auto& p : ibkr) {
        if (p.group_id == 0) continue;

        if (p.group_id != current_group_id) {
            current_group_id = p.group_id;

            trade = std::make_shared<Trade>();
            trade->ticker_symbol = p.contract.symbol;
            trade->ticker_name = trade->ticker_symbol;
            trade->future_expiry = "";
            state.db.trades.push_back(trade);
        }

        int intQuantity = (int)intelDecimalToDouble(p.position);

        if (trade) {
            trans = std::make_shared<Transaction>();
            trans->trans_date = AfxCurrentDate();
            if (p.contract.secType == "OPT" ||
                p.contract.secType == "FOP") {
                trans->description = "Options";
                trans->underlying = Underlying::Options;
            }
            if (p.contract.secType == "STK") {
                trans->description = "Shares";
                trans->underlying = Underlying::Shares;
                trans->share_action = (intQuantity > 0) ? Action::BTO : Action::STO;
            }
            if (p.contract.secType == "FUT") {
                trans->description = "Futures";
                trans->underlying = Underlying::Futures;
                trans->share_action = (intQuantity > 0) ? Action::BTO : Action::STO;
            }

            trans->quantity = abs(intQuantity);
            trans->multiplier = AfxValDouble(p.contract.multiplier);
            trade->transactions.push_back(trans);

            // Add the new transaction legs
            leg = std::make_shared<Leg>();

            trade->nextleg_id += 1;
            leg->leg_id = trade->nextleg_id;
            leg->underlying = trans->underlying;
            leg->expiry_date = p.contract.lastTradeDateOrContractMonth;

            if (p.contract.secType == "FOP") {
                trade->future_expiry = leg->expiry_date;
            }

            std::string str = std::to_string(p.contract.strike);
            // Remove trailing zeroes
            str = str.substr(0, str.find_last_not_of('0') + 1);
            // If the decimal point is now the last character, remove that as well
            if (str.find('.') == str.size() - 1) {
                str = str.substr(0, str.size() - 1);
            }
            leg->strike_price = str;

            leg->put_call = state.db.StringToPutCall(p.contract.right);
            leg->trans = trans;
            leg->original_quantity = intQuantity;
            leg->open_quantity = intQuantity;
            leg->action = (intQuantity < 0) ? Action::STO : Action::BTO;

            trans->legs.push_back(leg);
        }

    }

    ReloadAppState(state);
    Reconcile_LoadAllLocalPositions(state);
}


// ========================================================================================
// Create and format the GroupId display string
// ========================================================================================
std::string Make_GroupId_String(int group_id) {
    std::string text = "00000" + std::to_string(group_id);
    return text.substr(text.size() - 5);
}


// ========================================================================================
// Information received from TwsClient::position callback
// ========================================================================================
void ImportTrades_position(const Contract& contract, Decimal position, double avg_cost) {
    // This callback is initiated by the reqPositions() call when the user requests to
    // import existing IBR positions (i.e. the database is empty).

    // Remove any existing version of the contract before adding it again
    auto end = std::remove_if(ibkr.begin(),
        ibkr.end(),
        [contract](ImportStruct const& p) {
            return (p.contract.conId == contract.conId) ? true : false;
        });
    ibkr.erase(end, ibkr.end());

    // Add the position the vector
    ImportStruct p{};
    p.contract = contract;
    p.position = position;
    p.avg_cost = avg_cost;
    if (p.contract.secType == "FOP" ||
        p.contract.secType == "FUT") {
        p.contract.symbol = "/" + p.contract.symbol;
    }
    ibkr.push_back(p);
}


// ========================================================================================
// Sort all IBKR positions in order to start to group similar trades together.
// ========================================================================================
void ImportTrades_doPositionSorting(){
    // Sort by Symbol, secType, lastTradeDateOrContractMonth
    // Sort based on Category and then TickerSymbol
    std::sort(ibkr.begin(), ibkr.end(),
        [](const auto& trade1, const auto& trade2) {
            {
                if (trade1.contract.symbol < trade2.contract.symbol) return true;
                if (trade2.contract.symbol < trade1.contract.symbol) return false;

                // symbol=symbol for primary condition, go to secType
                if (trade1.contract.secType < trade2.contract.secType) return true;
                if (trade2.contract.secType < trade1.contract.secType) return false;

                // secType=secType for primary condition, go to lastTradeDateOrContractMonth
                if (trade1.contract.lastTradeDateOrContractMonth < trade2.contract.lastTradeDateOrContractMonth) return true;
                if (trade2.contract.lastTradeDateOrContractMonth < trade1.contract.lastTradeDateOrContractMonth) return false;

                return false;
            }
        });
}


void LoadImportTableData(AppState& state, TableType table_id, std::vector<CListPanelData>& vec, std::vector<CListPanelData>& vecHeader) {
    CListPanelData ld;

    vec.clear();
    vecHeader.clear();

    TickerId ticker_id = -1;
    int font9 = 9;

    ld.line_type = LineType::ticker_line;
    ld.SetData(0, nullptr, ticker_id, "Ticker", StringAlignment::left, clrBackDarkGray(state), clrTextLightWhite(state), font9, false);
    ld.SetData(1, nullptr, ticker_id, "Type",   StringAlignment::left, clrBackDarkGray(state), clrTextLightWhite(state), font9, false);
    ld.SetData(2, nullptr, ticker_id, "Date",   StringAlignment::left, clrBackDarkGray(state), clrTextLightWhite(state), font9, false);
    ld.SetData(3, nullptr, ticker_id, "Pos",    StringAlignment::left, clrBackDarkGray(state), clrTextLightWhite(state), font9, false);
    ld.SetData(4, nullptr, ticker_id, "Strike", StringAlignment::left, clrBackDarkGray(state), clrTextLightWhite(state), font9, false);
    ld.SetData(5, nullptr, ticker_id, "P/C",    StringAlignment::left, clrBackDarkGray(state), clrTextLightWhite(state), font9, false);
    if (table_id == TableType::import_right) {
        ld.SetData(6, nullptr, ticker_id, "ID",     StringAlignment::left, clrBackDarkGray(state), clrTextLightWhite(state), font9, false);
    }
    vecHeader.push_back(ld);

    for (int i = 0; i < ibkr.size(); ++i) {
        auto* p = &ibkr.at(i);
        if (table_id == TableType::import_left && p->group_id != 0) continue;
        if (table_id == TableType::import_right && p->group_id == 0) continue;

        std::string str;
        ld.SetImportData(0, p, p->contract.symbol, StringAlignment::left, clrBackDarkGray(state), clrTextLightWhite(state), font9);
        ld.SetImportData(1, p, p->contract.secType, StringAlignment::left, clrBackDarkGray(state), clrTextLightWhite(state), font9);
        str = AfxInsertDateHyphens(p->contract.lastTradeDateOrContractMonth);
        ld.SetImportData(2, p, str, StringAlignment::left, clrBackDarkGray(state), clrTextLightWhite(state), font9);
        str = AfxRSet(std::to_string((int)intelDecimalToDouble(p->position)), 10);
        ld.SetImportData(3, p, str, StringAlignment::left, clrBackDarkGray(state), clrTextLightWhite(state), font9);

        str = (p->contract.strike) ? std::to_string(p->contract.strike) : "";
        // Remove trailing zeroes
        str = str.substr(0, str.find_last_not_of('0') + 1);
        // If the decimal point is now the last character, remove that as well
        if (str.find('.') == str.size() - 1) {
            str = str.substr(0, str.size() - 1);
        }
        ld.SetImportData(4, p, str, StringAlignment::left, clrBackDarkGray(state), clrTextLightWhite(state), font9);

        ld.SetImportData(5, p, p->contract.right, StringAlignment::left, clrBackDarkGray(state), clrTextLightWhite(state), font9);

        if (table_id == TableType::import_right) {
            str = Make_GroupId_String(p->group_id);
            ld.SetImportData(6, p, str, StringAlignment::left, clrBackDarkGray(state), clrTextLightWhite(state), font9);
        }
        vec.push_back(ld);
    }
}


void SetupLeftImportTradesTable(AppState& state, CListPanel& lp, std::vector<CListPanelData>& vec, std::vector<CListPanelData>& vecHeader) {
    lp.table_id = TableType::import_left;
    lp.is_left_panel = true;
    lp.table_flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY;
    lp.column_count = 6;
    lp.vec = &vec;
    lp.vecHeader = &vecHeader;
    lp.header_backcolor = 0;
    lp.header_height = 12;
    lp.row_height = 12;
    lp.min_col_widths = nImportMinColWidth;
    LoadImportTableData(state, lp.table_id, vec, vecHeader);
}


void SetupRightImportTradesTable(AppState& state, CListPanel& lp, std::vector<CListPanelData>& vec, std::vector<CListPanelData>& vecHeader) {
    lp.table_id = TableType::import_right;
    lp.is_left_panel = true;
    lp.table_flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY;
    lp.column_count = 7;
    lp.vec = &vec;
    lp.vecHeader = &vecHeader;
    lp.header_backcolor = 0;
    lp.header_height = 12;
    lp.row_height = 12;
    lp.min_col_widths = nImportMinColWidth;
    LoadImportTableData(state, lp.table_id, vec, vecHeader);
}


void ShowImportDialogPopup(AppState& state) {
    // Wait until other modal dialogs are closed (eg. License dialog)
    if (state.is_modal_active()) return;

    // Only process once we are connected to TWS.
    if (!tws_IsConnected(state)) return;

    if (ibkr.empty()) return;
    if (!positionEnd_fired) return;   // ensure all positions are loaded in ibkr

    static bool first_run = true;

    if (first_run) {
        if (state.db.trades.empty()) {
            state.show_importdialog_popup = true;
        } else {
            // Trades exist so we will not be allowing an import so no need
            // to keep the ibkr data.
            ibkr.clear();
        }
        first_run = false;
    }

    if (!state.show_importdialog_popup) return;

    static bool is_first_open = false;
    bool close_dialog = false;

    float x_window_padding = state.dpi(40.0f);
    float y_window_padding = state.dpi(10.0f);

    static std::vector<CListPanelData> vec_left;
    static std::vector<CListPanelData> vec_right;
    static std::vector<CListPanelData> vecHeader_left;
    static std::vector<CListPanelData> vecHeader_right;

    static CListPanel lp_left;
    static CListPanel lp_right;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(x_window_padding, y_window_padding));
    ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, clrPopupBg(state));

    ImU32 back_color = clrBackMediumGray(state);
    ImU32 back_color_red = clrRed(state);
    ImU32 back_color_green = clrGreen(state);
    ImU32 text_color_white = clrTextLightWhite(state);
    ImU32 text_color_gray = clrTextDarkWhite(state);

    // Trigger the popup
    if (!ImGui::IsPopupOpen(state.id_importdialog_popup.c_str())) {
        ImVec2 size{state.dpi(1200.0f),state.dpi(600.0f)};
        ImGui::SetNextWindowSize(size);
    	ImGui::OpenPopup(state.id_importdialog_popup.c_str(), ImGuiPopupFlags_NoOpenOverExistingPopup);
        is_first_open = true;
    }

    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal(state.id_importdialog_popup.c_str(), NULL, ImGuiWindowFlags_NoDecoration)) {
        DialogTitleBar(state, "IMPORT TRADES", state.show_importdialog_popup, close_dialog);

        // if (is_first_open) {
        if (ImGui::IsWindowAppearing()) {
            ImportTrades_doPositionSorting();
            SetupLeftImportTradesTable(state, lp_left, vec_left, vecHeader_left);
            SetupRightImportTradesTable(state, lp_right, vec_right, vecHeader_right);
        }

        ImVec2 button_size{};

        ImVec2 child_size_left(state.dpi(480.0f), 0.0f);  // Width of 200, height set to auto-expand
        ImGui::BeginChild("ChildPanelLeft", child_size_left, true);
        DrawListPanel(state, lp_left);
        ImGui::EndChild();

        ImGui::SameLine();
        ImVec2 child_size_middle(state.dpi(140.0f), 0.0f);
        ImGui::BeginChild("ChildPanelMiddle", child_size_middle, true);
        button_size = ImVec2{state.dpi(100.0f), 0};
        ImGui::NewLine();
        ImGui::NewLine();
        if (ColoredButton(state, "Group >>", 20.0f, button_size, clrTextBrightWhite(state), clrGreen(state))) {
            // Loop through for is_checked == true and assign new group_id
            static int left_group_id = 0;
            left_group_id++;
            bool reload = false;
            for (auto& item : *lp_left.vec) {
                if (item.is_selected) {
                    item.ibkr_pointer->group_id = left_group_id;
                    reload = true;
                }
            }
            // If multiple selected lines in the right table were selected then ensure that they
            // are all grouped using the same group_id number.
            int right_group_id = 0;
            for (auto& item : *lp_right.vec) {
                if (item.is_selected) {
                    if (right_group_id == 0) right_group_id = item.ibkr_pointer->group_id;
                    item.ibkr_pointer->group_id = right_group_id;
                    reload = true;
                }
            }
            if (reload) {
                LoadImportTableData(state, lp_left.table_id, vec_left, vecHeader_left);
                LoadImportTableData(state, lp_right.table_id, vec_right, vecHeader_right);
            }
        }
        ImGui::NewLine();
        if (ColoredButton(state, "<< UnGroup", 20.0f, button_size, clrTextBrightWhite(state), clrRed(state))) {
            // Loop through for is_checked == true and assign new group_id = 0
            bool reload = false;
            for (auto& item : *lp_right.vec) {
                if (item.is_selected) {
                    item.ibkr_pointer->group_id = 0;
                    reload = true;
                }
            }
            if (reload) {
                LoadImportTableData(state, lp_left.table_id, vec_left, vecHeader_left);
                LoadImportTableData(state, lp_right.table_id, vec_right, vecHeader_right);
            }
        }
        ImGui::NewLine();
        ImGui::NewLine();
        ImGui::NewLine();
        ImGui::NewLine();
        ImGui::NewLine();
        if (ColoredButton(state, "SAVE", 20.0f, button_size, clrTextBrightWhite(state), clrGreen(state))) {
            ImportTrades_CreateTradeTransactions(state);
        }
        ImGui::NewLine();
        if (ColoredButton(state, "Cancel", 20.0f, button_size, clrTextBrightWhite(state), clrRed(state))) {
            close_dialog = true;
        }
        ImGui::EndChild();

        ImGui::SameLine();
        ImVec2 child_size_right(state.dpi(480.0f), 0.0f);
        ImGui::BeginChild("ChildPanelRight", child_size_right, true);
        DrawListPanel(state, lp_right);
        ImGui::EndChild();

        if (close_dialog) {
	        state.show_importdialog_popup = false;
            ImGui::CloseCurrentPopup();
        }

        if (is_first_open) is_first_open = false;

        ImGui::EndPopup();
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

