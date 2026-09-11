#include "bsp/native_renderer_surface_bindings.hpp"
#include "bsp/native_surface_owner.hpp"

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(offsetof(NativeSurfaceOwnerStorage, surface_2c) == 0x2c);

volatile std::uint32_t& word(const void* actual, std::uint32_t offset) noexcept {
    const auto location = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(actual)) + offset;
    return *reinterpret_cast<volatile std::uint32_t*>(location);
}
IDirect3DDevice9* device_at(const void* renderer) noexcept {
    return reinterpret_cast<IDirect3DDevice9*>(word(renderer, 0x1a10));
}
IDirect3DSurface9* surface_at(const NativeSurfaceOwnerStorage* wrapper) noexcept {
    return reinterpret_cast<IDirect3DSurface9*>(word(wrapper, 0x2c));
}
using SetColor = HRESULT (STDMETHODCALLTYPE*)(IDirect3DDevice9*, DWORD, IDirect3DSurface9*);
using SetDepth = HRESULT (STDMETHODCALLTYPE*)(IDirect3DDevice9*, IDirect3DSurface9*);
template<class Method> Method method_at(IDirect3DDevice9* device, std::uint32_t offset) noexcept {
    const auto* const table = reinterpret_cast<const void*>(word(device, 0));
    return reinterpret_cast<Method>(word(table, offset));
}
void enter(void* renderer, NativeRendererOptionalGuardStorage& guard,
    NativeRendererSynchronizationGlobals& globals) {
    if (globals.mode_00 != 0) {
        guard.renderer_04 = renderer;
        guard.entered_00 = enter_native_renderer_optional_guard_00b33ad0(renderer, globals);
    }
}
void leave(const NativeRendererOptionalGuardStorage& guard,
    NativeRendererSynchronizationGlobals& globals) {
    if (globals.mode_00 != 0) {
        // Native normal cleanup loads all four bytes at guard+0. Only AL was
        // initialized, and B33B00 ignores the entire argument. Reading the
        // defined low byte preserves the consumed value without reading padding.
        const std::uint32_t ignored = guard.entered_00;
        const void* const renderer = guard.renderer_04;
        leave_native_renderer_optional_guard_00b33b00(renderer, ignored, globals);
    }
}
} // namespace

void bind_native_renderer_color_surface_00b23d80(void* actual_renderer,
    std::uint32_t slot, const NativeSurfaceOwnerStorage* actual_wrapper,
    NativeRendererSynchronizationGlobals& globals) {
    NativeRendererOptionalGuardStorage guard;
    enter(actual_renderer, guard, globals); // Native EH state is still -1 here.
    try {
        if (actual_wrapper) {
            IDirect3DSurface9* const surface = surface_at(actual_wrapper);
            IDirect3DDevice9* const device = device_at(actual_renderer);
            const auto call = method_at<SetColor>(device, 0x94);
            (void)call(device, slot, surface);
            word(actual_renderer, 0x1ba0) = word(actual_renderer, 0x1ba0) + 1u;
        } else if (slot == 0) {
            const auto* const fallback = reinterpret_cast<const NativeSurfaceOwnerStorage*>(
                word(actual_renderer, 0x197c));
            IDirect3DSurface9* const surface = surface_at(fallback);
            IDirect3DDevice9* const device = device_at(actual_renderer);
            const auto call = method_at<SetColor>(device, 0x94);
            (void)call(device, 0, surface);
        } else {
            IDirect3DDevice9* const device = device_at(actual_renderer);
            const auto call = method_at<SetColor>(device, 0x94);
            (void)call(device, slot, nullptr);
        }
    } catch (...) {
        destroy_native_renderer_optional_guard_00b21110(guard, globals);
        throw;
    }
    leave(guard, globals); // Disarmed before normal cleanup; no second cleanup.
}

void bind_native_renderer_depth_surface_00b21690(void* actual_renderer,
    const NativeSurfaceOwnerStorage* actual_wrapper,
    NativeRendererSynchronizationGlobals& globals) {
    NativeRendererOptionalGuardStorage guard;
    enter(actual_renderer, guard, globals);
    try {
        if (actual_wrapper) {
            // The native depth path captures the device BEFORE wrapper+2C.
            IDirect3DDevice9* const device = device_at(actual_renderer);
            IDirect3DSurface9* const surface = surface_at(actual_wrapper);
            const auto call = method_at<SetDepth>(device, 0x9c);
            (void)call(device, surface);
            word(actual_renderer, 0x1bcc) = word(actual_renderer, 0x1bcc) + 1u;
        } else {
            IDirect3DDevice9* const device = device_at(actual_renderer);
            const auto call = method_at<SetDepth>(device, 0x9c);
            (void)call(device, nullptr);
        }
    } catch (...) {
        destroy_native_renderer_optional_guard_00b21110(guard, globals);
        throw;
    }
    leave(guard, globals);
}
} // namespace bsp
