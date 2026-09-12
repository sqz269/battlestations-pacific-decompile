#include "bsp/native_shader_constants.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <exception>

namespace bsp {
namespace {
using Word = std::uint32_t;

int terminate_cleanup_exception(unsigned long code) noexcept {
    if (code == 0xe06d7363u) std::terminate();
    return EXCEPTION_CONTINUE_SEARCH;
}
void unwind_guard(const NativeRendererOptionalGuardStorage& guard,
    NativeRendererSynchronizationGlobals& globals) noexcept {
    __try {
        destroy_native_renderer_optional_guard_00b21110(guard, globals);
    } __except (terminate_cleanup_exception(GetExceptionCode())) {
        __assume(0);
    }
}
struct Cleanup {
    const NativeRendererOptionalGuardStorage& guard;
    NativeRendererSynchronizationGlobals& globals;
    volatile bool armed;
    ~Cleanup() noexcept { if (armed) unwind_guard(guard, globals); }
};

template<Word DeviceSlot, Word CallCounter, Word ByteCounter>
void __cdecl upload_body(void* renderer,
    NativeRendererSynchronizationGlobals* synchronization, const void* slots) {
    Word count;
    __asm {
        mov eax, slots
        mov eax, [eax + 8]
        mov count, eax
    }
    if (count == 0) return;

    auto& globals = *synchronization;
    NativeRendererOptionalGuardStorage guard;
    if (globals.mode_00 != 0) {
        guard.renderer_04 = renderer;
        guard.entered_00 = enter_native_renderer_optional_guard_00b33ad0(renderer, globals);
    }
    Cleanup cleanup{guard, globals, false};
    // Native data/device/table/start/entry observations and outgoing pushes.
    // The source cleanup arms at the native state-0 store after all pushes.
    enum : Word { slot = DeviceSlot, calls = CallCounter, bytes = ByteCounter };
    __asm {
        mov esi, renderer
        mov edi, count
        mov ebx, slots
        mov edx, [ebx + 4]
        mov eax, [esi + 1a10h]
        mov ecx, [eax]
        push edi
        push edx
        mov edx, [ebx]
        push edx
        push eax
        mov eax, [ecx + slot]
        mov cleanup.armed, 1
        call eax
        add dword ptr [esi + calls], 1
        shl edi, 4
        add dword ptr [esi + bytes], edi
    }
    const auto current_mode = globals.mode_00;
    cleanup.armed = false;
    if (current_mode != 0) {
        const Word ignored = guard.entered_00;
        const void* const saved_renderer = guard.renderer_04;
        leave_native_renderer_optional_guard_00b33b00(saved_renderer, ignored, globals);
    }
}

void __cdecl vertex_body(void* renderer,
    NativeRendererSynchronizationGlobals* globals, const void* slots) {
    upload_body<0x178, 0x1bd8, 0x1be0>(renderer, globals, slots);
}
void __cdecl pixel_body(void* renderer,
    NativeRendererSynchronizationGlobals* globals, const void* slots) {
    upload_body<0x1b4, 0x1bdc, 0x1be4>(renderer, globals, slots);
}
} // namespace

// Borrow the public callee's live slots without copying original arguments.
// EDX is the only added context; the original ECX and stack layout remain.
__declspec(naked) void __fastcall set_native_vertex_shader_constants_f_00b21820(
    void*, NativeRendererSynchronizationGlobals*, Word, const float*, Word) {
    __asm { lea eax, [esp + 4] }
    __asm { push eax }
    __asm { push edx }
    __asm { push ecx }
    __asm { call vertex_body }
    __asm { add esp, 0ch }
    __asm { ret 0ch }
}
__declspec(naked) void __fastcall set_native_pixel_shader_constants_f_00b218c0(
    void*, NativeRendererSynchronizationGlobals*, Word, const float*, Word) {
    __asm { lea eax, [esp + 4] }
    __asm { push eax }
    __asm { push edx }
    __asm { push ecx }
    __asm { call pixel_body }
    __asm { add esp, 0ch }
    __asm { ret 0ch }
}
} // namespace bsp
