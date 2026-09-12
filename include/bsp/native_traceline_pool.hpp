#pragma once

#include "bsp/allocator_list.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {

inline constexpr std::size_t native_traceline_payload_bytes = 0x1b8;
inline constexpr std::size_t native_traceline_slot_bytes = 0x1bc;
inline constexpr std::size_t native_traceline_slab_bytes = 0x37c4;
inline constexpr std::size_t native_traceline_pool_bytes = 0x38;

// Complete AF19D0: ECX actual37C4h slab, stack slab ID; EAX same slab; RET4.
// Write WORD37C0=32, reverse WORD indices31..0 at3780 and 32 IDs at1B8
// stride1BC. Preserve every1B8h payload and final padding WORD37C2.
void* initialize_native_traceline_slab_00af19d0(
    void* actual_slab, std::uint32_t slab_index) noexcept;

// Complete AF32F0: ECX actual initialized38h pool, EAX raw1BCh slot, RET.
// Real CRITICAL_SECTION+0C/depth24; actual table28/count2C/capacity30/
// earliest34. Wrapped growth and native publication order. No EH frame or
// exceptional unlock: allocation failure retains lock/depth/publications.
void* allocate_native_traceline_slot_00af32f0(void* actual_pool);

// Complete AF1D30: ECX pool, stack raw slot; RET4. Retained ID1B8 selects
// slab; signed wrapped displacement /444 truncates toward zero, then WORD.
// Reload count after free-index write, lower earliest unsigned. No validation.
void return_native_traceline_slot_00af1d30(void* actual_pool, void* actual_raw_slot);

// Complete AF1EA0: ECX raw slot; selectF8C288; CALL AF1D30; RET.
// Borrow that SAME actual pool, including constructor-unwind raw returns.
void return_native_traceline_00af1ea0(void* actual_raw_slot, void* actual_pool_00f8c288);

// Complete AF1AC0: ECX table header, RET. Free nonnull current backing;
// preserve pointer/count/capacity. The owner's header starts at+28.
void free_native_traceline_pool_table_00af1ac0(void* actual_table_header) noexcept;

// Complete AF3170: ECX fresh38h owner; EAX same; RET. Prepend to the SAME
// application's E188B4 list, profileD5D924, real section, reserve32 pointers.
// Verified FH3 unwind actions: table -> section -> base. New C++ exception ABI.
void* initialize_native_traceline_pool_00af3170(void* actual_pool,
    AllocatorListDomain& actual_list);

// Complete AF1C70: ECX owner; RET. Ascending slab frees, table free, drain
// positive signed depth/DeleteCS and same-list unlink. Payloads are not destroyed.
// Retains stale table/count/capacity/links; does not physically free owner.
void destroy_native_traceline_pool_00af1c70(void* actual_pool,
    AllocatorListDomain& actual_list) noexcept;

// Complete AF3250: ECX owner, D5D924 virtual0; RET. Free empty32-slot slabs,
// move last pointer, rewrite all32 IDs, retry hole and recompute earliest.
// No internal lock; retain table capacity. Caller must ensure stable ownership.
void trim_native_traceline_pool_00af3250(void* actual_pool) noexcept;

// Host binding, no pool mutation: install real trim for the same actual owner
// and shared list BEFORE construction publishes the element. Borrow until exit.
void bind_static_native_traceline_pool_00f8c288(void* actual_pool,
    AllocatorListDomain& actual_list);
// Complete CD77F0 (raw body endingCD7805): construct SAME pool, register real
// std::atexit(CE0B70 source binding), EAX registration status; RET.
int initialize_static_native_traceline_pool_00cd77f0();
// Complete CE0B70: select SAMEF8C288, tail AF1C70; no arguments.
void destroy_static_native_traceline_pool_00ce0b70() noexcept;

// Host inspection, no original routine: exact slot boundary and retained ID
// agree with a current slab table entry. Does not prove allocated/live state.
// Caller keeps the actual initialized pool/slabs stable during the inspection.
bool owns_native_traceline_slot(const void* actual_pool, const void* actual_slot) noexcept;

// Descriptive names are hypotheses; new MSVC Win32 C++ interfaces, not drop-in
// original ABI entries. This is the 1BCh Traceline pool atF8C288, distinct from
// the BAD6F0 7B0h tracer pool and the188h Model pool. No second semantic pool.
} // namespace bsp
