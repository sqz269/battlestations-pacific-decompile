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
// Packet cc9_d3d_device_lost. A DELIBERATE DIVERGENCE, not the image: the
// DEVICELOST branch (00B2AC7D..00B2ACC6) recreates the device through 00B29670
// once the lost counter passes 10 with focus, and 00B29670 releases +1A10h and
// ignores CreateDevice's HRESULT (00B298EE), then calls 00B24460 through the
// device. While the session cannot create a device (a locked or switched RDP
// session) that is a null dereference, in the image as here. ON holds the lost
// branch at "mark lost, wait": no recreation from it, so recovery comes from
// DEVICENOTRESET and the image's own Reset path. docs/D3D_DEVICE_LOST.md.
inline constexpr bool kRendererLostDeviceHoldBound = true;
// DIAGNOSTIC, env-gated (BSP_RENDERER_FAKE_LOST=<first>,<count>[,createfail]):
// from the <first>th call of 00B2ABD0's request body, <count> calls see
// TestCooperativeLevel as DEVICELOST, the next one as DEVICENOTRESET, with the
// global lost byte set; `createfail` makes 00B29670 skip CreateDevice, leaving
// +1A10h null as a failed CreateDevice does. It exercises the host's own
// lost-device paths without touching the session. Never set in reference runs.
struct NativeRendererLostDeviceStats {
    std::uint32_t lost_polls{0};      // DEVICELOST observed (real or faked)
    std::uint32_t lost_holds{0};      // the hold returned where 00B29670 would run
    std::uint32_t recreations{0};     // 00B29670 entered
    std::uint32_t create_failures{0}; // CreateDevice left +1A10h null
    std::uint32_t resets{0};          // Reset returned 0
    std::uint32_t reset_failures{0};
    std::uint32_t faked_polls{0};
};
NativeRendererLostDeviceStats& native_renderer_lost_device_stats() noexcept;
bool native_renderer_fake_create_fail() noexcept;
// Optional-guard cleanup only. No new lifecycle lock, rollback, focus change,
// private retry counter, HRESULT normalization or recovery policy is added.
// Source context ABI does not establish original FH3/SEH/concurrency/gameplay.
} // namespace bsp
