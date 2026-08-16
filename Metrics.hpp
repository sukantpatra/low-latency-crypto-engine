#pragma once
#include <atomic>
#include <cstdint>

extern std::atomic<uint64_t> total_latency_ns;
extern std::atomic<uint64_t> total_messages_processed;
extern std::atomic<uint64_t> total_orders_processed;
