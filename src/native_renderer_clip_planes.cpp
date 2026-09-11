#include "bsp/native_renderer_clip_planes.hpp"
#include "bsp/native_renderer_cached_states.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <exception>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
Word word(const void* base, Word byte_offset) noexcept {
    Word result;
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov eax, dword ptr [eax + edx] }
    __asm { mov result, eax }
    return result;
}
void put(void* base, Word byte_offset, Word value) noexcept {
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov ecx, value }
    __asm { mov dword ptr [eax + edx], ecx }
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
struct GuardCleanup {
    const NativeRendererOptionalGuardStorage& guard;
    NativeRendererSynchronizationGlobals& globals;
    bool armed = true;
    ~GuardCleanup() noexcept { if (armed) unwind_guard(guard, globals); }
};
using SetClipPlane = std::int32_t (__stdcall*)(void*, Word, const void*);
} // namespace

void set_native_renderer_clip_plane_00b23e50(void* renderer, Word index,
    const void* input, NativeRendererSynchronizationGlobals& globals) {
    NativeRendererOptionalGuardStorage guard;
    if (globals.mode_00 != 0) {
        guard.renderer_04 = renderer;
        guard.entered_00 = enter_native_renderer_optional_guard_00b33ad0(renderer, globals);
    }
    void* destination;
    // Keep each memory operation in native order. In particular, do not use
    // a temporary float4 or memcpy: overlapping input can change later reads.
    __asm {
        mov ecx, input
        fld dword ptr [ecx]
        mov eax, index
        shl eax, 4
        mov edx, renderer
        lea eax, [eax + edx + 190ch]
        mov destination, eax
        fstp dword ptr [eax]
        fld dword ptr [ecx + 4]
        fstp dword ptr [eax + 4]
    }
    GuardCleanup cleanup{guard, globals}; // Native B23EA1, after second FSTP.
    __asm {
        mov eax, destination
        mov ecx, input
        fld dword ptr [ecx + 8]
        fstp dword ptr [eax + 8]
        fld dword ptr [ecx + 0ch]
        fstp dword ptr [eax + 0ch]
    }
    auto* const device = reinterpret_cast<void*>(word(renderer, 0x1a10));
    auto* const table = reinterpret_cast<void*>(word(device, 0));
    const auto call = reinterpret_cast<SetClipPlane>(word(table, 0xdc));
    (void)call(device, index, input);
    const auto current_mode = globals.mode_00;
    cleanup.armed = false;
    if (current_mode != 0) {
        const Word ignored = guard.entered_00;
        const void* const saved_renderer = guard.renderer_04;
        leave_native_renderer_optional_guard_00b33b00(saved_renderer, ignored, globals);
    }
}

void append_native_renderer_clip_plane_00b25040(void* renderer,
    const void* input, NativeRendererSynchronizationGlobals& globals) {
    set_native_renderer_clip_plane_00b23e50(renderer, word(renderer, 0x19ec), input, globals);
    __asm { mov eax, renderer }
    __asm { add dword ptr [eax + 19ech], 1 }
    const Word active = word(renderer, 0x19ec);
    const Word mask = (1u << (active & 31u)) - 1u;
    set_native_renderer_render_state_00b24460(renderer, 0x98, mask, globals);
}

void restore_native_renderer_pending_clip_planes_00b25080(void* renderer,
    NativeRendererSynchronizationGlobals& globals) {
    const Word pending = word(renderer, 0x19f0);
    const Word mask = (1u << (pending & 31u)) - 1u;
    set_native_renderer_render_state_00b24460(renderer, 0x98, mask, globals);
    put(renderer, 0x19ec, word(renderer, 0x19f0));
}

} // namespace bsp
