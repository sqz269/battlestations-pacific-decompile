# CC12 Application outer-construction Source admission

This Source-only packet admits one next implementation: a dormant borrowed
`00BEA970` loop-base constructor calling the existing genuine `00BEA810`
constructor, followed by exactly its two normal-return stores. It admits no
actual `E1AE90` producer, raw Application owner, full `00737970` Source constructor,
factory, startup activation, automatic failed-construction storage return,
retirement, callable Native profile, Original FH3 equivalence, or current external
ABI. This packet changes only this document and its report.

The baseline is integrated Main `c02232befa117182c0536d6e60dbaedeedddff58`, frozen after Root's new Application
Source3356 epoch (`d171fd187a6609a8be42fa29f39cb495ad8c3d0e3fd34b18f7470e734d8c602a`). All 3,356 Root project inputs are physically
retained and LF-equal to this worker epoch; 8 differ only in raw newline
bytes. The closed Source/build-input inventory contains 4,228 paths, including
all 4,210 `src/` and `include/` files. Actual committed blobs, complete directory
trees, the commit object, and 4255 retained Git rows are independently checked.
The only normalized Source differences from the retained BEA810 implementation
epoch are the accepted `native_mesh_owner.hpp/.cpp` changes. The four artifacts
named in the Root manifest are historical declarations, not new evidence here.

## Accepted facts and current Source boundaries

The complete retained Native shells are `00737970` (42 bytes, 15 instructions)
and `00BEA970` (22 bytes, eight instructions). Both retain the original ECX
receiver in ESI and return that original receiver in EAX with `RET0`; neither
allocates the enclosing receiver. `BEA970` calls `BEA810`, stores DWORD `D68BC8`
at original `+0`, then clears exactly BYTE `+04`. After that child returns,
`737970` stores DWORD `CFEAB0` at `+0`, clears DWORDs `+08/+0C/+10/+14`, clears
BYTEs `+19/+1A`, then writes BYTE `+18 = 1`. There is no direct store to `+05..07`
or `+1B`, no aggregate zeroing, and no authority in a child's returned pointer.
These are positive accepted body facts replayed from retained records; this
packet obtains no new Native bytes or live Ghidra evidence.

The current Source inventory is not an absent-function census by name alone.
Fourteen complete bounded queries and 32 exact excerpts retain the providers,
callers, member ownership and failure paths relevant to this decision:

| Area | Current positive Source fact | Admission limit |
|---|---|---|
| Dormant singleton base | `NativeApplicationSingletonBaseContext` borrows actual, distinct manager and Application pointer cells; genuine `BEA810` performs the accepted base schedule | Its supplied `E1AE90` reference does not create or authenticate a canonical cell owner; declaration and definition have no Source caller |
| Shared manager | `GameNativeStringProcess::manager_01090aa0()` supplies the actual process-owned cell already used by `GameSingletonHost` | A second manager domain, copied pointer value, private guard or fallback cell is not equivalent |
| Existing startup | `GameStartupHost::application_construct()` resets projected frame flags and `constructed_`; destruction clears `constructed_`; platform request identity is that host | These are not construction/destruction of a raw `1Ch` Application owner |
| Game frame | `GameNativeGameApplicationFrame` borrows actual Application `+14` and independent `E1AE78` cells, with a retained runtime/services/string domain and failure policy | Its separate `71A0` Game allocation/zeroing does not allocate or initialize Application storage; retirement does not clear the borrowed Application Game cell |
| Application vector | `NativeApplicationPointerVectorStorage` positively describes the 12-byte `+08` vector and existing preload borrowers | It supplies no enclosing owner or established object lifetime/access contract for a new raw buffer |
| Application name | Existing canonical duplicate/free primitives and accepted producer/retirement slices remain qualified positive evidence | Current startup parses mode directly; it supplies no actual canonical retained name-cell owner or combined factory |
| Allocation | The Source canonical allocation provider has the qualified malloc/new-handler policy; the manager getter owns its separate saved manager allocation | Neither establishes Application heap ownership or a failed-construction free/retry policy for a borrowed receiver |
| Retirement | Accepted ordinary `BEA8B0`, `BEA990` and outer destructor slices retain the existing evidence | No new Source removal/free/drain ordering, Application delete binding or callable profile is supplied |

Lexical absence is scoped to the closed Source inventory and the stated query
patterns. It does not claim that no foreign binary, future caller or unrelated
projection exists. The report contains the full hits and retained excerpts.
In particular, the existing `737970` log label describes the projected startup
shell, and `BootstrapHost::command_line()` returns a `std::string`; neither is
the corresponding actual Native owner/cell. The locale host explicitly lacks
the current Application Lua owner reached through Native virtual `+14` (which
is separate from the actual Game pointer field at object `+14`).

## Smallest next implementation contract

Append one declaration and definition to the existing
`native_render_service_base.hpp/.cpp` pair:

```cpp
void* construct_native_application_loop_base_00bea970(
    void* actual_receiver, NativeApplicationSingletonBaseContext& context);
```

The caller retains stable actual aligned enclosing `1Ch` storage, valid `+0`
word access and `+04` byte access, and every accepted BEA810 prerequisite. Its
actual manager and Application cells remain fixed, distinct and disjoint from
the object, actual manager/vector, captured real native section/current depth,
and all objects selected by providers. The original receiver, section and
providers remain live through callbacks, cleanup and any unresolved escaped
references. There is no projected section, invalidating concurrency, alternate
manager authority, null-manager fallback or private exception alias.

Capture the original receiver and call genuine BEA810 once. Only after normal
return, write DWORD `D68BC8` to original `+0`, then BYTE zero to original `+04`,
then return the original receiver. Preserve every other incoming byte and
domain. Do not widen the byte store, infer its meaning, reload the current
Application cell as receiver identity, inspect the child's returned pointer,
repair publication, repeat registration, add a caller or activate startup.

Publication and registration occur inside the child while the numeric profile
is `D68BC4`, before either new poststore. Qualified observers must tolerate that
stage. These numeric profile identities are not callable tables.

If BEA810 escapes or never returns, neither new poststore occurs. Reuse its
existing cleanup exactly: Source base cleanup is armed before the initial
profile; the guard is armed after the conditional captured-section
Enter/current-depth stage, and remains armed through conditional normal Leave.
A null captured section skips Enter/depth/release and still arms state1 before
publication. Cleanup invokes the genuine
guard, then generic base reset only if guard cleanup returns. A normal-Leave
escape may cause another current decrement/Leave; the earlier release's
disposition remains unknown. A second escaping MSVC C++ cleanup exception
terminates through the existing private boundary. Add no compensation, outer
cleanup, clear, unregister, free, retry, rollback or retain operation.

The accepted compiled base evidence distinguishes Source arm order from emitted
MSVC state-selector placement: the selector for state0 follows the profile store
but precedes the first throwing getter. The guarantee is ordinary C++ cleanup
under valid storage. Hardware faults, Original FH3/private frame layout and a
current external ABI are outside this contract. Under these prerequisites the
two new stores introduce no ordinary C++ throwing provider, so this wrapper adds
no post-base cleanup state.

This is admission for a future, separately assigned implementation and fresh
Win32 verification against its own frozen epoch. No C++ implementation, build,
test, probe, game execution, OS/audio validation or Native validation occurs here.

## Why owner construction and storage return remain held

Full `737970` Source composition first requires an explicit actual `+08` vector
subobject and `+14` pointer-cell object/lifetime/access contract, including their
preimages before base publication. A typed object, a byte array, a projection
and detached `void*` copies are not interchangeable. The accepted normal bodies
already settle store order; another normal-shell body read is unnecessary.

The same stable owner must supply actual `+14` to the retained Game frame and
remain alive with its exclusive runtime, canonical string domain, services and
failure record through every last use or observation. Independent `E1AE78`
production and retirement cannot be silently folded into Application scope.
The existing WinMain startup projection does not supply this owner or automatic
unwind ownership.

Partial base construction can expose the original receiver through publication
or the registry. Current publication may later name another object or null;
registration may select another/null argument. Base cleanup resets only the
original object's generic profile. It does not establish removal of all escaped
references or a storage-return token.

There is a concrete Source drain hazard: the manager drain reads each current
profile and dispatches deletion with flags `1`. Profile `CE3818` positively maps
to `delete_native_singleton_base_00412440`, whose bit0 path calls
`singleton_lifetime_free(original)`. Therefore an escaped receiver reset to
`CE3818` can reach the generic free path. Rejection of unsupported Application
profiles does not protect every failure state. Neither base success/reset,
changed publication, unproved insertion, nor C++ scope exit authorizes storage
return. Pre-entry allocation ownership, entered construction, partial effects,
returning/nonreturning cleanup and unresolved external references must remain
separate future obligations; retain indeterminate dispositions.

No new external Native evidence is prerequisite for the narrow BEA970 wrapper.
If stronger Original caller storage/failure behavior is later needed, the
smallest proposal is a separately gated metadata-only packet for WinMain
`008F81F0` and the enclosing EH range of the retained `008F8419` call. Reuse the
existing 57-byte receiver-identity slice first. Any later body/table/handler
read requires exact typed ranges and a separate byte cap; no such read is
performed or authorized by this report.

## Portable evidence

`local/cc12_application_singleton_outer_admission_readiness/` contains a finite
supplied ZIP plus portable helper, manifest, original replay and distinct
physical-copy replay. The strict guard allows one read-open of the supplied ZIP
only, after dependency preload; it blocks other filesystem opens/writes,
process creation, networking, new library loads and directory enumeration.

Replay verifies every member hash/size; real Source Git objects and complete
trees; all bounded query hits/excerpts; the new Root3356 physical Source epoch;
the accepted 42/22-byte bodies; and the decision boundary. It physically retains
and recursively replays all 853 prior readiness pins and 108 helper sources, plus
the final immutable BEA810 implementation ZIP, helper, exact committed/working
outputs and 23-helper graph. Historical compiled evidence remains historical.
Capture/mutation helpers are retained and parsed, never executed by replay.
Exact final ZIP/helper/output pins and replay results are in the accompanying
report and separate immutable delivery receipt.
