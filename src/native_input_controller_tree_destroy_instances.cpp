#include "bsp/native_input_controller_tree_destroy_instances.hpp"

#include "bsp/native_input_controller_tree_range.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);

std::uintptr_t address(const void* storage, Word byte_offset) {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(storage)) + byte_offset;
}

Word word(const void* storage, Word byte_offset) {
    return *reinterpret_cast<const volatile Word*>(address(storage, byte_offset));
}

void store(void* storage, Word byte_offset, Word value) {
    *reinterpret_cast<volatile Word*>(address(storage, byte_offset)) = value;
}

void* pointer(Word bits) {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(bits));
}
} // namespace

std::uint32_t __fastcall destroy_native_input_controller_tree_006a6c20(
    void* tree, std::uint32_t unused_edx, NativeStringRawPoolContext& strings) {
    static_cast<void>(unused_edx);
    auto* const captured_head = pointer(word(tree, 4));
    auto* const captured_left = pointer(word(captured_head, 0));
    NativeInputControllerTreeIterator ignored_output;
    erase_native_input_controller_tree_range_006a6a20(
        tree, 0u, &ignored_output, {tree, captured_left}, {tree, captured_head}, strings);
    singleton_lifetime_free(pointer(word(tree, 4)));
    store(tree, 4, 0u);
    store(tree, 8, 0u);
    return 0u;
}

std::uint32_t __fastcall destroy_native_input_controller_tree_006a6e70(
    void* tree, std::uint32_t unused_edx, NativeStringRawPoolContext& strings) {
    static_cast<void>(unused_edx);
    auto* const captured_head = pointer(word(tree, 4));
    auto* const captured_left = pointer(word(captured_head, 0));
    NativeInputControllerTreeIterator ignored_output;
    erase_native_input_controller_tree_range_006a6a20(
        tree, 0u, &ignored_output, {tree, captured_left}, {tree, captured_head}, strings);
    singleton_lifetime_free(pointer(word(tree, 4)));
    store(tree, 4, 0u);
    store(tree, 8, 0u);
    return 0u;
}

std::uint32_t __fastcall destroy_native_input_controller_tree_006a7040(
    void* tree, std::uint32_t unused_edx, NativeStringRawPoolContext& strings) {
    static_cast<void>(unused_edx);
    auto* const captured_head = pointer(word(tree, 4));
    auto* const captured_left = pointer(word(captured_head, 0));
    NativeInputControllerTreeIterator ignored_output;
    erase_native_input_controller_tree_range_006a6a20(
        tree, 0u, &ignored_output, {tree, captured_left}, {tree, captured_head}, strings);
    singleton_lifetime_free(pointer(word(tree, 4)));
    store(tree, 4, 0u);
    store(tree, 8, 0u);
    return 0u;
}
} // namespace bsp
