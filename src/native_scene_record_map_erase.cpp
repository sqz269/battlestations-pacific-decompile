#include "bsp/native_scene_record_map_erase.hpp"

#include "bsp/native_hardware_layout_tree.hpp"
#include "bsp/native_scene_record_map_extrema.hpp"
#include "bsp/native_scene_record_map_increment.hpp"
#include "bsp/native_scene_record_map_rotations.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"

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
bool nil(U p) noexcept { return byte(p, 0x19) != 0; }

struct CompletedTemporary {
    NativeLegacySboStringStorage& message;
    ~CompletedTemporary() noexcept {
        //00C61C70: LEA ECX,[native frame-50h];tail004072D0.
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
    // State0 is armed AFTER assignment, before00411700. The existing
    // owning carrier constructs logic_error, publishes D6926C, copies via
    //00441760 and destroys via004412B0. C++ throw transport is qualified.
    const CompletedTemporary completed{message};
    throw NativeHardwareLayoutInvalidIterator{message};
}
} // namespace

void* erase_native_scene_record_map_iterator_0046a470(void* actual_map,
    const NativeSceneRecordMapEraseFrame& frame, NativeStringRawPoolContext& strings) {
    const U parameters = address(frame.actual_parameters12);
    const U initial_map = address(actual_map);
    const U original = word(parameters, 8);
    const bool original_nil = nil(original);
    frame.saved_map = initial_map;
    if (original_nil) throw_invalid_iterator();

    increment_native_scene_record_map_iterator_00467980(pointer(parameters + 4u));
    const U original_left = word(original);
    U replacement;
    U parent;
    U successor = 0;
    bool transplant = false;
    if (nil(original_left)) {
        replacement = word(original, 8);
    } else {
        const U right = word(original, 8);
        if (nil(right)) {
            replacement = original_left;
        } else {
            successor = word(parameters, 8);
            replacement = word(successor, 8);
            transplant = successor != original;
        }
    }

    if (!transplant) {
        const bool replacement_nil = nil(replacement);
        parent = word(original, 4);
        if (!replacement_nil) put(replacement, 4, parent);
        U head = word(initial_map, 4);
        if (word(head, 4) == original) put(head, 4, replacement);
        else if (word(parent) == original) put(parent, 0, replacement);
        else put(parent, 8, replacement);

        head = word(initial_map, 4);
        if (word(head) == original) {
            const U minimum = nil(replacement) ? parent
                : address(minimum_native_scene_record_map_node_00466a20(pointer(replacement)));
            put(head, 0, minimum);
        }
        const U current_map = frame.saved_map;
        head = word(current_map, 4);
        if (word(head, 8) == original) {
            const U maximum = nil(replacement) ? parent
                : address(maximum_native_scene_record_map_node_00466a00(pointer(replacement)));
            put(head, 8, maximum);
        }
    } else {
        put(original_left, 4, successor);
        const U reloaded_left = word(original);
        put(successor, 0, reloaded_left);
        if (successor == word(original, 8)) {
            parent = successor;
        } else {
            const bool replacement_nil = nil(replacement);
            parent = word(successor, 4);
            if (!replacement_nil) put(replacement, 4, parent);
            put(parent, 0, replacement);
            const U right = word(original, 8);
            put(successor, 8, right);
            const U reloaded_right = word(original, 8);
            put(reloaded_right, 4, successor);
        }
        const U head = word(initial_map, 4);
        if (word(head, 4) == original) {
            put(head, 4, successor);
        } else {
            const U original_parent = word(original, 4);
            if (word(original_parent) == original) put(original_parent, 0, successor);
            else put(original_parent, 8, successor);
        }
        const U original_parent = word(original, 4);
        put(successor, 4, original_parent);
        const auto original_color = byte(original, 0x18);
        const auto successor_color = byte(successor, 0x18);
        put_byte(successor, 0x18, original_color);
        put_byte(original, 0x18, successor_color);
    }

    if (byte(original, 0x18) == 1) {
        U repair_map = frame.saved_map;
        U head = word(repair_map, 4);
        bool at_root = replacement == word(head, 4);
        while (!at_root && byte(replacement, 0x18) == 1) {
            U sibling = word(parent);
            if (replacement == sibling) {
                sibling = word(parent, 8);
                if (byte(sibling, 0x18) == 0) {
                    put_byte(sibling, 0x18, 1);
                    put_byte(parent, 0x18, 0);
                    rotate_left_native_scene_record_map_00467620(pointer(repair_map), 0, pointer(parent));
                    sibling = word(parent, 8);
                    repair_map = frame.saved_map;
                }
                if (!nil(sibling)) {
                    const U left = word(sibling);
                    bool both_black = false;
                    if (byte(left, 0x18) == 1) {
                        const U right = word(sibling, 8);
                        both_black = byte(right, 0x18) == 1;
                    }
                    if (!both_black) {
                        const U right = word(sibling, 8);
                        if (byte(right, 0x18) == 1) {
                            const U near_child = word(sibling);
                            put_byte(near_child, 0x18, 1);
                            put_byte(sibling, 0x18, 0);
                            rotate_right_native_scene_record_map_00467670(pointer(repair_map), 0, pointer(sibling));
                            sibling = word(parent, 8);
                            repair_map = frame.saved_map;
                        }
                        const auto parent_color = byte(parent, 0x18);
                        put_byte(sibling, 0x18, parent_color);
                        put_byte(parent, 0x18, 1);
                        const U far_child = word(sibling, 8);
                        put_byte(far_child, 0x18, 1);
                        rotate_left_native_scene_record_map_00467620(pointer(repair_map), 0, pointer(parent));
                        break;
                    }
                    put_byte(sibling, 0x18, 0);
                }
            } else {
                if (byte(sibling, 0x18) == 0) {
                    put_byte(sibling, 0x18, 1);
                    put_byte(parent, 0x18, 0);
                    rotate_right_native_scene_record_map_00467670(pointer(repair_map), 0, pointer(parent));
                    sibling = word(parent);
                    repair_map = frame.saved_map;
                }
                if (!nil(sibling)) {
                    const U right = word(sibling, 8);
                    bool both_black = false;
                    if (byte(right, 0x18) == 1) {
                        const U left = word(sibling);
                        both_black = byte(left, 0x18) == 1;
                    }
                    if (!both_black) {
                        const U left = word(sibling);
                        if (byte(left, 0x18) == 1) {
                            const U near_child = word(sibling, 8);
                            put_byte(near_child, 0x18, 1);
                            put_byte(sibling, 0x18, 0);
                            rotate_left_native_scene_record_map_00467620(pointer(repair_map), 0, pointer(sibling));
                            sibling = word(parent);
                            repair_map = frame.saved_map;
                        }
                        const auto parent_color = byte(parent, 0x18);
                        put_byte(sibling, 0x18, parent_color);
                        put_byte(parent, 0x18, 1);
                        const U far_child = word(sibling);
                        put_byte(far_child, 0x18, 1);
                        rotate_right_native_scene_record_map_00467670(pointer(repair_map), 0, pointer(parent));
                        break;
                    }
                    put_byte(sibling, 0x18, 0);
                }
            }
            head = word(repair_map, 4);
            replacement = parent;
            at_root = replacement == word(head, 4);
            // Native reads this even when the root comparison already matched.
            parent = word(parent, 4);
        }
        put_byte(replacement, 0x18, 1);
    }

    const U name_data = word(original, 0x10);
    if (name_data != 0) {
        const U returned_size = word(original, 0x0c) + 1u;
        auto* const pool = native_string_pool_get_or_create_00419cc0(
            strings.actual_published_01090aa8, strings.actual_manager_publication_01090aa0);
        return_native_string_pool_00bd1510(pool, pointer(name_data), returned_size,
            strings.actual_small_returns_disabled_01090aa4);
    }
    singleton_lifetime_free(pointer(original));
    const U count_map = frame.saved_map;
    const U count = word(count_map, 8);
    if (count != 0) put(count_map, 8, count - 1u);

    const U owner = word(parameters, 4);
    const U output = word(parameters);
    const U node = word(parameters, 8);
    put(output, 0, owner);
    put(output, 4, node);
    return pointer(output);
}
} // namespace bsp
