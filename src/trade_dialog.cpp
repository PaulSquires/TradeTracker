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
#include "imgui_stdlib.h"
#include "implot.h"
#include "implot_internal.h"
//#include <ctime>
#include <string>
#include <vector>
#include <algorithm>

#include "appstate.h"
#include "utilities.h"
#include "icons_material_design.h"
#include "trade_dialog.h"
#include "trade_dialog_save.h"
#include "trade_dialog_initialize.h"
#include "categories.h"
#include "messagebox.h"


// #include <iostream>


bool IsNewTradeAction(TradeAction action) {
    switch (action) {
    case TradeAction::new_options_trade:
    case TradeAction::new_shares_trade:
    case TradeAction::new_futures_trade:
        return true;
    default:
        return false;
    }
}


bool IsEditTradeAction(TradeAction action) {
    switch (action) {
    case TradeAction::edit_transaction:
        return true;
    default:
        return false;
    }
}


bool IsCloseLegTradeAction(TradeAction action) {
    switch (action) {
    case TradeAction::close_leg:
    case TradeAction::close_all_legs:
        return true;
    default:
        return false;
    }
}


bool IsNewOptionsTradeAction(TradeAction action) {
    switch (action) {
    case TradeAction::new_options_trade:
        return true;
    default:
        return false;
    }
}


bool IsAddSharesToTradeAction(TradeAction action) {
    switch (action) {
    case TradeAction::add_shares_to_trade:
        return true;
    default:
        return false;
    }
}


bool IsAddFuturesToTradeAction(TradeAction action) {
    switch (action) {
    case TradeAction::add_futures_to_trade:
        return true;
    default:
        return false;
    }
}


bool IsAddSharesFuturesToTradeAction(TradeAction action) {
    switch (action) {
    case TradeAction::add_shares_to_trade:
    case TradeAction::add_futures_to_trade:
        return true;
    default:
        return false;
    }
}


bool IsAddOptionsToTradeAction(TradeAction action) {
    switch (action) {
    case TradeAction::add_options_to_trade:
        return true;
    default:
        return false;
    }
}


bool IsAddToTradeAction(TradeAction action) {
    switch (action) {
    case TradeAction::add_options_to_trade:
    case TradeAction::add_dividend_to_trade:
    case TradeAction::add_shares_to_trade:
    case TradeAction::add_futures_to_trade:
    case TradeAction::add_income_expense_to_trade:
        return true;
    default:
        return false;
    }
}


bool IsManageSharesTradeAction(TradeAction action) {
    switch (action) {
    case TradeAction::manage_shares:
    case TradeAction::close_all_shares:
        return true;
    default:
        return false;
    }
}


bool IsManageFuturesTradeAction(TradeAction action) {
    switch (action) {
    case TradeAction::manage_futures:
    case TradeAction::close_all_futures:
        return true;
    default:
        return false;
    }
}


bool IsManageSharesFuturesTradeAction(TradeAction action) {
    switch (action) {
    case TradeAction::manage_shares:
    case TradeAction::manage_futures:
    case TradeAction::close_all_shares:
    case TradeAction::close_all_futures:
        return true;
    default:
        return false;
    }
}


bool IsNewFuturesTradeAction(TradeAction action) {
    if (action == TradeAction::new_futures_trade) {
        return true;
    }
    return false;
}


bool IsRollLegsTradeAction(TradeAction action) {
    if (action == TradeAction::roll_leg) {
        return true;
    }
    return false;
}


bool IsNewSharesTradeAction(TradeAction action) {
    if (action == TradeAction::new_shares_trade) {
        return true;
    }
    return false;
}


bool IsNewSharesFuturesTradeAction(TradeAction action) {
    switch (action) {
    case TradeAction::new_shares_trade:
    case TradeAction::new_futures_trade:
        return true;
    default:
        return false;
    }
}


bool IsOtherIncomeExpense(TradeAction action) {
    switch (action) {
    case TradeAction::other_income_expense:
        return true;
    default:
        return false;
    }
}


bool IsAddOtherIncomeExpenseToTrade(TradeAction action) {
    switch (action) {
    case TradeAction::add_income_expense_to_trade:
        return true;
    default:
        return false;
    }
}


bool IsAddDividendToTrade(TradeAction action) {
    switch (action) {
    case TradeAction::add_dividend_to_trade:
        return true;
    default:
        return false;
    }
}


bool IsCloseTrade(TradeAction action) {
    switch (action) {
    case TradeAction::close_all_legs:
        return true;
    default:
        return false;
    }
}


bool IsCloseAllSharesFutures(TradeAction action) {
    switch (action) {
    case TradeAction::close_all_shares:
    case TradeAction::close_all_futures:
        return true;
    default:
        return false;
    }
}


std::string LookupTickerName(AppState& state, std::string& ticker_symbol) {
    // Attempt to lookup the specified Ticker and fill in the corresponding ticker name.
    std::string ticker_name;

    auto iter = std::find_if(state.db.trades.begin(), state.db.trades.end(),
        [&](const auto t) { return (t->ticker_symbol == ticker_symbol); });

    if (iter != state.db.trades.end()) {
        auto index = std::distance(state.db.trades.begin(), iter);
        ticker_name = state.db.trades.at(index)->ticker_name;
    }

    return ticker_name;
}


double LookupTickerMultiplier(AppState& state, std::string& ticker_symbol) {
    double multiplier = AfxValDouble(state.config.GetMultiplier(ticker_symbol));
    if (state.trade_action == TradeAction::new_shares_trade ||
        state.trade_action == TradeAction::manage_shares ||
        state.trade_action == TradeAction::add_dividend_to_trade ||
        state.trade_action == TradeAction::other_income_expense ||
        state.trade_action == TradeAction::add_shares_to_trade) {
        multiplier = 1;
    }
    return multiplier;
}


void CategoriesComboBox(AppState& state, std::string id_combo, const float x_position, const float width) {
    ImU32 text_color = clrTextDarkWhite(state);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, clrBackMediumGray(state));  
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, clrBackMediumGray(state));  
    ImGui::PushStyleColor(ImGuiCol_PopupBg, clrBackMediumGray(state));
    ImGui::PushStyleColor(ImGuiCol_Text, text_color);

    ImGui::SameLine(state.dpi(x_position));
    ImGui::SetNextItemWidth(state.dpi(width));

    int selected_category = 0;
    if (state.show_tradedialog_popup) {
        selected_category = state.category_current_item;
    } else {
        selected_category = state.filterpanel_selected_category;
        if (selected_category == CATEGORY_END + 1) selected_category = 100;
        if (selected_category == CATEGORY_END + 2) selected_category = 99;
    }

    std::string id = "##" + id_combo;
    if (ImGui::BeginCombo(id.c_str(), state.config.GetCategoryDescription(selected_category).c_str(), ImGuiComboFlags_HeightLarge)) {

        // Loop through items and create a selectable for each
        ImGui::PushStyleColor(ImGuiCol_Text, clrTextLightWhite(state));
        for (int i = CATEGORY_START; i <= CATEGORY_END; ++i) {
            bool is_selected = false;
            if (state.show_tradedialog_popup) {
                is_selected = (state.category_current_item == i); 
            } else {
                is_selected = (state.filterpanel_selected_category == i); 
            }

            // Change background color when the item is hot tracked
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, clrSelection(state));

            // Indent by using spaces
            std::string text = state.config.GetCategoryDescription(i);
            text = "      " + text;
            if (is_selected) text.replace(0,5,ICON_MD_CIRCLE_SMALL);
            if (ImGui::Selectable(text.c_str(), is_selected)) {
                if (state.show_tradedialog_popup) {
                    state.category_current_item = i;
                } else {
                    state.filterpanel_selected_category = i;
                    state.is_closedtrades_data_loaded = false;
                    state.is_transactions_data_loaded = false;
                }
            }
            
            ImGui::PopStyleColor();  // pop hot tracking

            // Set the initial focus when opening the combo box
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }

        // If this is not the TradeDialog then we need to all "Other Income/Expense" and
        // ""All Categories" for use by the FilterPanel.
        if (!state.show_tradedialog_popup) {
            // Change background color when the item is hot tracked
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, clrSelection(state));
            
            std::string text;
            bool is_selected = false;
         
            // OTHER INCOME/EXPENSES
            is_selected = (state.filterpanel_selected_category == CATEGORY_END + 1); 
            text = state.config.GetCategoryDescription(100);
            text = "      " + text;
            if (is_selected) text.replace(0,5,ICON_MD_CIRCLE_SMALL);
            if (ImGui::Selectable(text.c_str(), is_selected)) {
                state.filterpanel_selected_category = CATEGORY_END + 1;
                state.is_closedtrades_data_loaded = false;
                state.is_transactions_data_loaded = false;
            }
            if (is_selected) ImGui::SetItemDefaultFocus();

            // ALL CATEGORIES
            is_selected = (state.filterpanel_selected_category == CATEGORY_END + 2); 
            text = state.config.GetCategoryDescription(99);
            text = "      " + text;
            if (is_selected) text.replace(0,5,ICON_MD_CIRCLE_SMALL);
            if (ImGui::Selectable(text.c_str(), is_selected)) {
                state.filterpanel_selected_category = CATEGORY_END + 2;
                state.is_closedtrades_data_loaded = false;
                state.is_transactions_data_loaded = false;
            }
            if (is_selected) ImGui::SetItemDefaultFocus();

            ImGui::PopStyleColor();  // pop hot tracking
        }

        ImGui::PopStyleColor();
        ImGui::EndCombo();
    }
    ImGui::PopStyleColor(4);

    text_color = (ImGui::IsItemHovered()) ? clrTextLightWhite(state) : clrTextDarkWhite(state);
}


void StrategiesComboBox(AppState& state, std::string id_combo, std::string& put_call, const float x_position, const float width) {
    ImGui::PushStyleColor(ImGuiCol_FrameBg, clrBackMediumGray(state));  
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, clrBackMediumGray(state));  
    ImGui::PushStyleColor(ImGuiCol_PopupBg, clrBackMediumGray(state));
    ImGui::PushStyleColor(ImGuiCol_Text, clrTextLightWhite(state));

    ImGui::SameLine(state.dpi(x_position));
    ImGui::SetNextItemWidth(state.dpi(width));

    const char* strategy_items[] = {
        "VERTICAL",
        "STRANGLE",
        "STRADDLE",
        "OPTION",
        "IRON CONDOR",
        "BUTTERFLY",
        "RATIO SPREAD",
        "LT112"
    };

    std::string id = "##" + id_combo;
    if (ImGui::BeginCombo(id.c_str(), strategy_items[state.strategy_current_item], ImGuiComboFlags_HeightLarge)) {

        // Loop through items and create a selectable for each
        for (int i = 0; i < IM_ARRAYSIZE(strategy_items); i++) {
            bool is_selected = (state.strategy_current_item == i);

            // Change background color when the item is hot tracked
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, clrSelection(state));

            // Indent by using spaces
            std::string text = strategy_items[i];
            text = "      " + text;
            if (is_selected) text.replace(0,5,ICON_MD_CIRCLE_SMALL);

            if (ImGui::Selectable(text.c_str(), is_selected)) {
                state.strategy_current_item = i; // Update current item if selected
                if (state.strategy_current_item == (int)Strategy::IronCondor ||
                    state.strategy_current_item == (int)Strategy::Straddle ||
                    state.strategy_current_item == (int)Strategy::Strangle) {
                    put_call = "";
                } else {
                    if (put_call == "") put_call = "PUT";
                }
            }

            ImGui::PopStyleColor();  // pop hot tracking
            
            // Set the initial focus when opening the combo box
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PopStyleColor(4);
}


void ManageComboBox(AppState& state, std::string id_combo, Action& share_action, const float x_position, const float width) {
    ImGui::PushStyleColor(ImGuiCol_FrameBg, clrBackMediumGray(state));  
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, clrBackMediumGray(state));  
    ImGui::PushStyleColor(ImGuiCol_PopupBg, clrBackMediumGray(state));
    ImGui::PushStyleColor(ImGuiCol_Text, clrTextLightWhite(state));

    ImGui::SameLine(state.dpi(x_position));
    ImGui::SetNextItemWidth(state.dpi(width));

    const char* manage_items[] = {
        "Buy to Open (BTO)",
        "Sell to Close (STC)",
        "Sell to Open (STO)",
        "Buy to Close (BTC)"
    };

    // Map the items to the Action enum because they are not in the same order
    if (share_action == Action::BTO) state.manage_current_item = 0;
    if (share_action == Action::STC) state.manage_current_item = 1;
    if (share_action == Action::STO) state.manage_current_item = 2;
    if (share_action == Action::BTC) state.manage_current_item = 3;

    std::string id = "##" + id_combo;
    if (ImGui::BeginCombo(id.c_str(), manage_items[state.manage_current_item], ImGuiComboFlags_HeightSmall)) {

        // Loop through items and create a selectable for each
        for (int i = 0; i < IM_ARRAYSIZE(manage_items); i++) {
            bool is_selected = (state.manage_current_item == i);

            // Change background color when the item is hot tracked
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, clrSelection(state));

            // Indent by using spaces
            std::string text = manage_items[i];
            text = "      " + text;
            if (is_selected) text.replace(0,5,ICON_MD_CIRCLE_SMALL);

            if (ImGui::Selectable(text.c_str(), is_selected)) {
                state.manage_current_item = i; // Update current item if selected
                // Map the items to the Action enum because they are not in the same order
                if (state.manage_current_item == 0) share_action = Action::BTO ;
                if (state.manage_current_item == 1) share_action = Action::STC ;
                if (state.manage_current_item == 2) share_action = Action::STO ;
                if (state.manage_current_item == 3) share_action = Action::BTC ;
            }

            ImGui::PopStyleColor();  // pop hot tracking
            
            // Set the initial focus when opening the combo box
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PopStyleColor(4);
}


bool CalendarComboBox(AppState& state, std::string id_combo, 
                        std::string& iso_date, std::string& display_date, const ImGuiComboFlags flags, 
                        const float x_position, const float width) {
    ImU32 text_color = (state.show_tradedialog_popup) ? clrTextLightWhite(state) : clrTextDarkWhite(state);
    ImU32 text_color_calendar = clrTextBrightWhite(state);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, clrBackMediumGray(state));  
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, clrBackMediumGray(state));  
    ImGui::PushStyleColor(ImGuiCol_PopupBg, clrBackMediumGray(state));
    ImGui::PushStyleColor(ImGuiCol_Text, text_color);

    ImGui::SameLine(state.dpi(x_position));
    ImGui::SetNextItemWidth(state.dpi(width));

    bool result = false;

    id_combo = "##" + id_combo;
    if (ImGui::BeginCombo(id_combo.c_str(), display_date.c_str(), flags)) {
        static int level = 0;

        static ImPlotTime selected_date = 0;
        static ImPlotTime highlight_date = 0; 

        if (iso_date.length() == 0) {
            iso_date = AfxCurrentDate();
            display_date = AfxShortDate(iso_date);
        }

        if (selected_date == 0) {
            selected_date = ImPlot::MakeTime(AfxGetYear(iso_date), AfxGetMonth(iso_date)-1, AfxGetDay(iso_date)); 
           // selected_date = AfxUnixTime(iso_date);
            highlight_date = selected_date;
        }

        ImGui::PushStyleColor(ImGuiCol_Text, text_color_calendar);
        ImGui::PushStyleColor(ImGuiCol_Button, clrBlue(state));  // selected date
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, clrBlue(state));

        std::string id = "##" + std::to_string((int)ImGui::GetCursorScreenPos().x) + std::to_string((int)ImGui::GetCursorScreenPos().y);
        if (ImPlot::ShowDatePicker(id.c_str(), &level, &selected_date, &highlight_date)) {
            char buffer[32];
            ImPlot::FormatDate(selected_date, buffer, IM_ARRAYSIZE(buffer), ImPlotDateFmt_DayMoYr, true);
            iso_date = buffer;
            selected_date = 0;
            highlight_date = 0;
            ImGui::CloseCurrentPopup();
            result = true;
        } else {
            char buffer[32];
            ImPlot::FormatDate(selected_date, buffer, IM_ARRAYSIZE(buffer), ImPlotDateFmt_DayMoYr, true);
            iso_date = buffer;
            selected_date = 0;
            highlight_date = 0;
        } 

        ImGui::PopStyleColor(3);
        ImGui::EndCombo();
    } 

    ImGui::PopStyleColor(4);

    return result;
}


// Callback function to filter input
int InputTextCallbackNumbersOnly(ImGuiInputTextCallbackData* data) {
    if (data->EventFlag == ImGuiInputTextFlags_CallbackCharFilter) {
        if (data->EventChar < 256 && strchr(".0123456789-", data->EventChar) == NULL) {
            return 1;   // Ignore the character
        }
    }
    return 0;   // Accept the character
}


void TradeGridLine(AppState& state, TradeDialogData& tdd, std::vector<TradeGridData>& tgd, const int linenum) {
    ImU32 back_color_dark  = clrBackDarkGray(state);
    ImU32 back_color       = clrBackMediumGray(state);
    ImU32 text_color_white = clrTextBrightWhite(state);
    ImU32 text_color_gray  = clrTextDarkWhite(state);
    ImU32 text_color_red   = clrRed(state);
    ImU32 text_color_green = clrGreen(state);

    ImGui::PushStyleColor(ImGuiCol_Text, text_color_white);

    float hsp = 2.0f;
    float colspacer = 20.0f;
    float left = 47.0f;
    float width = 50.0f;
    
    std::string id;
    std::string display_date;

    ImVec2 button_size{state.dpi(width), 0};
    ImVec2 button_size_small{state.dpi(width/2), 0};

    for (int gridnum = 0; gridnum < tdd.grid_count; gridnum++) {
        int array_index = (gridnum * 4) + linenum;

        if (state.trade_action == TradeAction::edit_transaction) {
            id = "##originalquantity" + std::to_string(array_index);
            TextInput(state, id.c_str(), &tgd.at(array_index).original_quantity, ImGuiInputTextFlags_None, left, width, text_color_white, back_color);
            left = left + width + hsp;
        }

        id = "##quantity" + std::to_string(array_index);
        TextInput(state, id.c_str(), &tgd.at(array_index).quantity, ImGuiInputTextFlags_None, left, width, text_color_white, back_color);

        // Display the date picker
        left = left + width + hsp;
        id = "##expirydate" + std::to_string(array_index);
        // Change the ISO Date to a Short Date for display purposes
        display_date = AfxShortDate(tgd.at(array_index).expiry_date);
        if (CalendarComboBox(state, id.c_str(), tgd.at(array_index).expiry_date, display_date, 
                ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_HeightLarge, left, width)) {
            // If the first grid row (or fourth row for rolls) Expiry Date is being changed then update all other non empty rows.
            if (state.trade_action != TradeAction::edit_transaction) {
                if (array_index == 0) {
                    for (int i = 0; i < 4; ++i ) {
                        if (tgd.at(i).expiry_date.length()) {
                            tgd.at(i).expiry_date = tgd.at(array_index).expiry_date;
                        }
                    }
                }
                if (array_index == 4) {
                    for (int i = 4; i < 8; ++i ) {
                        if (tgd.at(i).expiry_date.length()) {
                            tgd.at(i).expiry_date = tgd.at(array_index).expiry_date;
                        }
                    }
                }
            }
        }
        
        left = left + width + hsp;
        ImGui::PushItemFlag(ImGuiItemFlags_NoTabStop, true);
        std::string dte;
        if (tgd.at(array_index).expiry_date.length()) {
            dte = std::to_string(AfxDaysBetween(tdd.transaction_date, tgd.at(array_index).expiry_date)) + "d";
        }
        TextLabelCentered(state, dte.c_str(), left, button_size, clrTextDarkWhite(state), back_color);
        ImGui::PopItemFlag();

        left = left + width + hsp;
        id = "##strike" + std::to_string(array_index);
           if (TextInput(state, id.c_str(), &tgd.at(array_index).strike_price, ImGuiInputTextFlags_CallbackCharFilter, 
                left, width, text_color_white, back_color, InputTextCallbackNumbersOnly)) {
            // Remove any negative signs
            tgd.at(array_index).strike_price = AfxRemove(tgd.at(array_index).strike_price, "-");
        }

        left = left + width + hsp;
        if (ColoredButton(state, tgd.at(array_index).put_call.c_str(), left, button_size_small, clrTextBrightWhite(state), back_color)) {
            tgd.at(array_index).put_call = (tgd.at(array_index).put_call == "P") ? "C" : "P";
        }

        left = left + (width / 2) + hsp;
        ImU32 txtclr = text_color_white;
        if (tgd.at(array_index).action == "STO" || tgd.at(array_index).action == "STC") txtclr = text_color_red;
        if (tgd.at(array_index).action == "BTO" || tgd.at(array_index).action == "BTC") txtclr = text_color_green;
        if (ColoredButton(state, tgd.at(array_index).action.c_str(), left, button_size, txtclr, back_color)) {
            if (tgd.at(array_index).action.length() == 0) tgd.at(array_index).action = "BTC";
            // Cycle through the actions
            if (tgd.at(array_index).action == "STO") {
                tgd.at(array_index).action = "BTO";
            } else if (tgd.at(array_index).action == "BTO") {
                tgd.at(array_index).action = "STC";
            } else if (tgd.at(array_index).action == "STC") {
                tgd.at(array_index).action = "BTC";
            } else if (tgd.at(array_index).action == "BTC") {
                tgd.at(array_index).action = "STO";
            }
        }

        left = left + width + hsp;
        id = ICON_MD_REPLY;
        if (ColoredSmallButton(state, ICON_MD_REPLY, left, text_color_gray, back_color_dark)) {
            tgd.at(array_index).original_quantity = "";
            tgd.at(array_index).quantity = "";
            tgd.at(array_index).expiry_date = "";
            tgd.at(array_index).dte = "";
            tgd.at(array_index).strike_price = "";
            tgd.at(array_index).put_call = "";
            tgd.at(array_index).action = "";
        }
        Tooltip(state, "Reset line", clrTextLightWhite(state), clrBackMediumGray(state));

        float grid_spacer = (IsEditTradeAction(state.trade_action) ? 30 : 80);
        left = left + grid_spacer;
    }

    ImGui::PopStyleColor();
}


void TradeGrid(AppState& state, TradeDialogData& tdd, std::vector<TradeGridData>& tgd) {
    int maxlines = 4;

    for (int linenum = 0; linenum < maxlines; linenum++) {
        TradeGridLine(state, tdd, tgd, linenum);
        ImGui::NewLine();
    }
    ImGui::NewLine();
}


void ShowTradeDialogPopup(AppState& state) {
	if (!state.show_tradedialog_popup) return;

    static bool is_first_open = false;
    bool close_dialog = false;

    float x_window_padding = state.dpi(40.0f);
    float y_window_padding = state.dpi(10.0f);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(x_window_padding, y_window_padding));
    ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, clrPopupBg(state));

    ImU32 back_color = clrBackMediumGray(state);
    ImU32 back_color_red = clrRed(state);
    ImU32 back_color_green = clrGreen(state);
    ImU32 short_long_back_color = back_color_red;
    ImU32 dr_cr_back_color = back_color_green;
    ImU32 text_color_white = clrTextLightWhite(state);
    ImU32 text_color_gray = clrTextDarkWhite(state);

    // Trigger the popup
    if (!ImGui::IsPopupOpen(state.id_tradedialog_popup.c_str())) {
        ImVec2 size{state.dpi(815.0f),state.dpi(600.0f)};
        ImGui::SetNextWindowSize(size);
    	ImGui::OpenPopup(state.id_tradedialog_popup.c_str(), ImGuiPopupFlags_NoOpenOverExistingPopup);
        is_first_open = true;
    }

    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal(state.id_tradedialog_popup.c_str(), NULL, ImGuiWindowFlags_NoDecoration)) {
        DialogTitleBar(state, "TRADE MANAGEMENT", state.show_tradedialog_popup, close_dialog);

        static TradeDialogData tdd{};
        static std::vector<TradeGridData> tgd(8);

        // if (is_first_open) {
        if (ImGui::IsWindowAppearing()) {    
            InitializeTradeDialogDefaults(state, tdd, tgd);
        }

        bool show_ticker = true;
        bool show_description = true;
        bool show_category = true;
        bool show_strategy = true;
        bool show_static_labels = false;    // AddTo/Edit will True
        bool show_contract_expiry = false;
        bool show_grid = true;
        bool show_manage_shares_futures = false;
        bool show_buying_power = true;
        bool show_dte_warnings = true;

        if (!tdd.ticker_symbol.length()) tdd.ticker_name = "";
        
        tdd.is_futures_ticker = state.config.IsFuturesTicker(tdd.ticker_symbol);
        show_contract_expiry = tdd.is_futures_ticker;

        if (IsCloseLegTradeAction(state.trade_action) ||
            IsRollLegsTradeAction(state.trade_action)) {
            show_static_labels = true;
            show_category = false;
            show_strategy = false;
            show_contract_expiry = false;
        }

        if (IsNewSharesFuturesTradeAction(state.trade_action)) {
            show_description = false;
            show_strategy = false;
            show_contract_expiry = IsNewFuturesTradeAction(state.trade_action);
            show_grid = false;
            show_manage_shares_futures = true;
            show_dte_warnings = false;
        }

        if (IsManageSharesFuturesTradeAction(state.trade_action) ||
            IsAddSharesFuturesToTradeAction(state.trade_action)) {
            show_description = false;
            show_static_labels = true;
            show_category = false;
            show_strategy = false;
            if (IsManageFuturesTradeAction(state.trade_action) ||
                IsAddFuturesToTradeAction(state.trade_action)) {
                show_contract_expiry = true;
            }
            show_grid = false;
            show_manage_shares_futures = true;
            show_dte_warnings = false;
        }

        if (IsAddOptionsToTradeAction(state.trade_action)) {
            show_static_labels = true;
            show_category = false;
        }

        if (IsOtherIncomeExpense(state.trade_action)) {
            show_ticker = false;
            show_category = false;
            show_strategy = false;
            show_buying_power = false;
            show_contract_expiry = false;
            show_grid = false;
            show_dte_warnings = false;
        }

        if (IsAddOtherIncomeExpenseToTrade(state.trade_action)) {
            show_static_labels = true;
            show_category = false;
            show_strategy = false;
            show_buying_power = false;
            show_contract_expiry = false;
            show_grid = false;
        }

        if (IsAddDividendToTrade(state.trade_action)) {
            show_static_labels = true;
            show_description = false;
            show_category = false;
            show_strategy = false;
            show_buying_power = false;
            show_contract_expiry = false;
            show_grid = false;
        }

        if (IsEditTradeAction(state.trade_action)) {
            show_static_labels = true;
            show_strategy = false;
            if (state.trans_edit_transaction->underlying == Underlying::Shares ||
                state.trans_edit_transaction->underlying == Underlying::Futures) {
                tdd.trade_grid1_label = 
                    (state.trans_edit_transaction->underlying == Underlying::Shares) ? "Shares Action" : "Futures Action";
                tdd.trade_grid2_label = "";
                show_description = false;
                show_contract_expiry = (state.trans_edit_transaction->underlying == Underlying::Futures);
                show_grid = false;
                show_manage_shares_futures = true;
            }
            if (state.trans_edit_transaction->underlying == Underlying::Dividend) {
                show_description = false;
                show_category = false;
                show_buying_power = false;
                show_contract_expiry = false;
                show_grid = false;
                tdd.trade_grid1_label = "";
                show_dte_warnings = false;
            }
        }


        ImVec2 button_size{}; 

        if (show_static_labels) {
            TextLabel(state, tdd.ticker_name.c_str(), 0.0f, text_color_white, back_color);
        }

        // Right-align the banner text
        ImVec2 label_size = ImGui::CalcTextSize(tdd.trade_dialog_banner.c_str());
        TextLabel(state, tdd.trade_dialog_banner.c_str(), 770.0f - state.undpi(label_size.x), text_color_white, back_color);
        ImGui::NewLine();

        if (show_static_labels) {
            std::string ticker_symbol_extended = tdd.ticker_symbol;
            if (tdd.is_futures_ticker) ticker_symbol_extended += ": " + AfxUpper(AfxShortDate(tdd.futures_expiry_date));
            TextLabel(state, ticker_symbol_extended.c_str(), 0.0f, text_color_gray, back_color);
        } else {
            if (show_ticker) {
                TextLabel(state, "Ticker", 0.0f, text_color_gray, back_color);
                TextLabel(state, "Underlying", 115.0f, text_color_gray, back_color);
            }
        }

        if (show_contract_expiry) {
            TextLabel(state, "Contract Expiry", 440.0f, text_color_gray, back_color);
        }
        if (show_category) {
            TextLabel(state, "Category", 552.0f, text_color_gray, back_color);
        }

        ImGui::NewLine();

        if (!show_static_labels) {
            if (show_ticker) {
                if (is_first_open) ImGui::SetKeyboardFocusHere();
                if (TextInput(state, "##ticker_symbol", &tdd.ticker_symbol, ImGuiInputTextFlags_CharsUppercase, 0.0f, 65.0f, text_color_white, back_color)) {
                    if (tdd.ticker_symbol.length()) {
                        tdd.ticker_name = LookupTickerName(state, tdd.ticker_symbol);
                        tdd.multiplier = LookupTickerMultiplier(state, tdd.ticker_symbol);
                    }
                }
                TextInput(state, "##ticker_name", &tdd.ticker_name, ImGuiInputTextFlags_None, 115.0f, 215.0f, text_color_white, back_color);
            }
        }

        if (show_contract_expiry) {
            CalendarComboBox(state, "FuturesExpiryDate", tdd.futures_expiry_date, tdd.futures_expiry_date, ImGuiComboFlags_HeightLarge, 440.0f, 102.0f);
        }
        
        // Category-start            
        if (show_category) {
            CategoriesComboBox(state, "Categories", 552.0f, 184.0f);
            button_size = ImVec2{state.dpi(24.0f), 0}; 
            if (ColoredButton(state, ICON_MD_EDIT, 742.0f, button_size, clrTextDarkWhite(state), back_color)) {
                state.show_categoriesdialog_popup = true;
            }
            tdd.category = state.category_current_item;
            Tooltip(state, "Edit Categories", clrTextLightWhite(state), clrBackMediumGray(state));
        }
        // Category-end

        ImGui::NewLine();
        TextLabel(state, "Date", 0.0f, text_color_gray, back_color);
        if (show_description) {
            TextLabel(state, "Description", 159.0f, text_color_gray, back_color);
        }
        if (show_strategy) {
            TextLabel(state, "Strategy", 440.0f, text_color_gray, back_color);
        }

        ImGui::NewLine();
        CalendarComboBox(state, "TransactionDate", tdd.transaction_date, tdd.transaction_date, ImGuiComboFlags_HeightLarge, 0.0f, 102.0f);
        if (show_description) {
            TextInput(state, "##Description", &tdd.description, ImGuiInputTextFlags_None, 159.0f, 171.0f, text_color_white, back_color);
        }

        // Strategy-start            
        if (show_strategy) {
            button_size = ImVec2{state.dpi(52.0f), 0}; 
            short_long_back_color = (tdd.short_long == "SHORT" ? back_color_red : back_color_green);
            if (ColoredButton(state, tdd.short_long.c_str(), 440.0f, button_size, clrTextBrightWhite(state), short_long_back_color)) {
                tdd.short_long = (tdd.short_long == "SHORT" ? "LONG" : "SHORT");
            }
            if (ColoredButton(state, tdd.put_call.c_str(), 494.0f, button_size, clrTextBrightWhite(state), back_color)) {
                tdd.put_call = (tdd.put_call == "PUT") ? "CALL" : "PUT";
            }
            StrategiesComboBox(state, "Strategies", tdd.put_call, 548.0f, 184.0f);
            button_size = ImVec2{state.dpi(30.0f), 0}; 
            if (ColoredButton(state, "GO", 735.0f, button_size, clrBackDarkBlack(state), clrBlue(state))) {
                CreateTradeActionOptionType(state, tdd);
                InitializeTradeDialogDefaults(state, tdd, tgd);
            }
        }
        // Strategy-end

        ImGui::NewLine();
        ImGui::NewLine();

        // TRADE GRID
        TextLabel(state, tdd.trade_grid1_label.c_str(), 0.0f, text_color_gray, back_color);
        TextLabel(state, tdd.trade_grid2_label.c_str(), 415.0f, text_color_gray, back_color);
        ImGui::NewLine();
        if (show_grid) {
            TradeGrid(state, tdd, tgd);
        }

        // MANAGE SHARES/FUTURES (grid = false)
        if (show_manage_shares_futures) {
            ManageComboBox(state, "ManageComboBox", tdd.share_action, 0, 200);
            ImGui::NewLine(); ImGui::NewLine();
            ImGui::NewLine(); ImGui::NewLine();
            ImGui::NewLine(); 
        } else {
            if (!show_grid) {
                // If Grid is not shown at this point then fill in with blank lines
                ImGui::NewLine(); ImGui::NewLine();
                ImGui::NewLine(); ImGui::NewLine();
                ImGui::NewLine(); 
            }
        }

        // PRICE DETAILS
        TextLabel(state, "Quantity", 0.0f, text_color_gray, back_color);
        TextLabel(state, "Price", 130.0f, text_color_gray, back_color);
        TextLabel(state, "Multiplier", 220.0f, text_color_gray, back_color);
        TextLabel(state, "Fees", 310.0f, text_color_gray, back_color);
        TextLabel(state, "Total", 400.0f, text_color_gray, back_color);
        if (show_buying_power) {
            TextLabel(state, "Buying Power", 580.0f, text_color_gray, back_color);
        }

        ImGui::NewLine();
        if (show_static_labels || IsOtherIncomeExpense(state.trade_action)) {
            if (is_first_open) ImGui::SetKeyboardFocusHere();
        }
        if (DoubleInput(state, "##quantity", &tdd.quantity, 0.0f, 80.0f, text_color_white, back_color)) {
            tdd.quantity = std::abs(tdd.quantity);
        }
        if (DoubleInput(state, "##price", &tdd.price, 130.0f, 80.0f, text_color_white, back_color)) {
            tdd.price = std::abs(tdd.price);
        }
        if (DoubleInput(state, "##multiplier", &tdd.multiplier, 220.0f, 80.0f, text_color_white, back_color)) {
            tdd.multiplier = std::abs(tdd.multiplier);
        }
        DoubleInput(state, "##fees", &tdd.fees, 310.0f, 80.0f, text_color_white, back_color);

        // If closing a trade or legs then we need to update the Transaction Quantity legs being closed
        // in order to handle the situation where partial closing legs. This only works for legs that 
        // have the same quantity amounts. The logic would fail on ratio spreads so test first for non
        // ratio legs.
        if (state.trade_action == TradeAction::close_leg ||
            state.trade_action == TradeAction::close_all_legs) {

            bool is_legs_equal = true;
            int table_quantity = std::abs(AfxValInteger(tgd.at(0).quantity));
            for (int i = 1; i < 4; ++i) {
                int table_quantity_next = std::abs(AfxValInteger(tgd.at(i).quantity));
                if (table_quantity != table_quantity_next && table_quantity_next != 0) {
                    is_legs_equal = false;
                    break;
                }
            }

            if (is_legs_equal) {
                int quantity = static_cast<int>(tdd.quantity);
                if (quantity > 0) {
                    for (int i = 0; i < 4; ++i) {
                        table_quantity = AfxValInteger(tgd.at(i).quantity);
                        if (table_quantity > 0) tgd.at(i).quantity = std::to_string(quantity);
                        if (table_quantity < 0) tgd.at(i).quantity = std::to_string(-quantity);
                    }
                }
            }
        }

        tdd.total = (tdd.quantity * tdd.multiplier * tdd.price);
        if (tdd.dr_cr == "CR") {
            tdd.total = tdd.total +(tdd.fees * -1);
        } else {
            tdd.total = tdd.total +tdd.fees;
        }

        DoubleInput(state, "##total", &tdd.total, 400.0f, 80.0f, text_color_white, back_color);
        
        button_size = ImVec2{state.dpi(30.0f), 0}; 
        dr_cr_back_color = (tdd.dr_cr == "DR" ? back_color_red : back_color_green);
        if (ColoredButton(state, tdd.dr_cr.c_str(), 485.0f, button_size, clrBackDarkBlack(state), dr_cr_back_color)) {
            tdd.dr_cr = (tdd.dr_cr == "DR" ? "CR" : "DR");
        }
        if (show_buying_power) {
            TextInput(state, "##buying_power", &tdd.buying_power, ImGuiInputTextFlags_CallbackCharFilter, 
                580.0f, 80.0f, text_color_white, back_color, InputTextCallbackNumbersOnly);
        }

        ImGui::NewLine();

        ImGui::BeginGroup();
        ImGui::Spacing();
        TextLabel(state, "Notes", 0.0f, text_color_gray, back_color);
        ImGui::NewLine();

        // Push custom background and foreground colors
        ImGui::PushStyleColor(ImGuiCol_FrameBg, back_color);    
        ImGui::PushStyleColor(ImGuiCol_Text, text_color_white);
        button_size = ImVec2{state.dpi(470.0f), state.dpi(100.0f)}; 
        ImGui::SameLine(state.dpi(6));
        ImGui::InputTextMultiline("##TradeDialogMultiLine", &tdd.notes, button_size, ImGuiInputTextFlags_None);
        ImGui::PopStyleColor(2);

        button_size = ImVec2{state.dpi(80.0f), 0}; 
        if (ColoredButton(state, "SAVE", 540.0f, button_size, clrTextBrightWhite(state), clrGreen(state))) {
            if (IsEditTradeAction(state.trade_action)) {
                if (ValidateEditTradeData(state, tdd, tgd)) {
                    // save the trade/transaction
                    CreateEditTradeData(state, tdd, tgd);
                    SaveTradeData(state, tdd);
                    close_dialog = true;
                }
            } else if (IsOtherIncomeExpense(state.trade_action) ||
                IsAddOtherIncomeExpenseToTrade(state.trade_action)) {
                if (ValidateOtherIncomeExpenseData(state, tdd, tgd)) {
                    // save the trade/transaction
                    CreateOtherIncomeExpenseData(state, tdd, tgd);
                    SaveTradeData(state, tdd);
                    close_dialog = true;
                }
            } else if (IsAddDividendToTrade(state.trade_action)) {
                if (ValidateDividendTradeData(state, tdd, tgd)) {
                    // save the trade/transaction
                    CreateDividendTradeData(state, tdd, tgd);
                    SaveTradeData(state, tdd);
                    close_dialog = true;
                }
            } else {
                Underlying underlying = SetTradeUnderlying(state);
                if (underlying == Underlying::Options) {
                    if (ValidateOptionsTradeData(state, tdd, tgd)) {
                        // save the trade/transaction
                        CreateOptionsTradeData(state, tdd, tgd);
                        SaveTradeData(state, tdd);
                        close_dialog = true;
                    }
                }
                if (underlying == Underlying::Shares ||
                    underlying == Underlying::Futures) {
                    if (ValidateSharesFuturesTradeData(state, tdd, tgd)) {
                        // save the trade/transaction
                        CreateSharesFuturesTradeData(state, tdd, tgd);
                        SaveTradeData(state, tdd);
                        close_dialog = true;
                    }
                }

            }
        }

        if (ColoredButton(state, "Cancel", 630.0f, button_size, clrTextBrightWhite(state), clrRed(state))) {
            close_dialog = true;
        }

        ImGui::NewLine();
        if (show_dte_warnings) {
            CheckBox(state, "3 DTE indicator",  &tdd.warning_3_dte,    0.0f, text_color_white, back_color);
            CheckBox(state, "21 DTE indicator", &tdd.warning_21_dte, 140.0f, text_color_white, back_color);
        }

        if (close_dialog) {
	        state.show_tradedialog_popup = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndGroup();

        if (is_first_open) is_first_open = false;

        // If a secondary popup modal must be shown (eg the MessageBox), then we need to display it
        // here prior to the call to the TradeDialog's EndPopup()
        ShowMessageBoxPopup(state);
        ShowCategoriesDialogPopup(state);

        ImGui::EndPopup();
    }
    ImGui::PopStyleVar(); 
    ImGui::PopStyleColor(); 
}
 
