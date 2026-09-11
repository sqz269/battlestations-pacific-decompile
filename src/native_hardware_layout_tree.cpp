#include "bsp/native_hardware_layout_tree.hpp"

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native hardware-layout tree operations require MSVC Win32.
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
bool sentinel(void* node) noexcept { return byte(node, 0x25) != 0; }
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
    // B2EF52 / FuncInfo DF6024: state 0 begins only after assignment succeeds.
    // CBD7A0 destroys this completed string if construction or throwing unwinds.
    const CompletedTemporary completed{temporary};
    throw NativeHardwareLayoutInvalidIterator{temporary};
}

} // namespace

NativeHardwareLayoutInvalidIterator::NativeHardwareLayoutInvalidIterator(
    const NativeLegacySboStringStorage& message) {
    construct_native_legacy_logic_error_00411700(storage_, message);
    storage_.native_vtable_00 = 0x00d6926c;
}
NativeHardwareLayoutInvalidIterator::NativeHardwareLayoutInvalidIterator(
    const NativeHardwareLayoutInvalidIterator& source) {
    copy_native_tree_out_of_range_00441760(storage_, source.storage_);
}
NativeHardwareLayoutInvalidIterator::~NativeHardwareLayoutInvalidIterator() noexcept {
    destroy_native_tree_out_of_range_004412b0(storage_);
}

void* maximum_native_hardware_layout_node_00b20860(void* node) noexcept {
    auto* child = right(node);
    while (!sentinel(child)) {
        node = child;
        child = right(node);
    }
    return node;
}
void* minimum_native_hardware_layout_node_00b20880(void* node) noexcept {
    auto* child = left(node);
    while (!sentinel(child)) {
        node = child;
        child = left(node);
    }
    return node;
}

void rotate_native_hardware_layout_right_00b20910(void* tree, void* node) noexcept {
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

void rotate_native_hardware_layout_left_00b22bd0(void* tree, void* node) noexcept {
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

void increment_native_hardware_layout_iterator_00b20dc0(
    NativeHardwareLayoutTreeIterator& iterator, const SingletonLifetimeCallbacks& callbacks) {
    if (word(&iterator, 0) == nullptr) {
        invalid(callbacks);
    }
    auto* const node = word(&iterator, 4); // Reload after a returning owner handler.
    if (sentinel(node)) {
        invalid(callbacks);
        return; // Native tail jump, not a retry after possible callback mutation.
    }
    auto* selected = right(node);
    if (!sentinel(selected)) {
        auto* child = left(selected);
        while (!sentinel(child)) {
            selected = child;
            child = left(selected);
        }
    } else {
        selected = parent(node);
        while (!sentinel(selected) && word(&iterator, 4) == right(selected)) {
            word(&iterator, 4) = selected;
            selected = parent(selected);
        }
    }
    word(&iterator, 4) = selected;
}

NativeHardwareLayoutTreeIterator* erase_native_hardware_layout_iterator_00b2ef00(
    void* tree, NativeHardwareLayoutTreeIterator* output,
    NativeHardwareLayoutTreeIterator input, const SingletonLifetimeCallbacks& callbacks) {
    auto* const original = word(&input, 4);
    if (sentinel(original)) {
        throw_invalid_iterator();
    }
    increment_native_hardware_layout_iterator_00b20dc0(input, callbacks);
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
                : minimum_native_hardware_layout_node_00b20880(replacement);
        }
        current_head = head(tree);
        if (right(current_head) == original) {
            right(current_head) = sentinel(replacement) ? fixup_parent
                : maximum_native_hardware_layout_node_00b20860(replacement);
        }
    } else {
        // B2F00F..B2F065 is live assembly omitted by the original pseudocode.
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
                    rotate_native_hardware_layout_left_00b22bd0(tree, fixup_parent);
                    sibling = right(fixup_parent);
                }
                if (!sentinel(sibling)) {
                    auto* const near_child = left(sibling);
                    if (color(near_child) != 1 || color(right(sibling)) != 1) {
                        if (color(right(sibling)) == 1) {
                            color(near_child) = 1;
                            color(sibling) = 0;
                            rotate_native_hardware_layout_right_00b20910(tree, sibling);
                            sibling = right(fixup_parent);
                        }
                        color(sibling) = color(fixup_parent);
                        color(fixup_parent) = 1;
                        color(right(sibling)) = 1;
                        rotate_native_hardware_layout_left_00b22bd0(tree, fixup_parent);
                        break;
                    }
                    color(sibling) = 0;
                }
            } else {
                if (color(sibling) == 0) {
                    color(sibling) = 1;
                    color(fixup_parent) = 0;
                    rotate_native_hardware_layout_right_00b20910(tree, fixup_parent);
                    sibling = left(fixup_parent);
                }
                if (!sentinel(sibling)) {
                    auto* const near_child = right(sibling);
                    if (color(near_child) != 1 || color(left(sibling)) != 1) {
                        if (color(left(sibling)) == 1) {
                            color(near_child) = 1;
                            color(sibling) = 0;
                            rotate_native_hardware_layout_left_00b22bd0(tree, sibling);
                            sibling = left(fixup_parent);
                        }
                        color(sibling) = color(fixup_parent);
                        color(fixup_parent) = 1;
                        color(left(sibling)) = 1;
                        rotate_native_hardware_layout_right_00b20910(tree, fixup_parent);
                        break;
                    }
                    color(sibling) = 0;
                }
            }
            // B2F123 loads the current head before ascending and compares the
            // new replacement with that root before loading its parent.
            auto* const current_head = head(tree);
            replacement = fixup_parent;
            reached_root = replacement == parent(current_head);
            fixup_parent = parent(fixup_parent);
        }
        color(replacement) = 1;
    }
    singleton_lifetime_free(original);
    // Complete post-free tail B2F171..B2F1A6, absent from old pseudocode.
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

void remove_native_hardware_layout_value_00b2f4c0(
    void* actual_tree_0108d530, void* target_value,
    const SingletonLifetimeCallbacks& invalid_parameters) {
    auto* const captured_head = head(actual_tree_0108d530);
    NativeHardwareLayoutTreeIterator iterator{actual_tree_0108d530, left(captured_head)};
    auto* captured_owner = word(&iterator, 0);
    auto* captured_node = word(&iterator, 4);
    for (;;) {
        if (!captured_owner || captured_owner != actual_tree_0108d530) {
            invalid(invalid_parameters);
        }
        if (captured_node == captured_head) {
            return;
        }
        if (!captured_owner) {
            invalid(invalid_parameters);
        }
        if (captured_node == head(captured_owner)) {
            invalid(invalid_parameters);
        }
        // Native ESI/EDI stay captured across the returning validation calls.
        if (target_value == word(captured_node, 0x20)) {
            erase_native_hardware_layout_iterator_00b2ef00(actual_tree_0108d530,
                &iterator, {captured_owner, captured_node}, invalid_parameters);
            return;
        }
        increment_native_hardware_layout_iterator_00b20dc0(iterator, invalid_parameters);
        captured_node = word(&iterator, 4);
        captured_owner = word(&iterator, 0);
    }
}

} // namespace bsp
