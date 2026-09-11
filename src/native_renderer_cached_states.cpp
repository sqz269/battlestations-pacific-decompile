#include "bsp/native_renderer_cached_states.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <exception>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
Word word(const volatile void* base, Word byte_offset) noexcept {
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
        leave_native_renderer_optional_guard_00b33b00(saved_renderer, ignored, cleanup.globals);
    }
}
using SetRenderState = std::int32_t (__stdcall*)(void*, Word, Word);
using SetSamplerState = std::int32_t (__stdcall*)(void*, Word, Word, Word);
} // namespace

void set_native_renderer_render_state_00b24460(void* renderer, Word state,
    Word value, NativeRendererSynchronizationGlobals& globals) {
    NativeRendererOptionalGuardStorage guard;
    if (globals.mode_00 != 0) {
        guard.renderer_04 = renderer;
        guard.entered_00 = enter_native_renderer_optional_guard_00b33ad0(renderer, globals);
    }
    const bool valid = byte(renderer, 0x40u + state) != 0;
    GuardCleanup cleanup{guard, globals};
    if (!valid || word(renderer, 0x114u + state * 4u) != value) {
        make_valid(renderer, 0x40u + state);
        put(renderer, 0x114u + state * 4u, value);
        auto* const device = pointer(word(renderer, 0x1a10));
        const auto* const table = pointer(word(device, 0));
        const auto call = reinterpret_cast<SetRenderState>(word(table, 0xe4));
        (void)call(device, state, value);
        put(renderer, 0x1ba0, word(renderer, 0x1ba0) + 1u);
    }
    leave_guard(cleanup);
}

void set_native_renderer_sampler_state_00b24610(void* renderer, Word sampler,
    Word state, Word value, NativeRendererSynchronizationGlobals& globals) {
    NativeRendererOptionalGuardStorage guard;
    if (globals.mode_00 != 0) {
        guard.renderer_04 = renderer;
        guard.entered_00 = enter_native_renderer_optional_guard_00b33ad0(renderer, globals);
    }
    const Word valid_offset = 0x11ccu + sampler * 72u + state;
    const bool valid = byte(renderer, valid_offset) != 0;
    GuardCleanup cleanup{guard, globals};
    const Word value_offset = 0x11dcu + sampler * 72u + state * 4u;
    if (!valid || word(renderer, value_offset) != value) {
        make_valid(renderer, valid_offset);
        put(renderer, value_offset, value);
        const Word api_sampler = sampler >= 16u ? sampler + 0xf1u : sampler;
        auto* const device = pointer(word(renderer, 0x1a10));
        const auto* const table = pointer(word(device, 0));
        const auto call = reinterpret_cast<SetSamplerState>(word(table, 0x114));
        (void)call(device, api_sampler, state, value);
        put(renderer, 0x1ba4, word(renderer, 0x1ba4) + 1u);
    }
    leave_guard(cleanup);
}

void initialize_native_renderer_default_states_00b26170(void* renderer,
    NativeRendererSynchronizationGlobals& globals) {
    constexpr Word defaults[][2] = {
        {0x1a, 1}, {0x1c, 0}, {0x23, 0}, {7, 1}, {0x0e, 1}, {0x17, 2},
        {0x34, 0}, {0xb9, 0}, {0xae, 0}, {0x0f, 0}, {0x1b, 0}, {0x16, 3},
        {0x1d, 0}, {8, 3}, {0xa8, 0x0f}, {0xbe, 0x0f}, {0xbf, 0x0f},
        {0xc0, 0x0f}, {0xc2, 0}
    };
    for (const auto& item : defaults)
        set_native_renderer_render_state_00b24460(renderer, item[0], item[1], globals);
    for (Word sampler = 0; sampler < 20; ++sampler) {
        set_native_renderer_sampler_state_00b24610(renderer, sampler, 6, 2, globals);
        set_native_renderer_sampler_state_00b24610(renderer, sampler, 5, 2, globals);
        set_native_renderer_sampler_state_00b24610(renderer, sampler, 7, 1, globals);
        set_native_renderer_sampler_state_00b24610(renderer, sampler, 1, 1, globals);
        set_native_renderer_sampler_state_00b24610(renderer, sampler, 2, 1, globals);
        set_native_renderer_sampler_state_00b24610(renderer, sampler, 3, 1, globals);
        set_native_renderer_sampler_state_00b24610(renderer, sampler, 4, 0xffffffffu, globals);
    }
}

} // namespace bsp
