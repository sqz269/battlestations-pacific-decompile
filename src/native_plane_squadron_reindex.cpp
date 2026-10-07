#include "bsp/native_plane_squadron_reindex.hpp"

namespace bsp {

// Preserve all 83 instructions, including fresh count/member loads and the
// signed slot scans. A bounded C++ array changes five-member caller reads.
__declspec(naked) void __fastcall native_plane_squadron_reindex_007ed260(
    void*, void*) {
    __asm {
        sub esp, 0x14                    // 007ED260
        push ebx
        push ebp
        xor ebp, ebp
        xor eax, eax
        cmp dword ptr [ecx + 0x3cc], ebp
        push esi
        mov dword ptr [esp + 0x0c], ebp
        mov dword ptr [esp + 0x10], ebp
        mov dword ptr [esp + 0x14], ebp
        mov dword ptr [esp + 0x18], ebp
        mov dword ptr [esp + 0x1c], ebp
        jle first_pass_done
        lea edx, [ecx + 0x3d0]
        // Original four-byte LEA ESP,[ESP+0], retaining its displacement.
        _emit 0x8d
        _emit 0x64
        _emit 0x24
        _emit 0x00
    first_pass:
        mov esi, dword ptr [edx]          // 007ED290
        mov dword ptr [esi + 0x9d8], eax
        mov esi, dword ptr [edx]
        mov esi, dword ptr [esi + 0x9d0]
        add eax, 1
        add edx, 4
        cmp eax, dword ptr [ecx + 0x3cc]
        mov dword ptr [esp + esi * 4 + 0x0c], 1
        jl first_pass
    first_pass_done:
        xor ebx, ebx                     // 007ED2B6
        cmp dword ptr [ecx + 0x3cc], ebp
        jle finish
        or esi, -1
        push edi
    assign_member:
        cmp ebx, ebp                     // 007ED2C8
        jnz assign_follower
        mov eax, dword ptr [ecx + 0x3d0]
        mov dword ptr [eax + 0x9d0], ebp
        mov dword ptr [esp + 0x10], esi
        // Original near JMP 007ED2DC -> 007ED35E (signed displacement 7D).
        _emit 0xe9
        _emit 0x7d
        _emit 0x00
        _emit 0x00
        _emit 0x00
    assign_follower:
        cmp dword ptr [esp + 0x14], 0     // 007ED2E1
        mov esi, dword ptr [ecx + ebx * 4 + 0x3d0]
        mov edi, dword ptr [esi + 0x9d0]
        mov eax, 1
        lea edx, [eax + 1]
        jge odd_available
        // Original three-byte LEA ECX,[ECX+0], retaining its displacement.
        _emit 0x8d
        _emit 0x49
        _emit 0x00
    scan_odd:
        add eax, 2                       // 007ED300
        cmp dword ptr [esp + eax * 4 + 0x10], 0
        jl scan_odd
    odd_available:
        cmp dword ptr [esp + 0x18], 0     // 007ED30A
        jge even_available
    scan_even:
        add edx, 2                       // 007ED311
        cmp dword ptr [esp + edx * 4 + 0x10], 0
        jl scan_even
    even_available:
        lea ebp, [edx - 1]               // 007ED31B
        cmp eax, ebp
        jnz nonadjacent
        xor ebp, ebp
        cmp edi, ebp
        jle select_candidate
        and edi, 0x80000001
        jns signed_remainder_ready
        dec edi
        or edi, 0xfffffffe
        inc edi
    signed_remainder_ready:
        jnz select_candidate             // 007ED335
        add eax, 2
        jmp select_candidate
    nonadjacent:
        xor ebp, ebp                     // 007ED33C
    select_candidate:
        cmp eax, edx                     // 007ED33E
        jge use_even
        mov dword ptr [esi + 0x9d0], eax
        or esi, -1
        mov dword ptr [esp + eax * 4 + 0x10], esi
        jmp next_member
    use_even:
        mov dword ptr [esi + 0x9d0], edx  // 007ED351
        or esi, -1
        mov dword ptr [esp + edx * 4 + 0x10], esi
    next_member:
        add ebx, 1                       // 007ED35E
        cmp ebx, dword ptr [ecx + 0x3cc]
        jl assign_member
        pop edi
    finish:
        pop esi                          // 007ED36E
        pop ebp
        pop ebx
        add esp, 0x14
        ret
    }
}

} // namespace bsp
