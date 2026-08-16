#pragma once
#include <string>
#include <cstddef>
#include "simdjson.h"
#include <string_view>


enum class OrderSide;

class Parser {
    private:
        simdjson::ondemand::parser parser;

    public: 
        uint64_t ParseJson(const char* jsonStr, size_t length, size_t capacity);
        void ParseSnapshotJson(std::string_view jsonStr);
};