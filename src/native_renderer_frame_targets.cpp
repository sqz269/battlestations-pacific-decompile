#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include "bsp/native_renderer_frame_targets.hpp"
#include "bsp/native_frame_target_getters.hpp"
#include "bsp/native_renderer_cached_states.hpp"
#include "bsp/native_renderer_surface_bindings.hpp"

#include <exception>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
Word word(const volatile void* base, Word byte_offset = 0) noexcept {
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
NativeFrameTargetOwnerStorage* owner(Word bits) noexcept {
    return reinterpret_cast<NativeFrameTargetOwnerStorage*>(bits);
}
volatile LONG* references(void* storage) noexcept {
    return reinterpret_cast<volatile LONG*>(reinterpret_cast<Word>(storage) + 4u);
}
const volatile Word* current_group_table(const void* storage,
    NativeRendererFrameTargetsContext& context) noexcept {
    const Word profile = word(storage);
    __assume(profile == 0x00d5e600);
    return context.actual_group_profile_00d5e600;
}
void release_at_zero(NativeFrameTargetOwnerStorage* captured,
    NativeRendererFrameTargetsContext& context) {
    const Word invoker = word(current_group_table(captured, context));
    __assume(invoker == 0x00bd30e0);
    // Full BD30E0 uses the CURRENT profile for its scalar-delete slot.
    const Word terminal = word(current_group_table(captured, context), 4);
    __assume(terminal == 0x00b1fcf0);
    (void)delete_native_frame_target_owner_00b1fcf0(*captured, 1, context.actual_group_owner);
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
} // namespace

void bind_native_renderer_frame_targets_00b24e70(void* renderer,
    NativeFrameTargetOwnerStorage* incoming, NativeRendererFrameTargetsContext& context) {
    auto& globals = context.actual_synchronization_0108d6dc;
    NativeRendererOptionalGuardStorage guard;
    if (globals.mode_00 != 0) {
        guard.renderer_04 = renderer;
        guard.entered_00 = enter_native_renderer_optional_guard_00b33ad0(renderer, globals);
    }
    const bool changed = owner(word(renderer, 0x1908)) != incoming;
    GuardCleanup cleanup{guard, globals}; // CMP B24EAF precedes EH arm B24EB5.
    if (changed) {
        NativeFrameTargetOwnerStorage* const captured_old = owner(word(renderer, 0x1908));
        if (captured_old != incoming) {
            put(renderer, 0x1908, reinterpret_cast<Word>(incoming));
            if (incoming) InterlockedIncrement(references(incoming));
            if (captured_old && InterlockedDecrement(references(captured_old)) == 0)
                release_at_zero(captured_old, context);
        }
        if (incoming) {
            if (context.actual_srgb_enabled_00f8d398 != 0) {
                const Word value = native_frame_targets_get_srgb_write_byte_00b1f710(incoming);
                set_native_renderer_render_state_00b24460(renderer, 0xc2, value, globals);
            }
            for (Word slot = 1; slot < 4; ++slot)
                bind_native_renderer_color_surface_00b23d80(renderer, slot, nullptr, globals);
            for (Word slot = 0; slot < 4; ++slot) {
                const auto* const surface = static_cast<NativeSurfaceOwnerStorage*>(
                    native_frame_targets_get_color_surface_00b1f6d0(incoming, slot));
                bind_native_renderer_color_surface_00b23d80(renderer, slot, surface, globals);
            }
            const auto* const depth = static_cast<NativeSurfaceOwnerStorage*>(
                native_frame_targets_get_depth_surface_00b1f6e0(incoming));
            bind_native_renderer_depth_surface_00b21690(renderer, depth, globals);
        }
    }
    const auto current_mode = globals.mode_00;
    cleanup.armed = false;
    if (current_mode != 0) {
        const Word ignored = guard.entered_00;
        const void* const saved_renderer = guard.renderer_04;
        leave_native_renderer_optional_guard_00b33b00(saved_renderer, ignored, globals);
    }
}

} // namespace bsp
