#include "bsp/native_vfs_string_tree.hpp"
#include "bsp/native_hardware_layout_tree.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp {
namespace {
void* volatile& word(void* storage, std::size_t offset) noexcept {
    return *reinterpret_cast<void* volatile*>(static_cast<unsigned char*>(storage) + offset);
}
volatile std::uint32_t& number(void* storage, std::size_t offset) noexcept {
    return *reinterpret_cast<volatile std::uint32_t*>(static_cast<unsigned char*>(storage) + offset);
}
volatile std::uint8_t& byte(void* storage, std::size_t offset) noexcept {
    return *reinterpret_cast<volatile std::uint8_t*>(static_cast<unsigned char*>(storage) + offset);
}
void* volatile& left(void* node) noexcept { return word(node, 0); }
void* volatile& parent(void* node) noexcept { return word(node, 4); }
void* volatile& right(void* node) noexcept { return word(node, 8); }
volatile std::uint8_t& color(void* node) noexcept { return byte(node, 0x14); }
bool sentinel(void* node) noexcept { return byte(node, 0x15) != 0; }
void* head(void* tree) noexcept { return word(tree, 4); }
void invalid(const SingletonLifetimeCallbacks& callbacks) {
    callbacks.invalid_parameter(callbacks.context);
}
struct CompletedTemporary {
    NativeLegacySboStringStorage& storage;
    ~CompletedTemporary() noexcept { native_legacy_sbo_string_destroy_004072d0(storage); }
};
[[noreturn]] void throw_invalid_iterator() {
    NativeLegacySboStringStorage temporary;
    temporary.capacity_18 = 15;
    temporary.length_14 = 0;
    temporary.buffer_04.inline_bytes[0] = 0;
    native_legacy_sbo_string_assign_counted_00408720(
        temporary, "invalid map/set<T> iterator", 27);
    // 4CF8F6 arms state0 only after assignment. D8E714 -> C65CC0 releases
    // EBP-50 on construction/throw unwind; normal tree erasure stays state-1.
    const CompletedTemporary completed{temporary};
    throw NativeHardwareLayoutInvalidIterator{temporary};
}
void release_string(void* node, NativeStringStorage& storage) noexcept {
    auto* const data = static_cast<char*>(word(node, 0x10));
    if (data != nullptr) {
        const auto size = number(node, 0x0c) + std::uint32_t{1};
        storage.release(data, size);
    }
}
} // namespace

void* maximum_native_vfs_string_node_004b9fd0(void* node) noexcept {
    auto* child = right(node);
    while (!sentinel(child)) {
        node = child;
        child = right(node);
    }
    return node;
}
void* minimum_native_vfs_string_node_004be360(void* node) noexcept {
    auto* child = left(node);
    while (!sentinel(child)) {
        node = child;
        child = left(node);
    }
    return node;
}
void rotate_left_native_vfs_string_tree_004bf130(void* tree, void* node) noexcept {
    auto* const replacement = right(node);
    right(node) = left(replacement);
    auto* const child = left(replacement);
    if (!sentinel(child)) parent(child) = node;
    parent(replacement) = parent(node);
    auto* const current_head = head(tree);
    if (node == parent(current_head)) {
        parent(current_head) = replacement;
    } else {
        auto* const current_parent = parent(node);
        if (node == left(current_parent)) left(current_parent) = replacement;
        else right(current_parent) = replacement;
    }
    left(replacement) = node;
    parent(node) = replacement;
}
void rotate_right_native_vfs_string_tree_004bf180(void* tree, void* node) noexcept {
    auto* const replacement = left(node);
    left(node) = right(replacement);
    auto* const child = right(replacement);
    if (!sentinel(child)) parent(child) = node;
    parent(replacement) = parent(node);
    auto* const current_head = head(tree);
    if (node == parent(current_head)) {
        parent(current_head) = replacement;
    } else {
        auto* const current_parent = parent(node);
        if (node == right(current_parent)) right(current_parent) = replacement;
        else left(current_parent) = replacement;
    }
    right(replacement) = node;
    parent(node) = replacement;
}
void increment_native_vfs_string_iterator_004be730(void* iterator,
    const SingletonLifetimeCallbacks& callbacks) {
    if (word(iterator, 0) == nullptr) invalid(callbacks);
    auto* node = word(iterator, 4);
    if (sentinel(node)) {
        invalid(callbacks);
        return;
    }
    auto* child = right(node);
    if (!sentinel(child)) {
        node = left(child);
        while (!sentinel(node)) {
            child = node;
            node = left(child);
        }
        word(iterator, 4) = child;
        return;
    }
    node = parent(node);
    while (!sentinel(node)) {
        if (word(iterator, 4) != right(node)) break;
        word(iterator, 4) = node;
        node = parent(node);
    }
    word(iterator, 4) = node;
}
void erase_native_vfs_string_subtree_004cec60(void* tree, void* node,
    NativeStringStorage& storage) noexcept {
    if (sentinel(node)) return;
    do {
        erase_native_vfs_string_subtree_004cec60(tree, right(node), storage);
        auto* const data = static_cast<char*>(word(node, 0x10));
        auto* const next = left(node); // captured before getter/return/free
        if (data != nullptr) storage.release(data, number(node, 0x0c) + std::uint32_t{1});
        singleton_lifetime_free(node);
        node = next;
    } while (!sentinel(node));
}

void* erase_native_vfs_string_iterator_004cf8a0(void* tree, void* output,
    void* input_owner, void* input_node, NativeStringStorage& storage,
    const SingletonLifetimeCallbacks& callbacks) {
    auto* const removed = input_node;
    if (sentinel(removed)) throw_invalid_iterator();
    void* input[2]{input_owner, input_node};
    increment_native_vfs_string_iterator_004be730(input, callbacks);
    auto* const old_left = left(removed);
    void* replacement;
    void* replacement_parent = nullptr;
    bool transplanted = false;
    if (sentinel(old_left)) {
        replacement = right(removed);
    } else {
        auto* const old_right = right(removed);
        if (sentinel(old_right)) {
            replacement = old_left;
        } else {
            auto* const successor = input[1];
            replacement = right(successor);
            if (successor != removed) {
                parent(old_left) = successor;
                left(successor) = left(removed);
                if (successor == right(removed)) {
                    replacement_parent = successor;
                } else {
                    const bool nil_replacement = sentinel(replacement);
                    replacement_parent = parent(successor);
                    if (!nil_replacement) parent(replacement) = replacement_parent;
                    left(replacement_parent) = replacement;
                    right(successor) = right(removed);
                    parent(right(removed)) = successor;
                }
                auto* const current_head = head(tree);
                if (parent(current_head) == removed) {
                    parent(current_head) = successor;
                } else {
                    auto* const current_parent = parent(removed);
                    if (left(current_parent) == removed) left(current_parent) = successor;
                    else right(current_parent) = successor;
                }
                parent(successor) = parent(removed);
                const auto removed_color = color(removed);
                const auto successor_color = color(successor);
                color(successor) = removed_color;
                color(removed) = successor_color;
                transplanted = true;
            }
        }
    }
    if (!transplanted) {
        const bool nil_replacement = sentinel(replacement);
        replacement_parent = parent(removed);
        if (!nil_replacement) parent(replacement) = replacement_parent;
        auto* const current_head = head(tree);
        if (parent(current_head) == removed) parent(current_head) = replacement;
        else if (left(replacement_parent) == removed) left(replacement_parent) = replacement;
        else right(replacement_parent) = replacement;
        auto* const minimum_head = head(tree);
        if (left(minimum_head) == removed) {
            left(minimum_head) = sentinel(replacement) ? replacement_parent :
                minimum_native_vfs_string_node_004be360(replacement);
        }
        auto* const maximum_head = head(tree);
        if (right(maximum_head) == removed) {
            right(maximum_head) = sentinel(replacement) ? replacement_parent :
                maximum_native_vfs_string_node_004b9fd0(replacement);
        }
    }
    if (color(removed) == 1) {
        bool at_root = replacement == parent(head(tree));
        while (!at_root && color(replacement) == 1) {
            auto* sibling = left(replacement_parent);
            if (replacement == sibling) {
                sibling = right(replacement_parent);
                if (color(sibling) == 0) {
                    color(sibling) = 1;
                    color(replacement_parent) = 0;
                    rotate_left_native_vfs_string_tree_004bf130(tree, replacement_parent);
                    sibling = right(replacement_parent);
                }
                if (!sentinel(sibling)) {
                    if (color(left(sibling)) != 1 || color(right(sibling)) != 1) {
                        if (color(right(sibling)) == 1) {
                            color(left(sibling)) = 1;
                            color(sibling) = 0;
                            rotate_right_native_vfs_string_tree_004bf180(tree, sibling);
                            sibling = right(replacement_parent);
                        }
                        color(sibling) = color(replacement_parent);
                        color(replacement_parent) = 1;
                        color(right(sibling)) = 1;
                        rotate_left_native_vfs_string_tree_004bf130(tree, replacement_parent);
                        break;
                    }
                    color(sibling) = 0;
                }
            } else {
                if (color(sibling) == 0) {
                    color(sibling) = 1;
                    color(replacement_parent) = 0;
                    rotate_right_native_vfs_string_tree_004bf180(tree, replacement_parent);
                    sibling = left(replacement_parent);
                }
                if (!sentinel(sibling)) {
                    if (color(right(sibling)) != 1 || color(left(sibling)) != 1) {
                        if (color(left(sibling)) == 1) {
                            color(right(sibling)) = 1;
                            color(sibling) = 0;
                            rotate_left_native_vfs_string_tree_004bf130(tree, sibling);
                            sibling = left(replacement_parent);
                        }
                        color(sibling) = color(replacement_parent);
                        color(replacement_parent) = 1;
                        color(left(sibling)) = 1;
                        rotate_right_native_vfs_string_tree_004bf180(tree, replacement_parent);
                        break;
                    }
                    color(sibling) = 0;
                }
            }
            auto* const current_head = head(tree);
            replacement = replacement_parent;
            at_root = replacement == parent(current_head);
            replacement_parent = parent(replacement_parent);
        }
        color(replacement) = 1;
    }
    release_string(removed, storage);
    singleton_lifetime_free(removed);
    const auto current_count = number(tree, 8);
    if (current_count != 0) number(tree, 8) = current_count - 1;
    auto* const result_owner = input[0];
    auto* const result_node = input[1];
    word(output, 0) = result_owner;
    word(output, 4) = result_node;
    return output;
}

void* erase_native_vfs_string_range_004d1a50(void* tree, void* output,
    void* first_owner, void* first_node, void* last_owner, void* last_node,
    NativeStringStorage& storage, const SingletonLifetimeCallbacks& callbacks) {
    auto* owner = first_owner;
    auto* const begin = left(head(tree));
    if (owner == nullptr || owner != tree) invalid(callbacks);
    auto* node = first_node;
    if (node == begin) {
        auto* const current_head = head(tree);
        if (last_owner == nullptr || last_owner != tree) invalid(callbacks);
        if (last_node == current_head) {
            erase_native_vfs_string_subtree_004cec60(tree, parent(head(tree)), storage);
            auto* current = head(tree);
            parent(current) = current;
            current = head(tree);
            number(tree, 8) = 0;
            left(current) = current;
            current = head(tree);
            right(current) = current;
            auto* const result = left(head(tree));
            word(output, 0) = tree;
            word(output, 4) = result;
            return output;
        }
    }
    void* first[2]{first_owner, first_node};
    for (;;) {
        if (owner == nullptr || owner != last_owner) invalid(callbacks);
        if (node == last_node) break;
        increment_native_vfs_string_iterator_004be730(first, callbacks);
        void* discarded[2];
        erase_native_vfs_string_iterator_004cf8a0(tree, discarded, owner, node, storage, callbacks);
        node = first[1];
        owner = first[0];
    }
    word(output, 0) = owner;
    word(output, 4) = node;
    return output;
}
void destroy_native_vfs_string_tree_004d74a0(void* tree, NativeStringStorage& storage,
    const SingletonLifetimeCallbacks& callbacks) {
    auto* const current_head = head(tree);
    auto* const begin = left(current_head);
    void* discarded[2];
    erase_native_vfs_string_range_004d1a50(tree, discarded, tree, begin, tree, current_head,
        storage, callbacks);
    singleton_lifetime_free(head(tree));
    word(tree, 4) = nullptr;
    number(tree, 8) = 0;
}
} // namespace bsp
