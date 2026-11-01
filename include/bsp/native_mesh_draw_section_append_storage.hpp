#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw Mesh draw-section append storage requires MSVC Win32 assembly.
#endif

#include "bsp/native_shader_preload.hpp"
#include "bsp/native_texture_surface_getter.hpp"

#include <cstdint>

namespace bsp {

// A non-owning view of this append call's own outgoing Section DWORD, initially
// filled from the caller's captured result. It is not a Native permit or a view
// of a mutable Section HOME / Parent Acquired field. Keep the cell alive through
// callbacks and escapes. The body reads it after reserve, then overwrites it
// with the captured Section+04 address before loading the current import.
struct NativeMeshDrawSectionAppendArgumentWordView {
    volatile std::uint32_t& word_04;
};

struct NativeMeshDrawSectionAppendStorageContext {
    const NativeApplicationPointerVectorAllocation& allocation;
    NativeTextureSurfaceReferenceIncrement const volatile& increment_00ce221c;
};

// Complete ordinary B72E10..B72E6E storage schedule. Native ECX is the actual
// 0Ch header, stack request is signed, RET4; this is a new C++ interface.
// Clamp to min4, compare signed capacity, allocate low32(request*4), copy with
// live data/count reloads and a per-iteration zero-cursor skip, free CURRENT
// data (including null), then publish captured replacement/capacity in order.
// No count store, typed header overlay, allocator default or ownership change.
void reserve_native_mesh_section_pointers_storage_00b72e10(
    void* actual_header,
    std::int32_t requested,
    const NativeApplicationPointerVectorAllocation& allocation);

// Complete ordinary B73C60..B73CA3 storage/import schedule. Native ECX is mesh;
// the Section DWORD is stacked and the tail import supplies RET4/EAX. This new
// C++ interface returns the current stdcall import's long result; it does not
// reproduce the original calling convention or stack/register identity.
// Grow only on count==capacity, using signed max(low32(capacity*2),4). Reload
// actual data/count, read the outgoing word even for a zero calculated slot,
// conditionally store, ADD the CURRENT count, overwrite the word with captured
// Section+04, then load/call CURRENT increment_00ce221c. Null Section is not
// filtered; the import receives low32(Section+04).
long append_native_mesh_draw_section_storage_00b73c60(
    void* actual_mesh,
    NativeMeshDrawSectionAppendArgumentWordView argument,
    const NativeMeshDrawSectionAppendStorageContext& context);

// All addresses/arithmetic are native low32 values; signed comparisons use the
// same DWORD bits. Existing storage and actual service/cell lifetimes belong to
// the caller. No validation, quota, disjointness, count1, no-reentry, NoThrow,
// catch, rollback, fallback atomic or canonical/Origin contract is added.
// Accesses/services may fault or escape with preceding effects retained.
// Provider/caller HOME/creator-debt wiring remains UNBOUND; Original ABI, FH3,
// SEH, fault/concurrency and runtime equivalence remain separate obligations.

} // namespace bsp
