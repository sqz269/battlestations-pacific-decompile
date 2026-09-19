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

## 2. The seams, and that they converge

| Seam | Where | What it does now |
| --- | --- | --- |
| authored scene row | `queue_plane_squadron_wing_007f4580` in `src/game_hosts_scene_contents.cpp` | a created class-`18h` row reads `Type`, `WingCount`, `PlaneParentID` and `Behaviour` out of its own bag, runs the loop, and stages one entity record per extra wing; the records are flushed into the scene's record list after the census loops, so a member plane never enters the scene class tallies |
| air-ops launch | `create_air_ops_squadron_006c5050` in `src/game_hosts_script_orders.cpp` | `006C5050`'s bag already carried `WingCount`; the squadron record and its wing records are now built as one batch and handed to `create_units` in a single call |
| `GenerateObject` on a held-back row | `create_unit_from_scene_record_0046db4b` in the same file | the same plan, with `WingCount` read at hold-back time and carried on the spawn-pool entry. **Unvalidated by a run** - see section 6 |

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
| `007F492F`/`007F49F8`/`007F4A32`/`007F4A47` | `007F4580` | `set_plane_name` | squadron name; suffix | `<squadron>` plus the separator, or `<squadron>` plus the separator, `.-` and `<i+1>`, from `squadron_plane_name_suffix_007f4926`. `007F4926` and `007F49EF` are the PUSHes of the two suffix literals; the calls are the addresses in this row |
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
* **evidence**: the before column below; `TorpedoReleaseOrderBinding::is_flight_member` in
  `src/game_hosts_units.cpp` and `plane_release_issue_stage_007ce9fd` in
  `src/torpedo_issue_timing.cpp`, where `issue_requests_c20 > 0` is what selects the issue arm.

### 4.2 What actually stops the first torpedo run-in is the approach, not the member array

`unit+C20h` is raised in exactly one place in this host, `release_ordnance_007bbba0`, and it has
three callers there:

| caller | what it needs | what the run says |
| --- | --- | --- |
| the torpedo task's release timer `009FA3A0` | `009D2287` arms it **after** the aim stage | `aim_ticks=0` on all five Mavs, so it was never armed |
| `request_ordnance_release` (the device path, `009D4956`) | `manual_release_requested` at `009D4923`, which this host hard-codes to `false` and labels `contract` | never true |
| the dive-bomb release | not a torpedo aircraft's path | — |

So no aim stage ran, no release timer was armed, nothing dropped, `C20h` stayed 0 and the
release-order broadcast was never entered.

The same shape holds in the image, and it is exhaustive rather than inferred. Rel32 callsite
censuses over `.text` (`python tools/callsite_census.py`):

| routine | callsites |
| --- | --- |
| `007BBC00` | **0** - it is not a function. It is the store `ADD dword [ECX+C20h],EBX` inside `007BBBA0`'s body, past every early exit |
| `007BBBA0` | 34, every one of them a release decision: `009FA3D0` in `BSP_ReleaseTimer_Tick`, `009D4956` in `BSP_BotTaskTorpedo_TickArm`, `009D26F8`/`009D2938`/`009D29CB` in the torpedo prepare states, `009C5777`/`009C60F1`/`009C88C4` in the dive-bomb states, and so on |
| `007C0D90` | **1**: `007CEA8D` in `007CE040 BSP_PlaneTickElement_FixedStep`, the issue stage, which reaches it only when `unit+C20h > 0` |
| `007EEF30` | **1**: `007C0F01` in `007C0D90` |

So in the image too the release-order broadcast is unreachable until some aircraft has already
requested a release. It is what a plane that has just dropped tells its flight-mates; it is not what
starts a torpedo run. **A squadron with real members is necessary for `007EEF30` to have anyone to
issue to, and is not sufficient to open the gate.**

The USN01 rows say the same from the other end: `orders: ... arm_offers=0 blocked_0099af53=124` and
`prepare window: first_prepare_at_arm_tick=-1 first_blocked_no_order_at_arm_tick=1`. Each Mav's own
prepare stage is waiting for a release order, and with a one-wing squadron there is no flight-mate
to have dropped first. This packet does not claim the first torpedo run-in; the blocker is the
approach `009D3420` (which never reaches the aim stage, so `009D2287` never arms the release timer)
and the unread `009D4923`, and both belong to other streams.

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

### 4.6 Where section 4.2's chain actually stops: two different thresholds

Recorded here because the handoff conflated them, and whoever takes the approach blocker will meet
both. USN01's Mavs are **engaged but not in range**.

| test | routine | threshold | USN01 |
| --- | --- | --- | --- |
| the task-level engaged predicate | `009D3210 BSP_BotTaskTorpedo_IsEngaged` | `task+484h * 2.2` at `009D324F` = **4840 m** | satisfied: `range_first_mean = 4174.3 m` is inside it |
| the approach's in-range latch `+131h`, the attackrun-to-aim flag | `009D3420`, `009D35D4`..`009D361E` | `range < approach+8Ch`, and the run prints `engage 8Ch=2200.0` | **never satisfied**: `min = 3928.5`, `last = 3928.5` |

So the aim tick is not refusing an aircraft that reached it; the aircraft never reaches it, because
the latch wants 2200 m and the approach never closes below 3928.5 m with `replans=13`. The question
for that packet is why an ordered aircraft inside the engaged radius does not close, which is a
move-to and steering question rather than an aim one. `docs/TORPEDO_APPROACH_UPDATE.md` section on
the latch, and `docs/TORPEDO_ENGAGED_TEST.md` for the 2.2, carry both readings.

One thing that packet gains from this one: `009D3624`'s branch measures `00414C60` between the
target point and **`ctl->+3D0h`'s position** - the squadron's flight leader, which this host now
really has. That branch only ever clears the latch and only runs when the aircraft has no torpedoes
left, so it is not USN01's blocker; it is named because it is a second reader of the member array
and a host that stood `+3D0h` in would have had to substitute for it.

And one asymmetry in the same USN01 rows that is worth checking before anything else, because it
would reframe the question: the plane's own issue stage reports `stage_ticks=3000` while the torpedo
task reports `arm_ticks=124 transitions=1 states[attackrun=124]` and `approach ticks=124`. The tick
element ran for the whole mission and the task's arm ran for about six seconds of it. If that is
real rather than a sampling artefact of the census, then "the approach never closes" is a
consequence of the task not being ticked, and the first thing to establish is what stops it at 124.

## 5. Runs

Every column is from one binary. The before columns are `b00c3cd24` (this worktree's base, with the
three same-day baseline moves `0daec4b56`, `bb9123bc5` and `67e8ac821`+`6b0a12422` all already in
it); the after columns are this packet's tree.

### USN01, `--frames 3200 --press-start-frame 30 --mission-frames 3000 --mission-frame-seconds 0.05`

`local/sqn_before_usn01.log`, `local/sqn_after_usn01.log`. `query session` was `console Active` before
both runs.

| | before | after |
| --- | --- | --- |
| `PlaneSquadronGen` scene rows seen / created | 20 / 5 | 20 / 5 |
| authored `WingCount` of the five created | `" 1"` each | `" 1"` each |
| member aircraft | 5 | 5 |
| world units | 62 | 62 |
| AI squadrons (`build_squadrons`) | 5 over 5 | 5 over 5 |
| `PilotSetTarget` ordered | 5 | 5 |
| fan-out `007ECF80` | - | `+3CCh=1 -> 0 wingman task(s)`, five times |
| Mav1 approach `009D3420` | `ticks=124 replans=13 aim_ticks=0 min=3928.5 last=3928.5` | identical |
| Mav1 `orders` | `arm_offers=0 blocked_0099af53=124 peak_C58h=0` | identical |
| Mav1 issue stage `007CE9FD` | `stage_ticks=3000 waiting=3000 issues=0 requests_007BBBA0=0 C20h_left=0 C28h=-150.005` | identical |
| Mav1 issue gate `007EEF40` | `ctl+390h=0.0000 ctl+374h=0.0000 open=0` (unevaluated) | identical |
| gunnery | `queued_hits=10 total_damage=326.1 first_hit=58.65 s` | identical |
| pilot attack | `ordered=5 range_first_mean=4174.3 closed_mean=368.1` | identical |
| `EXITCODE` | 0 | 0 |
| host methods | 740 concrete | 741 concrete |

**Nothing moves, and that is the expected answer for this mission.** Its five created squadrons are
the five one-wing Mavis rows (section 4.3), so the wing loop makes one plane each and the fused
leader is that plane. The one new host method is `PlaneSquadron::spawn_planes 007f4580`. The
fan-out line is the evidence that the squadron table is live and that `+3CCh` is 1 rather than 0.

### USN04, `--frames 5000 --press-start-frame 30 --mission-frames 4800 --mission-frame-seconds 0.05`

`local/sqn_before_usn04.log`, `local/sqn_after_usn04.log`.

| | before | after |
| --- | --- | --- |
| `PlaneSquadronGen` scene rows seen / created | 2 / 1 (`movieval`) | 2 / 1 |
| authored `WingCount` | 3 | 3 |
| world units after the scene load | 19 | **21** |
| air-ops launches | 4, `wing=3` each | 4, `wing=3` each |
| units created per launch | 1 | **3** |
| world units at the end | 23 | **33** |
| squadrons / member aircraft | 5 / 5 | 5 / **15** |
| AI squadrons (`build_squadrons`) | 5 over 5 | **15 over 15** - the double count of section 6 |
| `PilotSetTarget` ordered | 1 | **3** |
| fan-out `007ECF80` | - | `squadron=movieval +3CCh=3 -> 2 wingman task(s)` |
| pilot attack means | `range_first_mean=11109.9 range_last_mean=1934.1 closed_mean=9175.9` | identical |
| gunnery | `queued_hits=16 total_damage=0.0 first_hit=190.71 s` | identical |
| `EXITCODE` | 0 | 0 |

The unit arithmetic is exact: 18 non-squadron units plus `movieval`'s three wings is 21, and four
launches of three is 33. The count rises by ten, which is the sum of the authored wing counts less
the five stand-ins they replace.

The after column was taken twice, because `air_ops_squadron_plane_count` changed after the first one
to report the real `+3CCh` instead of the authored wing. The second run (`local/sqn_after2_usn04.log`,
on the tree this packet ends with) reproduces every figure above exactly - 21 after the load, 33 at
the end, `+3CCh=3 -> 2 wingman task(s)`, `ordered=3`, `EXITCODE=0` - which is the expected result,
since the two readings differ only once a member dies and none does here.

The three members of `movieval` fly identical attack runs -
`divebomb movieval|.-2` and `movieval|.-3` both report `attackrun 009C4220 ticks=1527 rerolls=153`,
`dive entry alt=650.9 m`, `aim error 009C5C9B=-1689.72 m closest=374.9` - which is why the pilot
attack means are unchanged with three aircraft ordered instead of one. That is the expected result
and not a defect of the spawn: `007F4813` has no per-wing offset, `docs/PLANE_SQUADRON.md` section 4
records that the formation spacing belongs to the pilot bot rather than to the spawn, and this host
has no formation model, so three wings placed on one frame with one order fly one trajectory. The
aircraft are distinct units with distinct names, world registrations and tasks; only their motion
coincides.

### The gate

`007EEF40` did not open in either mission, before or after, and this packet did not expect it to.
Section 4.2 is why: the broadcast is unreachable until an aircraft has already requested a release,
and neither of the two routes into `007BBBA0` fires here. No torpedo run-in, no drop, no water entry
and no breakup or swim was produced by either run, so there is nothing to report under those
headings.

## 6. Open, by address

* **`src/game_hosts_ai.cpp`, `build_squadrons` (around line 2155): the AI layer now double-counts.**
  It seeds one `Squadron` per unit answering `IsKindOf(0Fh)`, and its own comment above
  `unit_owned_by_squadron` says a plane a squadron owns must not be an AI candidate of its own
  because "keeping both in a group double-orders the same aircraft". Now that wingmen exist, every
  wingman becomes its own one-member AI squadron and its own candidate. The fix is to skip a plane
  that is a wingman of a real squadron (`plane_squadron_registry().find_by_member_unit(unit)` whose
  `flight_leader()` is not that unit) and to take the leader's member array and `+3C8h` from the
  registry instead of attaching only itself. That file is leased to `agent/cc8-ai-squadron` for the
  length of this packet; the change was sent to its owner rather than made here.
* **A registry record whose `member_units` is empty is a trap for a consumer, and the contract is
  stated here.** `PlaneSquadronHostRecord::flight_leader()` answers `kPlaneSquadronNoUnit` when no
  member resolved, and `live_count()` answers 0, which is the right reading of `+3D0h`/`+3CCh` for a
  squadron that holds nothing. A consumer that filters "is this unit a wingman?" by comparing
  against `flight_leader()` must guard the empty case first, or it drops **every** plane of that
  record instead of none - a silent unit loss rather than the double-order it was fixing.
  `agent/cc8-ai-squadron` added that guard to `build_squadrons`, and it is the right shape: a
  seatbelt, since `resolve_plane_squadron_members` fills the array before any order path runs. The
  case to look at if it ever fires is a squadron whose planes have all died.
* `squadron+390h` has no located producer. `docs/PLANE_SQUADRON.md` bounds the negative.
* **`src/game_hosts_units.cpp`, `TorpedoReleaseOrderBinding`: the release-order gate still reads the
  stand-in.** The squadron table now holds the real array, so the binding's three substitutions can
  go. The change is bounded and is written out here so it can be applied as one edit when the lease
  frees:
  * `is_flight_member` (torpedo ordnance) and `controlled(int)` / `controlled_unit_count()` (a walk
    over every slot) become a walk over `plane_squadron_registry().find_by_member_unit(index_of_slot(slot_))`'s
    `member_units`, which is `ctl+3D0h` under `ctl+3CCh`. A caller in no squadron answers a count of
    0, which is `007EE891`'s own arm - it stores `+374h = 0`.
  * `in.force_flag_378 = false` becomes that record's `force_flag_378`, which
    `include/bsp/plane_squadron_host.hpp` seeds `true` after `007F2D1E`. Nothing in this host calls
    `007ED3C0`, whose caller is unlocated, so it never clears; that is a labelled divergence and its
    effect is that every member is offered the raise without the `007B8AD0` follow-target test.
  * the armed fraction `007EE7F0` then runs over the squadron's own members rather than over every
    torpedo-armed aircraft in the mission, which is what `+374h` means.
* `007ED0D0 BSP_PlaneSquadron_InsertPlaneSorted` and `007D5D20 BSP_Plane_ReadPropertyBag` are two
  further producers of `plane+9D4h`, both `contract: unread`, so membership in this host is the
  spawn tail's only.
* `00521E30`: `PlaneParentID` is never resolved, so an authored parent's placement arm
  (`007F48AE`..`007F48BA`, the identity matrix under the parent) is never taken.
* `GenerateObject 00944FD0`, the **third seam**, is implemented and **unvalidated by a run**. A
  `PlaneSquadronGen` row held back by `Hidden` reaches `004F0AD0` through
  `create_unit_from_scene_record_0046db4b` rather than at scene load, so its wing is spawned there
  from the same plan. The property bag does not survive the hold-back, so `WingCount` is read at
  hold-back time and carried on the spawn-pool entry, the same way the air-ops deck already is.
  Neither USN01 nor USN04 reaches a `GenerateObject` call in its frame budget, though both have rows
  waiting for one: USN01 holds back 15 of its 20 `PlaneSquadronGen` rows (`moviefisher` is USN04's
  one of two, out of 34 rows it holds back in all). So no run has exercised it. It is written to the
  same rule as the other two seams and measured by neither.
