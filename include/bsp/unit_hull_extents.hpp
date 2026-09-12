#pragma once
#include "bsp/hit_narrowphase.hpp"

namespace bsp {
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
