#include "bsp/native_render_state_leaves.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native render state leaves require MSVC Win32 assembly.
#endif

namespace bsp {
__declspec(naked) std::uint32_t __fastcall
native_render_can_execute_command_00b20240(const void*) noexcept {
    __asm {
        cmp dword ptr [ecx + 1d90h], 0
        jnz unavailable
        cmp byte ptr [ecx + 1d8ah], 0
        jnz unavailable
        mov eax, 1
        ret
    unavailable:
        xor eax, eax
        ret
    }
}

__declspec(naked) std::uint8_t __fastcall
native_render_is_frame_active_00b1fe20(const void*) noexcept {
    __asm {
        cmp dword ptr [ecx + 1998h], 0
        setnz al
        ret
    }
}

__declspec(naked) void __stdcall
native_render_command_hook_no_op_00b20210(const void*) noexcept {
    __asm { ret 4 }
}

__declspec(naked) std::int32_t __fastcall
native_render_batch_count_00b51b20(const NativeRenderBatchStorage*) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 10h]
        ret
    }
}
} // namespace bsp
