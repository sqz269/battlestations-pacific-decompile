#include "bsp/native_input_bit_sensitivity_map_full_cleanup.hpp"
#include "bsp/native_input_bit_sensitivity_subtrees.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
static_assert(sizeof(void*) == 4, "Actual bit/sensitivity-map storage requires Win32");
namespace {
// Both native full prefixes have the same ordered selection operations.
// Admitted outcomes are preconditions; no substitute bad-path branch exists.
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

void* __fastcall clear_native_input_bit_vector_tree_full_prefix_00559280(
    void* tree, std::uint32_t, void* output,
    void* first_owner, void* first_node, void* last_owner, void* last_node,
    NativeStringRawPoolContext& strings) {
    void* const root = selected_full_root(tree, first_owner,
        first_node, last_owner, last_node);
    destroy_native_input_bit_vector_subtree_00558420(tree,
        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(root)),
        root, strings);
    return finish_full_prefix(tree, output);
}

void* __fastcall clear_native_input_sensitivity_tree_full_prefix_00559a90(
    void* tree, std::uint32_t, void* output,
    void* first_owner, void* first_node, void* last_owner, void* last_node,
    NativeStringRawPoolContext& strings) {
    void* const root = selected_full_root(tree, first_owner,
        first_node, last_owner, last_node);
    destroy_native_input_sensitivity_subtree_00558ed0(tree,
        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(root)),
        root, strings);
    return finish_full_prefix(tree, output);
}

std::uint32_t __fastcall destroy_native_input_bit_vector_map_0055a430(
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
    clear_native_input_bit_vector_tree_full_prefix_00559280(tree, 0u,
        private_output, tree, first, tree, head, strings);
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
    return 0u;
}

std::uint32_t __fastcall destroy_native_input_sensitivity_map_0055aaf0(
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
    clear_native_input_sensitivity_tree_full_prefix_00559a90(tree, 0u,
        private_output, tree, first, tree, head, strings);
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
    return 0u;
}
} // namespace bsp
