#pragma once

#include "bsp/native_damageable_class_binding.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp {

struct NativeDamageableCrewMappedStorage {
    std::uint8_t bytes_00[0x44];
};
static_assert(sizeof(NativeDamageableCrewMappedStorage) == 0x44);

struct NativeDamageableCrewTreePairStorage {
    std::int32_t key_00;
    NativeDamageableCrewMappedStorage mapped_04;
};
static_assert(sizeof(NativeDamageableCrewTreePairStorage) == 0x48);
static_assert(offsetof(NativeDamageableCrewTreePairStorage, mapped_04) == 4);

struct NativeDamageableCrewTreeInsertResult {
    NativeKeyboardTreeIterator iterator_00;
    std::uint8_t inserted_08;
    std::uint8_t untouched_padding_09[3];
};
static_assert(sizeof(NativeDamageableCrewTreeInsertResult) == 12);
static_assert(offsetof(NativeDamageableCrewTreeInsertResult, inserted_08) == 8);

// Complete 00876340. Original ECX left iterator, stack right iterator, EAX bool,
// RET4. A null/mismatched owner invokes the returning invalid-parameter boundary
// before the node comparison. The new source interface borrows that boundary.
bool equal_native_damageable_crew_iterators_00876340(
    const NativeKeyboardTreeIterator&, const NativeKeyboardTreeIterator&,
    const SingletonLifetimeCallbacks&);

// Complete 00876630/008773A0. Original ECX tree, stack node, RET4. These mutate
// only links/root publication and preserve colors, nil bytes and payloads.
void rotate_native_damageable_crew_right_00876630(void* actual_tree, void* node) noexcept;
void rotate_native_damageable_crew_left_008773a0(void* actual_tree, void* node) noexcept;

// Complete 008767A0. Original ECX iterator, RET. A returning invalid-parameter
// callback continues from the iterator's then-current fields as native code does.
void decrement_native_damageable_crew_iterator_008767a0(
    NativeKeyboardTreeIterator&, const SingletonLifetimeCallbacks&);

// Complete 00877FE0. Original five stack arguments, EAX allocated node, RET14h.
// Allocate 58h, write left/right/parent, copy all 48h pair bytes in DWORD order,
// then write color+54h and nil+55h. No payload byte is cleared or defaulted.
void* allocate_native_damageable_crew_tree_node_00877fe0(
    void* left, void* parent, void* right,
    const NativeDamageableCrewTreePairStorage*, std::uint8_t color);

// Complete 0087A5F0. Original ECX tree, stack output/left/parent/pair, RET10h.
// The unsigned count limit is 038E38E2h. Allocation precedes count/link changes;
// successful insertion updates extrema, performs native red-black repair, makes
// the root black, and publishes output node before owner.
NativeKeyboardTreeIterator* link_native_damageable_crew_tree_node_0087a5f0(
    void* actual_tree, NativeKeyboardTreeIterator* output,
    std::uint8_t insert_left, void* parent,
    const NativeDamageableCrewTreePairStorage*);

// Complete 0087ADB0. Original ECX tree, stack output/pair, EAX output, RET8.
// Comparisons are signed int32. Only owner, node and inserted byte are written;
// result padding and a duplicate pair remain untouched.
NativeDamageableCrewTreeInsertResult* insert_native_damageable_crew_tree_pair_0087adb0(
    void* actual_tree, NativeDamageableCrewTreeInsertResult* output,
    const NativeDamageableCrewTreePairStorage*, const SingletonLifetimeCallbacks&);

// Complete 0087B260. Original ECX tree, stack output/two-word hint/pair, EAX
// output, RET10h. Checked hint validation can return, and fallback copies only
// the unique insertion result's owner and node into the output iterator.
NativeKeyboardTreeIterator* insert_hint_native_damageable_crew_tree_pair_0087b260(
    void* actual_tree, NativeKeyboardTreeIterator* output,
    NativeKeyboardTreeIterator hint, const NativeDamageableCrewTreePairStorage*,
    const SingletonLifetimeCallbacks&);

// Complete 0087B750. Original ECX tree, stack int32 key pointer, EAX mapped
// value pointer, RET4. A missing key inserts a 48h pair whose 44h mapped part is
// copied from uninitialized stack storage exactly as the native routine does.
// The returned pointer is node+10h; the reader writes SoldierClass at mapped+40h.
NativeDamageableCrewMappedStorage* get_or_insert_native_damageable_crew_mapped_0087b750(
    void* actual_tree, const std::int32_t* key,
    const SingletonLifetimeCallbacks&);

// These are strict MSVC Win32 source interfaces over borrowed native-shaped
// storage. They do not install original FH3 metadata or provide a binary ABI.
} // namespace bsp
