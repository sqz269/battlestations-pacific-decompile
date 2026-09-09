#include "bsp/random_threads.hpp"
#include <new>

namespace bsp {
TrackedCriticalSection* critical_section_create_00bd1860() {
    auto* lock = new (std::nothrow) TrackedCriticalSection;
    if (lock) {
        InitializeCriticalSection(&lock->native);
        lock->depth = 0;
    }
    return lock;
}

void critical_section_destroy_owned_0041cc80(TrackedCriticalSection*& lock) {
    if (!lock) return;
    while (lock->depth > 0) {
        --lock->depth;
        LeaveCriticalSection(&lock->native);
    }
    DeleteCriticalSection(&lock->native);
    delete lock;
    // 0041ccb6: MOV [EBP],0, hidden by the old incorrect free noreturn flag.
    lock = nullptr;
}

namespace {
class Guard {
public:
    explicit Guard(TrackedCriticalSection* lock) : lock_(lock) {
        if (lock_) { EnterCriticalSection(&lock_->native); ++lock_->depth; }
    }
    ~Guard() {
        if (lock_) { --lock_->depth; LeaveCriticalSection(&lock_->native); }
    }
    Guard(const Guard&) = delete;
    Guard& operator=(const Guard&) = delete;
private:
    TrackedCriticalSection* lock_;
};
}

RandomThreads::RandomThreads() {
    // Original CRT startup constructs 20 states before RandomThreads_Initialize.
    for (auto& state : states_) random_construct_default_00bd2e00(state);
    // Shared fallback has guard bytes 1,0 on disk and is statically seeded with 1105h.
    random_seed_00bf0cf0(fallback_, 0x1105);
    // 00bd2e20 creates the lock and clears IDs; high-water starts in zeroed globals.
    lock_ = critical_section_create_00bd1860();
}

RandomThreads::~RandomThreads() {
    // 00bd30d0 tail-calls 0041cc80 with the global lock pointer's address.
    critical_section_destroy_owned_0041cc80(lock_);
}

void RandomThreads::register_current_00bd2fe0() {
    Guard guard(lock_);
    for (std::int32_t i = 0; i < 10; ++i) {
        if (ids_[i] == 0) {
            ids_[i] = GetCurrentThreadId();
            if (high_water_ < i + 1) high_water_ = i + 1;
            break;
        }
    }
}

void RandomThreads::unregister_current_00bd3050() {
    Guard guard(lock_);
    const auto id = GetCurrentThreadId();
    for (std::int32_t i = 0; i < 10; ++i) {
        if (ids_[i] == id) {
            ids_[i] = 0;
            if (i == high_water_ - 1)
                while (high_water_ > 0 && ids_[high_water_ - 1] == 0) --high_water_;
            break;
        }
    }
}

RandomState& RandomThreads::state_00bd2ed0(RandomStream stream) {
    const auto id = GetCurrentThreadId();
    for (std::int32_t i = 0; i < high_water_; ++i)
        if (ids_[i] == id) return states_[2 * i + static_cast<std::uint32_t>(stream)];
    return fallback_;
}

std::uint32_t RandomThreads::next_00bd2fc0(RandomStream stream) {
    return random_next_u32_00ba2c20(state_00bd2ed0(stream));
}
void RandomThreads::seed_00bd2fd0(RandomStream stream, std::uint32_t seed) {
    random_seed_00bf0cf0(state_00bd2ed0(stream), seed);
}
}
