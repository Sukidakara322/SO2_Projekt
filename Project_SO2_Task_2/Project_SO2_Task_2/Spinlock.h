#pragma once
#include <atomic>

// A simple class for thread synchronization
class SpinLock {
    std::atomic_flag flag = ATOMIC_FLAG_INIT;  // Atomic flag used as the lock state (false = unlocked, true = locked)

public:
    // Spins (busy-waits) until the lock becomes available
    void lock() noexcept {
        while (flag.test_and_set(std::memory_order_acquire)) {
            // busy-wait loop (keeps spinning until the lock is acquired)
        }
    }

    // Releases the lock by clearing the flag
    void unlock() noexcept {
        flag.clear(std::memory_order_release);
    }

    // Attempts to acquire the lock once, returns true if successful, false otherwise.
    bool try_lock() noexcept {
        return !flag.test_and_set(std::memory_order_acquire);
    }
};
