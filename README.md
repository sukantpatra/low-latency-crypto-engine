# Low-Latency Crypto Engine

A high-frequency trading (HFT) order book engine built in C++, designed to process live Binance Level 2 WebSocket market data with sub-microsecond latency.

## ⚡ Performance & The Journey to 120ns

This engine was systematically profiled and optimized to transition from standard software engineering practices to an institutional-grade, cache-conscious HFT architecture. 

| Architecture Phase | Core Mechanics | Average Latency (per order) |
| :--- | :--- | :--- |
| **V1: The Baseline** | `boost::container::flat_map`, `std::stod()`, Spinlocks | **~10,000 - 50,000+ ns** |
| **V2: Lock-Free** | O(1) Tombstoning (`qty = 0`), `std::memory_order_relaxed` | **~600 - 1,400 ns** |
| **V3: Institutional** | Direct Array Indexing, Custom Fixed-Point Parsers, `-O3` | **~115 - 200 ns** |

### Key Architectural Optimizations:
* **O(1) Direct Array Indexing:** Replaced binary search trees (`flat_map`) with statically allocated, direct-mapped arrays. Price levels map directly to memory offsets (`index = price_int`), eliminating hash collisions and tree-rebalancing entirely.
* **Custom Zero-Allocation Parsers:** Bypassed `std::stod` by writing custom `Parse_fixed_point` and `FastStod` functions. These parse `simdjson` string views directly into integers and floats without OS locale checks or heap allocations.
* **Lock-Free Hot Path:** Completely eliminated mutexes and spinlocks. Cross-thread metric tracking is handled via `std::atomic` with `std::memory_order_relaxed` to prevent L1/L2 cache-line bouncing.
* **Zero-Copy JSON Parsing:** Utilizes `simdjson` to parse streaming WebSocket payloads directly from the network buffer.

## 🛠️ Tech Stack
* **C++20**
* **Boost.Asio / Boost.Beast** (Asynchronous Networking & WebSockets)
* **simdjson** (SIMD-accelerated JSON parsing)
* **OpenSSL** (Secure WSS connections)
* **CMake** (Cross-platform build system)

## 🚀 Build Instructions

This project supports cross-compilation on both **macOS (Apple Silicon/Intel)** and **Windows (MSYS2/MinGW)**.

### macOS (Apple Silicon / Intel)
```bash
# Generate the Release build environment (unlocks SIMD vectorization)
cmake -B build -DCMAKE_BUILD_TYPE=Release

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
