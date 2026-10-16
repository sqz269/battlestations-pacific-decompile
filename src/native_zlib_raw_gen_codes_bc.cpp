#include "bsp/native_zlib_raw_gen_codes_bc.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This raw returning register body requires MSVC Win32.
#endif

namespace {
// Complete 00BCF140..00BCF1B5. Custom EDX=counts, EBX=signed max_code,
// EDI=tree; no actual stack arguments, plain RET. Source spelling is private,
// not a recovered Original ABI. Only the real assembly CALL below enters it.
// ESP-based indexed memory retains its actual SS default and real frame cells.
__declspec(naked) void __cdecl gen_codes_body() noexcept {
    __asm {
        sub esp, 020h
        push esi
        lea esi, [esp + 6]
        xor ecx, ecx
        mov eax, 1
        sub edx, esi
    generate_next_code:
        lea esi, [edx + eax * 2]
        mov si, word ptr [esp + esi * 1 + 4]
        add si, cx
        add si, si
        movzx ecx, si
        mov word ptr [esp + eax * 2 + 4], cx
        add eax, 1
        cmp eax, 0fh
        jle generate_next_code
        xor esi, esi
        test ebx, ebx
        jl restore_esi
        push ebp
    encode_node:
        movzx edx, word ptr [edi + esi * 4 + 2]
        test edx, edx
        jz advance_node
        movzx eax, word ptr [esp + edx * 2 + 8]
        movzx ecx, ax
        add eax, 1
        mov word ptr [esp + edx * 2 + 8], ax
        xor eax, eax
    reverse_code_bit:
        mov ebp, ecx
        and ebp, 1
        or eax, ebp
        sub edx, 1
        shr ecx, 1
        add eax, eax
        test edx, edx
        jg reverse_code_bit
        shr eax, 1
        mov word ptr [edi + esi * 4], ax
    advance_node:
        add esi, 1
        cmp esi, ebx
        jle encode_node
        pop ebp
    restore_esi:
        pop esi
        add esp, 020h
        ret
    }
}
}

// Returning Source cdecl activation. Its real saves/argument reads/CALL and
// suffix remain observable; raw kernel aliases may redirect or skip the suffix.
extern "C" __declspec(naked) void __cdecl
bsp_native_zlib_gen_codes_00bcf140(const void*, std::int32_t, void*) noexcept {
    __asm {
        push ebx
        push edi
        mov edx, dword ptr [esp + 00ch]
        mov ebx, dword ptr [esp + 010h]
        mov edi, dword ptr [esp + 014h]
        call gen_codes_body
        pop edi
        pop ebx
        ret
    }
}
