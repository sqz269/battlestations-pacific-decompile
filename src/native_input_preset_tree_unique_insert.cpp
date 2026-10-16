#include "bsp/native_input_preset_tree_unique_insert.hpp"
#include "bsp/native_crt_stricmp.hpp"
#include "bsp/native_input_preset_tree_insert.hpp"
#include "bsp/native_input_preset_tree_predecessor.hpp"
#include "bsp/native_string_header_less_actual.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(Word) == 4);

Word word(const void* base, std::size_t offset) {
    return *reinterpret_cast<const volatile Word*>(
        static_cast<const std::byte*>(base) + offset);
}
void* pointer(Word value) {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(value));
}
const void* at(const void* base, std::size_t offset) {
    return pointer(static_cast<Word>(reinterpret_cast<std::uintptr_t>(base))
        + static_cast<Word>(offset));
}
bool nil(const void* node) {
    return *reinterpret_cast<const volatile std::uint8_t*>(
        static_cast<const std::byte*>(node) + 0x29) != 0;
}
void put(void* base, std::size_t offset, const void* value) {
    *reinterpret_cast<volatile Word*>(
        static_cast<std::byte*>(base) + offset) =
        static_cast<Word>(reinterpret_cast<std::uintptr_t>(value));
}
void put_flag(void* base, std::uint8_t value) {
    *reinterpret_cast<volatile std::uint8_t*>(
        static_cast<std::byte*>(base) + 8) = value;
}
} // namespace

NativeInputPresetTreeInsertResult* __fastcall
insert_unique_native_input_preset_tree_006a1050(
    void* actual_tree, std::uint32_t unused_edx,
    NativeInputPresetTreeInsertResult* actual_output, const void* actual_pair,
    NativeStringRawPoolContext& strings,
    const volatile std::uint32_t& actual_feature_word_0109eea4,
    const volatile std::int32_t& actual_locale_changed_0109de1c,
    NativeCrtStricmpCallees& actual_callees) {
    static_cast<void>(unused_edx);
    void* const tree = actual_tree;
    const void* const pair = static_cast<const void* volatile&>(actual_pair);
    void* const head = pointer(word(tree, 4));
    void* current = pointer(word(head, 4));
    bool current_is_nil = nil(current);
    void* candidate = head;
    bool insert_left = true;
    while (!current_is_nil) {
        const Word key_length = word(pair, 0);
        candidate = current;
        std::int32_t comparison;
        if (key_length == 0) {
            comparison = word(current, 0x0c) != 0 ? -1 : 0;
        } else if (word(current, 0x0c) == 0) {
            comparison = 1;
        } else {
            const auto* const node_data = static_cast<const char*>(pointer(word(current, 0x10)));
            const auto* const key_data = static_cast<const char*>(pointer(word(pair, 4)));
            comparison = compare_native_crt_strings_00bf7fbf(key_data, node_data,
                actual_locale_changed_0109de1c, actual_callees);
        }
        insert_left = comparison < 0;
        if (insert_left) {
            current = pointer(word(current, 0));
        } else {
            current = pointer(word(current, 8));
        }
        current_is_nil = nil(current);
    }

    void* selected = candidate;
    NativeInputPresetTreeIterator position;
    static_cast<void* volatile&>(position.node) = selected;
    static_cast<void* volatile&>(position.owner) = tree;
    if (insert_left) {
        void* const current_head = pointer(word(tree, 4));
        if (candidate == pointer(word(current_head, 0))) {
            auto* const child_output = insert_native_input_preset_tree_node_0069fbc0(
                tree, 0, &position, 1, candidate, pair, strings,
                actual_feature_word_0109eea4);
            void* const owner = pointer(word(child_output, 0));
            auto* const output = static_cast<NativeInputPresetTreeInsertResult* volatile&>(actual_output);
            void* const node = pointer(word(child_output, 4));
            put(output, 4, node);
            put_flag(output, 1);
            put(output, 0, owner);
            return output;
        }
        decrement_native_input_preset_tree_iterator_0069aec0(&position, 0);
        selected = pointer(word(&position, 4));
    }

    if (less_native_string_headers_with_callees_00443d00(at(selected, 0x0c), pair,
            actual_locale_changed_0109de1c, actual_callees)) {
        auto* const child_output = insert_native_input_preset_tree_node_0069fbc0(
            tree, 0, &position, static_cast<Word>(insert_left), candidate, pair,
            strings, actual_feature_word_0109eea4);
        void* const owner = pointer(word(child_output, 0));
        auto* const output = static_cast<NativeInputPresetTreeInsertResult* volatile&>(actual_output);
        void* const node = pointer(word(child_output, 4));
        put(output, 4, node);
        put_flag(output, 1);
        put(output, 0, owner);
        return output;
    }

    auto* const output = static_cast<NativeInputPresetTreeInsertResult* volatile&>(actual_output);
    void* const owner = pointer(word(&position, 0));
    put(output, 4, selected);
    put_flag(output, 0);
    put(output, 0, owner);
    return output;
}
} // namespace bsp
