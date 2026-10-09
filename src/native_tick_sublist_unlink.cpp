#include "bsp/native_tick_sublist_unlink.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4);
static_assert(sizeof(std::uint32_t) == 4);

// Keep the complete leaf, including TEST flags across conditional PUSH EDI
// and the actual ADD -1 flags. No Source caller or profile binding is supplied.
__declspec(naked) void* __fastcall unlink_native_tick_sublist_node_00874e60(
    void*, std::uint32_t, void*) {
    __asm {
        mov eax, dword ptr [esp + 4]           // 00874E60
        mov edx, dword ptr [eax + 8]           // 00874E64
        test edx, edx                         // 00874E67
        jnz short unlink_mutate               // 00874E69
        cmp dword ptr [eax + 0ch], edx         // 00874E6B
        jnz short unlink_mutate               // 00874E6E
        cmp dword ptr [ecx + 8], 1             // 00874E70
        jg short unlink_return                // 00874E74
    unlink_mutate:
        test edx, edx                         // 00874E76
        push edi                              // 00874E78
        jz short unlink_head                  // 00874E79
        mov edi, dword ptr [eax + 0ch]         // 00874E7B
        mov dword ptr [edx + 0ch], edi         // 00874E7E
        jmp short unlink_next                 // 00874E81
    unlink_head:
        mov edx, dword ptr [eax + 0ch]         // 00874E83
        mov dword ptr [ecx], edx               // 00874E86
    unlink_next:
        mov edx, dword ptr [eax + 0ch]         // 00874E88
        test edx, edx                         // 00874E8B
        jz short unlink_tail                  // 00874E8D
        mov edi, dword ptr [eax + 8]           // 00874E8F
        mov dword ptr [edx + 8], edi           // 00874E92
        jmp short unlink_clear                // 00874E95
    unlink_tail:
        mov edx, dword ptr [eax + 8]           // 00874E97
        mov dword ptr [ecx + 4], edx           // 00874E9A
    unlink_clear:
        mov dword ptr [eax + 0ch], 0           // 00874E9D
        mov dword ptr [eax + 8], 0             // 00874EA4
        add dword ptr [ecx + 8], -1            // 00874EAB
        pop edi                               // 00874EAF
    unlink_return:
        ret 4                                 // 00874EB0
    }
}

} // namespace bsp
