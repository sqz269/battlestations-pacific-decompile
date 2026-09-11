#include "bsp/native_renderer_index_binding.hpp"
#include "bsp/native_renderer_binding_getters.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <d3d9.h>

#include <exception>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);

std::uint32_t word(const volatile void* base, std::uint32_t byte_offset = 0) noexcept {
    std::uint32_t value;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov value, eax
    }
    return value;
}
void put(void* base, std::uint32_t byte_offset, std::uint32_t value) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}
void* pointer(std::uint32_t bits) noexcept { return reinterpret_cast<void*>(bits); }
volatile LONG* references(void* owner) noexcept {
    return reinterpret_cast<volatile LONG*>(reinterpret_cast<std::uintptr_t>(owner) + 4u);
}
const volatile std::uint32_t* current_logical_table(void* owner,
    NativeRendererIndexBindingContext& context) noexcept {
    const auto profile = word(owner);
    __assume(profile == 0x00d61de0);
    return context.actual_logical_profile_00d61de0;
}
const volatile std::uint32_t* current_physical_table(const void* physical,
    const NativeLogicalIndexPhysicalProfiles& profiles) noexcept {
    const auto profile = word(physical);
    if (profile == 0x00d61e10) return profiles.private_index_00d61e10;
    if (profile == 0x00d61e58) return profiles.pooled_index_00d61e58;
    __assume(0);
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
using SetIndices = HRESULT (STDMETHODCALLTYPE*)(IDirect3DDevice9*, IDirect3DIndexBuffer9*);
} // namespace

void* get_native_logical_index_buffer_00b48dc0(const void* logical,
    const NativeLogicalIndexPhysicalProfiles& profiles) {
    const void* const physical = pointer(word(logical, 8));
    const auto* const physical_table = current_physical_table(physical, profiles);
    const auto terminal = word(physical_table, 0x1c);
    __assume(terminal == 0x00b4b840);
    return native_physical_index_buffer_get_com_00b4b840(physical);
}

void bind_native_renderer_index_stream_00b24b00(void* renderer, void* incoming,
    std::uint32_t base_vertex, NativeRendererIndexBindingContext& context) {
    auto& owner_context = context.actual_logical_owner;
    auto& globals = owner_context.actual_synchronization_0108d6dc;
    NativeRendererOptionalGuardStorage guard;
    if (globals.mode_00 != 0) {
        guard.renderer_04 = renderer;
        guard.entered_00 = enter_native_renderer_optional_guard_00b33ad0(renderer, globals);
    }
    const bool changed = pointer(word(renderer, 0x17b8)) != incoming;
    GuardCleanup cleanup{guard, globals}; // Native arms after comparison, before base write.
    put(renderer, 0x17bc, base_vertex);
    if (changed) {
        void* const captured_old = pointer(word(renderer, 0x17b8));
        if (captured_old != incoming) {
            put(renderer, 0x17b8, reinterpret_cast<std::uint32_t>(incoming));
            if (incoming) InterlockedIncrement(references(incoming));
            if (captured_old && InterlockedDecrement(references(captured_old)) == 0) {
                const auto zero_terminal = word(current_logical_table(captured_old, context));
                __assume(zero_terminal == 0x00bd30e0);
                // Complete BD30E0 rereads the current profile before slot+04.
                const auto deleting_terminal = word(current_logical_table(captured_old, context), 4);
                __assume(deleting_terminal == 0x00b4c1f0);
                delete_native_pooled_logical_index_stream_00b4c1f0(
                    captured_old, 1, owner_context);
            }
        }
        IDirect3DIndexBuffer9* buffer = nullptr;
        if (incoming) {
            const auto getter_terminal = word(current_logical_table(incoming, context), 0x28);
            __assume(getter_terminal == 0x00b48dc0);
            buffer = static_cast<IDirect3DIndexBuffer9*>(get_native_logical_index_buffer_00b48dc0(
                incoming, owner_context.actual_physical_profiles));
        }
        auto* const device = static_cast<IDirect3DDevice9*>(pointer(word(renderer, 0x1a10)));
        const auto* const device_table = pointer(word(device));
        const auto call = reinterpret_cast<SetIndices>(word(device_table, 0x1a0));
        (void)call(device, buffer);
        put(renderer, 0x1bb8, word(renderer, 0x1bb8) + 1u);
    }
    const auto current_mode = globals.mode_00;
    cleanup.armed = false;
    if (current_mode != 0) {
        // B33B00 ignores native high padding bytes; pass only the defined AL.
        const std::uint32_t ignored = guard.entered_00;
        const void* const saved_renderer = guard.renderer_04;
        leave_native_renderer_optional_guard_00b33b00(saved_renderer, ignored, globals);
    }
}
} // namespace bsp
