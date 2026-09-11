#include "bsp/native_renderer_stream_frequency.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <exception>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
Word word(const void* base, Word byte_offset) noexcept {
    Word result;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov result, eax
    }
    return result;
}
void put(void* base, Word byte_offset, Word value) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
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
using SetFrequency = std::int32_t (__stdcall*)(void*, Word, Word);
} // namespace

void set_native_renderer_stream_frequency_00b24a40(void* renderer, Word stream,
    Word value, NativeRendererSynchronizationGlobals& globals) {
    NativeRendererOptionalGuardStorage guard;
    if (globals.mode_00 != 0) {
        guard.renderer_04 = renderer;
        guard.entered_00 = enter_native_renderer_optional_guard_00b33ad0(renderer, globals);
    }
    const Word offset = (stream + 0x178u) * 16u;
    // Native CMP B24A8E precedes arming its cleanup at B24A90.
    const Word previous = word(renderer, offset);
    GuardCleanup cleanup{guard, globals};
    if (previous != value) {
        put(renderer, offset, value);
        auto* const device = pointer(word(renderer, 0x1a10));
        const auto* const table = pointer(word(device, 0));
        const auto call = reinterpret_cast<SetFrequency>(word(table, 0x198));
        (void)call(device, stream, value);
    }
    // Both returning branches read mode before disarming; Leave sees the
    // saved renderer and current synchronization state. There is no counter.
    const auto mode = globals.mode_00;
    cleanup.armed = false;
    if (mode != 0) {
        const Word ignored = guard.entered_00;
        const void* const saved_renderer = guard.renderer_04;
        leave_native_renderer_optional_guard_00b33b00(saved_renderer, ignored, globals);
    }
}

} // namespace bsp
