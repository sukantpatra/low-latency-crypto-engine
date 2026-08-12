# low-latency-crypto-engine
A low-latency, zero-copy Binance order book engine written in C++ optimized for Apple Silicon.

## Performance & Benchmarks
This engine was designed with strict low-latency (HFT) C++ principles. On Apple Silicon (M-Series), the hot-path achieves a complete network-to-memory state update in **38µs to 260µs** (payload dependent).

### Latency Optimizations Implemented:
* **True Zero-Copy Networking:** Boost.Beast's raw socket buffers are fed directly into `simdjson`'s hardware registers, padding included. The JSON is parsed without a single `malloc` or `std::string` copy.
* **Cache-Friendly Data Structures:** Standard pointer-chasing `std::map` red-black trees were replaced with `boost::container::flat_map`. By maintaining a strictly contiguous array, the CPU hardware prefetcher loads the order book into L1 cache, turning O(N) memory shifts into sub-nanosecond operations.
* **Asymmetric OS Scheduling (QoS):** 
  * The network event loop is tagged with `QOS_CLASS_USER_INTERACTIVE` to aggressively pin the hot path to macOS **Performance (P) Cores**.
  * The UI/Console rendering thread is tagged with `QOS_CLASS_BACKGROUND` to banish blocking I/O system calls to **Efficiency (E) Cores**.
* **User-Space Synchronization:** OS-level `std::mutex` locks were stripped from the hot path and replaced with a custom **Spinlock** utilizing `std::atomic_flag` and ARM `yield` instructions, preventing the kernel from ever putting the network thread to sleep.
