#pragma once

#include "bsp/ship_ai_path_corridor.hpp"

#include <array>
#include <cstddef>
#include <vector>

namespace bsp::game {

// Caller-owned Win32 layout for the existing whole UnitGroup bodies. The
// host's member identities are one-based unit registry words, not pointers to
// native unit objects. Only the non-dereferencing record/identity getters may
// use this projection; its native profile word must never be dispatched.
// Members are refreshed at each read boundary, so joins, compaction, slot
// swaps and scripted offsets cannot leave a stale snapshot behind.
class GameUnitGroupStorage {
public:
    void initialize() noexcept;
    bool capture_members(const std::vector<ShipAiUnitGroupMember>& members) noexcept;
    void publish_speed_ceiling(float speed) noexcept;
    float speed_ceiling() const noexcept;
    void* data() noexcept { return bytes_.data(); }

private:
    alignas(4) std::array<std::byte, 0x508> bytes_{};
    bool initialized_{false};
};

} // namespace bsp::game
