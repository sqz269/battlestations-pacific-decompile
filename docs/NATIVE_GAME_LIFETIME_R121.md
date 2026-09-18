# Native game destruction schedule (R121)

Addresses: `004DCF90`, `004DE270`, `004D27C0`, `004CC760`.

## Scope

The four normal bodies are reconstructed in `native_game_lifetime.cpp`.
The parent schedule requires explicit callee bindings; its dependency closure
and ordinary application admission remain open. No successful empty cleanup
defaults are provided. This packet does not connect the raw game owner to the
application or claim native exception handling, binary ABI, or gameplay parity.

| Entry | Inclusive end | Bytes | Coverage | Original ABI |
|---|---|---:|---|---|
| 004DCF90 | 004DD5A6 | 1559 | Complete normal parent schedule; required callees remain | ECX=game, RET |
| 004DE270 | 004DE28D | 30 | Complete scalar wrapper | ECX=game, stack flags, EAX=original game, RET4 |
| 004D27C0 | 004D280D | 78 | Complete nested-storage cleanup | ECX=publication cell, RET |
| 004CC760 | 004CC79D | 62 | Complete reference-pointer range release | ECX=first, EDX=end, two unused stack arguments, RET8 |

The existing `NativeGameStorage` provides the actual 71A0 allocation extent.
No second game layout is introduced. The allocation producer at73E150 and
constructor4DDB90 establish that owner; its vptrCE7CB8 names scalar4DE270.
`CG_vector_deleting_dtor_004dcf90` was a heuristic mislabel:4DCF90 has no
array-count or scalar-delete flag argument.

## Ordering and ownership

The destructor stampsCE7CB8 and sets the shared small-string-return disable
cell1090AA4 before its first cleanup. Globals are read when reached; nonnull
virtual scalar calls use their current vtable and flags1. The corresponding
publication is cleared AFTER the callback, including callback replacement.
TheE19900 object is captured before6B9380 and that same pointer is freed even
if the callback changes the publication. The game publicationE188A8 is cleared
after capturing the first gridE19B0C, before its scalar callback. The following
two grids are reloaded when reached.

The reference atgame+21F4 is decremented and terminally released when zero,
but the parent does not clear that field. In contrast, five members19E4 down
to19D4 are cleared after their release callbacks. Each terminal dispatch reads
the captured object's CURRENT vtable after the atomic decrement.

The718C checked-vector header captures begin/end for4CC760, then frees the
CURRENT begin field after range release and clears7190/7194/7198.4CC760 walks
ascending4-byte cells to the captured end, skips nulls, uses a real atomic
decrement atobject+4, calls current vslot0 only on zero, then clears the cell.
Its two stack arguments are unused; the originalRET8 is preserved as ABI
evidence, not extra source inputs.

4D27C0 captures the object in its publication cell. It conditionally frees
current+14, clears14/18/1C, conditionally frees current+4, clears4/8/C, frees
the captured object, then clears the CURRENT publication cell. It never
switches to a replacement object published during a callback.

Three string cleanups capture pointer and unsigned length+1 before the pool
getter; getter/return receive those same arguments even when callbacks change
the header. The header is not cleared. Five checked-tree erasures capture
their first/head iterators, then free their CURRENT head after the erase call.
Private iterator outputs and CRT reverse array iteration remain contracts.
The array tuples are7134/C/4/4C4600,1008/118/8/4CB2F0,
748/118/8/4CB2F0,560/10/5/4CAFD0. These are observed offsets and destructor
addresses, not inferred high-level container types.

The scalar wrapper tests only low-byte bit0, optionally callsBF6989, and
returns the original owner pointer after the call.

## Required host contracts

All methods named `call_<address>` identify the reached native boundary.
Their semantic contract is **unread in this packet**, except the two helper
bodies implemented here. The report carries every direct call site and its
containing body; indirect sites are listed separately. The signatures preserve
the explicit receiver/stack setup used by this parent, not a general recovered
callee ABI. No claim is made about implicit register arguments in those
unreconstructed bodies. Current offsets and virtual slots come from the
parent listing; no semantic callee names are inferred from its arguments.

| Boundary | Supplied values / required behavior |
|---|---|
| Address-named cleanup/service bodies | Actual ECX receiver where prepared; contract unread |
| 004CB220 | Actual7178 header and zero stack argument; contract unread |
| 004D1A50/004D22F0/004D2000/004D41A0/004CEF40 | Actual tree, private8h output, first-owner/position, last-owner/position; checked-STL dependency |
| 00419CC0 then00BD1510 | Captured block,size,factor1; getter result is next receiver; existing raw-string implementation can supply this binding |
| 00BF7C6E | Actual base,stride,count,original destructor identifier; existing CRT semantics required |
| Virtual scalar / terminal | Captured owner,current vtable slot; scalar flags1, terminal no stack argument |
| InterlockedDecrement | Concrete Win32 default; decrement captured object's+4 |
| BF65AC/BF6989 | Concrete CRT defaults; every explicit native call including null frees is retained |

`NativeGameLifetimeOperation` is one-shot. A thrown source boundary retains
owner, context, reached call site and last native unwind-state value; it marks
the partial destruction failed and rejects replay. Diagnostic acknowledgement
changes only the bookkeeping phase and requires the caller to resolve the
graph. It does not implement native FH3 cleanup or rollback. The source's C++
operation and host interface are not part of the original allocation layout.

## Listing repair and validation

The locked flow-repair tool restoredADD ESP,4 after4DE280, correcting an
undefined return in pseudocode. It also restored11bytes after4D27D5,4D27EE
and4D2800, recovering both conditional-free fallthroughs and the final
publication clear. Prior bytes/listings and repair receipts are retained.

* Live Ghidra and original PE agree on all1729 bytes across four bodies.
* Strict MSVC Win32 build and all three existing CTests pass.
* A local copied-original probe executes all four original bodies.87 callsite
  wrappers observe controlled external boundaries; internal branches and the
  nested calls among copied bodies are preserved.116 relocations include
  native calls and explicit global cells. Required callees are controlled
  observers/mutators; fixture frees retain backing memory. Both lanes use
  actual Win32 atomic decrements.
* 52 paired cases, 3537 boundary snapshots and 141402076 matching bytes cover
  each optional global alone, full/null paths, reference counts0/1/2,
  null nested buffers, wrapped string size, scalar flags0/1/2/3/100/FFFFFFFF,
  and callback replacements of publications, next references, vector storage,
  string headers and tree heads. Full owner/block/global bytes are compared
  before each call and at completion, with only the private iterator-output
  address represented by a common marker.
* Four source failure observations at6B9380, range terminal, string return
  and scalar free retain partial state and reject replay. These source-only
  checks do not establish native exception behavior.

The probe remains under ignored `local/game_lifetime_r121`; no permanent
test suite was added. This packet adds no reachable ordinary application path,
so no new gameplay or frontend-run result is attributed to these bodies.

## Follow-up packets

Bind the remaining real cleanup services, container/array destructors, Lua,
input configuration, resource manager and DYN disposal to the same actual
owners used by4DDB90. Recover their called bodies and any required register
ABI before giving the address methods semantic names. Then compose73D410
allocation/construction and application teardown with retained exception
ownership. Reuse the grid lifetime implementation fromR120. Only after that
closure should the ordinary app admit this raw71A0 game owner.

## Correction from docs/NATIVE_GAME_ARRAY_LIFETIME_R122.md

R122 supplies the four array-call defaults at 4DD230, 4DD3F5, 4DD412 and
4DD56B. They now invoke concrete vector, list and participant destruction with
the existing actual string-pool and observer services. The parent takes an
explicit array context and retains four child operations. Remaining cleanup
services, native exception behavior and ordinary application admission stay
open. See the R122 report for copied-original and parent-binding evidence.

## Correction from docs/NATIVE_GAME_EMBEDDED_LIFETIME_R123.md

R123 binds embedded-state disable/destruction at 4DCFDE/4DD268 to four complete
normal bodies with an explicit actual context and retained child operation.
The existing tracked-lock, Lua-state and input-configuration cleanup bodies now
supply the corresponding parent defaults. Parent and embedded copied-original
comparisons pass; virtual peer/payload services, other parent dependencies,
native exception behavior and ordinary raw-game admission remain open.

## Correction from docs/NATIVE_GAME_PROFILE_LIFETIME_R124.md

R124 binds profile destruction, race-record destruction and direct pool returns
to the actual profile/string context. The parent retains a profile child and its
nested mission operation; the context retains the string adapter used by that
child. Four normal bodies889bytes and the consumed full-range library branch
have copied-original composition evidence. General partial transient-tree erase,
remaining parent cleanup services, native exceptions and ordinary admission remain
open; see the R124 report for the exact fixture domain.

## Correction from docs/NATIVE_GAME_CONTAINER_LIFETIME_R125.md

R125 supplies nine complete normal bodies (878 bytes), binding seven previously
required parent methods at fourteen sites: `4BF930`, `4BF8E0`, `4C2CE0`, `4C4A50`,
`4CF3F0`, `4C4B40` and `4CB220`. They share the retained `operation.containers`
progress record. The source preserves current list-head/count/base reloads,
captured cells across virtual callbacks, reverse pointer-block frees and reference
retains before old-storage release. Progress owns nothing and performs no rollback.

Eight returning-call gaps (70 bytes) and the truncated `4C4B40` body were repaired;
its `4C8180` thunk survived. A 71-case comparison of the copied original bodies
against seven actual parent defaults matches 315 observations and 46,400 normalized
bytes using real CRT/Win32 atomics and actual raw sentinel producers. Two source
failure cases retain the expected allocation/cell graph. The separate controlled
parent comparator still passes 52 cases, 3,537 snapshots, 141,402,076 bytes and four
failure/replay cases, including shared progress identity. Strict build/three CTests
pass. Actual payload virtual methods, remaining parent dependencies, native
exception ABI, application admission and gameplay remain open.

## Correction from docs/NATIVE_AWARD_REGISTRY_LIFETIME_R126.md

R126 reconstructs six complete normal bodies (767 bytes) and consumes only the
current full-range branch of the 201-byte6B91F0 library entry. Parent4DD037 now
uses actual award-registry tree/record/list/string cleanup with the existing raw
string context and retained awards child. Parent4DD134 now performs the four game
lists' payload pass before node cleanup, retaining their sentinels for the later
array destructor. Parent diagnostic retirement refuses a running/failed award child.

Seven returning-call gaps139 bytes, two truncated functions and the removed489CA0
thunk were repaired/restored with evidence. The nested comparison passes79 cases,
989 observations and4,796,352 normalized bytes using real pool/manager/string-range
services. The parent still passes52 cases/3,537 snapshots/141,402,076 bytes and four
failure/replay cases. Strict Win32/three CTests pass. Native exception ABI, actual
registry construction/session, remaining dependencies and gameplay stay open.

## Correction from docs/NATIVE_GAME_TREE_LIFETIME_R127.md

R127 supplies all five parent tree-range defaults at4DD3C6/4DD461/4DD4D5/
4DD50A/4DD53F. The three newly reconstructed subtree bodies total188 bytes;
three current full-range adapters use them. The int-only adapter reuses existing
settings cleanup; the string-tree adapter delegates the existing complete raw
VFS range implementation. Every parent tree site passes its actual profile/string
context and retained operation.trees. Other non-tree dependencies remain open.

Three11-byte returning-free gaps were repaired. The64 paired cases match434
observations/472,808 bytes using all five actual defaults and real pool/node
services. Existing ranges are compared at entry/final state,their source internal
frees are not instrumented. The controlled parent still passes52 cases/3,537
snapshots/141,402,076 bytes and four failure/replay cases. Strict Win32/three
CTests pass. Fourteen context/partial-range guards and one partial-tree source
failure pass. Native exception ABI,application admission and gameplay remain open.

## Correction from docs/NATIVE_GAME_SINGLETON_LIFETIME_R128.md

R128 supplies three explicit singleton deletion defaults and the verified one-byte
8D88F0 no-op. Parent sites4DD0E1/4DD0E6/4DD0F0 pass the actual borrowed publication
context and three separate retained child operations. The488-byte deletion bodies
preserve adjusted-before-getter versus unadjusted-after-getter object reads,
actual manager unregister, current scalar receiver and captured exit lock.
Payload virtual bodies remain required. Source failure retains partial state
and entered lock and prohibits replay; parent retirement checks child status.

All489 live/PE bytes match.49 paired cases/324 observations/94,392 normalized
bytes pass with actual raw manager and Win32 locks. Three source failure/replay
and six context guards pass. The controlled parent retains52 cases/3,537
snapshots/141,402,076 bytes and four failure/replay cases. Strict Win32/three
CTests pass. Native FH3/SEH,remaining dependencies,raw-game admission and gameplay
remain open.
