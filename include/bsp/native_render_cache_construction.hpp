#pragma once

#include <cstdint>

namespace bsp {

// Full B27E80, ordinary unpublished-storage construction. Original ECX is the
// first of twenty ACh banks; EAX returns that address, RET. Minimum span D70h.
// Per bank: zero owner+A8 FIRST, then DWORDs00..1C, then byte20. Preserve gaps.
// Its subsequent owner read is provably null: no intervening call/overlap.
// This does not release references from a live bank or support concurrent writes.
void* construct_native_render_cache_banks_00b27e80(void* actual_banks) noexcept;

// Full B29430 within the existing B659D0 finite-CRT contract and ordinary
// unpublished-storage domain. Original ECX=cache, EAX=same cache, RET.
// Borrow a minimum193Ch actual span and the SAME live D7A24C word. The actual
// CameraPlaneSet at cache+178C must already have its C++ lifetime established,
// with the intended native preimage restored before entry. No replacement
// planes, full renderer/cache owner, allocator or terminal provider is created.
//
// Initializes30 owner cells; all26 visited by final B241C0 are then null.
// Calls only that reset's private, proved all-null construction fragment.
// General nonnull B241C0 remains unimplemented. Gamma1938 is separately zeroed
// before reset; reset preserves it, planes178C..18CF,18D0 and other native gaps.
// See docs/NATIVE_RENDER_CACHE_CONSTRUCTION.md for concrete lifetime setup,
// exact write order, the26/30 proof, and inherited exceptional-CRT boundaries.
void* construct_native_render_cache_00b29430(void* actual_cache,
    const volatile std::uint32_t& live_one_00d7a24c);

// These are new C++ interfaces, not original register/stack ABI shims.

} // namespace bsp
