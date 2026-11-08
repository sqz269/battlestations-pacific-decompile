#pragma once

#include "bsp/ship_ai_path_corridor.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "bsp/observer_lifetime.hpp"

namespace bsp::game {

// Caller-owned Win32 layout for the existing whole UnitGroup bodies. The
// host's member identities are one-based unit registry words, not pointers to
// native unit objects. Only non-dereferencing record/identity operations may
// use this projection; its native profile word must never be dispatched.
// First-leader publication admits 0070D0C0 only while the constructor's +14h
// is null, so the native wake handoff cannot dereference an encoded identity.
// Members are refreshed at each read boundary, so joins, compaction, slot
// swaps and scripted offsets cannot leave a stale snapshot behind.
class GameUnitGroupStorage {
public:
    void initialize() noexcept;
    std::uint32_t initialize_first_leader(std::uint32_t identity);
    bool capture_members(const std::vector<ShipAiUnitGroupMember>& members) noexcept;
    void publish_speed_ceiling(float speed) noexcept;
    float speed_ceiling() const noexcept;
    void* data() noexcept { return bytes_.data(); }
    // 0070DAB0 initializes the group's callback-owner base at +0. Its
    // address must survive formation_groups vector growth while edges borrow it.
    NativeObserverOwnerStorage& observer_owner() noexcept {
        return *reinterpret_cast<NativeObserverOwnerStorage*>(bytes_.data());
    }

private:
    alignas(4) std::array<std::byte, 0x508> bytes_{};
    bool initialized_{false};
};

} // namespace bsp::game
