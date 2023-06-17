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
    return  (openQuantity == 0 ? false : true);
}


void Trade::setTradeOpenStatus()
{
    // default that the Trade is closed
    isOpen = false;

    // Roll up all of the SHARES or FUTURES TransDetail and display the aggregate rather than the individual legs.
    int aggregate = 0;
    bool doQuantityCheck = false;

    for (const auto &trans : Transactions) {
        for (const auto &leg : trans->legs) {
            if (leg->underlying == L"OPTIONS") {
                if (leg->isOpen()) {
                    // A leg is open so therefore the Trade must stay open
                    this->isOpen = true;
                    return;
                }
            } else if (leg->underlying == L"SHARES") {
               aggregate = aggregate + leg->openQuantity;
               doQuantityCheck = true;
            } else if (leg->underlying == L"FUTURES") {
                aggregate = aggregate + leg->openQuantity;
                doQuantityCheck = true;
            }
        }
    }

    // If this was a SHARES or FUTURES rollup then check to see if the aggregate amount is ZERO. If yes then
    // delete the added Tree item and set the Trade isOpen to False and exit.
    if (doQuantityCheck) {
        this->isOpen = (aggregate == 0 ? false : true);
        return;
    }
}


void Trade::createOpenLegsVector()
{
    // Create the openLegs vector. We need this vector because we have to sort the
    // collection of open legs in order to have Puts before Calls. There could be
    // multiple TransDetail in this trade and therefore Puts and Calls would not
    // already be a suitable display order.

    openLegs.clear();

    for (const auto &trans : Transactions) {
        for (const auto &leg : trans->legs) {
            if (leg->isOpen()) {
                openLegs.push_back(leg);
            }
        }
    }

    // Finally, sort the vector based on Puts first with lowest Strike Price.
    std::sort(openLegs.begin(), openLegs.end(),
        [](const auto leg1, const auto leg2) {
            if (leg1->PutCall == L"P" && leg2->PutCall == L"C") {return true;}
            if (leg1->PutCall == L"P" && leg2->PutCall == L"P") {
                if (std::stod(leg1->strikePrice) < std::stod(leg2->strikePrice)) return true;
            }
            if (leg1->PutCall == L"C" && leg2->PutCall == L"C") {
                if (std::stod(leg1->strikePrice) < std::stod(leg2->strikePrice)) return true;
            }
            return false;
        });
}


