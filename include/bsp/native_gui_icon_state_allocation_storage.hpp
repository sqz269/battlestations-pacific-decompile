#pragma once
#include <cstdint>

namespace bsp {
// Qualified raw allocation support for AB1520..AB1570 (81B/25 rows).
// Native ECX count/EAX allocation/plain RET become this ordinary Source ABI.
// Preserve the full count DWORD once: zero still requests allocation(0), and
// nonzero UINT32_MAX/count < 40h takes the overflow branch. Otherwise request
// count<<6 bytes through the EXISTING singleton_lifetime_allocate transport,
// with its existing object role and identical native_bytes/host_bytes.
//
// Overflow throws CURRENT Source std::bad_alloc; the existing allocator also
// supplies CURRENT malloc/_callnewh/retry/bad_alloc policy. No replacement
// allocator callback, new-handler installation, noexcept/terminal/retry policy
// or Native-origin/cleanup permission is introduced. Native BF6340 exception
// scratch, D6923C vtable, E03CC0 ThrowInfo, BF6885 and original OOM/SEH/FH3/fault
// ABI are not reconstructed by the Source error projection.
//
// Returned bytes are raw, uninitialized backing, including the zero request;
// no state/subobject/T[n] lifetime, vector owner, registry/custody/disposition
// or caller/production activation is created. Retain genuine Source allocator/
// new-handler/module/control lifetimes across callbacks and escapes; private
// Source request/control backing is separate from callback-visible data.
// No new cap, zero shortcut, Frame/Context, field seed, cleanup or FP policy.
// Keep the original high STL allocator classification/name unchanged. This
// storage support adds zero Original/vendor/helper/CG/duplicate-body credit.
void* allocate_native_gui_icon_state_backing_00ab1520(std::uint32_t count_ecx);
} // namespace bsp
