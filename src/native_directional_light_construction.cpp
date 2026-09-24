#include "bsp/native_directional_light_construction.hpp"
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw directional light construction requires MSVC Win32.
#endif

namespace bsp {
namespace {
// Source adapter only: ECX=actual slot, EDX=borrowed one cell, stack=CE7820.
// Raw MOVSS operations retain the native read/store order and bit patterns.
__declspec(naked) void* __fastcall finish_light(void*,
    const volatile std::uint32_t*, const volatile std::uint32_t*) {
    __asm {
        movss xmm0, dword ptr [edx] // B7C4CD, after B6F5A0
        xorps xmm1, xmm1
        mov dword ptr [ecx], 0d62f58h
        xor eax, eax
        mov dword ptr [ecx + 174h], eax
        mov dword ptr [ecx + 178h], eax
        mov dword ptr [ecx + 17ch], eax
        mov dword ptr [ecx + 180h], eax
        movss dword ptr [ecx + 184h], xmm0
        movss dword ptr [ecx + 188h], xmm0
        movss dword ptr [ecx + 18ch], xmm0
        movss dword ptr [ecx + 190h], xmm0
        movss dword ptr [ecx + 1a4h], xmm0
        movss dword ptr [ecx + 1a8h], xmm0
        movss dword ptr [ecx + 1ach], xmm0
        movss dword ptr [ecx + 1b0h], xmm0
        movss dword ptr [ecx + 1c4h], xmm1
        movss dword ptr [ecx + 1c8h], xmm1
        movss dword ptr [ecx + 1cch], xmm1
        mov edx, dword ptr [esp + 4]
        movss xmm1, dword ptr [edx] // B7C550, after the three zero stores
        movss dword ptr [ecx + 1d0h], xmm0
        movss dword ptr [ecx + 1d4h], xmm1
        movss dword ptr [ecx + 1d8h], xmm0
        mov eax, ecx
        ret 4
    }
}
}

void* construct_native_light_raw_00b7c4c0(void* actual_slot,
    std::size_t slot_bytes, const void* actual_name_header,
    NativeStringRawPoolContext& strings, const NativeNodeRawConstants& constants,
    const volatile std::uint32_t& sixty_four) {
    if (!actual_slot || (reinterpret_cast<std::uintptr_t>(actual_slot) & 3u) ||
        slot_bytes < 0x1f0u)
        throw std::invalid_argument("raw light construction requires an actual aligned 1F0h slot");
    construct_native_node_00b6f5a0(actual_slot, slot_bytes,
        actual_name_header, strings, constants); // B7C4C8
    return finish_light(actual_slot, &constants.one_00d7a24c, &sixty_four);
}

void* construct_native_directional_light_raw_00b7c6b0(void* actual_slot,
    std::size_t slot_bytes, const void* actual_name_header,
    NativeStringRawPoolContext& strings, const NativeNodeRawConstants& constants,
    const volatile std::uint32_t& sixty_four) {
    construct_native_light_raw_00b7c4c0(actual_slot, slot_bytes,
        actual_name_header, strings, constants, sixty_four); // B7C6B8
    *static_cast<volatile std::uint32_t*>(actual_slot) = 0x00d62fb0u;
    return actual_slot;
}

} // namespace bsp
