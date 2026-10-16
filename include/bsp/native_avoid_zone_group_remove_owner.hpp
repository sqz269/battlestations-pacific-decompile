#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw avoidance group owner removal requires MSVC Win32.
#endif

namespace bsp {

// Complete 0041D150..0041D1D8 (137B). Original ECX raw14h group plus one
// stacked owner word, RET4, no semantic EAX result. Source EDX is unused and
// not an Original input. Capture the owner word once; null calls the actual
// complete raw group-clear entry, retaining the group array/capacity/layer.
//
// Nonnull owner reads zone+10 BEFORE testing zone null. Each match destroys
// actual records/array/zone, nulls its slot, reloads current count/array, moves
// the current last slot over it, decrements CURRENT count, and rechecks that
// SAME slot. Nonmatches advance four bytes. Inactive slots are not normalized.
// Every reached raw address and matching heap allocation must be valid. No
// defensive null guard, stable compaction, callback or rollback is added.
//
// Fixed real current-heap/cleanup bindings replace Native call operands. The
// literal entry/stack schedule is not Native CRT/exception/frame/object-class,
// binary deployment or gameplay proof. Existing typed APIs remain distinct.
// Evidence: docs/GOAL15K_AVOID_ZONE_GROUP_REMOVE_OWNER.md.
void __fastcall remove_native_avoid_zone_group_owner_0041d150(
    void* actual_group, void* unused_edx, void* owner);

} // namespace bsp
