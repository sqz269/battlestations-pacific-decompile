#include "bsp/native_resource_cache_erase.hpp"

#include "bsp/native_hardware_layout_tree.hpp"
#include "bsp/native_legacy_sbo_string.hpp"
#include "bsp/native_resource_cache_links.hpp"
#include "bsp/native_resource_cache_lookup.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstdint>

namespace bsp {
namespace {
void* at(const void* owner, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(owner) + offset);
}
void* volatile& pointer(const void* owner, std::uint32_t offset = 0) noexcept {
    return *static_cast<void* volatile*>(at(owner, offset));
}
unsigned char volatile& color(void* node) noexcept {
    return *static_cast<unsigned char volatile*>(at(node, 0x18));
}
bool nil(const void* node) noexcept {
    return *static_cast<const unsigned char volatile*>(at(node, 0x19)) != 0;
}
struct CompletedTemporary {
    NativeLegacySboStringStorage& value;
    ~CompletedTemporary() noexcept { native_legacy_sbo_string_destroy_004072d0(value); }
};
[[noreturn]] void invalid_iterator() {
    NativeLegacySboStringStorage temporary;
    temporary.capacity_18 = 15;
    temporary.length_14 = 0;
    temporary.buffer_04.inline_bytes[0] = 0;
    native_legacy_sbo_string_assign_counted_00408720(
        temporary, "invalid map/set<T> iterator", 27);
    // FAB6 / FuncInfo DFB340 arms state 0 only after assignment completes.
    // CC2110 returns this temporary on construction/throw unwind. D863A8
    // selects the existing D6926C owning out_of_range payload and 4412B0 dtor.
    const CompletedTemporary completed{temporary};
    throw NativeHardwareLayoutInvalidIterator{temporary};
}
} // namespace

void* maximum_native_resource_cache_node_00b7cb70(void* node) noexcept {
    auto* child = pointer(node, 8);
    while (!nil(child)) {
        node = child;
        child = pointer(node, 8);
    }
    return node;
}

void* minimum_native_resource_cache_node_00b7cb90(void* node) noexcept {
    auto* child = pointer(node);
    while (!nil(child)) {
        node = child;
        child = pointer(node);
    }
    return node;
}

void increment_native_resource_cache_iterator_00b7ce80(void* iterator,
    const SingletonLifetimeCallbacks& callbacks) {
    if (!pointer(iterator)) callbacks.invalid_parameter(callbacks.context);
    auto* const node = pointer(iterator, 4);
    if (nil(node)) {
        callbacks.invalid_parameter(callbacks.context); // CE97 tail, may return.
        return;
    }
    auto* child = pointer(node, 8);
    if (!nil(child)) {
        auto* next = pointer(child);
        while (!nil(next)) {
            child = next;
            next = pointer(child);
        }
        pointer(iterator, 4) = child;
        return;
    }
    auto* ancestor = pointer(node, 4);
    while (!nil(ancestor) && pointer(iterator, 4) == pointer(ancestor, 8)) {
        pointer(iterator, 4) = ancestor;
        ancestor = pointer(ancestor, 4);
    }
    pointer(iterator, 4) = ancestor;
}

void* erase_native_resource_cache_iterator_00b7fa60(void* tree, void* output,
    NativeResourceCacheIteratorStorage input, NativeStringRawPoolContext& strings,
    const SingletonLifetimeCallbacks& callbacks) {
    auto* const original = pointer(&input, 4);
    if (nil(original)) invalid_iterator();
    increment_native_resource_cache_iterator_00b7ce80(&input, callbacks);

    auto* const original_left = pointer(original);
    void* replacement;
    void* transplant = original;
    if (nil(original_left)) {
        replacement = pointer(original, 8);
    } else {
        auto* const original_right = pointer(original, 8);
        if (nil(original_right)) replacement = original_left;
        else {
            transplant = pointer(&input, 4); // Advanced local, not original.
            replacement = pointer(transplant, 8);
        }
    }

    void* fixup_parent;
    if (transplant == original) {
        const bool replacement_nil = nil(replacement);
        fixup_parent = pointer(original, 4);
        if (!replacement_nil) pointer(replacement, 4) = fixup_parent;
        auto* head = pointer(tree, 4);
        if (pointer(head, 4) == original) pointer(head, 4) = replacement;
        else if (pointer(fixup_parent) == original) pointer(fixup_parent) = replacement;
        else pointer(fixup_parent, 8) = replacement;

        head = pointer(tree, 4);
        if (pointer(head) == original) {
            auto* const first = nil(replacement) ? fixup_parent :
                minimum_native_resource_cache_node_00b7cb90(replacement);
            pointer(head) = first;
        }
        head = pointer(tree, 4); // Reload separately for the maximum update.
        if (pointer(head, 8) == original) {
            auto* const last = nil(replacement) ? fixup_parent :
                maximum_native_resource_cache_node_00b7cb70(replacement);
            pointer(head, 8) = last;
        }
    } else {
        pointer(original_left, 4) = transplant;
        pointer(transplant) = pointer(original);
        if (transplant == pointer(original, 8)) fixup_parent = transplant;
        else {
            const bool replacement_nil = nil(replacement);
            fixup_parent = pointer(transplant, 4);
            if (!replacement_nil) pointer(replacement, 4) = fixup_parent;
            pointer(fixup_parent) = replacement;
            pointer(transplant, 8) = pointer(original, 8);
            pointer(pointer(original, 8), 4) = transplant;
        }
        auto* const head = pointer(tree, 4);
        if (pointer(head, 4) == original) pointer(head, 4) = transplant;
        else {
            auto* const parent = pointer(original, 4);
            if (pointer(parent) == original) pointer(parent) = transplant;
            else pointer(parent, 8) = transplant;
        }
        pointer(transplant, 4) = pointer(original, 4);
        const auto original_color = color(original);
        const auto successor_color = color(transplant);
        color(transplant) = original_color;
        color(original) = successor_color;
    }

    if (color(original) == 1) {
        if (replacement != pointer(pointer(tree, 4), 4)) {
            for (;;) {
                if (color(replacement) != 1) break;
                auto* sibling = pointer(fixup_parent);
                bool propagate;
                if (replacement == sibling) {
                    sibling = pointer(fixup_parent, 8);
                    if (color(sibling) == 0) {
                        color(sibling) = 1;
                        color(fixup_parent) = 0;
                        rotate_native_resource_cache_left_00b7d5f0(tree, fixup_parent);
                        sibling = pointer(fixup_parent, 8);
                    }
                    propagate = nil(sibling);
                    if (!propagate) {
                        propagate = color(pointer(sibling)) == 1 &&
                            color(pointer(sibling, 8)) == 1;
                        if (propagate) color(sibling) = 0;
                        else {
                            if (color(pointer(sibling, 8)) == 1) {
                                color(pointer(sibling)) = 1;
                                color(sibling) = 0;
                                rotate_native_resource_cache_right_00b7cbd0(tree, sibling);
                                sibling = pointer(fixup_parent, 8);
                            }
                            color(sibling) = color(fixup_parent);
                            color(fixup_parent) = 1;
                            color(pointer(sibling, 8)) = 1;
                            rotate_native_resource_cache_left_00b7d5f0(tree, fixup_parent);
                        }
                    }
                } else {
                    if (color(sibling) == 0) {
                        color(sibling) = 1;
                        color(fixup_parent) = 0;
                        rotate_native_resource_cache_right_00b7cbd0(tree, fixup_parent);
                        sibling = pointer(fixup_parent);
                    }
                    propagate = nil(sibling);
                    if (!propagate) {
                        propagate = color(pointer(sibling, 8)) == 1 &&
                            color(pointer(sibling)) == 1;
                        if (propagate) color(sibling) = 0;
                        else {
                            if (color(pointer(sibling)) == 1) {
                                color(pointer(sibling, 8)) = 1;
                                color(sibling) = 0;
                                rotate_native_resource_cache_left_00b7d5f0(tree, sibling);
                                sibling = pointer(fixup_parent);
                            }
                            color(sibling) = color(fixup_parent);
                            color(fixup_parent) = 1;
                            color(pointer(sibling)) = 1;
                            rotate_native_resource_cache_right_00b7cbd0(tree, fixup_parent);
                        }
                    }
                }
                if (!propagate) break;
                auto* const head = pointer(tree, 4);
                replacement = fixup_parent;
                const bool reached_root = replacement == pointer(head, 4);
                fixup_parent = pointer(fixup_parent, 4); // Even at root.
                if (reached_root) break;
            }
        }
        color(replacement) = 1;
    }

    // FCD1..FCEB captures data and length+1 before the current pool getter.
    // The existing raw-header destructor has exactly that release schedule.
    destroy_native_string_header_0041dd20(at(original, 0x0c), strings);
    singleton_lifetime_free(original);
    auto& count = *static_cast<volatile std::uint32_t*>(at(tree, 8));
    const auto current_count = count;
    if (current_count != 0) count = current_count - 1u;
    auto* const owner = pointer(&input);
    auto* const node = pointer(&input, 4);
    pointer(output) = owner;
    pointer(output, 4) = node;
    return output;
}

void erase_native_resource_manager_name_00b801c0(void* manager, const void* name,
    NativeStringRawPoolContext& strings, const SingletonLifetimeCallbacks& callbacks) {
    auto* const tree = at(manager, 0x14);
    NativeResourceCacheIteratorStorage found;
    find_native_resource_cache_name_00b7e7b0(tree, &found, name, callbacks);
    auto* const owner = pointer(&found);
    auto* const head = pointer(tree, 4); // Captured before returning validation.
    if (!owner || owner != tree) callbacks.invalid_parameter(callbacks.context);
    auto* const node = pointer(&found, 4);
    if (node != head) {
        erase_native_resource_cache_iterator_00b7fa60(tree, &found, {owner, node},
            strings, callbacks);
    }
}
} // namespace bsp
