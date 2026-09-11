# Native renderer stream frequency

The full `B24A40..B24AF7` body (183 bytes) uses the actual renderer and the
existing actual synchronization providers. The original interface is ECX
renderer with stack stream/value and RET8. The new C++ entry adds explicit
synchronization context and does not replace the original caller ABI.

Entry conditionally records the renderer and calls full `B33AD0`. It reads the
cache DWORD at `(stream+178h)*16+renderer` before arming cleanup (`B24A8E` before
`B24A90`). Equal values skip the COM call. Changed values publish the cache
before loading the current device at renderer+1A10 and its current table+198
`SetStreamSourceFreq` method. The raw stream/value are forwarded, including
wrapped index arithmetic. HRESULTs are ignored and no call counter exists.

Both returning branches read current mode before disarming cleanup, then use
the saved renderer and full `B33B00`. A thrown COM observer preserves the cache
and other earlier writes while invoking full `B21110` cleanup. The skipped
entry record remains uninitialized; no default renderer or taken flag repairs
it. Native `CBD038` selects `DF56C0`, with one unwind state and funclet `CBD030`
passing the guard at EBP-14 to `B21110`. The established actual-provider policy
terminates a second C++ exception during unwind and leaves other SEH handling
to the caller.

Complete source passes the strict Win32 build and both existing CTests.
Original-code fixture verification is complete as recorded below.
No drawing, original-caller ABI, full reset or gameplay validation is claimed.

## Primary integration

The primary independently verified 62 immutable worker pins, four current
root source files and ten fresh live-Ghidra/PE spans (396 bytes), then froze
the current strict-build main library
`3ca9a0274a0ce0fb9c7e0855bf75c1d8d2b70e70d005c7b3da1fcdcf6404600b`
and its two exact archive members. The unchanged fixture linked only this
main library. All thirteen original/library pairs match: 213,696 literal
DWORDs across 113 raw frames. Nine complete linked COFF sections, eight
source entries (653 unique code bytes), 27 full main image stages and both
secondary-exception processes passed independent postimage verification.

Two real HAL devices and actual Windows critical sections cover current
cache/device/table/lock/mode changes, the full native guard and FH3 cleanup,
unarmed entry and disarmed normal-leave throws. The full 7,204-byte renderer,
eight global bytes, both actual 28-byte critical sections, current pointers
and all 32 actual GetStreamSourceFreq values compare literally. Raw stream
10000000h was accepted by this runtime as wrapped stream zero. Frequency
zero produced actual INVALIDCALL 8876086Ch and was ignored after cache
publication. Both isolated secondary C++ cleanup exceptions completed real
Leave then reached the terminate handler, exiting 73 with the expected state.
Actual runtime provider module entries and native/source callsites are pinned;
FH3 import thunks remain distinguished from runtime exports.

Both existing CTests and eight fresh native seeds passed. The existing
Ghidra name and prior comments were preserved, evidence appended and saved,
the full reconstruction record replaced the earlier diagnostic boundary,
and the affected export refreshed. No permanent test was added. The new
C++ interface remains outside the original caller ABI, and these fixtures
do not validate the full renderer, game execution or visual behavior.
