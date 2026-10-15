#include "bsp/native_input_scalar_map_full_cleanup.hpp"
#include "bsp/native_soldier_loop_lengths.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
static_assert(sizeof(void*) == 4, "Actual scalar-map storage requires Win32");

void* __fastcall clear_native_input_scalar_tree_full_prefix_00444b10(
    void* tree, std::uint32_t, void* output,
    void* first_owner, void* first_node, void* last_owner, void* last_node,
    NativeStringRawPoolContext& strings) {
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
    destroy_native_soldier_loop_subtree_00444760(tree, root, strings);
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

std::uint32_t __fastcall destroy_native_input_scalar_map_00445900(
    void* tree, std::uint32_t, NativeStringRawPoolContext& strings) {
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
    clear_native_input_scalar_tree_full_prefix_00444b10(
        tree, 0, private_output, tree, first, tree, head, strings);
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
