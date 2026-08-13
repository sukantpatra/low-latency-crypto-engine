#pragma once
#include <atomic>

#if defined(_MSC_VER)
#include <emmintrin.h> // Required for _mm_pause() on Windows
#endif

class Spinlock
{
    private:
    std::atomic_flag locked = ATOMIC_FLAG_INIT;

public:
    void lock();
    void unlock();
};