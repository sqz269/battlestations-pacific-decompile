#include "bsp/native_scene_integer_set_clear.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {
using U = std::uint32_t;
static_assert(sizeof(void*) == 4);
U read(U address, U offset = 0) noexcept {
    return *reinterpret_cast<const volatile U*>(address + offset);
}
bool nil(U node) noexcept {
    return *reinterpret_cast<const volatile unsigned char*>(node + 0x11) != 0;
}
void write(U address, U offset, U value) noexcept {
    *reinterpret_cast<volatile U*>(address + offset) = value;
}
} // namespace

void __fastcall destroy_native_scene_integer_set_subtree_00468300(
    void* tree, std::uint32_t, void* actual_node) {
    U node = reinterpret_cast<std::uintptr_t>(actual_node);
    while (!nil(node)) {
        const U right = read(node, 8);
        destroy_native_scene_integer_set_subtree_00468300(
            tree, 0, reinterpret_cast<void*>(right));
        const U left = read(node);
        singleton_lifetime_free(reinterpret_cast<void*>(node));
        node = left;
    }
}

void __fastcall clear_native_scene_integer_set_00468980(void* actual_tree, std::uint32_t) {
    const U tree = reinterpret_cast<std::uintptr_t>(actual_tree);
    U head = read(tree, 4);
    const U root = read(head, 4);
    destroy_native_scene_integer_set_subtree_00468300(
        actual_tree, 0, reinterpret_cast<void*>(root));
    head = read(tree, 4);
    write(head, 4, head);
    head = read(tree, 4);
    write(tree, 8, 0);
    write(head, 0, head);
    head = read(tree, 4);
    write(head, 8, head);
}
} // namespace bsp
