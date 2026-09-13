#include "bsp/native_hardware_layout_tree_insert.hpp"
#include "bsp/detail/native_tree_insert_storage.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native hardware-layout tree insertion requires MSVC Win32.
#endif

namespace bsp {
namespace {

void* volatile& word(void* storage, std::size_t offset) noexcept {
    return *reinterpret_cast<void* volatile*>(static_cast<unsigned char*>(storage) + offset);
}
volatile std::uint8_t& byte(void* storage, std::size_t offset) noexcept {
    return *reinterpret_cast<volatile std::uint8_t*>(static_cast<unsigned char*>(storage) + offset);
}
void* volatile& left(void* node) noexcept { return word(node, 0); }
void* volatile& parent(void* node) noexcept { return word(node, 4); }
void* volatile& right(void* node) noexcept { return word(node, 8); }
volatile std::uint8_t& color(void* node) noexcept { return byte(node, 0x24); }
void* key(void* node) noexcept { return static_cast<unsigned char*>(node) + 0x0c; }
void invalid(const SingletonLifetimeCallbacks& callbacks) {
    callbacks.invalid_parameter(callbacks.context);
}

struct CompletedLengthMessage {
    NativeLegacySboStringStorage& storage;
    ~CompletedLengthMessage() noexcept { native_legacy_sbo_string_destroy_004072d0(storage); }
};
[[noreturn]] void throw_length_error() {
    NativeLegacySboStringStorage message;
    message.capacity_18 = 15;
    message.length_14 = 0;
    message.buffer_04.inline_bytes[0] = '\0';
    native_legacy_sbo_string_assign_counted_00408720(message, "map/set<T> too long", 19);
    // B2F201 / CBD7C0: only the completed assignment arms string destruction.
    const CompletedLengthMessage completed{message};
    throw NativeHardwareLayoutTreeLengthError{message};
}

void publish_insert_result(NativeHardwareLayoutTreeInsertResult* output,
    void* owner, void* node, std::uint8_t inserted) noexcept {
    word(output, 4) = node;
    byte(output, 8) = inserted;
    word(output, 0) = owner;
}

} // namespace

NativeHardwareLayoutTreeLengthError::NativeHardwareLayoutTreeLengthError(
    const NativeLegacySboStringStorage& message) {
    construct_native_legacy_logic_error_00411700(storage_, message);
    storage_.native_vtable_00 = 0x00d69260;
}
NativeHardwareLayoutTreeLengthError::NativeHardwareLayoutTreeLengthError(
    const NativeHardwareLayoutTreeLengthError& source) {
    copy_native_legacy_length_error_00411940(storage_, source.storage_);
}
NativeHardwareLayoutTreeLengthError::~NativeHardwareLayoutTreeLengthError() noexcept {
    destroy_native_legacy_logic_error_00411780(storage_);
}

void decrement_native_hardware_layout_iterator_00b20d30(
    NativeHardwareLayoutTreeIterator& iterator, const SingletonLifetimeCallbacks& callbacks) {
    detail::decrement_tree_iterator<detail::TreeInsertAccess<0x24, 0x25>>(
        &iterator, [&callbacks] { invalid(callbacks); });
}

void* initialize_native_hardware_layout_node_00b28370(
    void* node, void* left_node, void* parent_node, void* right_node,
    const void* pair, std::uint8_t node_color) noexcept {
    left(node) = left_node;
    right(node) = right_node;
    parent(node) = parent_node;
    copy_native_hardware_layout_pair_00b25ef0(key(node), pair);
    color(node) = node_color;
    byte(node, 0x25) = 0;
    return node;
}

void* allocate_native_hardware_layout_node_00b29cd0(
    void* left_node, void* parent_node, void* right_node,
    const void* pair, std::uint8_t node_color) {
    auto* const node = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x28, 0x28});
    if (node != nullptr) {
        initialize_native_hardware_layout_node_00b28370(
            node, left_node, parent_node, right_node, pair, node_color);
    }
    return node;
}

NativeHardwareLayoutTreeIterator* link_native_hardware_layout_node_00b2f1b0(
    void* tree, NativeHardwareLayoutTreeIterator* output,
    std::uint8_t insert_left, void* parent_node, const void* pair) {
    detail::link_tree_node<detail::TreeInsertAccess<0x24, 0x25>>(
        tree, output, insert_left, parent_node, pair, 0x0aaaaaa9u,
        allocate_native_hardware_layout_node_00b29cd0,
        rotate_native_hardware_layout_left_00b22bd0,
        rotate_native_hardware_layout_right_00b20910, throw_length_error);
    return output;
}

NativeHardwareLayoutTreeInsertResult* insert_native_hardware_layout_pair_00b2f540(
    void* tree, NativeHardwareLayoutTreeInsertResult* output,
    const void* pair, const SingletonLifetimeCallbacks& callbacks) {
    return detail::insert_unique_tree_pair<detail::TreeInsertAccess<0x24, 0x25>,
        NativeHardwareLayoutTreeIterator>(tree, output, pair,
        [pair](void* node) { return less_native_hardware_layout_key_00b20bf0(pair, key(node)); },
        [pair](void* node) { return less_native_hardware_layout_key_00b20bf0(key(node), pair); },
        [&callbacks](NativeHardwareLayoutTreeIterator& iterator) {
            decrement_native_hardware_layout_iterator_00b20d30(iterator, callbacks);
        }, link_native_hardware_layout_node_00b2f1b0, publish_insert_result);
}

} // namespace bsp
