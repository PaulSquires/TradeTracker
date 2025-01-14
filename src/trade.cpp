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

#include <algorithm>
#include "appstate.h"
#include "utilities.h"


bool Leg::isOpen() {
    return  (open_quantity == 0 ? false : true);
}


void Trade::CalculateTotalSharesProfit(AppState& state) {
    // Calculate the overall profit/loss for shares/futures in this trade
    total_share_profit = 0;
    for (const auto& share : shares_history) {
        if (share.leg_action == Action::STC || share.leg_action == Action::BTC) {
            int quantity = abs(share.open_quantity);
            double price = share.trans->price;

            if (state.config.IsFuturesTicker(ticker_symbol)) {
                double multiplier = AfxValDouble(state.config.GetMultiplier(ticker_symbol));
                price *= multiplier;
            }

            double diff = 0;
            if (share.leg_action == Action::STC) diff = (price + share.average_cost);
            if (share.leg_action == Action::BTC) diff = (share.average_cost - price);

            total_share_profit += (quantity * diff);
        }
    }
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
                   break;
                case Underlying::Dividend:
                case Underlying::Other:
                case Underlying::Nothing:
                    break;
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

void Trade::AddSharesHistory(std::shared_ptr<Transaction> trans, Action leg_action, int open_quantity, double average_cost) {
    SharesHistory shares{};
    shares.trans = trans;
    shares.leg_action = leg_action;
    shares.open_quantity = open_quantity;
    shares.average_cost = average_cost;
    shares_history.push_back(shares);
};


void Trade::CalculateAdjustedCostBase(AppState& state) {
    // Reset the shares_allocation vector that is used for displaying data in TradeHistory
    shares_history.clear();

    bool exclude_nonstock_costs = state.config.exclude_nonstock_costs;

    if (state.config.costing_method == CostingMethodType::AverageCost) {
        this->acb_total = 0;
        this->acb_shares = 0;
        this->acb_non_shares = 0;
        double total_shares = 0;

        for (auto& trans : this->transactions) {
            bool is_share_transaction =
                (trans->underlying == Underlying::Shares || trans->underlying == Underlying::Futures) ? true : false;

            if (is_share_transaction) {

                Action leg_action = trans->legs.at(0)->action;

                if (leg_action == Action::BTO || leg_action == Action::STO) {
                    // Buy Long shares
                    // Sell Short shares
                    this->acb_total += trans->total;
                    this->acb_shares += trans->total;
                    total_shares += trans->quantity;
                    AddSharesHistory(trans, leg_action, trans->legs.at(0)->open_quantity, trans->share_average_cost);
                    continue;
                }

                if (leg_action == Action::STC || leg_action == Action::BTC) {
                    // Sell Long shares
                    // Buy Short shares
                    trans->share_average_cost = (total_shares == 0) ? 0 : (this->acb_shares / total_shares);
                    double share_sale_cost = (trans->quantity * trans->share_average_cost);

                    total_shares -= trans->quantity;
                    this->acb_total -= share_sale_cost;
                    this->acb_shares -= share_sale_cost;
                    AddSharesHistory(trans, leg_action, trans->legs.at(0)->open_quantity, trans->share_average_cost);
                    continue;
                }
            }

            // bought shares or other items eg. options/dividends/other
            this->acb_total += trans->total;
            if (!exclude_nonstock_costs) {
                this->acb_shares += trans->total;
            }

            if (!is_share_transaction) {
                this->acb_non_shares += trans->total;
            }
        }
        return;
    }


    if (state.config.costing_method == CostingMethodType::fifo) {
        this->acb_total = 0;
        this->acb_shares = 0;
        this->acb_non_shares = 0;

        struct Shares {
            std::string trans_date{};
            int quantity_remaining{};
            double cost_per_share{};
        };

        // Load the vector and then sort based on earliest date of shares purchase.
        std::vector<Shares> vec;
        for (const auto& trans : this->transactions) {
            bool is_share_transaction =
                (trans->underlying == Underlying::Shares || trans->underlying == Underlying::Futures) ? true : false;

            if (is_share_transaction &&      // bought shares
                (trans->legs.at(0)->action == Action::BTO ||
                 trans->legs.at(0)->action == Action::STO)) {
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

            if (is_share_transaction) {       // sold shares

                Action leg_action = trans->legs.at(0)->action;

                if (leg_action == Action::STC || leg_action == Action::BTC) {
                    // Sell Long shares
                    // Buy Short shares

                    int shares_sold = trans->quantity;

                    while (shares_sold > 0) {

                        // See if we can take all the need shares being sold from this purchase
                        if (q.front().quantity_remaining - shares_sold >= 0) {
                            this->acb_total -= shares_sold * q.front().cost_per_share;
                            this->acb_shares -= shares_sold * q.front().cost_per_share;
                            trans->share_average_cost = q.front().cost_per_share;   // needed for display in closed trades
                            AddSharesHistory(trans, leg_action, shares_sold, q.front().cost_per_share);

                            q.front().quantity_remaining -= shares_sold;
                            shares_sold = 0;

                            // If all shares being sold have been costed by this leg then remove the leg
                            if (q.front().quantity_remaining <= 0) q.pop();
                            continue;
                        }

                        // Take all shares from this leg and continue looping because more remain to be costed.
                        this->acb_total -= q.front().quantity_remaining * q.front().cost_per_share;
                        this->acb_shares -= q.front().quantity_remaining * q.front().cost_per_share;
                        AddSharesHistory(trans, leg_action, q.front().quantity_remaining, q.front().cost_per_share);

                        shares_sold -= q.front().quantity_remaining;
                        q.pop();
                    }
                }

                if (leg_action == Action::BTO || leg_action == Action::STO) {
                    // Buy Long shares
                    // Sell Short shares
                    AddSharesHistory(trans, leg_action, trans->quantity, trans->share_average_cost);
                    this->acb_shares += trans->total;
                    continue;
                }
            }

            this->acb_total += trans->total;
            if (!exclude_nonstock_costs) {
                this->acb_shares += trans->total;
            }

            if (!is_share_transaction) {
                this->acb_non_shares += trans->total;
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

    this->open_legs.clear();
    this->aggregate_shares = 0;
    this->aggregate_futures = 0;

    std::string current_date = AfxCurrentDate();

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
                case Underlying::Dividend:
                case Underlying::Other:
                case Underlying::Nothing:
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
                if (AfxValDouble(leg1->strike_price) < AfxValDouble(leg2->strike_price)) return true;
            }
            if (leg1->put_call == PutCall::Call && leg2->put_call == PutCall::Call) {
                if (AfxValDouble(leg1->strike_price) < AfxValDouble(leg2->strike_price)) return true;
            }
            return false;
        });
}
