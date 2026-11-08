// Focused ownership fixture for the production ZoneOwner, without running the
// game. Substitute only the heap boundary to detect wrong/double frees and the
// constructor-failure path reading unconstructed storage.
#include "bsp/singleton_lifetime.hpp"
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>
#include <stdexcept>

namespace {
std::array<void*, 8> allocations{}, released{};
std::size_t allocated{}, freed{};
bool wrong_free{};
}
namespace bsp {
void* fixture_zone_allocate(const SingletonAllocationRequest& request) {
    void* value = std::malloc(request.host_bytes);
    if (!value) throw std::bad_alloc();
    std::memset(value, 0xcd, request.host_bytes);
    allocations.at(allocated++) = value;
    return value;
}
void fixture_zone_free(void* value) noexcept {
    std::size_t slot = 0;
    while (slot < allocated && allocations[slot] != value) ++slot;
    if (slot == allocated || freed == released.size()) { wrong_free = true; return; }
    allocations[slot] = nullptr;
    released[freed++] = value;
    std::free(value);
}
}
#define singleton_lifetime_allocate fixture_zone_allocate
#define singleton_lifetime_free fixture_zone_free
#include "../src/game_avoid_zone_runtime.cpp"
#undef singleton_lifetime_allocate
#undef singleton_lifetime_free

int main() {
    // A failed scene-path/zone construction must release just the object. Its
    // poisoned fields are not a successfully constructed polygon.
    { bsp::game::ZoneOwner failed; }
    if (wrong_free || allocated != 1 || freed != 1) return 1;

    void *zone, *slots, *first, *second;
    {
        bsp::game::ZoneOwner owner;
        zone = owner.native;
        slots = bsp::game::allocate_array(nullptr, 2 * sizeof(void*));
        first = bsp::game::allocate_record(nullptr, sizeof(bsp::ShipAiPathLateralRecord));
        second = bsp::game::allocate_record(nullptr, sizeof(bsp::ShipAiPathLateralRecord));
        owner.native->corners.records = static_cast<bsp::ShipAiPathLateralRecord**>(slots);
        owner.native->corners.records[0] = static_cast<bsp::ShipAiPathLateralRecord*>(first);
        owner.native->corners.records[1] = static_cast<bsp::ShipAiPathLateralRecord*>(second);
        owner.native->corners.count = 2;
        owner.constructed = true;
    }
    if (wrong_free || allocated != 5 || freed != 5 || released[1] != first ||
        released[2] != second || released[3] != slots || released[4] != zone) return 2;
    std::puts("PASS: production zone owner frees records, pointer buffer, object once; failed construction frees only object");
    return 0;
}
