#pragma once

#include "bsp/singleton_lifetime.hpp"

#include <cstdint>

namespace bsp {

struct NativeDiagnosticSinkStorage {
    std::uint32_t native_vtable_00;
};
static_assert(sizeof(NativeDiagnosticSinkStorage) == 4);

// 004C14C0: cdecl, no native inputs, EAX current publication, RET. Both
// references identify the application's actual publication and shared domain.
// The domain's destroy_registered callback dispatches this owner's deleter.
NativeDiagnosticSinkStorage* native_diagnostic_sink_get_or_create_004c14c0(
    NativeDiagnosticSinkStorage* volatile& actual_published_0109cf14,
    SingletonLifetimeDomain& actual_lifetime);

// 004BBCA0: ECX four-byte owner, stack flags, EAX original address, RET 4.
// Clears publication and installs the base profile; flag bit 0 also frees.
NativeDiagnosticSinkStorage* delete_native_diagnostic_sink_004bbca0(
    NativeDiagnosticSinkStorage& owner, std::uint32_t flags,
    NativeDiagnosticSinkStorage* volatile& actual_published_0109cf14) noexcept;

// 00411EE0: ECX actual eight-byte guard, RET. +04 is the actual native
// CRITICAL_SECTION pointer, whose tracked counter is physically at +18h;
// it is not a pointer to SystemSingletonCriticalSection's C++ projection.
// Captures +04 before writing the guard profile, decrements then leaves,
// and preserves +04. The caller owns entry/state arming and raw storage.
void destroy_native_singleton_guard_00411ee0(void* actual_8byte_guard);

} // namespace bsp
