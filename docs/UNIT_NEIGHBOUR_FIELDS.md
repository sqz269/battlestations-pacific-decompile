# Unit neighbour fields and DummyObjectID transfer

Packet `orch6_unit_neighbour_fields_o` reconstructs the complete control, store and call schedule of `00953A80..00953B7E` as `bind_unit_dummy_object_00953a80`. Its services operate on borrowed live fields and list nodes. It adds no persistent unit, dummy, scene flags, HUD or pending-kill owner. Names are hypotheses; the literal `DummyObjectID` supplies the index's meaning. This is a new C++ source interface, not a binary ABI replacement.

## Consumers and initial producers

`009F0D20` loads `[brain+3FC]+6B8` at `009F0D49..55` and takes a signed-negative branch at `009F0D5C`. A nonnegative ID enables an additional candidate kind-8 gate. This is the unit's signed dummy ID, not descriptor `+6B8` DamageThreshold. `009F0EA0` subsequently loads a retained object's owner at `009F1029` (`[object+14]`), rejects null, and checks that owner's byte `+5E` at `009F1034`. `009EAE20` obtains its receiver's owner at `009EAE26` and checks the same byte at `009EAE33`. The latter byte belongs to the observed owner, not the observer wrapper or a class descriptor.

The existing constructor chain is `0095CC90 ->0087B670 ->0077EED0 ->00928630 ->00925CE0`, with calls at `0095CCB3`, `0087B694`, `0077EEF9`, `00928651`. The middle call pushes registry selector 1 before the flag. Native cleanup is respectively RET8, RET4, RET4, RET8 and RET. The chain passes the same primary object in ECX. Existing `unit_instance_layout` contracts remain the constructor implementation.

At `0095CCAC` ESI captures ECX and remains the unit receiver. `0095CDBE OR EAX,FFFFFFFF` produces -1; no EAX write intervenes before `0095CDE9 MOV [ESI+6B8],EAX`. The next EAX write is `0095CE98`. `0095CC90..0095CF7E` therefore initializes this field to -1. Its seven direct callers include AirField, LandFort, LandVehicle, Shipyard, plane and vehicle base constructors; constructor initial state is not evidence that subsequent loaders are absent.

`00925CE0..00925EF3` captures its receiver in ESI and zeros EBX at `00925CFD`. No EBX reassignment precedes `00925E11 MOV byte [ESI+5E],BL`: the scene-node destroyed byte starts at zero. This does not justify a permanent false flag. `SceneNodeFlags` in `hit_narrowphase.hpp` is the existing semantic projection.

## Subsequent ID inputs and dummy ownership

The newly defined `00953250..0095325C` is a plain store: stack ID to `[ECX+6B8]`, RET4 at `0095325A`. Its xref query returned no references. It is named and documented as evidence only; no redundant C++ setter was added.

The xref query returned 25 DATA references to `00953A80`. All 21 supported creator tables independently resolve primary `+1BC` to it, using the constructor installs established in packet N and the actual slot bytes recorded here. The remaining DATA references are not treated as additional proven creator identities. Validated input sites include:

| Caller/site | Actual source and gates |
|---|---|
| `0095E5B0 /0095E8D0` | For unit `+C0` container kind 1, `008F2260` at `0095E8B3` finds `DummyObjectID` (`00CFC840`). Nonnull property and property kind `+4 ==0` are required; property `+C` is pushed, ECX remains the unit, then primary `+1BC` dispatches. |
| `0081F980 /008201A7` | Container kind 2: after optional submarine depth work, `[container+8]+120` is loaded into EDI and compared signed against zero (EBX). If nonnegative, PUSH EDI and ECX=unit precede primary `+1BC`. |
| `007D5D20 /007D643A` | Plane property loader's represented container record `+12C`, held in EBP, is passed to primary `+1BC` when nonnegative. This packet does not reconstruct the record parser. |

These are validated inputs, not a claim of an exhaustive indirect caller set. Candidate scans also hit unrelated object data and weapon virtual slots. For example `007CE6B7` dispatches on a child selected with kind `25h`, not the plane unit; its weapon count input must not be attributed to this unit writer. Candidate `00953F6E` is a role field load, not a virtual call. Undefined candidate areas remain outside the implementation; no invented routine extents are reported.

`006FF3F0..006FF50E` installs dummy primary table `00CFC910` at `006FF41B` and kind `2Dh` at `006FF4A0`. Its `+130` slot at `00CFCA40` contains `006FEA80`. That registrar calls parent registration `00928560`, then appends the direct dummy identity to parent lists 2, 42 and 45 (`+30`, `+210`, `+234`) in that order. `006FEAAD` is the list45 append; list head is parent `+238`.

The constructor copies process counter `00E19ABC` into dummy `+484` at `006FF4C0..C6` before testing the counter. At `006FF4CC` signed CMP against 250 and `006FF4D8 JL` choose between increment at `006FF4F8` and reset to zero at `006FF4DC`. EBP is zero from `006FF419` and remains so. Image initial bytes are zero; the normal sequence assigns 0 through 250 inclusively, then wraps. Negative explicit initial states increment under the signed branch, and values at least 250 reset after being assigned. Four counter xrefs are all in this constructor. No counter reset is implied on every scene load. This is producer evidence only; the rest of the dummy constructor and a live dummy owner are not synthesized.

## Complete compound operation

Native ABI: ECX=unit, one signed stack argument, preserved EDI/ESI, RET4 at `00953AC4` or `00953B7C` (three-byte last instruction, inclusive end `00953B7E`). The stored Ghidra prototype omits inputs; the listing and cleanup establish them. There is no x87 arithmetic in this body: visibility copies use MOVSS/MOVAPS, and the source uses word copies to avoid introducing float-return conversions.

1. Store the requested ID at unit `+6B8` at `00953A87`, before any world lookup. Load `[00E188A8]+19CC` then world list45 head `+238`; walk next `+4`, direct dummy pointer `+8`, and stop at the first matching signed word dummy `+484`. An empty list or no match returns with the ID store retained.
2. Load frontend `00E198C4`. Only if frontend mode `+20 ==26h`, resolve `[[frontend+8C]+4C]`; a null result means no selected dummy, otherwise read `+398`. If it matches the found dummy, call unit primary `+140` at `00953AF7`. Across the 21 supported creator tables, `0047F320..22` returns this; `006F57A0..AC` returns unit `+738` or this when null (LandFort/CommandBuilding); `0074CDA0..A6` returns `+738` (LandVehicle); and `007B97E0..E6` returns `+9D4` (eight plane creators). All take ECX with no stack arguments and plain RET. Preserve the actual result, including null. Reload frontend after that call, read HUD root `+40`, and call `00647300` at `00953B03` with the returned unit. That existing HUD service has additional selection/command logic; it is not replaced by a selected-pointer assignment.
3. Load global override byte `00F87152` and dummy `+2F0` under native short-circuit order. If either is nonzero, store float word 1 (`00D7A24C`, `3F800000`); otherwise copy dummy `+2F4` to unit `+2F4` at `00953B2F`. Load the global override a second time, apply the same gate, and copy dummy `+2F8` or 1 to unit `+2F8` at `00953B55`. References remain borrowed across the preceding callbacks.
4. Call `006E0B40(dummy,0)` at `00953B5D`. This service writes dummy `+459` and conditionally updates its attached visibility object. Reload game `00E188A8` after the call; if actual session `+1FE4 !=2`, call queued kill `00926D90(dummy,2)` at `00953B75`. Preserve this distinction: it does not directly set destroyed `+5E`.

HUD and visibility take ECX receivers and one stack argument, RET4. Queued kill also has RET4 and owns the registry lock, pending `+5F`, cause, child recursion and pending-list append. These services are required host contracts, not newly implemented library or lifecycle bodies.

## Destroyed-byte transitions and runtime gap

| Writer | Proven relevant transition |
|---|---|
| `00922FD0..00923011` | Direct kill sets `+5E=1` at `00922FDE`, `+5D=1`, `+5C=0`, `+6C=1`; child recursion and virtual `+84` are part of the native operation. Existing small flags helper covers only stores. |
| `009263C0..00926414` | Remove checks `+5E`; if clear, writes `+5D/+5E/+5F=1`, `+5C=0` (`+5E` at `009263F7`), samples/notifies observers, then dispatches `+80`. Existing `remove_killed_entity_009263c0` supplies the host sequence. |
| `009273A0..009275D1` | Pending flush contains the corresponding inline transition at `009274D2`; reuse `flush_pending_entity_queues_009273a0`. |
| `00925A00..00925A89` | Cancel under the registry critical section requires an eligible parent; erases pending entries as gated by `+5F/+60`, clears `+5E` at `00925A58`, and clears `+5D`. It can restore the byte to zero after construction. |

The original-byte candidate scan is explicitly limited to direct displacements in executable sections, not alias/bulk-store proof. Overlap decodes at `0098B517` and `00A27403` are not Ghidra instruction starts; GUI and stack `+5E` stores are different receivers. No claim of exhaustive arbitrary alias writes is made.

At the source snapshot `e21be33a`, GameUnitsHost has no canonical DummyObjectID state or dummy binding delivery. `GameReady`'s `PendingQueueBinding::scene_node_flags` returns a fresh object and `store_scene_node_flags` discards it; `GameWorld::unit_filter_flags` fixes `flag_5e=false`. These are missing owner/delivery bindings, not validated native state. Future integration needs one existing unit/scene owner projected through `SceneNodeFlags`, real pending queues and observer delivery, actual scene property/typed-record ID inputs, a live registered dummy list45 and the existing frontend/visibility/kill owners. N's registry node views can supply traversal once these owners exist. No game-host files or candidate walker are changed here.

## Verification and evidence limits

The ignored fixture executes all 255 original bytes `00953A80..00953B7E` with three relative direct-call targets redirected to supplied HUD, visibility and kill trace services and a supplied primary `+140` callback. Twelve cases compare the stored ID, raw output float words, first-match selection and ordered callback receiver/argument identities: empty/missing list, mode/selection gates, global/dummy overrides, signaling-NaN payloads and signed zero/subnormal words, duplicate IDs, negative IDs, frontend mutation during `+140`, HUD changes to borrowed fields, and session-owner mutation during hide. All pass. It does not compare x87 CW/status/TOP, MXCSR, flags/register residue or exception/unwind state, and it does not execute the external services' bodies. An initial fixture attempt could not reserve a fixed global page; linking the probe at `14000000` with ASLR disabled resolved that fixture address collision. It was not a core failure.

`local/neighbour_probe_inputs.json` records the original image/body hashes and redirections; `local/neighbour_probe.cpp` retains exact inputs, `local/neighbour_probe_result.txt` the observations, and `local/neighbour_manifest.json` the hash inventory including compiled source copies, executable and build log. The report records Win32/CTest and call verification results separately. There is no mission-process or live candidate-walker validation in this packet. The two root-defined entries now have complete normal bodies; no missing body or flow gap remains in the implemented target.

## Correction from docs/UNIT_SCENE_FLAGS_LIVE.md

Packet Q replaces the earlier fresh/discarded scene-flag hook behavior with
persistent cells in the existing stable GameUnitSlot. Existing active+5C and
simulate+5D remain canonical; the same slot now retains+5E,+5F and+60. World
filters and the alive/visible predicate read those actual projected cells.
The supported kind1 initializer paths establish the active store; type2/type3
delivery remains separate. The focused original-byte/production-host fixture
passed after correcting its assumption about repeated local-list rebuilds.

This closes the retained scene-flag storage gap only. Native pending queues,
observer delivery, compound destruction, actual DummyObjectID input delivery
and dummy/frontend/visibility/kill owners are still separate prerequisites.
