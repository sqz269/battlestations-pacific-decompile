#include "bsp/native_input_controller_tree_leaves.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"

#include <cstddef>

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

Word address(const void* pointer) {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(pointer));
}

void* pointer(Word bits) {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(bits));
}

bool is_nil(const void* node) {
    return *reinterpret_cast<const volatile std::uint8_t*>(
        static_cast<const std::byte*>(node) + 0x21) != 0;
}
} // namespace

void __fastcall increment_native_input_controller_tree_iterator_0069ad50(
    void* actual_iterator, std::uint32_t unused_edx) {
    static_cast<void>(unused_edx);
    if (word(actual_iterator, 0) == 0)
        invoke_native_invalid_parameter_00bf6713();

    auto* const node = pointer(word(actual_iterator, 4));
    if (is_nil(node)) {
        invoke_native_invalid_parameter_00bf6713();
        return;
    }
    auto* right = pointer(word(node, 8));
    if (!is_nil(right)) {
        auto* left = pointer(word(right, 0));
        while (!is_nil(left)) {
            right = left;
            left = pointer(word(right, 0));
        }
        put(actual_iterator, 4, address(right));
        return;
    }

    auto* parent = pointer(word(node, 4));
    while (!is_nil(parent)) {
        const auto current_node = word(actual_iterator, 4);
        if (current_node != word(parent, 8))
            break;
        put(actual_iterator, 4, address(parent));
        parent = pointer(word(parent, 4));
    }
    put(actual_iterator, 4, address(parent));
}

void __fastcall rotate_native_input_controller_tree_right_0069a6a0(
    void* actual_tree, std::uint32_t unused_edx, void* actual_node) {
    static_cast<void>(unused_edx);
    const auto node = address(actual_node);
    auto* const pivot = pointer(word(actual_node, 0));
    put(actual_node, 0, word(pivot, 8));
    auto* const current_child = pointer(word(pivot, 8));
    if (!is_nil(current_child))
        put(current_child, 4, node);

    put(pivot, 4, word(actual_node, 4));
    auto* const head = pointer(word(actual_tree, 4));
    if (node == word(head, 4)) {
        put(head, 4, address(pivot));
    } else {
        auto* const current_parent = pointer(word(actual_node, 4));
        if (node == word(current_parent, 8))
            put(current_parent, 8, address(pivot));
        else
            put(current_parent, 0, address(pivot));
    }
    put(pivot, 8, node);
    put(actual_node, 4, address(pivot));
}

void __fastcall rotate_native_input_controller_tree_left_0069c280(
    void* actual_tree, std::uint32_t unused_edx, void* actual_node) {
    static_cast<void>(unused_edx);
    const auto node = address(actual_node);
    auto* const pivot = pointer(word(actual_node, 8));
    put(actual_node, 8, word(pivot, 0));
    auto* const current_child = pointer(word(pivot, 0));
    if (!is_nil(current_child))
        put(current_child, 4, node);

    put(pivot, 4, word(actual_node, 4));
    auto* const head = pointer(word(actual_tree, 4));
    if (node == word(head, 4)) {
        put(head, 4, address(pivot));
    } else {
        auto* const current_parent = pointer(word(actual_node, 4));
        if (node == word(current_parent, 0))
            put(current_parent, 0, address(pivot));
        else
            put(current_parent, 8, address(pivot));
    }
    put(pivot, 0, node);
    put(actual_node, 4, address(pivot));
}

void* __fastcall maximum_native_input_controller_node_0069aa60(void* actual_subtree) {
    auto* current = actual_subtree;
    auto* child = pointer(word(current, 8));
    while (!is_nil(child)) {
        current = child;
        child = pointer(word(current, 8));
    }
    return current;
}

void* __fastcall minimum_native_input_controller_node_0069aa80(void* actual_subtree) {
    auto* current = actual_subtree;
    auto* child = pointer(word(current, 0));
    while (!is_nil(child)) {
        current = child;
        child = pointer(word(current, 0));
    }
    return current;
}

} // namespace bsp
