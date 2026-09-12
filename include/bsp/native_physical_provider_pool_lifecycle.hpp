#pragma once

#include "bsp/allocator_list.hpp"

namespace bsp {

// Complete BF2E30[0Eh]. ECX raw0Ch table header, RET. Free the captured
// nonnull backing pointer; retain pointer, count and capacity (including stale
// freed pointer). This is the BF3250 table-unwind operation, not slot return.
void free_native_physical_provider_pool_table_00bf2e30(void* actual_table_header) noexcept;

// Complete BF3250[D3h]. Native ECX fresh raw38h owner, EAX same owner, RET.
// Caller first binds this owner's D68E68/BF3430 trim through the SAME E188B4
// domain below. Prepend base/profile, initialize real CS+0C/depth24 and headers,
// publish cap32 before80h allocation. FH3 cleanup order: table, section, links.
void* initialize_native_physical_provider_pool_00bf3250(void* actual_pool,
    AllocatorListDomain& actual_list);

// Complete BF3430[A0h]. Native ECX owner; D68E68 virtual0; RET. No internal
// locking. Free entirely unused1F4h blocks, move last pointer into each hole,
// rewrite all eight moved slot+38 indices, retry hole, recompute first available.
void trim_native_physical_provider_pool_00bf3430(void* actual_pool) noexcept;

// Complete BF33A0[8Ah]. Native ECX owner, RET. Free every block and table,
// drain positive signed depth, delete the actual CS, publish base and unlink.
// Does not destroy providers, clear headers/own links, or free the owner itself.
void destroy_native_physical_provider_pool_00bf33a0(void* actual_pool,
    AllocatorListDomain& actual_list) noexcept;

// Host setup only: bind the recovered virtual0 before publishing a fresh pool.
// No raw bytes, list links or CRT callbacks change here. Borrow the application's
// real shared domain; callers of the pool primitives use this same pool/domain.
void bind_native_physical_provider_pool_trim(void* actual_pool,
    AllocatorListDomain& actual_list);

// Host setup for the ONE actual static0109DBF0 owner, also binding its trim.
// Pool storage and list must outlive real CRT exit. Bind once before startup;
// these are borrowed host references, not a private pool or allocator registry.
void bind_static_native_physical_provider_pool_0109dbf0(void* actual_pool,
    AllocatorListDomain& actual_list);

// Complete CD9010..CD9025: initialize bound pool, std::atexit(CE10F0 source
// callback), EAX actual registration status; no rollback on registration failure.
int initialize_static_native_physical_provider_pool_00cd9010();

// Complete CE10F0..CE10F9: select same0109DBF0 owner, tail JMP BF33A0.
void destroy_static_native_physical_provider_pool_00ce10f0() noexcept;

// New MSVC Win32 C++ interfaces. Complete operational bodies, with source SEH
// cleanup expressing the reviewed FH3 states. No original FH3/typeinfo/CRT ABI,
// static-image address identity, concurrent trim, repeated startup or game proof.
// BF30C0/BF3670 belong to a different physical STREAM pool.
} // namespace bsp
