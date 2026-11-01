#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw Mesh draw-section clear storage requires MSVC Win32 assembly.
#endif

#include "bsp/native_mesh_draw_section_append_storage.hpp"

#include <cstdint>

namespace bsp {

struct NativeMeshDrawSectionClearStorageContext {
    const NativeApplicationPointerVectorAllocation& allocation;
    NativeTextureSurfaceReferenceIncrement const volatile& decrement_00ce2220;
};

// Complete ordinary B73840..B7388F for arbitrary signed desired count. Native
// ECX is the actual 0Ch header, stack desired is signed, RET4; this is a new
// C++ interface. Reserve, when signed desired>capacity, delegates to the real
// published raw B72E10 with the SAME borrowed allocation binding. Capture
// CURRENT count ONCE as growth index after reserve; each growth step reloads
// CURRENT data, calculates low32(data+index*4), skips only a zero calculated
// destination, stores zero otherwise, and wraps index against saved desired.
// Then compare CURRENT count for shrink, ADD its actual word by-1 and reload
// it each iteration. Always finish by storing the original desired DWORD.
// No pointed-object retain/release or spare-cell/backing cleanup is added.
void resize_native_mesh_section_pointers_storage_00b73840(
    void* actual_header,
    std::int32_t desired,
    const NativeApplicationPointerVectorAllocation& allocation);

// Complete ordinary B73C10..B73C51. Native ECX is Mesh, RET; overall EAX has
// no established semantic result. This new C++ interface exposes no result.
// Signed count>0 alone admits the one CURRENT decrement-cell capture. Each
// iteration reloads CURRENT data, captures B, and calls that SAVED stdcall
// target on low32(B+4), including B==0. Only exact result0 reloads B's CURRENT
// vtable and CURRENT vtable[0], then invokes the actual no-argument thiscall
// provider with ECX=B and no stacked deleting flags; its EAX is ignored.
// Wrap index and compare CURRENT signed count after the callbacks. Every
// normal loop exit, including initially nonpositive count, executes FULL
// raw B73840(mesh+54,0). An escaping call does not reach that normal tail.
void clear_native_mesh_draw_sections_storage_00b73c10(
    void* actual_mesh,
    const NativeMeshDrawSectionClearStorageContext& context);

// Raw DWORD accesses form no typed header overlay. Addresses/arithmetic are
// low32; signed comparisons preserve the same DWORD bits. Storage, service
// objects, callable cell and reached virtual providers belong to the caller.
// Numeric read-only profile data alone establishes no callable/provider fact.
// No validators, quotas, disjointness, no-reentry, NoThrow, catches, rollback,
// alternate atomic, zero filter or ownership/credit authority is supplied.
// Accesses/calls may fault or escape with preceding effects retained. These
// interfaces do not reproduce original ABI/HOME/private stack, FH3/SEH,
// fault/concurrency or runtime identity. Current-B/provider/caller ownership
// and activation remain separate held obligations; Native/vendor credit0.

} // namespace bsp
