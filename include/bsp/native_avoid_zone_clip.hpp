#pragma once

namespace bsp {

// Complete raw-storage Source reconstruction of 0041A540..0041AC53 (1812 B).
// Original: ECX = 24h avoidance-zone storage; two stacked min/max XYZ pointers;
// RET 8, no semantic EAX result. Only X (+0) and Z (+8) bounds are consumed.
// Source keeps that physical entry shape with an unused EDX placeholder. It
// borrows the actual incoming argument cells and dispatches the whole literal
// algorithm through a persistent working set with two current C++ cell guards.
//
// Real current-heap allocation/free and concrete raw record cleanup are used.
// Caller-owned storage must meet the original raw pointer/count/alias contract;
// in particular an overlapping zone with zero corners still reads the previous
// point before the count test. No validation, rollback or pending-record guard
// is added. Derived bounds, layer and associated-entity fields remain untouched.
//
// Source frame/local addresses, its EBP spill, current C++ EH/CRT, and relocated
// constants/providers differ from Native. This is not Native FH3/SEH/frame or
// drop-in/gameplay proof. See docs/GOAL15I_AVOID_ZONE_CLIP.md for the full scope.
void __fastcall clip_native_avoid_zone_world_bounds_0041a540(
    void* actual_zone, void* unused_edx,
    const float* minimum_xyz, const float* maximum_xyz);

} // namespace bsp
