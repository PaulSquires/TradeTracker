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

#include <algorithm>
#include <string>
#include <unordered_map>

#include "appstate.h"
#include "utilities.h"
#include "list_panel_data.h"
#include "list_panel.h"
#include "tab_panel.h"
#include "reconcile.h"
#include "trade_history.h"
#include "tws-client.h"

#include "active_trades.h"
#include "active_trades_actions.h"

#include <iostream>



// Unfortunately these structures that have to
// be made global because the data needs to be updated in the TwsClient::tickPrice
// and TwsClient::updatePortfolio functions which are callbacks from the
// Interactive Brokers library (therefore I can't pass AppState into it).
// These maps are instantiated in tws-client.cpp
extern std::unordered_map<TickerId, TickerData> mapTickerData;
extern std::unordered_map<int, PortfolioData> mapPortfolioData;
extern double netliq_value;
extern double excessliq_value;
extern double maintenance_value;
extern bool is_connection_ready_for_data;
extern bool is_positions_ready_for_data;


std::string FormatAccountValue(AppState& state, double amount) {
    // Get the value and convert it into Million(M) or Thousands(K) amounts
    if (std::abs(amount) > 1000000) {
        amount = amount / 1000000;
        return AfxMoney(amount, 2, state) + "M";
    }
    else if (std::abs(amount) > 1000) {
        amount = amount / 1000;
        return AfxMoney(amount, 1, state) + "K";
    }
    else {
        return AfxMoney(amount, 2, state);
    }

    return "";
}


void SetFirstLineActiveTrades(AppState& state, std::vector<CListPanelData>& vec) {
    // Reset the selected trade whose Trade History will be shown for the first
    // selectable line in the list.
    state.activetrades_selected_trade = nullptr;

    // ensure all other lines are not selected
    for (auto& ld : vec) {
        ld.is_selected = false;
    }

    // If the database was reloaded then attempt to reposition to the row that
    // selected prior tot he reload.
    if (state.db.is_previously_loaded && (state.activetrades_current_row_index != -1)) {
        for (int i = 0; i < vec.size(); ++i) {
            if (i == state.activetrades_current_row_index) {
                vec.at(i).is_selected = true;
                state.activetrades_selected_trade = vec.at(i).trade;
                state.selected_tabpanelitem = TabPanelItem::ActiveTrades;
                SelectTabPanelItem(state);
                break;
            }
        }
    }

    // Finally, if still no selection then use the first selectable line in the grid.
    if (!state.activetrades_selected_trade) {
        for (auto& ld : vec) {
            if (!state.activetrades_selected_trade && ld.line_type == LineType::ticker_line) {
                ld.is_selected = true;
                state.activetrades_selected_trade = ld.trade;
                state.selected_tabpanelitem = TabPanelItem::ActiveTrades;
                SelectTabPanelItem(state);
                break;
            }
        }
    }
}


void DoAddOptionsToTrade(AppState& state) {
    state.show_tradedialog_popup = true;
    state.trade_action = TradeAction::add_options_to_trade;
    state.trade_action_option_type = TradeActionOptionType::none;
}


void DoAddPutToTrade(AppState& state) {
    state.show_tradedialog_popup = true;
    state.trade_action = TradeAction::add_options_to_trade;
    state.trade_action_option_type = TradeActionOptionType::short_put;
}


void DoAddCallToTrade(AppState& state) {
    state.show_tradedialog_popup = true;
    state.trade_action = TradeAction::add_options_to_trade;
    state.trade_action_option_type = TradeActionOptionType::short_call;
}


void DoCloseOptionsLegs(AppState& state) {
    state.show_tradedialog_popup = true;
    state.trade_action = TradeAction::close_leg;
}


void DoCloseAllOptionsLegs(AppState& state) {
    state.show_tradedialog_popup = true;
    state.trade_action = TradeAction::close_all_legs;
}


void DoCloseTrade(AppState& state) {
    state.show_tradedialog_popup = true;
    state.trade_action = TradeAction::close_all_legs;
}


void DoRollLegs(AppState& state) {
    state.show_tradedialog_popup = true;
    state.trade_action = TradeAction::roll_leg;
}


void DoManageShares(AppState& state) {
    state.show_tradedialog_popup = true;
    state.trade_action = TradeAction::manage_shares;
}


void DoManageFutures(AppState& state) {
    state.show_tradedialog_popup = true;
    state.trade_action = TradeAction::manage_futures;
}


void DoAddSharesToTrade(AppState& state) {
    state.show_tradedialog_popup = true;
    state.trade_action = TradeAction::add_shares_to_trade;
}


void DoAddFuturesToTrade(AppState& state) {
    state.show_tradedialog_popup = true;
    state.trade_action = TradeAction::add_futures_to_trade;
}


void DoOtherIncomeExpense(AppState& state) {
    state.show_tradedialog_popup = true;
    state.trade_action = TradeAction::other_income_expense;
}


void DoAddOtherIncomeExpenseToTrade(AppState& state) {
    state.show_tradedialog_popup = true;
    state.trade_action = TradeAction::add_income_expense_to_trade;
}


void DoAddDividendToTrade(AppState& state) {
    state.show_tradedialog_popup = true;
    state.trade_action = TradeAction::add_dividend_to_trade;
}


void DoCloseAllShares(AppState& state) {
    state.show_tradedialog_popup = true;
    state.trade_action = TradeAction::close_all_shares;
}


void DoCloseAllFutures(AppState& state) {
    state.show_tradedialog_popup = true;
    state.trade_action = TradeAction::close_all_futures;
}


void DoOptionAssignment(AppState& state) {
    state.show_assignment_popup = true;
}


void ShowActiveTradesRightClickPopup(AppState& state) {
    if (state.show_activetrades_rightclickmenu) {
        ImGui::OpenPopup("RightClickMenuPopup");
        state.show_activetrades_rightclickmenu = false;
    }

    if (ImGui::BeginPopup("RightClickMenuPopup")) {

        ImGui::PushStyleColor(ImGuiCol_Text, clrTextLightWhite(state));
        // Change background color when the item is hot tracked
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, clrSelection(state));

        bool is_futures_ticker = state.config.IsFuturesTicker(state.activetrades_selected_trade->ticker_symbol);

        if (state.activetrades_rightclickmenu_linetype == RightClickMenuLineType::trade_header_line) {
            if (state.activetrades_rightclickmenu_shares_exist) {
                if (ImGui::MenuItem("      Manage Shares"))         DoManageShares(state);
                ImGui::Separator();
            }

            if (state.activetrades_rightclickmenu_futures_exist) {
                if (ImGui::MenuItem("      Manage Futures"))        DoManageFutures(state);
                ImGui::Separator();
            }

            if (ImGui::MenuItem("      Add Options to Trade"))      DoAddOptionsToTrade(state);
            if (ImGui::MenuItem("      Add Put to Trade"))          DoAddPutToTrade(state);
            if (ImGui::MenuItem("      Add Call to Trade"))         DoAddCallToTrade(state);

            ImGui::Separator();
            if (!is_futures_ticker && !state.activetrades_rightclickmenu_shares_exist) {
                if (ImGui::MenuItem("      Add Shares to Trade"))   DoAddSharesToTrade(state);
            }
            if (is_futures_ticker && !state.activetrades_rightclickmenu_futures_exist) {
                if (ImGui::MenuItem("      Add Futures to Trade"))  DoAddFuturesToTrade(state);
            }
            if (state.activetrades_rightclickmenu_shares_exist) {
                if (ImGui::MenuItem("      Add Dividend to Trade")) DoAddDividendToTrade(state);
            }
            if (ImGui::MenuItem("      Add Income/Expense to Trade")) DoAddOtherIncomeExpenseToTrade(state);

            ImGui::Separator();

            // Determine what menu options to show based on what underlyings exist in the Trade
            if (state.activetrades_rightclickmenu_shares_exist) {
                if (!state.activetrades_rightclickmenu_futures_exist &&
                    !state.activetrades_rightclickmenu_options_exist) {
                    if (ImGui::MenuItem("      Close Trade"))       DoCloseTrade(state);
                }
                else {
                    if (ImGui::MenuItem("      Close All Shares"))  DoCloseAllShares(state);
                }
            }
            if (state.activetrades_rightclickmenu_futures_exist) {
                if (!state.activetrades_rightclickmenu_shares_exist &&
                    !state.activetrades_rightclickmenu_options_exist) {
                    if (ImGui::MenuItem("      Close Trade"))       DoCloseTrade(state);
                }
                else {
                    if (ImGui::MenuItem("      Close All Futures")) DoCloseAllFutures(state);
                }
            }
            if (state.activetrades_rightclickmenu_options_exist) {
                if (!state.activetrades_rightclickmenu_shares_exist &&
                    !state.activetrades_rightclickmenu_futures_exist) {
                    if (ImGui::MenuItem("      Close Trade"))           DoCloseTrade(state);
                }
                else {
                    if (ImGui::MenuItem("      Close All Option Legs")) DoCloseAllOptionsLegs(state);
                }
            }
        }


        std::string leg_label = (state.activetrades_rightclickmenu_multiselected) ? "Legs" : "Leg";

        if (state.activetrades_rightclickmenu_linetype == RightClickMenuLineType::options_leg_line) {
            std::string label = "      Roll " + leg_label;
            if (ImGui::MenuItem(label.c_str()))                     DoRollLegs(state);

            label = "      Expire " + leg_label;
            if (ImGui::MenuItem(label.c_str()))                     AskExpireSelectedLegs(state);

            label = "      Close " + leg_label;
            if (ImGui::MenuItem(label.c_str()))                     DoCloseOptionsLegs(state);

            if (state.activetrades_rightclickmenu_multiselected == false) {
                // Only allow Option Assignment if selected Leg is SHORT quantity.
                if (state.activetrades_rightclickmenu_assignment_leg) {
                    ImGui::Separator();
                    if (ImGui::MenuItem("      Option Assignment")) DoOptionAssignment(state);
                }
            }

            if (state.activetrades_rightclickmenu_shares_exist) {
                ImGui::Separator();
                if (ImGui::MenuItem("      Manage Shares"))         DoManageShares(state);
            }

            if (state.activetrades_rightclickmenu_futures_exist) {
                ImGui::Separator();
                if (ImGui::MenuItem("      Manage Futures"))        DoManageFutures(state);
            }

            ImGui::Separator();
            if (ImGui::MenuItem("      Add Options to Trade"))      DoAddOptionsToTrade(state);
            if (ImGui::MenuItem("      Add Put to Trade"))          DoAddPutToTrade(state);
            if (ImGui::MenuItem("      Add Call to Trade"))         DoAddCallToTrade(state);

            if (is_futures_ticker) {
                if (!state.activetrades_rightclickmenu_futures_exist) {
                    if (ImGui::MenuItem("      Add Futures to Trade")) DoAddFuturesToTrade(state);
                }
            }
            else {
                if (!state.activetrades_rightclickmenu_shares_exist) {
                    if (ImGui::MenuItem("      Add Shares to Trade")) DoAddSharesToTrade(state);
                }
            }

            ImGui::Separator();
            if (state.activetrades_rightclickmenu_shares_exist) {
                if (ImGui::MenuItem("      Add Dividend to Trade"))   DoAddDividendToTrade(state);
            }
            if (ImGui::MenuItem("      Add Income/Expense to Trade")) DoAddOtherIncomeExpenseToTrade(state);
        }


        if (state.activetrades_rightclickmenu_linetype == RightClickMenuLineType::shares_line ||
            state.activetrades_rightclickmenu_linetype == RightClickMenuLineType::futures_line) {

            if (state.activetrades_rightclickmenu_shares_exist) {
                if (ImGui::MenuItem("      Manage Shares"))         DoManageShares(state);
                ImGui::Separator();
            }

            if (state.activetrades_rightclickmenu_futures_exist) {
                if (ImGui::MenuItem("      Manage Futures"))        DoManageFutures(state);
                ImGui::Separator();
            }

            if (ImGui::MenuItem("      Add Options to Trade"))      DoAddOptionsToTrade(state);
            if (ImGui::MenuItem("      Add Put to Trade"))          DoAddPutToTrade(state);
            if (ImGui::MenuItem("      Add Call to Trade"))         DoAddCallToTrade(state);

            ImGui::Separator();
            if (state.activetrades_rightclickmenu_shares_exist) {
                if (ImGui::MenuItem("      Add Dividend to Trade"))   DoAddDividendToTrade(state);
            }
            if (ImGui::MenuItem("      Add Income/Expense to Trade")) DoAddOtherIncomeExpenseToTrade(state);
        }

        ImGui::PopStyleColor(2);
        ImGui::EndPopup();
    }
}


void LoadActiveTradesData(AppState& state, std::vector<CListPanelData>& vec) {
    if (state.db.trades.empty()) {
        ListBoxData_NoTradesExistMessage(state, vec);
    }

    if (state.db.trades.size()) {
        // In case of newly added/deleted data ensure data is sorted.

        if (state.activetrades_filter_type == ActiveTradesFilterType::Category) {
            // Sort based on Category and then TickerSymbol
            std::sort(state.db.trades.begin(), state.db.trades.end(),
                [](const auto& trade1, const auto& trade2) {
                    {
                        if (trade1->category < trade2->category) return true;
                        if (trade2->category < trade1->category) return false;

                        // a=b for primary condition, go to secondary
                        if (trade1->ticker_symbol < trade2->ticker_symbol) return true;
                        if (trade2->ticker_symbol < trade1->ticker_symbol) return false;

                        return false;
                    }
                });
        }

        if (state.activetrades_filter_type == ActiveTradesFilterType::TickerSymbol) {
            // Sort based on TickerSymbol and Expiration
            std::sort(state.db.trades.begin(), state.db.trades.end(),
                [](const auto& trade1, const auto& trade2) {
                    {
                        if (trade1->ticker_symbol < trade2->ticker_symbol) return true;
                        if (trade2->ticker_symbol < trade1->ticker_symbol) return false;

                        // a=b for primary condition, go to secondary
                        if (trade1->earliest_legs_DTE < trade2->earliest_legs_DTE) return true;
                        if (trade2->earliest_legs_DTE < trade1->earliest_legs_DTE) return false;

                        return false;
                    }
                });
        }

        if (state.activetrades_filter_type == ActiveTradesFilterType::Expiration) {
            // Sort based on Expiration and TickerSymbol
            std::sort(state.db.trades.begin(), state.db.trades.end(),
                [](const auto& trade1, const auto& trade2) {
                    {
                        if (trade1->earliest_legs_DTE < trade2->earliest_legs_DTE) return true;
                        if (trade2->earliest_legs_DTE < trade1->earliest_legs_DTE) return false;

                        // a=b for primary condition, go to secondary
                        if (trade1->ticker_symbol < trade2->ticker_symbol) return true;
                        if (trade2->ticker_symbol < trade1->ticker_symbol) return false;

                        return false;
                    }
                });
        }

        if (state.activetrades_filter_type == ActiveTradesFilterType::TradeCompletedPercentage) {
            // Sort based on Percentage of Profit obtained so far for the Trade
            std::sort(state.db.trades.begin(), state.db.trades.end(),
                [](const auto& trade1, const auto& trade2) {
                    {
                        if (trade1->trade_completed_percentage > trade2->trade_completed_percentage) return true;
                        if (trade2->trade_completed_percentage < trade1->trade_completed_percentage) return false;
                        return false;
                    }
                });
        }

        if (state.activetrades_filter_type == ActiveTradesFilterType::SharesFuturesQuantity) {
            // Sort based on Quantity of Shares/Futures
            std::sort(state.db.trades.begin(), state.db.trades.end(),
                [](const auto& trade1, const auto& trade2) {
                    {
                        int total_shares_futures_1 = trade1->aggregate_shares + trade1->aggregate_futures;
                        int total_shares_futures_2 = trade2->aggregate_shares + trade2->aggregate_futures;

                        if (total_shares_futures_1 < total_shares_futures_2) return false;
                        if (total_shares_futures_2 < total_shares_futures_1) return true;

                        return false;
                    }
                });
        }

        // Destroy any existing ListPanel line data
        vec.clear();
        vec.reserve(128);

        // Create the new ListPanel line data.
        int category_header = -1;

        for (auto& trade : state.db.trades) {
            // We are displaying only open trades
            if (trade->is_open) {

                // Set the decimals for this tickerSymbol. Most will be 2 but futures can have a lot more.
                trade->ticker_decimals = state.config.GetTickerDecimals(trade->ticker_symbol);

                if (state.activetrades_filter_type == ActiveTradesFilterType::Category) {
                    if (trade->category != category_header) {

                        // Count the number of open trades in this category
                        auto num_trades_category{ std::ranges::count_if(state.db.trades,
                            [trade](auto t) {return (t->is_open && (t->category == trade->category)) ? true : false; })
                        };

                        ListPanelData_AddCategoryHeader(state, vec, trade, (int)num_trades_category);
                        category_header = trade->category;
                    }
                }

                ListPanelData_OpenPosition(state, vec, trade, false);
            }
        }
    }

    state.is_activetrades_data_loaded = true;
}


void Create45DayTradeDateLabel(AppState& state) {
    // Find the closest Friday date that is greater than 45 days from today.
    std::string today_date = AfxCurrentDate();
    std::string closest_45_date = AfxDateAddDays(today_date, 45);
    std::string calculated_day;
    std::string closest_45_friday;
    int days_from_now = 0;

    // Sunday(0) through Saturday(6)
    for (int i = 0; i < 7; ++i) {
        calculated_day = AfxDateAddDays(closest_45_date, i);
        if (AfxDateWeekday(
                AfxGetDay(calculated_day),
                AfxGetMonth(calculated_day),
                AfxGetYear(calculated_day)) == 5) {  // 5 = Friday
            closest_45_friday = calculated_day;
            days_from_now = i;
            break;
        }
    }

    std::string place_trade_date = AfxDateAddDays(today_date, days_from_now);

    std::string closest_45_friday_formatted =
        AfxGetShortMonthName(closest_45_friday) + " " +
        std::to_string(AfxGetDay(closest_45_friday)) + ", " +
        std::to_string(AfxGetYear(closest_45_friday));

    std::string place_trade_date_formatted =  //  Tues Aug 20, 2024
        AfxGetShortDayName(place_trade_date) + " " +
        AfxGetShortMonthName(place_trade_date) + " " +
        std::to_string(AfxGetDay(place_trade_date)) + ", " +
        std::to_string(AfxGetYear(place_trade_date));

    std::string days_from_now_formatted =
        (days_from_now == 0) ? " (TODAY)" : " (" + std::to_string(days_from_now) +
        (days_from_now == 1 ? " day" : " days") + " from now)";

    // Calculate if the closest Friday is a regular 3rd Friday of the month expiration.
    int yr = AfxGetYear(closest_45_friday);
    int mnth = AfxGetMonth(closest_45_friday);
    std::chrono::year_month_day third_friday_date{year(yr)/mnth/Friday[3]};
    std::chrono::day d = third_friday_date.day();
    if (AfxGetDay(closest_45_friday) == static_cast<unsigned>(d)) {
        closest_45_friday_formatted += " (REGULAR)";
    } else {
        closest_45_friday_formatted += " (WEEKLY)";
    }

    state.config.label_45day_trade_date =
        "Closest 45 day Friday expiration date is " + closest_45_friday_formatted +
        ". Place trades on " + place_trade_date_formatted + days_from_now_formatted;
}


void ShowTradeFilterComboBox(AppState& state) {
    static ImU32 text_color = clrTextDarkWhite(state);
    ImGui::SetNextItemWidth(state.dpi(220.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, clrBackMediumGray(state));  // Custom background color
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, clrBackMediumGray(state));  // Custom background color
    ImGui::PushStyleColor(ImGuiCol_PopupBg, clrBackMediumGray(state));  // Custom popup background color
    ImGui::PushStyleColor(ImGuiCol_Text, text_color);

    const char* tradefilter_items[] = {
        "By Category",
        "By Days to Expiration (DTE)",
        "By Ticker Symbol and DTE",
        "By Profit Percentage",
        "By Shares & Futures Quantity"
    };

    if (ImGui::BeginCombo("##TradesFilter", tradefilter_items[state.tradefilter_current_item])) {

        // Loop through items and create a selectable for each
        ImGui::PushStyleColor(ImGuiCol_Text, clrTextLightWhite(state));
        for (int i = 0; i < IM_ARRAYSIZE(tradefilter_items); i++) {
            bool is_selected = (state.tradefilter_current_item == i);

            // Change background color when the item is hot tracked
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, clrSelection(state));

            // Indent by using spaces
            std::string text = tradefilter_items[i];
            text = "      " + text;
            if (is_selected) text.replace(0,5,ICON_MD_CIRCLE_SMALL);
            if (ImGui::Selectable(text.c_str(), is_selected)) {
                state.tradefilter_current_item = i; // Update current item if selected
                // If a item is selected then check if it differs from the currently active
                // sort filter. If yes, then reload the ActiveTrades grid.
                if ((int)state.activetrades_filter_type != state.tradefilter_current_item) {
                    state.activetrades_filter_type = (ActiveTradesFilterType)state.tradefilter_current_item;
                    state.is_activetrades_data_loaded = false;  // this will force the table reload
                }
            }

            ImGui::PopStyleColor();  // pop hot tracking

            // Set the initial focus when opening the combo box
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::PopStyleColor();
        ImGui::EndCombo();
    }
    ImGui::PopStyleColor(4);

    text_color = (ImGui::IsItemHovered()) ? clrTextLightWhite(state) : clrTextDarkWhite(state);
}


void ShowNewTradeComboBox(AppState& state) {
    static ImU32 text_color = clrTextDarkWhite(state);
    ImGui::SetNextItemWidth(state.dpi(220.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, clrBackMediumGray(state));  // Custom background color
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, clrBackMediumGray(state));  // Custom background color
    ImGui::PushStyleColor(ImGuiCol_PopupBg, clrBackMediumGray(state));  // Custom popup background color
    ImGui::PushStyleColor(ImGuiCol_Text, text_color);

    const char* newtrade_items[] = {
        "Custom Options Trade",
        "-",
        "Iron Condor",
        "Short Strangle",
        "Short Put",
        "Short Put Vertical",
        "Short Call",
        "Short Call Vertical",
        "Short Put 112",
        "-",
        "Shares Trade",
        "Futures Trade",
        "-",
        "Other Income/Expense"
     };

    if (ImGui::BeginCombo("##NewTrade", newtrade_items[state.newtrade_current_item], ImGuiComboFlags_HeightLargest)) {
        // Loop through items and create a selectable for each
        ImGui::PushStyleColor(ImGuiCol_Text, clrTextLightWhite(state));
        for (int i = 0; i < IM_ARRAYSIZE(newtrade_items); i++) {
            bool is_selected = (state.newtrade_current_item == i);

            if (strcmp(newtrade_items[i], "-") == 0) {
                ImGui::PushStyleColor(ImGuiCol_Separator, clrBackLightGray(state));
                ImGui::PushStyleColor(ImGuiCol_SeparatorActive, clrBackLightGray(state));
                ImGui::Separator();
                ImGui::PopStyleColor(2);
            } else {

                // Change background color when the item is hot tracked
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, clrSelection(state));

                // Indent by using spaces
                std::string text = newtrade_items[i];
                text = "      " + text;
                if (is_selected) text.replace(0,5,ICON_MD_CIRCLE_SMALL);
                if (ImGui::Selectable(text.c_str(), is_selected)) {
                    state.newtrade_current_item = i; // Update current item if selected
                    state.show_tradedialog_popup = true;

                    switch ((int)state.newtrade_current_item) {
                    case 0:   // "Custom Options Trade",
                        state.trade_action = TradeAction::new_options_trade;
                        state.trade_action_option_type = TradeActionOptionType::none;
                        break;
                    case 1:   // "-",
                        break;
                    case 2:   // "Iron Condor",
                        state.trade_action = TradeAction::new_options_trade;
                        state.trade_action_option_type = TradeActionOptionType::short_iron_condor;
                        break;
                    case 3:   // "Short Strangle",
                        state.trade_action = TradeAction::new_options_trade;
                        state.trade_action_option_type = TradeActionOptionType::short_strangle;
                        break;
                    case 4:   // "Short Put",
                        state.trade_action = TradeAction::new_options_trade;
                        state.trade_action_option_type = TradeActionOptionType::short_put;
                        break;
                    case 5:   // "Short Put Vertical",
                        state.trade_action = TradeAction::new_options_trade;
                        state.trade_action_option_type = TradeActionOptionType::short_put_vertical;
                        break;
                    case 6:   // "Short Call",
                        state.trade_action = TradeAction::new_options_trade;
                        state.trade_action_option_type = TradeActionOptionType::short_call;
                        break;
                    case 7:   // "Short Call Vertical",
                        state.trade_action = TradeAction::new_options_trade;
                        state.trade_action_option_type = TradeActionOptionType::short_call_vertical;
                        break;
                    case 8:   // "Short Put 112",
                        state.trade_action = TradeAction::new_options_trade;
                        state.trade_action_option_type = TradeActionOptionType::short_put_LT112;
                        break;
                    case 9:   // "-",
                        break;
                    case 10:  // "Shares Trade",
                        state.trade_action = TradeAction::new_shares_trade;
                        break;
                    case 11:  // "Futures Trade",
                        state.trade_action = TradeAction::new_futures_trade;
                        break;
                    case 12:  // "-",
                        break;
                    case 13:  // "Other Income/Expense"
                        state.trade_action = TradeAction::other_income_expense;
                        break;
                    }

                }

                ImGui::PopStyleColor();  // pop hot tracking
            }

            // Set the initial focus when opening the combo box
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::PopStyleColor();
        ImGui::EndCombo();
    }
    ImGui::PopStyleColor(4);

    text_color = (ImGui::IsItemHovered() || ImGui::IsPopupOpen("##NewTrade"))
        ? clrTextLightWhite(state) : clrTextDarkWhite(state);
}


void ShowActiveTrades(AppState& state) {
    if (!state.show_activetrades) return;

    // Can not make this vector part of AppState because the CListPanelData
    // will cause a circular include reference. Use a void pointer instead
    // and cast it as needed when referenced from AppState.
    static std::vector<CListPanelData> vec;
    state.vecActiveTrades = static_cast<void*>(&vec);

    static TwsClient client;
    state.client = static_cast<void*>(&client);

    static CListPanel lp;

    if (!state.is_activetrades_data_loaded) {
        lp.table_id = TableType::active_trades;
        lp.is_left_panel = true;
        lp.table_flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY;
        // If this table is filtered by Category then we need to turn off the
        // cell clipping so that the Category name will expand across multiple columns.
        if (state.activetrades_filter_type == ActiveTradesFilterType::Category) lp.table_flags = lp.table_flags | ImGuiTableFlags_NoClip;
        lp.column_count = MAX_COLUMNS - 1;
        lp.vec = &vec;
        lp.vecHeader = nullptr;
        lp.header_backcolor = 0;
        lp.header_height = 0;
        lp.row_height = 12;
        lp.min_col_widths = nTradesMinColWidth;
        LoadActiveTradesData(state, vec);

        // Calculate the closest 45 day trade expiration date. This is show at the top
        // of the grid (per Configuration setting). This creates the label that is shown.
        Create45DayTradeDateLabel(state);

        // Default to display the Trade History for the first entry in the list.
        SetFirstLineActiveTrades(state, vec);
        SetTradeHistoryTrade(state, state.activetrades_selected_trade);
    }


    if (tws_IsConnected(state) && is_connection_ready_for_data) {
        // Load all local positions in vector
        Reconcile_LoadAllLocalPositions(state);

        // Request Account Summary in order to get liquidity amounts
        tws_RequestAccountSummary(state);

        // The is_positions_ready_for_data flag will be true when all positions have been
        // received from TWS.
        tws_RequestPositions(state);

        is_connection_ready_for_data = false;
    }

    // Extern bool global set when positionEnd() callback fires in tws-client.cpp
    if (tws_IsConnected(state) && is_positions_ready_for_data) {
        // Request Positions has completed and has sent this notification so
        // we can now start requesting the portfolio updates real time data.
        client.RequestPortfolioUpdates();

        // Start getting market data (ticker price data & option leg deltas) for each active ticker.
        for (int row = 0; row < vec.size(); ++row) {
            CListPanelData* ld = &vec.at(row);
            if (ld && (ld->line_type == LineType::ticker_line || ld->line_type == LineType::options_leg)) {
                client.RequestMarketData(state, ld);
            }
        }
        is_positions_ready_for_data = false;

		// Update the ticker prices and position information now rather than waiting for the thread to process
        UpdateTickerPrices(state);
    }

    ImGui::PushStyleColor(ImGuiCol_ChildBg, clrBackDarkBlack(state));
    ImGui::BeginChild("ActiveTrades", ImVec2(state.left_panel_width, state.top_panel_height));
    ImGui::SetCursorPosY(state.dpi(10));
    ImGui::Spacing();

    ImGui::BeginGroup();
    ImGui::PushStyleColor(ImGuiCol_Text, clrTextDarkWhite(state));
    ImGui::SameLine(state.dpi(20.0f));
    ImGui::Text("Sort Filter");
    ImGui::PopStyleColor();
    ImGui::NewLine();
    ImGui::SameLine(state.dpi(20.0f));
    ShowTradeFilterComboBox(state);
    ImGui::EndGroup();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(state.dpi(3), state.dpi(3)));
    ImGui::SameLine(state.dpi(290.0f));
    ImGui::BeginGroup();
    if (state.config.show_portfolio_value) {
        ImGui::PushStyleColor(ImGuiCol_Text, clrTextDarkWhite(state));
        ImGui::Text("Net Liq:");
        ImGui::Text("Excess Liq:");
        ImGui::Text("Maintenance:");
        ImGui::PopStyleColor();
    }
    ImGui::EndGroup();

    ImGui::SameLine(state.dpi(390.0f));
    ImGui::BeginGroup();
    if (state.config.show_portfolio_value) {
        ImGui::PushStyleColor(ImGuiCol_Text, clrTextBrightWhite(state));
        ImGui::Text("%s", FormatAccountValue(state, netliq_value).c_str());
        ImGui::Text("%s", FormatAccountValue(state, excessliq_value).c_str());
        ImGui::Text("%s", FormatAccountValue(state, maintenance_value).c_str());
        ImGui::PopStyleColor();
    }
    ImGui::EndGroup();
    ImGui::PopStyleVar();

    ImGui::SameLine(state.dpi(500.0f));
    ImGui::BeginGroup();
    ImGui::PushStyleColor(ImGuiCol_Text, clrTextDarkWhite(state));
    ImGui::Text("New Trade");
    ImGui::PopStyleColor();
    ShowNewTradeComboBox(state);
    ImGui::EndGroup();

    if (state.config.show_45day_trade_date || state.is_paper_trading) {
        ImGui::BeginGroup();
        ImGui::Spacing();
        ImGui::Spacing();
        if (state.is_paper_trading) {
            TextLabel(state, "** USING PAPER TRADING ACCOUNT **", 20.0f, clrYellow(state), clrBackDarkBlack(state));
            ImGui::NewLine();
        }
        if (state.config.show_45day_trade_date) {
            TextLabel(state, state.config.label_45day_trade_date.c_str(), 20.0f, clrTextDarkWhite(state), clrBackDarkBlack(state));
        }
        ImGui::Spacing();
        ImGui::EndGroup();
    }

    ImGui::BeginGroup();
    ImGui::Spacing();
    lp.panel_height = ImGui::GetContentRegionAvail().y;

    // Add a same color control to the left of the grid in order to give the
    // illusion that the grid has some left margin.
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 min{0.0f, ImGui::GetCursorPos().y + state.dpi(8.0f)};
    ImVec2 max{state.dpi(40.0f), min.y + lp.panel_height};
    draw_list->AddRectFilled(min, max, clrBackDarkGray(state));
    ImVec2 pos{state.dpi(28.0f), ImGui::GetCursorPos().y + state.dpi(8.0f)};
    ImGui::SetNextWindowPos(pos);

    lp.panel_width = ImGui::GetContentRegionAvail().x - state.dpi(20.0f);

    ImGui::SameLine();
    DrawListPanel(state, lp);
    ImGui::EndGroup();

    ImGui::EndChild();
    ImGui::PopStyleColor();

}

