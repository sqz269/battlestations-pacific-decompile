#pragma once

#include <cstdint>

namespace bsp {
class AllocatorListDomain;

// Borrow an ALREADY initialized raw38h pool corresponding to109DBF0, linked
// into the application's existing E188B4 allocator-list domain with its actual
// D68E68/BF3430 trim binding. This context owns neither object. The initializer
// must have established the real CRITICAL_SECTION+0C and current pool headers.
// Primitive calls do not initialize, link, bind, trim or destroy anything in the
// domain; the reference records the required common domain, not a private list.
struct NativePhysicalProviderPoolContext {
    void* actual_initialized_pool_0109dbf0;
    AllocatorListDomain& actual_allocator_list;
};

// Complete BF2D50[64]. Original ECX raw1F4h block, stack block index, EAX same,
// RET4. Write free count8, reverse WORD indices7..0, and each3Ch slot's +38
// block index only. All provider bytes and block+1F2 padding remain untouched.
void* initialize_native_physical_provider_block_00bf2d50(void* actual_block,
    std::uint32_t block_index) noexcept;

// Complete BF34D0[316]. Original ECX raw38h pool; EAX raw3Ch slot; RET.
// Current block pointer/count/capacity/available fields at+28/+2C/+30/+34;
// real section+0C and depth+24. Native capacity arithmetic wraps. No extra
// failure rollback or exceptional unlock is installed; callers own recovery.
void* acquire_native_physical_provider_slot_00bf34d0(NativePhysicalProviderPoolContext&);

// Complete BF2FC0[105]. Original ECX pool, stacked actual slot, RET4. Uses
// the retained slot+38 block index and signed block-relative division by3Ch,
// returns its WORD index to the current free stack, lowers available if needed.
void return_native_physical_provider_slot_00bf2fc0(
    NativePhysicalProviderPoolContext&, void* actual_slot);
// Complete BF3200[12]. Original ECX slot; pushes it, selects ECX=109DBF0,
// CALL BF2FC0, RET. The source explicitly borrows that same initialized pool.
void return_native_physical_provider_slot_00bf3200(void* actual_slot,
    NativePhysicalProviderPoolContext&);

// New C++ interfaces, not drop-in ABI/FH3 replacements. Descriptive names are
// hypotheses. Pool lifecycle/CD9010 startup, BF3430 trim, provider construction
// and deletion are separate packets. No renderer-pool substitution is valid.
} // namespace bsp
