#include "bsp/native_renderer_reset_process.hpp"
#include "bsp/native_renderer_device_recreation_actual.hpp"
#include "bsp/native_renderer_resource_release.hpp"
#include "bsp/native_renderer_resource_restore.hpp"
#include "bsp/native_dynamic_buffer_device_release.hpp"
#include "bsp/native_renderer_reset_readiness.hpp"
#include "bsp/native_renderer_cached_states.hpp"
#include "bsp/native_renderer_gamma.hpp"
#include <Windows.h>
#include <cstdlib>
#include <cstring>
#include <exception>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual renderer reset requires MSVC Win32.
#endif
namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
Word word(const volatile void* base, Word byte_offset = 0) noexcept {
    Word result;
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov eax, dword ptr [eax + edx] }
    __asm { mov result, eax }
    return result;
}
Word byte(const volatile void* base, Word byte_offset = 0) noexcept {
    Word result;
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { movzx eax, byte ptr [eax + edx] }
    __asm { mov result, eax }
    return result;
}
void put(void* base, Word byte_offset, Word value) noexcept {
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov ecx, value }
    __asm { mov dword ptr [eax + edx], ecx }
}
void put_byte(void* base, Word byte_offset, Word value) noexcept {
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov ecx, value }
    __asm { mov byte ptr [eax + edx], cl }
}
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
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
__declspec(naked) void __fastcall current_gamma(void*, const GammaAccess*) {
    __asm {
        push ebx
        push esi
        mov ebx, edx
        mov esi, ecx
        fld dword ptr [esi + 196ch]
        mov edx, [esi]
        cmp edx, 0d5f0a8h
        jne unsupported
        mov edx, [ebx]
        mov eax, [edx + 0f0h]
        cmp eax, 0b21960h
        jne unsupported
        push ecx
        fstp dword ptr [esp]
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
bool captured_platform_is_focused(const void* captured_platform) noexcept {
    if (byte(captured_platform, 0x41) == 0) return false;
    const HWND focus = GetFocus();
    return reinterpret_cast<Word>(focus) == word(captured_platform, 0x30);
}
// Packet cc9_d3d_device_lost, DIAGNOSTIC: BSP_RENDERER_FAKE_LOST.
struct FakeLoss {
    long first{-1};
    long count{0};
    bool create_fail{false};
    long calls{0};
};
FakeLoss& fake_loss() noexcept {
    static FakeLoss fake = [] {
        FakeLoss f;
        char* text = nullptr;
        std::size_t bytes = 0;
        if (_dupenv_s(&text, &bytes, "BSP_RENDERER_FAKE_LOST") == 0 && text != nullptr) {
            char* end = nullptr;
            f.first = std::strtol(text, &end, 10);
            if (end != nullptr && *end == ',') f.count = std::strtol(end + 1, &end, 10);
            if (end != nullptr && *end == ',') f.create_fail = std::strstr(end, "createfail") != nullptr;
            if (f.first < 0 || f.count < 0) f.first = -1;
        }
        std::free(text);
        return f;
    }();
    return fake;
}
void process_current_request(void* renderer, NativeRendererResetProcessContext& context) {
    auto& recreation = context.recreation;
    auto& globals = recreation.release.actual_bindings.actual_vertex.actual_logical_owner.actual_synchronization_0108d6dc;
    auto& stats = native_renderer_lost_device_stats();
    FakeLoss& fake = fake_loss();
    bool fake_lost = false;
    bool fake_not_reset = false;
    if (fake.first >= 0) {
        const long call = fake.calls++;
        if (call >= fake.first && call < fake.first + fake.count) fake_lost = true;
        else if (call == fake.first + fake.count) fake_not_reset = true;
        if (fake_lost || fake_not_reset) {
            context.actual_lost_0108d4b9 = 1;
            ++stats.faked_polls;
        }
    }
    const Word pending = byte(&context.actual_pending_0108d4b8);
    const Word lost = byte(&context.actual_lost_0108d4b9); // Read even when pending!=0.
    if (pending == 0 && lost == 0) return;
    Word cooperative = 0;
    if (lost != 0) {
        void* const device = pointer(word(renderer, 0x1a10));
        using Cooperative = HRESULT (WINAPI*)(void*);
        const auto call = reinterpret_cast<Cooperative>(word(pointer(word(device)), 0x0c));
        cooperative = static_cast<Word>(call(device));
        if (fake_lost) cooperative = 0x88760868u;
        if (fake_not_reset) cooperative = 0x88760869u;
        if (cooperative == 0x88760868u) {
            ++stats.lost_polls;
            const Word retries = word(&context.actual_retries_0108d4c4) + 1u;
            context.actual_retries_0108d4c4 = retries;
            put_byte(renderer, 0x1d8a, 1);
            if (retries <= 10) return;
            if constexpr (kRendererLostDeviceHoldBound) {
                // DIVERGENCE (packet cc9_d3d_device_lost): no 00B29670 from the
                // lost branch; the device is kept and polled until DEVICENOTRESET.
                ++stats.lost_holds;
            } else {
                const void* const platform = context.actual_platform_0109cf04;
                if (!captured_platform_is_focused(platform)) return;
                context.actual_retries_0108d4c4 = 0;
                recreate_native_renderer_device_00b29670(renderer, recreation);
            }
            return;
        }
    }
    if (cooperative != 0x88760869u && byte(&context.actual_pending_0108d4b8) == 0)
        return;
    const void* const focused_platform = context.actual_platform_0109cf04;
    if (!captured_platform_is_focused(focused_platform)) return;
    const auto sleep_call = &Sleep; // Native captures the immutable Sleep import once.
    bool reset_succeeded = false;
    if (cooperative == 0x88760869u || cooperative == 0) {
        put_byte(renderer, 0x1d8a, 1);
        sleep_call(100);
        release_native_dynamic_buffers_for_reset_00b237d0(renderer, globals);
        release_native_renderer_resources_00b262c0(renderer, recreation.release);
        sleep_call(100);
        const void* const current_platform = context.actual_platform_0109cf04;
        void* const window = native_platform_window_00bec230(current_platform);
        put(renderer, 0x1a44, reinterpret_cast<Word>(window));
        void* const device = pointer(word(renderer, 0x1a10));
        void* const table = pointer(word(device));
        void* const parameters = pointer(reinterpret_cast<Word>(renderer) + 0x1a28u);
        using Reset = HRESULT (WINAPI*)(void*, void*);
        const auto reset = reinterpret_cast<Reset>(word(table, 0x40));
        const bool reset_ok = reset(device, parameters) == 0;
        if (reset_ok) ++stats.resets; else ++stats.reset_failures;
        if (reset_ok) {
            put_byte(renderer, 0x1d8a, 0);
            put(renderer, 0x1d90, 2);
            restore_native_renderer_resources_00b23b10(renderer, recreation.restore);
            restore_native_renderer_dynamic_buffers_00b1fd90(renderer, recreation.physical_profiles);
            set_native_renderer_render_state_00b24460(renderer, 0xa1, word(renderer, 0x1a38) != 0, globals);
            initialize_native_renderer_default_states_00b26170(renderer, globals);
            const GammaAccess gamma{recreation.release.actual_bindings.actual_renderer_profile_00d5f0a8, &recreation.gamma};
            current_gamma(renderer, &gamma);
            context.actual_pending_0108d4b8 = 0;
            context.actual_lost_0108d4b9 = 0;
            reset_succeeded = true;
        }
    }
    if (!reset_succeeded) {
        const void* const current_platform = context.actual_platform_0109cf04;
        if (native_platform_has_focus_00b20c50(current_platform) != 0)
            recreate_native_renderer_device_00b29670(renderer, recreation);
    }
    sleep_call(100);
}
} // namespace
NativeRendererLostDeviceStats& native_renderer_lost_device_stats() noexcept {
    static NativeRendererLostDeviceStats stats;
    return stats;
}
bool native_renderer_fake_create_fail() noexcept {
    return fake_loss().create_fail;
}
void* native_platform_window_00bec230(const void* platform) noexcept {
    return pointer(word(platform, 0x30));
}
void process_native_renderer_device_reset_00b2abd0(void* renderer,
    NativeRendererResetProcessContext& context) {
    auto& globals = context.recreation.release.actual_bindings.actual_vertex.actual_logical_owner.actual_synchronization_0108d6dc;
    NativeRendererOptionalGuardStorage guard;
    if (globals.mode_00 != 0) {
        guard.renderer_04 = renderer;
        guard.entered_00 = enter_native_renderer_optional_guard_00b33ad0(renderer, globals);
    }
    GuardCleanup cleanup{guard, globals};
    const DWORD current_thread = GetCurrentThreadId();
    if (word(&context.actual_render_thread_0108d4c8) == current_thread)
        process_current_request(renderer, context);
    const auto current_mode = globals.mode_00;
    cleanup.armed = false;
    if (current_mode != 0)
        leave_native_renderer_optional_guard_00b33b00(guard.renderer_04, word(&guard), globals);
}
} // namespace bsp
