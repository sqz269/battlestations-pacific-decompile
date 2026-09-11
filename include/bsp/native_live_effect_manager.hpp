#pragma once

#include "bsp/native_render_pointer_arrays.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {

struct NativeEffectDeletionNode {
    NativeEffectDeletionNode* next_00;
    NativeEffectDeletionNode* previous_04;
    void* payload_08;
};
struct NativeEffectDeletionListStorage {
    std::uint32_t untouched_allocator_00;
    NativeEffectDeletionNode* head_04;
    std::uint32_t count_08;
};
// Actual 28h owner published at F8765C. This is distinct from the 8h F87650
// insertion-lock owner and the 10h F87664 effect-definition owner. No implicit
// initialization/destruction, host vtable, replacement list, or shadow counts.
struct NativeLiveEffectManagerStorage {
    std::uint32_t native_vtable_00;
    NativeEffectDeletionListStorage pending_04;
    NativeRenderPointerArrayStorage effects_10;
    NativeRenderPointerArrayStorage references_1c;
};
static_assert(sizeof(NativeEffectDeletionNode) == 0x0c);
static_assert(offsetof(NativeEffectDeletionNode, previous_04) == 4);
static_assert(offsetof(NativeEffectDeletionNode, payload_08) == 8);
static_assert(sizeof(NativeEffectDeletionListStorage) == 0x0c);
static_assert(offsetof(NativeEffectDeletionListStorage, head_04) == 4);
static_assert(offsetof(NativeEffectDeletionListStorage, count_08) == 8);
static_assert(sizeof(NativeLiveEffectManagerStorage) == 0x28);
static_assert(offsetof(NativeLiveEffectManagerStorage, pending_04) == 4);
static_assert(offsetof(NativeLiveEffectManagerStorage, effects_10) == 0x10);
static_assert(offsetof(NativeLiveEffectManagerStorage, references_1c) == 0x1c);

// Complete 4C3200: no consumed inputs, EAX sentinel, RET. Allocate 0Ch and
// publish its self links; preserve payload+08. Native ECX is not consumed.
NativeEffectDeletionNode* create_native_effect_deletion_sentinel_004c3200();

// Complete 4C5940: ECX actual 0Ch list header, RET. Capture first node, detach
// the ring and publish count zero, then free nodes in next-link order, free
// current sentinel and clear current head. Payloads are never destroyed.
// Caller supplies an intact finite ring; no implicit effect deletion/locking.
void destroy_native_effect_deletion_list_004c5940(
    NativeEffectDeletionListStorage&) noexcept;

// Complete 4B7F50: ECX actual manager, RET. Unconditionally clear F8765C,
// then publish CE3818 base identity, preserving every other owner byte.
void unwind_native_live_effect_manager_base_004b7f50(
    NativeLiveEffectManagerStorage&,
    NativeLiveEffectManagerStorage* volatile& actual_global_00f8765c) noexcept;

// Complete 4CF700: ECX actual 28h raw storage, EAX same owner, RET. Publish
// CE789C, allocate/publish sentinel, zero list count and both arrays in order.
// Preserve allocator word+04 and sentinel payload. On allocation failure only
// base unwind4B7F50 runs (state0 C65C60); there is no constructed-list cleanup.
NativeLiveEffectManagerStorage& construct_native_live_effect_manager_004cf700(
    NativeLiveEffectManagerStorage&,
    NativeLiveEffectManagerStorage* volatile& actual_global_00f8765c);

// These are new C++ interfaces. Native addresses are evidence identities only.
// Full manager destruction, lazy registration and pending-object dispatch are
// separate operations; none are replaced by an implicit host destructor here.
} // namespace bsp
