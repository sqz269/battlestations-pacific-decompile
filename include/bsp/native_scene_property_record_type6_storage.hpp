#pragma once

#include <cstddef>
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native scene property record type-6 storage requires MSVC Win32.
#endif

namespace bsp {

inline constexpr std::size_t native_scene_property_record_type6_storage_bytes = 0x38;

// Whole [008EF780,008EF7BA): 58 bytes / 16 instructions. Names are hypotheses.
// Supply an actual fresh writable 38h allocation from the canonical current
// singleton_lifetime_allocate domain and the DISTINCT mutable 114h output of
// clone_empty_native_scene_property_bag_008f41f0_fragment. Child owner+110 must
// still be zero; both roots must be disjoint from each other and the call frame.
// This operation allocates/frees nothing. The caller retains responsibility for
// both actual allocations and their later lifetime; no C++ class is fabricated.
// ECX=record, incoming EDX unused, [entry ESP+4]=child. RET4; EAX=record,
// EDX=child, ECX=0. The unused second formal keeps the child on the STACK.
// EBX/ESI/EDI/EBP and DF are preserved; no x87/SSE/MXCSR operation occurs.
// Final XOR flags: CF=OF=SF=0, ZF=PF=1, AF undefined.
// Writes 41 record bytes: [00,10), [18,28), [2C,2D), [30,38).
// Preserves 15 record bytes: [10,18), [28,2C), [2D,30).
// Writes actual record to child+110 BEFORE record+34, byte+2C and final +8.
// Child is dereferenced unconditionally. Literal CE89D4 phase is DATA only;
// never dispatch it as a Source vtable or infer recursive destruction service.
void* __fastcall construct_native_scene_property_record_type6_storage_008ef780(
    void* actual_record_ecx, std::uint32_t unused_edx,
    void* actual_child_bag) noexcept;

// Ordinary C++ successful-empty-child fragment of [008F50ED,008F5134), 71 bytes.
// Borrow a stable actual record produced by the whole constructor above:
// tag+4=6, actual child+C, initialized ordinal+34. Its mutable child must remain
// in the qualified raw61-produced zero-count domain: +8=0 and all64 heads zero.
// DF=0; source root/child, both new roots and active frame are distinct; no
// concurrent mutation or reentry. Source roots and owner backlink stay borrowed.
// Allocate a real 38h root FIRST, load actual source+C, invoke the actual empty
// bag producer, attach its real returned child, then reread/copy source ordinal.
// The returned actual root and attached child use the SAME canonical malloc/free
// domain. The caller assumes both allocation lifetimes; this supplies neither a
// callable native profile nor the six-body recursive owning destruction family.
// Source error cleanup may free only its still-unattached record if child
// production throws. Original null allocation/SEH/state2 cleanup and whole-parent
// ECX/register/FS/stack ABI are not supplied by this ordinary, throwing interface.
void* clone_empty_child_native_scene_property_record_type6_008f50ed_fragment(
    const void* actual_source_record);

} // namespace bsp
