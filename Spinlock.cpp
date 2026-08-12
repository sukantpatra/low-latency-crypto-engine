#include "Spinlock.hpp"

void Spinlock::lock() {
    while (locked.test_and_set(std::memory_order_acquire)) {
        // --- APPLE SILICON OPTIMIZATION ---
#if defined(__aarch64__)
        __asm__ volatile("yield" ::: "memory");
#elif defined(__x86_64__)
        __asm__ volatile("pause" ::: "memory"); // Same thing for Intel/AMD 
#endif
    }
}

void Spinlock::unlock() {
    locked.clear(std::memory_order_release);
}