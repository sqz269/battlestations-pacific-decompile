#include "bsp/native_vfs_mount_tree.hpp"

#include "bsp/native_hardware_layout_tree.hpp"
#include "bsp/native_vfs_date_route.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native VFS mount trees require MSVC Win32.
#endif

namespace bsp {
namespace {
void* at(const void* p, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + offset);
}
void* volatile& link(void* p, std::uint32_t offset = 0) noexcept {
    return *static_cast<void* volatile*>(at(p, offset));
}
volatile std::uint32_t& word(void* p, std::uint32_t offset) noexcept {
    return *static_cast<volatile std::uint32_t*>(at(p, offset));
}
volatile unsigned char& color(void* node) noexcept {
    return *static_cast<volatile unsigned char*>(at(node, 0x20));
}
bool nil(void* node) noexcept {
    return *static_cast<const volatile unsigned char*>(at(node, 0x21)) != 0;
}
void* head(void* tree) noexcept { return link(tree, 4); }
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
    // BE00D6 arms state0 only after assignment. E00CC4 -> E00CBC ->
    // CC66C0 cleans this temporary; 411700 owns its own failed construction.
    const CompletedTemporary completed{temporary};
    throw NativeHardwareLayoutInvalidIterator{temporary};
}
} // namespace

void* maximum_native_vfs_mount_node_00bd94d0(void* node) noexcept {
    auto* child = link(node, 8);
    while (!nil(child)) {
        node = child;
        child = link(node, 8);
    }
    return node;
}
void* minimum_native_vfs_mount_node_00bd94f0(void* node) noexcept {
    auto* child = link(node);
    while (!nil(child)) {
        node = child;
        child = link(node);
    }
    return node;
}
void rotate_native_vfs_mount_right_00bd9530(void* tree, void* node) noexcept {
    auto* const replacement = link(node);
    link(node) = link(replacement, 8);
    auto* const child = link(replacement, 8);
    if (!nil(child)) link(child, 4) = node;
    link(replacement, 4) = link(node, 4);
    auto* const current_head = head(tree);
    if (node == link(current_head, 4)) {
        link(current_head, 4) = replacement;
    } else {
        auto* const parent = link(node, 4);
        if (node == link(parent, 8)) link(parent, 8) = replacement;
        else link(parent) = replacement;
    }
    link(replacement, 8) = node;
    link(node, 4) = replacement;
}
void rotate_native_vfs_mount_left_00bda0e0(void* tree, void* node) noexcept {
    auto* const replacement = link(node, 8);
    link(node, 8) = link(replacement);
    auto* const child = link(replacement);
    if (!nil(child)) link(child, 4) = node;
    link(replacement, 4) = link(node, 4);
    auto* const current_head = head(tree);
    if (node == link(current_head, 4)) {
        link(current_head, 4) = replacement;
    } else {
        auto* const parent = link(node, 4);
        if (node == link(parent)) link(parent) = replacement;
        else link(parent, 8) = replacement;
    }
    link(replacement) = node;
    link(node, 4) = replacement;
}

void destroy_native_vfs_mount_subtree_00bdf7e0(void* tree, void* node,
    NativeStringStorage& strings) noexcept {
    while (!nil(node)) {
        destroy_native_vfs_mount_subtree_00bdf7e0(tree, link(node, 8), strings);
        auto* const data = static_cast<char*>(link(node, 0x14));
        // BDF801 captures left even with null key, before reading length or
        // entering the pool. Later pool callbacks can change the current links.
        auto* const next = link(node);
        if (data) strings.release(data, word(node, 0x10) + 1u);
        singleton_lifetime_free(node);
        node = next;
    }
}

void* erase_native_vfs_mount_iterator_00be0080(void* tree, void* output,
    void* iterator_owner, void* iterator_node, NativeStringStorage& strings,
    const SingletonLifetimeCallbacks& callbacks) {
    void* input[2] = {iterator_owner, iterator_node};
    auto* const original = iterator_node;
    if (nil(original)) throw_invalid_iterator();
    advance_native_vfs_mount_iterator_00bd97e0(input, callbacks);
    auto* const original_left = link(original);
    void* replacement;
    void* fixup_parent;
    void* successor = nullptr;
    bool transplant = false;
    if (nil(original_left)) replacement = link(original, 8);
    else if (nil(link(original, 8))) replacement = original_left;
    else {
        successor = link(input, 4);
        replacement = link(successor, 8);
        transplant = successor != original;
    }
    if (!transplant) {
        const bool replacement_nil = nil(replacement);
        fixup_parent = link(original, 4);
        if (!replacement_nil) link(replacement, 4) = fixup_parent;
        auto* current_head = head(tree);
        if (link(current_head, 4) == original) link(current_head, 4) = replacement;
        else if (link(fixup_parent) == original) link(fixup_parent) = replacement;
        else link(fixup_parent, 8) = replacement;
        current_head = head(tree);
        if (link(current_head) == original) {
            link(current_head) = nil(replacement) ? fixup_parent
                : minimum_native_vfs_mount_node_00bd94f0(replacement);
        }
        current_head = head(tree);
        if (link(current_head, 8) == original) {
            link(current_head, 8) = nil(replacement) ? fixup_parent
                : maximum_native_vfs_mount_node_00bd94d0(replacement);
        }
    } else {
        // BE018C..BE01E3 is reachable despite the saved decompiler warning.
        link(original_left, 4) = successor;
        link(successor) = link(original);
        if (successor == link(original, 8)) fixup_parent = successor;
        else {
            const bool replacement_nil = nil(replacement);
            fixup_parent = link(successor, 4);
            if (!replacement_nil) link(replacement, 4) = fixup_parent;
            link(fixup_parent) = replacement;
            link(successor, 8) = link(original, 8);
            link(link(original, 8), 4) = successor;
        }
        auto* const current_head = head(tree);
        if (link(current_head, 4) == original) link(current_head, 4) = successor;
        else {
            auto* const parent = link(original, 4);
            if (link(parent) == original) link(parent) = successor;
            else link(parent, 8) = successor;
        }
        link(successor, 4) = link(original, 4);
        const auto original_color = color(original);
        const auto successor_color = color(successor);
        color(successor) = original_color;
        color(original) = successor_color;
    }
    if (color(original) == 1) {
        bool reached_root = replacement == link(head(tree), 4);
        while (!reached_root && color(replacement) == 1) {
            auto* sibling = link(fixup_parent);
            if (replacement == sibling) {
                sibling = link(fixup_parent, 8);
                if (color(sibling) == 0) {
                    color(sibling) = 1;
                    color(fixup_parent) = 0;
                    rotate_native_vfs_mount_left_00bda0e0(tree, fixup_parent);
                    sibling = link(fixup_parent, 8);
                }
                if (!nil(sibling)) {
                    if (color(link(sibling)) != 1 || color(link(sibling, 8)) != 1) {
                        if (color(link(sibling, 8)) == 1) {
                            color(link(sibling)) = 1;
                            color(sibling) = 0;
                            rotate_native_vfs_mount_right_00bd9530(tree, sibling);
                            sibling = link(fixup_parent, 8);
                        }
                        color(sibling) = color(fixup_parent);
                        color(fixup_parent) = 1;
                        color(link(sibling, 8)) = 1;
                        rotate_native_vfs_mount_left_00bda0e0(tree, fixup_parent);
                        break;
                    }
                    color(sibling) = 0;
                }
            } else {
                if (color(sibling) == 0) {
                    color(sibling) = 1;
                    color(fixup_parent) = 0;
                    rotate_native_vfs_mount_right_00bd9530(tree, fixup_parent);
                    sibling = link(fixup_parent);
                }
                if (!nil(sibling)) {
                    if (color(link(sibling, 8)) != 1 || color(link(sibling)) != 1) {
                        if (color(link(sibling)) == 1) {
                            color(link(sibling, 8)) = 1;
                            color(sibling) = 0;
                            rotate_native_vfs_mount_left_00bda0e0(tree, sibling);
                            sibling = link(fixup_parent);
                        }
                        color(sibling) = color(fixup_parent);
                        color(fixup_parent) = 1;
                        color(link(sibling)) = 1;
                        rotate_native_vfs_mount_right_00bd9530(tree, fixup_parent);
                        break;
                    }
                    color(sibling) = 0;
                }
            }
            auto* const current_head = head(tree);
            replacement = fixup_parent;
            reached_root = replacement == link(current_head, 4);
            fixup_parent = link(fixup_parent, 4);
        }
        color(replacement) = 1;
    }
    destroy_native_string_header_0041dd20(at(original, 0x10), strings);
    singleton_lifetime_free(original);
    // Raw BE0312..BE034B: current count is unsigned and zero stays zero.
    const auto count = word(tree, 8);
    if (count != 0) word(tree, 8) = count - 1u;
    auto* const advanced_owner = link(input);
    auto* const advanced_node = link(input, 4);
    link(output) = advanced_owner;
    link(output, 4) = advanced_node;
    return output;
}

void* erase_native_vfs_mount_range_00be0d00(void* tree, void* output,
    void* first_owner, void* first_node, void* last_owner, void* last_node,
    NativeStringStorage& strings, const SingletonLifetimeCallbacks& callbacks) {
    void* first[2] = {first_owner, first_node};
    auto* captured_owner = first_owner;
    auto* const minimum = link(head(tree));
    if (!captured_owner || captured_owner != tree) invalid(callbacks);
    auto* captured_node = link(first, 4);
    if (captured_node == minimum) {
        auto* const captured_head = head(tree);
        if (!last_owner || last_owner != tree) invalid(callbacks);
        if (last_node == captured_head) {
            destroy_native_vfs_mount_subtree_00bdf7e0(tree, link(head(tree), 4), strings);
            auto* current_head = head(tree);
            link(current_head, 4) = current_head;
            current_head = head(tree);
            word(tree, 8) = 0;
            link(current_head) = current_head;
            current_head = head(tree);
            link(current_head, 8) = current_head;
            auto* const result_node = link(head(tree));
            link(output) = tree;
            link(output, 4) = result_node;
            return output;
        }
    }
    for (;;) {
        if (!captured_owner || captured_owner != last_owner) invalid(callbacks);
        if (captured_node == last_node) break;
        advance_native_vfs_mount_iterator_00bd97e0(first, callbacks);
        void* ignored[2];
        erase_native_vfs_mount_iterator_00be0080(tree, ignored,
            captured_owner, captured_node, strings, callbacks);
        captured_node = link(first, 4);
        captured_owner = link(first);
    }
    link(output) = captured_owner;
    link(output, 4) = captured_node;
    return output;
}

void destroy_native_vfs_mount_tree_00be16c0(void* tree, NativeStringStorage& strings,
    const SingletonLifetimeCallbacks& callbacks) {
    auto* const captured_head = head(tree);
    auto* const first = link(captured_head);
    void* ignored[2];
    erase_native_vfs_mount_range_00be0d00(tree, ignored,
        tree, first, tree, captured_head, strings, callbacks);
    singleton_lifetime_free(head(tree));
    link(tree, 4) = nullptr;
    word(tree, 8) = 0;
}
} // namespace bsp
