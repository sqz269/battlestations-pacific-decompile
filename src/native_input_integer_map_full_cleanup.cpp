#include "bsp/native_input_integer_map_full_cleanup.hpp"
#include "bsp/native_input_int_subtrees.hpp"
#include "bsp/native_input_nested_int_subtree.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
static_assert(sizeof(void*) == 4, "Actual owned tree storage requires Win32");
namespace {
// The two complete native127B full prefixes match accepted5572F0 after only
// the three proven CALL rel32 fields. These private helpers share only their
// raw ordered field operations, without a runtime callee/provider abstraction.
__forceinline void* selected_full_root(void* tree, void* first_owner,
    void* first_node, void* last_owner, void* last_node) {
    void* root;
    __asm {
        mov eax, first_owner
        test eax, eax
        mov ecx, tree
        mov edx, dword ptr [ecx + 4]
        mov edx, dword ptr [edx]
        cmp eax, ecx
        mov eax, first_node
        cmp eax, edx
        mov eax, last_owner
        test eax, eax
        mov edx, dword ptr [ecx + 4]
        cmp eax, ecx
        mov eax, last_node
        cmp eax, edx
        mov edx, dword ptr [ecx + 4]
        mov edx, dword ptr [edx + 4]
        mov root, edx
    }
    return root;
}
__forceinline void* finish_full_prefix(void* tree, void* output) {
    __asm {
        mov ecx, tree
        mov eax, dword ptr [ecx + 4]
        mov dword ptr [eax + 4], eax
        mov eax, dword ptr [ecx + 4]
        mov dword ptr [ecx + 8], 0
        mov dword ptr [eax], eax
        mov eax, dword ptr [ecx + 4]
        mov dword ptr [eax + 8], eax
        mov eax, dword ptr [ecx + 4]
        mov edx, dword ptr [eax]
        mov eax, output
        mov dword ptr [eax], ecx
        mov dword ptr [eax + 4], edx
    }
    return output;
}
} // namespace

void* __fastcall clear_native_input_int_only_tree_full_prefix_004cef40(
    void* tree, std::uint32_t, void* output,
    void* first_owner, void* first_node, void* last_owner, void* last_node) {
    void* const root = selected_full_root(tree, first_owner, first_node, last_owner, last_node);
    destroy_native_input_int_only_subtree_004c1890(
        tree, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(root)), root);
    return finish_full_prefix(tree, output);
}

void* __fastcall clear_native_input_nested_int_tree_full_prefix_0055b230(
    void* tree, std::uint32_t, void* output,
    void* first_owner, void* first_node, void* last_owner, void* last_node) {
    void* const root = selected_full_root(tree, first_owner, first_node, last_owner, last_node);
    destroy_native_input_nested_int_subtree_0055af60(
        tree, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(root)), root);
    return finish_full_prefix(tree, output);
}

std::uint32_t __fastcall destroy_native_input_nested_int_map_0055b490(
    void* tree, std::uint32_t) {
    void* head;
    void* first;
    __asm {
        mov ecx, tree
        mov eax, dword ptr [ecx + 4]
        mov head, eax
        mov edx, dword ptr [eax]
        mov first, edx
    }
    std::uint32_t private_output[2];
    clear_native_input_nested_int_tree_full_prefix_0055b230(
        tree, 0, private_output, tree, first, tree, head);
    void* current_head;
    __asm {
        mov ecx, tree
        mov eax, dword ptr [ecx + 4]
        mov current_head, eax
    }
    singleton_lifetime_free(current_head);
    __asm {
        mov ecx, tree
        xor eax, eax
        mov dword ptr [ecx + 4], eax
        mov dword ptr [ecx + 8], eax
    }
    return 0;
}
} // namespace bsp
