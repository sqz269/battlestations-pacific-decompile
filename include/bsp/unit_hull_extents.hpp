#pragma once
#include "bsp/hit_narrowphase.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {
struct UnitHullInitializationHost {
    virtual ~UnitHullInitializationHost() = default;
    virtual void attach_lua_self_0077e830(void* unit) = 0;
    virtual void* allocate_00bf681b(std::size_t bytes) = 0;
    virtual void* construct_director_008366d0(void* memory, void* unit) = 0;
    virtual void instantiate_devices_0095a880(void* unit, std::uint32_t a,
        std::uint32_t b, std::uint32_t c) = 0;
    virtual void notify_slot_210(void* unit) = 0;
};
// 00810F60: ECX=unit, RET0. Full normal body around the established x87
// hull fragment. Actual raw pointers; the native allocation/FH3 ABI is unproven.
void initialize_unit_director_and_hull_00810f60(void* unit, void* const& game_00e188a8,
    UnitHullInitializationHost& host);
// Semantic storage, not the native unit object layout.
struct UnitHullExtents {
    float length_09c8{};
    float width_09cc{}; // Full width, despite older consumers' half-width name.
};

struct UnitHullExtentClassInputs {
    // Actual class+50 resource's local box at +28..+3C: minXYZ, maxXYZ.
    // A null pointer selects the native no-model branch. Do not substitute a
    // world-space collision box or fabricate a box from the class dimensions.
    const HitQueryBounds* model_local_box{};
    float length_00a0{}; // VehicleClass.Length, store0096039F.
    float width_00a4{};  // VehicleClass.Width, store00960368.
};

// Extent-only fragment00810FAF..00811073 of00810F60 (__thiscall unit, RET).
// A null class preserves both prior fields. With no model, copy class A4/A0;
// with a model, store 2*max(-minX,maxX), then 2*max(-minZ,maxZ), using the
// original x87 float spills, comparison/tie/unordered choice, and double2.
// These are symmetric full extents about the local origin, not max-minus-min.
// New C++ interface; does not attach Lua self, construct a weapon director or
// execute the enclosing routine's resource-kind notification tail.
void produce_unit_hull_extents_00810faf(UnitHullExtents& output,
    const UnitHullExtentClassInputs* unit_class) noexcept;
}
