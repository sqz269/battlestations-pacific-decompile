#pragma once

#include "bsp/native_event_owner.hpp"
#include "bsp/native_renderer_cache_clear.hpp"

namespace bsp {

// Borrow the SAME actual renderer, global bytes, owner lifetimes and profiles
// as the complete binding/cache providers. No owners or events are copied.
struct NativeRendererStopPipelineContext {
    NativeRendererBindingResetContext& actual_bindings;
    NativeRendererCacheClearContext& actual_cache;
    // At least three original-token DWORDs. Current event profile D6821C and
    // current slot+08 BD17C0 select the complete real Win32 wait provider.
    const volatile std::uint32_t* actual_event_profile_00d6821c;
};

// Complete B33BF0..B33BFD, original ECX worker, RET via tail jump; forwards
// the current wait's EAX. Write raw run byte+04=0, THEN read current ack+10,
// its current profile and slot+08, then WaitForSingleObject(current HANDLE,
// INFINITE). Worker storage through+13 and actual eight-byte event stay live.
// B33DA0 produces ack+10 with BD1970(CL=0), which publishes D6821C.
std::uint32_t request_native_renderer_worker_stop_00b33bf0(void* actual_worker,
    const volatile std::uint32_t* actual_event_profile_00d6821c) noexcept;

// Complete B26920..B26A00, original ECX renderer, no stack args, RET.
// Current virtual texture slots0..19, vertex streams0..3, index(null,0), full
// B241C0(renderer+34), current default color+197C at slot0, guarded current
// device SetDepthStencilSurface(null). All binding bodies and reached owner
// terminal providers execute; no null-unbind projection or cache substitute.
void clear_native_renderer_stop_pipeline_00b26920(void* actual_renderer,
    NativeRendererStopPipelineContext&);

// Complete B28A90..B28ABB, original ECX renderer, no stack args, RET.
// Optional current worker+1970 stop/ack wait; B33AA0(raw0); full B26920;
// actual Sleep(100), only after returning pipeline. Does not clear the worker,
// signal wake, join a thread, branch on wait result, or add timeout/recovery.
void stop_native_renderer_worker_mode_00b28a90(void* actual_renderer,
    NativeRendererStopPipelineContext&);

// Complete normal bodies in the existing providers' actual-profile domains:
// renderer D5F0A8, actual event D6821C, and cache's ten terminal profiles.
// Actual renderer touched extent is 1BC8h including nested counters. Current
// default surface must be valid; raw pointers remain valid at native accesses.
// Bindings/cache contexts share canonical globals/owners/pools/allocators.
// Pipeline state0 arms only AFTER final device/table capture. Guard fields
// remain uninitialized when entry is skipped; enabling cleanup on such storage
// is outside the valid native domain. No binding/cache rollback is added.
// A never-started worker can wait forever. Full worker lifecycle, native FH3/
// hardware SEH, arbitrary concurrent mutation and game validation are unproved.
// These are new source interfaces, not drop-in original ABI replacements.
} // namespace bsp
