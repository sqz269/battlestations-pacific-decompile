#include "bsp/native_resource_registry_erase.hpp"
#include "bsp/native_string_pool_storage.hpp"

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource-registry erase requires MSVC Win32.
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
volatile std::uint8_t& color(void* node) noexcept { return byte(node, 0x18); }
bool sentinel(void* node) noexcept { return byte(node, 0x19) != 0; }
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
    // B19FE6 / FuncInfo DF48F0 arms state 0 only after counted assignment.
    // CBC620 destroys the completed temporary on owner construction/throw unwind.
    const CompletedTemporary completed{temporary};
    // Existing concrete owning transport: 411700, D6926C, 441760, 4412B0.
    // Its host C++ RTTI/catch type and throw ABI are an explicit source boundary.
    throw NativeHardwareLayoutInvalidIterator{temporary};
}
} // namespace

NativeResourceRegistryTreeIterator* erase_native_resource_registry_iterator_00b19f90(
    void* tree, NativeResourceRegistryTreeIterator* output,
    NativeResourceRegistryTreeIterator input, ActualNativeStringPoolStorage& strings,
    const SingletonLifetimeCallbacks& callbacks) {
    auto* const original = word(&input, 4);
    if (sentinel(original)) {
        throw_invalid_iterator();
    }
    increment_native_resource_registry_iterator_00b19890(input, callbacks);
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
        const bool replacement_is_nil = sentinel(replacement);
        fixup_parent = parent(original);
        if (!replacement_is_nil) {
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
                : minimum_native_resource_registry_node_00b196f0(replacement);
        }
        current_head = head(tree);
        if (right(current_head) == original) {
            right(current_head) = sentinel(replacement) ? fixup_parent
                : maximum_native_resource_registry_node_00b196d0(replacement);
        }
    } else {
        // B1A09C..B1A0F3 is live assembly omitted by the original pseudocode.
        parent(original_left) = successor;
        left(successor) = left(original);
        if (successor == right(original)) {
            fixup_parent = successor;
        } else {
            const bool replacement_is_nil = sentinel(replacement);
            fixup_parent = parent(successor);
            if (!replacement_is_nil) {
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
                    rotate_native_resource_registry_left_00b19830(tree, fixup_parent);
                    sibling = right(fixup_parent);
                }
                if (!sentinel(sibling)) {
                    auto* const near_child = left(sibling);
                    if (color(near_child) != 1 || color(right(sibling)) != 1) {
                        if (color(right(sibling)) == 1) {
                            color(left(sibling)) = 1;
                            color(sibling) = 0;
                            rotate_native_resource_registry_right_00b19640(tree, sibling);
                            sibling = right(fixup_parent);
                        }
                        color(sibling) = color(fixup_parent);
                        color(fixup_parent) = 1;
                        color(right(sibling)) = 1;
                        rotate_native_resource_registry_left_00b19830(tree, fixup_parent);
                        break;
                    }
                    color(sibling) = 0;
                }
            } else {
                if (color(sibling) == 0) {
                    color(sibling) = 1;
                    color(fixup_parent) = 0;
                    rotate_native_resource_registry_right_00b19640(tree, fixup_parent);
                    sibling = left(fixup_parent);
                }
                if (!sentinel(sibling)) {
                    auto* const near_child = right(sibling);
                    if (color(near_child) != 1 || color(left(sibling)) != 1) {
                        if (color(left(sibling)) == 1) {
                            color(right(sibling)) = 1;
                            color(sibling) = 0;
                            rotate_native_resource_registry_left_00b19830(tree, sibling);
                            sibling = left(fixup_parent);
                        }
                        color(sibling) = color(fixup_parent);
                        color(fixup_parent) = 1;
                        color(left(sibling)) = 1;
                        rotate_native_resource_registry_right_00b19640(tree, fixup_parent);
                        break;
                    }
                    color(sibling) = 0;
                }
            }
            // B1A1BA loads the current head before ascending and compares the
            // new replacement with that root before loading its parent.
            auto* const current_head = head(tree);
            replacement = fixup_parent;
            reached_root = replacement == parent(current_head);
            fixup_parent = parent(fixup_parent);
        }
        color(replacement) = 1;
    }
    auto* const data = static_cast<char*>(word(original, 0x10));
    if (data != nullptr) {
        const auto bytes = *reinterpret_cast<volatile std::uint32_t*>(
            static_cast<unsigned char*>(original) + 0x0c) + 1u;
        strings.release(data, bytes);
    }
    singleton_lifetime_free(original);
    // Complete post-free tail B1A222..B1A25B, absent from old pseudocode.
    auto& count = *reinterpret_cast<volatile std::uint32_t*>(
        static_cast<unsigned char*>(tree) + 8);
    const auto current_count = count;
    if (current_count != 0) {
        count = current_count - 1u;
    }
    auto* const advanced_owner = word(&input, 0);
    auto* const result = output;
    auto* const advanced_node = word(&input, 4);
    word(result, 0) = advanced_owner;
    word(result, 4) = advanced_node;
    return result;
}

NativeResourceRegistryTreeIterator* erase_native_resource_registry_range_00b1a2f0(
    void* tree, NativeResourceRegistryTreeIterator* output,
    NativeResourceRegistryTreeIterator first, NativeResourceRegistryTreeIterator last,
    ActualNativeStringPoolStorage& strings, const SingletonLifetimeCallbacks& callbacks) {
    void* captured_owner = word(&first, 0);
    void* const captured_minimum = word(word(tree, 4), 0);
    if (!captured_owner || captured_owner != tree) {
        invalid(callbacks);
    }
    void* captured_node = word(&first, 4);
    if (captured_node == captured_minimum) {
        void* const last_owner = word(&last, 0);
        void* const captured_head = word(tree, 4);
        if (!last_owner || last_owner != tree) {
            invalid(callbacks);
        }
        if (word(&last, 4) == captured_head) {
            destroy_native_resource_registry_subtree_00b1a260(tree, word(word(tree, 4), 4), strings);
            void* current_head = word(tree, 4);
            word(current_head, 4) = current_head;
            current_head = word(tree, 4);
            *reinterpret_cast<volatile std::uint32_t*>(
                static_cast<unsigned char*>(tree) + 8) = 0;
            word(current_head, 0) = current_head;
            current_head = word(tree, 4);
            word(current_head, 8) = current_head;
            void* const result_node = word(word(tree, 4), 0);
            word(output, 0) = tree;
            word(output, 4) = result_node;
            return output;
        }
    }
    for (;;) {
        if (!captured_owner || captured_owner != word(&last, 0)) {
            invalid(callbacks);
        }
        if (captured_node == word(&last, 4)) {
            word(output, 0) = captured_owner;
            word(output, 4) = captured_node;
            return output;
        }
        increment_native_resource_registry_iterator_00b19890(first, callbacks);
        NativeResourceRegistryTreeIterator ignored;
        erase_native_resource_registry_iterator_00b19f90(
            tree, &ignored, {captured_owner, captured_node}, strings, callbacks);
        captured_node = word(&first, 4);
        captured_owner = word(&first, 0);
    }
}
} // namespace bsp
