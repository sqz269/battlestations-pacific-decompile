#include "bsp/native_input_controller_subtree.hpp"

#include "bsp/native_input_controller_name_pair_destroy.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp {
namespace {
using Word = std::uint32_t;
Word word(const void* base, std::size_t offset) {
    return *reinterpret_cast<const volatile Word*>(
        static_cast<const std::byte*>(base) + offset);
}
void put(void* base, std::size_t offset, Word value) {
    *reinterpret_cast<volatile Word*>(
        static_cast<std::byte*>(base) + offset) = value;
}
void* pointer(Word bits) {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(bits));
}
Word bits(const void* value) {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(value));
}
bool nil(const void* node) {
    return *reinterpret_cast<const volatile std::uint8_t*>(
        static_cast<const std::byte*>(node) + 0x21) != 0;
}
} // namespace

void __fastcall destroy_native_input_controller_subtree_006a62d0(
    void* actual_tree, std::uint32_t unused_edx, void* actual_node,
    NativeStringRawPoolContext& strings) {
    static_cast<void>(unused_edx);
    auto* node = actual_node;
    while (!nil(node)) {
        auto* const removed = node;
        auto* const right = pointer(word(removed, 8));
        destroy_native_input_controller_subtree_006a62d0(
            actual_tree, 0u, right, strings);
        // Capture only after right recursion and before pair destruction.
        auto* const left = pointer(word(removed, 0));
        destroy_native_input_controller_name_pair_006a2bb0(
            static_cast<std::byte*>(removed) + 0x0c, 0u, strings);
        singleton_lifetime_free(removed);
        node = left;
    }
}

void __fastcall clear_native_input_controller_tree_006a6650(
    void* actual_tree, std::uint32_t unused_edx,
    NativeStringRawPoolContext& strings) {
    static_cast<void>(unused_edx);
    auto* const initial_head = pointer(word(actual_tree, 4));
    auto* const initial_root = pointer(word(initial_head, 4));
    destroy_native_input_controller_subtree_006a62d0(
        actual_tree, 0u, initial_root, strings);
    auto* const head_a = pointer(word(actual_tree, 4));
    put(head_a, 4, bits(head_a));
    auto* const head_b = pointer(word(actual_tree, 4));
    put(actual_tree, 8, 0u);
    put(head_b, 0, bits(head_b));
    auto* const head_c = pointer(word(actual_tree, 4));
    put(head_c, 8, bits(head_c));
}
} // namespace bsp
