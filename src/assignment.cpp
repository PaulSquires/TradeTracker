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

#include "appstate.h"
#include "utilities.h"
#include "active_trades_actions.h"
#include "questionbox.h"
#include "trade_dialog.h"    // CalendarComboBox
#include "assignment.h"

#include <cstddef>
// #include <iostream>


// ========================================================================================
// Create Transaction for Shares or Futures that have been called away.
// ========================================================================================
void CalledAwayAssignment(AppState& state,
			std::shared_ptr<Trade> trade, std::shared_ptr<Leg> leg,
			bool is_shares, int quantity_assigned, double multiplier, std::string& called_away_date) {

    std::shared_ptr<Transaction> trans;
    std::shared_ptr<Leg> newleg;

    // Close the Option. Save this transaction's leg quantities
    trans = std::make_shared<Transaction>();
    trans->trans_date = called_away_date;
    trans->description = "Called away";
    trans->underlying = Underlying::Options;
    trade->transactions.push_back(trans);

    newleg = std::make_shared<Leg>();
    trade->nextleg_id += 1;
    newleg->leg_id = trade->nextleg_id;
    newleg->underlying = trans->underlying;
    newleg->open_quantity = 0;

    if (is_shares) {
        newleg->original_quantity = (quantity_assigned / 100);
        leg->open_quantity = (leg->open_quantity + (quantity_assigned / 100));;
    }
    else {
        newleg->original_quantity = quantity_assigned;
        leg->open_quantity = (leg->open_quantity + quantity_assigned);;
    }
    newleg->leg_back_pointer_id = leg->leg_id;

    if (leg->action == Action::STO) newleg->action = Action::BTC;
    if (leg->action == Action::BTO) newleg->action = Action::STC;

    newleg->expiry_date = called_away_date;
    newleg->strike_price = leg->strike_price;
    newleg->put_call = leg->put_call;
    trans->legs.push_back(newleg);


    // Remove the SHARES/FUTURES that have been called away.
    trans = std::make_shared<Transaction>();
    trans->trans_date = called_away_date;
    trans->description = "Called away";
    trans->underlying = (is_shares) ? Underlying::Shares : Underlying::Futures;
    trans->quantity = quantity_assigned;
    trans->price = AfxValDouble(leg->strike_price);
    trans->multiplier = multiplier;
    trans->fees = 0;
    trans->share_action= (leg->put_call == PutCall::Put) ? Action::BTC : Action::STC;
    trade->transactions.push_back(trans);

    newleg = std::make_shared<Leg>();
    trade->nextleg_id += 1;
    newleg->leg_id = trade->nextleg_id;
    newleg->underlying = trans->underlying;
    newleg->strike_price = leg->strike_price;

    if (leg->put_call == PutCall::Put) {
        newleg->action = Action::BTC;
        newleg->original_quantity = quantity_assigned;
        newleg->open_quantity = quantity_assigned;
        trans->total = trans->quantity * multiplier * trans->price * -1;  // DR
    }
    else {
        newleg->action = Action::STC;
        newleg->original_quantity = quantity_assigned * -1;
        newleg->open_quantity = quantity_assigned * -1;
        trans->total = trans->quantity * multiplier * trans->price;  // CR
    }

    trans->legs.push_back(newleg);

    // Set the open status of the entire trade based on the new modified legs
    trade->SetTradeOpenStatus();

    // Save/Load the new data to the database
    ReloadAppState(state);
}


// ========================================================================================
// Create Transaction for option assignment for the selected leg.
// ========================================================================================
void CreateAssignment(AppState& state, std::shared_ptr<Trade> trade, std::shared_ptr<Leg> leg,
		bool is_shares, int quantity_assigned, double multiplier, std::string& assignment_date) {
    std::shared_ptr<Transaction> trans;
    std::shared_ptr<Leg> newleg;

    // Close the Option. Save this transaction's leg quantities
    trans = std::make_shared<Transaction>();
    trans->trans_date = assignment_date;
    trans->quantity = 1;   // must have something > 0 otherwise "Quantity error" if saving an Edit
    trans->description = "Assignment";
    trans->underlying = Underlying::Options;
    trade->transactions.push_back(trans);

    newleg = std::make_shared<Leg>();
    trade->nextleg_id += 1;
    newleg->leg_id = trade->nextleg_id;
    newleg->underlying = trans->underlying;

    newleg->open_quantity = 0;

    if (is_shares) {
        newleg->original_quantity = (quantity_assigned / 100);
        leg->open_quantity = (leg->open_quantity + (quantity_assigned / 100));;
    }
    else {
        newleg->original_quantity = quantity_assigned;
        leg->open_quantity = (leg->open_quantity + quantity_assigned);;
    }
    newleg->leg_back_pointer_id = leg->leg_id;

    if (leg->action == Action::STO) newleg->action = Action::BTC;
    if (leg->action == Action::BTO) newleg->action = Action::STC;

    newleg->expiry_date = assignment_date;
    newleg->strike_price = leg->strike_price;
    newleg->put_call = leg->put_call;
    trans->legs.push_back(newleg);

    // Make the SHARES/FUTURES that have been assigned.
    trans = std::make_shared<Transaction>();
    trans->trans_date = assignment_date;
    trans->description = "Assignment";
    trans->underlying = (is_shares) ? Underlying::Shares : Underlying::Futures;
    trans->quantity = quantity_assigned;
    trans->price = AfxValDouble(leg->strike_price);
    trans->multiplier = multiplier;
    trans->fees = 0;
    trade->transactions.push_back(trans);

    newleg = std::make_shared<Leg>();
    trade->nextleg_id += 1;
    newleg->leg_id = trade->nextleg_id;
    newleg->underlying = trans->underlying;
    newleg->strike_price = leg->strike_price;

    if (leg->put_call == PutCall::Put) {
        newleg->action = Action::BTO;
        newleg->original_quantity = quantity_assigned;
        newleg->open_quantity = quantity_assigned;
        trans->total = trans->quantity * multiplier * trans->price * -1;  // DR
    }
    else {
        newleg->action = Action::STO;
        newleg->original_quantity = quantity_assigned * -1;
        newleg->open_quantity = quantity_assigned * -1;
        trans->total = trans->quantity * multiplier * trans->price;  // CR
    }

    trans->legs.push_back(newleg);

    // Set the open status of the entire trade based on the new modified legs
    trade->SetTradeOpenStatus();

    // Save/Load the new data to the database
    ReloadAppState(state);
}


// Callback function to filter input
int CallbackNumbersOnly(ImGuiInputTextCallbackData* data) {
    if (data->EventFlag == ImGuiInputTextFlags_CallbackCharFilter) {
        if (data->EventChar < 256 && strchr("0123456789-", data->EventChar) == NULL) {
            return 1;   // Ignore the character
        }
    }
    return 0;   // Accept the character
}


void ShowAssignmentPopup(AppState& state) {
    if (!state.show_assignment_popup) return;

	std::shared_ptr<Trade> trade = state.activetrades_selected_trade;
	if (!trade) return;

    // Check to see if Shares/Futures exist that could be "called away". If yes, then process
    // that type of transaction rather than adding a new assigned to transaction.
    int aggregate_shares = state.activetrades_rightclickmenu_shares_count;
    int aggregate_futures = state.activetrades_rightclickmenu_futures_count;

    std::shared_ptr<Leg> leg = state.activetrades_rightclickmenu_assignment_leg;
    if (!leg) return;

    static bool is_first_open = false;

    if (!ImGui::IsPopupOpen(state.id_assignment_popup.c_str())) {
        ImGui::OpenPopup(state.id_assignment_popup.c_str());
        is_first_open = true;
    }

    ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, clrPopupBg(state));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{state.dpi(26.0f), state.dpi(26.0f)});

    if (ImGui::BeginPopupModal(state.id_assignment_popup.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        static bool is_called_away = false;
        static bool is_assignment = false;
        static std::string long_short_text;
        static std::string quantity_assigned_text;
        static std::string strike_price_text;
        static std::string assignment_date = AfxCurrentDate();
        static std::string caption;

        static int quantity_assigned = 0;
        int leg_quantity = 0;
        std::string message;
        double multiplier = 1;
        float left = 0.0f;

        ImU32 back_color = clrBackMediumGray(state);
        ImU32 text_color_red = clrRed(state);
        ImU32 text_color_green = clrGreen(state);
        ImU32 text_color_white = clrTextLightWhite(state);
        ImU32 text_color_gray = clrTextDarkWhite(state);

        bool is_shares = (trade->ticker_symbol.substr(0,1) == "/") ? false : true;
        static bool show_long_short = false;

        if (is_first_open) {
            is_called_away = false;
            is_assignment = false;
            long_short_text = "";
            quantity_assigned_text = "";
            strike_price_text = "";

            // Are LONG SHARES or LONG FUTURES being called away
            // Are SHORT SHARES or SHORT FUTURES being called away
            if (((aggregate_shares > 0 || aggregate_futures > 0) && leg->put_call == PutCall::Call) ||
                ((aggregate_shares < 0 || aggregate_futures < 0) && leg->put_call == PutCall::Put)) {

                is_called_away = true;
                if (is_shares) {
                    quantity_assigned = MIN(std::abs(leg->open_quantity * 100), std::abs(aggregate_shares));
                    leg_quantity = quantity_assigned / 100;
                    quantity_assigned_text = std::to_string(quantity_assigned);
                    strike_price_text = " shares called away at $" + leg->strike_price + " per share.";
                    multiplier = 1;
                    caption = "SHARES CALLED AWAY";
                }
                else {
                    quantity_assigned = MIN(std::abs(leg->open_quantity), std::abs(aggregate_futures));
                    leg_quantity = quantity_assigned;
                    quantity_assigned_text = std::to_string(quantity_assigned);
                    strike_price_text = " futures called away at $" + leg->strike_price + " per future.";
                    multiplier = trade->multiplier;
                    caption = "FUTURES CALLED AWAY";
                }
                leg_quantity = (leg->open_quantity < 0) ? leg_quantity * -1 : leg_quantity;
            } else {
                is_assignment = true;
                long_short_text = (leg->put_call == PutCall::Put) ? "LONG" : "SHORT";
                show_long_short = true;

                if (is_shares) {
                    quantity_assigned = std::abs(leg->open_quantity * 100);
                    quantity_assigned_text = std::to_string(quantity_assigned);
                    strike_price_text = " shares at $" + leg->strike_price + " per share.";
                    multiplier = 1;
                    caption = "SHARES ASSIGNED";
                }
                else {
                    quantity_assigned = std::abs(leg->open_quantity);
                    quantity_assigned_text = std::to_string(quantity_assigned);
                    strike_price_text = " futures at $" + leg->strike_price + " per future.";
                    multiplier = trade->multiplier;
                    caption = "FUTURES ASSIGNED";
                }
            }
        }

        TextLabel(state, caption.c_str(), left, text_color_white, back_color);

        ImGui::BeginGroup();

        ImGui::NewLine();
        ImGui::Spacing();

        TextLabel(state, "Date", left, text_color_gray, back_color);
        ImGui::NewLine();
        CalendarComboBox(state, "AssignmentDate", assignment_date, assignment_date, ImGuiComboFlags_HeightLarge, left, 120.0f);

        ImGui::NewLine();
        ImGui::AlignTextToFramePadding();
        if (show_long_short && is_assignment) {
            TextLabel(state, long_short_text.c_str(), left, (long_short_text == "LONG") ? text_color_green : text_color_red, back_color);
        	left += 50.0f;
        }

        if (is_first_open) ImGui::SetKeyboardFocusHere();
        TextInput(state, "##assignment_id", &quantity_assigned_text, ImGuiInputTextFlags_None,
        	left, 50.0f, text_color_white, back_color, CallbackNumbersOnly);
        left += 62.0f;

       	TextLabel(state, strike_price_text.c_str(), left, text_color_white, back_color);

        ImGui::NewLine();

        ImVec2 button_size(ImGui::GetFontSize() * 7.0f, 0.0f);

        if (ImGui::Button("Yes", button_size)) {
	        // Do not exceed the available max quantity
            int quantity_assigned_input = AfxValInteger(quantity_assigned_text);

            // Error if quantity assigned is invalid
            bool is_error = (quantity_assigned_input <= 0 || quantity_assigned_input > quantity_assigned);
            if (is_error) {
                std::string error_message = "Amount entered is invalid!";
                CustomQuestionBox(state, "Error", error_message, QuestionCallback::None, true);
            } else {
		        if (is_called_away) {
		        	CalledAwayAssignment(state, trade, leg, is_shares, quantity_assigned_input, multiplier, assignment_date);
		        }
		        if (is_assignment) {
				    CreateAssignment(state, trade, leg, is_shares, quantity_assigned_input, multiplier, assignment_date);
		        }
                state.show_assignment_popup = false;
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("No", button_size)) {
            state.show_assignment_popup = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", button_size)) {
            state.show_assignment_popup = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndGroup();

        if (is_first_open) is_first_open = false;

        // If a secondary popup modal must be shown (eg the MessageBox), then we need to display it
        // here prior to the call to the TradeDialog's EndPopup()
        ShowQuestionPopup(state);

        ImGui::EndPopup();
    }

    ImGui::PopStyleVar();
	ImGui::PopStyleColor();
}

