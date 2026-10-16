#include "bsp/native_input_controller_tree_erase.hpp"
#include "bsp/native_input_controller_tree_leaves.hpp"
#include "bsp/native_input_controller_name_pair_destroy.hpp"
#include "bsp/native_hardware_layout_tree.hpp"
#include "bsp/native_legacy_sbo_string.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));

Word word(const void* base, std::size_t offset) {
    return *reinterpret_cast<const volatile Word*>(
        static_cast<const std::byte*>(base) + offset);
}
void put(void* base, std::size_t offset, Word value) {
    *reinterpret_cast<volatile Word*>(
        static_cast<std::byte*>(base) + offset) = value;
}
std::uint8_t byte(const void* base, std::size_t offset) {
    return *reinterpret_cast<const volatile std::uint8_t*>(
        static_cast<const std::byte*>(base) + offset);
}
void put_byte(void* base, std::size_t offset, std::uint8_t value) {
    *reinterpret_cast<volatile std::uint8_t*>(
        static_cast<std::byte*>(base) + offset) = value;
}
Word address(const void* value) {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(value));
}
void* pointer(Word bits) {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(bits));
}
bool is_nil(const void* node) {
    return byte(node, 0x21) != 0;
}

struct CompletedMessage {
    NativeLegacySboStringStorage& storage;
    ~CompletedMessage() noexcept {
        cleanup_native_input_controller_erase_message_00c7f490(storage);
    }
};

[[noreturn]] __declspec(noinline) void throw_invalid_iterator() {
    NativeLegacySboStringStorage message;
    message.capacity_18 = 15;
    message.length_14 = 0;
    message.buffer_04.inline_bytes[0] = '\0';
    native_legacy_sbo_string_assign_counted_00408720(
        message, "invalid map/set<T> iterator", 27);
    // Native006A6062 arms state0 AFTER successful counted assignment.
    const CompletedMessage completed{message};
    throw NativeHardwareLayoutInvalidIterator{message};
}
} // namespace

void __fastcall cleanup_native_input_controller_erase_message_00c7f490(
    NativeLegacySboStringStorage& completed_message) noexcept {
    native_legacy_sbo_string_destroy_004072d0(completed_message);
}

NativeInputControllerTreeIterator* __fastcall
erase_native_input_controller_tree_iterator_006a6010(
    void* actual_tree, std::uint32_t unused_edx,
    NativeInputControllerTreeIterator* actual_output,
    NativeInputControllerTreeIterator input,
    NativeStringRawPoolContext& strings) {
    static_cast<void>(unused_edx);
    auto* const original_removed = pointer(word(&input, 4));
    if (is_nil(original_removed))
        throw_invalid_iterator();

    // Native EBP retains the receiver; its saved original-node word is
    // distinct from EBX, which later becomes head/black. Source EBP is free.
    auto* const receiver = actual_tree;
    auto* const removed = original_removed;
    increment_native_input_controller_tree_iterator_0069ad50(&input, 0u);
    auto* const captured_left = pointer(word(removed, 0));
    auto* successor = removed;
    void* replacement;
    void* parent;
    if (is_nil(captured_left)) {
        replacement = pointer(word(removed, 8));
    } else {
        auto* const right = pointer(word(removed, 8));
        if (is_nil(right)) {
            replacement = captured_left;
        } else {
            successor = pointer(word(&input, 4));
            replacement = pointer(word(successor, 8));
        }
    }

    if (successor == removed) {
        const bool replacement_nil = is_nil(replacement);
        parent = pointer(word(removed, 4));
        if (!replacement_nil)
            put(replacement, 4, address(parent));

        auto* head = pointer(word(receiver, 4));
        if (word(head, 4) == address(removed)) {
            put(head, 4, address(replacement));
        } else if (word(parent, 0) == address(removed)) {
            put(parent, 0, address(replacement));
        } else {
            put(parent, 8, address(replacement));
        }

        head = pointer(word(receiver, 4));
        const auto current_minimum = word(head, 0);
        const auto original_for_minimum = word(&original_removed, 0);
        if (current_minimum == original_for_minimum) {
            const auto selected = is_nil(replacement)
                ? address(parent)
                : address(minimum_native_input_controller_node_0069aa80(replacement));
            put(head, 0, selected);
        }
        head = pointer(word(receiver, 4));
        const auto current_original = word(&original_removed, 0);
        if (word(head, 8) == current_original) {
            const auto selected = is_nil(replacement)
                ? address(parent)
                : address(maximum_native_input_controller_node_0069aa60(replacement));
            put(head, 8, selected);
        }
    } else {
        // Retain the initial left capture, then use CURRENT removed links.
        put(captured_left, 4, address(successor));
        put(successor, 0, word(removed, 0));
        if (address(successor) == word(removed, 8)) {
            parent = successor;
        } else {
            const bool replacement_nil = is_nil(replacement);
            parent = pointer(word(successor, 4));
            if (!replacement_nil)
                put(replacement, 4, address(parent));
            put(parent, 0, address(replacement));
            put(successor, 8, word(removed, 8));
            auto* const current_right = pointer(word(removed, 8));
            put(current_right, 4, address(successor));
        }

        auto* const head = pointer(word(receiver, 4));
        if (word(head, 4) == address(removed)) {
            put(head, 4, address(successor));
        } else {
            auto* const current_parent = pointer(word(removed, 4));
            if (word(current_parent, 0) == address(removed))
                put(current_parent, 0, address(successor));
            else
                put(current_parent, 8, address(successor));
        }
        put(successor, 4, word(removed, 4));
        const auto removed_color = byte(removed, 0x20);
        const auto successor_color = byte(successor, 0x20);
        put_byte(successor, 0x20, removed_color);
        put_byte(removed, 0x20, successor_color);
    }

    constexpr std::uint8_t black = 1;
    auto* const color_removed = pointer(word(&original_removed, 0));
    if (byte(color_removed, 0x20) == black) {
        auto* const head = pointer(word(receiver, 4));
        if (address(replacement) != word(head, 4)) {
            for (;;) {
                if (byte(replacement, 0x20) != black)
                    break;
                auto* const captured_parent_left = pointer(word(parent, 0));
                if (replacement == captured_parent_left) {
                    auto* sibling = pointer(word(parent, 8));
                    if (byte(sibling, 0x20) == 0) {
                        put_byte(sibling, 0x20, black);
                        put_byte(parent, 0x20, 0);
                        rotate_native_input_controller_tree_left_0069c280(receiver, 0u, parent);
                        sibling = pointer(word(parent, 8));
                    }
                    bool move_up = is_nil(sibling);
                    void* near_child = nullptr;
                    if (!move_up) {
                        // 006A61C1 EDX survives both later far-child reads.
                        near_child = pointer(word(sibling, 0));
                        if (byte(near_child, 0x20) == black) {
                            auto* const far_child = pointer(word(sibling, 8));
                            move_up = byte(far_child, 0x20) == black;
                        }
                        if (move_up)
                            put_byte(sibling, 0x20, 0);
                    }
                    if (!move_up) {
                        auto* child = pointer(word(sibling, 8));
                        if (byte(child, 0x20) == black) {
                            put_byte(near_child, 0x20, black);
                            put_byte(sibling, 0x20, 0);
                            rotate_native_input_controller_tree_right_0069a6a0(receiver, 0u, sibling);
                            sibling = pointer(word(parent, 8));
                        }
                        const auto parent_color = byte(parent, 0x20);
                        put_byte(sibling, 0x20, parent_color);
                        put_byte(parent, 0x20, black);
                        child = pointer(word(sibling, 8));
                        put_byte(child, 0x20, black);
                        rotate_native_input_controller_tree_left_0069c280(receiver, 0u, parent);
                        break;
                    }
                } else {
                    auto* sibling = captured_parent_left;
                    if (byte(sibling, 0x20) == 0) {
                        put_byte(sibling, 0x20, black);
                        put_byte(parent, 0x20, 0);
                        rotate_native_input_controller_tree_right_0069a6a0(receiver, 0u, parent);
                        sibling = pointer(word(parent, 0));
                    }
                    bool move_up = is_nil(sibling);
                    void* near_child = nullptr;
                    if (!move_up) {
                        // 006A6220 EDX survives both later far-child reads.
                        near_child = pointer(word(sibling, 8));
                        if (byte(near_child, 0x20) == black) {
                            auto* const far_child = pointer(word(sibling, 0));
                            move_up = byte(far_child, 0x20) == black;
                        }
                        if (move_up)
                            put_byte(sibling, 0x20, 0);
                    }
                    if (!move_up) {
                        auto* child = pointer(word(sibling, 0));
                        if (byte(child, 0x20) == black) {
                            put_byte(near_child, 0x20, black);
                            put_byte(sibling, 0x20, 0);
                            rotate_native_input_controller_tree_left_0069c280(receiver, 0u, sibling);
                            sibling = pointer(word(parent, 0));
                        }
                        const auto parent_color = byte(parent, 0x20);
                        put_byte(sibling, 0x20, parent_color);
                        put_byte(parent, 0x20, black);
                        child = pointer(word(sibling, 0));
                        put_byte(child, 0x20, black);
                        rotate_native_input_controller_tree_right_0069a6a0(receiver, 0u, parent);
                        break;
                    }
                }

                // Native6233: root capture/comparison before parent.parent,
                // including the parent read when the new node is the root.
                auto* const current_head = pointer(word(receiver, 4));
                replacement = parent;
                const bool reached_root = address(replacement) == word(current_head, 4);
                parent = pointer(word(parent, 4));
                if (reached_root)
                    break;
            }
        }
        put_byte(replacement, 0x20, black);
    }

    auto* const payload_node = pointer(word(&original_removed, 0));
    destroy_native_input_controller_name_pair_006a2bb0(
        static_cast<std::byte*>(payload_node) + 0x0c, 0u, strings);
    // No parent payload guard: only a successful child permits this reload.
    auto* const node_to_free = pointer(word(&original_removed, 0));
    singleton_lifetime_free(node_to_free);
    const auto current_count = word(receiver, 8);
    if (current_count != 0)
        put(receiver, 8, current_count - 1u);

    const auto current_owner = word(&input, 0);
    auto* const output = static_cast<NativeInputControllerTreeIterator*>(
        pointer(word(&actual_output, 0)));
    const auto current_node = word(&input, 4);
    put(output, 0, current_owner);
    put(output, 4, current_node);
    return output;
}
} // namespace bsp
