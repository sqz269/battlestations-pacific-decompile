#include "bsp/native_input_default_scalar_tree_clear.hpp"

#include "bsp/native_input_default_scalar_subtree.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));

Word load_word(Word address) {
    return *reinterpret_cast<const volatile Word*>(
        static_cast<std::uintptr_t>(address));
}
void store_word(Word address, Word value) {
    *reinterpret_cast<volatile Word*>(
        static_cast<std::uintptr_t>(address)) = value;
}
} // namespace

void __fastcall clear_native_input_default_scalar_tree_0069dc70(
    void* actual_tree, std::uint32_t /*unused_edx*/) {
    const auto tree = static_cast<Word>(
        reinterpret_cast<std::uintptr_t>(actual_tree));
    const auto initial_head = load_word(tree + 4u);
    const auto initial_root = load_word(initial_head + 4u);
    destroy_native_input_default_scalar_subtree_0069d3f0(
        actual_tree, 0u,
        reinterpret_cast<void*>(static_cast<std::uintptr_t>(initial_root)));

    const auto root_head = load_word(tree + 4u);
    store_word(root_head + 4u, root_head);
    const auto left_head = load_word(tree + 4u);
    store_word(tree + 8u, 0u);
    store_word(left_head, left_head);
    const auto right_head = load_word(tree + 4u);
    store_word(right_head + 8u, right_head);
}

} // namespace bsp
