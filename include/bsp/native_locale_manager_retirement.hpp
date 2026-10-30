#pragma once

#include "bsp/native_locale_manager_constructor.hpp"

#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;

// Immutable borrowed bindings; this constructs no owner, publication or pool.
// The raw string context's actual AA0 cell MUST be the same cell borrowed by
// publication. Pool must be the genuine initialized stable E1768C authority.
struct NativeLocaleManagerRetirementContext {
    const NativeLocaleManagerConstructorContext& publication;
    NativeStringRawPoolContext& strings;
    void* const actual_initialized_locale_pool_00e1768c;
};

// Complete game-owned 73C9F0[251] schedule and selected state2/map, state1/names,
// state0/base cleanup. Native ECX receiver, plain RET, no meaningful result.
// Use actual aligned/addressable4040h receiver bytes, SAME construction cell
// identities, raw strings and initialized pool. No semantic container casts.
void destroy_native_locale_manager_0073c9f0(
    void* actual_receiver, const NativeLocaleManagerRetirementContext&);

// Complete73CAF0[30]: capture receiver, destroy, THEN read actual flags' low
// byte; bit0 frees captured receiver through matched actual BF65AC contract.
// Return address bits only, with no post-free receiver read. Keep the genuine
// caller-owned DWORD object alive throughout destruction; never copy it early.
void* delete_native_locale_manager_0073caf0(void* actual_receiver,
    const volatile std::uint32_t& actual_flags,
    const NativeLocaleManagerRetirementContext&);

// Caller proves allocation provenance, disjoint receiver/context/cells/flags/
// providers and nonoverlapping genuine sidecar/string/map ownership, valid raw
// range endpoints, and actual sections/slots/pools alive across all escapes.
// No conflicting writers, retargeting or invalidating/reentrant lifecycle.
// Partial effects are retained; no rollback, retry, recovery or receiver free
// is invented by the destructor. Current names begin is reloaded AFTER range;
// language data/size are captured BEFORE getter and its header is not cleared.
//
// Require actual aa56 base retirement, f411 raw411EE0/412430,550 map clear and
// existing raw432050/4324A0/419CC0/BD1510 services. Own raw DWORD access uses
// fenced byte copies; typed local progress and real context/flags are objects.
// Secondary escaping MSVC C++ cleanup fails-stop. Native FH3/SEH/hardware/
// private-spill/throw identity, binary ABI, canonical lifetime/profile/host,
// startup/loading/uppercase/Text and source-ready activation are not supplied.

} // namespace bsp
