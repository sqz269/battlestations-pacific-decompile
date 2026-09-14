# Canonical mutable CRT data owner DD

The rebuilt Win32 game's controlled child now owns actual storage at the four
recovered mutable CRT cell sites. Its one joint transaction retains the existing
read-only D6 exception pair as well. This is a bounded data owner and one real
cookie initialization, not a complete CRT or an original-entry provider.

The input remains the exact original `battlestationspacific.exe` (12,223,752
bytes, SHA-256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`).
The accepted CY proposal and independent DA review are frozen under the root
orchestrator's `local/native-crt-mutable-owner-cy-worker-delivery` and
`local/native-crt-mutable-owner-review-da-worker-delivery` bundles. DD does not
change the installed image or the saved Ghidra project.

## Exact committed ownership

| Reservation | Committed page | Source and protection | Advertised cells |
|---|---|---|---|
| E10000[10000h] | E15000[1000h] | full PE bytes A15000, RW/non-execute | E15590 cookie and E15594 complement |
| E10000[10000h] | E16000[1000h] | full PE bytes A16000, RW/non-execute | E16830[10h] NLG descriptor |
| 1090000[10000h] | 109E000[1000h] | explicit loader zero-fill, RW/non-execute | 109E568[324h] failure block; 109EEA4 feature; 109EEA8 hook |
| existing D60000[10000h] | D6E000, within RO mapping | PE `.rdata`, read-only/non-execute | D6E1CC pair `{109E568,109E5C0}` |

The failure block contains a 50h record, four-byte debugger word, four bytes
of untouched padding and a 2CCh context. The other copied page bytes are not
promoted to initialized TLS, heap, locks, feature policy or callbacks. The
source maps no original code and does not resolve original imports. A whole
`.data` reservation would conflict at E08000 with the RO E00000 band; only the
two exact extra 64-KB allocation bases are requested.

The v1 read-only API and ordinary mapper remain available. The game uses an
explicit v2 handoff record: 19 original RO slots followed by slot 19=E10000
and slot 20=1090000. Its six-band mask is `18018Ah`; the parent and child
validate the version, exact sparse address set, ordered page/role plan
`{E15000 initialized, E16000 initialized,109E000 zero}`, inherited handle,
identity, one-time state and exact uncommitted MEM_RESERVE allocations. The
restricted handle list, timeout, collision rejection and link placement remain
as before (`/BASE:0x10000000 /DYNAMICBASE:NO /FIXED:NO /MAP`).

After the child claims the capability, the coordinator constructs both RO and
mutable owner objects. Each transfers its disjoint allocation bases before
either commits a page. A failure before publication destroys these owners and
releases only their known allocation bases; the capability releases only its
untransferred exact reservations. Committed pages split VirtualQuery regions,
so an established owner's destructor uses its retained allocation base for
MEM_RELEASE rather than the pre-transfer exact-MEM_RESERVE predicate.

The RO mapper verifies the full input SHA-256 and protects its selected pages.
The mutable owner independently verifies the full input SHA-256, compares both
complete initialized page copies, explicitly checks zero-fill and checks all
three page protections. The coordinator verifies the actual D6 pair points to
the shared failure block and context. It then calls the existing complete
`initialize_native_crt_security_cookie_00c1815e` with references to the actual
E15590/E15594 words, once, before any native-frame consumer is exposed. The
parent ACK is a checked CAS. If the parent cancelled it, the owners roll back
and startup fails. All heap ownership metadata exists before that ACK; success
permanently publishes one non-destructed process owner. Both RO and RW bands
therefore remain valid through host destruction, atexit and thread teardown,
with OS reclamation at process exit. Duplicate initialization is rejected.

The owner exposes typed borrowed references to the scoped cells and a borrowed
RO mapper. It does not claim native C0DC54/BFE120 checker/report, NLG31,
C0DBC4/C0DCCD unwind, Watson, TLS/PTD, heap, native registrations or gameplay
readiness. The existing context-bearing CT source ABI remains separate.

## Validation and limits

The settled-input strict MSVC 19.51 Win32 build passed with `/W4 /WX
/fp:strict`; all eight export seed byte comparisons and both existing CTests
(`reconstructed_math`, `native_math_differential`) passed. A single local
controlled-child probe under `local/native_crt_canonical_owner_dd` links the
same final `bsp_core.lib` objects as the rebuilt game. It checks bad-image
rollback and an injected claimed-state cancelled ACK with all six bands
MEM_FREE before the caller's moved-from capability is destroyed; the parent
requires the exact failed-state diagnostic after each child exits. A separate
short parent timeout exercises the pre-claim cancellation path. Success checks
the three pages, cookie complement, D6 relationship and an owner access from
one host atexit callback. Game and probe MAP files identify the same archive
members for the coordinator, bootstrap and actual cookie initializer. Both
executables retain Win32 base10000000h and relocation data.

The probe does not execute original game code, failure/Watson paths or cleanup
handlers. The atexit callback is one observed teardown point, not proof of all
thread or CRT teardown. Its success establishes availability in that controlled
process only; other loader environments may collide and must fail safely.
Build/fixture proof is not gameplay validation.
