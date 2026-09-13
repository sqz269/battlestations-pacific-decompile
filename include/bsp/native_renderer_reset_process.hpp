#pragma once
#include <cstdint>
namespace bsp {
struct NativeRendererDeviceRecreationContext;
struct NativeRendererResetProcessContext {
    NativeRendererDeviceRecreationContext& recreation;
    const void* volatile& actual_platform_0109cf04;
    volatile std::uint8_t& actual_pending_0108d4b8;
    volatile std::uint8_t& actual_lost_0108d4b9;
    volatile std::uint32_t& actual_retries_0108d4c4;
    const volatile std::uint32_t& actual_render_thread_0108d4c8;
};
// Full BEC230..BEC233, native ECX raw platform; EAX raw HWND; RET.
// This reads actual storage through +33, not the semantic Win32PlatformState.
void* native_platform_window_00bec230(const void* actual_platform) noexcept;
// Full B2ABD0..B2AE1C normal body, native ECX renderer/no stack args/RET.
// Borrow the same actual renderer, pending byte, publication and service
// domains used by recreation and presentation mode. Current Win32 imports
// are real immutable API bindings; every reached raw access remains valid.
void process_native_renderer_device_reset_00b2abd0(void* actual_renderer,
    NativeRendererResetProcessContext&);
// Optional-guard cleanup only. No new lifecycle lock, rollback, focus change,
// private retry counter, HRESULT normalization or recovery policy is added.
// Source context ABI does not establish original FH3/SEH/concurrency/gameplay.
} // namespace bsp
