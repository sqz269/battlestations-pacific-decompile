#pragma once
#include <cstdint>

namespace bsp {
struct NativeGuiIconStorageContext;
// Whole AB57E0..AB5802 forward range schedule (35B/15 rows). Native ECX is
// unused; two incoming DWORD homes hold first/end, RET8, no defined result.
// This ordinary Source API borrows two real addressable DWORD objects and the
// SAME genuine raw-state context. Capture home0 then home1 once, before any
// state release. Callback changes to either home do not replace those captures.
//
// Compare address DWORDs; while unequal call EXISTING whole raw AB3280 at the
// captured cursor, then advance it by40h with DWORD wrap. There is no nullskip,
// count/domain cap, late end reread, typed T[n] arithmetic/owner, Frame, Acquired
// or callback adapter. Equal first/end, including0/0, does not release a state.
// Caller proves all reached actual40B states/subobjects/backing and the genuine
// registry/texture/raw-string/pool/context domain of that existing destructor.
// No arbitrary raw address is converted into a typed state owner/reference.
//
// The callee's existing Source policy is inherited exactly: release CURRENT
// texture, clear it only after normal release, release CURRENT name; its own
// texture-escape arm releases name and terminates on a secondary failure. This
// wrapper adds no catch, cleanup, replay, rollback/retry, noexcept, terminal or
// Native-origin policy and is not an Original FH3/SEH/fault/NoThrow certificate.
// An entered escape requires caller retention of the genuine current/remaining
// graph, input homes, backing and providers; normal iteration is no disposition
// or Native cleanup authority. Retain all lifetimes through callbacks/escapes.
// Source control/spill storage is separate from provider-write extents; input
// home/record aliases are only those supported by actual C++ object lifetimes.
// Original registers/private-stack/FLAGS/fault ABI and production/custody/
// activation remain unproved. Keep the existing high STL name/tag unchanged;
// add zero Original/vendor/helper/CG/duplicate-body credit.
void destroy_native_gui_icon_state_range_storage_00ab57e0(
    const volatile std::uint32_t (&actual_argument_homes)[2],
    NativeGuiIconStorageContext& context);
} // namespace bsp
