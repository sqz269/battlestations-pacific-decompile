#pragma once
#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;
// Whole normal AB4AF0..AB4B15,38B/18ops, three DWORD stack arguments and plain
// RET; no defined result. This ordinary four-argument Source API changes that
// ABI and deliberately returns void, not the callee's incidental EAX.
//
// Capture HOME0 first, HOME1 end second. Equal captures return before reading
// HOME2, including0/0. Nonempty captures source identity HOME2 ONCE, then calls
// the EXISTING whole raw AB3C10 assignment at each current destination, SAME
// captured source pointer, ordinary preserve HOME, rawpool and LIVE feature ref.
// Source CONTENTS remain current through that real callee on every iteration;
// callback changes to incoming HOME2 never replace the captured identity. This
// differs from AB5400's per-iteration late source-HOME read. After each normal
// call, wrap destination DWORD+40h and compare the captured end. No count/cap,
// nullskip, constructor/birth/owner, T[n] arithmetic or callback adapter exists.
//
// Caller proves genuine already-constructed40B records/subobjects, source and
// all reached current/partial/completed/remaining graph/backing/provider/module
// lifetimes, including legal C++ HOME/record aliases. Private Source control/
// spill storage is disjoint from provider-write extents. The real assignment's
// header/string/texture/extent/DF0/CPU/pool/feature/exception contracts remain;
// the wrapper neither reads nor seeds the preserve HOME or feature value.
//
// UNKNOWN escapes propagate without cleanup/refund/retry/reset/rollback; retain
// the complete graph and outstanding callee debt even if incoming homes changed.
// No progress/cursor recovery, normal retirement, Native EH/Origin/disposition
// or NoThrow authority follows. No Frame/Acquired or new terminal policy. Actual
// callers/custody/activation and Original private-stack/register/FLAGS/fault/
// FH3/SEH/unmaskedFP/emitted code remain separate. Existing high STL name/tag
// stays; new Original/vendor/helper/CG/duplicate-body credit is ZERO.
void fill_native_gui_icon_state_range_storage_00ab4af0(
    const volatile std::uint32_t (&actual_argument_homes)[3],
    std::uint32_t& actual_assignment_preserve_home, NativeStringRawPoolContext& strings,
    const volatile std::uint32_t& actual_feature_word_0109eea4);
} // namespace bsp
