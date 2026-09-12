#pragma once
#include <cstddef>
#include <cstdint>

// Parameterized extraction of existing hardware-layout tree mechanics.
// No native address or independent STL implementation is claimed here.
namespace bsp::detail {
template<std::size_t ColorOffset, std::size_t NilOffset>
struct TreeInsertAccess {
    static void* volatile& word(void* storage, std::size_t offset) noexcept {
        return *reinterpret_cast<void* volatile*>(static_cast<unsigned char*>(storage) + offset);
    }
    static volatile std::uint32_t& count(void* tree) noexcept {
        return *reinterpret_cast<volatile std::uint32_t*>(static_cast<unsigned char*>(tree) + 8);
    }
    static volatile std::uint8_t& byte(void* storage, std::size_t offset) noexcept {
        return *reinterpret_cast<volatile std::uint8_t*>(static_cast<unsigned char*>(storage) + offset);
    }
    static void* volatile& left(void* node) noexcept { return word(node, 0); }
    static void* volatile& parent(void* node) noexcept { return word(node, 4); }
    static void* volatile& right(void* node) noexcept { return word(node, 8); }
    static volatile std::uint8_t& color(void* node) noexcept { return byte(node, ColorOffset); }
    static bool sentinel(void* node) noexcept { return byte(node, NilOffset) != 0; }
    static void* head(void* tree) noexcept { return word(tree, 4); }
};

// Existing B23020 traversal with its node comparison made explicit. The
// comparison policy controls when the caller's key storage is read.
template<class Access, class NodeLess>
void* lower_bound_tree_node(void* tree, NodeLess less) noexcept {
    auto* candidate = Access::head(tree);
    auto* node = Access::parent(candidate);
    while (!Access::sentinel(node)) {
        if (less(node)) {
            node = Access::right(node);
        } else {
            candidate = node;
            node = Access::left(node);
        }
    }
    return candidate;
}

// Existing B2F540 unique driver. Iterator shape, comparison loads, leaf
// operations and output publication remain policies of the consuming source.
template<class Access, class Iterator, class Output, class SearchLess,
    class FinalLess, class Decrement, class Link, class Publish>
Output* insert_unique_tree_pair(void* tree, Output* output, const void* pair,
    SearchLess search_less, FinalLess final_less, Decrement decrement,
    Link link, Publish publish) {
    auto* selected_parent = Access::head(tree);
    auto* node = Access::parent(selected_parent);
    bool insert_left = true;
    while (!Access::sentinel(node)) {
        selected_parent = node;
        insert_left = search_less(node);
        node = insert_left ? Access::left(node) : Access::right(node);
    }
    Iterator predecessor{tree, selected_parent};
    if (insert_left) {
        if (selected_parent == Access::left(Access::head(tree))) {
            link(tree, &predecessor, 1, selected_parent, pair);
            publish(output, predecessor.owner, predecessor.node, 1);
            return output;
        }
        decrement(predecessor);
    }
    node = predecessor.node;
    if (final_less(node)) {
        link(tree, &predecessor, static_cast<std::uint8_t>(insert_left), selected_parent, pair);
        publish(output, predecessor.owner, predecessor.node, 1);
    } else {
        publish(output, predecessor.owner, node, 0);
    }
    return output;
}

template<class Access>
void rotate_left_inlined(void* tree, void* node) noexcept {
    auto* const replacement = Access::right(node);
    Access::right(node) = Access::left(replacement);
    auto* const current_child = Access::left(replacement);
    if (!Access::sentinel(current_child)) {
        Access::parent(current_child) = node;
    }
    Access::parent(replacement) = Access::parent(node);
    auto* const current_head = Access::head(tree);
    if (node == Access::parent(current_head)) {
        Access::parent(current_head) = replacement;
    } else {
        auto* const current_parent = Access::parent(node);
        if (node == Access::left(current_parent)) {
            Access::left(current_parent) = replacement;
        } else {
            Access::right(current_parent) = replacement;
        }
    }
    Access::left(replacement) = node;
    Access::parent(node) = replacement;
}

template<class Access, class Invalid>
void decrement_tree_iterator(void* iterator, Invalid invalid) {
    if (Access::word(iterator, 0) == nullptr) {
        invalid();
    }
    auto* node = Access::word(iterator, 4);
    if (Access::sentinel(node)) {
        node = Access::right(node);
        Access::word(iterator, 4) = node;
        if (Access::sentinel(node)) {
            invalid();
        }
        return;
    }
    auto* child = Access::left(node);
    if (!Access::sentinel(child)) {
        auto* next = Access::right(child);
        while (!Access::sentinel(next)) {
            child = next;
            next = Access::right(child);
        }
        Access::word(iterator, 4) = child;
        return;
    }
    auto* ancestor = Access::parent(node);
    while (!Access::sentinel(ancestor)) {
        auto* const current = Access::word(iterator, 4);
        if (current != Access::left(ancestor)) {
            break;
        }
        Access::word(iterator, 4) = ancestor;
        ancestor = Access::parent(ancestor);
    }
    node = Access::word(iterator, 4);
    if (Access::sentinel(node)) {
        invalid();
        return;
    }
    Access::word(iterator, 4) = ancestor;
}

template<class Access, class Allocate, class RotateLeft, class RotateRight, class ThrowLength>
void* link_tree_node(void* tree, void* output, std::uint8_t insert_left,
    void* parent_node, const void* pair, std::uint32_t count_limit,
    Allocate allocate, RotateLeft rotate_left, RotateRight rotate_right, ThrowLength throw_length) {
    if (Access::count(tree) >= count_limit) {
        throw_length();
    }
    auto* const allocated_head = Access::head(tree);
    auto* const node = allocate(
        allocated_head, parent_node, allocated_head, pair, 0);
    auto* const current_head = Access::head(tree);
    Access::count(tree) = Access::count(tree) + 1u;
    if (parent_node == current_head) {
        Access::parent(current_head) = node;
        Access::left(Access::head(tree)) = node;
        Access::right(Access::head(tree)) = node;
    } else if (insert_left != 0) {
        Access::left(parent_node) = node;
        auto* const current = Access::head(tree);
        if (parent_node == Access::left(current)) {
            Access::left(current) = node;
        }
    } else {
        Access::right(parent_node) = node;
        auto* const current = Access::head(tree);
        if (parent_node == Access::right(current)) {
            Access::right(current) = node;
        }
    }
    auto* repair = node;
    while (Access::color(Access::parent(repair)) == 0) {
        auto* const direct_parent = Access::parent(repair);
        auto* const grandparent = Access::parent(direct_parent);
        if (direct_parent == Access::left(grandparent)) {
            auto* const uncle = Access::right(grandparent);
            if (Access::color(uncle) == 0) {
                Access::color(direct_parent) = 1;
                Access::color(uncle) = 1;
                Access::color(Access::parent(Access::parent(repair))) = 0;
                repair = Access::parent(Access::parent(repair));
            } else {
                if (repair == Access::right(direct_parent)) {
                    repair = direct_parent;
                    rotate_left(tree, repair);
                }
                Access::color(Access::parent(repair)) = 1;
                Access::color(Access::parent(Access::parent(repair))) = 0;
                rotate_right(tree, Access::parent(Access::parent(repair)));
            }
        } else {
            auto* const uncle = Access::left(grandparent);
            if (Access::color(uncle) == 0) {
                Access::color(direct_parent) = 1;
                Access::color(uncle) = 1;
                Access::color(Access::parent(Access::parent(repair))) = 0;
                repair = Access::parent(Access::parent(repair));
            } else {
                if (repair == Access::left(direct_parent)) {
                    repair = direct_parent;
                    rotate_right(tree, repair);
                }
                Access::color(Access::parent(repair)) = 1;
                Access::color(Access::parent(Access::parent(repair))) = 0;
                rotate_left_inlined<Access>(tree, Access::parent(Access::parent(repair)));
            }
        }
    }
    Access::color(Access::parent(Access::head(tree))) = 1;
    Access::word(output, 4) = node;
    Access::word(output, 0) = tree;
    return output;
}
} // namespace bsp::detail
