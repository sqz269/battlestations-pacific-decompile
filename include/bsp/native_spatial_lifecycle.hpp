#pragma once
#include "bsp/native_spatial_attachment.hpp"
#include <cstdint>

namespace bsp {
// Stable access record over the same actual publication cells and borrowed pose
// views used by attachment. No shadow index, ownership domain or fallback.
struct NativeSpatialLifecycleAccess {
    const NativeSpatialAttachmentAccess* spatial;
    void* volatile* manager_publication_01090aa0;
    void* volatile* index_publication_00f8a0d8;
};

// Complete 42D4B0[41], native ECX index, stack flags, EAX original address,
// RET4. Source EDX adds the actual F8A0D8 cell. Clear publication unconditionally,
// write base profile CE3818, then free iff flag bit 0 was set. Does not traverse
// nodes, release their buffers or remove the index from its singleton manager.
void* __fastcall destroy_native_spatial_index_0042d4b0(void* actual_index,
    void* volatile* actual_index_publication_00f8a0d8, std::uint32_t flags) noexcept;

// Complete 98A2C0[77], ECX parent, stack child, RET4. Decrement count first;
// signed slot comparison; swap with last and update the moved child's slot.
// Clear child+108 only. Retain stale slots, child+15C, capacity and bounds.
void __fastcall detach_native_spatial_child_0098a2c0(void* actual_parent,
    void* unused_edx, void* actual_child);
// Complete 98A3D0[239], ECX index, EDX node, RET. Unlink using the current signed
// node+40 count, then repair matching heads across its captured packed rectangle.
// Clear count; retain key and inline link fields. No classification or shrinking.
void __fastcall unregister_native_spatial_cells_0098a3d0(void* actual_index,
    void* actual_node);
// Complete 98A4C0[52], ECX index, EDX node, RET. Search the captured positive
// signed loose count; swap first match with captured last, decrement live count.
void __fastcall remove_native_spatial_loose_node_0098a4c0(void* actual_index,
    void* actual_node);
// Complete 98A500[134], ECX index, stack node, RET4. Any zero +158 is inert;
// parent, loose or grid branch then clears +158. Root links retain native reloads.
void __fastcall detach_native_spatial_node_0098a500(void* actual_index,
    void* unused_edx, void* actual_node);
// Complete 710B80[34], ECX part, RET. Source EDX adds access. Nonzero +184 calls
// the canonical index getter and full detach body before clearing +184.
void __fastcall detach_native_unit_part_00710b80(void* actual_part,
    const NativeSpatialLifecycleAccess*);

// Complete 98BC70[260], ECX node, RET. EDX adds access. Copy current pose and
// inverse, rebuild world bounds, and re-register a grid node if its key changed.
// Preserve both separate index lookups. Capture children begin/end after those
// calls, then recurse over current elements in that captured storage range.
void __fastcall refresh_native_spatial_node_0098bc70(void* actual_node,
    const NativeSpatialLifecycleAccess*);
// Complete 98BDB0[40], ECX index, one UNUSED float stack word, RET4. EDX adds
// access. Visit grid roots only; skip a root and its subtree for any nonzero +8.
// Read each root's next pointer after refresh. Child recursion ignores child +8.
void __fastcall refresh_native_spatial_roots_0098bdb0(void* actual_index,
    const NativeSpatialLifecycleAccess*, float unused_step);

// Explicit source interfaces, not drop-in binary replacements. Valid raw storage,
// grid span capacity and acyclic child ownership remain native caller invariants.
// Native fault delivery, actual executable admission and gameplay are unproved.
} // namespace bsp
