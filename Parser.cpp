#include "Parser.hpp"
#include "simdjson.h"
#include <charconv>

enum class OrderSide { BUY, SELL };

extern void UpdateBook(OrderSide side, uint64_t price, double qty);
extern uint64_t currentUpdateId;

uint64_t Parser::ParseJson(const char* jsonStr, size_t length, size_t capacity)
{
    simdjson::ondemand::document doc = parser.iterate(jsonStr, length, capacity);
    uint64_t updateId = doc["u"].get_uint64();
    if(updateId <= currentUpdateId) {
        // Ignore this update as it's older than the current state
        return 0;
    }

    currentUpdateId = updateId;

    uint64_t orderCount = 0;

    for(simdjson::ondemand::array bid : doc["b"])
    {
        auto it = bid.begin();
        uint64_t price = Parse_fixed_point((*it).get_string());
        ++it;
        double qty = FastStod((*it).get_string());
        UpdateBook(OrderSide::BUY, price, qty);
        orderCount++;
    }

    for(simdjson::ondemand::array ask : doc["a"])
    {
        auto it = ask.begin();
        uint64_t price = Parse_fixed_point((*it).get_string());
        ++it;
        double qty = FastStod((*it).get_string());
        UpdateBook(OrderSide::SELL, price, qty);
        orderCount++;
    }

    return orderCount;
}

void Parser::ParseSnapshotJson(std::string_view jsonStr)
{
    
    try{
        simdjson::padded_string padded_json(jsonStr);
        simdjson::ondemand::document doc = parser.iterate(padded_json);
        for(simdjson::ondemand::array bid : doc["bids"])
        {   
        auto it = bid.begin();
        uint64_t price = Parse_fixed_point((*it).get_string());
        ++it;
        double qty = FastStod((*it).get_string());
        UpdateBook(OrderSide::BUY, price, qty);
        }

        for(simdjson::ondemand::array ask : doc["asks"])
        {   
        auto it = ask.begin();
        uint64_t price = Parse_fixed_point((*it).get_string());
        ++it;
        double qty = FastStod((*it).get_string());
        UpdateBook(OrderSide::SELL, price, qty);
        }

        currentUpdateId = doc["lastUpdateId"].get_uint64();
    }
    catch (const simdjson::simdjson_error &e) {
        std::cerr << "Error parsing bids: " << e.what() << std::endl;
    }
}


