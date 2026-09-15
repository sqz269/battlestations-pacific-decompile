#pragma once
#include "bsp/native_frame_target_owner.hpp"
#include "bsp/native_renderer_synchronization_actual.hpp"
#include <cstdint>

namespace bsp {
struct NativeRendererDeviceRecreationContext;

// Complete159-byte B21210 and its original126-byte text-section switch tables.
// ECX format token, EAX original table result, RET. This is the game's storage
// accounting classification, not an SDK replacement; preserve its unusual64
// results for many nominal16-bit formats and its DXT1 result4.
std::uint32_t native_format_storage_bits_00b21210(std::uint32_t format) noexcept;

// Complete95-byte B22850. ECX actual12-byte header: pointer,count,
// capacity. Stack signed requested capacity, RET4. Clamp to1, compare signed,
// allocate wrapping DWORD(capacity*4), copy current live DWORD entries, free old,
// then publish pointer/capacity. No retain, gap clear or count change.
void reserve_native_renderer_surface_slots_00b22850(void* actual_three_word_header,
    std::int32_t requested_capacity);

struct NativeRendererSurfaceFactoryContext {
    NativeSurfaceOwnerContext& surfaces;
    NativeRendererSynchronizationGlobals& synchronization;
    NativeRendererDeviceRecreationContext* recreation; // Required if native retry reaches B29670.
    volatile std::uint32_t& allocation_bytes_0108d4c0;
    const volatile float& unsigned_dword_fix_00ce3978;
    const volatile double& unsigned_counter_fix_00d57da0;
};
struct NativeRendererSurfaceFactoryAcquired {
    enum class Phase { fresh, guard, create, construct, bookkeeping, complete, failed };
    Phase phase{Phase::fresh};
    std::uint32_t native_site{};
    std::int32_t unwind_state{-1};
    NativeRendererOptionalGuardStorage guard; // Unwritten when entry mode is zero.
    bool guard_initialized{}, raw_slot_returned{}, creator_release_started{};
    void* raw_slot{};
    IDirect3DSurface9* volatile com_output{};
    NativeSurfaceOwnerStorage* owner{};
    HRESULT create_result{};
};

// Complete477-byte B2A7C0: ECX renderer; width,height,format,multisample DWORDs;
// EAX actual surface creator, RET10h. Current COM CreateRenderTarget, native
// retry predicate and real B29670 when reached; actual pool/B3F630(flags10,kind0);
// append borrowed owner to renderer+1B0C BEFORE release of current COM output,
// then original x87 accounting. Failed HRESULT does not become a null fallback.
NativeSurfaceOwnerStorage* create_native_renderer_render_target_00b2a7c0(void* renderer,
    std::uint32_t width, std::uint32_t height, std::uint32_t format,
    std::uint32_t multisample, NativeRendererSurfaceFactoryContext&,
    NativeRendererSurfaceFactoryAcquired&);

// Complete526-byte B2A9A0: ECX renderer; width,height,format,multisample,
// FLOAT quality, low-byte discard; RET18h/EAX actual creator. Preserve x87
// FISTP64 truncation then LOW32 quality, flags100/kind1, account BEFORE append,
// then COM release. Retry never clears the original output cell.
NativeSurfaceOwnerStorage* create_native_renderer_depth_surface_00b2a9a0(void* renderer,
    std::uint32_t width, std::uint32_t height, std::uint32_t format,
    std::uint32_t multisample, float quality, std::uint8_t discard,
    NativeRendererSurfaceFactoryContext&, NativeRendererSurfaceFactoryAcquired&);

using NativeDepthSurfaceDecrement = long (__stdcall *)(volatile long*);
struct NativeRenderResourcesDepthContext {
    NativeRendererSurfaceFactoryContext& factory;
    const volatile std::uint32_t* renderer_profile_00d5f0a8; // slot94=B2A9A0.
    const volatile std::uint32_t* surface_profile_00d619a0;
    NativeDepthSurfaceDecrement const volatile& decrement_iat_00ce2220;
};
// Complete104-byte B0FC10. ECX service; width,height,UNUSED DWORD; RET0Ch.
// Null+1C8 skips all work. Otherwise clear captured frame depth, capture current
// renderer94 and call B2A9A0(width,height,4B,0,+0.0f,1); reread current+1C8,
// retained depth assignment, then drop returned creator through current CE2220.
// Actual frame and factory domains are shared. No synthetic frame or surface.
void resize_native_render_resources_depth_00b0fc10(void* actual_service,
    std::uint32_t width, std::uint32_t height, std::uint32_t unused_argument,
    NativeRenderResourcesDepthContext&, NativeRendererSurfaceFactoryAcquired&);

// Factories require fresh persistent acquired state and the bound actual static
// surface pool. EH only returns a failed constructor's raw slot and handles the
// optional guard; it does not undo a completed owner, COM output or publication.
// Current renderer/global changes must satisfy existing provider contracts.
// Source interfaces do not establish native caller/FH3/SEH/gameplay equivalence.
} // namespace bsp
