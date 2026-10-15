#include "bsp/native_input_nested_int_subtree.hpp"
#include "bsp/native_input_int_subtrees.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
static_assert(sizeof(void*) == 4, "Actual native storage requires Win32 words");
namespace {
__forceinline bool nil_1d(void* node) {
    std::uint32_t value;
    __asm {
        mov ecx, node
        movzx eax, byte ptr [ecx + 1dh]
        mov value, eax
    }
    return value != 0;
}
} // namespace

void* __fastcall clear_native_input_scalar_tree_full_prefix_005572f0(
    void* tree, std::uint32_t, void* output,
    void* first_owner, void* first_node, void* last_owner, void* last_node) {
    void* root;
    // Preserve the full-path field captures and TEST/CMP order. Their admitted
    // outcomes are preconditions, so no unsupported diagnostic/partial path or
    // substitute guard is introduced. 0055AF60 proves these exact arguments.
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
    destroy_native_input_int_scalar_subtree_00554f10(
        tree, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(root)), root);
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

void __fastcall destroy_native_input_nested_int_subtree_0055af60(
    void* opaque_tree, std::uint32_t, void* node) {
    while (!nil_1d(node)) {
        void* const current = node;
        void* right;
        __asm {
            mov ecx, current
            mov eax, dword ptr [ecx + 8]
            mov right, eax
        }
        destroy_native_input_nested_int_subtree_0055af60(opaque_tree, 0, right);
        void* nested_head;
        void* nested_first;
        void* left;
        void* nested_tree;
        __asm {
            mov ecx, current
            mov eax, dword ptr [ecx + 14h]
            mov nested_head, eax
            mov edx, dword ptr [eax]
            mov nested_first, edx
            mov edx, dword ptr [ecx]
            mov left, edx
            lea edx, [ecx + 10h]
            mov nested_tree, edx
        }
        std::uint32_t private_output[2];
        clear_native_input_scalar_tree_full_prefix_005572f0(
            nested_tree, 0, private_output,
            nested_tree, nested_first, nested_tree, nested_head);
        void* current_head;
        __asm {
            mov ecx, nested_tree
            mov eax, dword ptr [ecx + 4]
            mov current_head, eax
        }
        singleton_lifetime_free(current_head);
        __asm {
            mov ecx, nested_tree
            mov dword ptr [ecx + 4], 0
            mov dword ptr [ecx + 8], 0
        }
        singleton_lifetime_free(current);
        node = left;
    }
}
} // namespace bsp
