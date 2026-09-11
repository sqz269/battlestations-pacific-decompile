#pragma once

#include "bsp/native_hardware_layout_tree_key.hpp"

namespace bsp {

struct NativeHardwareLayoutTreeInsertResult {
    NativeHardwareLayoutTreeIterator iterator;
    std::uint8_t inserted_08;
    std::uint8_t untouched_padding_09[3];
};
static_assert(sizeof(NativeHardwareLayoutTreeInsertResult) == 12);
static_assert(offsetof(NativeHardwareLayoutTreeInsertResult, inserted_08) == 8);

// Owning host transport for the native D69260/28h length-error storage. Uses
// native logic-error construction/destruction and the actual 411940 copy.
// Host RTTI/catch and exception ABI are new, not the original D83F98 ABI.
class NativeHardwareLayoutTreeLengthError final {
public:
    explicit NativeHardwareLayoutTreeLengthError(const NativeLegacySboStringStorage&);
    NativeHardwareLayoutTreeLengthError(const NativeHardwareLayoutTreeLengthError&);
    NativeHardwareLayoutTreeLengthError& operator=(const NativeHardwareLayoutTreeLengthError&) = delete;
    ~NativeHardwareLayoutTreeLengthError() noexcept;
    const NativeLegacyExceptionStorage& native_storage() const noexcept { return storage_; }
private:
    NativeLegacyExceptionStorage storage_;
};
static_assert(sizeof(NativeHardwareLayoutTreeLengthError) == 0x28);

// Native ECX iterator, RET or invalid-parameter tail. A returning first
// handler continues with the iterator's current node. End decrements to max.
void decrement_native_hardware_layout_iterator_00b20d30(
    NativeHardwareLayoutTreeIterator&, const SingletonLifetimeCallbacks&);

// Native ECX node, stack left/parent/right/pair/color, RET14h, EAX node.
// Write links left/right/parent, copy current pair into +0Ch, then color/+25=0.
// Unused key words and final two padding bytes are preserved.
void* initialize_native_hardware_layout_node_00b28370(
    void* node, void* left, void* parent, void* right, const void* pair,
    std::uint8_t color) noexcept;

// Native stack left/parent/right/pair/color, RET14h, EAX allocated node.
// Shared original-new service boundary, fixed28h request, no payload clear.
void* allocate_native_hardware_layout_node_00b29cd0(
    void* left, void* parent, void* right, const void* pair, std::uint8_t color);

// Native ECX tree, stack output/left-byte/parent/pair, RET10h, EAX output.
// Unsigned count >=0AAAAAA9h throws the complete owning length error above.
// After successful allocation increment current count, link and rebalance;
// set root black, then publish output NODE before OWNER. No key ownership.
NativeHardwareLayoutTreeIterator* link_native_hardware_layout_node_00b2f1b0(
    void* actual_tree, NativeHardwareLayoutTreeIterator* output,
    std::uint8_t insert_left, void* parent, const void* pair);

// Native ECX tree, stack output/pair, RET8, EAX output. Actual comparator and
// predecessor validation. Duplicate leaves the supplied value unretained.
// Output publication is NODE, INSERTED BYTE, OWNER; padding stays untouched.
NativeHardwareLayoutTreeInsertResult* insert_native_hardware_layout_pair_00b2f540(
    void* actual_tree, NativeHardwareLayoutTreeInsertResult* output,
    const void* pair, const SingletonLifetimeCallbacks&);

// New MSVC Win32 APIs. Borrow a valid initialized red-black tree, supported
// allocator domain and valid reached key words; no native ABI or game claim.
} // namespace bsp
