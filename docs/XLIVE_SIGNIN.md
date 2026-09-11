# XLive cached local-user refresh and debounce

Addresses: 00a3ebd0, 00a3f3e0, 00a3f440, 00a3e600, 00a3e6a0.

The five complete normal functions are reconstructed in `src/xlive_signin.cpp`.
They reuse `OnlineSystemState` and `PlatformManagerFlags`, bind the pump's actual
+120/+3BC booleans, and add the remaining cached-name/user/debounce storage through
`XLiveSigninState`. The caller supplies scalar preimages; this packet supplies no
manager constructor or replacement SDK results. Required SDK, clock and callback
operations are explicit `XLiveSigninHost` methods with no default implementations.

## ABI and native storage

| Entry | Native ABI | Last instruction / inclusive end |
| --- | --- | --- |
| 00a3ebd0 | ECX=manager; forced byte stack; RET4 | 00a3ed04 RET4, 3 bytes / 00a3ed06 |
| 00a3f3e0 | ECX=manager; no stack args; RET | 00a3f436 RET, 1 byte / 00a3f436 |
| 00a3f440 | ECX=manager; no stack args; RET | 00a3f494 RET, 1 byte / 00a3f494 |
| 00a3e600 | ECX=manager; mask byte stack; RET4 | 00a3e687 RET4, 3 bytes / 00a3e689 |
| 00a3e6a0 | ECX=manager; no stack args; RET | 00a3e6f4 RET, 1 byte / 00a3e6f4 |

The saved timestamp spans manager+8..17. Its frequency low DWORD at +10 is the
same storage already projected as `OnlineSystemState.field_10`; a separate
`ClockTimestamp` member would shadow that word. The new storage therefore keeps
ticks+8 and frequency-high+14, while timestamp accessors read/write frequency-low
through the canonical online field. Callback+18 follows that timestamp.

Manager+8C is the DWORD returned by `XUserGetSigninState(0)`. The next 128 bytes,
+90..10F, are the username, followed by XUID+110/+114 and privilege byte+118.
The pump's indexed DWORD read at `8Ch + index*4` therefore overlaps the username
for nonzero indices. `read_cached_manager_dword_8c_indexed` reads the same storage,
including 32-bit offset wrap; it does not invent a per-user array. The supported
range is the state DWORD and 32 username DWORDs. Other offsets are explicitly
rejected, as are any username words containing unknown bytes.

`XLiveUserName128` records byte values and a 128-bit defined mask. Backing zeros
for undefined bytes are implementation storage, not evidence about the original
stack or name padding. SDK adapters mark only bytes known to have been written.
Successful C-string output normally defines its prefix and terminator; padding
remains unknown unless the adapter has evidence it was written. An error in
00a3ebd0 overwrites only byte zero with zero and makes that byte known, preserving
any other reported SDK writes. The native commit is REP MOVSD count20h, so the
entire 128-byte value/mask is copied into cache, including its unknown tail.

## Refresh, name changes and callbacks

00a3ebd0 always queries sign-in state, name with capacity128, and XUID for user0,
in that order. Name failure writes only local byte0=0; XUID failure writes both
DWORDs zero. Only then may a nonforced refresh with state0 return without updating
cache. For state2, it queries privilege FEh into an initially-zero BOOL. Query
failure resets that BOOL to zero. State2 also clears canonical link_failure+128
when the current field3B8 is either 0 or 1.

When field3B8 is zero, the function compares old/new sign-in *presence* first.
If presence differs, or their case-sensitive C strings differ, it invokes the
recovered 00a3e6a0 restart. Presence change short-circuits name comparison. Otherwise,
when field3B8 is nonzero, a nonnull callback20 is invoked only for nonzero new
sign-in state. This callback sees the precommit cache; afterwards the function
stores captured sign-in state, all128 name bytes, XUID and normalized privilege.
Unrelated callback mutations are retained. Callback exceptions prevent commit.

00a3e600 checks only mask bit0. On a successful user0 name query it compares the
cached/new names case-sensitively. A different name replaces all128 bytes before
testing current signin_state11C. Only state0 and nonnull callback18 invoke the
callback. Name-query failure leaves cache unchanged. In both callback sites
(A3ECBD and A3E67B) the native sets ECX=0 and supplies no stack arguments. The
required `invoke_state_callback(const void*)` boundary matches the existing
startup callback interface; a concrete adapter must preserve this register input.

00a3e6a0 clears +3BC only if both +120 and +3BC are true. It then clears online
+119/+11A, writes state11C=1, clears storage_removed+2F and the new +2D/+2E bytes,
clears +120, writes slot124=-1 and field3B8=1, sets profile_changed+2C, and clears
invite_accepted+31. The two pump booleans are bound references, not duplicate state.

## Debounce and exception ordering

00a3f440 toggles the debounce state. If already pending, forced refresh runs
before clearing the pending flag. Otherwise nonforced refresh runs, pending is
set, the current clock virtual+20 is sampled, and all16 timestamp bytes are stored.
Thus a sampling exception leaves pending set; a refresh exception prevents the
later flag change. No clock sample occurs in the already-pending branch.

00a3f3e0 does nothing when not pending. Otherwise it samples the current clock,
then subtracts the live saved timestamp through the existing 00530890 operation.
Assembly passes the saved pointer across sampling, so reading its value before
the callback would be wrong. FILD/FILD/FDIVP converts the signed timestamp ratio,
FSTP spills it to binary32, and FLD1/FLD/FCOMIP/JBE implements strict ordered >1.
The reconstruction reuses `timestamp_seconds_x87` and reproduces the x87 compare;
exactly one second and unordered results do not trigger. On expiry, pending is
cleared *before* forced refresh. This differs deliberately from the toggle path's
clear-after order, including when SDK/callback operations throw.

## Verification and remaining boundaries

All five existing Ghidra function bodies cover the terminal instructions above.
Read-only flow inspection found no gaps and no new function starts are needed.
The configured target was verified as `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Descriptive names are hypotheses; saved annotations
and tag changes remain the parent's responsibility.

MSVC Win32 Release build and both existing CTests passed; eight native seed ranges
matched. One ignored focused fixture checks query/commit ordering, failed-name
unknown padding, canonical restart flags, callback pre/postcommit visibility,
the actual username/index overlap, timestamp/field10 aliasing, the strict one-second
boundary, and the two exception-sensitive pending-flag orders. Commands/output:
`local/run_xlive_signin_fixture.cmd`, `local/xlive-signin-fixture.log` and
`local/xlive-signin-build.log`. It supplies explicit SDK/clock test services and
does not sign in, change profiles, or invoke live XLive callbacks.

These are reconstructed/build-tested/fixture-tested interfaces. Unknown name
bytes, an unterminated comparison escaping the 128-byte region, and unsupported
indexed offsets fail explicitly instead of using invented contents. Native
physical layout, arbitrary SEH/allocator behavior, live SDK integration and game
validation remain unclaimed. See `reports/xlive_signin.json` for exact scope.

## Integration from docs/PLATFORM_SERVICES.md

The parent now composes these routines through PlatformServices and XLiveManagerRuntime, with original-ordinal SDK forwarding and shared canonical state. Input lookup/reset/tick use one published pointer. The combined build and existing tests passed; installed XLive loader probes failed before pretranslation. Required owner construction, game dependencies and runtime validation remain explicit in docs/PLATFORM_SERVICES.md.
