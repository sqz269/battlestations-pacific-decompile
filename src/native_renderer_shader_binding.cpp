#include "bsp/native_renderer_shader_binding.hpp"

#include <exception>
#include <windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native renderer shader binding requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
using Guard = NativeRendererOptionalGuardStorage;
using Globals = NativeRendererSynchronizationGlobals;

int terminate_cleanup_exception(unsigned long code) noexcept {
    if (code == 0xe06d7363u) std::terminate();
    return EXCEPTION_CONTINUE_SEARCH;
}

void unwind_guard(const Guard& guard, Globals& globals) noexcept {
    __try {
        destroy_native_renderer_optional_guard_00b21110(guard, globals);
    } __except (terminate_cleanup_exception(GetExceptionCode())) {
        __assume(0);
    }
}

struct Cleanup {
    const Guard& guard;
    Globals& globals;
    volatile bool armed;
    ~Cleanup() noexcept {
        if (armed) unwind_guard(guard, globals);
    }
};

template<Word CacheOffset, Word CounterOffset, Word DeviceSlot>
void bind_body(void* renderer, Globals* globals, const void* argument_slot) {
    Guard guard; // Native storage is not initialized when entry mode is zero.
    Cleanup cleanup{guard, *globals, false};
    if (globals->mode_00 != 0) {
        guard.renderer_04 = renderer;
        guard.entered_00 = enter_native_renderer_optional_guard_00b33ad0(
            renderer, *globals);
    }
    enum : Word { cache = CacheOffset, counter = CounterOffset, slot = DeviceSlot };
    __asm {
        mov esi, renderer
        mov ecx, dword ptr [esi + cache]
        mov edx, argument_slot
        mov eax, dword ptr [edx]
        mov cleanup.armed, 1
        test ecx, ecx
        je publish_shader
        test eax, eax
        je publish_shader
        mov edx, dword ptr [ecx + 8]
        cmp edx, dword ptr [eax + 8]
        je finished_body
    publish_shader:
        test eax, eax
        mov dword ptr [esi + cache], eax
        je incoming_null
        mov edx, dword ptr [esi + 01a10h]
        mov eax, dword ptr [eax + 8]
        mov ecx, dword ptr [edx]
        push eax
        push edx
        mov ecx, dword ptr [ecx + slot]
        call ecx
        jmp count_call
    incoming_null:
        test ecx, ecx
        je finished_body
        mov eax, dword ptr [esi + 01a10h]
        mov ecx, dword ptr [eax]
        mov edx, dword ptr [ecx + slot]
        push 0
        push eax
        call edx
    count_call:
        add dword ptr [esi + counter], 1
    finished_body:
    }
    const auto current_mode = globals->mode_00;
    cleanup.armed = false; // Consumed before normal leave, including if it throws.
    if (current_mode != 0) {
        const Word ignored_saved_result = guard.entered_00;
        const void* const guarded_renderer = guard.renderer_04;
        leave_native_renderer_optional_guard_00b33b00(
            guarded_renderer, ignored_saved_result, *globals);
    }
}

void __cdecl pixel_body(void* renderer, Globals* globals, const void* argument_slot) {
    bind_body<0x176c, 0x1bbc, 0x1ac>(renderer, globals, argument_slot);
}
void __cdecl vertex_body(void* renderer, Globals* globals, const void* argument_slot) {
    bind_body<0x1770, 0x1bc0, 0x170>(renderer, globals, argument_slot);
}
} // namespace

__declspec(naked) void __fastcall bind_native_renderer_pixel_shader_00b21c20(
    void*, Globals*, const void*) {
    __asm { lea eax, [esp + 4] }
    __asm { push eax }
    __asm { push edx }
    __asm { push ecx }
    __asm { call pixel_body }
    __asm { add esp, 0ch }
    __asm { ret 4 }
}

__declspec(naked) void __fastcall bind_native_renderer_vertex_shader_00b21d10(
    void*, Globals*, const void*) {
    __asm { lea eax, [esp + 4] }
    __asm { push eax }
    __asm { push edx }
    __asm { push ecx }
    __asm { call vertex_body }
    __asm { add esp, 0ch }
    __asm { ret 4 }
}
} // namespace bsp
