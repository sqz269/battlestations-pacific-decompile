# Unit class-resource selection readiness

The complete `0087BCC0` gate confirms how the initializer captures a class
resource, calls its selected-instance provider and publishes the constructed
unit model. One approved profile cell establishes a **conditional** target:
profile `00CFD8CC`, slot `+8`, contains `007137F0`. It does not establish that
the actual production class resource has that profile at this call.

Existing Source also contains the actual class `+50` publisher, resource
loader and game-resource constructor. Their presence advances the ownership
map but does not complete a production unit/class/resource composition.

## Whole Native body and permitted extension

Ghidra was verified as `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, image base `00400000`, 64,730 functions.
`0087BCC0..0087BF73` is **692 bytes / 232 instructions / 21 calls**:
12 direct sites over eight targets, plus nine indirect sites. Original PE and
fresh Ghidra bytes agree; every saved/live listing row and independent
instruction start agrees. Body SHA-256:
`da924710786a0ee671717c865c0352fca292beb684abd1282bdc0447bdaa9c23`.

Original ABI is ECX=unit, preserved EBX/EBP/ESI/EDI and bare RET. The body sets
an FS exception frame naming `00C967E6`. The zero-parameter live prototype and
decompiler's apparent float argument to unit `+190` are not the ABI contract.
Source uses an ordinary borrowed-view C++ interface; Native FH3/SEH/fault
equivalence remains unproved.

The primary agent approved exactly one additional read: four bytes at
`00CFD8D4` (`00CFD8CC+8`). Full Original PE SHA-256 was rechecked as
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
PE and live Ghidra bytes are `F0 37 71 00`, little-endian `007137F0`.
No child function, other profile cell or unwind body was opened. Here,
"live" means the current Ghidra program, not a running game's heap.

## Capture, dispatch and publication order

After activation, health/vector initialization and the parent traversal,
`0087BDF5` reloads `unit+354`; `0087BDFB` captures that descriptor's `+50`
resource in EDI. This identity survives subsequent kind, allocation and
selector calls. Replacing the unit descriptor or its `+50` later does not
replace EDI. The captured resource and every reached table must stay valid.

| Step | Kind 1B branch | Other branch | Required behavior |
| --- | --- | --- | --- |
| Allocate 1ACh | 0087BE1C | 0087BE5F | Save exact allocation in EBP and cleanup slot; retain null-allocation arm |
| Unit table, detail, resource table | 0087BE32..3C | 0087BE79..7F | Capture unit table; FLD current CED9E0 or FLD1; capture resource table; read unit +190 entry |
| Spill detail | 0087BE45 | 0087BE88 | FSTP float32 to stack, preserving actual x87 effects |
| Unit selector call | 0087BE4B | 0087BE8E | ECX=unit, no stack argument consumed; EAX=selector; detail remains on stack |
| Late resource target read | 0087BE4D | 0087BE91 | Read +8 from the **saved resource table**, after selector returns |
| Resource call | 0087BE52 | 0087BE95 | ECX=captured resource; stack `(selector, detail)`; required RET8; EAX=selected instance |
| Unit-part constructor | 0087BE58 | 0087BE9B | ECX=saved allocation; stack `(unit, selected instance)`; RET8 |
| Model publication | 0087BEA4 | 0087BEA4 | Store constructor result at current unit+360 only after return |

The caller saves the table pointer, not the `+8` entry value. A selector may
change the saved table's entry before the late read. A selector that merely
replaces the resource's vptr does not redirect this call to the new table.
Therefore neither an early cached target nor a later current-vptr lookup
matches the established order.

If class `+50` is null, the body clears `unit+360` and returns immediately.
If allocation is null, it clears `unit+360` and continues the property/group/
numbering tail. A returned selected instance receives no caller-side AddRef,
Release or null check before construction. There is no old-model release at
the publication store. Existing constructor Source stores the selected
instance at its `+160`; its staged member cleanup remains a separate contract.
The caller Source catch frees the saved outer allocation on selection or
construction failure and rethrows. It adds no selected-instance rollback;
early constructor failure before selected-set ownership is established must
not be described as an unconditional successful transfer/cleanup guarantee.

The remaining body is also covered. Parent `+B0` is called twice after a first
nonzero result, with a fresh target lookup. Returning vector traps preserve the
captured class-vector identity while required fields are reread. Saved-property
kind 2 reads numbering from the same saved object after state update; the other
arm reloads current holder/bag. The final model-presence test precedes the
numbering store, kind 6/1B queries short-circuit, and `unit+360` is reloaded
after those callbacks before numbering dispatch.

## Current Source provenance and the conditional witness

`src/native_unit_health_parts.cpp` already implements the complete initializer
and preserves the saved-table/late-entry sequence. Its `call_part_set_08`
remains a required captured-target provider. Historical checks of two unit
`+190` profiles support the no-argument selector contract; they are not fresh
profile/body gates or permission to replace every selector with constant 6.

| Existing Source | Established contribution | Remaining limit |
| --- | --- | --- |
| `native_damageable_class_construction.cpp:126` | Initializes actual class+50 to null | Does not load a resource |
| `native_damageable_class_model.cpp:62-106` | `00879590` requires nonempty class name and null class+50; stores loader result at +50 before temporary-name destruction | Existing nonnull resources bypass loading; complete class/storage/context lifetime remains required |
| Same file, lines 56-60 | `007188A0` captures actual game factory, gets actual manager, invokes full load/cache | Does not force every cache result or callback-mutated manager publication to one profile |
| `native_resource_load_cache.cpp:96-180,209-213` | Cache hit retains resource+4 and returns current manager+24; miss dispatches captured factory target, parses/caches and returns its captured final result; known target 0071B870 has a concrete provider | Cache/current-target/resource provenance must be established for the actual call; no new factory profile cell was examined |
| `native_resource_construction.cpp:69-95` | Actual 0071B870 allocation calls 0071B810, which stamps resource profile CFD8CC | Conditional constructor path, not proof of the actual class+50 object's history |
| Approved cell CFD8D4 | CFD8CC+8 contains 007137F0 in original/live analysis memory | Valid witness only for that saved profile and that late entry value |
| `native_resource_graph_builder.cpp:118-143` | Existing 007137F0 wrapper preserves FLD/FSTP then uses the full graph builder; builder publishes instance+0C root | Actual graph providers, node/resource lifetimes and production invocation still required |

Vehicle-class activation already composes the actual class-model loader in
Source. Unit activation also has a complete Source caller with required
bindings; its name alone proves no particular class-resource side effect.
No production `game_*` consumer of the named class-resource/health composition
was found in either this worktree or current Root. Twenty-six relevant Source
files match Root after LF normalization; this is not a compiler-input receipt.

This corrects a possible overreading of the older health document's unresolved
class+50 producer: an actual Source publisher exists. The unresolved condition
is the genuine production class owner, its loaded/cached resource identity,
saved profile/entry and retained graph, rather than absence of a loader body.

## Readiness boundary

The static `CFD8CC+8 -> 007137F0` mapping is now established, subject to the
saved-table contract. Production admission remains held. A concrete next gate
can start at the existing class+50 publisher `00879590`, with actual class
storage and load/cache/current-profile provenance kept explicit. That body was
not opened here. No semantic receiver cast, raw root accessor, numeric-vtable
call or fallback provider closes this ownership gap.

Source121 and Source507 are frozen historical receipts. Root reported its
current 509-input build, three checks, 43 Core plus one App whole objects,
51 roots, real-Lua validation and a bounded three-tick/two-Present startup.
This worker did not execute or verify those receipts and read no current build
artifact against an old pin. This two-file readiness packet adds no C++, build,
test, probe, ledger/GPR mutation, Original-ABI, startup or gameplay credit.

Evidence: `reports/cc12_unit_class_resource_selection_readiness.json`.
