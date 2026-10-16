#include "bsp/native_input_controller_tree_range.hpp"
#include "bsp/native_input_controller_subtree.hpp"
#include "bsp/native_input_controller_tree_leaves.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"

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

Word address(const void* value) {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(value));
}

void* pointer(Word bits) {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(bits));
}
} // namespace

NativeInputControllerTreeIterator* __fastcall
erase_native_input_controller_tree_range_006a6a20(
    void* actual_tree, std::uint32_t unused_edx,
    NativeInputControllerTreeIterator* actual_output,
    NativeInputControllerTreeIterator first,
    NativeInputControllerTreeIterator last,
    NativeStringRawPoolContext& strings) {
    static_cast<void>(unused_edx);
    auto* const receiver = actual_tree;
    auto cached_owner = word(&first, 0);
    auto* const initial_head = pointer(word(receiver, 4));
    const auto captured_begin = word(initial_head, 0);
    if (cached_owner == 0 || cached_owner != address(receiver))
        invoke_native_invalid_parameter_00bf6713();
    auto cached_node = word(&first, 4);

    if (cached_node == captured_begin) {
        const auto captured_last_owner = word(&last, 0);
        auto* const captured_end_head = pointer(word(receiver, 4));
        if (captured_last_owner == 0 || captured_last_owner != address(receiver))
            invoke_native_invalid_parameter_00bf6713();
        if (word(&last, 4) == address(captured_end_head)) {
            auto* const destroy_head = pointer(word(receiver, 4));
            auto* const root = pointer(word(destroy_head, 4));
            destroy_native_input_controller_subtree_006a62d0(receiver, 0u, root, strings);

            auto* const head_a = pointer(word(receiver, 4));
            put(head_a, 4, address(head_a));
            auto* const head_b = pointer(word(receiver, 4));
            put(receiver, 8, 0u);
            put(head_b, 0, address(head_b));
            auto* const head_c = pointer(word(receiver, 4));
            put(head_c, 8, address(head_c));
            auto* const result_head = pointer(word(receiver, 4));
            const auto result_node = word(result_head, 0);
            auto* const output = static_cast<NativeInputControllerTreeIterator*>(
                pointer(word(&actual_output, 0)));
            put(output, 0, address(receiver));
            put(output, 4, result_node);
            return output;
        }
    }

    NativeInputControllerTreeIterator ignored_output;
    for (;;) {
        if (cached_owner == 0 || cached_owner != word(&last, 0))
            invoke_native_invalid_parameter_00bf6713();
        if (cached_node == word(&last, 4)) {
            auto* const output = static_cast<NativeInputControllerTreeIterator*>(
                pointer(word(&actual_output, 0)));
            put(output, 0, cached_owner);
            put(output, 4, cached_node);
            return output;
        }

        const NativeInputControllerTreeIterator old{
            pointer(cached_owner), pointer(cached_node)};
        increment_native_input_controller_tree_iterator_0069ad50(&first, 0u);
        erase_native_input_controller_tree_iterator_006a6010(
            receiver, 0u, &ignored_output, old, strings);
        cached_node = word(&first, 4);
        cached_owner = word(&first, 0);
    }
}
} // namespace bsp
