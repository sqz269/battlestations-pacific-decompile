#pragma once

#include "bsp/native_input_keyboard_apply.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp {

// Actual pair stored at node+0Ch: an eight-byte pooled string header followed
// by the SoldierClass pointer cell returned by 004B0CB0.
struct NativeSoldierRegistryPairStorage {
    std::uint32_t key_length_00;
    char* key_data_04;
    void* mapped_08;
};
static_assert(sizeof(NativeSoldierRegistryPairStorage) == 0x0c);
static_assert(offsetof(NativeSoldierRegistryPairStorage, mapped_08) == 8);

struct NativeSoldierRegistryInsertResult {
    NativeKeyboardTreeIterator iterator_00;
    std::uint8_t inserted_08;
    std::uint8_t untouched_padding_09[3];
};
static_assert(sizeof(NativeSoldierRegistryInsertResult) == 0x0c);
static_assert(offsetof(NativeSoldierRegistryInsertResult, inserted_08) == 8);

// Full 004AFE50. Original ECX tree, key-header stack, EAX selected node,
// RET4. Tree+4 is head; head+4 is root. Node links are+0/+4/+8, key+C/+10,
// mapped pointer+14, color+18 and nil+19. Comparison is case-insensitive.
void* lower_bound_native_soldier_registry_004afe50(
    void* actual_tree, const void* actual_key_header);

// Full 004AF530. Original ECX left iterator, right iterator stack, AL result,
// RET4. A null/mismatched captured left owner invokes the returning invalid
// boundary; node words are read after it returns.
bool equal_native_soldier_registry_iterators_004af530(
    const NativeKeyboardTreeIterator& left,
    const NativeKeyboardTreeIterator& right,
    const SingletonLifetimeCallbacks& invalid_parameters);

// Full 004AFB50/004AFBE0. Original ECX iterator, RET or invalid tail. Both
// reload the current node after initial validation. Increment rejects end;
// decrement(end) first stores head->right, then validates that stored node.
void increment_native_soldier_registry_iterator_004afb50(
    NativeKeyboardTreeIterator& iterator,
    const SingletonLifetimeCallbacks& invalid_parameters);
void decrement_native_soldier_registry_iterator_004afbe0(
    NativeKeyboardTreeIterator& iterator,
    const SingletonLifetimeCallbacks& invalid_parameters);

// Full 004AFAA0/004AFAF0. Original ECX tree, node stack, RET4. These update
// only links/root publication and preserve colors, nil bytes and payload.
void rotate_native_soldier_registry_left_004afaa0(
    void* actual_tree, void* node) noexcept;
void rotate_native_soldier_registry_right_004afaf0(
    void* actual_tree, void* node) noexcept;

// Full 004B0090, reached by 004B0CB0 state0 unwind. Destroy only the current
// string header at pair+0 through the real raw pool; mapped+8 is untouched.
void destroy_native_soldier_registry_pair_004b0090(
    void* actual_pair, NativeStringRawPoolContext& strings);

// Full 004B01E0. Original ECX node, stack left/parent/right/pair/color,
// EAX node, RET14h. Write links, clear destination key, copy through the real
// raw pool with post-callback source reloads, then copy current mapped pointer,
// color and nil=0. Padding+1A/+1B stays untouched.
void* initialize_native_soldier_registry_node_004b01e0(
    void* node, void* left, void* parent, void* right,
    const NativeSoldierRegistryPairStorage* pair, std::uint8_t color,
    NativeStringRawPoolContext& strings);

// Full 004B02A0 including placement cleanup. Original five stack arguments,
// EAX node, RET14h. Allocate1Ch; initializer failure frees the raw node and
// rethrows. The new interface supplies the raw string-pool publications.
void* allocate_native_soldier_registry_node_004b02a0(
    void* left, void* parent, void* right,
    const NativeSoldierRegistryPairStorage* pair, std::uint8_t color,
    NativeStringRawPoolContext& strings);

// Full 004B0660. Original ECX tree, stack output/left/parent/pair, EAX output,
// RET10h. Unsigned count>=15555554h throws the established owning native-layout
// length-error transport. Allocation precedes current count/head reloads; after
// native red-black repair it publishes output node before owner.
NativeKeyboardTreeIterator* link_native_soldier_registry_node_004b0660(
    void* actual_tree, NativeKeyboardTreeIterator* output,
    std::uint8_t insert_left, void* parent,
    const NativeSoldierRegistryPairStorage* pair,
    NativeStringRawPoolContext& strings);

// Full 004B0880. Original ECX tree, stack output/pair, EAX output, RET8.
// Search/final comparisons are case-insensitive. Only owner, node and inserted
// byte are written; result padding and a duplicate pair remain untouched.
NativeSoldierRegistryInsertResult* insert_native_soldier_registry_pair_004b0880(
    void* actual_tree, NativeSoldierRegistryInsertResult* output,
    const NativeSoldierRegistryPairStorage* pair,
    NativeStringRawPoolContext& strings,
    const SingletonLifetimeCallbacks& invalid_parameters);

// Full 004B0A70. Original ECX tree, stack output/two-word hint/pair, EAX output,
// RET10h. The first checked-hint comparison uses minimum captured before its
// returning invalid callback; the end comparison similarly uses a pre-callback
// head. Fallback copies only owner/node from the unique insertion result.
NativeKeyboardTreeIterator* insert_hint_native_soldier_registry_pair_004b0a70(
    void* actual_tree, NativeKeyboardTreeIterator* output,
    NativeKeyboardTreeIterator hint,
    const NativeSoldierRegistryPairStorage* pair,
    NativeStringRawPoolContext& strings,
    const SingletonLifetimeCallbacks& invalid_parameters);

// Full 004B0CB0. Original ECX tree, key-header stack, EAX mapped-cell pointer,
// RET4. A miss copies the key into a zero-mapped temporary, inserts with the
// lower-bound hint and returns node+14h. Normal cleanup returns the data pointer
// captured before insertion with the current length; exceptional state0 cleanup
// destroys the current header. Final validation reloads head after callback.
void** get_or_insert_native_soldier_registry_mapped_004b0cb0(
    void* actual_tree, const void* actual_key_header,
    NativeStringRawPoolContext& strings,
    const SingletonLifetimeCallbacks& invalid_parameters);

// Descriptive names are hypotheses. These strict MSVC Win32 source interfaces
// borrow actual tree/string storage; they do not provide original ABI, FH3/SEH,
// concurrent-mutation guarantees, a binary replacement or gameplay proof.
} // namespace bsp
