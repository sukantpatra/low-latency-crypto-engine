#include "Spinlock.hpp"

void Spinlock::lock() {
    while (locked.test_and_set(std::memory_order_acquire)) {
        // --- APPLE SILICON OPTIMIZATION ---
#if defined(__aarch64__)
        __asm__ volatile("yield" ::: "memory");
#elif defined(__x86_64__)|| defined(_M_X64)
    #if defined(_MSC_VER)
            _mm_pause(); // Windows MSVC intrinsic for x86/x64 pause
    #else
            __asm__ volatile("pause" ::: "memory"); // GCC / Clang on x86
    #endif
#endif
    }
}

void Spinlock::unlock() {
    locked.clear(std::memory_order_release);
}