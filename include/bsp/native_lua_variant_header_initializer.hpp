#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Lua variant header initialization requires MSVC Win32.
#endif

namespace bsp {

// Provisional Source entry for 006EEAF0..006EEB1A. actual_header is the opaque
// identity returned unchanged; the references MUST bind to its actual pointer
// cell+4 and actual DWORD count+8. They are not copies or detached projections.
// No complete header/node type or original thiscall interface is introduced.
//
// Call the admitted allocate_native_lua_variant_link_record_006edea0 provider,
// publish its actual allocation through head+4, then set that allocation's
// BYTE+31h=1. Freshly read the volatile head cell before EACH ordered pointer
// DWORD store +4, +0, +8; each store uses that read for both target and value.
// Zero the actual count only after those stores, then return actual_header.
// There is no direct header+0 access, head-read hoisting or node copy.
//
// Caller supplies live, aligned, type-compatible cells at the stated offsets
// and usable pointer-store backing for every reloaded head. Compatible changes
// between reads retain the ordered reload schedule. This supplies no thread
// synchronization or arbitrary Native raw-alias/fault equivalence. Keeping
// header+0 and other node bytes untouched also requires that the specified
// target stores not alias those other locations. No guards enforce the domain.
//
// The actual allocator's usable aligned nonnull52-byte success domain and
// current-host-CRT throw policy apply. Allocation failure precedes this body's
// header writes; there is no catch, rollback, cleanup or noexcept promise.
// Native allocator/handler/CRT binding, registers, flags, stack, fault timing,
// EH/caller ABI, production lifetime, startup and gameplay remain unproved.
void* initialize_native_lua_variant_header_006eeaf0(
    void* actual_header,
    void* volatile& actual_head_04,
    volatile std::uint32_t& actual_count_08);

} // namespace bsp
