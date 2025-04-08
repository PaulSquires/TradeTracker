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

#include "appstate.h"
#include "utilities.h"
#include "reconcile.h"

// #include <iostream>


#ifdef __WXMSW__
	#include "tws-api/windows/Decimal.h"
#else
	#include "tws-api/linux/Decimal.h"
#endif

double intelDecimalToDouble(Decimal decimal) {
    unsigned int flags;
    //return intel_bid64_to_binary64(decimal, 0, &flags);
    return __bid64_to_binary64(decimal, 0, &flags);
}


std::vector<positionStruct> ibkr_positions;    // persistent 
std::vector<positionStruct> local_positions;   // persistent 

// ========================================================================================
// Load one open trades into the local_positions vector
// ========================================================================================
void Reconcile_LoadOneLocalPosition(AppState& state, const auto& trade) {
	for (const auto& leg : trade->open_legs) {
		positionStruct p{};
		p.open_quantity = leg->open_quantity;
		p.ticker_symbol = trade->ticker_symbol;
		p.trade = trade;
		p.leg = leg;

		if (leg->underlying == Underlying::Futures) p.underlying = "FUT";
		if (leg->underlying == Underlying::Options) p.underlying = "OPT";
		if (leg->underlying == Underlying::Shares) p.underlying = "STK";

		p.strike_price = AfxValDouble(leg->strike_price);
		p.expiry_date = AfxRemoveDateHyphens(leg->expiry_date);
		p.put_call = state.db.PutCallToString(leg->put_call);

		// Check if the ticker is a future
		if (state.config.IsFuturesTicker(p.ticker_symbol)) {
			p.ticker_symbol = trade->ticker_symbol.substr(1);
			if (p.underlying == "OPT") p.underlying = "FOP";
		}

		// Check to see if the LOCAL position already exists in the vector. If it
		// does then simply update the Local quantity. We need to do this because IBKR
		// aggregates all similar positions.
		bool found = false;
		for (auto& local : local_positions) {
			if (p.underlying == "OPT" ||
				p.underlying == "FOP") {
				if (p.strike_price == local.strike_price &&
					p.ticker_symbol == local.ticker_symbol &&
					p.expiry_date == local.expiry_date &&
					p.put_call == local.put_call) {
					found = true;
					local.open_quantity += p.open_quantity;
					local.legs.push_back(leg);
					break;
				}
			}
			if (p.underlying == "STK" ||
				p.underlying == "FUT") {
				if (p.ticker_symbol == local.ticker_symbol &&
					p.underlying == local.underlying) {
					found = true;
					local.open_quantity += p.open_quantity;
					local.legs.push_back(leg);
					break;
				}
			}
		}

		if (found == false) {
			p.legs.push_back(leg);
			local_positions.push_back(p);
		}
	}
}


// ========================================================================================
// Load all open trades into the local_positions vector
// ========================================================================================
void Reconcile_LoadAllLocalPositions(AppState& state) {
	// Add all of the current LOCAL "Open" positions to the local_positions vector
	local_positions.clear();
	for (const auto& trade : state.db.trades) {
		if (!trade->is_open) continue;
		Reconcile_LoadOneLocalPosition(state, trade);
	}
}


// ========================================================================================
// Information received from TwsClient::position callback
// ========================================================================================
void Reconcile_position(const Contract& contract, Decimal position, double avg_cost) {
	// This callback is initiated by the reqPositions() call.
	// This will receive every open position as reported by IBKR. We take these positions and
	// store them in the IBKRPositions vector for later processing.
	// This function is also called whenever the position data on IBKR server changes. We update
	// positions here as they are received.

	// Remove any existing version of the contract before adding it again
	auto end = std::remove_if(ibkr_positions.begin(),
		ibkr_positions.end(),
		[contract](positionStruct const& p) {
			return (p.contract_id == contract.conId) ? true : false;
		});
	ibkr_positions.erase(end, ibkr_positions.end());

	// Add the position the vector
	positionStruct p{};
	p.contract_id   = contract.conId;
	p.contract      = contract;
	p.open_quantity = (int)intelDecimalToDouble(position);
	p.ticker_symbol = contract.symbol;
	p.underlying    = contract.secType;
	p.expiry_date   = contract.lastTradeDateOrContractMonth;   // YYYYMMDD
	p.strike_price  = contract.strike;
	p.put_call      = contract.right;
	// If this is a Lean Hog Futures contract then we multiply the strike by 100 b/c IBKR
	// stores it as cents but we placed the trade as "dollars".  eg. .85 vs. 85
	if (p.ticker_symbol == "HE" && p.underlying == "FOP") {
		p.strike_price *= 100;
	}
	if (p.ticker_symbol == "LE" && p.underlying == "FOP") {
		p.strike_price *= 100;
	}
	ibkr_positions.push_back(p);
}


// ========================================================================================
// Test if IBKR and LOCAL position are equal
// ========================================================================================
bool Reconcile_ArePositionsEqual(positionStruct ibkr, positionStruct local) {
	if (ibkr.underlying == "OPT" ||
		ibkr.underlying == "FOP") {
		if (ibkr.strike_price == local.strike_price &&
			ibkr.open_quantity == local.open_quantity &&
			ibkr.ticker_symbol == local.ticker_symbol &&
			ibkr.expiry_date == local.expiry_date &&
			ibkr.put_call == local.put_call &&
			ibkr.underlying == local.underlying) {
			return true;
		}
	}
	if (ibkr.underlying == "STK" ||
		ibkr.underlying == "FUT") {
		if (ibkr.open_quantity == local.open_quantity &&
			ibkr.ticker_symbol == local.ticker_symbol &&
			ibkr.underlying == local.underlying) {
			return true;
		}
	}

	return false;
}


// ========================================================================================
// Information received from TwsClient::positionEnd callback and when Reconciliation
// is requested to be run.
// ========================================================================================
void Reconcile_doPositionMatching() {
	for (const auto& ibkr : ibkr_positions) {
		if (ibkr.open_quantity == 0) continue;
		bool found = false;
		for (auto& local : local_positions) {
			// Check if the contract has previously already been assigned.
			if (local.contract_id == ibkr.contract_id) break;
			found = Reconcile_ArePositionsEqual(ibkr, local);
			// If found, then update the Local vector to point to the contract_id of the 
			// actual IBKR position. We use this when dealing with UpdatePortfolio() callbacks.
			if (found) {
				local.contract_id = ibkr.contract_id;
				for (auto& leg : local.legs) {
					leg->contract_id = ibkr.contract_id;
				}
				break;
			}
		}
	}
}


// ========================================================================================
// Perform a reconciliation between IBKR positions and local positions.
// ========================================================================================
void Reconcile_doReconciliation(AppState& state) {
	std::string text;
	std::string sp = "  ";

	// (1) Determine what IBKR "real" positions do not exist in the Local database.
	state.reconciliation_results_text = sp + "IBKR that do not exist in Local:\r\n";

	for (const auto& ibkr : ibkr_positions) {
		if (ibkr.open_quantity == 0) continue;
		bool found = false;
		for (auto& local : local_positions) {
			found = Reconcile_ArePositionsEqual(ibkr, local);
			if (found) break;
		}
		if (!found) {
			text += sp + 
				AfxRSet(std::to_string(ibkr.open_quantity), 8) +
				"  " +
				AfxLSet(ibkr.ticker_symbol, 8) + 
				AfxLSet(ibkr.underlying, 5);
			if (ibkr.underlying == "OPT" || ibkr.underlying == "FOP") {
				text += sp + 
					AfxLSet(AfxInsertDateHyphens(ibkr.expiry_date), 12) + 
					AfxRSet(std::to_string(ibkr.strike_price), 16) +
					AfxRSet(ibkr.put_call, 3);
			}
			text += "\n";
		}
	}
	if (text.length() == 0) text = sp + "** Everything matches correctly **";
	state.reconciliation_results_text += text;

	state.reconciliation_results_text += "\n\n";   // blank lines

	// (2) Determine what Local positions do not exist in the IBKR "real" database.
	state.reconciliation_results_text += sp + "Local that do not exist in IBKR:\n";
	text = "";
	for (const auto& local : local_positions) {
		bool found = false;
		for (const auto& ibkr : ibkr_positions) {
			found = Reconcile_ArePositionsEqual(ibkr, local);
			if (found == true) break;
		}
		if (!found) {
			if (local.open_quantity != 0) {   // test b/c local may aggregate to zero and may already disappeard from IB
				text += sp +
					AfxRSet(std::to_string(local.open_quantity), 8) +
					"  " +
					AfxLSet(local.ticker_symbol, 8) +
					AfxLSet(local.underlying, 5);
				if (local.underlying == "OPT" || local.underlying == "FOP") {
					text += sp +
						AfxLSet(AfxInsertDateHyphens(local.expiry_date), 12) +
						AfxRSet(std::to_string(local.strike_price), 16) +
						AfxRSet(local.put_call, 3);
				}
				text += "\n";
			}
		}
	}
	if (text.length() == 0) text = sp + "** Everything matches correctly **";
	state.reconciliation_results_text += text;
	state.reconciliation_results_text = "\n" + state.reconciliation_results_text;
}


// ========================================================================================
// Create and show the Reconcile modeless dialog.
// ========================================================================================
void ShowReconciliationResultsPopup(AppState& state) {
	if (!state.show_reconciliation_popup) return;

	static bool is_popup_open = false;

    // Trigger the popup
    if (!is_popup_open) {
        ImVec2 size{state.dpi(600.0f),state.dpi(420.0f)};
        ImGui::SetNextWindowSize(size);
    	ImGui::OpenPopup(state.id_reconciliation_popup.c_str(), ImGuiPopupFlags_NoOpenOverExistingPopup);
    	is_popup_open = true;
    }

    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	// Check if popup is open
	if (ImGui::BeginPopup(state.id_reconciliation_popup.c_str(), ImGuiWindowFlags_None)) {
        ImGui::Text("Reconcile Local Data to IBKR TWS");

        // Push custom background and foreground colors
	    ImGui::PushFont(state.gui_font_mono);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, clrBackDarkGray(state));    
        ImGui::PushStyleColor(ImGuiCol_Text, clrTextLightWhite(state));
        ImVec2 size{state.dpi(600.0f), state.dpi(390.0f)};
        ImGui::InputTextMultiline("##ReconcileMultiLine", 
        	&state.reconciliation_results_text, size, ImGuiInputTextFlags_ReadOnly);
        ImGui::PopStyleColor(2);
        ImGui::PopFont();    // gui_font_mono

        ImGui::NewLine();
        ImVec2 button_size{state.dpi(80.0f), 0}; 
        if (ColoredButton(state, "Close", 520.0f, button_size, clrTextBrightWhite(state), clrRed(state))) { 
            ImGui::CloseCurrentPopup();
    		is_popup_open = false;
	        state.show_reconciliation_popup = false;
        }

        ImGui::EndPopup();

    } else if (state.show_reconciliation_popup) {
        // Popup was open, but now it's closed
    	is_popup_open = false;
        state.show_reconciliation_popup = false;
    }
}

