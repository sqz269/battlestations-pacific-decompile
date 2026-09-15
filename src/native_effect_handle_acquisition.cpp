#include "bsp/native_effect_handle_acquisition.hpp"

#include "bsp/detail/native_tree_insert_storage.hpp"
#include "bsp/native_hardware_layout_tree_insert.hpp"
#include "bsp/native_int_pointer_tree18_leaves.hpp"
#include "bsp/native_legacy_sbo_string.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstdlib>

namespace bsp {
namespace {
using Access = detail::TreeInsertAccess<0x14, 0x15>;

std::int32_t pair_key(const NativeIntPointerTree18Pair* pair) noexcept {
    return *reinterpret_cast<const volatile std::int32_t*>(pair);
}
std::int32_t node_key(void* node) noexcept {
    return *reinterpret_cast<const volatile std::int32_t*>(
        static_cast<const unsigned char*>(node) + 0x0c);
}

void invalid_parameter() { _invalid_parameter_noinfo(); }

struct CompletedLengthMessage {
    NativeLegacySboStringStorage& storage;
    ~CompletedLengthMessage() noexcept {
        native_legacy_sbo_string_destroy_004072d0(storage);
    }
};

[[noreturn]] void throw_tree_length_error() {
    NativeLegacySboStringStorage message;
    message.capacity_18 = 15;
    message.length_14 = 0;
    message.buffer_04.inline_bytes[0] = '\0';
    native_legacy_sbo_string_assign_counted_00408720(
        message, "map/set<T> too long", 19);
    const CompletedLengthMessage completed{message};
    throw NativeHardwareLayoutTreeLengthError{message};
}

void publish_insert_result(NativeIntPointerTree18InsertResult* output,
    void* owner, void* node, std::uint8_t inserted) noexcept {
    // 0086F99E..A5 and0086F9D9..DE publish in this order.
    output->iterator_00.owner = owner;
    output->iterator_00.node = node;
    output->inserted_08 = inserted;
}
} // namespace

NativeIntPointerTree18Iterator* find_native_int_pointer_tree18_0086b650(
    void* tree, NativeIntPointerTree18Iterator* output,
    const std::int32_t* key) {
    void* const head = Access::head(tree);
    void* candidate = head;
    void* node = Access::parent(head);
    while (!Access::sentinel(node)) {
        const auto current_key = *static_cast<const volatile std::int32_t*>(key);
        const auto current_node_key = node_key(node);
        if (current_node_key < current_key) {
            node = Access::right(node);
        } else {
            candidate = node;
            node = Access::left(node);
        }
    }
    void* result = head;
    if (candidate != head) {
        const auto current_key = *static_cast<const volatile std::int32_t*>(key);
        const auto candidate_key = node_key(candidate);
        if (!(current_key < candidate_key)) result = candidate;
    }
    output->owner = tree;
    output->node = result;
    return output;
}

void decrement_native_int_pointer_tree18_00869990(
    NativeIntPointerTree18Iterator& iterator) {
    detail::decrement_tree_iterator<Access>(
        &iterator, [] { invalid_parameter(); });
}

void* allocate_native_int_pointer_tree18_node_0086ab70(
    void* left, void* parent, void* right,
    const NativeIntPointerTree18Pair* pair, std::uint8_t color) {
    void* const node = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x18, 0x18});
    if (node != nullptr) {
        Access::left(node) = left;
        Access::right(node) = right;
        Access::parent(node) = parent;
        *reinterpret_cast<volatile std::int32_t*>(
            static_cast<unsigned char*>(node) + 0x0c) = pair_key(pair);
        Access::word(node, 0x10) = pair->value_04;
        Access::color(node) = color;
        Access::byte(node, 0x15) = 0;
    }
    return node;
}

NativeIntPointerTree18Iterator* link_native_int_pointer_tree18_node_0086ebe0(
    void* tree, NativeIntPointerTree18Iterator* output,
    std::uint8_t insert_left, void* parent,
    const NativeIntPointerTree18Pair* pair) {
    detail::link_tree_node<Access>(tree, output, insert_left, parent, pair,
        0x1ffffffeu,
        [](void* left, void* parent_node, void* right, const void* value,
            std::uint8_t color) {
            return allocate_native_int_pointer_tree18_node_0086ab70(
                left, parent_node, right,
                static_cast<const NativeIntPointerTree18Pair*>(value), color);
        },
        [](void* actual_tree, void* pivot) {
            rotate_left_native_int_pointer_tree18_0086a2f0(
                actual_tree, nullptr, pivot);
        },
        [](void* actual_tree, void* pivot) {
            rotate_right_native_int_pointer_tree18_00869810(
                actual_tree, nullptr, pivot);
        },
        throw_tree_length_error);
    return output;
}

NativeIntPointerTree18InsertResult* insert_native_int_pointer_tree18_unique_0086f930(
    void* tree, NativeIntPointerTree18InsertResult* output,
    const NativeIntPointerTree18Pair* pair) {
    return detail::insert_unique_tree_pair<Access, NativeIntPointerTree18Iterator>(
        tree, output, pair,
        [pair](void* node) {
            const auto incoming = pair_key(pair);
            const auto current = node_key(node);
            return incoming < current;
        },
        [pair](void* node) {
            const auto current = node_key(node);
            const auto incoming = pair_key(pair);
            return current < incoming;
        },
        [](NativeIntPointerTree18Iterator& iterator) {
            decrement_native_int_pointer_tree18_00869990(iterator);
        },
        [](void* actual_tree, NativeIntPointerTree18Iterator* iterator,
            std::uint8_t insert_left, void* parent, const void* value) {
            return link_native_int_pointer_tree18_node_0086ebe0(
                actual_tree, iterator, insert_left, parent,
                static_cast<const NativeIntPointerTree18Pair*>(value));
        },
        publish_insert_result);
}

} // namespace bsp
