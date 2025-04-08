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

#include <algorithm>
#include <vector>

#include "appstate.h"
#include "list_panel.h"
#include "list_panel_data.h"
#include "tab_panel.h"
#include "filter_panel.h"
#include "transaction_panel.h"


void SetFirstLineTransactions(AppState& state, std::vector<CListPanelData>& vec) {
    // Reset the selected trade whose Trade History will be shown for the first
    // selectable line in the list.
    state.transactions_selected_trade = nullptr;

    // ensure all other lines are not selected
    for (auto& ld : vec) {
        ld.is_selected = false;
    }

    // If the database was reloaded then attempt to reposition to the row that
    // selected prior tot he reload.
    if (state.db.is_previously_loaded && (state.transactions_current_row_index != -1)) {
        for (int i = 0; i < vec.size(); ++i) {
            if (i == state.transactions_current_row_index) {
                vec.at(i).is_selected = true;
                state.transactions_selected_trade = vec.at(i).trade;
                state.selected_tabpanelitem = TabPanelItem::Transactions;
                SelectTabPanelItem(state);
                break;
            }
        }
    }

    // Finally, if still no selection then use the first selectable line in the grid.
    if (!state.transactions_selected_trade) {
        for (auto& ld : vec) {
            if (!state.transactions_selected_trade && ld.line_type == LineType::ticker_line) {
                ld.is_selected = true;
                state.transactions_selected_trade = ld.trade;
                state.selected_tabpanelitem = TabPanelItem::Transactions;
                SelectTabPanelItem(state);
                break;
            }
        }
    }
}


void LoadTransactionsData(AppState& state, std::vector<CListPanelData>& vec) {
    std::string start_date = state.filterpanel_start_date;
    std::string end_date = state.filterpanel_end_date;
    std::string ticker = state.filterpanel_ticker_symbol;
    int selected_category = state.filterpanel_selected_category;

    if (selected_category == CATEGORY_END + 1) selected_category = CATEGORY_OTHER;
    if (selected_category == CATEGORY_END + 2) selected_category = CATEGORY_ALL;

    struct TransData {
        std::shared_ptr<Trade> trade;
        std::shared_ptr<Transaction> trans;
    };
    std::vector<TransData> tdata;
    tdata.reserve(2000);    // reserve space for 2000 Transactions

    for (auto& trade : state.db.trades) {
        for (auto& trans : trade->transactions) {
            if (ticker.length() > 0) {
                if (ticker != trade->ticker_symbol) continue;
            }

            if (selected_category != CATEGORY_ALL) {
                if (trade->category != selected_category) continue;
            }

            if (trans->trans_date < start_date || trans->trans_date > end_date) continue;

            TransData td;
            td.trade = trade;
            td.trans = trans;
            tdata.push_back(td);
        }
    }

    // Sort the vector based on most recent date then by ticker
    std::sort(tdata.begin(), tdata.end(),
        [](const TransData data1, const TransData data2) {
            {
                if (data1.trans->trans_date > data2.trans->trans_date) return true;
                if (data2.trans->trans_date > data1.trans->trans_date) return false;

                // a=b for primary condition, go to secondary
                if (data1.trade->ticker_symbol < data2.trade->ticker_symbol) return true;
                if (data2.trade->ticker_symbol < data1.trade->ticker_symbol) return false;

                return false;
            }
        });


     // Destroy any existing ListPanel line data
    vec.clear();
    vec.reserve(128);

    // Create the new Listbox data that will display for the Transactions
    double running_gross_total = 0;
    double running_fees_total = 0;
    double running_net_total = 0;

    for (const auto& td : tdata) {
        ListPanelData_OutputTransaction(state, vec, td.trade, td.trans);
        running_net_total += td.trans->total;
        running_fees_total += td.trans->fees;
        running_gross_total += (td.trans->total + td.trans->fees);
    }
    ListPanelData_OutputTransactionRunningTotal(state, vec, running_gross_total, running_fees_total, running_net_total);

    // If no transactions then add at least one line
    if (vec.size() == 0) {
        ListPanelData_AddBlankLine(state, vec);
    }

    state.is_transactions_data_loaded = true;
}


void ShowTransPanel(AppState& state) {
    if (!state.show_transpanel) return;

    static std::vector<CListPanelData> vec;

    static CListPanel lp;

    ImGui::BeginChild("TransPanel", ImVec2(state.left_panel_width, state.top_panel_height));

    // Must call ShowFilterPanel here prior to initializing the closed trades data because
    // ShowFilterPanel makes a call to SetFilterPanelStartEndDates which sets the dates
    // that the initialization code needs in order to properly filter the trades.
    ShowFilterPanel(state);

    if (!state.is_transactions_data_loaded) {
        lp.table_id = TableType::trans_panel;
        lp.is_left_panel = true;
        lp.table_flags = ImGuiTableFlags_ScrollY;
        lp.column_count = 10;
        lp.vec = &vec;
        lp.vecHeader = nullptr;
        lp.header_backcolor = 0;
        lp.header_height = 0;
        lp.row_height = 12;
        lp.min_col_widths = nTransMinColWidth;
        LoadTransactionsData(state, vec);

        // Default to display the Trade History for the first entry in the list.
        SetFirstLineTransactions(state, vec);
    }

    ImGui::BeginGroup();
    lp.panel_width = ImGui::GetContentRegionAvail().x;
    lp.panel_height = ImGui::GetContentRegionAvail().y;
    DrawListPanel(state, lp);
    ImGui::EndGroup();

    ImGui::EndChild();
}

