# Native Lua variant link-repair Source candidates

The registered MSVC Win32 build passes all three existing checks. Primary review verifies 2 complete Core definitions covering 160 Original bytes; ABI, production startup and gameplay remain open.

The following candidate receipt is historical; registration and compiled review are recorded below.

`repair_native_lua_variant_links_006edc80` and
`repair_native_lua_variant_links_006edcd0` implement the reviewed instruction
schedules of the two raw link helpers as separate naked MSVC Win32 functions.
Each takes the actual receiver in ECX, an unused EDX placement word, and the
actual pivot as its first stack argument. Each exposes the surviving EAX
pointer as its Source result and retains all three `RET 4` arms.

The Source text maps one-for-one to the 30 accepted operations in each body,
including repeated reads and ESI restoration before the attachment branch.
This is an unregistered candidate. Compiler emission, exact emitted bytes,
Core membership, graph review and admission belong to the subsequent normal
registered build; none is preclaimed by the static mapping.

## Scope and accepted evidence

Packet `cc12_native_lua_variant_link_repair_helpers`, fixed published baseline
`c1548de013da2874a258c5a7de7c82e028aba45f`. Exactly four outputs are owned:

- `include/bsp/native_lua_variant_link_repair_helpers.hpp`
- `src/native_lua_variant_link_repair_helpers.cpp`
- This document.
- `reports/cc12_native_lua_variant_link_repair_helpers_source.json`

The accepted audit is
`reports/cc12_mission_lua_variant_link_repair_helpers_ABI_readiness.json`,
whose document and 84 input pins replay at their recorded revisions.
The two admitted Original spans remain separate:

| Original body | Bytes / operations | SHA-256 |
| --- | --- | --- |
| `006EDC80..006EDCCD` | 78 / 30 | `acd8854a9bf9c7b03fb8668432e70075be53ebf6a2f6b453f970bfeefd6f5bc5` |
| `006EDCD0..006EDD21` | 82 / 30 | `948bd60eeef4fbedf3d20796401d881873df045f2bdcb0935ea617572e96b856` |

Both exact spans were rechecked against the configured installed PE and
independently decoded: all 160 bytes and 60 operations match the accepted
evidence. No inter-body padding, Native caller, child, handler, profile,
table, string or neighboring body was opened. No Ghidra query, mutation or
prototype/flow/AddressSet repair was needed for this Source packet.

The caller's accepted 691-byte candidate remains a composition of 637 saved
bytes and a separate 54-byte continuation; 13 following INT3 bytes remain
outside it. This packet implements neither that caller nor its ownership,
storage producer, exception, allocation or whole repair-loop behavior.

## Source interface and physical contract

Both declarations have the same explicit placement:

```cpp
void* __fastcall repair_native_lua_variant_links_006edc80(
    void* actual_receiver,
    std::uint32_t unused_edx,
    void* actual_pivot);
```

The second entry uses the suffix `006edcd0`. Neither declaration or definition
has `noexcept`. The names describe a provisional role; they are not recovered
symbols, a Native C++ class, or a recovered semantic return API. The Source
`void*` result deliberately exposes the actual EAX surviving the reviewed
machine schedule. The unused EDX parameter positions the following pointer
on the stack; the body's first instruction replaces EDX with that stack word.

With S=entry ESP, R=entry ECX and X=`[S+4]`, both bodies perform these actions:

1. Capture X in EDX and the selected neighboring pointer Q in EAX.
2. Push incoming ESI at `S-4` only after those two loads.
3. Perform the ordered link reads/stores and root comparison.
4. Pop the current saved word into ESI **before** root/parent branch selection.
5. Perform the selected final stores and `RET 4`, leaving ESP=`S+8` on a
   valid normal return.

EDX retains captured X and EAX retains captured Q. ECX becomes the late-loaded
header H or nonroot parent P1. EBX/BL, EDI and EBP are untouched. ESI equals
its incoming value only if earlier stores leave its save slot intact. A raw
alias can also let PUSH ESI affect the first subsequent read through Q.
The pivot word is captured once and not reloaded after later argument-slot
changes. Both bodies contain no calls, callbacks, indirect branches or loops.

## Preserved ordered operations

Use `a(V)=[V+0]`, `p(V)=[V+4]`, `b(V)=[V+8]`, and
`s(V)=byte[V+31h]` solely as offset notation.

For `006EDC80`, Q initially comes from `b(X)`. Read `T0=a(Q)`, store
`b(X)=T0`, then **reload** `T1=a(Q)`. Test `s(T1)`; only zero writes
`p(T1)=X`. Next read current `P0=p(X)` and store `p(Q)=P0`. Only then
load current H from `[R+4]`, compare X with `[H+4]`, and restore ESI.

The equal root arm writes `[H+4]=Q`, then `a(Q)=X`, then `p(X)=Q`.
The nonroot arm reloads current `P1=p(X)` and compares X with `a(P1)`;
equality writes `a(P1)=Q`, otherwise it writes `b(P1)=Q` without verifying
the old value of that fallback link. Both arms then write `a(Q)=X` followed
by `p(X)=Q`. All three physical returns remain distinct.

For `006EDCD0`, Q initially comes from `a(X)`. Read `T0=b(Q)`, store
`a(X)=T0`, then **reload** `T1=b(Q)`. The conditional moved-parent write,
late P0 read, Q-parent store, late H read, root comparison and ESI restore
keep the same order as the first helper.

Its equal root arm writes `[H+4]=Q`, then `b(Q)=X`, then `p(X)=Q`.
Its nonroot arm reloads P1 and compares X with `b(P1)`; equality writes
`b(P1)=Q`, otherwise it writes `a(P1)=Q` without verifying the fallback
link. It then writes `b(Q)=X` followed by `p(X)=Q`. Its three returns also
remain distinct. No guard, field initialization, inferred invariant or
shared Source helper replaces any instruction.

Final root or matching-parent exits retain the last equality-CMP flags
(ZF=1, CF=0, OF=0); fallback exits retain ZF=0 and the other flags of the
actual DWORD subtraction. POP, MOV and RET preserve those arithmetic
results. The prior flag-byte comparison is superseded by the root CMP.
This is the intended raw schedule; Source emission and Original-to-Source
flag compatibility still require their separate reviews.

The header records the direct backing spans: R and H require the accessed
DWORD at `+4`; first-body X and second-body Q use up to 12 bytes, while
first-body Q and second-body X use up to eight. T1 has the byte at `+31h`
and conditional writable DWORD `+4`, giving a 50-byte highest-address span
rather than a proved object size. A nonroot parent's selected fields use
up to 12 bytes. No raw region is cast to a larger typed allocation.

All node/receiver/stack aliases remain meaningful. The repeated reads are
not merged; the header and nonroot parent are read after the earlier stores.
Final attachment, Q-backlink and pivot-parent stores keep their order.
The current return slot must remain usable, and the ESI save word must
survive until POP for incoming-ESI preservation. Completed stores remain
after a later fault. No null/alignment/membership/ownership/lifetime guard,
rollback, atomicity, allocator, default storage, producer, Native type,
profile/vtable or exception-runtime bridge is introduced.

## Current Source and historical receipt qualification

Against the accepted audit's baseline, exactly three of its 84 input paths
have changed: the BF names shard, the BF reconstruction shard, and
`src/native_legacy_exception_owner.cpp`. Historical pins replay exactly and
are kept separate from the new current pins. The legacy-owner diff adds the
default-constructor adapter include and changes the private `construct_base`
helper to call `construct_native_allocator_base_default_00bf632f(&owner,0u)`.
Its existing `noexcept` policy remains. The raw declaration has no `noexcept`
and receives the actual existing 40-byte owner with a valid 12-byte prefix.

The current raw default Source performs two ordered DWORD read-modify-write
AND clears before profile publication and returns the receiver. The actual
typed public constructor's 703-byte Source slice remains unchanged from the
later cleanup-correction receipt, but the helper beneath it has changed.
The original 283-byte `construct_base` slice is now a 293-byte raw-call slice.
The earlier initial owner's constructor slice also differs from the current
RAII constructor; only the later corrected slice matches it.

The latest primary default-constructor report records the current compiler
effects: the public constructor's inlined default call is covered by a
60-byte EH section with flags 1, max-state 3 and an explicit `std_terminate`
unwind entry. The private helper retains a 36-byte FuncInfo section with
flags 5/max-state 0. Its complete code span is 64 bytes, including five
trailing CC bytes; the public constructor is 132 bytes / 40 operations.
The eight typed-owner EH sections and two actual direct-call records are
retained as inherited current Source evidence. Their objects, compiler
execution, build and fixtures are not rerun by this packet. They do not
establish Original CRT/EH identity or confer `noexcept` on a raw declaration.

All 57 latest primary repository input pins plus the three relevant worker
Source pins match current canonical bytes. One recorded readiness report
uses a CRLF physical form while this worktree has LF bytes; both hashes and
length domains are replayed explicitly. It is not a content drift.

All 12 earlier string/owner Source artifact hashes replay in their historical
canonical or CRLF forms; six whole-file references differ from current Source.
The six earlier cleanup Source references also replay historically, but the
two references to the typed-owner file now differ from current bytes. The
raw cleanup header/body remain unchanged. Therefore the older cleanup receipt
is not asserted wholly current, even though selected raw Source and constructor
text slices still match. No older build/fixture/EH claim is silently promoted
to a new current execution claim.

## Validation and integration boundary

Static validation compares all 60 inline-assembly operations with the complete
accepted PE decode, resolving all six local labels and normalizing only
JNE/JNZ spelling, numeric notation and whitespace. It accounts for every
operation, repeated read, branch and distinct return; it compiles nothing.
The report pins 140 current canonical/physical repository inputs, the three
new header/body/document outputs, accepted evidence and local replay artifacts.

Neither new function is registered in CMake, called by a migrated consumer,
forced into a linker map, or supplied with fabricated backing/storage.
The integrator must register the translation unit, run the normal MSVC Win32
build, review both complete emitted leaf bodies, physical Core membership,
symbols/graphs and any actual selection before Source admission. Identical
emitted bytes are not presumed from identical assembly operation text.

No CMake, ledger or Ghidra mutation, Native body expansion, worker build,
test, probe or annotation was performed. This candidate records zero new
Source-function/byte, Original ABI, runtime or gameplay credit. The existing
default, cleanup, copy and caller admissions are not counted again.

## Primary compiled review

Normal registered build: `2026-10-09T13:39:47Z..13:40:03Z`, exit 0; all three
existing checks pass. The primary retains 61 Source/build input pins, four
artifact pins, nine complete COFF objects and every physical relocation
graph, with ten unique positive Core roots. No tests or probes were added.

Both naked helpers emit exactly the owned 78 and 82 Original bytes, 30
operations each, with no relocation or local EH. All six RET4 sites, late
reloads, CMP/POP/JNE order and branch targets match. Each has one actual
Core definition; both are absent from the application map.

The prior typed owner has identical complete code/relocation contracts
for all 20 functions and identical payload/relocation contracts for all
eight EH sections. This Source context remains distinct from Original EH.
No production consumer, Native profile binding, default storage or lifecycle
was added. Core membership and build results do not prove execution.
