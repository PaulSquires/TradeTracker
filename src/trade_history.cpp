#include "imgui.h"
#include "imgui_stdlib.h"

#include <algorithm>

#include "appstate.h"
#include "list_panel.h"
#include "list_panel_data.h"
#include "utilities.h"

#include "trade_history.h"

//#include <iostream>


void SaveTradeHistoryNotes(AppState& state) {
    if (state.tradehistory_notes_modified) {
        state.tradehistory_trade->notes = state.tradehistory_notes;
        state.tradehistory_notes_modified = false;
        state.db.SaveDatabase(state);
    }
}


void SetTradeHistoryTrade(AppState& state, std::shared_ptr<Trade> trade) {
    if (state.tradehistory_trade != trade) {
        SaveTradeHistoryNotes(state);
    }
    state.tradehistory_trade = trade;
}


void LoadTradeHistory(AppState& state, std::vector<CListPanelData>& vec, std::shared_ptr<Trade> trade) {

    // Destroy any existing ListPanel line data
    vec.clear();
    vec.reserve(128);

    if (!trade) return;

    std::string ticker = trade->ticker_symbol + ": " + trade->ticker_name;
    if (state.config.IsFuturesTicker(trade->ticker_symbol)) ticker += " (" + AfxFormatFuturesDate(trade->future_expiry) + ")";

    state.tradehistory_ticker = ticker;
    state.tradehistory_notes = trade->notes;

    // Calculate the total gain/loss on any shares/futures
    trade->CalculateTotalSharesProfit(state);

    // Show the final rolled up Open position for this trade
    ListPanelData_OpenPosition(state, vec, trade, true);

    // Output the BP, Days, ROI
    ListPanelData_TradeROI(state, vec, trade, -1);

    // If this Trade has Shares/Futures transactions showing the costing would have been created
    // during the CalculateAdjustedCostBase function. We combine the SharesHistory vector with
    // all non-shares/futures transactions, sort them, and then display them.
    std::vector<HistoryItem> trade_history;
    trade_history.reserve(50);

    for (const auto& share : trade->shares_history) {
        HistoryItem item;
        item.trans_orig = share.trans;
        item.trans = std::make_shared<Transaction>();
        item.trans->underlying = share.trans->underlying;
        item.trans->description = share.trans->description;
        item.trans->trans_date = share.trans->trans_date;
        item.trans->quantity = share.open_quantity;
        item.trans->price = share.trans->price;
        item.trans->multiplier = trade->multiplier;
        item.trans->fees = share.trans->fees;
        item.trans->total = share.trans->total;
        item.trans->share_average_cost = share.average_cost;
        item.trans->share_action = share.trans->share_action;

        std::shared_ptr<Leg> leg = std::make_shared<Leg>();
        leg->action = share.leg_action;
        leg->open_quantity = share.open_quantity;
        leg->underlying = item.trans->underlying;
        item.trans->legs.push_back(leg);

        trade_history.push_back(item);
    }

    // Add all non-shares/futures transactions to the history
    for (const auto& trans : trade->transactions) {
        if (trans->underlying != Underlying::Shares &&
            trans->underlying != Underlying::Futures) {
            HistoryItem item;
            item.trans_orig = trans;
            item.trans = std::make_shared<Transaction>();
            item.trans = trans;
            trade_history.push_back(item);
        }
    }

    // Sort transactions based on date (latest first)
    std::sort(trade_history.begin(), trade_history.end(),
        [](const auto item1, const auto item2) {
            {
                if (item1.trans->trans_date > item2.trans->trans_date) return true;
                return false;
            }
        });


    for (auto& item : trade_history) {

        ListPanelData_HistoryHeader(state, vec, trade, item.trans, item.trans_orig);

        // Show the detail leg information for this transaction.
        for (const auto& leg : item.trans->legs) {
            switch (leg->underlying) {
            case Underlying::Options:
                ListPanelData_HistoryOptionsLeg(state, vec, trade, item.trans, leg);
                break;
            case Underlying::Shares:
            case Underlying::Futures:
                ListPanelData_HistorySharesLeg(state, vec, trade, item.trans, leg);
                break;
            case Underlying::Dividend:
                ListPanelData_HistoryDividendLeg(state, vec, trade, item.trans, leg);
                break;
            case Underlying::Other:
                ListPanelData_OtherIncomeLeg(state, vec, trade, item.trans, leg);
                break;
            case Underlying::Nothing:
              break;
            }
        }
    }

    state.is_tradehistory_data_loaded = true;
}


void ShowTradeHistory(AppState& state) {
    if (!state.show_tradehistory) return;

    static std::vector<CListPanelData> vec;

    static CListPanel lp;

    static std::shared_ptr<Trade> current_tradehistory_trade = nullptr;

    // Trigger a table reload if the currently displayed trade does not
    // equal the incoming trade.
    if (current_tradehistory_trade != state.tradehistory_trade) {
        state.is_tradehistory_data_loaded = false;
        current_tradehistory_trade = state.tradehistory_trade;
    }

    if (!state.is_tradehistory_data_loaded) {
        lp.table_id = TableType::trade_history;
        lp.is_left_panel = false;
        lp.table_flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY;
        lp.column_count = 9;
        lp.vec = &vec;
        lp.vecHeader = nullptr;
        lp.header_backcolor = 0;
        lp.header_height = 0;
        lp.row_height = 12;
        lp.min_col_widths = nHistoryMinColWidth;
        LoadTradeHistory(state, vec, state.tradehistory_trade);
    }

    ImGui::PushStyleColor(ImGuiCol_ChildBg, clrBackDarkGray(state));
    ImGui::BeginChild("TradeHistory", ImVec2(state.right_panel_width, state.top_panel_height));
    ImGui::PopStyleColor();

    ImGui::BeginGroup();

    if (!state.tradehistory_trade) {
        ImGui::PushStyleColor(ImGuiCol_Text, clrTextDarkWhite(state));
        ImGui::Text("Trade History");
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, clrTextDarkWhite(state));
        ImGui::Text("%s", state.tradehistory_ticker.c_str());
        ImGui::PopStyleColor();

        lp.panel_width = ImGui::GetContentRegionAvail().x;
        lp.panel_height = ImGui::GetContentRegionAvail().y - state.dpi(200);
        DrawListPanel(state, lp);

        ImGui::PushStyleColor(ImGuiCol_Text, clrTextDarkWhite(state));
        ImGui::SeparatorText("Notes");
        ImGui::PopStyleColor();

        // Push custom background and foreground colors
        ImGui::PushStyleColor(ImGuiCol_Border, clrBackDarkGray(state));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, clrBackDarkGray(state));
        ImGui::PushStyleColor(ImGuiCol_Text, clrTextLightWhite(state));
        ImGui::Indent(state.dpi(8));
        bool modified = ImGui::InputTextMultiline("##TradeHistoryMultiLine", &state.tradehistory_notes, ImVec2(-1, -1), ImGuiInputTextFlags_None);
        if (modified) state.tradehistory_notes_modified = true;
        ImGui::PopStyleColor(3);

        // If textbox loses focus but text had been modified then write it to the file
        if (!ImGui::IsItemFocused()) SaveTradeHistoryNotes(state);
    }

    ImGui::EndGroup();
    ImGui::EndChild();
}

