#pragma once

namespace bsp {

// Complete A8AAA0 (351 bytes): original ECX actual shadow owner, public stack
// words output then viewport, EAX output, RET8. The unused EDX parameter keeps
// that machine shape. Owner dimensions are current raw DWORDs at +388/+38C;
// viewport origin/dimensions are signed words from existing B1F730/B1F740.
// Preserve original x87/SSE order, 32/64-bit spills, mutable public argument
// scratch slot, intermediate precision and all 16 output-store timings.
// The original read-only image constants are rebound to identical const bits.
void* __fastcall build_native_shadow_viewport_matrix_00a8aaa0(
    const void* actual_shadow_owner, void* unused_source_edx,
    void* actual_output_matrix, const void* actual_viewport) noexcept;

// Complete four-byte raw getters. Original ECX actual receiver, EAX current
// pointer at +14h or +1Ch, RET. No retain, validation or constructor is added.
void* __fastcall native_shadow_target_field14_00a8fda0(const void*) noexcept;
void* __fastcall native_shadow_target_field1c_00a8fdc0(const void*) noexcept;

// Caller must provide actual readable/writable storage and appropriate x87/SSE
// state. No dimensions, alignment, aliasing, range or floating-point policy is
// invented. Native hardware faults, unmasked exceptions and game integration
// require separate evidence. Descriptive names are hypotheses.
} // namespace bsp
