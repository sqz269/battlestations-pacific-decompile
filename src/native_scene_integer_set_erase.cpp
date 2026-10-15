#include "bsp/native_scene_integer_set_erase.hpp"

#include "bsp/native_hardware_layout_tree.hpp"
#include "bsp/native_scene_integer_set_extrema.hpp"
#include "bsp/native_scene_integer_set_increment.hpp"
#include "bsp/native_scene_integer_set_rotations.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstdint>

namespace bsp {
namespace {
using U = std::uint32_t;
static_assert(sizeof(void*) == 4);
U address(const void* p) noexcept { return reinterpret_cast<std::uintptr_t>(p); }
void* pointer(U p) noexcept { return reinterpret_cast<void*>(p); }
U word(U p, U offset = 0) noexcept {
    return *reinterpret_cast<const volatile U*>(p + offset);
}
void put(U p, U offset, U value) noexcept {
    *reinterpret_cast<volatile U*>(p + offset) = value;
}
unsigned char byte(U p, U offset) noexcept {
    return *reinterpret_cast<const volatile unsigned char*>(p + offset);
}
void put_byte(U p, U offset, unsigned char value) noexcept {
    *reinterpret_cast<volatile unsigned char*>(p + offset) = value;
}
bool nil(U p) noexcept { return byte(p, 0x11) != 0; }
struct CompletedMessage {
    NativeLegacySboStringStorage& message;
    ~CompletedMessage() noexcept {
        //00C61C90: completed temporary at native frame-50h.
        native_legacy_sbo_string_destroy_004072d0(message);
    }
};
[[noreturn]] void throw_invalid_iterator() {
    NativeLegacySboStringStorage message;
    message.capacity_18 = 15;
    message.length_14 = 0;
    message.buffer_04.inline_bytes[0] = 0;
    native_legacy_sbo_string_assign_counted_00408720(
        message, "invalid map/set<T> iterator", 27);
    const CompletedMessage completed{message}; // State0 only after assignment.
    throw NativeHardwareLayoutInvalidIterator{message};
}
} // namespace

void* erase_native_scene_integer_set_iterator_0046a740(void* actual_tree,
    const NativeSceneIntegerSetEraseFrame& frame) {
    const U parameters = address(frame.actual_parameters12);
    const U tree = address(actual_tree);
    const U original = word(parameters, 8);
    if (nil(original)) throw_invalid_iterator();
    const U saved_node = address(frame.actual_saved_node4);
    put(saved_node, 0, original);
    increment_native_scene_integer_set_iterator_00466ae0(pointer(parameters + 4u));

    const U original_left = word(original);
    U replacement;
    U parent;
    U successor = 0;
    bool transplant = false;
    if (nil(original_left)) {
        replacement = word(original, 8);
    } else {
        const U right = word(original, 8);
        if (nil(right)) replacement = original_left;
        else {
            successor = word(parameters, 8);
            replacement = word(successor, 8);
            transplant = successor != original;
        }
    }
    if (!transplant) {
        const bool replacement_nil = nil(replacement);
        parent = word(original, 4);
        if (!replacement_nil) put(replacement, 4, parent);
        U head = word(tree, 4);
        if (word(head, 4) == original) put(head, 4, replacement);
        else if (word(parent) == original) put(parent, 0, replacement);
        else put(parent, 8, replacement);

        head = word(tree, 4);
        const U current_minimum = word(head);
        const U saved_for_minimum = word(saved_node);
        if (current_minimum == saved_for_minimum) {
            const U minimum = nil(replacement) ? parent
                : address(minimum_native_scene_integer_set_node_00466a60(pointer(replacement)));
            put(head, 0, minimum);
        }
        head = word(tree, 4);
        const U saved_for_maximum = word(saved_node);
        if (word(head, 8) == saved_for_maximum) {
            const U maximum = nil(replacement) ? parent
                : address(maximum_native_scene_integer_set_node_00466a40(pointer(replacement)));
            put(head, 8, maximum);
        }
    } else {
        put(original_left, 4, successor);
        const U reloaded_left = word(original);
        put(successor, 0, reloaded_left);
        if (successor == word(original, 8)) parent = successor;
        else {
            const bool replacement_nil = nil(replacement);
            parent = word(successor, 4);
            if (!replacement_nil) put(replacement, 4, parent);
            put(parent, 0, replacement);
            const U right = word(original, 8);
            put(successor, 8, right);
            const U reloaded_right = word(original, 8);
            put(reloaded_right, 4, successor);
        }
        const U head = word(tree, 4);
        if (word(head, 4) == original) put(head, 4, successor);
        else {
            const U original_parent = word(original, 4);
            if (word(original_parent) == original) put(original_parent, 0, successor);
            else put(original_parent, 8, successor);
        }
        const U original_parent = word(original, 4);
        put(successor, 4, original_parent);
        const auto original_color = byte(original, 0x10);
        const auto successor_color = byte(successor, 0x10);
        put_byte(successor, 0x10, original_color);
        put_byte(original, 0x10, successor_color);
    }

    const U color_target = word(saved_node);
    if (byte(color_target, 0x10) == 1) {
        U head = word(tree, 4);
        bool at_root = replacement == word(head, 4);
        while (!at_root && byte(replacement, 0x10) == 1) {
            U sibling = word(parent);
            if (replacement == sibling) {
                sibling = word(parent, 8);
                if (byte(sibling, 0x10) == 0) {
                    put_byte(sibling, 0x10, 1);
                    put_byte(parent, 0x10, 0);
                    rotate_left_native_scene_integer_set_00467740(pointer(tree), 0, pointer(parent));
                    sibling = word(parent, 8);
                }
                if (!nil(sibling)) {
                    // Native EDX retains this near child across far-child reads.
                    const U near_child = word(sibling);
                    bool both_black = false;
                    if (byte(near_child, 0x10) == 1) {
                        const U far_child = word(sibling, 8);
                        both_black = byte(far_child, 0x10) == 1;
                    }
                    if (!both_black) {
                        const U far_child = word(sibling, 8);
                        if (byte(far_child, 0x10) == 1) {
                            put_byte(near_child, 0x10, 1);
                            put_byte(sibling, 0x10, 0);
                            rotate_right_native_scene_integer_set_00467790(pointer(tree), 0, pointer(sibling));
                            sibling = word(parent, 8);
                        }
                        const auto parent_color = byte(parent, 0x10);
                        put_byte(sibling, 0x10, parent_color);
                        put_byte(parent, 0x10, 1);
                        const U current_far_child = word(sibling, 8);
                        put_byte(current_far_child, 0x10, 1);
                        rotate_left_native_scene_integer_set_00467740(pointer(tree), 0, pointer(parent));
                        break;
                    }
                    put_byte(sibling, 0x10, 0);
                }
            } else {
                if (byte(sibling, 0x10) == 0) {
                    put_byte(sibling, 0x10, 1);
                    put_byte(parent, 0x10, 0);
                    rotate_right_native_scene_integer_set_00467790(pointer(tree), 0, pointer(parent));
                    sibling = word(parent);
                }
                if (!nil(sibling)) {
                    const U near_child = word(sibling, 8);
                    bool both_black = false;
                    if (byte(near_child, 0x10) == 1) {
                        const U far_child = word(sibling);
                        both_black = byte(far_child, 0x10) == 1;
                    }
                    if (!both_black) {
                        const U far_child = word(sibling);
                        if (byte(far_child, 0x10) == 1) {
                            put_byte(near_child, 0x10, 1);
                            put_byte(sibling, 0x10, 0);
                            rotate_left_native_scene_integer_set_00467740(pointer(tree), 0, pointer(sibling));
                            sibling = word(parent);
                        }
                        const auto parent_color = byte(parent, 0x10);
                        put_byte(sibling, 0x10, parent_color);
                        put_byte(parent, 0x10, 1);
                        const U current_far_child = word(sibling);
                        put_byte(current_far_child, 0x10, 1);
                        rotate_right_native_scene_integer_set_00467790(pointer(tree), 0, pointer(parent));
                        break;
                    }
                    put_byte(sibling, 0x10, 0);
                }
            }
            head = word(tree, 4);
            replacement = parent;
            at_root = replacement == word(head, 4);
            parent = word(parent, 4); // Native still reads this when root matched.
        }
        put_byte(replacement, 0x10, 1);
    }

    // This is a fresh saved-cell read,not the captured original/color target.
    const U freed_node = word(saved_node);
    singleton_lifetime_free(pointer(freed_node));
    const U count = word(tree, 8);
    if (count != 0) put(tree, 8, count - 1u);
    const U owner = word(parameters, 4);
    const U output = word(parameters);
    const U node = word(parameters, 8);
    put(output, 0, owner);
    put(output, 4, node);
    return pointer(output);
}
} // namespace bsp
