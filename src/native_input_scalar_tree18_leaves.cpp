#include "bsp/native_input_scalar_tree18_leaves.hpp"

#include "bsp/native_invalid_parameter_noinfo_call.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error These recovered register/stack schedules require MSVC Win32.
#endif

namespace bsp {

// Whole0069B190. Preserve both seven-byte LEA ESP,[ESP] instructions as
// actual instructions. The two external E9 tails restoreESI before transfer;
// no return continuation may publish another iterator node.
__declspec(naked) void __fastcall
decrement_native_input_scalar_tree_iterator_0069b190(void*, void*) {
    __asm {
        push esi
        mov esi, ecx
        cmp dword ptr [esi], 0
        jnz owner_ready
        call invoke_native_invalid_parameter_00bf6713
    owner_ready:
        mov eax, [esi + 4]
        cmp byte ptr [eax + 15h], 0
        jz nonnil_node
        mov eax, [eax + 8]
        mov [esi + 4], eax
        cmp byte ptr [eax + 15h], 0
        jz done
        pop esi
        jmp invoke_native_invalid_parameter_00bf6713
    nonnil_node:
        mov ecx, [eax]
        cmp byte ptr [ecx + 15h], 0
        jnz climb
        mov eax, [ecx + 8]
        cmp byte ptr [eax + 15h], 0
        jnz publish_max
        _emit 08dh
        _emit 0a4h
        _emit 024h
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
    maximum_loop:
        mov ecx, eax
        mov eax, [ecx + 8]
        cmp byte ptr [eax + 15h], 0
        jz maximum_loop
    publish_max:
        mov [esi + 4], ecx
        pop esi
        ret
    climb:
        mov eax, [eax + 4]
        cmp byte ptr [eax + 15h], 0
        jnz climb_finished
        _emit 08dh
        _emit 0a4h
        _emit 024h
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
    climb_loop:
        mov ecx, [esi + 4]
        cmp ecx, [eax]
        jnz climb_finished
        mov [esi + 4], eax
        mov edx, eax
        mov eax, [edx + 4]
        cmp byte ptr [eax + 15h], 0
        jz climb_loop
    climb_finished:
        mov ecx, [esi + 4]
        cmp byte ptr [ecx + 15h], 0
        jz publish_parent
        pop esi
        jmp invoke_native_invalid_parameter_00bf6713
    publish_parent:
        mov [esi + 4], eax
    done:
        pop esi
        ret
    }
}

// Whole005530D0, fixed nil+15h. The rereads after stores are intentional,
// including under aliases; no node overlay or generic rotation is used.
__declspec(naked) void __fastcall
rotate_left_native_input_scalar_tree_005530d0(void*, void*, void*) {
    __asm {
        mov edx, [esp + 4]
        mov eax, [edx + 8]
        push esi
        mov esi, [eax]
        mov [edx + 8], esi
        mov esi, [eax]
        cmp byte ptr [esi + 15h], 0
        jnz left_child_ready
        mov [esi + 4], edx
    left_child_ready:
        mov esi, [edx + 4]
        mov [eax + 4], esi
        mov ecx, [ecx + 4]
        cmp edx, [ecx + 4]
        pop esi
        jnz left_nonroot
        mov [ecx + 4], eax
        mov [eax], edx
        mov [edx + 4], eax
        ret 4
    left_nonroot:
        mov ecx, [edx + 4]
        cmp edx, [ecx]
        jnz left_other_side
        mov [ecx], eax
        mov [eax], edx
        mov [edx + 4], eax
        ret 4
    left_other_side:
        mov [ecx + 8], eax
        mov [eax], edx
        mov [edx + 4], eax
        ret 4
    }
}

// Whole00553120; the corresponding near link is promoted.right+8.
__declspec(naked) void __fastcall
rotate_right_native_input_scalar_tree_00553120(void*, void*, void*) {
    __asm {
        mov edx, [esp + 4]
        mov eax, [edx]
        push esi
        mov esi, [eax + 8]
        mov [edx], esi
        mov esi, [eax + 8]
        cmp byte ptr [esi + 15h], 0
        jnz right_child_ready
        mov [esi + 4], edx
    right_child_ready:
        mov esi, [edx + 4]
        mov [eax + 4], esi
        mov ecx, [ecx + 4]
        cmp edx, [ecx + 4]
        pop esi
        jnz right_nonroot
        mov [ecx + 4], eax
        mov [eax + 8], edx
        mov [edx + 4], eax
        ret 4
    right_nonroot:
        mov ecx, [edx + 4]
        cmp edx, [ecx + 8]
        jnz right_other_side
        mov [ecx + 8], eax
        mov [eax + 8], edx
        mov [edx + 4], eax
        ret 4
    right_other_side:
        mov [ecx], eax
        mov [eax + 8], edx
        mov [edx + 4], eax
        ret 4
    }
}

} // namespace bsp
