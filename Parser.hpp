#pragma once
#include <string>
#include <cstddef>
#include "simdjson.h"
#include <string_view>


enum class OrderSide;

class Parser {
    private:
        simdjson::ondemand::parser parser;
        static constexpr double POWERS_OF_TEN[] = {
        1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 
        1e9, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15
    };

        inline uint64_t Parse_fixed_point(std::string_view str)
        {

            uint64_t result = 0;
        int fraction_digits = 0;
        bool in_fraction = false;

        for (char c : str) {
            if (c == '.') {
                in_fraction = true;
                continue;
            }
            result = result * 10 + (c - '0');
            if (in_fraction) {
                fraction_digits++;
            }
        }
        
        while (fraction_digits < 2) {
            result *= 10;
            fraction_digits++;
        }
        return result;
        }
        
        inline double FastStod(std::string_view str)
        {
        uint64_t result = 0;
        int fraction_digits = 0;
        bool in_fraction = false;

        for (char c : str) {
        if (c == '.') {
            in_fraction = true;
            continue;
        }
        
        result = result * 10 + (c - '0');
        
        if (in_fraction) {
            fraction_digits++;
        }
        }
        if (fraction_digits > 15) fraction_digits = 15;
        return static_cast<double>(result) / POWERS_OF_TEN[fraction_digits];
        }


    public: 
        uint64_t ParseJson(const char* jsonStr, size_t length, size_t capacity);
        void ParseSnapshotJson(std::string_view jsonStr);
};