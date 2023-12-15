/* Copyright (C) 2019 Interactive Brokers LLC. All rights reserved. This code is subject to the terms
 * and conditions of the IB API Non-Commercial License or the IB API Commercial License, as applicable. */
#pragma once
#ifndef TWS_API_SAMPLES_TESTCPPCLIENT_UTILS_H
#define TWS_API_SAMPLES_TESTCPPCLIENT_UTILS_H

#include <vector>
#include <cstdint>
#include <string>

class Utils {

public:
    static inline bool is_base64(std::uint8_t c);
    static std::vector<std::uint8_t> base64_decode(std::string const&);

    static std::string doubleMaxString(double d, std::string def);
    static std::string doubleMaxString(double d);
    static std::string intMaxString(int value);
    static std::string longMaxString(long value);
    static std::string llongMaxString(long long value);
};

#endif
