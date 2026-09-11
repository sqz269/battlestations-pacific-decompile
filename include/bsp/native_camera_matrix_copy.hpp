#pragma once

namespace bsp {
// 004134F0: ECX destination, stack source, RET4; EAX remains destination.
// Sixteen sequential x87 FLD/FSTP pairs, including original overlap and
// floating-state effects. Raw byte views may be unaligned. Both pointers
// must support the accesses actually reached; no validation or snapshot.
// The unused EDX argument places source in the original stack position.
// This entry does not make companion-backed camera hierarchy words raw.
void* __fastcall copy_native_camera_matrix_004134f0(
    void* destination, void* unused_edx, const void* source);
} // namespace bsp
