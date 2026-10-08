#include "bsp/native_unit_group_direct_views.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw unit group direct views require MSVC Win32 assembly.
#endif

namespace bsp {
// Whole0070D060..0070D06E:14B4, no calls/globals/relocations; native RET4.
__declspec(naked) void* __fastcall native_unit_group_member_at_0070d060(
    const void*, void*, std::uint32_t) noexcept {
    __asm {
        mov eax, dword ptr [esp + 4]
        imul eax, eax, 0x34
        mov eax, dword ptr [eax + ecx + 0x18]
        ret 4
    }
}

// Whole0070D070..0070D07E:14B4. Same wrapped address, returned without a load.
__declspec(naked) void* __fastcall native_unit_group_record_at_0070d070(
    void*, void*, std::uint32_t) noexcept {
    __asm {
        mov eax, dword ptr [esp + 4]
        imul eax, eax, 0x34
        lea eax, dword ptr [eax + ecx + 0x18]
        ret 4
    }
}

// Whole0070D0F0..0070D0F7:7B2. Preserve raw FLD/ST0 and plain RET semantics.
__declspec(naked) float __fastcall native_unit_group_retained_speed_0070d0f0(
    const void*, void*) noexcept {
    __asm {
        fld dword ptr [ecx + 0x504]
        ret
    }
}

// Whole0070D100..0070D133:51B16. Keep both signed fresh-count reads, early
// XMM0-preserving exit and every matching MOVSS store; no validation default.
__declspec(naked) void __fastcall publish_native_unit_group_member_speed_0070d100(
    void*, void*, void*, std::uint32_t) noexcept {
    __asm {
        xor eax, eax
        cmp dword ptr [ecx + 0x4f8], eax
        jle done
        movss xmm0, dword ptr [esp + 8]
        push esi
        mov esi, dword ptr [esp + 8]
        lea edx, dword ptr [ecx + 0x48]
    next_record:
        cmp dword ptr [edx - 0x30], esi
        jne advance
        movss dword ptr [edx], xmm0
    advance:
        add eax, 1
        add edx, 0x34
        cmp eax, dword ptr [ecx + 0x4f8]
        jl next_record
        pop esi
    done:
        ret 8
    }
}
} // namespace bsp
