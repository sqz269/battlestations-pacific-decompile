#include "bsp/native_input_device_tree_rotations.hpp"

namespace bsp {
static_assert(sizeof(void*) == 4, "Actual InputDevice rotations require Win32");

void __fastcall rotate_native_input_device_tree_right_00552340(
    void* tree, std::uint32_t, void* node) {
    __asm {
        mov ecx, tree
        mov edx, node
        mov eax, dword ptr [edx]
        mov esi, dword ptr [eax + 8]
        mov dword ptr [edx], esi
        mov esi, dword ptr [eax + 8]
        cmp byte ptr [esi + 99h], 0
        jne right_parent
        mov dword ptr [esi + 4], edx
    right_parent:
        mov esi, dword ptr [edx + 4]
        mov dword ptr [eax + 4], esi
        mov ecx, dword ptr [ecx + 4]
        cmp edx, dword ptr [ecx + 4]
        jne right_nonroot
        mov dword ptr [ecx + 4], eax
        jmp right_finish
    right_nonroot:
        mov ecx, dword ptr [edx + 4]
        cmp edx, dword ptr [ecx + 8]
        jne right_left_slot
        mov dword ptr [ecx + 8], eax
        jmp right_finish
    right_left_slot:
        mov dword ptr [ecx], eax
    right_finish:
        mov dword ptr [eax + 8], edx
        mov dword ptr [edx + 4], eax
    }
}

void __fastcall rotate_native_input_device_tree_left_00553f00(
    void* tree, std::uint32_t, void* node) {
    __asm {
        mov ecx, tree
        mov edx, node
        mov eax, dword ptr [edx + 8]
        mov esi, dword ptr [eax]
        mov dword ptr [edx + 8], esi
        mov esi, dword ptr [eax]
        cmp byte ptr [esi + 99h], 0
        jne left_parent
        mov dword ptr [esi + 4], edx
    left_parent:
        mov esi, dword ptr [edx + 4]
        mov dword ptr [eax + 4], esi
        mov ecx, dword ptr [ecx + 4]
        cmp edx, dword ptr [ecx + 4]
        jne left_nonroot
        mov dword ptr [ecx + 4], eax
        jmp left_finish
    left_nonroot:
        mov ecx, dword ptr [edx + 4]
        cmp edx, dword ptr [ecx]
        jne left_right_slot
        mov dword ptr [ecx], eax
        jmp left_finish
    left_right_slot:
        mov dword ptr [ecx + 8], eax
    left_finish:
        mov dword ptr [eax], edx
        mov dword ptr [edx + 4], eax
    }
}
} // namespace bsp
