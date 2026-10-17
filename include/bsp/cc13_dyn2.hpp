#pragma once
// cc13_dyn2: Dyn::TerrainConvexIntersect slot 0 (00C58EB0) and the Mit::cPoolAllocator<36,128>
// free (00407960) with its JMP thunk (00C36990). Reconstructed, build-tested only; nothing is
// wired. Descriptive names are hypotheses, not recovered symbols. docs/CC13_DYN2.md.

#include <cstddef>
#include <cstdint>

#include <windows.h>

#include "bsp/cc13_dyn1.hpp"
#include "bsp/dyn_dispatch_initialization.hpp"

namespace bsp::cc13_dyn2 {

// ---- Mit::cPoolAllocator<36,128>, the global at 0109EA0C (constructor 00407590) ----
// Slots are 28h bytes (a 24h-byte payload, then the owning page's index at slot+24h), 128 per
// 1504h-byte page; each page keeps a stack of 80h free slot indices (uint16) at page+1400h and
// its depth (uint16) at page+1500h.
struct MitPoolAllocator36x128 {
    std::uint32_t vtable_00;            // 0109EA0C: 00D7A1DC
    std::uint32_t field_04;             // 0109EA10: 0
    std::uint32_t registry_next_08;     // 0109EA14: previous [00E188B4]
    CRITICAL_SECTION lock_0c;           // 0109EA18
    std::uint32_t lock_depth_24;        // 0109EA30: raised after Enter, lowered before Leave
    std::uint8_t** pages_28;            // 0109EA34
    std::uint32_t page_count_2c;        // 0109EA38
    std::uint32_t page_capacity_30;     // 0109EA3C: 20h
    std::uint32_t first_free_page_34;   // 0109EA40: -1 at construction
};
static_assert(offsetof(MitPoolAllocator36x128, lock_0c) == 0x0c);
static_assert(offsetof(MitPoolAllocator36x128, lock_depth_24) == 0x24);
static_assert(offsetof(MitPoolAllocator36x128, pages_28) == 0x28);
static_assert(offsetof(MitPoolAllocator36x128, first_free_page_34) == 0x34);

// 00407960, register ABI: EAX = slot, no stack arguments, plain RET; ESI/EDI preserved, EAX,
// ECX, EDX clobbered. The image addresses the global pool 0109EA0C by absolute address; the
// host passes its instance. EnterCriticalSection/LeaveCriticalSection are the IAT calls
// [00CE2218]/[00CE2210].
void pool36_free_00407960(MitPoolAllocator36x128& pool, void* slot);
// 00C36990: `JMP 00407960`, the same register ABI (its seven callers are unwind funclets
// 00CC838D..00CC85DD that load EAX from their frame).
void pool36_free_thunk_00c36990(MitPoolAllocator36x128& pool, void* slot);

// ---- Dyn::TerrainConvexIntersect (vtable 00D7A1C4, object 00E1743C) ----
// The image's Dyn::TriangleShape vtable stored into the stack triangle.
inline constexpr std::uint32_t kDynTriangleShapeVtable_00d7a1b0 = 0x00d7a1b0u;

// 00C58EB0 slot 0, __thiscall(ECX = owner, unused), stack (result, shape_a, matrix_a,
// shape_b, matrix_b), RET 14h; AL is the hit flag (count != 0), upper EAX bits are not.
// One shape is a TerrainShape (kind [S+8] == 5, constructor 00C58840); if shape_a is not
// kind 5 the terrain is shape_b and the contacts are swapped/negated at the end.
// result: int count, then up to eight 9-float contacts (point on A, point on B, normal).
// general is the GeneralConvexIntersect object (image 0109EA48): its +8 directions and +278h
// lock go into the work record, as the image's literal 0109EA50/0109ECC0 do. The two vtable
// words are those the image stores in the stack triangle (Dyn::Shape base, TriangleShape);
// 00C53010 calls the triangle's support slot through the second, so the host passes a table
// that is callable with the native ABI.
// The image faults past the empty-range test: its inlined Dyn::Shape base constructor reads
// a null descriptor (00C59EDA MOV EAX,[10h]). This source performs the same reads.
bool terrain_convex_intersect_00c58eb0(
    void* result, void* shape_a, const float* matrix_a, void* shape_b, const float* matrix_b,
    DynGeneralConvexIntersectStorage& general, const CameraAxesCrtAccess& crt,
    std::uint32_t shape_base_vtable = cc13_dyn1::kDynShapeVtable_00d7a04c,
    std::uint32_t triangle_vtable = kDynTriangleShapeVtable_00d7a1b0);

}  // namespace bsp::cc13_dyn2
