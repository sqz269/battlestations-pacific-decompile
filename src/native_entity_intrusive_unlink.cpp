#include "bsp/native_entity_intrusive_unlink.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4 && native_entity_intrusive_header_bytes == 12,
    "The borrowed physical headers and entity links are Win32.");

// Complete 00903F30..00903F82. Retain the original memory reloads and final
// ADD (not DEC): the two instructions have different carry-flag behavior.
__declspec(naked) void* __fastcall unlink_native_entity_world_chain_00903f30(
    void*, std::uint32_t, void*) noexcept {
    __asm {
        mov eax, dword ptr [esp + 4]
        mov edx, dword ptr [eax + 34h]
        test edx, edx
        jnz world_unlink_mutate
        cmp dword ptr [eax + 38h], edx
        jnz world_unlink_mutate
        cmp dword ptr [ecx + 8], 1
        jg world_unlink_done
    world_unlink_mutate:
        test edx, edx
        push edi
        jz world_unlink_first
        mov edi, dword ptr [eax + 38h]
        mov dword ptr [edx + 38h], edi
        jmp world_unlink_next
    world_unlink_first:
        mov edx, dword ptr [eax + 38h]
        mov dword ptr [ecx], edx
    world_unlink_next:
        mov edx, dword ptr [eax + 38h]
        test edx, edx
        jz world_unlink_last
        mov edi, dword ptr [eax + 34h]
        mov dword ptr [edx + 34h], edi
        jmp world_unlink_clear
    world_unlink_last:
        mov edx, dword ptr [eax + 34h]
        mov dword ptr [ecx + 4], edx
    world_unlink_clear:
        mov dword ptr [eax + 38h], 0
        mov dword ptr [eax + 34h], 0
        add dword ptr [ecx + 8], -1
        pop edi
    world_unlink_done:
        ret 4
    }
}

// Complete 00924710..00924762. Only the entity link displacements differ
// from the world-chain body; this header remains {first, last, count}.
__declspec(naked) void* __fastcall unlink_native_entity_sibling_chain_00924710(
    void*, std::uint32_t, void*) noexcept {
    __asm {
        mov eax, dword ptr [esp + 4]
        mov edx, dword ptr [eax + 40h]
        test edx, edx
        jnz sibling_unlink_mutate
        cmp dword ptr [eax + 44h], edx
        jnz sibling_unlink_mutate
        cmp dword ptr [ecx + 8], 1
        jg sibling_unlink_done
    sibling_unlink_mutate:
        test edx, edx
        push edi
        jz sibling_unlink_first
        mov edi, dword ptr [eax + 44h]
        mov dword ptr [edx + 44h], edi
        jmp sibling_unlink_next
    sibling_unlink_first:
        mov edx, dword ptr [eax + 44h]
        mov dword ptr [ecx], edx
    sibling_unlink_next:
        mov edx, dword ptr [eax + 44h]
        test edx, edx
        jz sibling_unlink_last
        mov edi, dword ptr [eax + 40h]
        mov dword ptr [edx + 40h], edi
        jmp sibling_unlink_clear
    sibling_unlink_last:
        mov edx, dword ptr [eax + 40h]
        mov dword ptr [ecx + 4], edx
    sibling_unlink_clear:
        mov dword ptr [eax + 44h], 0
        mov dword ptr [eax + 40h], 0
        add dword ptr [ecx + 8], -1
        pop edi
    sibling_unlink_done:
        ret 4
    }
}

} // namespace bsp
