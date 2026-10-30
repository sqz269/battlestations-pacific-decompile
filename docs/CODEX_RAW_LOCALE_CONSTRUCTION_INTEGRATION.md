# Dormant raw locale construction and map clear

This batch registers three qualified library entries in bsp_core: the
`00736540` publication base, full `0073C960` constructor, and `0073C240` map clear.
Canonical ownership, loading and application startup remain unbound.

The constructor borrows actual manager/locale publication cells. It retains
the first manager's section, publishes the original receiver, performs a second
manager lookup, then reloads the current locale argument for registration.
Saved state 0/1 cleanup resets the original receiver and destroys the actual
guard in the proven order. Normal Leave remains armed. Failure does not clear
publication, unregister, free the receiver or roll back registration. Derived
writes follow base success and preserve allocator words at +4/+4020/+4030.

Held `37ce` assumed live C++ field objects over the intended raw storage. Final
`d5c116833` replaces only those accesses with fenced byte copies. Primary and
independent Frame review confirmed the raw-storage gap is closed; inverse
primitive substitution recovers the exact held CPP, including controls, EH,
capture epochs and 14 receiver stores. Both object artifacts remain preserved.
The old field-only API is unchanged.

Dependency review also found typed field accesses in the shared 411EE0 guard
cleanup and 412430 profile reset. Fix `f411f273` changes only those functions to
fenced raw byte copies, retaining capture/decrement/Leave and profile-store
order. Their APIs, exception specifications, shared access helpers and all
other functions are unchanged by whole-file inverse projection. This closes
the identified cleanup access gap, not arbitrary transitive provider proof.

Map clear `550488c70` composes the real `737CE0` payload and `736090` slot-return
services. It reads each current bucket head, saves next before cleanup, then
returns the node slot. Only whole normal completion clears 4000h bucket bytes
and the separate count. Escapes retain partial history; no retry or later drain
is inferred. Factoring the native cached-Enter sequence requires stable imports.
The actual initialized E1768C owner remains a caller prerequisite.

Strict worker object checks passed. The combined current-main MSVC Win32
Release build including the shared cleanup fix passed both existing checks in
19.29s. The earlier 13.12s build is preserved as pre-fix evidence. No tests/probes were added
and no new body or game was executed for qualification. Current CRT/providers,
ordinary C++ exception behavior and raw-input lifetime/alias contracts remain
qualified; Original ABI/FH3/SEH/fault timing and production parity are unproved.

Canonical F8BC4C placement/profile dispatch, `7365E0`/full `73C9F0`/scalar retirement,
loaded maps/sidecars, node-pool producer and real startup consumers remain open.
The existing pool lifecycle bodies and shared allocator list are reuse
candidates, not a constructed owner receipt. Registration alone proves no
loaded locale. All nine raw Text choices remain FALSE and frozen ac60 remains
unchanged on the Font48/Viper19 Source-admission hold. No runtime, whole caller,
full13/USN21/Page/P4 or vendor/new-native-body credit follows.

Exact Source/build/peer evidence: reports/codex_raw_locale_construction_integration.json.
