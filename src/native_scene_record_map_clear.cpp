#include "bsp/native_scene_record_map_clear.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include "bsp/native_scene_record_map_increment.hpp"
#include "bsp/native_scene_record_map_subtree_destroy.hpp"

#include <cstdint>

namespace bsp {
namespace {
using U = std::uint32_t;
static_assert(sizeof(void*) == 4);
U read(U address, U offset = 0) noexcept {
    return *reinterpret_cast<const volatile U*>(address + offset);
}
void write(U address, U offset, U value) noexcept {
    *reinterpret_cast<volatile U*>(address + offset) = value;
}
void reset_head(U tree) noexcept {
    U head = read(tree, 4);
    write(head, 4, head);
    head = read(tree, 4);
    write(tree, 8, 0);
    write(head, 0, head);
    head = read(tree, 4);
    write(head, 8, head);
}
void destroy_root(U tree, NativeStringRawPoolContext& strings) {
    const U head = read(tree, 4);
    const U root = read(head, 4);
    destroy_native_scene_record_map_subtree_0046ad10(reinterpret_cast<void*>(root), strings);
}
} // namespace

void clear_native_scene_record_map_0046b460(void* actual_tree,
    NativeStringRawPoolContext& strings) {
    const U tree = reinterpret_cast<std::uintptr_t>(actual_tree);
    destroy_root(tree, strings);
    reset_head(tree);
}

void clear_native_scene_database_hidden_records_0046b680(void* actual_database,
    void* actual_iterator8, NativeStringRawPoolContext& strings) {
    const U database = reinterpret_cast<std::uintptr_t>(actual_database);
    if (read(database, 0x20) == 0) return;
    const U initial_head = read(database, 0x1c);
    U node = read(initial_head);
    const U tree = database + 0x18u;
    const U iterator = reinterpret_cast<std::uintptr_t>(actual_iterator8);
    U owner = tree;
    write(iterator, 4, node);
    write(iterator, 0, owner);
    for (;;) {
        const U head = read(tree, 4);
        if (owner == 0 || owner != tree) invoke_native_invalid_parameter_00bf6713();
        if (node == head) break;
        if (owner == 0) invoke_native_invalid_parameter_00bf6713();
        if (node == read(owner, 4)) invoke_native_invalid_parameter_00bf6713();
        const bool at_end = node == read(owner, 4);
        const U mapped = read(node, 0x14);
        if (at_end) invoke_native_invalid_parameter_00bf6713();
        write(node, 0x14, 0);
        if (mapped != 0) {
            const U vtable = read(mapped);
            const U deleting_destructor = read(vtable);
            using Delete = void (__thiscall*)(void*, U);
            reinterpret_cast<Delete>(deleting_destructor)(reinterpret_cast<void*>(mapped), 1);
        }
        increment_native_scene_record_map_iterator_00467980(actual_iterator8);
        node = read(iterator, 4);
        owner = read(iterator, 0);
    }
    destroy_root(tree, strings);
    reset_head(tree);
}
} // namespace bsp
