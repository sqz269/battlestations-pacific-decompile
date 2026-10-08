#pragma once
#include "bsp/allocator_list.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {
inline constexpr std::size_t native_enum_node_slot_bytes = 0x14;
inline constexpr std::size_t native_enum_node_page_bytes = 0x584;
inline constexpr std::uint32_t native_enum_node_pool_profile = 0x00ce37a4;

// New Win32 interfaces for six complete ordinary normal bodies. Borrow real
// raw38h owners and the SAME00E188B4 list domain; never substitute zeroed storage
// for construction. Real OS critical section+0C, depth24, table28/count2C/cap30,
// earliest page34. Native64-slot pages preserve payload0..F and padding582..583.
// Slot+10 is allocator-owned page identity, including after trim movement.
void* initialize_native_enum_node_pool_00411050(void* actual_pool,
    AllocatorListDomain& actual_list);
void destroy_native_enum_node_pool_00410a60(void* actual_pool,
    AllocatorListDomain& actual_list) noexcept;
void* initialize_native_enum_node_page_004e6370(void* actual_page,
    std::uint32_t page_index) noexcept;
void* allocate_native_enum_node_004e7c00(void* actual_initialized_pool);
void return_native_enum_node_0043b0a0(void* actual_initialized_pool,
    void* actual_allocated_slot);
void trim_native_enum_node_pool_00410cd0(void* actual_pool) noexcept;

// Source host setup only: bind exact CE37A4 -> COMPLETE real00410CD0 before
// constructing/publishing this borrowed owner. No physical fields or foreign
// profile/default callback are created. The list binding must outlive the pool.
void bind_native_enum_node_pool_virtual0_00ce37a4(void* actual_pool,
    AllocatorListDomain& actual_list);

// Admission: successful nonoverflowing current CRT allocations, coherent finite
// tables, valid unique aligned allocated slots, externally synchronized trim,
// no alias/reentry/concurrent mutation. Keep native DWORD publications/reloads;
// no new overflow guard, payload destruction, metadata reset or rollback policy.
// Destroy owning payloads before returning/freeing their slots. Original EH/SEH,
// class/vtable ABI, static owner wrappers, CRT fault/failure parity, dictionary
// insertion/declaration ownership, traffic and game binding remain external.

// Distinct borrowed host binding for Original 00E175B0: one actual aligned,
// initially unconstructed38h property-map node pool and the SAME canonical
// 00E188B4 domain. Establish the genuine CE37A4/00410CD0 trim binding before
// publishing borrowed pointers. This neither constructs/reset the owner/list
// nor supplies sibling pool storage, an empty CString or property records.
// Bind once before startup; owner/list/binding remain alive through callback.
void bind_static_native_property_record_pool_00e175b0(void* actual_pool,
    AllocatorListDomain& actual_list);

// Complete Original CC8A30..CC8A45: initialize the bound actual owner with
// whole00411050, then real std::atexit for the matching CD9260 Source callback.
// Return real registration status; no once guard, retry or failure rollback.
// Construction failure propagates before registration; no own EH is added.
int initialize_static_native_property_record_pool_00cc8a30();

// Complete Original CD9260..CD9269: destroy SAME owner/list with whole00410A60.
// Meet the raw helper's payload-before-pages precondition. These are new Win32
// ordinary Source interfaces; original ABI/static-dispatcher/game publication,
// rebinding/repeated lifecycle, populated maps and record lifetime stay external.
void destroy_static_native_property_record_pool_00cd9260() noexcept;
} // namespace bsp
