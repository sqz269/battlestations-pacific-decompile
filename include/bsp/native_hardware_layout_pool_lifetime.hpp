#pragma once

#include "bsp/allocator_list.hpp"

namespace bsp {

// Complete B604D0: ECX actual pool, EAX same pool, RET. New MSVC Win32 C++
// interface over borrowed, aligned native 38h storage. The first 12 bytes are
// an actual AllocatorListElement; +0C is an actual Win32 CRITICAL_SECTION.
// Establish the genuine D62AF0/B60350 binding in this same shared list domain
// before calling: real new-handler processing can reach the published node.
// Call once for uninitialized storage; callers preserve the existing shared list.
void* initialize_native_hardware_layout_pool_00b604d0(
    void* pool, AllocatorListDomain& list);

// Complete B60270: ECX actual initialized pool, RET. Frees current slabs and
// table, drains positive recursion, deletes the section and unlinks the node.
// It neither destroys slot owners nor clears retained native fields. The
// actual pool storage and its host virtual binding remain caller-owned.
void destroy_native_hardware_layout_pool_00b60270(
    void* pool, AllocatorListDomain& list) noexcept;

// Complete B60020: ECX actual table header (pool+28), RET. Frees its current
// nonnull first DWORD and clears nothing. It does not receive a pool pointer.
void free_native_hardware_layout_pool_table_00b60020(
    void* actual_table_header) noexcept;

} // namespace bsp
