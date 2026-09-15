#pragma once
#include "bsp/native_soldier_registry_tree.hpp"

namespace bsp {
// Actual pair is0Ch, node is1Ch. Key header0/4, float8; node key+C,
// float+14, color18,nil19. Borrow actual pool and returning invalid boundary.
struct NativeSoldierLoopLengthPairStorage {
    std::uint32_t key_length_00;
    char* key_data_04;
    float value_08;
};
static_assert(sizeof(NativeSoldierLoopLengthPairStorage) == 0x0c);
static_assert(offsetof(NativeSoldierLoopLengthPairStorage, value_08) == 8);
using NativeSoldierLoopLengthInsertResult = NativeSoldierRegistryInsertResult;

// Complete native specializations, with instruction-level equivalence evidence
// for shared registry mechanics. The initializer keeps ordered FLD32/FSTP32;
// subscript defaults the mapped float to+0 after key construction. Key copies
// use actual raw pool and overlap-safe native BF7680 semantics. Normal temporary
// release uses captured data/current length; true unwind uses current header.
// No std::map or NativeStringStorage substitution. New MSVC Win32 interfaces;
// original register/stack ABI, FH3/SEH and gameplay remain unvalidated.
void destroy_native_soldier_loop_pair_00443fb0(
    void* actual_pair, NativeStringRawPoolContext& strings);

void* initialize_native_soldier_loop_node_00444150(
    void* node, void* left_node, void* parent_node, void* right_node,
    const NativeSoldierLoopLengthPairStorage* pair, std::uint8_t node_color,
    NativeStringRawPoolContext& strings);

void* allocate_native_soldier_loop_node_004441e0(
    void* left_node, void* parent_node, void* right_node,
    const NativeSoldierLoopLengthPairStorage* pair, std::uint8_t node_color,
    NativeStringRawPoolContext& strings);

NativeKeyboardTreeIterator* link_native_soldier_loop_node_004442a0(
    void* actual_tree, NativeKeyboardTreeIterator* output,
    std::uint8_t insert_left, void* parent_node,
    const NativeSoldierLoopLengthPairStorage* pair,
    NativeStringRawPoolContext& strings);

NativeSoldierLoopLengthInsertResult* insert_native_soldier_loop_pair_004447c0(
    void* actual_tree, NativeSoldierLoopLengthInsertResult* output,
    const NativeSoldierLoopLengthPairStorage* pair,
    NativeStringRawPoolContext& strings,
    const SingletonLifetimeCallbacks& callbacks);

NativeKeyboardTreeIterator* insert_hint_native_soldier_loop_pair_00444910(
    void* actual_tree, NativeKeyboardTreeIterator* output,
    NativeKeyboardTreeIterator hint,
    const NativeSoldierLoopLengthPairStorage* pair,
    NativeStringRawPoolContext& strings,
    const SingletonLifetimeCallbacks& callbacks);

float* get_or_insert_native_soldier_loop_length_00444be0(
    void* actual_tree, const void* actual_key_header,
    NativeStringRawPoolContext& strings,
    const SingletonLifetimeCallbacks& callbacks);

void* lower_bound_native_soldier_loop_lengths_00443d60(void* tree, const void* key_header);

bool equal_native_soldier_loop_iterators_004435a0(const NativeKeyboardTreeIterator& left,
    const NativeKeyboardTreeIterator& right, const SingletonLifetimeCallbacks& calls);

void increment_native_soldier_loop_iterator_00443880(NativeKeyboardTreeIterator& iterator,
    const SingletonLifetimeCallbacks& calls);

void decrement_native_soldier_loop_iterator_004437f0(NativeKeyboardTreeIterator& iterator,
    const SingletonLifetimeCallbacks& calls);

void rotate_native_soldier_loop_left_00443ba0(void* tree, void* node) noexcept;

void rotate_native_soldier_loop_right_004436e0(void* tree, void* node) noexcept;

void* minimum_native_soldier_loop_node_004437c0(void* node) noexcept;

void* maximum_native_soldier_loop_node_004437a0(void* node) noexcept;

void destroy_native_soldier_loop_subtree_00444760(void* tree, void* node,
    NativeStringRawPoolContext& strings);

NativeKeyboardTreeIterator* erase_native_soldier_loop_iterator_00444490(void* tree,
    NativeKeyboardTreeIterator* output, NativeKeyboardTreeIterator input,
    NativeStringRawPoolContext& strings, const SingletonLifetimeCallbacks& calls);

NativeKeyboardTreeIterator* erase_native_soldier_loop_range_00444b10(void* tree,
    NativeKeyboardTreeIterator* output, NativeKeyboardTreeIterator first,
    NativeKeyboardTreeIterator last, NativeStringRawPoolContext& strings,
    const SingletonLifetimeCallbacks& calls);
} // namespace bsp
