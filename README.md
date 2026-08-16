# Low-Latency Crypto Engine

A high-frequency trading (HFT) order book engine built in C++, designed to process live Binance Level 2 WebSocket market data with sub-microsecond latency.

## ⚡ Performance & Architecture
This engine is heavily optimized for zero-contention and maximum CPU cache efficiency, processing order updates in the **~600ns to 1.4µs** range per order.

* **Lock-Free Hot Path:** Completely eliminated mutexes and spinlocks. Cross-thread metric tracking is handled via `std::atomic` with `std::memory_order_relaxed` to prevent cache-line bouncing.
* **O(1) Tombstoning:** Order book deletions are handled via "Soft Deletes" (Quantity = 0.0) instead of `erase()` operations. This prevents O(N) memory shifts in the underlying `boost::container::flat_map`, ensuring insertion and deletion times remain deterministic.
* **Zero-Copy JSON Parsing:** Utilizes `simdjson` (SIMD-accelerated JSON parser) to parse streaming WebSocket payloads directly from the buffer without copying memory.

## 🛠️ Tech Stack
* **C++20**
* **Boost.Asio / Boost.Beast** (Asynchronous Networking & WebSockets)
* **Boost.Container** (`flat_map` for cache-friendly contiguous memory)
* **simdjson** (SIMD-accelerated JSON parsing)
* **OpenSSL** (Secure WSS connections)
* **CMake** (Cross-platform build system)

## 🚀 Build Instructions

This project supports cross-compilation on both **macOS (Apple Silicon/Intel)** and **Windows (MSYS2/MinGW)**.
### macOS
```bash
# Generate the build environment
cmake -B build

# Compile the engine
cmake --build build -j $(sysctl -n hw.ncpu)

# Run
./build/HFT_Basics
```
### Windows (MSYS2 / UCRT64)
Ensure you have the MSYS2 UCRT64 environment installed with GCC, CMake, and Boost.
```Powershell
# Generate the build environment
cmake -B build -G "MinGW Makefiles"

# Compile the engine
cmake --build build -j 16

# Run
.\build\HFT_Basics.exe
```
