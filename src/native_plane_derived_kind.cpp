#include "bsp/native_plane_derived_kind.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error These complete raw entries require MSVC Win32.
#endif

namespace bsp {

// COMPLETE[007D77F0,007D782B) 59B/21 instructions.
__declspec(naked) std::uint32_t __fastcall native_plane_derived_is_kind_007d77f0(
    const void*, void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4]
        cmp eax, 010h
        je accept_007d77f0
        cmp eax, 00Fh
        je accept_007d77f0
        cmp eax, 005h
        je accept_007d77f0
        cmp eax, 004h
        je accept_007d77f0
        cmp eax, 002h
        je accept_007d77f0
        cmp eax, 001h
        je accept_007d77f0
        test eax, eax
        je accept_007d77f0
        cmp eax, dword ptr [ecx + 0C4h]
        je accept_007d77f0
        xor eax, eax
        ret 4
    accept_007d77f0:
        mov eax, 1
        ret 4
    }
}

// COMPLETE[009535C0,009535FB) 59B/21 instructions.
__declspec(naked) std::uint32_t __fastcall native_plane_derived_is_kind_009535c0(
    const void*, void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4]
        cmp eax, 011h
        je accept_009535c0
        cmp eax, 00Fh
        je accept_009535c0
        cmp eax, 005h
        je accept_009535c0
        cmp eax, 004h
        je accept_009535c0
        cmp eax, 002h
        je accept_009535c0
        cmp eax, 001h
        je accept_009535c0
        test eax, eax
        je accept_009535c0
        cmp eax, dword ptr [ecx + 0C4h]
        je accept_009535c0
        xor eax, eax
        ret 4
    accept_009535c0:
        mov eax, 1
        ret 4
    }
}

// COMPLETE[00953530,0095356B) 59B/21 instructions.
__declspec(naked) std::uint32_t __fastcall native_plane_derived_is_kind_00953530(
    const void*, void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4]
        cmp eax, 012h
        je accept_00953530
        cmp eax, 00Fh
        je accept_00953530
        cmp eax, 005h
        je accept_00953530
        cmp eax, 004h
        je accept_00953530
        cmp eax, 002h
        je accept_00953530
        cmp eax, 001h
        je accept_00953530
        test eax, eax
        je accept_00953530
        cmp eax, dword ptr [ecx + 0C4h]
        je accept_00953530
        xor eax, eax
        ret 4
    accept_00953530:
        mov eax, 1
        ret 4
    }
}

// COMPLETE[007DDA80,007DDABB) 59B/21 instructions.
__declspec(naked) std::uint32_t __fastcall native_plane_derived_is_kind_007dda80(
    const void*, void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4]
        cmp eax, 013h
        je accept_007dda80
        cmp eax, 00Fh
        je accept_007dda80
        cmp eax, 005h
        je accept_007dda80
        cmp eax, 004h
        je accept_007dda80
        cmp eax, 002h
        je accept_007dda80
        cmp eax, 001h
        je accept_007dda80
        test eax, eax
        je accept_007dda80
        cmp eax, dword ptr [ecx + 0C4h]
        je accept_007dda80
        xor eax, eax
        ret 4
    accept_007dda80:
        mov eax, 1
        ret 4
    }
}

// COMPLETE[0074E480,0074E4BB) 59B/21 instructions.
__declspec(naked) std::uint32_t __fastcall native_plane_derived_is_kind_0074e480(
    const void*, void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4]
        cmp eax, 014h
        je accept_0074e480
        cmp eax, 00Fh
        je accept_0074e480
        cmp eax, 005h
        je accept_0074e480
        cmp eax, 004h
        je accept_0074e480
        cmp eax, 002h
        je accept_0074e480
        cmp eax, 001h
        je accept_0074e480
        test eax, eax
        je accept_0074e480
        cmp eax, dword ptr [ecx + 0C4h]
        je accept_0074e480
        xor eax, eax
        ret 4
    accept_0074e480:
        mov eax, 1
        ret 4
    }
}

// COMPLETE[0084C9F0,0084CA30) 64B/23 instructions.
__declspec(naked) std::uint32_t __fastcall native_plane_derived_is_kind_0084c9f0(
    const void*, void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4]
        cmp eax, 015h
        je accept_0084c9f0
        cmp eax, 014h
        je accept_0084c9f0
        cmp eax, 00Fh
        je accept_0084c9f0
        cmp eax, 005h
        je accept_0084c9f0
        cmp eax, 004h
        je accept_0084c9f0
        cmp eax, 002h
        je accept_0084c9f0
        cmp eax, 001h
        je accept_0084c9f0
        test eax, eax
        je accept_0084c9f0
        cmp eax, dword ptr [ecx + 0C4h]
        je accept_0084c9f0
        xor eax, eax
        ret 4
    accept_0084c9f0:
        mov eax, 1
        ret 4
    }
}

// COMPLETE[0074E4E0,0074E520) 64B/23 instructions.
__declspec(naked) std::uint32_t __fastcall native_plane_derived_is_kind_0074e4e0(
    const void*, void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4]
        cmp eax, 016h
        je accept_0074e4e0
        cmp eax, 014h
        je accept_0074e4e0
        cmp eax, 00Fh
        je accept_0074e4e0
        cmp eax, 005h
        je accept_0074e4e0
        cmp eax, 004h
        je accept_0074e4e0
        cmp eax, 002h
        je accept_0074e4e0
        cmp eax, 001h
        je accept_0074e4e0
        test eax, eax
        je accept_0074e4e0
        cmp eax, dword ptr [ecx + 0C4h]
        je accept_0074e4e0
        xor eax, eax
        ret 4
    accept_0074e4e0:
        mov eax, 1
        ret 4
    }
}

// COMPLETE[009534A0,009534DB) 59B/21 instructions.
__declspec(naked) std::uint32_t __fastcall native_plane_derived_is_kind_009534a0(
    const void*, void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4]
        cmp eax, 017h
        je accept_009534a0
        cmp eax, 00Fh
        je accept_009534a0
        cmp eax, 005h
        je accept_009534a0
        cmp eax, 004h
        je accept_009534a0
        cmp eax, 002h
        je accept_009534a0
        cmp eax, 001h
        je accept_009534a0
        test eax, eax
        je accept_009534a0
        cmp eax, dword ptr [ecx + 0C4h]
        je accept_009534a0
        xor eax, eax
        ret 4
    accept_009534a0:
        mov eax, 1
        ret 4
    }
}

} // namespace bsp
