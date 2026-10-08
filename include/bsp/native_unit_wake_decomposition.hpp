#pragma once

namespace bsp {

// Ordinary C++ interface to the complete native 00811180 wake leaf.
// actual_entity supplies the original addressed storage: 40 records at +0xBD8,
// stride 0x18, and the signed head at +0xF98 (caller domain: 0..39).
// point addresses three readable float words. Outputs address writable float
// words and may alias each other, point, or writable entity storage. Across is
// stored first; along is stored second. There is no validity/written guard.
// Requires all eight x87 stack slots free. Preserves ambient x87 CW and MXCSR
// settings; arithmetic status effects and the actual read/spill order remain.
// Uses the existing native normalizer and genuine current CRT _CIsqrt;
// original CRT dispatch/diagnostic policy and entity production are unproved.
void decompose_native_unit_wake_00811180(const void* actual_entity,
    const float* point, float* across, float* along);

} // namespace bsp
