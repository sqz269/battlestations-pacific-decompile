# Native hardware layout factory

`get_or_create_native_hardware_layout_00b2f710` reconstructs the complete
231-byte `00B2F710..00B2F7F7` factory. The native function ignores renderer
`ECX`, receives the actual stream-list identity on the stack, returns the owner
in `EAX`, and ends in `RET 4`. The new interface uses the existing construction
context and is not a binary replacement.

The factory borrows the actual tree `0108D530`, canonical pool `0108FE9C`,
invalid-parameter service, and existing owner/string domains. Tree and pool
identities remain fixed during a call; the constructor's reached-input storage
preconditions apply. Its keys are raw declaration identities, not a normalized
description of the resulting D3D declaration.

## Behavior and exception boundary

After actual `B28220` lookup, capture iterator owner and current global head
before validating ownership. Read the selected node after the first possible
invalid-parameter callback. A hit validates the captured owner and its current
head again, reads node `+20h`, performs real `InterlockedIncrement(owner+4)`,
and returns that owner. There is no hit allocation or construction.

A miss calls actual `B606F0` allocation and arms raw-slot cleanup only after it
returns. For a nonnull slot, actual `B60CB0` construction runs. The guard is
disarmed before raw pair construction `B282B0`, raw pair copy `B25EF0`, and
actual insertion `B2F540`. Original scratch pairs remain uninitialized outside
the fields written by those helpers. Ignore the insertion result and return
the new pointer; there is no extra cache reference and no destruction or pool
return if insertion subsequently fails. The explicit null-allocation path
still copies and inserts a null value; this path is preserved in both source
and compiled branching but is not forced through a synthetic allocator test.

The original one-state unwind map at `DF60A0`, `FuncInfo` at `DF60A8`, action
`CBD800`, and handler `CBD808` return the captured raw slot through `B60260`
only for incomplete construction. Native `nTryBlocks` is zero. The source
cleanup uses an SEH search filter for a second MSVC C++ exception, preserving
termination before nested exception cleanup; it introduces no C++ catch. The
compiled source likewise has zero C++ try blocks and one unwind action.

## Verification

The private fixture executes the full original factory and original EH action,
handler, map, and `FuncInfo` against explicit ABI bridges into unchanged current
lookup, allocation, construction, pair, insertion, and raw-return APIs. Its
original image is copied read-only from the installed PE into a reserved range
of a new fixture child before that child's loader initializes heaps. It does
not patch or execute the game process. All 21 captured live-Ghidra ranges
match installed PE bytes or zero-filled PE virtual data. Runtime postimages
are checked against those preimages with exactly 11 declared binding sites:
nine entry jumps, the factory's EH registration operand, and Interlocked IAT.
The original profile words and internal factory branches remain unchanged.

The fixture uses a canonical initialized pool postimage at `0108FE9C`, real
Win32 critical sections, allocator-list and singleton/string domains, real
slab allocation/return, a retained actual CPU declaration, and the current
constructor. The COM device is a deterministic ABI fixture which writes the
actual `owner+40h` output and returns `E_FAIL` or throws. It proves the caller
boundary and state ordering, not D3D driver behavior. Observation aliases
forward the current source APIs and real lock operations.

Four paired scenarios match **9,230 DWORDs in 71 events**, with 130 words per
event:

| Scenario | Original/current words | Checked consequence |
| --- | ---: | --- |
| Miss followed by hit | 2,470 | One construction, refcount 1 then 2, no hit allocation or extra cache reference |
| Constructor writes COM then throws | 3,120 | Actual base cleanup precedes one raw return; same slot is reusable; no derived COM release |
| Insertion node allocation throws | 2,730 | Completed owner, COM, declaration reference and allocated slot remain; no rollback |
| Initial slab allocation throws | 910 | Factory guard is unarmed; actual pool lock and earlier publications remain |

A separate original/current terminal pair matches **2,470 DWORDs in 19
events**, both exiting 86 through the fixture's terminate handler. During
constructor unwind, a second C++ exception after real raw-return lock entry
terminates with one in-flight exception, zero nested probe cleanup, one raw
return call, 31 free slots, OS recursion 1 and pool depth 0. It never publishes
a second free slot. This is an actual original EH comparison.

The strict MSVC Win32 whole build and its two existing tests pass; all eight
native math seed spans verify. No new repository test suite was added. The
constructor dependency is the exact source integrated at `3159da4`; its
independent original comparison covers 30,494 words. The separately completed
insertion fixture (`93af8a3`, report
`reports/native_hardware_layout_tree_insert_fixture.json`) has 8,582 matched
behavior words; its 14 code pins match this checkout. Primary integration
later closes those dependencies at `8384134` without changing this source.

The audit is `reports/native_hardware_layout_factory_audit.json`. Reproduce
the retained local evidence with `local/prepare_hardware_factory.py`,
`local/build_hardware_factory.ps1`, and
`local/build_hardware_factory_check.ps1`; the audit writer validates saved
traces, runtime postimages, source and artifact hashes. The private harness
and raw exports remain ignored. Primary integration registered the source,
created the complete Ghidra function, saved evidence comments and ledger records,
and refreshed the export. A new comparison selects the factory and complete
constructor/tree dependencies from a frozen current primary library. It retains
the unchanged worker fixture and exact production observation providers, and
again matches all 11,700 DWORDs. All 160 worker pins, 21 fresh live/PE spans,
and 63 loaded postimages were independently checked; the linked factory's
exception information still has zero C++ try blocks.
The initial saved program had no function object at `B2F710`; its complete
span is established by fresh disassembly and the executed original bytes.

Status: complete source reconstruction, strict build tested and focused
original-code/API-boundary fixture tested. This does not establish the new
interface's original ABI compatibility, full renderer startup, arbitrary
invalid-key behavior, driver behavior, or gameplay validation.
