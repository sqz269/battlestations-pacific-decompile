#pragma once

#include "bsp/native_tree_out_of_range_exception.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>
#include <type_traits>

namespace bsp {

// Actual Win32 checked-iterator words. Owner is address identity, not a header
// snapshot. Node storage is borrowed: links +0/+4/+8, value +20h, color +24h,
// sentinel +25h. Neither the opaque key nor the node allocation size is known.
struct NativeHardwareLayoutTreeIterator {
    void* owner;
    void* node;
};
static_assert(sizeof(NativeHardwareLayoutTreeIterator) == 8);
static_assert(offsetof(NativeHardwareLayoutTreeIterator, node) == 4);
static_assert(std::is_trivially_copyable_v<NativeHardwareLayoutTreeIterator>);

// Owning host exception transport: native 28h storage and D6926C profile data,
// with new host RTTI/catch type/exception ABI. Uses the exact 441760 copy owner.
class NativeHardwareLayoutInvalidIterator final {
public:
    explicit NativeHardwareLayoutInvalidIterator(const NativeLegacySboStringStorage&);
    NativeHardwareLayoutInvalidIterator(const NativeHardwareLayoutInvalidIterator&);
    NativeHardwareLayoutInvalidIterator& operator=(const NativeHardwareLayoutInvalidIterator&) = delete;
    ~NativeHardwareLayoutInvalidIterator() noexcept;
    const NativeLegacyExceptionStorage& native_storage() const noexcept { return storage_; }

private:
    NativeLegacyExceptionStorage storage_;
};
static_assert(sizeof(NativeHardwareLayoutInvalidIterator) == 0x28);

// Native ECX node, RET, EAX selected node; no allocation or owner validation.
void* maximum_native_hardware_layout_node_00b20860(void* node) noexcept;
void* minimum_native_hardware_layout_node_00b20880(void* node) noexcept;

// Native ECX actual tree, stack node, RET4. The tree's head is at +4; its head
// stores minimum/root/maximum at +0/+4/+8. No semantic native return.
void rotate_native_hardware_layout_right_00b20910(void* actual_tree, void* node) noexcept;
void rotate_native_hardware_layout_left_00b22bd0(void* actual_tree, void* node) noexcept;

// Native ECX actual iterator, RET or invalid-parameter tail. A returning
// sentinel handler returns without advancing. Invalid handlers may throw.
void increment_native_hardware_layout_iterator_00b20dc0(
    NativeHardwareLayoutTreeIterator&, const SingletonLifetimeCallbacks&);

// Native ECX destination, stack output/input-owner/input-node; EAX output,
// RET0Ch. Input is by value. Does not compare input owner with destination.
// Frees the original node, conditionally decrements current unsigned count +8,
// then publishes output owner before node. A sentinel input throws the owner
// above after the original counted-string assignment and EH arming point.
NativeHardwareLayoutTreeIterator* erase_native_hardware_layout_iterator_00b2ef00(
    void* actual_tree, NativeHardwareLayoutTreeIterator* output,
    NativeHardwareLayoutTreeIterator input, const SingletonLifetimeCallbacks&);

// Native stack target, renderer ECX unused, RET4. Borrow the actual 12-byte
// 0108D530 header; its address is the canonical checked-owner identity. Scan
// from the captured head minimum and erase only the first matching value +20h.
void remove_native_hardware_layout_value_00b2f4c0(
    void* actual_tree_0108d530, void* target_value,
    const SingletonLifetimeCallbacks& invalid_parameters);

// New MSVC Win32 interfaces; neither native calling-convention replacements
// nor a general tree constructor/inserter/destructor or COM ownership layer.
} // namespace bsp
