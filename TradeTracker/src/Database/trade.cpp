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

#include "Utilities/AfxWin.h"

#include "trade.h"


bool Leg::isOpen() {
    return  (open_quantity == 0 ? false : true);
}


void Trade::SetTradeOpenStatus() {
    // default that the Trade is closed
    is_open = false;

    // Roll up all of the SHARES or FUTURES TransDetail and display the aggregate rather than the individual legs.
    int aggregate = 0;
    bool do_quantity_check = false;

    for (const auto& trans : transactions) {
        for (const auto& leg : trans->legs) {
            switch (leg->underlying) {
            case Underlying::Options:
                if (leg->isOpen()) {
                    // A leg is open so therefore the Trade must stay open
                    this->is_open = true;
                    return;
                }
                break;
            case Underlying::Shares:
            case Underlying::Futures:
               aggregate += leg->open_quantity;
               do_quantity_check = true;
            }
        }
    }

    // If this was a SHARES or FUTURES rollup then check to see if the aggregate amount is ZERO. 
    if (do_quantity_check) {
        this->is_open = (aggregate == 0) ? false : true;
        return;
    }

    // If the Trade is closed then set the latest Buying Power date to the close date
    this->bp_end_date = this->oldest_trade_trans_date;
}


void Trade::CalculateAdjustedCostBase() {
    this->acb = 0;
    for (const auto& trans : this->transactions) {
        this->acb += trans->total;
    }
}


void Trade::CreateOpenLegsVector() {
    // Create the openLegs vector. We need this vector because we have to sort the
    // collection of open legs in order to have Puts before Calls. There could be
    // multiple Transactions in this trade and therefore Puts and Calls would not
    // already be a suitable display order.

    open_legs.clear();
    this->aggregate_shares = 0;
    this->aggregate_futures = 0;

    std::wstring current_date = AfxCurrentDate();

    for (const auto& trans : transactions) {
        if (trans->multiplier > 0) this->multiplier = trans->multiplier;

        int dte = 0;
        for (auto& leg : trans->legs) {

            if (leg->isOpen()) {
                leg->trans = trans;
                open_legs.push_back(leg);

                switch (leg->underlying) {
                case Underlying::Options:
                    // Do check to calculate the DTE and compare to the earliest already calculated DTE
                    // for the Trade. We store the earliest DTE value because ActiveTrades uses it when
                    // sorted by Expiration.
                    dte = AfxDaysBetween(current_date, leg->expiry_date);
                    if (dte < this->earliest_legs_DTE) this->earliest_legs_DTE = dte;
                    break;
                case Underlying::Shares:
                    this->aggregate_shares += leg->open_quantity;
                    break;
                case Underlying::Futures:
                    this->aggregate_futures += leg->open_quantity;
                    break;
                }
            }
        }
    }

    // Finally, sort the vector based on Puts first with lowest Strike Price.
    std::sort(open_legs.begin(), open_legs.end(),
        [](const auto& leg1, const auto& leg2) {
            if (leg1->put_call == PutCall::Put && leg2->put_call == PutCall::Call) {return true;}
            if (leg1->put_call == PutCall::Put && leg2->put_call == PutCall::Put) {
                if (std::stod(leg1->strike_price) < std::stod(leg2->strike_price)) return true;
            }
            if (leg1->put_call == PutCall::Call && leg2->put_call == PutCall::Call) {
                if (std::stod(leg1->strike_price) < std::stod(leg2->strike_price)) return true;
            }
            return false;
        });
}


// ========================================================================================
// Calculate the running cost for each leg in the Trade. Open legs will display in the 
// Active Trades grid. When connected to TWS, the Leg cost will be displayed and compared
// against the current market price.
// ========================================================================================
void Trade::CalculateLegCosting() {

    std::unordered_map<int, std::shared_ptr<Leg>> map;

    for (const auto& trans : transactions) {
        if (trans->underlying != Underlying::Options) continue;

        int trans_contract_count = 0;

        // Get the total number of quantity of contracts for the transaction. Only
        // count the legs that generate money on an open.
        for (const auto& lleg : trans->legs) {
            if (lleg->action == Action::STO || lleg->action == Action::BTO) {
                trans_contract_count += abs(lleg->original_quantity);
            }
        }
        if (trans_contract_count == 0) continue;


        for (auto& leg : trans->legs) {

            // Only process BTC or STC legs if they were not completely closed. Calculate
            // the income/cost from the leg and update the back_pointer with the value.
            if (leg->action == Action::BTC || leg->action == Action::STC) {
                if (leg->open_quantity) {
                    double closing_leg_cost =
                        (trans->total / trans_contract_count) * abs(leg->original_quantity);

                    auto it = map.find(leg->leg_back_pointer_id);
                    if (it != map.end()) {
                        it->second->calculated_leg_cost += closing_leg_cost;
                    }
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

    }

    // Display Results
    //std::cout << "TRADE: " << std::endl;
    //for (auto& leg : open_legs) {
    //    std::cout << "  Leg cost: " << leg->calculated_leg_cost << std::endl;
    //}

}

