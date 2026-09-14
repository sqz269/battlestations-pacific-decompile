#include "bsp/native_filestore_resident_insert.hpp"

#include "bsp/native_filestore_completion.hpp"
#include "bsp/native_filestore_resident_tree.hpp"
#include "bsp/native_hardware_layout_tree_insert.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
void* at(const void* p, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + offset);
}
void* volatile& link(void* p, std::uint32_t offset = 0) noexcept {
    return *static_cast<void* volatile*>(at(p, offset));
}
volatile std::uint32_t& word(void* p, std::uint32_t offset) noexcept {
    return *static_cast<volatile std::uint32_t*>(at(p, offset));
}
volatile std::uint8_t& color(void* p) noexcept {
    return *static_cast<volatile std::uint8_t*>(at(p, 0x18));
}
bool nil(void* p) noexcept {
    return *static_cast<const volatile std::uint8_t*>(at(p, 0x19)) != 0;
}
void invalid(const SingletonLifetimeCallbacks& callbacks) {
    callbacks.invalid_parameter(callbacks.context);
}
struct CompletedMessage {
    NativeLegacySboStringStorage& value;
    ~CompletedMessage() noexcept { native_legacy_sbo_string_destroy_004072d0(value); }
};
[[noreturn]] void throw_length_error() {
    NativeLegacySboStringStorage message;
    message.capacity_18 = 15;
    message.length_14 = 0;
    message.buffer_04.inline_bytes[0] = 0;
    native_legacy_sbo_string_assign_counted_00408720(message, "map/set<T> too long", 19);
    // BE6D41 arms only the completed message; CC6D40 destroys it on unwind.
    const CompletedMessage completed{message};
    throw NativeHardwareLayoutTreeLengthError{message};
}
} // namespace

void decrement_native_file_store_resident_iterator_00be4d20(void* iterator,
    const SingletonLifetimeCallbacks& callbacks) {
    if (link(iterator) == nullptr) invalid(callbacks);
    auto* node = link(iterator, 4);
    if (nil(node)) {
        auto* rightmost = link(node, 8);
        link(iterator, 4) = rightmost;
        if (nil(rightmost)) invalid(callbacks);
        return;
    }
    auto* left = link(node);
    if (!nil(left)) {
        auto* right = link(left, 8);
        while (!nil(right)) {
            left = right;
            right = link(left, 8);
        }
        link(iterator, 4) = left;
        return;
    }
    node = link(node, 4);
    while (!nil(node)) {
        auto* current = link(iterator, 4);
        if (current != link(node)) break;
        link(iterator, 4) = node;
        node = link(node, 4);
    }
    auto* current = link(iterator, 4);
    if (nil(current)) {
        invalid(callbacks);
        return;
    }
    link(iterator, 4) = node;
}

void* copy_native_file_store_resident_pair_00be6170(void* output,
    const void* pair, NativeStringStorage& strings) {
    return copy_native_file_store_resident_pair_00be6250(output, pair, strings);
}

void* allocate_native_file_store_resident_node_00be6590(void* left, void* parent,
    void* right, const void* pair, std::uint8_t node_color, NativeStringStorage& strings) {
    void* const node = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x1c, 0x1c});
    try {
        if (node != nullptr) {
            link(node) = left;
            link(node, 4) = parent;
            link(node, 8) = right;
            copy_native_file_store_resident_pair_00be6170(at(node, 0xc), pair, strings);
            color(node) = node_color;
            *static_cast<volatile std::uint8_t*>(at(node, 0x19)) = 0;
        }
    } catch (...) {
        // State1 -> state0 placement cleanup CC6CC0 only calls RET401130.
        // Catch_All BE660E frees the allocation, then BF6885(0,0) rethrows.
        singleton_lifetime_free(node);
        throw;
    }
    return node;
}

void* link_native_file_store_resident_node_00be6cf0(void* tree, void* output,
    std::uint8_t insert_left, void* parent, const void* pair, NativeStringStorage& strings) {
    if (word(tree, 8) >= 0x15555554u) throw_length_error();
    auto* const original_head = link(tree, 4);
    auto* const inserted = allocate_native_file_store_resident_node_00be6590(
        original_head, parent, original_head, pair, 0, strings);
    auto* current_head = link(tree, 4);
    word(tree, 8) = word(tree, 8) + 1u;
    if (parent == current_head) {
        link(current_head, 4) = inserted;
        link(link(tree, 4)) = inserted;
        link(link(tree, 4), 8) = inserted;
    } else if (insert_left != 0) {
        link(parent) = inserted;
        current_head = link(tree, 4);
        if (parent == link(current_head)) link(current_head) = inserted;
    } else {
        link(parent, 8) = inserted;
        current_head = link(tree, 4);
        if (parent == link(current_head, 8)) link(current_head, 8) = inserted;
    }

    auto* node = inserted;
    while (color(link(node, 4)) == 0) {
        auto* current_parent = link(node, 4);
        auto* grandparent = link(current_parent, 4);
        if (current_parent == link(grandparent)) {
            auto* uncle = link(grandparent, 8);
            if (color(uncle) == 0) {
                color(current_parent) = 1;
                color(uncle) = 1;
                color(link(link(node, 4), 4)) = 0;
                node = link(link(node, 4), 4);
            } else {
                if (node == link(current_parent, 8)) {
                    node = current_parent;
                    rotate_native_file_store_resident_left_00be50e0(tree, node);
                }
                color(link(node, 4)) = 1;
                color(link(link(node, 4), 4)) = 0;
                rotate_native_file_store_resident_right_00be4980(tree, link(link(node, 4), 4));
            }
        } else {
            auto* uncle = link(grandparent);
            if (color(uncle) == 0) {
                color(current_parent) = 1;
                color(uncle) = 1;
                color(link(link(node, 4), 4)) = 0;
                node = link(link(node, 4), 4);
            } else {
                if (node == link(current_parent)) {
                    node = current_parent;
                    rotate_native_file_store_resident_right_00be4980(tree, node);
                }
                color(link(node, 4)) = 1;
                color(link(link(node, 4), 4)) = 0;
                // BE6E66..BE6EA2 inlines the same current-link left rotation.
                rotate_native_file_store_resident_left_00be50e0(tree, link(link(node, 4), 4));
            }
        }
    }
    color(link(link(tree, 4), 4)) = 1;
    link(output, 4) = inserted;
    link(output) = tree;
    return output;
}
} // namespace bsp
