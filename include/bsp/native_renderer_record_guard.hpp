#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native renderer record guard requires MSVC Win32.
#endif

namespace bsp {
// Borrow the SAME actual mutable cells used by the raw manager and all record
// producers. The bindings outlive publication, worker use and manager drain.
struct NativeRendererRecordGuardContext {
    void* volatile& actual_manager_01090aa0;
    void* volatile& actual_guard_0108d5a0;
};

// B22360[17]: clear current D5A0 unconditionally, then stamp owner CE3818.
// This is constructor unwind cleanup, not manager unregister or owner free.
void destroy_native_renderer_record_guard_base_00b22360(
    void* owner, NativeRendererRecordGuardContext& context) noexcept;

// B23750[69]: raw eight-byte owner, D5E60C and a genuine BD1860 section at+4.
// On section-creation failure run B22360, leaving allocation cleanup to caller.
void* construct_native_renderer_record_guard_00b23750(
    void* owner, NativeRendererRecordGuardContext& context);

// B25BE0[189]: captured hot return; cold path locks the FIRST manager's section,
// rechecks, allocates/constructs/publishes, gets the CURRENT manager and registers
// the CURRENT publication. Return reloads D5A0 after unlock. Registration failure
// retains publication/allocation. Constructor failure frees captured allocation.
void* get_native_renderer_record_guard_00b25be0(
    NativeRendererRecordGuardContext& context);

// B26130[55]: stamp D5E60C, release actual owner+4, clear current D5A0, stamp
// CE3818, free only for flags&1. Return captured owner even after free. No
// unregister occurs: the raw manager pops before dispatching this scalar.
void* delete_native_renderer_record_guard_00b26130(
    void* owner, std::uint32_t flags, NativeRendererRecordGuardContext& context);

// Complete source schedules over actual raw storage. These interfaces add an
// explicit context; original register/private-frame, FH3 and hardware-fault
// compatibility are not established. No application publication is activated.
} // namespace bsp
