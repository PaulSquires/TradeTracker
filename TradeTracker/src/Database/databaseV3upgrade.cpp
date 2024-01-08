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

#include "pch.h"

#include "trade.h"
#include "database.h"


// ========================================================================================
// Calculate the running cost for each leg in the Trade. Need to do this upgrade because
// the existing database does not have leg backpointers for the new leg of a rolled trans.
// ========================================================================================
void UpgradeOptionTransaction(const std::shared_ptr<Trade> trade, const std::shared_ptr<Transaction> trans) {

    std::unordered_map<int, std::shared_ptr<Leg>> map;

    int trans_contract_count = 0;

    // Get the total number of quantity of contracts for the transaction. Only
    // count the legs that generate money on an open.
    for (const auto& lleg : trans->legs) {
        if ((lleg->action == Action::STO || lleg->action == Action::BTO) ||
            (lleg->action == Action::BTC || lleg->action == Action::STC && lleg->open_quantity != 0)) {
            trans_contract_count += abs(lleg->original_quantity);
        }
    }
    if (trans_contract_count == 0) return;


    for (auto& leg : trans->legs) {

        // Only process BTC or STC legs if they were not competely closed. Calculate
        // the income/cost from the leg and update the back_pointer with the value.
        if (leg->action == Action::BTC || leg->action == Action::STC && leg->open_quantity != 0) {
            double closing_leg_cost =
                (trans->total / trans_contract_count) * abs(leg->original_quantity);

            auto it = map.find(leg->leg_back_pointer_id);
            if (it != map.end()) {
                it->second->calculated_leg_cost += closing_leg_cost;
            }
            continue;
        }

        // Process a normal STO/BTO leg
        // Calculate the cost of the leg based on this transaction
        double current_leg_cost =
            (trans->total / trans_contract_count) * abs(leg->original_quantity);

        // Get the running cost from the leg referenced in the back pointer
        double leg_cost_from_back_pointer = 0;
        if (leg->leg_back_pointer_id) {
            auto it = map.find(leg->leg_back_pointer_id);
            if (it != map.end()) {
                leg_cost_from_back_pointer = it->second->calculated_leg_cost;
            }
        }

        leg->calculated_leg_cost = current_leg_cost + leg_cost_from_back_pointer;

        // Save cost in case a later back pointer needs it
        map.insert({ leg->leg_id, leg });
    }


    // Display Results
    //std::cout << "TRADE: " << std::endl;
    //for (auto& leg : open_legs) {
    //    std::cout << "  Leg cost: " << leg->calculated_leg_cost << std::endl;
    //}

}


// ========================================================================================
// Calculate the running cost for each leg in the Trade. Need to do this upgrade because
// the existing database does not have leg backpointers for the new leg of a rolled trans.
// ========================================================================================
void DatabaseV3upgrade() {
    for (const auto& trade: trades) {
        for (const auto& trans : trade->transactions) {
            if (trans->underlying != Underlying::Options) continue;
            UpgradeOptionTransaction(trade, trans);
        }
    }
}

