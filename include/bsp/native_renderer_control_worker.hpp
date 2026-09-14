#pragma once

#include "bsp/frame_clock.hpp"
#include "bsp/native_event_owner.hpp"
#include <cstddef>
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual renderer control worker requires MSVC Win32.
#endif

namespace bsp {
struct NativeRendererBeginFrameContext;
using NativeRendererControlCallback = void (__thiscall*)(void* actual_context);

// Actual24h owner; no field initializers or host vptr. The constructor leaves
// bytes06..07, rate18, callback1C and callback-context20 unchanged.
struct NativeRendererControlWorkerStorage {
    volatile std::uint32_t profile_00;
    volatile std::uint8_t run_04, shutdown_05;
    std::byte untouched_06[2];
    void* volatile thread_08;
    NativeEventOwnerStorage* volatile wake_0c;
    NativeEventOwnerStorage* volatile idle_10;
    volatile std::uint32_t thread_id_14, rate_18;
    NativeRendererControlCallback volatile callback_1c;
    void* volatile callback_context_20;
};
static_assert(sizeof(NativeRendererControlWorkerStorage)==0x24);
static_assert(offsetof(NativeRendererControlWorkerStorage,run_04)==4);
static_assert(offsetof(NativeRendererControlWorkerStorage,wake_0c)==0xc);
static_assert(offsetof(NativeRendererControlWorkerStorage,thread_id_14)==0x14);
static_assert(offsetof(NativeRendererControlWorkerStorage,callback_context_20)==0x20);

// Stable process bindings, with CURRENT publication values read at each native
// global access. Original-token tables are data, never callable EXE addresses.
// The actual begin context must carry its existing substantive providers.
// EndFrame B2F4A0 -> B2D8E0 has no complete actual provider in this packet:
// callers must supply a real implementation before that path can execute.
struct NativeRendererControlWorkerContext {
    void* const volatile& actual_clock_01090ab0;
    void* const volatile& actual_renderer_00f8d394;
    volatile std::uint32_t& actual_time_bits_0108d6e4;
    const volatile std::uint32_t* actual_event_profile_00d6821c;
    const volatile std::uint32_t* actual_clock_profile_00d68d50;
    const volatile std::uint32_t* actual_renderer_profile_00d5f0a8;
    NativeRendererBeginFrameContext* begin_frame;
    void (*end_frame_00b2f4a0)(void* actual_renderer,void* borrowed_context);
    void* end_frame_context;
};

// One immutable process binding supports CreateThread(raw owner), without an
// owner map or stealing the native callback/context fields. Rebinding to a
// different context is rejected. Retain the context and every borrowed provider
// through all worker joins; no unbind or automatic teardown is provided.
void bind_native_renderer_control_worker_process_context(NativeRendererControlWorkerContext&);

// Complete91-byte BEE080 raw body, including QPC's ignored BOOL and output
// writes even on failure. The native local QPC buffer keeps stack preimage.
// ECX actual80h clock, EDX unused, stack output, EAX output, RET4.
ClockTimestamp* __fastcall sample_native_frame_clock_00bee080(
    const void* actual_clock,void* unused_edx,ClockTimestamp* output);

// Exact timing fragment B33C72..B33CE3, reused by the full source thread.
// Preserves x87/SSE order, two float stores, signed64 conversion, unsigned rate
// correction, unordered compares, and incoming FPU/MXCSR state. It writes the
// rounded sample bits to output but does not publish the global itself.
bool native_renderer_worker_interval_00b33c72(const ClockTimestamp&,
    const volatile std::uint32_t& previous_time_bits,
    const volatile std::uint32_t& rate,std::uint32_t& sample_time_bits) noexcept;

// Complete normal lifetime bodies. Original ECX owner; scalar stackflags/RET4.
// Source constructor uses real auto-reset events and CreateThread suspended,
// priority-2, then ResumeThread. API failures are not converted into success.
// No exception rollback or run/rate/callback setup is added.
NativeRendererControlWorkerStorage* construct_native_renderer_control_worker_00b33da0(void* fresh);
void destroy_native_renderer_control_worker_00b33b50(NativeRendererControlWorkerStorage&);
NativeRendererControlWorkerStorage* delete_native_renderer_control_worker_00b33c00(
    NativeRendererControlWorkerStorage*,std::uint32_t flags);

// Native Win32 thread entry, stack actual owner/RET4 convention; normal exit is
// ExitThread(0). Inner run loop does not test shutdown; destructor therefore
// needs the original stop protocol if run remains set. Missing reached
// providers/profiles throw a source contract error, never no-op successfully.
unsigned long __stdcall native_renderer_control_worker_thread_00b33c20(void* actual_worker);
} // namespace bsp
