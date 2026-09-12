#pragma once

#include "bsp/allocator_list.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {

inline constexpr std::size_t native_tracer_payload_bytes = 0x7ac;
inline constexpr std::size_t native_tracer_slot_bytes = 0x7b0;
inline constexpr std::size_t native_tracer_slab_bytes = 0x3d94;

// Complete BA9B50: ECX actual3D94h slab, stack slab ID; EAX same slab; RET4.
// Eight7B0h slots, trailing ID7AC, free indices7..0 at3D80, free count3D90.
// Preserve every7ACh payload and final padding WORD3D92. No tracer construction.
void* initialize_native_tracer_slab_00ba9b50(
    void* actual_slab, std::uint32_t slab_index) noexcept;

// Complete BAC4A0: ECX actual initialized38h pool; EAX raw slot; RET.
// Real section0C/depth24; table28/count2C/capacity30/earliest34. Actual CRT
// growth preserves unsigned wrap and publication order. No native EH frame:
// allocation failure retains the lock, depth and all preceding publications.
void* allocate_native_tracer_slot_00bac4a0(void* actual_pool);

// Complete BABF70: ECX pool, stack raw slot; RET4. Signed wrapped displacement
// divided by1968 with truncation toward zero, truncated WORD free index,
// reloaded free count, unsigned earliest lowering. No destruction/validation.
void return_native_tracer_slot_00babf70(void* actual_pool, void* actual_raw_slot);

// Complete BAC660: discard incoming ECX (caller7AC), select0109049C, tail BAC4A0.
// The source adapter borrows that actual owner rather than allocating a pool.
void* allocate_native_tracer_00bac660(void* actual_pool_0109049c);

// Complete BAC2B0: ECX raw slot; select0109049C, call BABF70; RET.
// Constructor unwind returns unconstructed storage without tracer destruction.
void return_native_tracer_00bac2b0(void* actual_raw_slot, void* actual_pool_0109049c);

// Complete BA9C80: ECX0Ch table header; RET. Free current nonnull backing,
// preserving pointer/count/capacity. This header is at+28 in the actual owner.
void free_native_tracer_pool_table_00ba9c80(void* actual_table_header) noexcept;

// Complete BAC3C0: ECX fresh38h owner, EAX same owner; RET. Prepend to the SAME
// application's E188B4 list, profileD63F90, real CS, reserve32 pointer cells.
// Abnormal termination follows verified FH3 table -> section -> base order.
void* initialize_native_tracer_pool_00bac3c0(void* actual_pool,
    AllocatorListDomain& actual_list);

// Complete BABDD0: ECX owner; RET. Ascending slab frees, current table free,
// positive signed depth drain/DeleteCS, base profile and same-element unlink.
// No tracer payload destruction, stale metadata clearing or physical owner free.
void destroy_native_tracer_pool_00babdd0(void* actual_pool,
    AllocatorListDomain& actual_list) noexcept;

// Complete BABED0: ECX owner, D63F90 virtual0; RET. Free wholly empty8-slot
// slabs, move last table pointer, rewrite ALL eight IDs, retry hole, recompute
// earliest from captured table/current count. No internal lock; retain capacity.
void trim_native_tracer_pool_00babed0(void* actual_pool) noexcept;

// Host binding: real trim before shared-list publication. Borrow actual static
// owner and domain until CRT exit; this changes no pool bytes, links or callbacks.
void bind_static_native_tracer_pool_0109049c(void* actual_pool,
    AllocatorListDomain& actual_list);

// Complete CD88D0: construct SAME owner, real std::atexit(CE0F90 source binding),
// EAX actual registration status; RET. No rollback/repeated-startup promise.
int initialize_static_native_tracer_pool_00cd88d0();

// Complete CE0F90: select SAME0109049C; tail BABDD0; no arguments.
void destroy_static_native_tracer_pool_00ce0f90() noexcept;

// Names are hypotheses; new MSVC Win32 C++ interfaces, not original ABI entries.
// Bind real trim before publication. Every reached raw address needs backing.
// Physical BAD6F0 tracer construction, original FH3 dispatch and gameplay are
// separate boundaries; a raw7B0 slot is not a successfully constructed tracer.
} // namespace bsp
