#pragma once

#include <cstddef>
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native effect acquisition tree providers require MSVC Win32.
#endif

namespace bsp {

// Actual manager tree header begins at manager+4: opaque allocator word0,
// sentinel pointer4 and unsigned count8. Nodes are18h bytes: left/parent/right,
// signed key0C, unowned definition pointer10, color14, nil15 and untouched
// padding16/17. These projections own no storage or definition references.
struct NativeIntPointerTree18Iterator {
    void* owner;
    void* node;
};
struct NativeIntPointerTree18Pair {
    std::int32_t key_00;
    void* value_04;
};
struct NativeIntPointerTree18InsertResult {
    NativeIntPointerTree18Iterator iterator_00;
    std::uint8_t inserted_08;
    std::uint8_t untouched_padding_09[3];
};
static_assert(sizeof(NativeIntPointerTree18Iterator) == 8);
static_assert(sizeof(NativeIntPointerTree18Pair) == 8);
static_assert(sizeof(NativeIntPointerTree18InsertResult) == 12);
static_assert(offsetof(NativeIntPointerTree18InsertResult, inserted_08) == 8);

// 0086B650[105]: original ECX tree, stack output/key pointer, EAX output,
// RET8. Signed lower-bound with a fresh key read for the reverse comparison.
// Publish owner/node, returning end (the current sentinel) when not equal.
NativeIntPointerTree18Iterator* find_native_int_pointer_tree18_0086b650(
    void* actual_tree, NativeIntPointerTree18Iterator* output,
    const std::int32_t* key);

// 00869990[137]: original ECX mutable iterator, RET. Null owner and invalid
// predecessor use the source CRT invalid-parameter service, which may return.
// End decrements to maximum; an ordinary node uses left/right-parent traversal.
void decrement_native_int_pointer_tree18_00869990(
    NativeIntPointerTree18Iterator& iterator);

// 0086AB70[63]: five original stack inputs, EAX node, RET14h. Allocate18h,
// then write links, pair, color and nil=0. Padding16/17 stays untouched and
// the mapped pointer is copied without retain/release.
void* allocate_native_int_pointer_tree18_node_0086ab70(
    void* left, void* parent, void* right,
    const NativeIntPointerTree18Pair* pair, std::uint8_t color);

// 0086EBE0[492]: original ECX tree, stack output/left-byte/parent/pair,
// EAX output, RET10h. Count>=1FFFFFFEh throws the source-layout length error;
// otherwise allocate, increment, link, rebalance and publish node then owner.
NativeIntPointerTree18Iterator* link_native_int_pointer_tree18_node_0086ebe0(
    void* actual_tree, NativeIntPointerTree18Iterator* output,
    std::uint8_t insert_left, void* parent,
    const NativeIntPointerTree18Pair* pair);

// 0086F930[185]: original ECX tree, stack output/pair, EAX output, RET8.
// Signed unique search; predecessor is checked through00869990 when needed.
// A duplicate preserves its current mapped pointer and publishes inserted=0.
NativeIntPointerTree18InsertResult* insert_native_int_pointer_tree18_unique_0086f930(
    void* actual_tree, NativeIntPointerTree18InsertResult* output,
    const NativeIntPointerTree18Pair* pair);

// These are source interfaces over actual raw storage. They close the native
// manager cache's find/insert mechanics but do not implement008700E0's Lua
// miss path,00870400's component load, component virtual readers, native FH3
// identity or a binary replacement ABI.

} // namespace bsp
