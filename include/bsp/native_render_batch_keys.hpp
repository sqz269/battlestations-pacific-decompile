#pragma once

#include <cstdint>

namespace bsp {
struct NativeRenderBatchStorage;

// Complete BF7456 hardware entry. Input is already in x87 ST0; consumes it
// and returns the native signed-integer bit pattern in EDX:EAX. Preserves the
// original x87 instructions, current control word, exception and status effects.
// This unusual ABI must be called by an x86 assembly caller that supplies ST0.
std::uint64_t __cdecl native_x87_truncate_st0_00bf7456() noexcept;

// Shared BF7420 dispatch over that same hardware entry. Input is already ST0;
// ECX supplies the actual mutable0109EEA4 address. Reads it at conversion time,
// selects the original FSTP double/CVTTSD2SI sequence or BF7456, consumes ST0
// and returns the low signed EAX word. Assembly caller only, not a float cast.
std::int32_t __fastcall native_crt_truncate_st0_00bf7420(
    const volatile std::uint32_t* actual_0109eea4) noexcept;

// Complete B51AB0: ECX/EDX are actual entry addresses, not semantic projections.
// Only AL is the predicate result. Unequal material values retain the right
// value's upper 24 EAX bits. Equal values compare depth with native x87 FCOMIP.
std::uint32_t __fastcall native_render_entry_material_depth_less_00b51ab0(
    const void* actual_left_entry, const void* actual_right_entry) noexcept;

// B51DF0 index-zero key loop [B51E2E,B51ED4), after enabled/index checks.
// Reloads the actual array and signed count; writes raw entry+20/+24 in order.
// Does not acquire the queue or perform the following pointer-slot sort.
// Required live spans: batch18h, each entry28h, section24h, material80h,
// effectC1h and conditional nonnull texture24h. These are minimum touched
// extents, not recovered complete class definitions. No added null guards.
void prepare_native_render_batch_keys_00b51df0(NativeRenderBatchStorage&) noexcept;
} // namespace bsp
