#include "bsp/native_renderer_viewport_clear.hpp"
#include "bsp/native_renderer_cached_states.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <exception>

namespace bsp {
__declspec(naked) const void* __fastcall native_viewport_origin_00b1f730(const void*) noexcept {
    __asm { lea eax, [ecx + 8] }
    __asm { ret }
}
__declspec(naked) const void* __fastcall native_viewport_size_00b1f740(const void*) noexcept {
    __asm { lea eax, [ecx + 10h] }
    __asm { ret }
}
__declspec(naked) const void* __fastcall native_viewport_scissor_rect_00b1f7b0(const void*) noexcept {
    __asm { lea eax, [ecx + 24h] }
    __asm { ret }
}
__declspec(naked) std::uint8_t __fastcall native_viewport_scissor_enabled_00b1f7d0(const void*) noexcept {
    __asm { mov al, byte ptr [ecx + 20h] }
    __asm { ret }
}

namespace {
using Word = std::uint32_t;
Word word(const void* p, Word byte_offset) noexcept {
    Word value;
    __asm { mov ecx, p }
    __asm { mov edx, byte_offset }
    __asm { mov eax, [ecx + edx] }
    __asm { mov value, eax }
    return value;
}
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
void normal_leave(Cleanup& cleanup, const NativeRendererOptionalGuardStorage& guard,
    NativeRendererSynchronizationGlobals& globals) {
    const auto current_mode = globals.mode_00;
    cleanup.armed = false;
    if (current_mode != 0) {
        const Word ignored = guard.entered_00;
        const void* const renderer = guard.renderer_04;
        leave_native_renderer_optional_guard_00b33b00(renderer, ignored, globals);
    }
}

// Receive the public callee's actual argument address, not a C++ parameter
// snapshot. The assembly retains four separate getter calls and the device /
// constant / vtable order; Guard state0 corresponds to original B267A8.
void __cdecl viewport_body(void* renderer, const NativeRendererViewportClearContext* context,
    const void* argument_slot) {
    auto& globals = *context->synchronization_00;
    NativeRendererOptionalGuardStorage guard;
    if (globals.mode_00 != 0) {
        guard.renderer_04 = renderer;
        guard.entered_00 = enter_native_renderer_optional_guard_00b33ad0(renderer, globals);
    }
    void* const viewport = reinterpret_cast<void*>(word(argument_slot, 0));
    Cleanup cleanup{guard, globals, true};
    Word values[6];
    __asm {
        mov edi, renderer
        mov esi, viewport
        mov ecx, esi
        call native_viewport_origin_00b1f730
        mov eax, [eax]
        mov ecx, esi
        mov values[0], eax
        call native_viewport_origin_00b1f730
        mov ecx, [eax + 4]
        mov values[4], ecx
        mov ecx, esi
        call native_viewport_size_00b1f740
        mov edx, [eax]
        mov ecx, esi
        mov values[8], edx
        call native_viewport_size_00b1f740
        mov eax, [eax + 4]
        xorps xmm0, xmm0
        mov values[0ch], eax
        mov eax, [edi + 1a10h]
        movss values[10h], xmm0
        mov edx, context
        mov edx, [edx + 4]
        movss xmm0, dword ptr [edx]
        lea edx, values
        movss values[14h], xmm0
        mov ecx, [eax]
        push edx
        push eax
        mov eax, [ecx + 0bch]
        call eax
        add dword ptr [edi + 1bd0h], 1
        mov ecx, esi
        mov [edi + 1904h], esi
        call native_viewport_scissor_enabled_00b1f7d0
        movzx ecx, al
        mov edx, context
        push dword ptr [edx]
        push ecx
        push 0aeh
        push edi
        call set_native_renderer_render_state_00b24460
        add esp, 10h
        mov ecx, esi
        call native_viewport_scissor_enabled_00b1f7d0
        test al, al
        jz no_scissor
        mov edx, [edi + 1a10h]
        mov ebx, [edx]
        mov ecx, esi
        call native_viewport_scissor_rect_00b1f7b0
        mov ecx, [ebx + 12ch]
        push eax
        mov eax, [edi + 1a10h]
        push eax
        call ecx
    no_scissor:
    }
    normal_leave(cleanup, guard, globals);
}

// This local record represents the native outgoing argument values, captured
// in their original order before cleanup arms. Depth is stored by x87 between
// color loading and subsequent rectangle/count reads, not converted by C++.
struct ClearCall {
    void* device;
    Word count;
    const void* rectangles;
    Word flags;
    Word color;
    Word depth_bits;
    Word stencil;
    void* entry;
};
static_assert(sizeof(ClearCall) == 32);
void __cdecl clear_body(void* renderer, const NativeRendererViewportClearContext* context,
    const void* argument_slots) {
    const Word flags = word(argument_slots, 8); // original B2144A, before guard
    if (flags == 0) return;
    auto& globals = *context->synchronization_00;
    NativeRendererOptionalGuardStorage guard;
    if (globals.mode_00 != 0) {
        guard.renderer_04 = renderer;
        guard.entered_00 = enter_native_renderer_optional_guard_00b33ad0(renderer, globals);
    }
    ClearCall outgoing;
    __asm {
        mov edi, argument_slots
        fld dword ptr [edi + 10h]
        mov esi, renderer
        mov eax, [esi + 1a10h]
        mov edx, [edi + 14h]
        mov ecx, [eax]
        mov outgoing.stencil, edx
        mov edx, [edi + 0ch]
        mov edx, [edx]
        fstp outgoing.depth_bits
        mov outgoing.color, edx
        mov edx, [edi + 4]
        mov ebx, flags
        mov outgoing.flags, ebx
        mov outgoing.rectangles, edx
        mov edx, [edi]
        mov outgoing.count, edx
        mov outgoing.device, eax
        mov eax, [ecx + 0ach]
        mov outgoing.entry, eax
    }
    Cleanup cleanup{guard, globals, false};
    __asm {
        push outgoing.stencil
        push outgoing.depth_bits
        push outgoing.color
        push outgoing.flags
        push outgoing.rectangles
        push outgoing.count
        push outgoing.device
        mov cleanup.armed, 1 // original B21498, after all seven pushes
        call outgoing.entry
        mov eax, renderer
        add dword ptr [eax + 1bd4h], 1
    }
    normal_leave(cleanup, guard, globals);
}
} // namespace

// New EDX context; preserve public callee argument slots until their exact
// loads in the full bodies. The naked adapters neither dereference context nor
// load any original argument values. RET sizes remain native 4 and 18h.
__declspec(naked) void __fastcall bind_native_renderer_viewport_00b26770(void*,
    const NativeRendererViewportClearContext*, void*) {
    __asm { lea eax, [esp + 4] }
    __asm { push eax }
    __asm { push edx }
    __asm { push ecx }
    __asm { call viewport_body }
    __asm { add esp, 0ch }
    __asm { ret 4 }
}
__declspec(naked) void __fastcall clear_native_renderer_00b21430(void*,
    const NativeRendererViewportClearContext*, Word, const void*, Word,
    const void*, float, Word) {
    __asm { lea eax, [esp + 4] }
    __asm { push eax }
    __asm { push edx }
    __asm { push ecx }
    __asm { call clear_body }
    __asm { add esp, 0ch }
    __asm { ret 18h }
}
} // namespace bsp
