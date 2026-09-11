#include "bsp/native_frame_target_getters.hpp"

namespace bsp {

__declspec(naked) void* __fastcall
native_frame_targets_get_color_surface_00b1f6d0(const void*, std::uint32_t) noexcept {
    __asm {
        mov eax, dword ptr [ecx + edx * 4 + 8]
        ret
    }
}

__declspec(naked) void* __fastcall
native_frame_targets_get_depth_surface_00b1f6e0(const void*) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 18h]
        ret
    }
}

__declspec(naked) std::uint8_t __fastcall
native_frame_targets_get_srgb_write_byte_00b1f710(const void*) noexcept {
    __asm {
        mov al, byte ptr [ecx + 3ch]
        ret
    }
}

} // namespace bsp
