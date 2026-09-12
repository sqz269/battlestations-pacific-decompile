#pragma once

#include <cstdint>

namespace bsp {
class NativeStringStorage;

// Actual14h record: signed priority+0, string length/data+4/+8, provider+C,
// flag byte+10; padding+11..13 untouched. No provider retain or ownership.
// Complete BDEE60..BDEEBB: ECX destination, stack source, RET4, EAX dest.
// Captures priority, zeroes destination string even for self-alias, copies
// distinct strings, then reloads current source provider and flag independently.
void* copy_native_vfs_mount_record_00bdee60(void* destination,
    const void* source, NativeStringStorage&);

// Complete BE05C0..BE063D plus catch BE063E..BE0652. No consumed ECX;
// stack left/parent/right/record/color, RET14, EAX actual24h node. Raw shared
// CRT allocation, link writes, BDEE60 at node+C, then color20/nil21=0.
// Catch frees captured node and rethrows, without destroying its payload.
void* allocate_native_vfs_mount_node_00be05c0(void* left, void* parent,
    void* right, const void* record, std::uint8_t color, NativeStringStorage&);

// Complete BE0DD0..BE0FBB: ECX tree, stack output/left-byte/parent/record,
// RET10, EAX output. Count>=0CCCCCCBh throws the existing owning source
// NativeHardwareLayoutTreeLengthError. On success increment CURRENT count,
// link/rebalance, blacken current root, then publish node+4 BEFORE owner+0.
void* link_native_vfs_mount_node_00be0dd0(void* tree, void* output,
    std::uint8_t insert_left, void* parent, const void* record,
    NativeStringStorage&);

// Complete BE1330..BE139A: ECX tree, stack output/record, RET8, EAX output.
// Search compares signed priorities descending; equal keys descend right.
// New priority is captured once only when the initial root is not nil.
// Output is 12 actual bytes: owner+0, node+4, inserted byte+8=1 in that order;
// trailing padding untouched. Equal priorities always insert, never replace.
void* insert_native_vfs_mount_record_00be1330(void* tree, void* output,
    const void* record, NativeStringStorage&);

// Actual tree head/count+4/+8, node links0/4/8, payload+C, color20/nil21.
// Names are hypotheses. Explicit-service source interfaces are not original
// ABI replacements. Source CRT/EH, noexcept string release, arbitrary stack
// aliasing, concurrent mutation, original FH3/SEH and gameplay remain limits.
} // namespace bsp
