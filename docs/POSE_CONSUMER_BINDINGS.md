# Audit of five remaining pose consumers

Packet `orch3_pose_consumer_audit_j` is a read-only source/native audit atop
`2dcee1664904119c34b373188601a6dda2559108`. It covers exactly five existing
reconstructed consumers below. It adds no consumer implementation or native
coverage claim. Force-event spatial composition is already closed and is not
counted again. No additional reconstructed 414E10/B63D50 callback consumer was
identified in this bounded source search; that is not an inventory claim about
all native callers.

The smallest currently unblocked implementation packet is the **parent-pose
branch of scene generation gate 0046C550**. Its arithmetic dependencies are
available, but its current inputs discard the native parent identity. The
recommended change is described below. Positional voice is also small, but
its shared header is presently leased by another owner.

## Field map that every binding must preserve

| Actual entity pose | Canonical view | Meaning |
| --- | --- | --- |
| owner +3Ch | `PoseRefreshView.parent_3c` | Actual pose-parent link, reloaded by refresh |
| +74h | `local_74` | Local matrix |
| +C8h | `world_valid_c8` | Actual byte, any nonzero is valid |
| +CCh | `world_cc` | World matrix; X/Y/Z are +FCh/+100h/+104h, indices 12/13/14 |
| +10Ch | `derived_valid_10c` | Derived-cache byte, cleared by world refresh |
| +110h | `PoseDerivedView.derived_110` | Actual derived matrix, not a second owned cache |

This is distinct from `CameraTransform`: parent +30h, validity flags +5Ch bit
2, local +B0h, world +F0h, position +120h/+124h/+128h. In particular, entity
offset +F0h is **world index 9**, not the entity world-matrix start and not an
X/Z translation. `unit_instance.hpp:34` names a different field at +30h; it
must not be substituted for the pose-parent +3Ch field proven by 414DB0. This
audit does not rename that other relationship or assert its ownership role.

All five consumers need the same live owner flags and matrices, not an owned
Boolean synchronized after callbacks. A temporary three-float coordinate
snapshot used by a calculation is distinct from a duplicate pose/hierarchy.

## 1. Unit instance wake/prop-wash update — 008255B0

Source: `unit_instance.hpp:183-189,255` and `unit_instance.cpp:174-196`.
`UnitInstanceState` owns `bool pose_valid`, `float pose_base` and
`float pose_lateral`; its host refresh receives no explicit owner identity.
These fields are diagnostic/value projections, not references to +C8h/+100h/
+F0h. Calling the real refresh in a host would leave them stale unless the
owner binding itself is changed; copying the flag/coordinates back is not a
canonical field binding.

Native `825A64..825AEC` tests +C8h four times around actual matrix reads. It
uses +F0h = `world_cc[9]` and +100h = `world_cc[13]` for the wake span, with
calls at 825A85/825AA8/825AC6/825AE1. The current source places all four checks
before the reads, so a faithful replacement should restore the read/check
schedule rather than retain that abstraction. This schedule difference alone
need not change ordinary results when the first pure refresh makes C8 valid;
the copied-field binding problem remains regardless.

Required next work: bind this update's actual unit identity to an existing
`PoseRefreshView`, read the byte/matrix fields directly, and preserve the
native wake/prop-wash snapshot sequence. Other update host calls, the scene
node transform and controller/effect lifetimes remain separate dependencies.
No additional math leaf is needed for this pose slice.

## 2. Timed unit damage/anchor update — 00956600

Source: `unit_motion.hpp:193-218`, `unit_motion.cpp:211-249`.
`UnitTimerState.pose_valid` is another owned Boolean projection; its refresh
callback also has no owner argument. Native out-of-range anchor handling at
9567F0..95681B refreshes the actual ECX/ESI unit and captures world X/Y/Z into
stack locals. Those coordinates flow into later notification/effect work.
The current `void transform_damage_anchor(index)`/`void refresh_pose` host
surface does not expose that result flow; replacing just the refresh callback
does not prove the downstream effects use the captured point.

The valid-anchor path additionally calls two small unreconstructed wrappers:
`0042D7E0` checks C8, calls canonical 414DB0 and returns owner +CCh (25 bytes,
final RET 42D7F8); `00414D10` takes ECX=output float3, EDX=input float3,
stack matrix, calls existing `004142E0` through disjoint float3 scratch and
copies the result (64 bytes, final RET4 414D4D..414D4F). Neither wrapper was
implemented or claimed here. A complete bounded anchor-position packet should
claim both, bind the actual unit pose, and carry the captured point explicitly
through the effect/announcement boundary. Descriptor anchors, effect ownership
and unrelated timer/animation operations remain required services.

## 3. Scene parent-position generation gate — 0046C550

Source: `scene_entity_factory.hpp:173-175,196-203` and
`scene_entity_factory.cpp:235-275`.
This is a **generation predicate**, not the native unit-placement constructor.
Native stack argument 3 is an actual captured parent owner in EBP.
`46C6B7..46C6DF` tests its C8, calls 414DB0, and x87-adds parent world X/Z to
the local X/Z. With a null parent, `46C6E5..46C70F` instead multiplies the local
frame by the parent frame passed by value, using existing 413920.

`SceneEntityGateInputs` currently reduces the parent to `bool has_parent`,
and `parent_world_offset(x,z)` leaves both identity and arithmetic in a host.
It owns no pose cache, so the missing identity can be restored without replacing
an existing hierarchy. Recommended packet: preserve the actual nullable parent
identity in the inputs, supply the existing `PoseRefreshResolver`, replace this
one callback with real refresh plus exact X/Z field reads/additions, and derive
the parent-present branch from that identity. Keep parent naming/deferred-record
ownership and unrelated game-mode/property services explicit. A mere helper
that leaves the old callback and Boolean-only input untouched is not closure.

Proposed lease: address `0046C550`; existing
`include/bsp/scene_entity_factory.hpp`, `src/scene_entity_factory.cpp`; new
`include/bsp/scene_pose_binding.hpp`, `src/scene_pose_binding.cpp`,
`docs/SCENE_POSE_BINDING.md`, `reports/scene_pose_binding.json`. All were unleased
when checked. The tracked call-site search found only declaration/implementation
of this entry, which keeps the immediate signature migration bounded. The
canonical 414DB0/413920 operations already exist and need no reconstruction.
Closing the no-parent matrix callback can be separately included only if its
raw-array alias/temporary contract is preserved; it is not required to pretend
that the entire generation gate is a concrete runtime host.

## 4. HUD squad markers — 006435D0

Source: `hud_updates.hpp:239-281`, `hud_updates.cpp:277-294`.
HUD units are DWORD owner identities, with no local pose storage in the HUD
state. Existing `refresh_unit_pose(unit)` and `unit_position(unit,...)` can
eventually use the same owner-to-pose resolver. However, the current algorithm
has an evidenced identity/order discrepancy that an adapter alone cannot fix.

Native `6437B4..6437D8` refreshes each squad payload from node +8h **first**,
then reloads actual controlled owner E188D8 and refreshes it **per member**.
Its position is read from that controlled owner. Current source instead
refreshes and snapshots `state.self_marker_unit` once before the loop, then
refreshes each mate. That marker identity came from 927880 at 6436B7; 927880
calls the owner's vslot114 and then another object's vslot18, or returns null.
It is not an identity getter proven equivalent to the controlled owner.

A repair packet must restore a current-controlled-owner lookup after each mate
refresh, keep the two identities distinct, and read both actual poses in native
order. The squad list, marker operations and GUI lifetimes remain host services.
The source's radius arithmetic is also a typed numerical projection; this audit
does not prove its complete native x87 equivalence. Address 6435D0 and both HUD
source/header files were unleased. The second HUD minimap consumer is outside
this five-consumer audit.

## 5. Positional voice admission — 005BBDC0

Source: `voice_playback.hpp:123-127,150-153`, `voice_playback.cpp:164-180`.
The function already receives the actual entity identity as `void*`. Its pose
is not copied into a voice state. `5BBDEB..5BBE00` reads camera/transform from
current game +19FCh and refreshes B6DB70 as required; it snapshots camera
+120/+124/+128. `5BBE29..5BBE3D` captures the entity argument, checks C8 and
refreshes 414DB0; the entity position is +FC/+100/+104. These two distinct
representations map directly to a live CameraTransform lookup and existing
PoseRefreshResolver, followed by the already recovered 42B2F0 vector length.

The host's double distance need not lose the native return precision:
42B2F0 spills its result to float before reloading ST0, so widening that returned
float to double is exact. The existing header's general extended-precision caveat
is overstated for that returned distance alone; it does not prove the separate
subtraction/attenuation expressions equivalent in every FPU environment.

A small voice spatial adapter can provide actual camera/entity positions and
the canonical length, then replace these three spatial host operations while
retaining the unrelated line/audio/subtitle services. Proposed address 5BBDC0
and new `voice_spatial.hpp/.cpp`/doc/report names were free, but
`include/bsp/voice_playback.hpp` was held by
`agent/orch4-20260910:orch4_dialog_owner_e` until 2026-09-11T15:16 UTC;
coordinate or wait for release before assigning that source-level integration.

## Audit boundary and lease snapshot

Lease checks were taken at 2026-09-11T07:37 UTC and can change. All five consumer
addresses were unleased; all their listed existing source/header files were
unleased except the voice header above. Both small anchor wrappers were also
unleased. Recheck and claim exact implementation scope before editing.

Ghidra analysis remained read-only in the verified `bsp` project/program. One
initial raw decode began inside an instruction at 956793; its leading output
was discarded, and the fallback evidence was reread from valid start 9567F0.
No consumer source, shared metadata or analysis state was changed. No build,
test or game run was needed or performed for these two evidence documents.
