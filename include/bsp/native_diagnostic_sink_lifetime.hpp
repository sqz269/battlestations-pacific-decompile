#pragma once

#include "bsp/sound_lifetime_access.hpp"

#include <cstdint>

namespace bsp {

struct NativeDiagnosticSinkStorage {
    std::uint32_t native_vtable_00;
};
static_assert(sizeof(NativeDiagnosticSinkStorage) == 4);

// 004C14C0: cdecl, no native inputs, EAX current publication, RET. Both
// references identify the application's actual publication and shared domain.
// Borrows the caller's semantic domain or actual 01090AA0 publication cell;
// creates no alternate manager. The caller's shutdown dispatch owns deletion.
NativeDiagnosticSinkStorage* native_diagnostic_sink_get_or_create_004c14c0(
    NativeDiagnosticSinkStorage* volatile& actual_published_0109cf14,
    SoundLifetimeAccess actual_lifetime);

// 004BBCA0: ECX four-byte owner, stack flags, EAX original address, RET 4.
// Clears publication and installs the base profile; flag bit 0 also frees.
NativeDiagnosticSinkStorage* delete_native_diagnostic_sink_004bbca0(
    NativeDiagnosticSinkStorage& owner, std::uint32_t flags,
    NativeDiagnosticSinkStorage* volatile& actual_published_0109cf14) noexcept;

// 007363B0: cdecl, no native inputs, RET. Initial-null fast path; otherwise
// capture the first resolved manager's +10 section, enter/increment, recheck,
// resolve a second manager, unregister the then-current publication, reload it,
// invoke current slot zero with flags1, clear redundantly, and release the
// captured section. This typed source supports the established CE752C derived
// and CE3818 base profiles. An unsupported current profile throws only at the
// native indirect-dispatch boundary, after any completed unregister mutation.
void destroy_native_diagnostic_sink_007363b0(
    NativeDiagnosticSinkStorage* volatile& actual_published_0109cf14,
    SoundLifetimeAccess actual_lifetime);

// 00411EE0: ECX actual eight-byte guard, RET. +04 is the actual native
// CRITICAL_SECTION pointer, whose tracked counter is physically at +18h;
// it is not a pointer to SystemSingletonCriticalSection's C++ projection.
// Captures +04 before writing the guard profile, decrements then leaves,
// and preserves +04. The caller owns entry/state arming and raw storage.
void destroy_native_singleton_guard_00411ee0(void* actual_8byte_guard);

} // namespace bsp
