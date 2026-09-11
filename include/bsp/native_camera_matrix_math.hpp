#pragma once

namespace bsp {
// Raw 64-byte row-major matrix storage. No typed camera/node overlay is used.
// All addressed bytes and native stack space must be valid. These entries keep
// original x87/SSE rounding, exception and write order, including unsafe alias
// effects; they do not snapshot input or provide singular/overlap fallbacks.

// B63B30[531]: ECX destination, EDX source, RET, EAX destination. REP MOVSD16
// writes the destination first, then later instructions read source again.
// The native DF state is retained (normal Win32 callers supply clear DF).
// Scaled orthogonal affine inverse; no general inverse or shear validation.
void* __fastcall invert_native_camera_scaled_affine_00b63b30(
    void* actual_destination, const void* actual_source);

// 413920[874]: ECX left, stack destination/right, RET8, EAX destination.
// The unused EDX parameter preserves that native stack order. Native caches
// and x87 lifetimes are retained; arbitrary partial overlap is not repaired.
void* __fastcall multiply_native_camera_matrices_00413920(
    const void* actual_left, void* unused_edx, void* actual_destination,
    const void* actual_right);

// B6D4D0[339]: ECX destination, EDX left, stack right, RET4. The original
// incidental EAX is right, not destination. Ordered stores force the affine
// last column to +0,+0,+0,+1; overlapping output may corrupt later input reads.
const void* __fastcall compose_native_camera_affine_00b6d4d0(
    void* actual_destination, const void* actual_left, const void* actual_right);

// Descriptive names remain hypotheses. Existing semantic camera APIs are
// unchanged. Raw routines do not establish a general camera hierarchy, object
// lifetime, original-caller integration, runtime fixture or game-validation claim.
} // namespace bsp
