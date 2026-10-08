#pragma once

namespace bsp {
// Whole 00414DB0: ECX is the actual borrowed pose owner, RET, no stack input.
// Required live bytes: parent pointer+3C, local64B+74, valid byte+C8,
// world64B+CC, derived-valid byte+10C. Every reached ancestor must remain
// valid and stable; dirty ancestry must terminate. Any nonzero C8 returns.
// Native recursion reloads parent after refresh. Raw canonical 00413920 and
// 004134F0 preserve their full matrix/x87/alias/ambient-stack requirements.
// Reaching parent matrix multiply requires all EIGHT x87 slots free. A root
// copy alone needs one free slot. Production does not change the FP state.
// No resolver, semantic pose view, ownership, cache copy or default is supplied.
// The unused EDX parameter spells the native ECX-only entry in C++.
void __fastcall refresh_native_entity_pose_00414db0(
    void* actual_pose_owner, void* unused_edx);
} // namespace bsp
