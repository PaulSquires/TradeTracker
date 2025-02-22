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
#include "list_panel.h"
#include "list_panel_data.h"
#include "questionbox.h"
#include "active_trades_actions.h"
#include "transaction_edit.h"
#include "utilities.h"

#include <iostream>


std::shared_ptr<Leg> GetLegBackPointer(std::shared_ptr<Trade> trade, int back_pointer_id) {
    if (back_pointer_id == 0) return nullptr;
    for (auto& trans : trade->transactions) {
        for (auto& leg : trans->legs) {
            if (leg->leg_id == back_pointer_id) {
                return leg;
            }
        }
    }
    return nullptr;
}


void EditTransaction(AppState& state) {
    state.show_tradedialog_popup = true;
    state.trade_action = TradeAction::edit_transaction;
}


void DeleteTransaction(AppState& state) {
    std::shared_ptr<Trade> trade = state.trans_edit_trade;
    std::shared_ptr<Transaction> trans = state.trans_edit_transaction;

    // Iterate the trades and look into each TransDetail vector to match the
    // transaction that we need to delete.
    auto iter = trade->transactions.begin();
    while (iter != trade->transactions.end()) {
        // If element matches the element to be deleted then delete it
        if (*iter == trans) {
            // Cycle through the legs being deleted to see if any contains a leg backpointer.
            // If yes, then retrieve the leg related to that pointer and update its open
            // quantity amount. Backpointers exist for Transactions that modify quantity
            // amounts like CLOSE, EXPIRE, ROLL.
            for (auto& leg : trans->legs) {
                if (leg->leg_back_pointer_id != 0) {
                    // Get the backpointer leg.
                    std::shared_ptr<Leg> LegBackPointer = nullptr;
                    LegBackPointer = GetLegBackPointer(trade, leg->leg_back_pointer_id);
                    if (LegBackPointer != nullptr) {
                        LegBackPointer->open_quantity = LegBackPointer->open_quantity - leg->original_quantity;
                    }
                }
            }
            iter = trade->transactions.erase(iter);
        }
        else
        {
            iter++;
        }
    }

    // If no more Transactions exist in the Trade then delete the Trade itself
    if (trade->transactions.size() == 0) {
        auto it = std::find(state.db.trades.begin(), state.db.trades.end(), trade);
        if (it != state.db.trades.end()) state.db.trades.erase(it);
    }
    else {
        // Calculate the Trade open status because we may have just deleted the
        // transaction that sets the trades open_quantity to zero.
        trade->SetTradeOpenStatus();
    }

    // Save/Load the modified data
    ReloadAppState(state);
}


std::string TransEditTickerName(AppState& state, std::shared_ptr<Trade> trade) {
    std::string text = trade->ticker_symbol + ": " + trade->ticker_name;
    if (trade->future_expiry.length()) {
        text = text + " ( " + AfxFormatFuturesDate(trade->future_expiry) + " )";
    }
    return text;
}


std::string TransEditTransPrice(AppState& state, std::shared_ptr<Trade> trade, std::shared_ptr<Transaction> trans) {
    // Construct the string to display the cost details of the selected transaction
    std::string dr_cr = (trans->total < 0) ? " DR" : " CR";
    std::string plus_minus = (trans->total < 0) ? " + " : " - ";
    std::string text;
    text = std::to_string(trans->quantity) + " @ " +
                AfxMoney(trans->price, state.config.GetTickerDecimals(trade->ticker_symbol), state) + plus_minus +
                AfxMoney(trans->fees, 2, state) + " = " +
                AfxMoney(std::abs(trans->total), 2, state) + dr_cr;
    return text;
}


void LoadTransEditData(AppState& state, std::vector<CListPanelData>& vec,
            std::shared_ptr<Trade> trade, const std::shared_ptr<Transaction> trans) {

    // Destroy any existing ListPanel line data
    vec.clear();
    vec.reserve(128);

    ListPanelData_HistoryHeader(state, vec, trade, trans, trans);

    // Show the detail leg information for this transaction.
    for (const auto& leg : trans->legs) {
        switch (leg->underlying) {
        case Underlying::Options:
            ListPanelData_HistoryOptionsLeg(state, vec, trade, trans, leg);
            break;
        case Underlying::Shares:
        case Underlying::Futures:
            ListPanelData_HistorySharesLeg(state, vec, trade, trans, leg);
            break;
        case Underlying::Dividend:
            ListPanelData_HistoryDividendLeg(state, vec, trade, trans, leg);
            break;
        case Underlying::Other:
            ListPanelData_OtherIncomeLeg(state, vec, trade, trans, leg);
            break;
        case Underlying::Nothing:
          break;
        }
    }

    state.is_transedit_data_loaded = true;
}


void ShowTransEdit(AppState& state) {

    if (!state.show_transedit) return;

    static std::vector<CListPanelData> vec;

    static CListPanel lp;

    if (!state.is_transedit_data_loaded) {
        lp.table_id = TableType::trans_edit;
        lp.is_left_panel = false;
        lp.table_flags = ImGuiTableFlags_ScrollY;
        lp.column_count = 9;
        lp.vec = &vec;
        lp.vecHeader = nullptr;
        lp.header_backcolor = 0;
        lp.header_height = 0;
        lp.row_height = 12;
        lp.min_col_widths = nHistoryMinColWidth;
        LoadTransEditData(state, vec, state.trans_edit_trade, state.trans_edit_transaction);
    }

    ImGui::BeginChild("TransEdit", ImVec2(state.right_panel_width, state.top_panel_height));

    ImGui::BeginGroup();
    TextLabel(state, "Transaction Details", 0, clrTextLightWhite(state), clrBackDarkBlack(state));

    ImGui::NewLine();
    std::string ticker_name = TransEditTickerName(state, state.trans_edit_trade);
    TextLabel(state, ticker_name.c_str(), 0, clrTextDarkWhite(state), clrBackDarkBlack(state));

    ImGui::NewLine();
    std::string trans_price = TransEditTransPrice(state, state.trans_edit_trade, state.trans_edit_transaction);
    TextLabel(state, trans_price.c_str(), 0, clrTextDarkWhite(state), clrBackDarkBlack(state));

    ImVec2 button_size{state.dpi(80.0f), 0};
    float x_position = 0;

    x_position = state.undpi(state.right_panel_width) - (80.0f * 3.0f) - 12.0f;
    if (ColoredButton(state, "EDIT", x_position, button_size, clrTextBrightWhite(state), clrGreen(state))) {
        EditTransaction(state);
    }

    x_position = state.undpi(state.right_panel_width) - (80.0f * 2.0f) - 6.0f;
    if (ColoredButton(state, "DELETE", x_position, button_size, clrTextBrightWhite(state), clrRed(state))) {
        CustomQuestionBox(state, "Confirm", "Are you sure you wish to DELETE this Transaction?", QuestionCallback::DeleteTransaction);
    }

    x_position = state.undpi(state.right_panel_width) - 80.0f;
    if (ColoredButton(state, "CANCEL", x_position, button_size, clrTextLightWhite(state), clrBackMediumGray(state))) {
        state.show_transedit = false;
        state.show_tradehistory = true;
    }

    ImGui::Spacing();
    ImGui::EndGroup();

    ImGui::BeginGroup();
    lp.panel_width = ImGui::GetContentRegionAvail().x;
    lp.panel_height = ImGui::GetContentRegionAvail().y;
    DrawListPanel(state, lp);
    ImGui::EndGroup();

    ImGui::EndChild();
}