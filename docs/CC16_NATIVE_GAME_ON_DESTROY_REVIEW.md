# Independent review: native Game OnDestroy

**Pass for the qualified normal parent source.** No source blocker was found in
candidate `231bcaaf5627f7a8e7c5abc9072e3d547c4b4038`. This review is frozen to
that commit and its parent `605373bf732e4bd3089e8f07ddca226ab4d635a3`; it does
not import later integration changes or promote the six unresolved suppliers
to completed runtime bindings.

Only this review and its JSON report are changed. There were no source, CMake,
ledger, Ghidra or original-install mutations, no runtime/probe/new-test launches,
and no new native/vendor/activation credit. One independent strict Win32 object
was compiled; no executable was linked or run by this review.

## Original sequence and caller

The existing BSP project/program was verified before bounded native queries.
Installed PE bytes independently decode to all **219 bytes / 67 instructions**
of `004DC5C0..004DC69A`, and every instruction address agrees with the shared
export. Body SHA256 is
`8c32ce9af196221132a316e98d790da4b2162420d718879aba910e57b2a94cd4`.
There are seven direct CALLs, six indirect CALLs, and the final tail JMP.
The actual call verifier independently passes all eight direct/tail rows.

The complete source retains the native observable normal order:

1. Actual CE7C98 trace argument to the real RET-only role, then Game+5D4=10h.
2. Whole 46F3E0, raw-Game 4DA780, then raw-Game 4DA650.
3. One current F8A2FC capture. A nonnull capture invokes current slot8 with no
   stacked native arguments, then 992AC0 unconditionally within that arm.
4. Close the embedded Lua state at Game+1A0C, then call whole raw 8844F0 on
   the freshly loaded Game+1A08, including null.
5. Separately reload Game+1A08 for its scalar delete, then Game+21A0, +21D8,
   actual E18678 and E1867C. Each nonnull captured receiver receives slot0(1),
   then the current owning field/cell is cleared. A callback replacement is
   discarded by that clear; a null initial value receives no call or clear.
6. Run the real validated class-cleanup default corresponding to tail 4A9AC0.

The fresh machine-release pointer is not reused for the following scalar test.
Publication loads occur after the preceding callbacks. The source uses volatile
DWORD Game access and actual volatile cell references for those epochs, without
substitute object layouts or cached publication copies.

Independent callee-head checks confirm that 46F3E0 reloads E18680 and 992AC0
reloads F8A300 themselves, so the required no-argument interfaces do not drop
incoming register inputs. 4DA780 captures Game ECX; 4DA650's first call epoch
retains the original Game ECX. Its required interface correctly carries that
raw Game rather than assuming a frontend object layout.

At the sole original caller, 737FB9 calls OnDestroy, 737FBE reloads the current
Application+14 pointer, 737FC7 overwrites EAX, then 737FCB scalar-deletes with
flag1 and 737FCD clears that application field. The new void API does not assert
the original physical tail, register result, flags, FP return or binary entry
ABI. Caller attachment and the complete application shutdown sequence remain
outside this candidate.

## Real reused helpers

B65E80 and B669A0 independently have the same complete normal instruction
sequence after rebasing local branches: capture state+4, test it, test the
**byte** ownership flag at+0, optionally call the same A68A90 Lua close, then
clear state+4. The reused source has the corresponding uint8 flag and captured
state pointer. Ownership, padding and tracking metadata are untouched. This
supports normal actual-storage composition, not native ABI, invalid-storage,
Lua nonlocal-error or foreign exception equivalence.

The final class call is explicitly qualified to
`NativeGameLifetimeCalls::call_004a9ac0`; it cannot be replaced by a derived
no-op override. That real default checks both contexts, the same call service,
actual vector storage and matching string/manager publication references, then
calls the existing real class cleanup. Inspection confirms that cleanup walks
the actual registry, invokes current payload scalars, frees/reset its tree and
clears all six actual vector headers. It is not an empty placeholder. Actual
registry, payload tables, string pool and heap custody are still required.

OnDestroy owns a separate class-cleanup operation from the later storage
destructor. Source failure retains completed effects, marks the operation failed
and rethrows. Diagnostic retirement performs no cleanup or publication repair.
The operation's no-replay/abandonment protocol is an explicit source contract;
it does not reproduce the original parent's exception machinery. Original
FH/SEH/fault, arbitrary reentrancy and concurrent mutation remain qualified.

## Supplier closure remains an activation blocker

All six `NativeGameOnDestroyCalls` operations are required pure virtual
contracts. None is supplied by a successful empty default. These gaps are real:

| Binding | Independently confirmed limitation |
| --- | --- |
| Whole 46F3E0 | The existing singleton body is usable with a genuine Seg6 service, but the current application `SceneDatabaseSeg6Host` marks its deleting virtual method `not_reached`. |
| Whole 4DA780 | The existing screen-level helper explicitly covers only the 4DA99D..4DA9CF fragment of the much larger session parent. |
| Whole 4DA650 | The existing frontend helper explicitly clears its in-mission pointer without implementing that object's destruction. |
| Captured online slot8 | Actual current receiver/table/code/lifetime and no-argument ECX method shape must be produced; a native PE token is not callable host code. |
| 992AC0 | Its semantic helper needs the actual current F8A300/F8A2FC publications and matching factory, observed after slot8. Neither a captured receiver nor local copies close it. |
| Raw 8844F0 | Existing `MissionLuaHostState` uses a semantic C++ layout; it must not be reinterpreted as the actual raw host. |

The same actual `NativeGameLifetimeContext`, genuine child scalar tables/heaps,
class/profile/string domains and mapped trace literal remain mandatory.
`NativeGameCurrentVirtualCalls` is real dispatch glue but owns no table or child.
Its existence does not make raw native profile/vtable identities callable.
No source or runtime admission should mark these holes closed merely because
a conditional path was absent in a fixture or a link-only artifact resolved.

## Independent compile and provenance

All **28 source pins** and **11 retained author artifacts** match their receipts;
the 26 existing dependencies also match their pinned605 git blobs. The candidate
changes exactly its four new files. No future dependency was pulled in.

A fresh `/c /std:c++17 /O2 /Ob2 /DNDEBUG /MD /EHsc /W4 /WX /fp:strict`
MSVC Win32 compilation passed. Independent COFF parsing finds the same 637-byte
typed parent section as the author's final object, with identical machine bytes
and normalized relocation targets. This is compiler-output reproducibility,
not native/source whole-body identity. The private trace section is exactly
one byte C3. The earlier C2 00 00 attempt is retained and matches its separate
failed-attempt receipt; it is not used as final evidence.

The fresh parent object directly references real B669A0 close and the validated
class default. Its emitted Game field accesses preserve the state write,
machine load/call/reload and all post-scalar clears; publication scalars and
clears follow in order. Review artifacts and exact hashes are recorded in
`reports/cc16_native_game_on_destroy_review.json`.

This supports qualified source integration and subsequent primary build/link
review. It is not provider closure, original ABI/EH compatibility, application
attachment or gameplay validation. Those remain explicit admission work.
