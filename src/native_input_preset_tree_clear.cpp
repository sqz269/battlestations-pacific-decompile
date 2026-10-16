#include "bsp/native_input_preset_tree_clear.hpp"
#include "bsp/native_input_preset_key_subtree.hpp"

namespace bsp {
static_assert(sizeof(void*) == sizeof(std::uint32_t));

void __fastcall clear_native_input_preset_tree_006a1230(
    void* tree, std::uint32_t, NativeStringRawPoolContext& strings) {
    std::uint32_t root;
    __asm {
        mov eax, tree
        mov eax, dword ptr [eax + 4]
        mov eax, dword ptr [eax + 4]
        mov root, eax
    }
    destroy_native_input_preset_key_subtree_006a0770(
        tree, 0u, reinterpret_cast<void*>(root), strings);
    __asm {
        mov edx, tree
        mov eax, dword ptr [edx + 4]
        mov dword ptr [eax + 4], eax
        mov eax, dword ptr [edx + 4]
        mov dword ptr [edx + 8], 0
        mov dword ptr [eax], eax
        mov eax, dword ptr [edx + 4]
        mov dword ptr [eax + 8], eax
    }
}
} // namespace bsp
