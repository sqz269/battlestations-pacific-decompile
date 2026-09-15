#include "bsp/native_soldier_registry_lifetime.hpp"
#include "bsp/native_hardware_layout_tree.hpp"
#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <exception>

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native SoldierClass registry lifetime requires MSVC Win32.
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
    // 4B0386 / FuncInfo D8CD08 arms state 0 only after counted assignment.
    // C64940 destroys the completed temporary on owner construction/throw unwind.
    const CompletedTemporary completed{temporary};
    // Existing concrete owning transport: 411700, D6926C, 441760, 4412B0.
    // Its host C++ RTTI/catch type and throw ABI are an explicit source boundary.
    throw NativeHardwareLayoutInvalidIterator{temporary};
}
} // namespace

NativeKeyboardTreeIterator* erase_native_soldier_registry_iterator_004b0330(
    void* tree, NativeKeyboardTreeIterator* output,
    NativeKeyboardTreeIterator input, NativeStringRawPoolContext& strings,
    const SingletonLifetimeCallbacks& callbacks) {
    auto* const original = word(&input, 4);
    if (sentinel(original)) {
        throw_invalid_iterator();
    }
    increment_native_soldier_registry_iterator_004afb50(input, callbacks);
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
                : minimum_native_soldier_registry_node_004af680(replacement);
        }
        current_head = head(tree);
        if (right(current_head) == original) {
            right(current_head) = sentinel(replacement) ? fixup_parent
                : maximum_native_soldier_registry_node_004af6c0(replacement);
        }
    } else {
        // 4B043C..4B0493 is live assembly omitted by the original pseudocode.
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
                    rotate_native_soldier_registry_left_004afaa0(tree, fixup_parent);
                    sibling = right(fixup_parent);
                }
                if (!sentinel(sibling)) {
                    auto* const near_child = left(sibling);
                    if (color(near_child) != 1 || color(right(sibling)) != 1) {
                        if (color(right(sibling)) == 1) {
                            color(left(sibling)) = 1;
                            color(sibling) = 0;
                            rotate_native_soldier_registry_right_004afaf0(tree, sibling);
                            sibling = right(fixup_parent);
                        }
                        color(sibling) = color(fixup_parent);
                        color(fixup_parent) = 1;
                        color(right(sibling)) = 1;
                        rotate_native_soldier_registry_left_004afaa0(tree, fixup_parent);
                        break;
                    }
                    color(sibling) = 0;
                }
            } else {
                if (color(sibling) == 0) {
                    color(sibling) = 1;
                    color(fixup_parent) = 0;
                    rotate_native_soldier_registry_right_004afaf0(tree, fixup_parent);
                    sibling = left(fixup_parent);
                }
                if (!sentinel(sibling)) {
                    auto* const near_child = right(sibling);
                    if (color(near_child) != 1 || color(left(sibling)) != 1) {
                        if (color(left(sibling)) == 1) {
                            color(right(sibling)) = 1;
                            color(sibling) = 0;
                            rotate_native_soldier_registry_left_004afaa0(tree, sibling);
                            sibling = left(fixup_parent);
                        }
                        color(sibling) = color(fixup_parent);
                        color(fixup_parent) = 1;
                        color(left(sibling)) = 1;
                        rotate_native_soldier_registry_right_004afaf0(tree, fixup_parent);
                        break;
                    }
                    color(sibling) = 0;
                }
            }
            // 4B055A loads the current head before ascending and compares the
            // new replacement with that root before loading its parent.
            auto* const current_head = head(tree);
            replacement = fixup_parent;
            reached_root = replacement == parent(current_head);
            fixup_parent = parent(fixup_parent);
        }
        color(replacement) = 1;
    }
    // 4B05A1 captures data before length+1, resolves the actual pool even for
    // large/disabled returns, and leaves the key header untouched.
    destroy_native_string_header_0041dd20(
        static_cast<unsigned char*>(original) + 0x0c, strings);
    singleton_lifetime_free(original);
    // Complete post-free tail 4B05C2..4B05FB, absent from old pseudocode.
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

NativeKeyboardTreeIterator* erase_native_soldier_registry_range_004b09a0(
    void* tree, NativeKeyboardTreeIterator* output,
    NativeKeyboardTreeIterator first, NativeKeyboardTreeIterator last,
    NativeStringRawPoolContext& strings, const SingletonLifetimeCallbacks& callbacks) {
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
            destroy_native_soldier_registry_subtree_004b0600(tree, word(word(tree, 4), 4), strings);
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
        increment_native_soldier_registry_iterator_004afb50(first, callbacks);
        NativeKeyboardTreeIterator ignored;
        erase_native_soldier_registry_iterator_004b0330(
            tree, &ignored, {captured_owner, captured_node}, strings, callbacks);
        captured_node = word(&first, 4);
        captured_owner = word(&first, 0);
    }
}

void* minimum_native_soldier_registry_node_004af680(void* node) noexcept {
    void* next = left(node);
    while (!sentinel(next)) {
        node = next;
        next = left(node);
    }
    return node;
}
void* maximum_native_soldier_registry_node_004af6c0(void* node) noexcept {
    void* next = right(node);
    while (!sentinel(next)) {
        node = next;
        next = right(node);
    }
    return node;
}

void destroy_native_soldier_registry_subtree_004b0600(
    void* tree, void* node, NativeStringRawPoolContext& strings) {
    while (!sentinel(node)) {
        destroy_native_soldier_registry_subtree_004b0600(tree, right(node), strings);
        // Native order is data, then left, then (for nonnull data) length.
        void* const data = word(node, 0x10);
        void* const next = left(node);
        if (data != nullptr) {
            // A local header keeps the captured pointer while the actual raw
            // pool getter runs; it does not modify or own the original header.
            struct Key { std::uint32_t length; void* data; } captured{
                *reinterpret_cast<volatile std::uint32_t*>(
                    static_cast<unsigned char*>(node) + 0x0c), data};
            destroy_native_string_header_0041dd20(&captured, strings);
        }
        singleton_lifetime_free(node);
        node = next;
    }
}

void destroy_native_soldier_registry_004b1210(void* owner,
    const NativeSoldierClassConstructionAccess& access,
    const SingletonLifetimeCallbacks& callbacks) {
    void* const original_head = word(owner, 8);
    void* const original_minimum = left(original_head);
    void* const tree = static_cast<unsigned char*>(owner) + 4;
    NativeKeyboardTreeIterator ignored;
    try {
        erase_native_soldier_registry_range_004b09a0(tree, &ignored,
            {tree, original_minimum}, {tree, original_head}, access.strings, callbacks);
        singleton_lifetime_free(word(tree, 4));
        word(tree, 4) = nullptr;
        *reinterpret_cast<volatile std::uint32_t*>(
            static_cast<unsigned char*>(tree) + 8) = 0;
    } catch (...) {
        // C64A10 / D8CE20 state0: true FH3 unwind only unpublishes/stamps base.
        unwind_native_soldier_registry_004af950(owner, access);
        throw;
    }
    unwind_native_soldier_registry_004af950(owner, access);
}

void* delete_native_soldier_registry_004b1280(void* owner, std::uint32_t flags,
    const NativeSoldierClassConstructionAccess& access,
    const SingletonLifetimeCallbacks& callbacks) {
    destroy_native_soldier_registry_004b1210(owner, access, callbacks);
    if ((flags & 1u) != 0) singleton_lifetime_free(owner);
    return owner;
}

void* get_native_soldier_registry_004b1330(
    const NativeSoldierClassConstructionAccess& access) {
    void* const initial = access.actual_registry_publication_00e187f0;
    if (initial != nullptr) return initial;

    void* const first_manager = get_native_singleton_manager_00415350(
        access.strings.actual_manager_publication_01090aa0);
    auto* const captured_section = *reinterpret_cast<CRITICAL_SECTION* volatile*>(
        static_cast<unsigned char*>(first_manager) + 0x10);
    struct Guard { std::uint32_t profile; CRITICAL_SECTION* section; } guard{
        0x00ce37fcu, captured_section};
    static_assert(sizeof(Guard) == 8);
    static_assert(sizeof(CRITICAL_SECTION) == 0x18);
    const auto tracked_depth = [](CRITICAL_SECTION* section) -> volatile std::uint32_t& {
        return *reinterpret_cast<volatile std::uint32_t*>(
            reinterpret_cast<unsigned char*>(section) + 0x18);
    };
    if (captured_section != nullptr) {
        EnterCriticalSection(captured_section);
        auto& depth = tracked_depth(captured_section);
        depth = depth + 1u;
    }
    // D8CE78 state0 starts after Enter and raw depth increment. State1 owns
    // only the captured raw allocation until construction completes.
    try {
        if (access.actual_registry_publication_00e187f0 == nullptr) {
            void* const allocation = singleton_lifetime_allocate({
                SingletonAllocationKind::object, 0x10, 0x10});
            void* fresh = nullptr;
            try {
                if (allocation != nullptr)
                    fresh = construct_native_soldier_registry_004b11a0(allocation, access);
            } catch (...) {
                singleton_lifetime_free(allocation);
                throw;
            }
            access.actual_registry_publication_00e187f0 = fresh;
            void* const current_manager = get_native_singleton_manager_00415350(
                access.strings.actual_manager_publication_01090aa0);
            void* const current_registry = access.actual_registry_publication_00e187f0;
            register_native_singleton_object_00bd0c30(current_manager, nullptr, current_registry);
        }
        if (captured_section != nullptr) {
            auto& depth = tracked_depth(captured_section);
            depth = depth - 1u;
            LeaveCriticalSection(captured_section);
        }
    } catch (...) {
        // C64A50 is a true unwind action; a second C++ exception terminates.
        try { destroy_native_singleton_guard_00411ee0(&guard); }
        catch (...) { std::terminate(); }
        throw;
    }
    return access.actual_registry_publication_00e187f0;
}
} // namespace bsp
