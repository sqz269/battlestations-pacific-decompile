# Raw Group storage lifetime, CC10

The new source covers four complete normal bodies (211 bytes) on genuine `18Ch`
Group pool storage. It composes the already reconstructed raw node lifetime,
parenting, scene/resource ownership and pool providers. It does not install these
paths into the legacy logical `NativeGroupOwner` graph.

| Entry | Exclusive end | Bytes | Contract |
| --- | --- | ---: | --- |
| B8F680 | B8F6E6 | 102 | stamp Group profile, destroy borrowed array backing, raw node destruction |
| B8F8C0 | B8F8E0 | 32 | destroy, then current low-byte flags bit0 controls same-pool return |
| B8EEC0 | B8EF05 | 69 | clear current borrowed backs' A0, then tail to raw B6F310 |
| B8E6B0 | B8E6B8 | 8 | only AND actual138 with FFFFFFCF |

Live Ghidra and installed PE bytes agree for every body, the direct pool and
constructor dependencies, array equivalence ranges, profile, EH map and compiler
boundaries. The worker made no Ghidra changes. CC2AF0 is the eight-byte state0
cleanup tail to B6F440. CC2AF8 is an undefined ten-byte raw handler ending at
CC2B02; its final five-byte jump starts CC2AFD and reaches BF6B43. The handler
loads descriptor DFC1C0. Map DFC1B8 contains `{-1, CC2AF0}`; the descriptor has
magic 19930522 and one unwind state. Primary owns any later definition/annotation.

## Actual storage and source lifetime admission

Reuse the existing raw-name `construct_native_group_00b8f5e0` overload in
`gui_page_root.hpp`: its complete 106-byte body calls raw B6F5A0 on the same slot,
reads the current one constant after that call, stamps D634F8, initializes the
native tail and later reads current CE4970. It preserves opaque bytes and the
pool index at188. The constructor establishes the actual node prefix, including
atomic04, and a trivial actual20B `NativeGroupTailStorage` at174.

The separate host companion first checks provenance and duplicate admission,
the same owner registry/import domain, current native profile/slots and actual
positive count. Pool proof uses current188, the supplied real010902F4 pool's
slab table, exact18Ch slot modulus/range, 32 slots, the 3180 free-index stack and
31C0 free count in each31C4 slab. Returned or wrong-pool slots are rejected before
atomic/registry binding and before native writes.

Registry insertion occurs before descriptor preparation, so insertion failure
leaves the old typed tail lifetime intact. Successful admission ends that trivial
tail view, establishes a live12B `SystemAmbientBacklinks` at178, and restores all
12 preimage bytes. No field is changed or retained. The obsolete typed tail view
must never be accessed afterward. Its pointer element type is used only as
opaque pointer representation by the shared array helpers; no host SceneResource
or CameraTransform is fabricated or dereferenced.

Pure pool checks require valid accessible backing and quiescent ownership; they
do not synchronize concurrent mutation. Explicit raw scalar callers provide
genuine same-pool provenance themselves. Canonical admission and source lifetime
preparation are separate host operations, not invented calls in the native body.

## Native order and provider boundaries

B8F680 stamps D634F8 before state0, resizes the actual178 array to zero, reads
CURRENT begin after resize and frees that backing. Native begin and capacity
remain stale. State0 is consumed to -1 at B8F6C8 before direct raw B6F440.
Direct destruction leaves borrowed entries' A0 untouched.

The implementation factors that inline array schedule through the genuine
pointer-only B7C1C0 provider. Pinned full normalized equivalence proves
59FCE0/B7BC70 (80B), their 59E5E0/B7B390 reserves (95B), and
5A1610/B7C1C0 cleanup (23B). Only documented rel32 operand bytes differ; both
reserve bodies call the same allocator/free entries. The valid nonnegative
count0 path cannot grow. This reuse does not invoke the point-specific allocation
registry or imply a native B7C1C0 call in Group. Native call rows retain 59FCE0
and BF6989 separately.

B8F8C0 adds no profile stamp or count decrement. After destruction it reads the
CURRENT flags low byte, then conditionally calls real B8ED40 on the captured
identity. Pool return uses current188 after entering the genuine pool critical
section. There are no payload/count reads after slot return. Flags0 destroys the
payload while leaving slot disposition to the caller, even at positive count.

B8EEC0 first tests current17C. Each iteration captures current count, current
begin and the back pointer before the conditional count decrement; it clears
that captured node's A0 and rereads current count. Only afterward does it tail
to B6F310. Therefore repeated release still clears newly present borrowed entries
before B6F310's late44 gate. The tail uses actual current child18 and current0
bindings and may retire this Group; only persistent diagnostics are used afterward.
B8E6B0 performs only the eight-byte field mask; it does not invoke an enclosing
owner callback through A0.

The D634F8 profile has current0 BD30E0, current4 B8F8C0, current18 B8EEC0,
current40 B8E6B0, and node scene50/54 B6ED80/B6EE10. Canonical zero release checks
SAME actual+4 equals zero and current0, then BD30E0 obtains a fresh current4 and
invokes flags1. The companion supplies no count, credit, unknown-profile fallback
or terminal shortcut. Reached children/other families require explicit genuine
providers and persistent recursive frames.

## Failure and retirement

Source exception handling projects the native state0 node cleanup and consumes
the state before cleanup. A cleanup exception terminates; a failure after the
state becomes -1 does not repeat node destruction. This is source C++ transport,
not execution or equivalence proof of native FH3/SEH. Caller-owned frames and
acquisitions retain failure progress, backing and unresolved credits.

Every admitted destruction route uses its canonical reference. Explicit scalar
deletion retires the registry binding on return or source failure, including
flags0. Registry unbinding is map-only and never reads dead native storage.
Retirement on failure proves neither payload cleanup nor pool return. The
noexcept canonical zero boundary terminates on an escaping source failure;
quiescence excludes callback reentry and slot reuse until retirement completes.

## Verification

`scripts/build.ps1` passed strict MSVC Win32 and both configured existing CTests.
One ignored focused executable passed with assertions active (`NDEBUG` is rejected
at compile time). Its exact build command is retained in
`local/output/cc10_group_storage_probe.cmd`: `/MD /EHsc /std:c++20 /O2 /Gy /W4
/WX /fp:strict`, existing libraries, and `/link /OPT:REF /MANIFEST:EMBED`.

The probe compares copied original bodies with source using real18Ch pool slots,
the genuine raw106B constructor/name pool, actual3Ch scene resources/canonical
counts, and raw parenting/scene/tree/node providers. It checks the normalized
complete396B flags0 result, a genuine decrement import changing the current
stack flags from0 to1 during destruction, same-pool return, duplicate borrowed
backs, A0 clearing before real child18 calls, repeat-release44 behavior, and the
exact world40 mask. It also checks unchanged bytes on wrong-pool/duplicate
admission and canonical explicit positive-count flags0 retirement. Child fixture
counts stay nonzero; the actual Group and resource zero terminals are exercised.

211B denotes full live/PE body extents. The copied-original comparison relocates
six rel32 operands (24 bytes): B8F6B6/B8F6BE/B8F6D0 to genuine resize/free/node
bridges, B8F8C3 to copied destruction, B8F8D5 to the real pool, and B8EF00 to raw
B6F310. All remaining body bytes are preserved; world40 is copied unchanged.
The external constructor, array helpers, nested lifecycle/dispatch providers and
canonical bridge are shared source boundaries, not independently compared native
callees. Native exceptional execution, private stack parity, arbitrary derived
targets, binary replacement, application installation and game behavior remain
unclaimed. No tracked tests or shared provider edits were added.
