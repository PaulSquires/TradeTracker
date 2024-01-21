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
#include "Config/Config.h"

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


#include <queue>

void Trade::CalculateAdjustedCostBase() {

    bool exclude_nonstock_costs = config.GetExcludeNonStockCosts();

    if (config.GetCostingMethod() == CostingMethod::AverageCost) {
        this->acb_total = 0;
        this->shares_acb = 0;
        double total_shares = 0;

        for (auto& trans : this->transactions) {
            bool is_share_transaction = 
                (trans->underlying == Underlying::Shares || trans->underlying == Underlying::Futures) ? true : false;

            if (is_share_transaction) {

                if (trans->total > 0) {
                    // Sold shares
                    trans->share_average_cost = (this->shares_acb / total_shares);
                    double share_sale_cost = (trans->quantity * trans->share_average_cost);

                    total_shares -= trans->quantity;
                    this->acb_total -= share_sale_cost;
                    this->shares_acb -= share_sale_cost;
                    continue;
                }
                else {
                    // Bought shares
                    this->acb_total += trans->total;
                    this->shares_acb += trans->total;
                    total_shares += trans->quantity;
                    continue;
                }
            }

            // bought shares or other items eg. options/dividends/other
            this->acb_total += trans->total;
            if (!exclude_nonstock_costs) {
                this->shares_acb += trans->total;
            }
        }
        return;
    }


    if (config.GetCostingMethod() == CostingMethod::fifo) {
        this->acb_total = 0;
        this->shares_acb = 0;

        struct Shares {
            std::wstring trans_date{};
            int quantity_remaining{};
            double cost_per_share{};
        };
        
        // Load the vector and then sort based on earliest date of shares purchase.
        std::vector<Shares> vec;
        for (const auto& trans : this->transactions) {
            bool is_share_transaction = 
                (trans->underlying == Underlying::Shares || trans->underlying == Underlying::Futures) ? true : false;
            if (is_share_transaction && trans->total < 0) {   // bought shares
                double cost_per_share = (trans->total / trans->quantity);
                Shares share{ trans->trans_date, trans->quantity, cost_per_share };
                vec.push_back(share);
            }
        }
        // Sort based on date
        std::sort(vec.begin(), vec.end(),
            [](const auto& lhs, const auto& rhs) {
                {
                    if (lhs.trans_date < rhs.trans_date) return true;
                    if (rhs.trans_date < lhs.trans_date) return false;
                    return false;
                }
            });

        // Add the sorted vector to the queue (doesn't look like we can sort a queue directly)
        std::queue<Shares> q;
        for (const auto& shares : vec) {
            q.push(shares);
        }

        for (const auto& trans : this->transactions) {
            bool is_share_transaction = (trans->underlying == Underlying::Shares || 
                trans->underlying == Underlying::Futures) ? true : false;
            if (is_share_transaction && trans->total >= 0) {  // sold shares

                int shares_sold = trans->quantity;

                while (shares_sold > 0) {

                    // See if we can take all the need shares being sold from this purchase
                    if (q.front().quantity_remaining - shares_sold >= 0) {
                        this->acb_total -= shares_sold * q.front().cost_per_share;
                        this->shares_acb -= shares_sold * q.front().cost_per_share;
                        trans->share_average_cost = q.front().cost_per_share;

                        shares_sold -= q.front().quantity_remaining;

                        // All shares being sold have been costed by this leg
                        q.pop();
                        continue;
                    }

                    // Take all shares from this leg and continue looping because more remain to be costed.
                    this->acb_total -= q.front().quantity_remaining * q.front().cost_per_share;
                    this->shares_acb -= q.front().quantity_remaining * q.front().cost_per_share;
                    shares_sold -= q.front().quantity_remaining;
                    q.pop();
                }
            }
            else if (is_share_transaction && trans->total < 0) {  // buy shares
                this->shares_acb += trans->total;
                continue;
            }

            this->acb_total += trans->total;
            if (!exclude_nonstock_costs) {
                this->shares_acb += trans->total;
            }
        }

        return;
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


