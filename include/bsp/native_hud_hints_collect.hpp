#pragma once

#include "bsp/native_scene_dword_vector_push_back.hpp"

namespace bsp {
// Source metadata only, not the Original screen/iterator/vector layout.
// actual_parameter4 is the readable live destination-pointer DWORD at native
// collector entry+4. The child input home is writable: this parent stores the
// ADDRESS screen+1C there before reading that current destination pointer.
// Metadata stays stable/disjoint from homes and all reached payload/backing.
// Valid home/payload aliases retain that order; native private-frame/address
// coincidence and arbitrary asynchronous mutation are not established.
struct NativeHudHintsCollectFrame {
    const void* actual_parameter4;
    void* actual_push_parameter4;
    void* actual_push_private_output8;
    const NativeSceneDwordVectorInsertOneFrame* child_frame;
};

// Whole0054E370[16B/5ops], original ECX genuine7Ch hints screen, one stacked
// destination-vector pointer, RET4. Source adds explicit homes/child backing.
// Actual screen+1C must remain a readable page-pointer slot. No page-value
// snapshot, null-page skip, reference acquisition or page destruction occurs.
// The real child owns the late reads/callbacks/One/N behavior; fallback needs
// actual writable output8/One/N backing, including on an initially empty vector.
// Use its SAME actual Source allocator/free/providers and valid raw vector16.
// Screen/Page/vector/providing owners and real provenance remain caller proof;
// no semantic receiver cast, fabricated class or automatic authentication.
// Ordinary C++ Source/private ABI; child failure contracts and completed effects
// remain qualified. No Original register/stack/FH3/SEH/fault or game admission.
void collect_native_hud_hints_children_0054e370(void* actual_hints,
    const NativeHudHintsCollectFrame&);
} // namespace bsp
