#include "bsp/native_renderer_texture_stage_state.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <exception>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
Word word(const void* base, Word byte_offset) noexcept {
    Word value;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov value, eax
    }
    return value;
}
std::uint8_t byte(const void* base, Word byte_offset) noexcept {
    std::uint8_t value;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov al, byte ptr [eax + edx]
        mov value, al
    }
    return value;
}
void put(void* base, Word byte_offset, Word value) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}
void make_valid(void* base, Word byte_offset) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov byte ptr [eax + edx], 1
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
void leave_guard(GuardCleanup& cleanup) {
    const auto mode = cleanup.globals.mode_00;
    cleanup.armed = false;
    if (mode != 0) {
        const Word ignored = cleanup.guard.entered_00;
        const void* const saved_renderer = cleanup.guard.renderer_04;
        leave_native_renderer_optional_guard_00b33b00(saved_renderer, ignored,
            cleanup.globals);
    }
}
using SetTextureStageState = std::int32_t (__stdcall*)(void*, Word, Word, Word);
} // namespace

void set_native_renderer_texture_stage_state_00b24510(void* renderer, Word stage,
    Word state, Word value, NativeRendererSynchronizationGlobals& globals) {
    NativeRendererOptionalGuardStorage guard;
    if (globals.mode_00 != 0) {
        guard.renderer_04 = renderer;
        guard.entered_00 = enter_native_renderer_optional_guard_00b33ad0(renderer,
            globals);
    }
    const Word valid_offset = 0x45cu + stage * 0xacu + state;
    const bool valid = byte(renderer, valid_offset) != 0;
    GuardCleanup cleanup{guard, globals};
    const Word value_offset = 0x480u + (stage * 0x2bu + state) * 4u;
    if (!valid || word(renderer, value_offset) != value) {
        make_valid(renderer, valid_offset);
        put(renderer, value_offset, value);
        void* const device = pointer(word(renderer, 0x1a10));
        const void* const table = pointer(word(device, 0));
        const auto call = reinterpret_cast<SetTextureStageState>(word(table, 0x10c));
        (void)call(device, stage, state, value);
        put(renderer, 0x1ba8, word(renderer, 0x1ba8) + 1u);
    }
    leave_guard(cleanup);
}
} // namespace bsp
