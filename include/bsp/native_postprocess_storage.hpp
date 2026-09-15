#pragma once
#include "bsp/native_render_pointer_arrays.hpp"
#include "bsp/native_material_pass_states.hpp"

namespace bsp {

// B79F22..B79F85 produces a zeroed 0Ch header and 10h records. The first
// word is a borrowed node; the nested header owns pointer-buffer storage,
// while its four-byte resource-item identities remain borrowed.
struct NativePostprocessNodeTracks {
    void* node_00;
    NativeRenderPointerArrayStorage items_04;
};
struct NativePostprocessNodeTracksArray {
    NativePostprocessNodeTracks* data_00;
    std::int32_t count_04,capacity_08;
};
static_assert(sizeof(NativePostprocessNodeTracks)==0x10);
static_assert(offsetof(NativePostprocessNodeTracks,items_04)==4);
static_assert(sizeof(NativePostprocessNodeTracksArray)==0xc);
static_assert(offsetof(NativePostprocessNodeTracksArray,count_04)==4);
static_assert(offsetof(NativePostprocessNodeTracksArray,capacity_08)==8);

// B76830/B76C40 are instruction-identical specializations after verified call
// relocation: use canonical reserve/resize_native_instance_entry_pointers at
// B1C500/B1C770 on the SAME header. B76890 similarly reuses the existing
// B40C80/B40AC0 eight-byte reserve algorithm and physical header. That existing
// reserve rejects corrupt extents/overflow; it does not reproduce OOB writes.
using NativePostprocessPairArray=NativeMaterialStateArray;

// Native ECX destination header, stack source header, RET4/EAX destination.
// Source keeps the stacked source via unused EDX. Resize destination0 first,
// then reserve source's current count. Each source cell address is captured
// BEFORE possible destination growth; source count/backing reload each turn.
// Self-copy therefore empties the array. Neither side's items are retained.
NativeRenderPointerArrayStorage* __fastcall copy_native_postprocess_item_pointers_00b77790(
    NativeRenderPointerArrayStorage&,void*,const NativeRenderPointerArrayStorage&);

// ECX actual outer header, stack signed request, RET4. Reserve clamps to1,
// deep-copies each current record's nested pointer buffer, destroys old nested
// buffers ascending, frees the current outer backing, then publishes pointer
// and capacity. Resize leaves new node words UNWRITTEN, clears only nested
// headers, and destroys removed records descending with current outer reads.
// Borrowed nodes/items are never released; freed pointer/capacity words remain.
// All accessed storage/extents must be valid. No replacement vector or rollback.
// The native reserve's sole unwind action is placement delete 401130 (RET),
// so failure does NOT clean prior copies or the new outer block. Native FH3
// integration remains unproved; source exceptions preserve those partial states.
void __fastcall reserve_native_postprocess_node_tracks_00b78a90(
    NativePostprocessNodeTracksArray&,void*,std::int32_t);
void __fastcall resize_native_postprocess_node_tracks_00b78d70(
    NativePostprocessNodeTracksArray&,void*,std::int32_t);

// Actual track-item header: backing+8/current signed count+C. B8A2D0 uses
// ECX/RET/EAX count. B8A370 uses ECX/stack raw index/RET4/EAX pointer; it has
// no bounds handler and uses DWORD index scaling. Valid addressed cell required.
std::int32_t __fastcall native_track_item_count_00b8a2d0(const void*) noexcept;
void* __fastcall native_track_item_at_00b8a370(const void*,void*,std::uint32_t) noexcept;

} // namespace bsp
