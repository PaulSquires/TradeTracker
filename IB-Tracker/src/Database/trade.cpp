/*

MIT License

Copyright(c) 2023 Paul Squires

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


bool Leg::isOpen()
{
    return  (open_quantity == 0 ? false : true);
}


void Trade::SetTradeOpenStatus()
{
    // default that the Trade is closed
    is_open = false;

    // Roll up all of the SHARES or FUTURES TransDetail and display the aggregate rather than the individual legs.
    int aggregate = 0;
    bool do_quantity_check = false;

    for (const auto &trans : transactions) {
        for (const auto &leg : trans->legs) {
            if (leg->underlying == L"OPTIONS") {
                if (leg->isOpen()) {
                    // A leg is open so therefore the Trade must stay open
                    this->is_open = true;
                    return;
                }
            } else if (leg->underlying == L"SHARES") {
               aggregate = aggregate + leg->open_quantity;
               do_quantity_check = true;
            } else if (leg->underlying == L"FUTURES") {
                aggregate = aggregate + leg->open_quantity;
                do_quantity_check = true;
            }
        }
    }

    // If this was a SHARES or FUTURES rollup then check to see if the aggregate amount is ZERO. 
    if (do_quantity_check) {
        this->is_open = (aggregate == 0 ? false : true);
        return;
    }

    // If the Trade is closed then set the latest Buying Power date tot he close date
    this->bp_end_date = this->oldest_trade_trans_date;
}


void Trade::CreateOpenLegsVector()
{
    // Create the openLegs vector. We need this vector because we have to sort the
    // collection of open legs in order to have Puts before Calls. There could be
    // multiple TransDetail in this trade and therefore Puts and Calls would not
    // already be a suitable display order.

    open_legs.clear();

    for (const auto &trans : transactions) {
        if (trans->multiplier > 0) this->multiplier = trans->multiplier;
        for (auto &leg : trans->legs) {
            if (leg->isOpen()) {
                leg->trans = trans;
                open_legs.push_back(leg);
            }
        }
    }

    // Finally, sort the vector based on Puts first with lowest Strike Price.
    std::sort(open_legs.begin(), open_legs.end(),
        [](const auto leg1, const auto leg2) {
            if (leg1->PutCall == L"P" && leg2->PutCall == L"C") {return true;}
            if (leg1->PutCall == L"P" && leg2->PutCall == L"P") {
                if (std::stod(leg1->strike_price) < std::stod(leg2->strike_price)) return true;
            }
            if (leg1->PutCall == L"C" && leg2->PutCall == L"C") {
                if (std::stod(leg1->strike_price) < std::stod(leg2->strike_price)) return true;
            }
            return false;
        });
}


