#pragma once
#include <cstdint>
namespace bsp {
struct NativeRendererResetProcessContext;
struct NativeRendererDefaultSurfacesContext;
struct NativePhysicalBufferOwnerContext;

// The ten original DWORD slots, including the non-boolean fullscreen/sync words.
// Borrow the live slots: later fullscreen, multisample and x87 reads are current.
struct NativeRendererDeviceStartupSlots {
    std::uint32_t window, fullscreen, width, height, backbuffer_format;
    std::uint32_t backbuffer_count, multisample, depth_format, presentation_sync;
    std::uint32_t fullscreen_refresh_rate;
};
static_assert(sizeof(NativeRendererDeviceStartupSlots) == 40);
struct NativeRendererDeviceStartupContext {
    NativeRendererResetProcessContext& reset;
    const NativeRendererDefaultSurfacesContext& default_surfaces;
    NativePhysicalBufferOwnerContext& physical;
    // Must alias reset.actual_render_thread_0108d4c8, with mutable access here.
    volatile std::uint32_t& actual_render_thread_0108d4c8;
    const volatile float& unsigned_bias_00ce3978; // original bits 4F800000
    const volatile double& widescreen_limit_00cf5750; // bits 3FF5555560000000
};
static_assert(sizeof(NativeRendererDeviceStartupContext) == 24);

// Complete B2AEB0..B2B1F1 normal body. Original ECX renderer, ten stack DWORDs,
// RET28, void. This source ABI borrows the raw slots and the SAME actual domains
// used by Reset/recreation/default surfaces: initialized pools, current renderer
// D5F0A8 and pooled D61E58/D61E7C profiles, publication and synchronization.
// Native ignores HRESULTs, uses direct output fields and has no startup rollback.
// Only each pooled constructor has a raw-slot-return cleanup state. Callers must
// provide valid reached raw storage; no full renderer/pool initialization here.
void initialize_native_renderer_device_00b2aeb0(void* actual_renderer,
    const volatile NativeRendererDeviceStartupSlots& actual_slots,
    NativeRendererDeviceStartupContext&);

// Complete 12-byte ECX raw-slot/no-stack-argument/RET native convenience thunks.
// New source interfaces borrow the actual initialized pool through the context.
void return_native_index_buffer_slot_00b49940(void* actual_raw_slot,
    const NativePhysicalBufferOwnerContext&);
void return_native_vertex_buffer_slot_00b49950(void* actual_raw_slot,
    const NativePhysicalBufferOwnerContext&);
// Source interfaces and bounded fixtures do not establish original caller ABI,
// FH3/private stack aliases, hardware-fault SEH, concurrency or gameplay parity.
} // namespace bsp
