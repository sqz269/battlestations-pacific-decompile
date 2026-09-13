#include "bsp/native_renderer_presentation_mode.hpp"
#include "bsp/native_renderer_device_recreation_actual.hpp"
#include "bsp/native_renderer_resource_release.hpp"
#include "bsp/native_renderer_gamma.hpp"
#include "bsp/native_render_state_leaves.hpp"

#include <windows.h>
#include <exception>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual renderer presentation mode requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
Word word(const volatile void* base, Word byte_offset = 0) noexcept {
    Word value;
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov eax, dword ptr [eax + edx] }
    __asm { mov value, eax }
    return value;
}
Word byte(const void* base, Word byte_offset) noexcept {
    Word value;
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { movzx eax, byte ptr [eax + edx] }
    __asm { mov value, eax }
    return value;
}
void put(void* base, Word byte_offset, Word value) noexcept {
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov ecx, value }
    __asm { mov dword ptr [eax + edx], ecx }
}
void adjust_depth(void* section, Word delta) noexcept {
    __asm { mov eax, section }
    __asm { mov edx, delta }
    __asm { add dword ptr [eax + 18h], edx }
}
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
Word interval_value(Word slot) noexcept {
    Word value;
    __asm {
        mov eax, slot
        neg al
        sbb eax, eax
        and eax, 80000000h
        add eax, 80000000h
        mov value, eax
    }
    return value;
}
int cleanup_exception(unsigned long code) noexcept {
    if (code == 0xe06d7363u) std::terminate();
    return EXCEPTION_CONTINUE_SEARCH;
}
void unwind_guard(const NativeRendererOptionalGuardStorage& guard,
    NativeRendererSynchronizationGlobals& globals) noexcept {
    __try { destroy_native_renderer_optional_guard_00b21110(guard, globals); }
    __except (cleanup_exception(GetExceptionCode())) { __assume(0); }
}
struct GuardCleanup {
    const NativeRendererOptionalGuardStorage& guard;
    NativeRendererSynchronizationGlobals& globals;
    bool armed = true;
    ~GuardCleanup() noexcept { if (armed) unwind_guard(guard, globals); }
};
struct GammaAccess {
    const volatile Word* profile;
    const NativeRendererGammaContext* gamma;
};
// Preserve the native FLD between the two presentation reads, the current
// profile capture, and the width/FSTP/height/slot sequence. The borrowed float
// passed to the full gamma provider is this actual outgoing stack word.
__declspec(naked) void __fastcall publish_dimensions_and_gamma(void*, const GammaAccess*) {
    __asm {
        push ebx
        push esi
        mov ebx, edx
        mov esi, ecx
        mov ecx, [esi + 1a28h]
        fld dword ptr [esi + 196ch]
        mov edx, [esi + 1a2ch]
        mov eax, [esi]
        push ecx
        mov [esi + 1a20h], ecx
        fstp dword ptr [esp]
        mov [esi + 1a24h], edx
        mov ecx, [ebx]
        mov edx, [ecx + 0f0h]
        cmp eax, 0d5f0a8h
        jne unsupported
        cmp edx, 0b21960h
        jne unsupported
        lea eax, [esp]
        push dword ptr [ebx + 4]
        push eax
        push esi
        call set_native_renderer_gamma_00b21960
        add esp, 10h
        pop esi
        pop ebx
        ret
    unsupported:
        _emit 0x0f
        _emit 0x0b
    }
}
} // namespace

std::uint8_t change_native_renderer_presentation_mode_00b29e60(void* renderer,
    Word width, Word height, Word fullscreen_slot, Word multisample,
    Word interval_selector_slot, Word force_slot,
    NativeRendererPresentationModeContext& context) {
    auto& recreation = context.recreation;
    auto& bindings = recreation.release.actual_bindings;
    auto& globals = bindings.actual_vertex.actual_logical_owner.actual_synchronization_0108d6dc;
    NativeRendererOptionalGuardStorage guard;
    if (globals.mode_00 != 0) {
        guard.renderer_04 = renderer;
        guard.entered_00 = enter_native_renderer_optional_guard_00b33ad0(renderer, globals);
    }
    void* const entered_lifecycle = pointer(word(renderer, 0x199c));
    GuardCleanup cleanup{guard, globals}; // Native state0 before EnterCriticalSection.
    EnterCriticalSection(static_cast<CRITICAL_SECTION*>(entered_lifecycle));
    adjust_depth(entered_lifecycle, 1);

    const Word old_interval_is_zero = word(renderer, 0x1a5c) == 0;
    const auto interval = static_cast<std::uint8_t>(interval_selector_slot);
    const auto fullscreen = static_cast<std::uint8_t>(fullscreen_slot);
    const bool matches = static_cast<std::uint8_t>(force_slot) == 0
        && width == word(renderer, 0x1a28)
        && height == word(renderer, 0x1a2c)
        && multisample == word(renderer, 0x1a38)
        && (word(renderer, 0x1a48) != 0) != fullscreen
        && old_interval_is_zero == interval;
    if (!matches) {
        const Word windowed = fullscreen == 0;
        const bool windowed_changed = word(renderer, 0x1a48) != windowed;
        if (width != 0) put(renderer, 0x1a28, width);
        if (height != 0) put(renderer, 0x1a2c, height);
        put(renderer, 0x1d90, 2);
        put(renderer, 0x1a40, 1);
        put(renderer, 0x1a38, multisample);
        put(renderer, 0x1a3c, 0);
        put(renderer, 0x1a5c, interval_value(interval_selector_slot));
        put(renderer, 0x1a48, windowed);
        put(renderer, 0x1a54, 2);
        if (windowed_changed)
            recreate_native_renderer_device_00b29670(renderer, recreation);
        else
            context.actual_pending_0108d4b8 = 1;

        const GammaAccess gamma{bindings.actual_renderer_profile_00d5f0a8, &recreation.gamma};
        publish_dimensions_and_gamma(renderer, &gamma);
        const Word profile = word(renderer);
        __assume(profile == 0x00d5f0a8u);
        const Word target = word(bindings.actual_renderer_profile_00d5f0a8, 0x2c);
        __assume(target == 0x00b1fe20u);
        const auto active = native_render_is_frame_active_00b1fe20(renderer);
        if (active == 0 && word(renderer, 0x1d90) == 0
            && byte(renderer, 0x1d8a) == active) {
            void* const device = pointer(word(renderer, 0x1a10));
            using BeginScene = HRESULT (WINAPI*)(void*);
            auto begin = reinterpret_cast<BeginScene>(word(pointer(word(device)), 0xa4));
            (void)begin(device);
        }
    }
    void* const current_lifecycle = pointer(word(renderer, 0x199c));
    adjust_depth(current_lifecycle, 0xffffffffu);
    LeaveCriticalSection(static_cast<CRITICAL_SECTION*>(current_lifecycle));
    const auto current_mode = globals.mode_00;
    cleanup.armed = false;
    if (current_mode != 0)
        leave_native_renderer_optional_guard_00b33b00(guard.renderer_04, word(&guard), globals);
    return 1;
}
} // namespace bsp
