#pragma once
#include "bsp/native_frame_clock_actual.hpp"
#include <cstdint>
namespace bsp {
// Persistent borrowed context. These are the SAME actual mutable AA0/AB0
// cells used by every raw consumer, and the real D68D50 method-profile view.
// Retain this context, its method context and both cells through manager drain
// and any worker joins. No publication cell, process owner or clock is created.
struct NativeFrameClockLifetimeContext final {
    void* volatile& actual_manager_publication_01090aa0;
    void* volatile& actual_clock_publication_01090ab0;
    const NativeFrameClockActualContext& methods;
};
// BEDE00[145]: native ECX self / EAX same / RET. Source adds EDX context.
// Stamp D68D20, enter the captured first manager section, publish self, get the
// current manager AGAIN, reload current AB0, register it, leave captured section.
void* __fastcall construct_native_frame_clock_base_00bede00(
    void* actual_clock, NativeFrameClockLifetimeContext&);
// BEDEA0[153]: same captured-section/current-manager order; unregister CURRENT
// AB0 (even if it differs from self), clear it, leave, then stamp CE3818.
void __fastcall destroy_native_frame_clock_base_00bedea0(
    void* actual_clock, NativeFrameClockLifetimeContext&);
// BEDFB0[156]: caller supplies a genuine 80h allocation in the same allocation
// domain as singleton_lifetime_free. Preserve +60 until QPF and untouched
// +6A..6F/+70/+78; set native members in order and call the existing raw init.
// If initialization raises a source C++ exception, destroy only the base;
// allocation ownership/cleanup remains with the caller.
void* __fastcall construct_native_frame_clock_00bedfb0(
    void* actual_clock, NativeFrameClockLifetimeContext&);
// BEE110[36]: stamp D68D50, base destruction, then test flags bit0 and free.
// Native ECX self / stack flags / EAX captured pointer / RET4, extra EDX context.
// The current manager must remain alive through destruction and manager drain.
void* __fastcall delete_native_frame_clock_00bee110(
    void* actual_clock, NativeFrameClockLifetimeContext&, std::uint32_t flags);
// Profiles are identity DWORDs, not executable source vtables. C++ cleanup
// reproduces the established ordinary unwind actions; original FH3 stack maps,
// arbitrary exception-spill aliases, SEH/hardware faults and CRT identity are
// not claimed. This module does not bind application/frame/sound/input/worker
// consumers or create a second advancing clock.
} // namespace bsp
