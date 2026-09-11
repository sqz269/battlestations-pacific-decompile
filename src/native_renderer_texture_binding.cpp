#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <d3d9.h>

#include "bsp/native_renderer_texture_binding.hpp"
#include "bsp/native_renderer_binding_getters.hpp"

#include <exception>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual renderer texture binding requires MSVC Win32.
#endif

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
const volatile std::uint32_t* current_texture_table(const void* owner,
    const NativeRendererTextureBindingProfiles& profiles) noexcept {
    const auto current_profile = word(owner);
    if (current_profile == 0x00d61948) return profiles.actual_texture_2d_00d61948;
    if (current_profile == 0x00d61870) return profiles.actual_cube_00d61870;
    if (current_profile == 0x00d618b0) return profiles.actual_volume_00d618b0;
    __assume(0);
}
void release_at_zero(void* captured, NativeRendererTextureBindingContext& context) {
    const auto invoker = word(current_texture_table(captured, context.actual_profiles));
    __assume(invoker == 0x00bd30e0);
    // Complete BD30E0 rereads the owner's current profile before slot+04.
    const auto terminal = word(current_texture_table(captured, context.actual_profiles), 4);
    if (terminal == 0x00b3f590) {
        delete_native_texture_2d_00b3f590(captured, 1, context.actual_texture_2d_owner);
        return;
    }
    if (terminal == 0x00b3f410) {
        delete_native_cube_texture_00b3f410(captured, 1, context.actual_cube_owner);
        return;
    }
    if (terminal == 0x00b3f430) {
        delete_native_volume_texture_00b3f430(captured, 1, context.actual_volume_owner);
        return;
    }
    __assume(0);
}
void* borrowed_texture_com(void* incoming, const NativeRendererTextureBindingProfiles& profiles) {
    const auto terminal = word(current_texture_table(incoming, profiles), 0x1c);
    if (terminal == 0x00b3cea0) return native_texture_get_com_00b3cea0(incoming);
    if (terminal == 0x00b3cf90) return native_cube_texture_get_com_00b3cf90(incoming);
    if (terminal == 0x00b3d0c0) return native_volume_texture_get_com_00b3d0c0(incoming);
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
using SetTexture = HRESULT (STDMETHODCALLTYPE*)(IDirect3DDevice9*, DWORD, IDirect3DBaseTexture9*);
} // namespace

__declspec(naked) void* __fastcall
native_cube_texture_get_com_00b3cf90(const void*) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 10h]
        ret
    }
}
__declspec(naked) void* __fastcall
native_volume_texture_get_com_00b3d0c0(const void*) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 10h]
        ret
    }
}

void bind_native_renderer_texture_00b24710(void* renderer, std::uint32_t sampler,
    void* incoming, NativeRendererTextureBindingContext& context) {
    auto& globals = context.actual_synchronization_0108d6dc;
    NativeRendererOptionalGuardStorage guard;
    if (globals.mode_00 != 0) {
        guard.renderer_04 = renderer;
        guard.entered_00 = enter_native_renderer_optional_guard_00b33ad0(renderer, globals);
    }
    const std::uint32_t slot_offset = sampler * 0xacu + 0x504u;
    const bool changed = pointer(word(renderer, slot_offset)) != incoming;
    GuardCleanup cleanup{guard, globals}; // Native arms state0 after comparison.
    if (changed) {
        void* const captured_old = pointer(word(renderer, slot_offset));
        if (captured_old != incoming) {
            put(renderer, slot_offset, reinterpret_cast<std::uint32_t>(incoming));
            if (incoming) InterlockedIncrement(references(incoming));
            if (captured_old && InterlockedDecrement(references(captured_old)) == 0)
                release_at_zero(captured_old, context);
        }
        const std::uint32_t device_sampler = sampler >= 16u ? sampler + 0xf1u : sampler;
        IDirect3DBaseTexture9* texture = nullptr;
        if (incoming)
            texture = static_cast<IDirect3DBaseTexture9*>(borrowed_texture_com(incoming, context.actual_profiles));
        auto* const device = static_cast<IDirect3DDevice9*>(pointer(word(renderer, 0x1a10)));
        const auto* const current_device_table = pointer(word(device));
        const auto call = reinterpret_cast<SetTexture>(word(current_device_table, 0x104));
        (void)call(device, device_sampler, texture);
        put(renderer, 0x1bc4, word(renderer, 0x1bc4) + 1u);
    }
    const auto current_mode = globals.mode_00;
    cleanup.armed = false;
    if (current_mode != 0) {
        // Native DWORD padding is uninitialized but ignored by full B33B00.
        const std::uint32_t ignored = guard.entered_00;
        const void* const saved_renderer = guard.renderer_04;
        leave_native_renderer_optional_guard_00b33b00(saved_renderer, ignored, globals);
    }
}
} // namespace bsp
