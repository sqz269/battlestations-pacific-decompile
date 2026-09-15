# Native mesh resource parsing and ownership

Addresses: 00b947a0, 00b94850, 00b94900, 00b93910, 00b93b40, 00b93b60,
00b93b80, 00b93ba0, 00cc2df0, 00cc2df8, 00cc2ef0, 00cc2ef8, 00cc2f03,
00cc2f10, 00cc2f18, 00cc2f23, 00cc2f30, 00cc2f38, 00cc2f43.

The three resource parsers now call the completed native mesh loader, produce
actual 10h items and connect to the existing numeric resource dispatcher.
Their deletion adapter consumes the actual item reference and releases the
mesh through the same canonical geometry owner domain. Source lives in the
already registered `native_mesh_subset_loading` module; CMake is unchanged.

Eight complete ordinary spans total 751 bytes. Eleven compiler supports total
105 bytes. Saved Ghidra and installed PE agree for every span and all 272
instruction owners; the report records 27 direct and 13 indirect transfers.
The executable SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
All live batches verified project `bsp`, `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Descriptive names remain reconstruction hypotheses.

## Construction and ownership

| Entry | Bytes | Final item profile |
| --- | ---: | --- |
| B947A0 | 169 | D63738 |
| B94850 | 175 | D6375C |
| B94900 | 175 | D63780 |

Each ignores incoming parser ECX, takes one stacked node handle and returns
the item or null in EAX with RET4. B94710 receives an actual eight-byte local
pair. BF681B allocates 10h; B868B0 establishes the item base/reference count1.
The parser stamps D63738, copies mesh/prefix to +8/+C, and increments the
captured mesh's actual +4. The two variants then replace only the profile.
Cleanup is disarmed before normal release of the captured temporary mesh.
Successful construction therefore leaves item ref1 and mesh ref1. Null item
allocation still consumes the temporary mesh and returns null.

The normal release does not clear the local pair. Exception state0 instead
calls B93BA0, which captures pair[0], decrements nonnull mesh+4, invokes its
current slot0 only at zero, and clears pair[0] after successful return. A
throwing terminal skips the clear. Pair[1] is untouched.

| FH3 descriptor | State | Next | Action |
| --- | ---: | ---: | --- |
| DFC714 | 0 | -1 | CC2EF0 -> B93BA0 local pair |
| DFC714 | 1 | 0 | CC2EF8 -> BF65AC raw item |
| DFC748 | 0 | -1 | CC2F10 -> B93BA0 local pair |
| DFC748 | 1 | 0 | CC2F18 -> BF65AC raw item |
| DFC77C | 0 | -1 | CC2F30 -> B93BA0 local pair |
| DFC77C | 1 | 0 | CC2F38 -> BF65AC raw item |
| DFC59C | 0 | -1 | CC2DF0 -> B86890 item base |

State0 is armed before B94710 publishes the native stack pair. Source keeps a
persistent acquired frame and initializes only its bookkeeping. A failure
before publication, or in the added canonical registration of a completed
mesh, explicitly sets `cleanup_deferred`. It retains the completed unbound
creator for caller resolution instead of treating uninitialized native stack
bytes or an absent metadata binding as a valid cleanup target. These source
failures are an explicit boundary, not native exception equivalence. Once
publication/admission succeeds, loader failures execute the recovered mesh
handle cleanup. Failed frames must persist; pointer fields are audit identities
and do not authorize replay or an additional release after retirement.

## Item destruction and dispatch

B93910[101] stamps D63738 and unconditionally decrements the mesh at item+8.
It invokes mesh virtual0 only at zero and destroys the item base normally or
during unwind. It never clears mesh+8 or prefix+C. B93B40/B93B60/B93B80 are
identical 30-byte scalar deletes: call B93910, free iff flags bit0 is set,
return the captured item pointer, RET4. B93BA0 is a separate 41-byte handle
release with plain RET.

`NativeMeshResourceCalls` recognizes exactly the three parser targets and
retains a distinct acquisition frame for every reached call. Other operations
forward to the supplied chain. `NativeMeshResourceReferences` recognizes
BD30E0 with one of the three item profiles, reads CURRENT item table/slot4
and selects the corresponding scalar delete with flags1. Resource references
remain the actual item+4; mesh references remain the actual mesh+4. No second
ownership registry or extra reference is introduced.

Five returning-free continuations were truncated in Ghidra: B93B60, B93B80,
CC2EF8, CC2F18 and CC2F38. Their instruction-level call overrides were cleared
and their bodies restored under the write lock. Four missing dispatch handlers
CC2DF8/CC2F03/CC2F23/CC2F43 were defined. No bytes were cleared and no global
no-return setting was changed. Prior comments were preserved before edits;
19 annotations and eight ABI/source records are saved and exports refreshed.

## Validation and limits

The tracked MSVC Win32 build and both existing CTests pass. The existing mesh
D3D9 probe was extended locally, with no repository test added. All three
parsers execute the nine-field loader and two complete subsets with explicit
and default stream selection, real vertex/index uploads and generic layout
readback on an RTX5090. Each actual item enters B87AA0's resource pointer
array, then 483850 -> BD30E0 -> current slot4 reaches its proper scalar delete.

A second outer sphere read failure after both subsets completes exercises
parent state0: current child cleans up, mesh creator is consumed, pair[0]
clears, prefix remains and dependent tree/resources retire. A separate source
metadata bind rejection demonstrates explicit deferred ownership until fixture
retirement. All 57 successfully registered canonical owners retire; pools,
caches, hardware tree and allocator list are empty, with D3D9 device/API refs0.

The cached effect is an actual B407A0 construction on an explicit zero-pass
C8..137 preimage and an actual B43700 descriptor with fixture name `generic`.
Cold shader/compiler/state-cache providers remain unexecuted and throw if
reached. This is no substitute for their successful implementation.
Allocation-null/throw branches, flags with bit0 clear, terminal lookup failure,
concurrent mutation and native full-parent machine-code execution are not
newly exercised. The source interfaces are not native ABI/FH3/SEH replacements.
Full resource root traversal and game type classification were not exercised
by this packet. There is no gameplay or rendered-image validation claim.

Reports: [native_mesh_resource_co.json](../reports/native_mesh_resource_co.json),
[flow repairs](../reports/native_mesh_resource_flow_co.json),
[annotations](../reports/native_mesh_resource_annotations_co.json),
[integration receipts](../reports/native_mesh_resource_integration_co.json).
Frozen local proof: `local/native_mesh_resource_co/registered/`.

## Follow-up packets

Recover the actual item type predicates B93000/B930D0/B931D0 and their current
type-publication cells, then compose `NativeResourceItemTypeCalls` so game
classification can consume these items. Item slotC targets
B936B0/B939C0/B93A40, shared B94020 and remaining per-profile accessors are
separate bounded work. General material admission still needs cold effect
children. Changes were published only on `agent/orch4-20260910`; moving main
deltas remain unreviewed.

Correction from [NATIVE_MESH_RESOURCE_CLASSIFICATION_CP.md](NATIVE_MESH_RESOURCE_CLASSIFICATION_CP.md):
B93000/B930D0/B931D0 at item slot8 are current own-ID getters, not predicates.
B936B0/B939C0/B93A40 at slotC are the actual 3/4/4-ID classification predicates.
CP reconstructs those nine leaves plus the scene/mesh/derived type producers
and composes the real predicates with game-resource classification. Production
selector-cell publication and application startup routing remain separate work.
