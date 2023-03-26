#include "framework.h"
#include "trade.h"


bool Leg::isOpen()
{
    return  (openQuantity == 0 ? false : true);
}

Transaction::~Transaction()
{
    // Destroy all of the Leg pointers in the QList
    for (const auto &leg : legs) {
        delete leg;
    }
}

Trade::~Trade()
{
    // Destroy all of the Transaction pointers in the QList
    for (const auto &trans : transactions) {
        delete trans;
    }
}


void Trade::setTradeOpenStatus()
{
    // default that the Trade is closed
    isOpen = false;

    // Roll up all of the SHARES or FUTURES transactions and display the aggregate rather than the individual legs.
    int aggregate = 0;
    bool doQuantityCheck = false;

    for (const auto &trans : transactions) {
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
    // multiple transactions in this trade and therefore Puts and Calls would not
    // already be a suitable display order.

    openLegs.clear();

    for (const auto &trans : transactions) {
        for (const auto &leg : trans->legs) {
            if (leg->isOpen()) {
                openLegs.push_back(leg);
            }
        }
    }

    // Finally, sort the vector based on Puts first with lowest Strike Price.
    std::sort(openLegs.begin(), openLegs.end(),
        [](const Leg* leg1, const Leg* leg2) {
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


