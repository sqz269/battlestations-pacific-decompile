#pragma once
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual group-vector erase and resize require MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Complete006A7630. Native ECX group header, EDX unused; five stacked words
// (output iterator, first owner, first position, last owner, last position),
// RET14h/EAX output address. Source reserves EDX and adds actual pool context.
// Only first-owner null/owner equality is checked. Equal positions skip all
// receiver accesses. Otherwise assign forward, destroy the suffix up to
// CURRENT receiver end, publish the returned end, then store iterator owner
// and position in order. No new owner-receiver/range/null/self guard.
void* __fastcall erase_native_group_vector_range_006a7630(void* header,
    std::uint32_t unused_edx, void* output_iterator, const void* first_owner,
    void* first_position, const void* last_owner, const void* last_position,
    NativeStringRawPoolContext& strings);

// Complete006A79A0. Native ECX group header, EDX unused; stacked requested
// count followed by an OWNED by-value16B outer header, RET14h, no semantic
// result. Source transfers the four scalar fill words into one private header
// without an extra constructor/copy. All admitted exits consume that owner.
// Growth uses full actual6A7690; shrink uses full actual6A7630. Preserve all
// captured endpoints and returning-diagnostic reads/comparisons.
void __fastcall resize_native_group_vector_006a79a0(void* header,
    std::uint32_t unused_edx, std::uint32_t requested_count,
    std::uint32_t owned_fill_opaque, std::uint32_t owned_fill_begin,
    std::uint32_t owned_fill_end, std::uint32_t owned_fill_capacity,
    NativeStringRawPoolContext& strings);

// Complete00C7F5E0: native parent-bound LEA ECX,[EBP+8]; JMP0055B110.
// Source receives the canonical owned header explicitly, not native EBP.
// Unwind uses CURRENT fields and ordered clears; normal cleanup separately
// frees the cached begin after disabling ownership and does not clear fields.
void __fastcall unwind_native_group_resize_parameter_00c7f5e0(
    void* owned_fill, NativeStringRawPoolContext& strings);

// Actual group16B headers own16B outer headers, owning16B inner headers with
//8B strings. Admitted taken storage/ownership/pool/CRT and reachable endpoints
// apply; private native/Source frame aliases are excluded. Current SDK policy
// may return from diagnostics. Added actual context/current CRT/C++ interfaces
// do not prove native register/stack/FH3/SEH/RTTI/fault/cleanup-failure identity
// or gameplay. No allocator substitute, callback, catch retry or rollback.
} // namespace bsp
