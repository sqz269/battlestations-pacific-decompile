#pragma once

#include "bsp/native_hardware_layout_tree.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp {

// Native tree-node key at +0Ch. These words are stream-owner identities; this
// packet borrows them and does not retain or release the owners.
struct NativeHardwareLayoutTreeKey {
    std::uint32_t stream_words_00[4];
    std::int32_t count_10;
};
struct NativeHardwareLayoutTreePair {
    NativeHardwareLayoutTreeKey key_00;
    void* value_14;
};
static_assert(sizeof(NativeHardwareLayoutTreeKey) == 0x14);
static_assert(offsetof(NativeHardwareLayoutTreeKey, count_10) == 0x10);
static_assert(sizeof(NativeHardwareLayoutTreePair) == 0x18);
static_assert(offsetof(NativeHardwareLayoutTreePair, value_14) == 0x14);

// Native ECX left, stack right, RET4, AL boolean. Compare signed counts in
// descending order, then unsigned stream DWORDs in ascending order. Both
// counts are captured once, right first. Equal nonpositive counts compare equal.
bool less_native_hardware_layout_key_00b20bf0(
    const void* left_key, const void* right_key) noexcept;

// Native ECX tree, stack key, RET4, EAX node. Borrow the actual tree header
// (+4 head) and nodes (+0/+4/+8 links, +0C key, +25 sentinel). No validation.
void* lower_bound_native_hardware_layout_key_00b23020(
    void* actual_tree, const void* key) noexcept;

// Native ECX tree, stack output/key, RET8, EAX output. Lower-bound precedes
// the null-owner handler; therefore a null tree already faults on ordinary
// Win32 storage. After selection, publish output owner before output node.
NativeHardwareLayoutTreeIterator* find_native_hardware_layout_key_00b28220(
    void* actual_tree, NativeHardwareLayoutTreeIterator* output, const void* key,
    const SingletonLifetimeCallbacks& invalid_parameters);

// Native ECX destination, stack key/value-address, RET8, EAX destination.
// Forward DWORD copy, rereading the source count after every store. Read the
// value through its address only after writing the count. Unused words stay.
void* construct_native_hardware_layout_pair_00b282b0(
    void* destination_pair, const void* source_key, const void* value_address) noexcept;

// Native ECX destination, stack source, RET4, EAX destination. Same forward
// copy and current-count rules; read source +14h after writing destination +10h.
void* copy_native_hardware_layout_pair_00b25ef0(
    void* destination_pair, const void* source_pair) noexcept;

// Every accessed word must have valid backing storage, including aliased and
// native out-of-profile counts. These routines add no count clamp. New MSVC
// Win32 C++ interfaces, not drop-in native calling-convention replacements.
} // namespace bsp
