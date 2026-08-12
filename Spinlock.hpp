#pragma once
#include <atomic>

class Spinlock
{
    private:
    std::atomic_flag locked = ATOMIC_FLAG_INIT;

public:
    void lock();
    void unlock();
};