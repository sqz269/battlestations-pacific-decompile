#include "bsp/native_input_preset_tree_hint_insert.hpp"

#include "bsp/native_input_preset_iterator_equal.hpp"
#include "bsp/native_input_preset_tree_insert.hpp"
#include "bsp/native_input_preset_tree_leaves.hpp"
#include "bsp/native_input_preset_tree_predecessor.hpp"
#include "bsp/native_input_preset_tree_unique_insert.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include "bsp/native_string_header_less_actual.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(Word) == 4);

Word address(const void* value) {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(value));
}

void* pointer(Word value) {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(value));
}

void* at(const void* base, Word byte_offset) {
    return pointer(address(base) + byte_offset);
}

Word word(const void* base, Word byte_offset) {
    return *reinterpret_cast<const volatile Word*>(
        static_cast<std::uintptr_t>(address(base) + byte_offset));
}

void put(void* base, Word byte_offset, Word value) {
    *reinterpret_cast<volatile Word*>(
        static_cast<std::uintptr_t>(address(base) + byte_offset)) = value;
}

bool is_nil(const void* node) {
    return *reinterpret_cast<const volatile std::uint8_t*>(
        static_cast<std::uintptr_t>(address(node) + 0x29u)) != 0;
}
} // namespace

NativeInputPresetTreeIterator* __fastcall
insert_hint_native_input_preset_tree_006a15e0(
    void* actual_tree, std::uint32_t,
    NativeInputPresetTreeIterator* actual_output,
    NativeInputPresetTreeIterator actual_hint, const void* actual_pair,
    NativeStringRawPoolContext& strings,
    const volatile std::uint32_t& actual_feature_word_0109eea4,
    const volatile std::int32_t& actual_locale_changed_0109de1c,
    NativeCrtStricmpCallees& actual_callees) {
    auto* const tree = actual_tree;
    if (word(tree, 8) == 0) {
        const void* const pair = static_cast<const void* volatile&>(actual_pair);
        auto* const head = pointer(word(tree, 4));
        auto* const output = static_cast<NativeInputPresetTreeIterator* volatile&>(actual_output);
        insert_native_input_preset_tree_node_0069fbc0(tree, 0u, output,
            1u, head, pair, strings, actual_feature_word_0109eea4);
        return output;
    }

    auto* const initial_head = pointer(word(tree, 4));
    auto* const initial_left = pointer(word(initial_head, 0));
    auto* const original_owner = pointer(word(&actual_hint, 0));
    if (original_owner == nullptr || original_owner != tree)
        invoke_native_invalid_parameter_00bf6713();
    auto* const original_node = pointer(word(&actual_hint, 4));

    // Same real 12B storage serves as private end iterator and unique result.
    // There is no value initialization, flag/padding read, or preimage argument.
    NativeInputPresetTreeInsertResult private_result;
    const void* pair;
    if (original_node == initial_left) {
        pair = static_cast<const void* volatile&>(actual_pair);
        if (less_native_string_headers_with_callees_00443d00(pair,
                at(original_node, 0xcu), actual_locale_changed_0109de1c, actual_callees)) {
            auto* const output = static_cast<NativeInputPresetTreeIterator* volatile&>(actual_output);
            insert_native_input_preset_tree_node_0069fbc0(tree, 0u, output,
                1u, original_node, pair, strings, actual_feature_word_0109eea4);
            return output;
        }
    } else {
        auto* const second_head = pointer(word(tree, 4));
        if (original_owner == nullptr || original_owner != tree)
            invoke_native_invalid_parameter_00bf6713();
        // Native CMP precedes the private pair-argument reload. This private
        // volatile predicate adds Source spill/flags/frame qualifications only.
        const volatile bool at_end = original_node == second_head;
        pair = static_cast<const void* volatile&>(actual_pair);
        if (at_end) {
            auto* const head_for_compare = pointer(word(tree, 4));
            auto* const right_for_compare = pointer(word(head_for_compare, 8));
            if (less_native_string_headers_with_callees_00443d00(
                    at(right_for_compare, 0xcu), pair,
                    actual_locale_changed_0109de1c, actual_callees)) {
                auto* const current_head = pointer(word(tree, 4));
                auto* const current_right = pointer(word(current_head, 8));
                auto* const output = static_cast<NativeInputPresetTreeIterator* volatile&>(actual_output);
                insert_native_input_preset_tree_node_0069fbc0(tree, 0u, output,
                    0u, current_right, pair, strings, actual_feature_word_0109eea4);
                return output;
            }
        } else {
            if (less_native_string_headers_with_callees_00443d00(pair,
                    at(original_node, 0xcu), actual_locale_changed_0109de1c, actual_callees)) {
                put(&actual_hint, 0, address(original_owner));
                put(&actual_hint, 4, address(original_node));
                decrement_native_input_preset_tree_iterator_0069aec0(&actual_hint, 0u);
                auto* const predecessor_for_compare = pointer(word(&actual_hint, 4));
                if (less_native_string_headers_with_callees_00443d00(
                        at(predecessor_for_compare, 0xcu), pair,
                        actual_locale_changed_0109de1c, actual_callees)) {
                    auto* const current_predecessor = pointer(word(&actual_hint, 4));
                    auto* const predecessor_right = pointer(word(current_predecessor, 8));
                    if (is_nil(predecessor_right)) {
                        auto* const output = static_cast<NativeInputPresetTreeIterator* volatile&>(actual_output);
                        insert_native_input_preset_tree_node_0069fbc0(tree, 0u, output,
                            0u, current_predecessor, pair, strings, actual_feature_word_0109eea4);
                        return output;
                    }
                    auto* const output = static_cast<NativeInputPresetTreeIterator* volatile&>(actual_output);
                    insert_native_input_preset_tree_node_0069fbc0(tree, 0u, output,
                        1u, original_node, pair, strings, actual_feature_word_0109eea4);
                    return output;
                }
                // Failed predecessor ordering continues to the greater test.
            }
            if (less_native_string_headers_with_callees_00443d00(
                    at(original_node, 0xcu), pair,
                    actual_locale_changed_0109de1c, actual_callees)) {
                auto* const captured_head = pointer(word(tree, 4));
                put(&actual_hint, 0, address(original_owner));
                put(&actual_hint, 4, address(original_node));
                put(&private_result.iterator_00, 4, address(captured_head));
                put(&private_result.iterator_00, 0, address(tree));
                increment_native_input_preset_tree_iterator_0069c0f0(&actual_hint, 0u);
                const bool equals_end = equal_native_input_preset_tree_iterators_00699ef0(
                    &actual_hint, 0u, &private_result.iterator_00);
                auto* const saved_successor = pointer(word(&actual_hint, 4));
                if (equals_end || less_native_string_headers_with_callees_00443d00(
                        pair, at(saved_successor, 0xcu),
                        actual_locale_changed_0109de1c, actual_callees)) {
                    auto* const original_right = pointer(word(original_node, 8));
                    if (is_nil(original_right)) {
                        auto* const output = static_cast<NativeInputPresetTreeIterator* volatile&>(actual_output);
                        insert_native_input_preset_tree_node_0069fbc0(tree, 0u, output,
                            0u, original_node, pair, strings, actual_feature_word_0109eea4);
                        return output;
                    }
                    auto* const output = static_cast<NativeInputPresetTreeIterator* volatile&>(actual_output);
                    insert_native_input_preset_tree_node_0069fbc0(tree, 0u, output,
                        1u, saved_successor, pair, strings, actual_feature_word_0109eea4);
                    return output;
                }
            }
        }
    }

    auto* const child = insert_unique_native_input_preset_tree_006a1050(
        tree, 0u, &private_result, pair, strings, actual_feature_word_0109eea4,
        actual_locale_changed_0109de1c, actual_callees);
    const auto returned_owner = word(child, 0);
    auto* const output = static_cast<NativeInputPresetTreeIterator* volatile&>(actual_output);
    put(output, 0, returned_owner);
    const auto returned_node = word(child, 4);
    put(output, 4, returned_node);
    return output;
}
} // namespace bsp
