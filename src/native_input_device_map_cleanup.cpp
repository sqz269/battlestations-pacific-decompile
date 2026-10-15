#include "bsp/native_input_device_map_cleanup.hpp"
#include "bsp/native_input_device_subtree.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
static_assert(sizeof(void*) == 4, "Actual InputDevice map storage requires Win32");

void __fastcall clear_native_input_device_tree_006a7930(
    void* tree, std::uint32_t, NativeStringRawPoolContext& strings) {
    void* root;
    __asm {
        mov ecx, tree
        mov eax, dword ptr [ecx + 4]
        mov edx, dword ptr [eax + 4]
        mov root, edx
    }
    destroy_native_input_device_subtree_006a7540(tree, 0, root, strings);
    __asm {
        mov ecx, tree
        mov eax, dword ptr [ecx + 4]
        mov dword ptr [eax + 4], eax
        mov eax, dword ptr [ecx + 4]
        mov dword ptr [ecx + 8], 0
        mov dword ptr [eax], eax
        mov eax, dword ptr [ecx + 4]
        mov dword ptr [eax + 8], eax
    }
}

void* __fastcall clear_native_input_device_tree_full_prefix_006a7aa0(
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
    destroy_native_input_device_subtree_006a7540(tree, 0, root, strings);
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

std::uint32_t __fastcall destroy_native_input_device_map_006a7ba0(
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
    clear_native_input_device_tree_full_prefix_006a7aa0(
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

// Complete native bodies differ only in the two CALL displacements; both
// targets are the same6A7AA0/BF65AC. Reuse the whole storage/return schedule.
std::uint32_t __fastcall destroy_native_input_device_map_006aa3e0(
    void* tree, std::uint32_t, NativeStringRawPoolContext& strings) {
    return destroy_native_input_device_map_006a7ba0(tree, 0, strings);
}

std::uint32_t __fastcall destroy_native_input_device_map_006aa420(
    void* tree, std::uint32_t, NativeStringRawPoolContext& strings) {
    return destroy_native_input_device_map_006a7ba0(tree, 0, strings);
}
} // namespace bsp
