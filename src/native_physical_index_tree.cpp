#include "bsp/native_physical_index_tree.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_hardware_layout_tree.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstdint>

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
volatile std::uint8_t& color(void* node) noexcept { return byte(node, 0x1c); }
bool sentinel(void* node) noexcept { return byte(node, 0x1d) != 0; }
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
    temporary.buffer_04.inline_bytes[0] = '\0';
    native_legacy_sbo_string_assign_counted_00408720(
        temporary, "invalid map/set<T> iterator", 27);
    // BDFDD2 / FuncInfo E00C98: state 0 begins only after assignment succeeds.
    // CC66A0 destroys this completed string if construction or throwing unwinds.
    const CompletedTemporary completed{temporary};
    throw NativeHardwareLayoutInvalidIterator{temporary};
}

} // namespace

void* maximum_native_physical_index_node_00bd9380(void* node) noexcept {
    auto* child = right(node);
    while (!sentinel(child)) {
        node = child;
        child = right(node);
    }
    return node;
}
void* minimum_native_physical_index_node_00bd93a0(void* node) noexcept {
    auto* child = left(node);
    while (!sentinel(child)) {
        node = child;
        child = left(node);
    }
    return node;
}

void rotate_native_physical_index_right_00bd93e0(void* tree, void* node) noexcept {
    auto* const replacement = left(node);
    left(node) = right(replacement);
    auto* const child = right(replacement);
    if (!sentinel(child)) {
        parent(child) = node;
    }
    parent(replacement) = parent(node);
    auto* const current_head = head(tree);
    if (node == parent(current_head)) {
        parent(current_head) = replacement;
    } else {
        auto* const current_parent = parent(node);
        if (node == right(current_parent)) {
            right(current_parent) = replacement;
        } else {
            left(current_parent) = replacement;
        }
    }
    right(replacement) = node;
    parent(node) = replacement;
}

void rotate_native_physical_index_left_00bda090(void* tree, void* node) noexcept {
    auto* const replacement = right(node);
    right(node) = left(replacement);
    auto* const child = left(replacement);
    if (!sentinel(child)) {
        parent(child) = node;
    }
    parent(replacement) = parent(node);
    auto* const current_head = head(tree);
    if (node == parent(current_head)) {
        parent(current_head) = replacement;
    } else {
        auto* const current_parent = parent(node);
        if (node == left(current_parent)) {
            left(current_parent) = replacement;
        } else {
            right(current_parent) = replacement;
        }
    }
    left(replacement) = node;
    parent(node) = replacement;
}

void* erase_native_physical_index_iterator_00bdfd80(
    void* tree, void* output, void* owner, void* node,
    NativeStringStorage& strings, const SingletonLifetimeCallbacks& callbacks) {
    NativeHardwareLayoutTreeIterator input{owner, node};
    auto* const original = word(&input, 4);
    if (sentinel(original)) {
        throw_invalid_iterator();
    }
    advance_native_physical_index_00bd9860(&input, callbacks);
    auto* const original_left = left(original);
    void* replacement;
    void* fixup_parent;
    bool transplant = false;
    void* successor = nullptr;
    if (sentinel(original_left)) {
        replacement = right(original);
    } else if (sentinel(right(original))) {
        replacement = original_left;
    } else {
        successor = word(&input, 4);
        replacement = right(successor);
        transplant = successor != original;
    }

    if (!transplant) {
        fixup_parent = parent(original);
        if (!sentinel(replacement)) {
            parent(replacement) = fixup_parent;
        }
        auto* current_head = head(tree);
        if (parent(current_head) == original) {
            parent(current_head) = replacement;
        } else if (left(fixup_parent) == original) {
            left(fixup_parent) = replacement;
        } else {
            right(fixup_parent) = replacement;
        }
        current_head = head(tree);
        if (left(current_head) == original) {
            left(current_head) = sentinel(replacement) ? fixup_parent
                : minimum_native_physical_index_node_00bd93a0(replacement);
        }
        current_head = head(tree);
        if (right(current_head) == original) {
            right(current_head) = sentinel(replacement) ? fixup_parent
                : maximum_native_physical_index_node_00bd9380(replacement);
        }
    } else {
        // BDFE8F..BDFEE5 is live assembly omitted by the original pseudocode.
        parent(original_left) = successor;
        left(successor) = left(original);
        if (successor == right(original)) {
            fixup_parent = successor;
        } else {
            fixup_parent = parent(successor);
            if (!sentinel(replacement)) {
                parent(replacement) = fixup_parent;
            }
            left(fixup_parent) = replacement;
            right(successor) = right(original);
            parent(right(original)) = successor;
        }
        auto* const current_head = head(tree);
        if (parent(current_head) == original) {
            parent(current_head) = successor;
        } else {
            auto* const original_parent = parent(original);
            if (left(original_parent) == original) {
                left(original_parent) = successor;
            } else {
                right(original_parent) = successor;
            }
        }
        parent(successor) = parent(original);
        const auto original_color = color(original);
        const auto successor_color = color(successor);
        color(successor) = original_color;
        color(original) = successor_color;
    }

    if (color(original) == 1) {
        bool reached_root = replacement == parent(head(tree));
        while (!reached_root && color(replacement) == 1) {
            auto* sibling = left(fixup_parent);
            if (replacement == sibling) {
                sibling = right(fixup_parent);
                if (color(sibling) == 0) {
                    color(sibling) = 1;
                    color(fixup_parent) = 0;
                    rotate_native_physical_index_left_00bda090(tree, fixup_parent);
                    sibling = right(fixup_parent);
                }
                if (!sentinel(sibling)) {
                    auto* const near_child = left(sibling);
                    if (color(near_child) != 1 || color(right(sibling)) != 1) {
                        if (color(right(sibling)) == 1) {
                            color(near_child) = 1;
                            color(sibling) = 0;
                            rotate_native_physical_index_right_00bd93e0(tree, sibling);
                            sibling = right(fixup_parent);
                        }
                        color(sibling) = color(fixup_parent);
                        color(fixup_parent) = 1;
                        color(right(sibling)) = 1;
                        rotate_native_physical_index_left_00bda090(tree, fixup_parent);
                        break;
                    }
                    color(sibling) = 0;
                }
            } else {
                if (color(sibling) == 0) {
                    color(sibling) = 1;
                    color(fixup_parent) = 0;
                    rotate_native_physical_index_right_00bd93e0(tree, fixup_parent);
                    sibling = left(fixup_parent);
                }
                if (!sentinel(sibling)) {
                    auto* const near_child = right(sibling);
                    if (color(near_child) != 1 || color(left(sibling)) != 1) {
                        if (color(left(sibling)) == 1) {
                            color(near_child) = 1;
                            color(sibling) = 0;
                            rotate_native_physical_index_left_00bda090(tree, sibling);
                            sibling = left(fixup_parent);
                        }
                        color(sibling) = color(fixup_parent);
                        color(fixup_parent) = 1;
                        color(left(sibling)) = 1;
                        rotate_native_physical_index_right_00bd93e0(tree, fixup_parent);
                        break;
                    }
                    color(sibling) = 0;
                }
            }
            // BDFFA3 loads the current head before ascending and compares the
            // new replacement with that root before loading its parent.
            auto* const current_head = head(tree);
            replacement = fixup_parent;
            reached_root = replacement == parent(current_head);
            fixup_parent = parent(fixup_parent);
        }
        color(replacement) = 1;
    }
    destroy_native_physical_index_pair_00489d50(
        static_cast<unsigned char*>(original) + 0xc, strings);
    singleton_lifetime_free(original);
    // Complete post-free tail BDFFFD..BE0032, absent from old pseudocode.
    auto& count = *reinterpret_cast<volatile std::uint32_t*>(
        static_cast<unsigned char*>(tree) + 8);
    const auto current_count = count;
    if (current_count != 0) {
        count = current_count - 1u;
    }
    auto* const advanced_owner = word(&input, 0);
    auto* const advanced_node = word(&input, 4);
    word(output, 0) = advanced_owner;
    word(output, 4) = advanced_node;
    return output;
}

void destroy_native_physical_index_pair_00489d50(void* pair,
    NativeStringStorage& strings) noexcept {
    auto* data = static_cast<char*>(word(pair, 0xc));
    if (data) {
        const auto length = *reinterpret_cast<const volatile std::uint32_t*>(
            static_cast<const unsigned char*>(pair) + 8);
        strings.release(data, length + 1u);
    }
    // The first data pointer is read only after the second return completes.
    data = static_cast<char*>(word(pair, 4));
    if (data) {
        const auto length = *static_cast<const volatile std::uint32_t*>(pair);
        strings.release(data, length + 1u);
    }
}

void destroy_native_physical_index_subtree_00bdf7a0(void* tree, void* root,
    NativeStringStorage& strings) noexcept {
    auto* current = root;
    while (!sentinel(current)) {
        destroy_native_physical_index_subtree_00bdf7a0(tree, right(current), strings);
        auto* const next = left(current);
        destroy_native_physical_index_pair_00489d50(
            static_cast<unsigned char*>(current) + 0xc, strings);
        singleton_lifetime_free(current);
        current = next;
    }
}

void* erase_native_physical_index_range_00be0c30(void* tree, void* output,
    void* first_owner, void* first_node, void* last_owner, void* last_node,
    NativeStringStorage& strings, const SingletonLifetimeCallbacks& callbacks) {
    NativeHardwareLayoutTreeIterator first{first_owner, first_node};
    auto* captured_owner = first_owner;
    auto* const captured_minimum = left(head(tree));
    if (!captured_owner || captured_owner != tree) invalid(callbacks);
    auto* captured_node = word(&first, 4);
    if (captured_node == captured_minimum) {
        auto* const captured_head = head(tree);
        if (!last_owner || last_owner != tree) invalid(callbacks);
        if (last_node == captured_head) {
            destroy_native_physical_index_subtree_00bdf7a0(tree, parent(head(tree)), strings);
            auto* current_head = head(tree);
            parent(current_head) = current_head;
            current_head = head(tree);
            *reinterpret_cast<volatile std::uint32_t*>(
                static_cast<unsigned char*>(tree) + 8) = 0;
            left(current_head) = current_head;
            current_head = head(tree);
            right(current_head) = current_head;
            auto* const result_node = left(head(tree));
            word(output, 0) = tree;
            word(output, 4) = result_node;
            return output;
        }
    }
    for (;;) {
        if (!captured_owner || captured_owner != last_owner) invalid(callbacks);
        if (captured_node == last_node) break;
        advance_native_physical_index_00bd9860(&first, callbacks);
        NativeHardwareLayoutTreeIterator discarded;
        erase_native_physical_index_iterator_00bdfd80(tree, &discarded,
            captured_owner, captured_node, strings, callbacks);
        captured_node = word(&first, 4);
        captured_owner = word(&first, 0);
    }
    word(output, 0) = captured_owner;
    word(output, 4) = captured_node;
    return output;
}

void destroy_native_physical_index_00be1700(void* tree, NativeStringStorage& strings,
    const SingletonLifetimeCallbacks& callbacks) {
    auto* const captured_head = head(tree);
    auto* const captured_minimum = left(captured_head);
    NativeHardwareLayoutTreeIterator discarded;
    erase_native_physical_index_range_00be0c30(tree, &discarded,
        tree, captured_minimum, tree, captured_head, strings, callbacks);
    singleton_lifetime_free(head(tree));
    word(tree, 4) = nullptr;
    *reinterpret_cast<volatile std::uint32_t*>(
        static_cast<unsigned char*>(tree) + 8) = 0;
}

} // namespace bsp
