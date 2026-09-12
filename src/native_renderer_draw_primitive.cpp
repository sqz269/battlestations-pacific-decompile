#include "bsp/native_renderer_draw_primitive.hpp"
#include "bsp/native_renderer_indexed_draw.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <exception>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
Word word(const volatile void* base, Word byte_offset) noexcept {
    Word result;
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov eax, dword ptr [eax + edx] }
    __asm { mov result, eax }
    return result;
}
std::uint8_t byte(const void* base, Word byte_offset) noexcept {
    std::uint8_t result;
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov al, byte ptr [eax + edx] }
    __asm { mov result, al }
    return result;
}
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
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
using DrawPrimitive = std::int32_t (__stdcall*)(void*, Word, Word, Word);
} // namespace

void draw_native_renderer_primitive_00b21b40(void* renderer,
    Word primitive_type, Word start_vertex, Word primitive_count,
    NativeRendererSynchronizationGlobals& globals) {
    if (word(renderer, 0x1d90) != 0) return;
    if (byte(renderer, 0x1d8a) != 0) return;
    (void)native_viewport_size_address_00b1f740(pointer(word(renderer, 0x1904)));
    if (primitive_count == 0) return;

    NativeRendererOptionalGuardStorage guard;
    if (globals.mode_00 != 0) {
        guard.renderer_04 = renderer;
        guard.entered_00 = enter_native_renderer_optional_guard_00b33ad0(renderer, globals);
    }
    auto* const device = pointer(word(renderer, 0x1a10));
    const auto* const table = pointer(word(device, 0));
    const auto call = reinterpret_cast<DrawPrimitive>(word(table, 0x144));
    GuardCleanup cleanup{guard, globals}; // Native EH state arms after lookup.
    (void)call(device, primitive_type, start_vertex, primitive_count);
    const auto current_mode = globals.mode_00;
    cleanup.armed = false; // Native disarms after mode read, before saved fields.
    if (current_mode != 0) {
        // Normal B21BCE reads the complete saved DWORD, including unused bytes;
        // the complete B33B00 provider consumes but ignores that argument.
        const Word ignored = word(&guard, 0);
        const void* const saved_renderer = guard.renderer_04;
        leave_native_renderer_optional_guard_00b33b00(saved_renderer, ignored, globals);
    }
}

} // namespace bsp
