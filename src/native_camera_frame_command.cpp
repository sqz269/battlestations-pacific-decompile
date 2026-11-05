#include "bsp/native_camera_frame_command.hpp"
#include "bsp/wire_goal4_chain567_switches.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native camera frame command requires MSVC Win32 x87 assembly.
#endif

namespace bsp {
// Integer-only context and concrete-profile plumbing surrounds the original
// register/stack/FP schedule. The saved context is outside the native frame.
__declspec(naked) void __fastcall execute_native_camera_frame_command_00b71360(
    void*, const NativeCameraFrameCommandContext*) {
    __asm {
        push edx
        push esi // B71360
        mov esi, ecx
        cmp byte ptr [esi + 17ch], 0
        je finished

        mov eax, [esp + 4]
        mov ecx, [eax]
        mov ecx, [ecx] // B7136C: current renderer
        mov eax, [ecx] // B71372: current profile
        cmp eax, 0d5f0a8h
        jne unsupported_profile
        mov edx, [esp + 4]
        mov eax, [edx + 4]
        mov edx, [eax + 0a0h] // B71374: current slot+A0
        cmp edx, 0b285a0h
        jne unsupported_profile
        push esi
        mov edx, [esp + 8]
        mov edx, [edx + 8]
        call prepare_native_renderer_camera_00b285a0 // B7137B, RET4

        mov eax, [esp + 4]
        mov ecx, [eax]
        mov ecx, [ecx] // B7137D: reread current renderer
        mov eax, [ecx] // B71383: reread profile
        cmp eax, 0d5f0a8h
        jne unsupported_profile
        mov edx, [esi + 180h] // B71385: borrowed viewport, before slot load
        mov eax, [esp + 4]
        mov eax, [eax + 4]
        mov eax, [eax + 0a4h] // B7138B
        cmp eax, 0b26770h
        jne unsupported_profile
        push edx
        mov edx, [esp + 8]
        mov edx, [edx + 0ch]
        call bind_native_renderer_viewport_00b26770 // B71392, RET4

        fld dword ptr [esi + 18ch] // B71394: before stencil/global/profile
        mov eax, [esi + 194h]
        mov edx, [esp + 4]
        mov ecx, [edx]
        mov ecx, [ecx] // B713A0: reread current renderer
        cmp dword ptr [ecx], 0d5f0a8h // B713A6: reread profile
        jne unsupported_profile
        mov edx, [edx + 4]
        mov edx, [edx + 8] // B713A8: current slot+08
        cmp edx, 0b21430h
        jne unsupported_profile
        push eax // stencil
        push ecx // reserve original outgoing depth word
        fstp dword ptr [esp] // B713AD: including flags=0
    }
    if constexpr (kWireGoal4Chain567Bound) {
        // Lend the SAME actual camera to the clear adapter. It calls the
        // existing 00B6FE60 at the original color-load site AFTER guard entry.
        __asm {
            mov eax, esi
        }
    } else {
        __asm {
        lea eax, [esi + 190h]
        }
    }
    __asm {
        push eax // actual camera on the bound arm; original color address otherwise
        mov eax, [esi + 188h] // B713B7: flags read after depth store
        push eax
        push 0 // rectangles
        push 0 // count
        mov edx, [esp + 1ch]
        mov edx, [edx + 0ch]
    }
    if constexpr (kWireGoal4Chain567Bound) {
        __asm { call clear_native_renderer_camera_color } // B713C2, RET18h
    } else {
        __asm { call clear_native_renderer_00b21430 } // B713C2, RET18h
    }
    __asm {
    finished:
        pop esi
        pop edx
        ret
    unsupported_profile:
        _emit 0x0f
        _emit 0x0b
    }
}
} // namespace bsp
