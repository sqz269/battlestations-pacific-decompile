# `Hidden`, and the host for `GenerateObject` (`00944FD0`)

Packet `cc8_lua_generate_object`. Two commits: the scene pass holds back the entities the script is
supposed to create, and `GenerateObject` creates them.

## 1. `Hidden` is the hold-back, and it sits before the gate

The discriminator was an authored diff, not a class default. Two `MotherShipGen` entities in
`usn_19_coralus.scn`, same class, same templates `(Common, GameUnit, Ship, Command,
MotherShipPlanes)`, same `"MultiType" { }` block:

| entity | the line that differs |
| --- | --- |
| `Lexington-class01` | `--Hidden = B true ;` — commented out |
| `Zuikaku-class01` | `Hidden = B true ;` — active |

`"Hidden"` lives at `00CE5708` and has **five** references in exactly two functions,
`0046CF40 BSP_SceneFile_ReadEntityBlock` (2) and `0046BF70 BSP_SceneEntity_RegisterMultiplayerStock`
(3). The instantiate path is:

```
0046d39d: CMP byte ptr [EDI + 0x4],0x0   ; the pass flag
0046d3b3: JZ  0x0046d3cb                 ; clear -> no Hidden test
0046d3b5: PUSH 0xce5708                  ; "Hidden"
0046d3bc: CALL 0x008f2260                ; bag.find
0046d3c1: CMP byte ptr [EAX + 0xc],0x0
0046d3c5: JNZ 0x0046d5e4                 ; SET -> jumps past the creation
...
0046d426: CALL 0x0046c550                ; never reached for a hidden one
```

So a hidden entity is **registered and not created**. The registration branch still runs `0046BF70`
at `0046D531`, which reads the same string three more times, so the record stays in the scene
database's named-object map and `GenerateObject` instantiates it from there by name. That is what
stops the script's own spawn step producing a second carrier.

**It also settles a thread that went the wrong way twice.** The test is *before* `0046C550`, not
inside it, so the gate never sees a held-back entity. This host's `rejected=0` was a faithful answer
to the wrong question, and the class default for `GenerateInGame` — the previous suspect — needs no
change. `register_multiplayer_stock`'s 106 calls come from the registration branch and are
consistent with that.

### The set, checked against the scripts

| scene | entities | hidden | the names the packet named |
| --- | --- | --- | --- |
| `usn_19_coralus.scn` | 58 | **34** | `Zuikaku-class01`, `Shokaku-class01`, and all seven `luaMoveToPh3` spawns |
| `usn_1_marshall.scn` | 147 | **15** | `ScoutDauntless` |
| `usn_ormoc.scn` | 280 | **29** | `TBM_1` |

USN04's 34 are every Japanese unit plus the movie and dummy props; the Americans are exactly the
ones carrying `--Hidden`.

## 2. The pool

The executable looks a name up in the scene database's named-object map at `sceneDb+18h`
(`0046D96F`). This process has no scene database, so the instantiate pass publishes what it held back
into one process-wide `SceneSpawnPool` keyed by name — the same shape `bsp::air_ops_decks()` already
uses for the decks. An entry survives being spawned, carrying the entity id, for the reason in
section 4.

## 3. The binding

`docs/LUA_BINDING_GENERATE_OBJECT.md` had already recovered the argument shape; this implements it.

| index | type | role |
| --- | --- | --- |
| 0 | string | the authored name, the key `0046D96F` looks up |
| 1 | string | a second name, forwarded; or |
| 1 or 2 | table of three numbers | the world position |
| 2 or 3 | number | the yaw, radians |

* A name the pool does not hold creates nothing and returns nothing, which is `0046D9AF`'s answer.
* A position rewrites only the translation row, as `0046DD48` copies its three floats into the local
  frame's `+30h`/`+34h`/`+38h` and leaves the authored rotation alone.
* The yaw applies only when it is **smaller** than the double `2*pi` at `00CE3828` (`0046DD4C`). The
  `10.0f` default at `00CE38B8` therefore needs no special case at all: `10.0 > 2*pi`, so the
  sentinel fails the same comparison, which is precisely what "keep the authored orientation" means.
  Writing it as a comparison rather than an equality test is the faithful reading.
* Creation runs through the orders host, which owns the units host, on the same `descriptor[1]` path
  `0046DB4B CALL EAX` runs for a load-time entity.
* Of `0046DBE8`'s `00925F20 BSP_SEntity_InitAll`, the part this host can do is the `thisTable` slot
  that every entity reaching virtual slot 39 carries. Without it the script's own variable resolves
  to nothing, which is the whole defect. The binding pushes that slot, which is what the
  entity-returning tail `0089903C` pushes — and the script indexes the result (`Mission.Zuikaku.Dead`),
  so a table is required here, unlike the `squadron` key, which is a number the script converts.

## 4. One labelled deviation

`0046D930` has **no already-created arm**: it instantiates on every call. This process keeps the pool
entry and answers a second call with the same entity. A mission that asks twice would otherwise get
two carriers, and a duplicated capital ship is a worse failure than a repeated handle. It is a
deviation, not a reading, and it is the only one in this packet.

## 5. The sibling bindings, measured

Counted in the scripts rather than assumed:

| script | `GenerateObject` | `SpawnNew` | others |
| --- | --- | --- | --- |
| `usn_1_marshall.lua` (USN01) | 17 | — | none |
| `usn_ormoc.lua` (USN22) | 27 | 2 | none |
| `usn_19_coralus.lua` (USN04) | 34 | 14 | none |

`Spawn`, `SpawnNewIDIsRequested`, `SpawnNewIDRemove` and `SpawnLight` are called by none of the
three. **USN01 needs `GenerateObject` alone**, which makes it the clean first validation.
`SpawnNew` `0094C480` is a different mechanism — it builds a group from class types out of a Lua
table (`party`, `groupMembers` with `Type`, `Name`, `Crew`, `Race`, `WingCount`, `Equipment`), and
USN04 uses it for bomber waves including `Mission.TypeB5N`, the Kate. It is a separate packet and a
second, independent route to a torpedo aircraft.

## Uncertainty

* `0046BF70`'s three `Hidden` reads on the registration branch were not decoded; this packet takes
  only that the record survives, which the `GenerateObject` lookup requires.
* `00925F20 BSP_SEntity_InitAll` is reconstructed only as the `thisTable` slot.
* Whether a held-back entity should also be absent from the world lists and the gun census until it
  is spawned follows from not creating it, but the blast radius is large and is what the before/after
  pair below measures.

## Host methods

| method | address | coverage |
| --- | --- | --- |
| the `Hidden` hold-back | 0046D3C5 | complete for the instantiate pass |
| `GameMissionLuaHost::run_generate_object_00944fd0` | 00944FD0 | arguments, lookup, placement, yaw, table slot; one deviation in section 4 |
| `GameScriptOrdersHost::create_unit_from_scene_record_0046db4b` | 0046DB4B | the creator call, through the units host |

## no_ghidra_function

None.

## Measured

Two missions, and **both columns of each pair sit after `0daec4b56` and `bb9123bc5`**, so neither is
comparable to the three USN04 columns in `docs/USN04_STRIKE_CLASS.md` on the command-target or
AI-target-weight axes. The before column is `main` (which does not carry the hold-back); the after is
this branch. The mission-length before column was deliberately **not** measured: every mission-level
number re-baselines when the script-spawned side stops existing at load, so a long before run buys a
lock slot's worth of numbers that only say "the fleet was there".

### The census, which is the whole point of the hold-back

| | USN04 before | USN04 after | USN01 before | USN01 after |
| --- | --- | --- | --- | --- |
| registered / instantiated | 58 / 58 | 58 / 58 | 147 / 147 | 147 / 147 |
| generated / rejected | 58 / 0 | **24 / 34** | 147 / 0 | **132 / 15** |
| created | 53 | **19** | 77 | **62** |
| held back by `Hidden` | — | **34** | — | **15** |

**34 and 15 are exactly the counts the scene scan predicted**, which is the prediction landing on the
nose in both missions. The blast radius is as large as promised: USN04's air-ops decks fall 6 to 2,
its units with guns 57 to 23 and its torpedo-ordnance owners 34 to 11; USN01 keeps `Airfield2` and
`Enterprise` and falls 41 to fewer units with guns.

### `GenerateObject` was not called in either run

It has **no row in the native call table** of either log. USN01 ended with `MissionPhase=1`, and
`ScoutDauntless` is created at `luaMoveToPh2:707`, so the phase-2 trigger was never reached in 7200
frames. USN04's `luaMoveToPh3` needs phase 2.5 and was not reached either, while `SpawnNew` fired
8 times there.

So the binding is implemented and unexercised, and the honest statement is that **this packet's
`GenerateObject` has not been validated by a run**. What the runs did validate is the hold-back, in
both missions, against a predicted count.

### What USN01 did instead, and it is the stream's best result so far

With the Japanese side of USN04 absent this is the mission that still has aircraft, and for the first
time the torpedo path runs:

```
torpedo task 009D4E30 kind Eh installed for an ordered aircraft   x5   (Mav1..Mav5, flight_lead=1 then 0)
009D4A70 sets the engage distance task+484h=2200.0 (Pilot/Torpedo/AttackDist)
summary mission pilot attack: ordered=5 range_first_mean=4174.3 m range_last_mean=3858.8 m closed_mean=315.6 m
torpedo Mav1 approach 009D3420: ticks=120 no_target=0 replans=13 aim_ticks=0 | range min=3978.9 last=3978.9
torpedo Mav2 ... min=3708.4 last=3708.4     torpedo Mav3 ... min=3633.5 last=3633.5
```

Five kind Eh tasks, which this stream had never seen built. The aircraft start at 4174.3 m against an
engage threshold of 2200 x 2.2 = 4840 m, so they begin **inside** it — USN01 remains the wrong
geometry for the move-to, exactly as `docs/TORPEDO_MISSION_SURVEY.md` predicted, though 4174.3 m is
far outside the 1490.8 m that survey measured.

And they do not close: `min == last` on every one, `aim_ticks=0`, and the order issue gate never
opens:

```
torpedo Mav5 issue stage 007CE9FD: stage_ticks=7200 guard_blocked=0 waiting=7200 issues=0
torpedo Mav5 issue gate 007EEF40: ctl+390h=0.0000 ctl+374h=0.0000 open=0
torpedo Mav5 aim tick 009D15F0: aim_complete_2Ch=0 first_true_at_aim_tick=-1
torpedo Mav5 prepare window: first_prepare_at_arm_tick=1 first_blocked_no_order_at_arm_tick=1 coincide=yes
```

`blocked_0099af53=121`, no drops, no swims, no shots. That is a precise next address for whoever owns
the torpedo task: `007EEF40`'s two gate inputs are both zero from the first tick.

### A defect the USN01 log exposed, fixed after the run

`ScoutDauntless` appears in the run exactly once, and not as a unit:

```
scene marker ScoutDauntless class=PlaneSquadronGen id=126 findable=1 attach=007f4580 pos=(-200.0,700.0,0.0)
```

The marker rule is `if (!entity.generated || entity.created) continue;` — a marker is an entity the
instantiate pass **took** but whose creator is a record. A held-back entity was leaving the gate's
`generated = true` in its record while `created` stayed false, so **every one of them became a scene
marker with its own `thisTable` slot**. USN01 built 81 markers and USN04 6.

That is wrong twice over. `FindEntity` would answer for a unit that does not exist, and when
`GenerateObject` later created the real one it would add a **second** slot under a different id for
the same name — the duplicate the whole hold-back exists to prevent, reintroduced by the back door.

The fix is one line and it is the faithful reading: a hidden entity is skipped at `0046D3C5`
**before** `0046C550` is called, so it has no gate answer and is on no pending list. `generated` is
now false for a held-back record. The counts above were measured before this fix and the census
columns are unaffected by it, because `generated` there is the gate's own tally, not the record's.

### One caveat on the USN01 run

It exited **1** while running its full frame budget: `frames_presented=5642 presents_skipped=1757`
sums to 7399, `loop_finished=1`, every summary printed and the shutdown sequence completed cleanly to
`native renderer final COM release`. Nothing in the log names a cause and I have not established one,
so the data is reported as it stands with the non-zero exit noted rather than explained away. The
USN04 run and the USN01 before run both exited 0.

## Validation

Build clean, both ctest suites pass. The before/after run pair is below; **both columns sit after
`0daec4b56` and `bb9123bc5`**, so neither is comparable to the three USN04 columns in
`docs/USN04_STRIKE_CLASS.md` on the command-target or AI-target-weight axes.

## Correction: a held-back `PlaneSquadronGen` row now spawns its whole wing

Packet `cc8_plane_squadron_host`. This is an addition to section 3's binding, not a change to any
reading in this document.

`create_unit_from_scene_record_0046db4b` is the third of the three seams that reach
`004F0AD0 BSP_SceneUnit_CreatePlaneSquadronGen`, after the authored scene row and the air-ops launch
`006C5050`. Since that creator's slot-39 attach `007F4580` is what spawns a squadron's `WingCount`
planes, a squadron created here has to spawn them too, or a script-spawned squadron would be one
aircraft where an authored one is three.

* **was**: the binding created one unit per held-back record, whatever its class.
* **is**: a record of class `18h` also runs `plane_squadron_plan_members_007f4580` and its wing
  records go into the same `create_units` batch, exactly as the other two seams do. The squadron
  registers in `bsp::plane_squadron_registry()` with its `+3D0h` array, its `+3CCh` and the `+9D8h`
  spawn stamps.
* **the `WingCount` problem, and how it is solved**: `007F4580` reads the key out of the entity's
  property bag, and the bag does not survive the hold-back - `SceneSpawnPoolEntry` keeps the
  `GameSceneEntityRecord` and not the block it was read from. So the key is read at hold-back time
  and carried on the pool entry, which is the same thing this document's section 2 already does for
  a held-back carrier's air-ops deck and for the same reason. An absent key gets the code default of
  3 that `007F473A` stores.

**UNVALIDATED BY A RUN, and deliberately recorded as such.** Section "GenerateObject was not called
in either run" still holds: neither USN01 nor USN04 reaches a call in its frame budget, so no run
has exercised this arm. What *is* measured is that both missions hold rows back that would take it -
USN01 holds back 15 of its 20 `PlaneSquadronGen` rows, and `moviefisher` is USN04's one of two. The
seam is written to the same rule as the two the runs do exercise, and to nothing else.

`docs/PLANE_SQUADRON_HOST.md` carries the design, the other two seams and the fused-leader
substitution that makes the squadron's own unit wing 0.
