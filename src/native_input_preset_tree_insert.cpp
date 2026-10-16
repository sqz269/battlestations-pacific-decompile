#include "bsp/native_input_preset_tree_insert.hpp"

#include "bsp/native_hardware_layout_tree_insert.hpp"
#include "bsp/native_input_preset_node_factory.hpp"
#include "bsp/native_input_preset_tree_leaves.hpp"
#include "bsp/native_legacy_sbo_string.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp {
namespace {
using Word = std::uint32_t;

Word address(const void* value) {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(value));
}

void* pointer(Word value) {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(value));
}

void* at(void* base, Word offset) {
    return pointer(address(base) + offset);
}

Word word(const void* base, Word offset) {
    return *reinterpret_cast<const volatile Word*>(
        static_cast<std::uintptr_t>(address(base) + offset));
}

void put(void* base, Word offset, Word value) {
    *reinterpret_cast<volatile Word*>(
        static_cast<std::uintptr_t>(address(base) + offset)) = value;
}

std::uint8_t color(const void* node) {
    return *reinterpret_cast<const volatile std::uint8_t*>(
        static_cast<std::uintptr_t>(address(node) + 0x28u));
}

void put_color(void* node, std::uint8_t value) {
    *reinterpret_cast<volatile std::uint8_t*>(
        static_cast<std::uintptr_t>(address(node) + 0x28u)) = value;
}

bool is_nil(const void* node) {
    return *reinterpret_cast<const volatile std::uint8_t*>(
        static_cast<std::uintptr_t>(address(node) + 0x29u)) != 0;
}

struct CompletedLengthMessage {
    NativeLegacySboStringStorage& message;
    ~CompletedLengthMessage() noexcept {
        cleanup_native_input_preset_insert_message_00c7efe0(message);
    }
};

[[noreturn]] void throw_length_error() {
    NativeLegacySboStringStorage message;
    message.capacity_18 = 15;
    message.length_14 = 0;
    message.buffer_04.inline_bytes[0] = '\0';
    native_legacy_sbo_string_assign_counted_00408720(
        message, "map/set<T> too long", 19);
    const CompletedLengthMessage completed{message};
    throw NativeHardwareLayoutTreeLengthError{message};
}
} // namespace

void __fastcall cleanup_native_input_preset_insert_message_00c7efe0(
    NativeLegacySboStringStorage& completed_message) noexcept {
    native_legacy_sbo_string_destroy_004072d0(completed_message);
}

NativeInputPresetTreeIterator* __fastcall
insert_native_input_preset_tree_node_0069fbc0(
    void* actual_tree, std::uint32_t,
    NativeInputPresetTreeIterator* actual_output,
    std::uint32_t insert_left_word, void* actual_parent,
    const void* actual_pair, NativeStringRawPoolContext& strings,
    const volatile std::uint32_t& actual_feature_word_0109eea4) {
    auto* const tree = actual_tree;
    if (word(tree, 8) >= 0x09249248u)
        throw_length_error();

    const void* const pair = static_cast<const void* volatile&>(actual_pair);
    auto* const initial_head = pointer(word(tree, 4));
    void* const parent = static_cast<void* volatile&>(actual_parent);
    auto* const original_node = buy_native_input_preset_node_0069f5f0(
        tree, address(pair), address(initial_head), address(parent),
        address(initial_head), pair, 0u, strings, actual_feature_word_0109eea4);

    auto* const head = pointer(word(tree, 4));
    put(tree, 8, word(tree, 8) + 1u);
    if (parent == head) {
        put(head, 4, address(original_node));
        auto* const head_b = pointer(word(tree, 4));
        put(head_b, 0, address(original_node));
        auto* const head_c = pointer(word(tree, 4));
        put(head_c, 8, address(original_node));
    } else if (*reinterpret_cast<const volatile std::uint8_t*>(&insert_left_word) != 0) {
        put(parent, 0, address(original_node));
        auto* const current_head = pointer(word(tree, 4));
        if (address(parent) == word(current_head, 0))
            put(current_head, 0, address(original_node));
    } else {
        put(parent, 8, address(original_node));
        auto* const current_head = pointer(word(tree, 4));
        if (address(parent) == word(current_head, 8))
            put(current_head, 8, address(original_node));
    }

    auto* const first_parent = pointer(word(original_node, 4));
    auto* current = original_node;
    auto* parent_slot = at(original_node, 4);
    if (color(first_parent) == 0) {
        for (;;) {
            auto* const current_parent = pointer(word(parent_slot, 0));
            auto* const grandparent = pointer(word(current_parent, 4));
            if (address(current_parent) == word(grandparent, 0)) {
                auto* const uncle = pointer(word(grandparent, 8));
                if (color(uncle) == 0) {
                    put_color(current_parent, 1);
                    put_color(uncle, 1);
                    auto* const parent_after_colors = pointer(word(parent_slot, 0));
                    auto* const grand_after_colors = pointer(word(parent_after_colors, 4));
                    put_color(grand_after_colors, 0);
                    auto* const reloaded_parent = pointer(word(parent_slot, 0));
                    current = pointer(word(reloaded_parent, 4));
                } else {
                    if (address(current) == word(current_parent, 8)) {
                        current = current_parent;
                        rotate_native_input_preset_tree_left_0069c1c0(tree, 0u, current);
                    }
                    auto* const black_parent = pointer(word(current, 4));
                    put_color(black_parent, 1);
                    auto* const reloaded_parent = pointer(word(current, 4));
                    auto* const red_grandparent = pointer(word(reloaded_parent, 4));
                    put_color(red_grandparent, 0);
                    auto* const rotation_parent = pointer(word(current, 4));
                    auto* const rotation_grandparent = pointer(word(rotation_parent, 4));
                    rotate_native_input_preset_tree_right_0069a560(tree, 0u, rotation_grandparent);
                }
            } else {
                auto* const uncle = pointer(word(grandparent, 0));
                if (color(uncle) == 0) {
                    put_color(current_parent, 1);
                    put_color(uncle, 1);
                    auto* const parent_after_colors = pointer(word(parent_slot, 0));
                    auto* const grand_after_colors = pointer(word(parent_after_colors, 4));
                    put_color(grand_after_colors, 0);
                    auto* const reloaded_parent = pointer(word(parent_slot, 0));
                    current = pointer(word(reloaded_parent, 4));
                } else {
                    if (address(current) == word(current_parent, 0)) {
                        current = current_parent;
                        rotate_native_input_preset_tree_right_0069a560(tree, 0u, current);
                    }
                    auto* const black_parent = pointer(word(current, 4));
                    put_color(black_parent, 1);
                    auto* const reloaded_parent = pointer(word(current, 4));
                    auto* const red_grandparent = pointer(word(reloaded_parent, 4));
                    put_color(red_grandparent, 0);
                    auto* const rotation_parent = pointer(word(current, 4));
                    auto* const rotation_grandparent = pointer(word(rotation_parent, 4));

                    auto* const pivot = pointer(word(rotation_grandparent, 8));
                    put(rotation_grandparent, 8, word(pivot, 0));
                    auto* const current_child = pointer(word(pivot, 0));
                    if (!is_nil(current_child))
                        put(current_child, 4, address(rotation_grandparent));
                    put(pivot, 4, word(rotation_grandparent, 4));
                    auto* const rotation_head = pointer(word(tree, 4));
                    if (address(rotation_grandparent) == word(rotation_head, 4)) {
                        put(rotation_head, 4, address(pivot));
                    } else {
                        auto* const current_grandparent_parent = pointer(word(rotation_grandparent, 4));
                        if (address(rotation_grandparent) == word(current_grandparent_parent, 0))
                            put(current_grandparent_parent, 0, address(pivot));
                        else
                            put(current_grandparent_parent, 8, address(pivot));
                    }
                    put(pivot, 0, address(rotation_grandparent));
                    put(rotation_grandparent, 4, address(pivot));
                }
            }

            auto* const next_parent = pointer(word(current, 4));
            const bool parent_is_red = color(next_parent) == 0;
            parent_slot = at(current, 4);
            if (!parent_is_red)
                break;
        }
    }

    auto* const current_head = pointer(word(tree, 4));
    auto* const current_root = pointer(word(current_head, 4));
    put_color(current_root, 1);
    auto* const output = static_cast<NativeInputPresetTreeIterator* volatile&>(actual_output);
    put(output, 4, address(original_node));
    put(output, 0, address(tree));
    return output;
}
} // namespace bsp
