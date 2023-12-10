#pragma once

#include <iostream>
#include <chrono>

std::chrono::steady_clock::time_point start;
inline void TIMER_START() {
    start = std::chrono::high_resolution_clock::now();
}

inline void TIMER_END() {
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "Time taken by code section: " << duration.count() << " milliseconds" << std::endl;
}

