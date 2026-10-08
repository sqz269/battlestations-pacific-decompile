#include "bsp/native_ship_kind.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4, "native ship kind entries require Win32");

__declspec(naked) std::uint32_t __fastcall native_ship_base_is_kind_006dfe50(
    const void*, void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4]      // 006DFE50
        cmp eax, 6
        jz matched
        cmp eax, 5
        jz matched
        cmp eax, 4
        jz matched
        cmp eax, 2
        jz matched
        cmp eax, 1
        jz matched
        test eax, eax
        jz matched
        cmp eax, dword ptr [ecx + 0xc4]   // Sole late actual receiver read
        jz matched
        xor eax, eax
        ret 4
    matched:
        mov eax, 1                       // 006DFE7E
        ret 4
    }
}

__declspec(naked) std::uint32_t __fastcall native_destroyer_is_kind_006fe530(
    const void*, void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4]      // 006FE530
        cmp eax, 7
        jz matched
        cmp eax, 6
        jz matched
        cmp eax, 5
        jz matched
        cmp eax, 4
        jz matched
        cmp eax, 2
        jz matched
        cmp eax, 1
        jz matched
        test eax, eax
        jz matched
        cmp eax, dword ptr [ecx + 0xc4]   // Sole late actual receiver read
        jz matched
        xor eax, eax
        ret 4
    matched:
        mov eax, 1                       // 006FE563
        ret 4
    }
}

__declspec(naked) std::uint32_t __fastcall native_submarine_is_kind_00853050(
    const void*, void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4]      // 00853050
        cmp eax, 8
        jz matched
        cmp eax, 6
        jz matched
        cmp eax, 5
        jz matched
        cmp eax, 4
        jz matched
        cmp eax, 2
        jz matched
        cmp eax, 1
        jz matched
        test eax, eax
        jz matched
        cmp eax, dword ptr [ecx + 0xc4]   // Sole late actual receiver read
        jz matched
        xor eax, eax
        ret 4
    matched:
        mov eax, 1                       // 00853083
        ret 4
    }
}

__declspec(naked) std::uint32_t __fastcall native_mothership_is_kind_00758510(
    const void*, void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4]      // 00758510
        cmp eax, 9
        jz matched
        cmp eax, 6
        jz matched
        cmp eax, 5
        jz matched
        cmp eax, 4
        jz matched
        cmp eax, 2
        jz matched
        cmp eax, 1
        jz matched
        test eax, eax
        jz matched
        cmp eax, dword ptr [ecx + 0xc4]   // Sole late actual receiver read
        jz matched
        xor eax, eax
        ret 4
    matched:
        mov eax, 1                       // 00758543
        ret 4
    }
}

__declspec(naked) std::uint32_t __fastcall native_cruiser_is_kind_006fb3d0(
    const void*, void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4]      // 006FB3D0
        cmp eax, 10
        jz matched
        cmp eax, 6
        jz matched
        cmp eax, 5
        jz matched
        cmp eax, 4
        jz matched
        cmp eax, 2
        jz matched
        cmp eax, 1
        jz matched
        test eax, eax
        jz matched
        cmp eax, dword ptr [ecx + 0xc4]   // Sole late actual receiver read
        jz matched
        xor eax, eax
        ret 4
    matched:
        mov eax, 1                       // 006FB403
        ret 4
    }
}

__declspec(naked) std::uint32_t __fastcall native_cargo_is_kind_006eb230(
    const void*, void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4]      // 006EB230
        cmp eax, 11
        jz matched
        cmp eax, 6
        jz matched
        cmp eax, 5
        jz matched
        cmp eax, 4
        jz matched
        cmp eax, 2
        jz matched
        cmp eax, 1
        jz matched
        test eax, eax
        jz matched
        cmp eax, dword ptr [ecx + 0xc4]   // Sole late actual receiver read
        jz matched
        xor eax, eax
        ret 4
    matched:
        mov eax, 1                       // 006EB263
        ret 4
    }
}

__declspec(naked) std::uint32_t __fastcall native_landing_ship_is_kind_0074bc60(
    const void*, void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4]      // 0074BC60
        cmp eax, 12
        jz matched
        cmp eax, 6
        jz matched
        cmp eax, 5
        jz matched
        cmp eax, 4
        jz matched
        cmp eax, 2
        jz matched
        cmp eax, 1
        jz matched
        test eax, eax
        jz matched
        cmp eax, dword ptr [ecx + 0xc4]   // Sole late actual receiver read
        jz matched
        xor eax, eax
        ret 4
    matched:
        mov eax, 1                       // 0074BC93
        ret 4
    }
}

__declspec(naked) std::uint32_t __fastcall native_battleship_is_kind_006dfe90(
    const void*, void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4]      // 006DFE90
        cmp eax, 13
        jz matched
        cmp eax, 6
        jz matched
        cmp eax, 5
        jz matched
        cmp eax, 4
        jz matched
        cmp eax, 2
        jz matched
        cmp eax, 1
        jz matched
        test eax, eax
        jz matched
        cmp eax, dword ptr [ecx + 0xc4]   // Sole late actual receiver read
        jz matched
        xor eax, eax
        ret 4
    matched:
        mov eax, 1                       // 006DFEC3
        ret 4
    }
}

__declspec(naked) std::uint32_t __fastcall native_torpedo_boat_is_kind_00857dc0(
    const void*, void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4]      // 00857DC0
        cmp eax, 14
        jz matched
        cmp eax, 6
        jz matched
        cmp eax, 5
        jz matched
        cmp eax, 4
        jz matched
        cmp eax, 2
        jz matched
        cmp eax, 1
        jz matched
        test eax, eax
        jz matched
        cmp eax, dword ptr [ecx + 0xc4]   // Sole late actual receiver read
        jz matched
        xor eax, eax
        ret 4
    matched:
        mov eax, 1                       // 00857DF3
        ret 4
    }
}

} // namespace bsp
