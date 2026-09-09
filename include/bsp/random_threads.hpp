#pragma once
#include "bsp/random.hpp"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace bsp {
struct TrackedCriticalSection {
    CRITICAL_SECTION native;
    std::int32_t depth;
};
static_assert(offsetof(TrackedCriticalSection, depth) == 0x18);
static_assert(sizeof(TrackedCriticalSection) == 0x1c);
TrackedCriticalSection* critical_section_create_00bd1860();
void critical_section_destroy_owned_0041cc80(TrackedCriticalSection*& lock);

enum class RandomStream : std::uint32_t { primary = 0, secondary = 1 };

// Owns the formerly-global state. Not a drop-in replacement for native addresses.
// Initialize before workers start; destroy only after they stop. Lookup is unlocked,
// as in the original; callers must coordinate registration vs concurrent lookup.
class RandomThreads {
public:
    RandomThreads();
    ~RandomThreads();
    RandomThreads(const RandomThreads&) = delete;
    RandomThreads& operator=(const RandomThreads&) = delete;
    void register_current_00bd2fe0();
    void unregister_current_00bd3050();
    RandomState& state_00bd2ed0(RandomStream stream);
    std::uint32_t next_00bd2fc0(RandomStream stream);
    void seed_00bd2fd0(RandomStream stream, std::uint32_t seed);
private:
    TrackedCriticalSection* lock_{};
    DWORD ids_[10]{};
    std::int32_t high_water_{};
    RandomState states_[20];
    RandomState fallback_;
};
}
