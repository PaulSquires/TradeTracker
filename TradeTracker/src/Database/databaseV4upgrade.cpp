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

#include <queue>

#include "trade.h"
#include "database.h"



// ========================================================================================
// Calculate the running cost for each leg in the Trade. Need to do this upgrade because
// the existing database does not have leg backpointers for the new leg of a rolled trans.
// ========================================================================================
void UpgradeOptionTransaction(const std::shared_ptr<Trade> trade, const std::shared_ptr<Transaction> trans) {

    // Attempt to trace the legs to a point where missing Rolled Leg backpointer data can be updated.

    std::queue<int> backpointer_queue;

    // Only need to process a "Roll" transaction. Unfortunately, there is no built in way to
    // identify a rolled transaction other than the generated "Roll" description. If the user
    // had changed this description then this update will nnot correctly update the pointer.
    if (trans->description != L"Roll") return;


    for (auto& leg : trans->legs) {

        // Only process BTC or STC legs if they were not completely closed. Calculate
        // the income/cost from the leg and update the back_pointer with the value.
        if (leg->action == Action::BTC || leg->action == Action::STC) {
            if (leg->leg_back_pointer_id) {
                backpointer_queue.push(leg->leg_back_pointer_id);
            }
            continue;
        }

        if (leg->action == Action::STO || leg->action == Action::BTO) {
            if (leg->leg_back_pointer_id == 0 && !backpointer_queue.empty()) {
                leg->leg_back_pointer_id = backpointer_queue.front();
                backpointer_queue.pop();
            }
            continue;
        }
    }
}


// ========================================================================================
// Calculate the running cost for each leg in the Trade. Need to do this upgrade because
// the existing database does not have leg backpointers for the new leg of a rolled trans.
// ========================================================================================
void DatabaseV4upgrade() {
    for (const auto& trade: trades) {
        for (const auto& trans : trade->transactions) {
            if (trans->underlying != Underlying::Options) continue;
            UpgradeOptionTransaction(trade, trans);
        }
    }
    db.SaveDatabase();
}

