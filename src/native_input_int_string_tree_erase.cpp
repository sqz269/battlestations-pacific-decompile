#include "bsp/native_input_int_string_tree_erase.hpp"
#include "bsp/native_input_int_string_tree_leaves.hpp"
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
    return byte(node, 0x19) != 0;
}

struct CompletedMessage {
    NativeLegacySboStringStorage& storage;
    ~CompletedMessage() noexcept {
        cleanup_native_input_int_string_erase_message_00c7f050(storage);
    }
};

[[noreturn]] __declspec(noinline) void throw_invalid_iterator() {
    NativeLegacySboStringStorage message;
    message.capacity_18 = 15;
    message.length_14 = 0;
    message.buffer_04.inline_bytes[0] = '\0';
    native_legacy_sbo_string_assign_counted_00408720(
        message, "invalid map/set<T> iterator", 27);
    // Original006A0826 arms state0 only after counted assignment succeeds.
    const CompletedMessage completed{message};
    throw NativeHardwareLayoutInvalidIterator{message};
}
} // namespace

void __fastcall cleanup_native_input_int_string_erase_message_00c7f050(
    NativeLegacySboStringStorage& completed_message) noexcept {
    native_legacy_sbo_string_destroy_004072d0(completed_message);
}

NativeInputIntStringTreeIterator* __fastcall
erase_native_input_int_string_tree_iterator_006a07d0(
    void* actual_tree, std::uint32_t unused_edx,
    NativeInputIntStringTreeIterator* actual_output,
    NativeInputIntStringTreeIterator input,
    NativeStringRawPoolContext& strings) {
    static_cast<void>(unused_edx);
    auto* const removed = pointer(word(&input, 4));
    if (is_nil(removed))
        throw_invalid_iterator();

    auto* receiver = actual_tree;
    increment_native_input_int_string_tree_iterator_0069ae50(&input, 0u);
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
        // 006A0879: nil test precedes the current removed.parent capture.
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
        if (word(head, 0) == address(removed)) {
            const auto selected = is_nil(replacement)
                ? address(parent)
                : address(minimum_native_input_int_string_node_0069aae0(replacement));
            put(head, 0, selected);
        }
        receiver = pointer(word(&actual_tree, 0));
        head = pointer(word(receiver, 4));
        if (word(head, 8) == address(removed)) {
            const auto selected = is_nil(replacement)
                ? address(parent)
                : address(maximum_native_input_int_string_node_0069aac0(replacement));
            put(head, 8, selected);
        }
    } else {
        // 006A08DC: retained first left capture, followed by CURRENT rereads.
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
        const auto removed_color = byte(removed, 0x18);
        const auto successor_color = byte(successor, 0x18);
        put_byte(successor, 0x18, removed_color);
        put_byte(removed, 0x18, successor_color);
    }

    constexpr std::uint8_t black = 1;
    if (byte(removed, 0x18) == black) {
        receiver = pointer(word(&actual_tree, 0));
        auto* const head = pointer(word(receiver, 4));
        if (address(replacement) != word(head, 4)) {
            for (;;) {
                // Subsequent iterations enter006A0952 after the move-up's
                // root comparison and parent read; do not repeat that test.
                if (byte(replacement, 0x18) != black)
                    break;
                auto* const captured_parent_left = pointer(word(parent, 0));
                if (replacement == captured_parent_left) {
                    auto* sibling = pointer(word(parent, 8));
                    if (byte(sibling, 0x18) == 0) {
                        put_byte(sibling, 0x18, black);
                        put_byte(parent, 0x18, 0);
                        rotate_native_input_int_string_tree_left_0069c300(receiver, 0u, parent);
                        sibling = pointer(word(parent, 8));
                        receiver = pointer(word(&actual_tree, 0));
                    }
                    bool move_up = is_nil(sibling);
                    if (!move_up) {
                        auto* child = pointer(word(sibling, 0));
                        if (byte(child, 0x18) == black) {
                            child = pointer(word(sibling, 8));
                            move_up = byte(child, 0x18) == black;
                        }
                        if (move_up)
                            put_byte(sibling, 0x18, 0);
                    }
                    if (!move_up) {
                        auto* child = pointer(word(sibling, 8));
                        if (byte(child, 0x18) == black) {
                            child = pointer(word(sibling, 0));
                            put_byte(child, 0x18, black);
                            put_byte(sibling, 0x18, 0);
                            rotate_native_input_int_string_tree_right_0069a780(receiver, 0u, sibling);
                            sibling = pointer(word(parent, 8));
                            receiver = pointer(word(&actual_tree, 0));
                        }
                        const auto parent_color = byte(parent, 0x18);
                        put_byte(sibling, 0x18, parent_color);
                        put_byte(parent, 0x18, black);
                        child = pointer(word(sibling, 8));
                        put_byte(child, 0x18, black);
                        rotate_native_input_int_string_tree_left_0069c300(receiver, 0u, parent);
                        break;
                    }
                } else {
                    auto* sibling = captured_parent_left;
                    if (byte(sibling, 0x18) == 0) {
                        put_byte(sibling, 0x18, black);
                        put_byte(parent, 0x18, 0);
                        rotate_native_input_int_string_tree_right_0069a780(receiver, 0u, parent);
                        sibling = pointer(word(parent, 0));
                        receiver = pointer(word(&actual_tree, 0));
                    }
                    bool move_up = is_nil(sibling);
                    if (!move_up) {
                        auto* child = pointer(word(sibling, 8));
                        if (byte(child, 0x18) == black) {
                            child = pointer(word(sibling, 0));
                            move_up = byte(child, 0x18) == black;
                        }
                        if (move_up)
                            put_byte(sibling, 0x18, 0);
                    }
                    if (!move_up) {
                        auto* child = pointer(word(sibling, 0));
                        if (byte(child, 0x18) == black) {
                            child = pointer(word(sibling, 8));
                            put_byte(child, 0x18, black);
                            put_byte(sibling, 0x18, 0);
                            rotate_native_input_int_string_tree_left_0069c300(receiver, 0u, sibling);
                            sibling = pointer(word(parent, 0));
                            receiver = pointer(word(&actual_tree, 0));
                        }
                        const auto parent_color = byte(parent, 0x18);
                        put_byte(sibling, 0x18, parent_color);
                        put_byte(parent, 0x18, black);
                        child = pointer(word(sibling, 0));
                        put_byte(child, 0x18, black);
                        rotate_native_input_int_string_tree_right_0069a780(receiver, 0u, parent);
                        break;
                    }
                }

                // 006A09FA: head/root capture, replacement=old parent, root
                // comparison, THEN current parent.parent read even at root.
                auto* const current_head = pointer(word(receiver, 4));
                replacement = parent;
                const bool reached_root = address(replacement) == word(current_head, 4);
                parent = pointer(word(parent, 4));
                if (reached_root)
                    break;
            }
        }
        put_byte(replacement, 0x18, black);
    }

    // The native inline006A0A41..5B has the exact RawCtx41DD20 read/release
    // schedule here. No payload guard is armed while the getter can throw.
    destroy_native_string_header_0041dd20(
        static_cast<std::byte*>(removed) + 0x10, strings);
    singleton_lifetime_free(removed);
    receiver = pointer(word(&actual_tree, 0));
    const auto current_count = word(receiver, 8);
    if (current_count != 0)
        put(receiver, 8, current_count - 1u);

    const auto current_owner = word(&input, 0);
    auto* const output = static_cast<NativeInputIntStringTreeIterator*>(
        pointer(word(&actual_output, 0)));
    const auto current_node = word(&input, 4);
    put(output, 0, current_owner);
    put(output, 4, current_node);
    return output;
}
} // namespace bsp
