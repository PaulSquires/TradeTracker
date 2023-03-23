/* Copyright (C) 2019 Interactive Brokers LLC. All rights reserved. This code is subject to the terms
 * and conditions of the IB API Non-Commercial License or the IB API Commercial License, as applicable. */
#include "pch.h"

#include "Utils.h"
#include <iostream>
#include <sstream>
#include <climits>
#include <cfloat>
#include "platformspecific.h"

static const std::string base64_chars = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

inline bool Utils::is_base64(std::uint8_t c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}

std::vector<std::uint8_t> Utils::base64_decode(std::string const& encoded_string) {
   // int in_len = encoded_string.size();
   // int i = 0;
   // int j = 0;
   //int in_ = 0;
   // std::uint8_t char_array_4[4], char_array_3[3];
    std::vector<std::uint8_t> ret;

   // while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
   //     char_array_4[i++] = encoded_string[in_]; in_++;
   //     if (i ==4) {
   //         for (i = 0; i <4; i++)
   //             char_array_4[i] = base64_chars.find(char_array_4[i]);

   //         char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
   //         char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
   //         char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

   //         for (i = 0; (i < 3); i++)
   //             ret.push_back(char_array_3[i]);
   //         i = 0;
   //     }
   // }

   // if (i) {
   //     for (j = i; j <4; j++)
   //         char_array_4[j] = 0;

   //     for (j = 0; j <4; j++)
   //     char_array_4[j] = base64_chars.find(char_array_4[j]);

   //     char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
   //     char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
   //     char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

   //     for (j = 0; (j < i - 1); j++) ret.push_back(char_array_3[j]);
   // }

    return ret;
}

std::string Utils::doubleMaxString(double d, std::string def) {
    if (d == DBL_MAX) {
        return def;
    } else {
        std::ostringstream oss;
        oss.precision(8);
        oss << std::fixed << d;
        std::string str = oss.str();

        std::size_t pos1 = str.find_last_not_of("0");
        if (pos1 != std::string::npos)
            str.erase(pos1 + 1);

        std::size_t pos2 = str.find_last_not_of(".");
        if (pos2 != std::string::npos)
            str.erase(pos2 + 1);

        return str;
    }
}

std::string Utils::intMaxString(int value) {
    return value == INT_MAX ? "" : std::to_string(value);
}

std::string Utils::longMaxString(long value) {
    return value == LONG_MAX ? "" : std::to_string(value);
}

std::string Utils::llongMaxString(long long value) {
    return value == LLONG_MAX ? "" : std::to_string(value);
}

std::string Utils::doubleMaxString(double d) {
    return doubleMaxString(d, "");
}