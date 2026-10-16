# Original CRT initializer lock cleanup

| Native routine | Span | Coverage |
| --- | --- | --- |
| `00C051AE` | `[00C051AE,00C051B7)` 9B/4ops | complete no-argument cleanup helper |

The entire native body is PUSH0Ch, CALL canonical unlock, POP ECX and RET.
It reads no incoming register/stack data and has no semantic EAX return.
Ordinary Source delegates to the unchanged real canonical unlock with
literal12. It adds no naked assembly, new
critical section, descriptor, global, owner, callback, validation or recovery.

The actual canonical unlock is already complete21B Source. Its one raw cdecl
index selects the DWORD at fixed E16478+8*index, then invokes actual
LeaveCriticalSection. Index12 selects E164D8. The caller must supply the real
readable descriptor and its initialized Win32 critical section, currently
owned by the calling thread. Addressability alone does not initialize or own
that lock. The helper does not acquire, lazily create or replace it.

The larger C050F8 routine operates on the actual PTD and optional locale.
Its inspected body pushes12 before its lock call, then calls this cleanup
after publishing state=-2. This establishes the cleanup role; it does not
make PTD construction, lock acquisition, allocation or parent SEH ready.
The role name is descriptive, not a recovered library/compiler symbol;
the actual canonical library name `__unlock` and its provider remain intact.

| Containing routine | Native listing | Contract |
| --- | --- | --- |
| `00C050F8` | `00C051A3 CALL 00C051AE` | no pushed helper arguments; state=-2 precedes call; private parent SEH transport unproved |
| `00C051AE` | `00C051B0 CALL 00C11B31` | literal12; caller POP ECX cleans4; helper plainRET |

Ordinary Source supplies a new no-argument C++ interface. It does not reproduce
the incidental ECX value from native POP, native instruction placement,
private frames, exact register/fault/SEH continuation or binary-entry identity.
It adds no exception translation, catch, rollback or fabricated success value.

The read-only readiness report
`reports/goal_oct15_crt_locale_provider_readiness.json` records the larger
Original chain. Nullable getter C051B7 still needs genuine allocation,
encoded getter/setter and PTD initialization transport; C0522E also needs its
actual fatal path. LocaleUpdate still needs actual TLS/locale/multibyte owners.
No current SDK LocaleUpdate, synthetic PTD or parent fastpath is supplied.
General locale/TLS/errno/parent readiness stays false.

All9 native bytes match live Ghidra and the installed PE; the real21B canonical
callee was read in full. Source/provider hashes, prior names/comments and both
typed transfers are in the report. Primary independent review passed the complete
helper and unchanged real provider. Locked annotation preserved the old name
and comment, saved the project and refreshed the export. The CMake Source
append was made on the first unheld queue turn and the queue was released
before build. Normal `scripts/build.ps1` at `327b2ecfb` passed MSVC Win32
Release and both enabled existing checks (math and tool tests; 10.07 seconds).
Native differential tests were not enabled in this worker build. No tests
were added. The full emitted helper has 9 bytes and one REL32 relocation to
the unchanged canonical unlock; its body equals the native body after that
call displacement is normalized. Its public archive symbol occurs once.
These checks do not establish binary-entry/private-ABI or runtime/game proof.
