#include "bsp/native_authored_path_points.hpp"

#include "bsp/native_scene_path_point.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4);

__declspec(naked) void* __fastcall get_native_authored_path_point_007b10a0(
    void*, void*, void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 8]
        mov ecx, dword ptr [ecx + 8]
        push esi
        mov esi, dword ptr [esp + 8]
        push eax
        push esi
        call transform_native_scene_path_point_007af800
        mov eax, esi
        pop esi
        ret 8
    }
}

__declspec(naked) void* __fastcall get_last_native_authored_path_point_007b10c0(
    void*, void*, void*) {
    __asm {
        mov ecx, dword ptr [ecx + 8]
        mov edx, dword ptr [ecx + 8]
        test edx, edx
        jne have_first
        push esi
        mov esi, dword ptr [esp + 8]
        xor eax, eax
        mov eax, 0ffffffffh
        push eax
        push esi
        call transform_native_scene_path_point_007af800
        mov eax, esi
        pop esi
        ret 4
    have_first:
        mov eax, dword ptr [ecx + 0ch]
        sub eax, edx
        sar eax, 2
        push esi
        mov esi, dword ptr [esp + 8]
        add eax, -1
        push eax
        push esi
        call transform_native_scene_path_point_007af800
        mov eax, esi
        pop esi
        ret 4
    }
}

} // namespace bsp
