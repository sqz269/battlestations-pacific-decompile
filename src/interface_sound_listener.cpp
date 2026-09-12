#include "bsp/interface_sound_listener.hpp"
#include "bsp/native_camera_matrix_copy.hpp"
#include "bsp/native_camera_world.hpp"
#include "bsp/system_camera_axes.hpp"

#include <cstddef>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Interface sound listener requires MSVC Win32 x87/SSE instructions.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);

// MOV preserves unaligned raw access and keeps each publication/field load at
// its native point. This does not interpret the native object as a companion.
__declspec(naked) std::uint32_t __fastcall load_word(const volatile void*) noexcept {
    __asm {
        mov eax, dword ptr [ecx]
        ret
    }
}
__declspec(naked) std::uint32_t __fastcall load_byte(const volatile void*) noexcept {
    __asm {
        movzx eax, byte ptr [ecx]
        ret
    }
}
void* load_pointer(const void* object, std::size_t offset) noexcept {
    return reinterpret_cast<void*>(load_word(
        static_cast<const unsigned char*>(object) + offset));
}

__declspec(naked) void __fastcall copy_velocity_x87(void*, const void*) {
    __asm {
        fld dword ptr [edx]
        fstp dword ptr [ecx]
        fld dword ptr [edx + 4]
        fstp dword ptr [ecx + 4]
        fld dword ptr [edx + 8]
        fstp dword ptr [ecx + 8]
        ret
    }
}
__declspec(naked) void __fastcall copy_fallback_sse(void*, const volatile float*) {
    __asm {
        movss xmm0, dword ptr [edx]
        movss dword ptr [ecx], xmm0
        movss xmm0, dword ptr [edx + 4]
        movss dword ptr [ecx + 4], xmm0
        movss xmm0, dword ptr [edx + 8]
        movss dword ptr [ecx + 8], xmm0
        ret
    }
}
struct VelocityMath {
    const volatile float* threshold;
    const volatile float* compare;
    const volatile double* scale;
    const CameraAxesCrtAccess* crt;
};
static_assert(offsetof(VelocityMath, crt) == 12);

// Reassembled 0068A7B1..0068A86D. Added pointer bindings only; retain native
// x87 operand order, single-precision spills, SSE comparison/NaN branch and
// the surprising overwrite of squared length by float(comparison_result).
__declspec(naked) void __fastcall copy_target_velocity(
    const void*, void*, const VelocityMath*) {
    __asm {
        push esi
        push edi
        sub esp, 10h
        mov eax, ecx
        mov esi, edx
        mov edi, dword ptr [esp + 1ch]
        fld dword ptr [eax] // 0068A7B1
        fstp dword ptr [esi]
        fld dword ptr [eax + 4]
        fstp dword ptr [esi + 4]
        movss xmm0, dword ptr [eax + 8]
        fld dword ptr [esi + 4]
        movss dword ptr [esp], xmm0
        fstp dword ptr [esp + 4]
        movss dword ptr [esi + 8], xmm0
        fld dword ptr [esp + 4]
        fld dword ptr [esi]
        fstp dword ptr [esp + 8]
        fld dword ptr [esp + 8]
        fld dword ptr [esp]
        fld st(1)
        fmulp st(2), st(0)
        fld st(2)
        fmulp st(3), st(0)
        fxch st(1)
        faddp st(2), st(0)
        fmul st(0), st(0)
        faddp st(1), st(0)
        fstp dword ptr [esp]
        mov edx, dword ptr [edi]
        fld dword ptr [edx] // 0068A7FC, live CE3D64
        fld dword ptr [esp]
        fcomip st(0), st(1)
        fstp st(0)
        jbe comparison_false
        mov eax, 1
        jmp comparison_ready
    comparison_false:
        xor eax, eax
    comparison_ready:
        cvtsi2ss xmm0, eax
        mov edx, dword ptr [edi + 4]
        ucomiss xmm0, dword ptr [edx] // 0068A819
        lahf
        test ah, 44h
        movss dword ptr [esp], xmm0 // overwrites squared length
        jnp finished
        fld dword ptr [esp]
        mov ecx, dword ptr [edi + 0ch]
        call native_crt_sqrt_st0_00bf7030 // 0068A830, ST0 operand/result
        fstp dword ptr [esp]
        fld dword ptr [esp]
        mov edx, dword ptr [edi + 8]
        fdivr qword ptr [edx]
        fstp dword ptr [esp]
        fld dword ptr [esp + 8]
        fld dword ptr [esp]
        fld st(0)
        fmulp st(2), st(0)
        fxch st(1)
        fstp dword ptr [esi]
        fld dword ptr [esp + 4]
        fmul st(0), st(1)
        fstp dword ptr [esi + 4]
        fmul dword ptr [esi + 8]
        fstp dword ptr [esi + 8]
    finished:
        add esp, 10h
        pop edi
        pop esi
        ret 4
    }
}
} // namespace

void get_interface_sound_listener_0068a670(const void* actual_interface,
    void* matrix64, void* velocity12, InterfaceSoundListenerContext& context) {
    std::uint32_t matrix_scratch[16]; // native unwritten local_40
    alignas(4) unsigned char velocity_scratch[12]; // native unwritten local_4C
    const void* matrix_source;
    auto* game = context.game_00e188a8; // 0068A670: captured for both initial tests
    if (load_pointer(game, 0x1ed4)) {
        auto* target = load_pointer(game, 0x1ed4);
        matrix_source = context.calls.call_slot120(target, matrix_scratch);
    } else if (load_pointer(game, 0x19fc)) {
        auto* camera = load_pointer(game, 0x19fc); // captured across refresh
        if (!(load_byte(static_cast<unsigned char*>(camera) + 0x5c) & 2))
            refresh_native_camera_world_00b6db70(camera); // 0068A6BB
        matrix_source = static_cast<unsigned char*>(camera) + 0xf0;
    } else {
        // Native MOVSS broadcast: one is loaded once, no float conversion.
        const auto one = load_word(&context.one_00d7a24c);
        for (unsigned i = 0; i != 16; ++i)
            matrix_scratch[i] = i % 5 == 0 ? one : 0;
        matrix_source = matrix_scratch;
    }
    copy_native_camera_matrix_004134f0(matrix64, nullptr, matrix_source); // 0068A73A

    auto* controlled = context.controlled_00e188d8; // reload after matrix callback
    if (controlled) {
        const auto id = load_word(static_cast<const unsigned char*>(actual_interface) + 4);
        if (id != 0x29 && id != 0x2b && id != 0x2c && id != 0x2d && id != 0x34) {
            const auto* source = context.calls.call_slot34(controlled, velocity_scratch);
            copy_velocity_x87(velocity12, source); // 0068A771..0068A784
            return;
        }
    }
    game = context.game_00e188a8; // 0068A78D: do not reuse pre-callback game
    if (load_pointer(game, 0x1ed4)) {
        auto* target = load_pointer(game, 0x1ed4); // current target, not matrix target
        const auto* source = context.calls.call_slot34(target, velocity_scratch);
        const VelocityMath math{&context.threshold_00ce3d64, &context.compare_00d7a218,
            &context.scale_00d7a220, &context.crt};
        copy_target_velocity(source, velocity12, &math);
    } else {
        copy_fallback_sse(velocity12, context.fallback_00f87574); // 0068A86E
    }
}
} // namespace bsp
