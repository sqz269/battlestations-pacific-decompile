# The squadron in this host: real member planes on both creation seams

Packet `cc8_plane_squadron_host`. Addresses: `007F4580` (the slot-39 attach whose mode-1 loop makes
the planes), `007F4B43`..`007F4B6E` (its tail), `004F0AD0` (the scene creator), `006C5050` (the
air-ops launch bag that reaches the same creator), `007ECF80` (the member fan-out), `007EEF30` /
`007EE7F0` / `007EEF40` (the release-order gate this stream arrived from).

Files: `include/bsp/plane_squadron_host.hpp`, `src/plane_squadron_host.cpp`,
`src/game_hosts_scene_contents.cpp`, `src/game_hosts_script_orders.cpp`.

The listing evidence for the loop is the re-reading section of `docs/PLANE_SQUADRON.md`; the object
and its AI-facing fields are `docs/PLANE_SQUADRON_ENTITY.md`. The rule this host drives already
existed and is not restated: `plane_squadron_spawn_planes_007f4580` in
`include/bsp/plane_squadron.hpp` (the loop) and `plane_squadron_attach_plane_007f4b43` in
`include/bsp/plane_squadron_entity.hpp` (the tail). What this packet adds is the **binding**: a
`PlaneSquadronSpawnHost` whose "plane instance" is a scene entity record, because in this process a
plane becomes a unit only when `create_units` runs over the whole record list.

## 1. The substitution, stated once

The native's squadron is a separate `0x414` container that owns `WingCount` planes and is itself no
aircraft. **This host fuses the container with its own flight leader.** The unit a
`PlaneSquadronGen` row makes - the one carrying the authored name the mission script orders by - is
wing 0, `members[0]`, and the packet creates wings 1..`WingCount-1` beside it.

That is the substitution `docs/PLANE_SQUADRON_ENTITY.md` section 5 already made ("a scene aircraft
entity **is** a `PlaneSquadronGen`, so the unit the loader made is the squadron's first wing"); what
was missing, and what this packet supplies, is the rest of the wing. Keeping the fusion rather than
building a separate container entity is deliberate: a container that is not a unit has no entity id,
so `FindEntity` could not answer for it, `PilotSetTarget` could not address it and the air-ops
`squadron` key would have nothing to be. Every one of those keys off the unit index in this process.

One consequence is visible and is **not** hidden: the native names wing 0 `<squadron>|.-1` and this
leaves it the authored name. `007F3820 BSP_PlaneSquadron_SetName` cuts a member name at the first
`|` (`_strcspn` at `007F38AF`) and re-prefixes it, which for a name with no separator yields the
squadron name unchanged, so the fused leader carries the name that routine would give it. That is a
consistency, not a proof.

`src/game_hosts_units.cpp` is leased to another worker for the length of this packet, so the
container override that would let the squadron be its own non-flying slot was not available and is
not attempted. The fusion is the design, not a placeholder for it.

## 2. The two seams, and that they converge

| Seam | Where | What it does now |
| --- | --- | --- |
| authored scene row | `queue_plane_squadron_wing_007f4580` in `src/game_hosts_scene_contents.cpp` | a created class-`18h` row reads `Type`, `WingCount`, `PlaneParentID` and `Behaviour` out of its own bag, runs the loop, and stages one entity record per extra wing; the records are flushed into the scene's record list after the census loops, so a member plane never enters the scene class tallies |
| air-ops launch | `create_air_ops_squadron_006c5050` in `src/game_hosts_script_orders.cpp` | `006C5050`'s bag already carried `WingCount`; the squadron record and its wing records are now built as one batch and handed to `create_units` in a single call |

Both call `plane_squadron_plan_members_007f4580`, so a launched squadron and an authored one have
the same shape, the same naming and the same array order. The squadron table
(`bsp::plane_squadron_registry()`) holds `+3C8h`, `+364h`, the `+3D0h` array in array order and the
`+9D8h` spawn stamp per member; `+3CCh` is the count of members whose record became a unit.

`GameScriptOrdersHost::resolve_plane_squadron_members` fills the array with unit indices by name.
It runs from the order path rather than from `create_units`, because `game_hosts_mission_frame.cpp`
owns that call site and is not this packet's to edit.

## 3. Host table

One row per native call site. `contract: unread` is stated where it applies.

| Site | In | Host method | this / args | ret |
| --- | --- | --- | --- | --- |
| `007F45A7` | `007F4580` | `MemberPlanSpawnHost::attach_lua_self_base_0077e830` | squadron | void - counted, not performed: the Lua self table is `attach_created_entity_00928a00`'s and is not per-wing |
| `007F4724`/`007F4742`/`007F4759`/`007F479D`/`007F47C7` | `007F4580` | `read_spawn_properties_008f2260` | bag; the four key literals | the four properties, read by the caller out of the `ScenePropertyBlock` |
| `007F477E`, `007F47E1` | `007F4580` | `vehicle_class_for_type_007b8a80` | classId | the class id stands for the class; a zero id resolves to nothing |
| `007F47AE` | `007F4580` | `resolve_parent_entity_00521e30` | `PlaneParentID` | **0**: no entity handle table exists at scene-load time, so every wing takes the no-parent arm `007F48BE`. `contract: unread` |
| `007F4811` | `007F4580` | `create_plane_instance` | factory; `0` | a member plan, which becomes a unit when `create_units` runs |
| `007F48BA`/`007F48D2` | `007F4580` | `place_plane_in_world` | plane; parent, world node, matrix | records which arm was taken; the frame itself is the squadron's, which is what `007F48C9 LEA ECX,[ESI+74h]` passes |
| `007F48D6`/`007F48FD` | `007F4580` | `clone_spawn_descriptor_00922de0` | `0Ch` block; `squadron+C0h` | the same bag is carried to every member record, which is the sharing the clone produces |
| `007F492F`/`007F49F8`/`007F4A32`/`007F4A47` | `007F4580` | `set_plane_name` | squadron name; suffix | `<squadron>|` or `<squadron>|.-<i+1>`, from `squadron_plane_name_suffix_007f4926`. `007F4926` and `007F49EF` are the PUSHes of the two suffix literals; the calls are the addresses in this row |
| `007F4B43`..`007F4B6E` | `007F4580` | `plane_squadron_attach_plane_007f4b43` | squadron; plane | the spawn stamp, the array slot, `+3CCh` and `+3ECh` |
| `007ECF80` | vtable `+128h` | the fan-out in `run_pilot_set_target` | squadron; the chosen command | one `entity_issue_command` and one `0099A170` install per wingman |

## 4. Corrections to the handoff this packet was given

The handoff at the end of `docs/PLANE_SQUADRON.md` was written from the torpedo side. Three of its
statements do not survive a run of this host, and one of its projections does not survive the
authored data. They are corrected here rather than quietly worked around.

### 4.1 `+3CCh` was never 0 in this host, and the gate is never reached at all

* **was**: "This host makes one plane per scene row and one plane per carrier launch with no
  squadron object, so `+3CCh` is always 0, the gate never opens".
* **is**: `TorpedoReleaseOrderBinding::controlled_unit_count` in `src/game_hosts_units.cpp` counts
  every slot whose ordnance mask carries torpedo `2Bh` - a labelled substitution for `+3CCh` that
  it states in its own comment - so for USN01 it answers **5**, not 0. The gate would pass its count
  test. It is never asked: `read_issue_inputs` is only called from
  `torpedo_issue_release_orders_007c0d90`, which the issue stage only calls when `unit+C20h > 0`,
  and the USN01 before column reads `requests_007BBBA0=0 C20h_left=0`. Both operands the run prints
  as `ctl+390h=0.0000 ctl+374h=0.0000 open=0` are **unset fields, not measurements**.
* **evidence**: the before column below; `src/game_hosts_units.cpp:3598` (`is_flight_member`) and
  `src/torpedo_issue_timing.cpp` (`issue_requests_c20 > 0` is what selects the issue arm).

### 4.2 What actually stops the first torpedo run-in is the approach, not the member array

`unit+C20h` is raised in exactly one place in this host, `release_ordnance_007bbba0`, and its three
callers are the device release path and the torpedo task's release timer `009FA3A0`, which
`009D2287` arms **after** the aim stage. The USN01 before column reads `aim_ticks=0` on all five
Mavs with `min == last` on every approach, so no aim stage ran, no release timer was armed, nothing
dropped, `C20h` stayed 0 and the release-order broadcast was never entered.

In the image the same shape holds: `007BBC00`'s callers are `007BBBA0`'s callers. The release-order
broadcast is what a plane that has just dropped tells its flight-mates; it is not what starts a
torpedo run. So **a squadron with real members is necessary for `007EEF30` to have anyone to issue
to, and is not sufficient to open the gate**. This packet does not claim the first torpedo run-in,
and the blocker belongs to whoever owns the approach `009D3420`.

### 4.3 USN01's aircraft census does not move, because its five squadrons are one-wing

* **was**: "Expect every aircraft census to move by about the wing count", from the `universe/`-wide
  histogram whose mode is 3.
* **is**: USN01 creates exactly the five `PlaneSquadronGen` rows that author
  `WingCount = E PlaneWingCount :" 1"`. `universe/Scenes/missions/USN/usn_1_marshall.scn` carries 20
  such rows with the histogram wing 1 x5, 2 x1, 3 x10, 4 x3, 5 x1, and the five one-wing rows are
  `Mav1`..`Mav5` at lines 10490, 10511, 10532, 10553 and 10574; the other 15 are the ones the
  generate gate rejects. So USN01's unit count is unchanged by this packet and its five Mavs stay
  five aircraft.
* **evidence**: the scene file lines, and the before/after columns below, which agree.

USN04 is where the multiplier is real: its created scene squadron and its four air-ops launches all
carry `WingCount` 3.

### 4.4 USN01 exits 0

The handoff asks for `EXITCODE` and the last twenty lines "if USN01 exits with code 1 after a
complete run". Both USN01 runs of this packet, before and after, ended `EXITCODE=0` with
`loop_finished=1 exit_code=0`. The earlier exit 1 did not reproduce; nothing here explains it, and
it is left recorded rather than closed.

### 4.5 `+378h` is seeded set, and the binding passes it clear

`007F2D1E MOV byte [ESI+378h],1` (evidence in `docs/PLANE_SQUADRON.md`) seeds the force flag that
`007EEF62 CMP byte [ESI+378h],0 / 007EEF6B JNZ` tests, so a squadron that has not been through
`007ED3C0` raises a release order for **every** member without consulting `007B8AD0`.
`TorpedoReleaseOrderBinding::read_issue_inputs` passes `in.force_flag_378 = false`, which is the
opposite of the constructed state and makes the host's issue path stricter than the native's. The
fix belongs in `src/game_hosts_units.cpp` and that file is leased elsewhere; it is recorded here as
an open item with its address rather than applied.

## 5. Runs

Every column is from one binary. The before columns are `b00c3cd24` (this worktree's base, with the
three same-day baseline moves `0daec4b56`, `bb9123bc5` and `67e8ac821`+`6b0a12422` all already in
it); the after columns are this packet's tree.

_(filled in below)_

## 6. Open, by address

* `squadron+390h` has no located producer. `docs/PLANE_SQUADRON.md` bounds the negative.
* `+378h`: `src/game_hosts_units.cpp`, `TorpedoReleaseOrderBinding::read_issue_inputs`, must pass
  the constructed `1` rather than `false` (`007F2D1E`).
* `+3CCh` / `+3D0h` in the same binding: `is_flight_member` must become "this unit's own squadron",
  read from `bsp::plane_squadron_registry()`, rather than "carries torpedo ordnance".
* `007ED0D0 BSP_PlaneSquadron_InsertPlaneSorted` and `007D5D20 BSP_Plane_ReadPropertyBag` are two
  further producers of `plane+9D4h`, both `contract: unread`, so membership in this host is the
  spawn tail's only.
* `00521E30`: `PlaneParentID` is never resolved, so an authored parent's placement arm
  (`007F48AE`..`007F48BA`, the identity matrix under the parent) is never taken.
* `GenerateObject 00944FD0`: a `PlaneSquadronGen` row held back by `Hidden` and spawned by the
  script reaches `create_unit_from_scene_record_0046db4b`, which this packet did not extend, so such
  a squadron would still be one plane. Neither USN01 nor USN04 reaches a `GenerateObject` call in
  its frame budget, so nothing measured this.
