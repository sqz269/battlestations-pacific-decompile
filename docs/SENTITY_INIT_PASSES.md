# InitAll passes B, C and E per class, against the host (packet `cc9_sentity_init_passes_bce_read`)

Addresses: 00822C20, 0081F980, 007593D0, 00758210, 007F1FE0, 007F4BA0, 007D5D20, 007C9770,
007B38D0, 009277E0, 009295B0, 00926390; read only.

Worker cc9-world-init, 2026-09-27, main a5a19b83e. Docs only, with Ghidra read only. This
extends `docs/CONSTRUCT_WORLD.md` section 17 (the InitAll binding, `kSEntityInitAllBound`
ON), whose passes B, C and E are named records. That doc is leased to cc9-scene-entities right
now, so this is a separate file.

## 1. Method, and what it can and cannot show

- **The call list.** Each pass body's distinct callees were taken in listing order from Ghidra.
  Helpers were dropped: 0041E870, 00419CC0, 00BD1510, operator new/free, and the 008F2260
  property reads.
- **Host coverage.** Each callee was matched against the native table of this tree's two ON
  runs, `local\ss_on_e2.log` and `local\ss_on_usn02.log`. The scripts are
  `local\cc9-world-init-passcensus.py`, and their outputs are
  `local\cc9-world-init-pc-822c20.txt` and `local\cc9-world-init-pc-rest.txt`.
- **What a match proves.** A match means the host ran or recorded that native, somewhere.
- **What "not named" does not prove.** It does not prove the step is missing: the host
  sometimes logs the call site instead of the callee. The ship's pose-ring fill 00818EA0, for
  example, is logged as 00810020 by `create_units`. The table marks those cases from the code.
- **So** this is a map of where to look, not a line-by-line proof. Each "missing" row is a
  **contract: unread** until its owner opens the callee.

## 2. Order: the one difference that holds for every class

- **The image** creates the entity, and InitAll then runs pass A (the `thisTable` attach) over
  the whole pending list, then pass B over the whole list, then pass C, D and E. The +9Ch Lua
  attach therefore precedes every B and C body.
- **The host** runs its B- and C-like work inside `create_units`, `src/game_hosts_units.cpp`.
  Examples are the StartSpeed arm of 00822C20, the pose-ring fill 00810020, the gunnery
  sections of 0081F980 and the carrier's deck load. That work runs **before** the Lua attach,
  which is pass A in `src/game_hosts_lua.cpp` or the load-time `attach_scene_entities_00928a00`.
- **Where the order matters.** Any B or C step that reads the entity's `thisTable` sees nothing
  in the host. Three do:
  - the squadron's `BSP_Unit_BindLuaClass` 009292B0 (007F218E, pass B);
  - the plane's `thisTable.SquadronID` store (007C97E3..007C9805, pass C);
  - the default pass C 009295B0's Lua read (00929601..00929786).

## 3. Per class

Status legend: **host** = run by the host (the file named); **rec** = a named record in the
host; **missing** = not reached, a contract.

### Ships: Battleship, Cruiser, Destroyer, Cargo (USN02's four GenerateObject destroyers included)

**Pass B, 00822C20 `UnitInstance_SEntityInit`** (00822C20..00824B57):

| site | callee | step | status |
| --- | --- | --- | --- |
| 00822CCB | 00822B70 | navigator parameters from tuning | host (`NavigatorParams::reset_from_tuning`) |
| 00822CDB | 00955420 | scene bindings (the `+5Ch` store chain, docs/UNIT_SCENE_FLAGS_LIVE.md) | host, at creation (`native_unit_scene_initialization.cpp`) |
| 00822DCF..008234C0 | 008687C0, 00866CD0, 006FBEB0 (x4) | four named point effects on the hull | missing (render only) |
| 00823501, 00823508 | 00822460, 00818EA0 | sub-objects, the wake/pose ring | host (`create_units`, 00810020 path) |
| 0082356C..008235F7 | 0080FC30, 0080D9B0, 0092D770 | the StartSpeed arm | host (`SceneStartSpeed::*`) |
| 00823766..0082381B | 0081E350, 0074F490, 00711C30, 00814870 | part records and names | missing (contract: unread) |
| 008238C1 | 00927A80 | `Skill` from the property bag | missing |
| 00823A9F | 007214C0 | the weapon director's enable message | missing: gunnery owner |
| 00823BCF..00823F04 | 008346E0, 0080D690, 0081DA30, 004A6830 | unread helpers | missing (contract: unread) |
| 00823F9C..008243D4 | point effects, 0092D730, 008E6430, pose refresh | effect setup and speed scale | partly host (0092D730, 008E6430 are read by motion) |
| 00824A67, 00824ADA | 0074D780, 00442190 | live effect references | missing (render) |

**Pass C, 0081F980 `ShipUnit_BindSectionPoints`** (0081F980..008206EB):
- **The unit's Lua table** (0081FA78..0081FE3C, 004425C0/00BD8E20/00BD6830): immediate
  throttle (0080D9B0) and rudder (0080DA00). The throttle is **host** (StartSpeed). The
  **rudder is missing**.
- **Submarine and torpedo:** the depth level (008528B0), the torpedo stock (0081F8B0) and four
  unread setters (00812310..008124C0). All **missing**. `SetTorpedoStock` bears on torpedo
  gunnery, so it is a **gameplay candidate**.
- **Section points and the pose ring:** 00810020 is **host** (`fill_at_spawn`). The section
  meshes (00723030) are **host** in the gunnery file (`cc9_hull_sections`).
- **Unread:** 0095E5B0, 009238A0, 00815520, 008367F0, 0074EDC0, 00B85200, 0081B7C0,
  008127E0 and 00809BC0.

### Carrier (Mothership)

- **Pass B, 007593D0.** It runs 00822C20 first (as above). Then the deck: 006CADD0
  `AirOps_LoadFromScene` is **host** (`AirOps::load_from_scene`, the scene-contents path at
  creation). The helpers 006C0D20, 006D0A10, 006BA690 and 006BEF30, plus a Lua read, are
  **missing** (contract: unread).
- **Pass C, 00758210.** It runs 0081F980, then sets `+11A8h = 1`, which is **missing**: its
  reader is unread.

### Plane squadron (`PlaneSquadronGen`, USN04's 16 SpawnNew and 4 air-ops squadrons, the Kingfisher row)

**Pass B, 007F1FE0 `PlaneSquadron_BeginOrderSpeed`:**

| site | callee | step | status |
| --- | --- | --- | --- |
| 007F1FFC | 0077F0E0 | activate and register (docs/UNIT_SCENE_FLAGS_LIVE.md) | host (units registrar at creation) |
| 007F2070 | 0084D810 | the command block | missing (contract: unread) |
| 007F20CD | 007F1C00 | home air base | host for air-ops launches only (`air_operations.cpp`) |
| 007F2114 | 007F1D90 | avoid-zone layer selection | missing: **gameplay** (avoid zones steer squadrons; owner cc9-plane-release) |
| 007F211B | 00927A80 | `Skill` | not named in the host's table; docs/PILOT_SKILL_LEVEL.md has the skill chain (contract: check) |
| 007F218E | 009292B0 | `BSP_Unit_BindLuaClass`: needs the `thisTable` from pass A | missing, and **ordered wrongly** if added at creation |
| 007F219E | 00721280 | the command-controller message | missing |

**Pass C, 007F4BA0** (007F4BA0..007F544F, a Ghidra function since 297fcf7fe):
- 0077FAD0; the wing count `+3C8h` from `+3CCh`; `+354h` from the `+3D0h` member's `+170h`
  slot; `+3E4h = 1`; the formation indices 007ED260. The formation indices and wing count are
  **host** (`plane_squadron_host.cpp`, "007F4B55's +3D0h array").
- The property bag at `[+C0h]+8h`: keys 00CF8818, 00CF8820 and 00CE56B8.
- A named entity found through 00925A90 (**rec**, `SceneCommand::find_entity_by_name`).
- The air-ops block 006BCD20 and the home base 007F1C00.
- The formation matrix (0085DC80, **host**).
- The initial command: 0071BE40, 00465080 and **0077D600 `Entity_IssueCommand`** (007F4E9E),
  whose status is **host** elsewhere. Whether the squadron's authored first order is issued
  here or by the host's own authored-command path is a **contract: unread**, and it is a
  **gameplay candidate**.
- A Lua read (007F4F1E..007F504D), observer pairs (006952A0/00694A60, **rec**), and the
  avoid-zone layer by slope (0041DF40, **missing**, gameplay).
- Unread: 007F2920, 007EDA90, 007EF8F0, 00468560, 007F15F0, 007ED6E0, 007F0DC0, 007F1DE0.

### Planes (Fighter, DiveBomber, TorpedoBomber, SmallRecon: USN04's wings, USN02's Kingfisher)

**Pass B, 007D5D20 `Plane_ReadPropertyBag`** (7 KB):
- **Host at creation:** the scene bindings 00955420; the gunnery AI 00864580
  (`Gunnery::pass_construct`); PilotFires 007CD930; the pose commit 0085DC80.
- **Missing, gameplay candidates:**
  - the physics body 00C5D580 and the flight controller setters 007D9E80/007D9EE0;
  - the actuator block 007EABC0 and the neighbours 007E1E20;
  - the firing-gun registration 007C74A0;
  - `Skill` 00927A80, the random draw 00BD2F10 (a consumer of the process-wide generator), and the step record 007CDC70.
  - The host has its own plane motion (`plane_flight.cpp`), so many of these may exist under
    other addresses. Each needs its owner's check.
- **Missing, render only:** the bow-wave effects 007D5890 and the visibility factor 00B6DA70.

**Pass C, 007C9770** (read in `docs/CONSTRUCT_WORLD.md` section 17):
- `thisTable.SquadronID` (**missing**; no USN04 or USN02 script reads it);
- the disable 00922F80 for `+900h` in {0, 1} (**missing**; a plane parked in a hangar);
- 007C5AC0(-1.0) (**missing**, unread);
- 0095E5B0, 00951F80, 007C95A0, 007BC550 (unread).

### Path and Landscape rows

- **Path.** Pass B is 007B38D0 `ScenePathEntity_LoadHolderProperties`, which is **host** at
  creation (`game_hosts_scene_contents.cpp` line 1105, labelled). Pass C is the default
  009295B0.
- **Landscape.** Pass B is the thunk 009277E0 (the `+5Ch`/`+BDh` store, per
  docs/UNIT_SCENE_FLAGS_LIVE.md), **host**. Pass C is 009295B0.
- **009295B0** (009295B0..009297FA) reads the entity's Lua data and sets its **think script
  name** through 0088A330 `Entity_SetThinkScriptName`. That is **missing**. It matters only for
  a scene entity with an authored think function; whether USN04 or USN02 has one is unchecked.

### Projectiles

`BSP_BombProjectile_Construct` 006E2670 reaches the base constructor through 0077EED0 at
006E2694. So every bomb is pushed on the pending list and passes through InitAll in the step it
is released: row 12, or RunExtraFixedStep. This host's bombs are not entities, so none of their
passes run. The bomb's slot bodies are unread. This tree's ON E2 run released 1 dive bomb and 4 torpedoes.

### Pass E, all classes

00926317 destroys the `+C0h` holder, which for scene units is the kind-1 property-bag holder
(00922E20). After InitAll the image cannot read the bag. This host keeps the bag on the scene
record for the whole run. Any host read of an authored key after creation is therefore a read
the image cannot make. **Contract (scene-contents owner):** list the post-creation readers of
`GameSceneEntityRecord` properties. None was found in this read, and the search was not
exhaustive.

## 4. Contracts, by owner (owners from `bsp.py lease list` at 2026-09-27T10:00Z)

| file | owner now | contract |
| --- | --- | --- |
| `src/game_hosts_units.cpp` | cc9-plane-release (cc9_ship_motion_tail) | Run the creation-time B/C work after pass A, in InitAll order. Bind the squadron's 007F1D90 and 0041DF40 avoid-zone layers, and the plane's pass B setters (00C5D580, 007D9E80/007D9EE0, 007EABC0, 007E1E20, 007C74A0) where plane_flight.cpp lacks them. Bind the ship's immediate rudder 0080DA00 and torpedo stock 0081F8B0 (0081FD6E, 008201B8). Bind the carrier's `+11A8h`. |
| `src/game_hosts_gunnery.cpp` | cc9-scene-entities (cc9_death_route_destroy) | The weapon director's enable message 007214C0 (00823A9F); `BSP_Plane_RegisterFiringGuns` 007C74A0 if the gunnery host owns plane guns. |
| `src/game_hosts_lua.cpp` | free | Pass C of the plane, `thisTable.SquadronID` (007C97E3). Pass B of the squadron, `BindLuaClass` 009292B0 (007F218E). The default pass C's think-script name 0088A330. All three need pass A first, which the InitAll binding now provides. |
| `src/game_hosts_scene_contents.cpp` | free | Pass E: drop the property bag after InitAll, or list its later readers. The think script of Path and Landscape through 009295B0. |
| bombs (units or gunnery) | as above | Bombs as entities through InitAll (006E2694). |

## 5. Coverage

| routine | coverage |
| --- | --- |
| 00822C20, 0081F980, 007593D0, 007F1FE0, 007F4BA0, 007D5D20, 009295B0 | call lists only, each matched against the host's native table |
| 00758210, 007C9770 | complete |
| 007B38D0, 009277E0 | from their existing docs |
| the bomb's slots | unread |

No body without a Ghidra function was found: 007C9770 and 007F4BA0 were defined at 297fcf7fe,
and the other bodies above are Ghidra functions (`ghidra proto --brief`).

## 6. Handoff: cc9-world-init retires after this packet

Owners are from `bsp.py lease list` at 2026-09-27T10:00Z. A successor should re-check them.

**Landed and ON, on main through a5a19b83e:**

| switch | where | doc |
| --- | --- | --- |
| `kSEntityInitAllBound` | `include/bsp/game_hosts_fixed_step.hpp` | CONSTRUCT_WORLD section 17 |
| `kScanProximityUnitsEntriesBound` | `src/game_hosts_mission_frame.cpp` | section 18 |
| `kLossWarningBound` | `include/bsp/game_hosts_mission_frame.hpp` | section 19 |
| `kSunkShipFlushBound` | `include/bsp/game_hosts_ready.hpp` | section 21 |

The loss-report entry 009813A0 is in the mission-frame header and has no switch of its own.

**Open items, with owners at this moment:**

| item | what is left | files and owner | written up in |
| --- | --- | --- | --- |
| InitAll feed | `create_units` should push each constructed instance through `push_pending_entity_00926be0`. Then the Lua-route pushes and the pass-A wing append go. | `src/game_hosts_units.cpp`, cc9-plane-release (routed) | CONSTRUCT_WORLD section 17 |
| InitAll at load | The scene read's four InitAll calls (0046EB4B..0046ED0F) still go through the mission frame's `attach_scene_entities_00928a00`, not the pending list. | `src/game_hosts_mission_frame.cpp`, free | section 17 |
| RunExtraFixedStep | Only its InitAll row (00874D79) runs on the GenerateObject route. Rows 00888230, 00929460, 00778450, 0077EC20, 00874C90 and 0076FFC0 inside 00874D00 are not run there. | `src/game_hosts_lua.cpp`, free | section 17 |
| Deck-tick launches | A queued launch attaches at the next row 12, up to one frame late, because 006CDC70 runs outside the fixed step. Neither reference mission queues one. | mission frame / script orders, free | section 17 |
| Passes B, C, E | The contracts table in section 4 above. | units (cc9-plane-release), gunnery (cc9-scene-entities), Lua and scene contents (free) | section 4 above |
| Loss warning | The text post 005CF3D0, 006E6670, the `kill` channel's Lua listeners and the teardown are records. So are the readers of manager+15Ch, the aircraft caller 007F3B56 (`BSP_Aircraft_OnDestroyed`) and a census of scripts that register `kill` listeners. | `src/game_hosts_mission_frame.cpp` (free), plane side (cc9-plane-release) | section 19, docs/LOSS_WARNING.md |
| Sunk ship | Destroy at the kill in the gunnery death route. cc9-scene-entities holds cc9_death_route_destroy now, which is likely this. | `src/game_hosts_gunnery.cpp` | section 21 |
| Sunk ship | The KillDepth kill (00826628, -200 m), the removal, 00903610's free, and the world-list unlink. | `src/game_hosts_units.cpp`, cc9-plane-release (routed) | section 21 |
| Proximity scan | When a sunk ship leaves the scan is settled by section 21 (+60h at the death step). The substitution left is that a squadron node with no live leader is skipped. | mission frame, free | section 18 |
| Pump | The `[00F8A2FC]` step and the loopback drain 0076C600 are named records. | `src/game_hosts_fixed_step.cpp` (free), `src/game_hosts_ship_ai.cpp` | CONSTRUCT_WORLD section 16 |
| Scene traffic | Stopped as a read by the lead. The member creators 0087FA20/0087FB90, 00496BD0's listing, 004A4B70's tail and the binding are left, with JM01 as the measuring mission. | new files (`cmake/startup.cmake`, cc9-plane-release until 16:39Z), scene contents (free) | docs/SCENE_TRAFFIC_RUNTIME.md section 5 |
| Old section 16 items | The world's `+Ch..+14h`, `+4A4h` and `+4A8h`; the 0047F130 getter; the fillers of lists 28 and 71; the other `+4ACh` readers. | - | CONSTRUCT_WORLD section 16 |

**Tools this line left in its tree** (`J:\PROG\battlestations-pacific-decompile-cc9-world-init\local`):
- `vt_slots.py` reads vtable slots from the PE.
- `cc9-world-init-passcensus.py` matches a routine's call list against a run's native table.
- `traffic_census.py` counts traffic items per scene.
- `pairdiff.py` is superseded by `tools/pair_diff.py`.

Promote any of them through the tooling worker if they are wanted.

## 7. The ship's weapon-director enables at pass B (packet `cc9_ship_weapon_director_enable`)

Worker cc9-scene-entities, on main bb6a76f67. It takes up section 4's gunnery contract.

### What 007214C0 is, and what pass B does with it

- **007214C0 `BSP_WeaponDirectorStateMessage_BuildEnables` sets nothing on the director.**
  - `__thiscall(message)(controller)`, `RET 4`.
  - It builds a state message **from** the controller: 00721280, then the enables
    `+220h..+223h` into msg+40h..+43h and `+240h..+242h` into +44h..+46h, then 008356C0 into
    +47h and two floats into +48h and +4Ch.
- **The ship's pass B, 00822C20's property-bag arm, overwrites that message and sends it back.**
  - It reads nine keys from the merged bag with 008F2260, testing `+0Ch`:
    - `ArtilleryDirector` (00D09944), `AADirector` (00D09928), `TorpedoDirector` (00D09934) and
      `DCDirector` (00D0991C), at 008238F0..008239A3;
    - `FireStance` and `PlayerCommandsEnabled`;
    - `TorpedoEvasion`, `ShipCollAvoid` and `LandCollAvoid`.
  - `NavigatorAllowMa...` goes to `[unit+73Ch]+20h` (00823A8E).
  - After the build at 00823A9F it overwrites the message:

    | message field | value |
    | --- | --- |
    | +40h | `ArtilleryDirector` |
    | +41h | `AADirector` |
    | +42h | `TorpedoDirector` |
    | +43h | `DCDirector` |
    | +44h..+46h | the evasion and collision-avoid keys |
    | +47h | 0 |
    | +48h, +4Ch | 0.0 and `unit+980h` |
    | +8h | 1FFh or 0 |
    | +4h, +5h | director slots 24h and 28h (00823AB0, 00823AC5) |

  - It passes the message to the controller's `vt[3Ch]` at 00823B6B.
- **The controller's `vt[3Ch]`** (derived vtable 00D09F58) is 00835690. It forwards to the
  message's slot 8h, 007219C0.
- **007219C0** runs 00721890 (the inbound base, which clears the command slots) and stores
  msg+40h..+43h into **`+220h..+223h`** (007219DB..007219ED) and msg+44h..+46h into `+240h..+242h`.

**What this installation authors.**
- `universe/library/ship.props` (2024-07-13), group `Ship`, sets `ArtilleryDirector = B true`,
  **`TorpedoDirector = B false`**, `AADirector = B true` and `DCDirector = B true`.
- Neither `usn_2_java.scn` nor `usn_19_coralus.scn` overrides any of them.
- No script calls a director setter: there is no `SetWeaponDirector*` in the two mission scripts
  or `commandhelpers.lua`.
- **So after pass B, every ship's torpedo enable `+222h` is 0.**

**Who reads it.**
- 008624C0 (`docs/DIRECTOR_UPDATE_ARMS.md`, `src/unit_gunnery_pass.cpp`) pushes `+222h` into the
  masks of the torpedo categories (`kTorpedoCategories`, category 7) at 00861D70. **The gunnery
  pass fires no category-7 gun of a ship whose mask is off.**
- 009F1BC0 (the ship AI's per-pass cache) and 00720450 read the same bytes. What the ship AI's
  own attack orders do with a masked torpedo category was not read.
- The host keeps all four at the constructor's 1 (007202FD, `DirectorGunneryStance` defaults).

### The binding

`kShipDirectorEnablesBound` (`include/bsp/game_hosts_gunnery.hpp`, committed **false**):
- **The table.** The scene host keeps each entity's four merged-bag values by name
  (`scene_director_enables_*`, `src/game_hosts_scene_contents.cpp`).
- **The stance.** The gunnery pass's stance push reads them for a ship.
- **LABELLED SUBSTITUTION.** The values are read at the push, not stored on the controller at
  pass B. They never change afterwards on these missions.
- **The census:** `summary mission gunnery ship director enables torpedo_disabled_pushes=.. bound=..`.

### Predictions (written before the pairs; same tree, switch only, streams and the death table on)

**USN02 9200/9000.** OFF, on bb6a76f67: 20 deaths, 329 hit records, 807 shots, and 314 torpedo
gyro launches. The mission fails at 39.65 s after DeRuyter's death at 30.25 s (`killer=Jintsu
killer_cat=7`, a torpedo).
- **Every ship's torpedo category is masked:** 0 torpedo gyro launches, and no death with
  `killer_cat=7`. OFF has 10 such deaths, DeRuyter's among them.
- **DeRuyter survives its 30.25 s death.** The 39.65 s `Game Over` does not happen at that time.
- **Outcome, with bands:**
  - deaths down, 5..16;
  - hit records down, 150..329;
  - shots down, 350..700 (the torpedo launches are shots);
  - pair_diff exit 3, gameplay moved.
- **Uncertain.** A ship-AI attack order may still launch torpedoes in the image. The host routes
  every launch through the masked pass, so it will show none.

**E2 = USN04 9200/9000.**
- OFF has 0 torpedo gyro launches. The fleet's tube-carrying ships never launch, and its 52
  deaths are all aircraft.
- The torpedo mask changes nothing, and the other three enables are 1.
- **Identity on every gameplay row:** 52 deaths, 875 hit records, 6,092 shots. Only the census
  pushes and the native row differ.

### The pairs, measured

- **Builds.** One tree (`agent/cc9-scene-entities` at `453041b29`, which is main `bb6a76f67`
  plus this packet), built twice with only the switch flipped:
  - `local\wd_off`, SHA-256 prefix `EE0CB4008F2E`;
  - `local\wd_on`, SHA-256 prefix `88CF702DD4A6`.
- **Logs.** `local\wd_{off,on}_{usn02,e2}.log`, each with the 1600x900 override and its own
  module directory.

**`tools/pair_diff.py`, USN02 9200/9000: exit 3, gameplay moved.**

```
GAMEPLAY: MOVED
* deaths                                 20                                       19
* hit records                            329                                      529
* hull hits                              167                                      264
* damage                                 59663.8                                  29803.7
* shots                                  807                                      779
* first hit                              30.25 s                                  37.75 s
  torpedo-task releases                                                           
  dive-bomb-task releases                                                         
  torpedo drops                          0                                        0
  plane water contacts                                                            
* controlled moved                       DeRuyter 316.14                          DeRuyter 1603.01
  units                                  32                                       32
* mission end                            failed at 39.65 s (Mission.EndMission) text="Game Over" e... none (Mission.EndMission never true)
* host methods concrete/unimplemented    980 / 519                                967 / 510
DEATH ROWS: 20 -> 19 rows, 8 only ON, 9 only OFF, 11 changed
```

- **Torpedoes.** Gyro launches go from 314 to 0 (`torpedo_disabled_pushes=4383`). No death is
  `killer_cat=7`: the ON causes are category 2 (3), 3 (10) and 6 (6).
- **DeRuyter** now dies at 151.95 s (Yudachi, category 6), not at 30.25 s (Jintsu, category 7).
  The 39.65 s `Game Over` does not happen: mission end is `none`.
- **The dead flip sides.**
  - OFF only: Exeter, Houston, Alden, John1, Perth, Witte, John2, Jupiter and Encounter survive ON.
  - ON only: Minegumo, Harusame, Haguro, Tokitsukaze, Jintsu, Hatsukaze, Amatsukaze and Yukikaze
    die.
  - Eleven other death rows move in time and killer.

**`tools/pair_diff.py`, E2 = USN04 9200/9000: exit 1, gameplay identical.**

```
GAMEPLAY: identical
  deaths                                 52                                       52
  hit records                            875                                      875
  hull hits                              345                                      345
  damage                                 13618.3                                  13618.3
  shots                                  6092                                     6092
  first hit                              93.00 s                                  93.00 s
  torpedo-task releases                  4 of 16                                  4 of 16
  dive-bomb-task releases                1 of 19                                  1 of 19
  torpedo drops                          1                                        1
  plane water contacts                   19                                       19
  controlled moved                       Lexington-class01 6017.22                Lexington-class01 6017.22
  units                                  81                                       81
  mission end                            none (Mission.EndMission never true)     none (Mission.EndMission never true)
* host methods concrete/unimplemented    1043 / 548                               1044 / 548
DEATH ROWS: identical (52 rows)
PLANE DEATH MODES: identical (52 rows)
UNIT TABLE: identical (81 rows)
```

The E2 census reads `torpedo_disabled_pushes=3960`. No ship there launched before either.

**Predictions against the measurement (USN02):**

| row | predicted | measured | held |
| --- | --- | --- | --- |
| torpedo launches | 0 | 0 | yes |
| category-7 deaths | 0 | 0 | yes |
| DeRuyter / mission end | survives 30.25 s, no 39.65 s fail | dies 151.95 s, end `none` | yes |
| deaths | down, 5..16 | 20 -> 19 | direction yes, band **no** |
| hit records | down, 150..329 | 329 -> 529 | **no** (up) |
| shots | down, 350..700 | 807 -> 779 | band **no** |

Why the hit records rose: without the early torpedo kills the battle lasts longer, and both
fleets trade more gunfire. Why shots barely moved: the 314 launches are gyro launches, not 314
separate shots in the shot counter.

**Verdict: stays OFF, with the read that gates it named.**
- The reading holds for the director's automatic fire: pass B's message sets `+222h` from
  `TorpedoDirector`, 007219C0 stores it, and 008624C0 masks category 7 on it.
- **What is not read** is whether a ship's torpedoes in the image are fired only by that masked
  pass. The ship AI's attack orders, the torpedo-attack arm of the command step 00836920, and
  009F1BC0's per-pass cache could launch torpedoes past the mask.
- If they do, this switch removes torpedo attacks the image makes. The Java Sea is a mission where
  the enemy's torpedo attack is the scenario.
- **Next packet:** read the ship-side torpedo launch paths in the image against the category mask
  (00861D70's mask word, its readers, and the command step's torpedo arm). Flip only if no path
  bypasses the mask.


## 8. Does anything launch a ship's torpedoes past the category-7 mask? (packet `cc9_ship_torpedo_mask_read`)

Worker cc9-ships, on main `fe43c66cd`. Ghidra was read-only. Every census below is a rel32 or
absolute-dword scan of the image on disk (`tools/callsite_census.py`, and a displacement scan of
`.text` for `+39Ch`, `+222h`, `+12B4h` and `+12BAh`), not `ghidra callers`.

**Answer: no launch path reads past the mask on the two reference missions, but the mask does not
stay where pass B leaves it.** Two runtime writers put `+222h` back to 1, and both are reached on
USN02. Section 7's premise, "no script calls a director setter", was wrong: it searched for
`SetWeaponDirector*`, and the setter the scripts call is `TorpedoEnable`.

### The launch paths

| path | where | reads `+222h` or the mask before launching? | reached on USN02 / USN04? |
| --- | --- | --- | --- |
| the gunnery pass | 00864FE0 -> 00727F10 at 00865833 -> torpedo bot `vtable[38h]` 009035A0 -> 006DF170 (stores `bot+38h`) -> tick 008FFF20 -> launch 0072C970 at 00900912 | **yes.** Step 8.2's `enabled` (00865191..008651A6, `BL`) gates only the recon sweep; `EBX` is reloaded with `this` at 008651DD. The director-target arm (8.7) goes through 00863990, whose first call is the mask test 008633D0 at 008639A2. `mask[7]` is 3 or 0 (00861D70), so 0 refuses every target | yes, the only automatic route |
| the player's torpedo seat | message 79h group 4, 00959C20's arm 0095A1CC..0095A426 (jump table 0095A5C0, entry 4) -> 0072C970 at 0095A326 (surface ship: `+34h` held, within `00CEDF5C` of the pair) and 0095A3DD (`IsKindOf(8)` or `0Eh`: `+35h` pressed and `vtable[1D0h](1)`) | **no**, and it needs none: it fires on the player's input. 79h has one builder, 005484F0 at 005489DA (00954A10 census: 1) | only with a player at the seat; the harness player is idle |
| `NavigatorForceTorpedo` 008A7200 | walks `unit+48h`; every kind-20h device whose `[[dev+3F4h]+80h]` is 7 gets `vtable[1D8h](0,0,0)` (contract unread); a true second argument stops after the first | **no, a real bypass** | no. Five files use it: `jm05`/`jm06` (IJN, three copies each counted once) and `competitive12`/`14`; not usn_2_java.lua, usn_19_coralus.lua or commandhelpers.lua |
| `SubmarineAttack` 00894440 | not read | - | no; only `jm05` uses it |
| the command step 00836920 | body 00836920..00836EA7, 409 instructions | it has **no torpedo arm**: no read of `+222h`, `+240h` or the `torpedo` class 00E08F18, and no call into a gun. 005457C0 at 00836BD5 is a hostile-party test (`[ecx+54h] != arg && arg != 2`) | - |
| the `torpedo` command class 00E08F18 | 18 references: the HUD (00534870, 00648C20), the plane attack-command choice (007EE8F0, 007EEC50, 0099A170, 009F6D80, 009F8160, 009F9770, 00A08460), `PilotTorpedo` 008A5310, and 0071D6D0 | these are the squadron's torpedo run, not a ship launch | - |
| the ship AI, 009F1BC0 | caches `+222h` into `nested+12BAh` (009F2BAD..009F2BC2, 009F2E54..009F2E5C) and stores into `+12B4h` at 009F2D8C a value built from 00729F40(7) at 009F2D46 and `unit+44Ch` (the arithmetic between is not read). 00729F40 answers the torpedo bot's slot `20h`, 008FB530, which returns the skill row's `+14h` (FireTargetAccuracy in docs/TORPEDO_LAUNCH_GATE.md's layout), or 1.0 for another kind | it launches nothing. 009E6E80 reads the pair at 009E72F3..009E7338: with `+12BAh` set and `+12B4h` above the floor it caps the standoff at `+12B4h` minus the turn radius | the host does not model it: see "Open" |

### The two writers that re-open the mask

`+222h` has five writers in `.text`: the constructor 0072030B (1), pass B's 007219E7, 007219A7
(00721980, referenced only from the message vtables 00CFDBA0/00CFDBAC), 00720DFC (00720DE0, no
reference), and **0071C25B**, the sub-kind 5 arm of 0071C1E0. Sub-kind 5 is sent only by 0071E0D0,
which has two callers:

1. **`TorpedoEnable` 0089C8F0**, the Lua native (0089CA52).
   - usn_2_java.lua (2024-07-13) calls it for the eight `EnemyDestroya` (lines 333..340; 349
     `false` at difficulty 0, 354 `true` at 1 or 2) and the four `FinalShips` (lines 689..692; 701
     and 706).
   - The harness runs difficulty 1 (`game+6ACh=1`; `GetDifficulty` 008AE030 pushes it, or 2 when
     `[00E188A8]+1FE4h` is set). So in the image the twelve ships get `+222h = 1` after pass B:
     Yamakaze, Minegumo, Asagumo, Yukikaze, Kawakaze, Tokitsukaze, Amatsukaze, Hatsukaze, Nachi,
     Sazanami, Naka and Ushio.
   - The native is UNIMPLEMENTED in the host today (`calls=12` on USN02, 0 on E2).
   - usn_19_coralus.lua calls it at 1765..1911, in arms the E2 run does not reach.
2. **CLOSEATTACK's tail 00A11AF0** (00A15490 ends `JMP 00A11AF0` at 00A154F7; census: that is
   its only reference).
   - It walks the command's own group (`[cmd+4h]+563Ch`, the list 00A13B60 counts at `+5644h`).
   - Every member that answers `IsKindOf(6)` gets `director->0071E0D0(1)`, on every tick.
   - On USN02 the host promotes the Allied group (leader DeRuyter, 14 members) at fixed step 208,
     about 10.4 s. On E2 it promotes the Lexington group (18 members) four times.

Pass B runs in InitAll at scene load; `luaInit` runs later as a script thread (`luaStageInit`
calls `CreateScript("luaInit")`). A `GenerateObject` ship runs InitAll inside the call (the GenerateObject/SpawnNew route of docs/CONSTRUCT_WORLD.md),
before the script line that follows it. So in the image the script's value wins in both cases.

**Not a writer on a fresh mission: pass C's 008367F0.** 0081F980 calls it at 00820046 with a Lua
reader, and 00836510 hands the reader `&+220h..+223h`. `BSP_LuaReader_ReadField` 00BD6830 has no
presence test, and 00BD63B0 stores `lua_toboolean`, so a nil would store 0. But the block runs only
when `[unit+0C0h]` is set and its `+4h` is 3 (0081FA4B..0081FA5D). The table is the savegame's
(009238A0 builds the `_savedata` / `_entities` reader), and its keys are runtime state
(`repairTimer`, `helmsmanControl`, `formaciosGenya`, `pathstuff`, `gameUnit`). No `.lua`, `.props`,
`.scn` or `.txt` file of this installation contains `torpedoEnabled` or `artilleryEnabled`.

### The binding (under `kShipDirectorEnablesBound`)

- `scene_director_enables_set_torpedo` (`src/game_hosts_scene_contents.cpp`) writes the torpedo
  byte of the per-name table the stance push reads. A name with no entry starts from the
  constructor's four 1s.
- `TorpedoEnable` is handled in `src/game_hosts_script_orders.cpp` only when the switch is on. A
  plane is skipped, since its `vtable[114h]` 0047F180 answers null.
- CLOSEATTACK's tick in `src/game_hosts_ai.cpp` sends 1 for every ship of the command's own group
  after 00A13B60. 00A11B80 before it stays unbound.
- **LABELLED SUBSTITUTION:** both writers store at the send. The image routes message 5Ah through
  0077C2A0 and the session delivers it to 0071C1E0.
- **The census:** `summary mission gunnery ship director torpedo writes lua_enable=.. lua_disable=..
  close_attack_sends=.. changed=.. bound=..`.

### Predictions (written before the pairs; one tree, switch only, streams and the death table on)

**USN02 9200/9000.**
- **The writers:** `lua_enable=12`, `lua_disable=0`, `close_attack_sends > 0` from about 10.4 s.
  `changed` is 26: the twelve script ships and the fourteen Allied members, all of which pass B
  left at 0. The band is 23..26, in case a member has left the group before the first tick.
- **Who can launch ON:**
  - the eleven Allied tube ships (Kortenaer, Electra, Alden, John1..3, Exeter, Perth, Encounter,
    Jupiter, Witte), from about 10.4 s;
  - the twelve script ships;
  - **not** Haguro, Jintsu, Yudachi, Samidare, Murasame or Harusame. No death row names one of
    these six with `killer_cat=7`.
- **DeRuyter's 30.25 s death** (`killer=Jintsu killer_cat=7` on section 7's OFF) does not happen
  from Jintsu.
- **Gyro launches ON:** between 40% and 95% of OFF. The six masked Japanese ships and the Allied
  launches before 10.4 s are removed.
- **Outcome:** pair_diff exit 3. The deaths, the first hit and the mission end all move. Whether
  the mission still ends in `Game Over` is not predicted: the script ships' torpedoes can still
  reach DeRuyter.

**E2 = USN04 9200/9000.**
- `lua_enable=0` and `close_attack_sends > 0`.
- No ship launched a torpedo with every enable at 1, so nothing can launch more.
- Gameplay identical: pair_diff exit 1, and the death, plane and unit tables identical.

### The pairs, measured

- **Builds.** `tools/pair_export.py` from commit `4cd2ce621` (main `fe43c66cd` plus this packet,
  the switch OFF):
  - `local\tm_off`, no flip, SHA-256 prefix `D63EBC8453A8`;
  - `local\tm_on`, `--flip kShipDirectorEnablesBound=true`, SHA-256 prefix `B791ED12EAB6`.
- **Logs.** `local\tm_{off,on}_{usn02,e2}.log` in worktree cc9-ships. Each has the 1600x900
  override, the immediate present interval, its own module directory and the final COM release.

**`tools/pair_diff.py`, USN02 9200/9000: exit 3, gameplay moved.**

```
GAMEPLAY: MOVED
* deaths                                 20                                       22
* hit records                            329                                      597
* hull hits                              167                                      302
* damage                                 59663.8                                  57705.4
* shots                                  807                                      1063
* first hit                              30.25 s                                  35.80 s
  torpedo-task releases                                                           
  dive-bomb-task releases                                                         
  torpedo drops                          0                                        0
  plane water contacts                                                            
* controlled moved                       DeRuyter 316.14                          DeRuyter 1941.74
  units                                  32                                       32
  mission end                            failed at 39.65 s (Mission.EndMission) text="Game Over" e... failed at 39.65 s (Mission.EndMission) text="Game Over" e...
* host methods concrete/unimplemented    981 / 518                                984 / 516
DEATH ROWS: 20 -> 22 rows, 5 only ON, 3 only OFF, 15 changed
```

- **The writers.** `lua_enable=12 lua_disable=0 close_attack_sends=1090 changed=26`. The first
  `close attack torpedo enable` line follows fixed step 208.
- **Launches.** Gyro launches went from 314 to 212. `torpedo_disabled_pushes` is 1123.
- **Who killed with torpedoes ON.** The eight category-7 deaths are Exeter (Tokitsukaze, 35.95 s),
  Houston (Nachi), Perth, Jupiter and Witte (Amatsukaze), John1 (Minegumo), Asagumo (Yukikaze)
  and Alden (John3, 380.28 s). Every killer is a script-enabled ship or an Allied ship after the
  CLOSEATTACK enable. None of Haguro, Jintsu, Yudachi, Samidare, Murasame and Harusame fires one.
- **DeRuyter** now dies at 179.06 s to Murasame's category 6. Java dies at 187.21 s to Jintsu's
  category 2.

**`tools/pair_diff.py`, E2 = USN04 9200/9000: exit 1, gameplay identical.** The death, plane and
unit tables are identical (52, 52 and 81 rows). The writers read `lua_enable=0
close_attack_sends=522 changed=18`, and there are 0 gyro launches on both sides.

**Predictions against the measurement:**

| row | predicted | measured | held |
| --- | --- | --- | --- |
| lua_enable / lua_disable | 12 / 0 | 12 / 0 | yes |
| close_attack_sends | > 0 from about 10.4 s | 1090, first after fixed step 208 | yes |
| changed | 26 (band 23..26) | 26 | yes |
| category-7 kills by the six masked ships | none | none | yes |
| DeRuyter's 30.25 s death by Jintsu | does not happen | dies at 179.06 s, Murasame, category 6 | yes |
| gyro launches | 40%..95% of OFF | 212 of 314, 68% | yes |
| USN02 pair_diff | exit 3 | exit 3 | yes |
| USN02 mission end | not predicted | failed at 39.65 s on both sides | - |
| E2 | exit 1, tables identical, lua 0, sends > 0 | exit 1, identical, lua 0, sends 522 | yes |

### Verdict: `kShipDirectorEnablesBound` ON

- No launch path the reference missions reach reads past the mask, and both writers that re-open
  it are bound. So the switch now gives each ship the image's enables.
- **The Java Sea reference row keeps its meaning.** The 39.65 s `Game Over` is not a host
  artefact. usn_2_java.lua line 521 fails the mission when Houston or Exeter is dead. In the image
  Exeter is sunk at 35.95 s by Tokitsukaze, a destroyer the script's `TorpedoEnable` re-armed, as
  on OFF.
- **What changes is how the Java Sea opens.**
  - DeRuyter and Java are no longer torpedoed at 30.25 s and 32.40 s by Jintsu and Haguro: those
    two cruisers stay masked unless a Japanese group reaches CLOSEATTACK, and the host promotes
    none on USN02 (its one promotion is the Allied group).
  - The torpedo attack comes from the eight script destroyers and the four `FinalShips`, as the
    mission authors wrote it.
  - The Allied tube ships join from about 10.4 s, when their group enters CLOSEATTACK.
- **Section 7's ON measurement is superseded.** It showed no failure and 0 launches because
  neither writer was bound. It was not a picture of the image.

### Open

- **The ship AI's torpedo standoff.** 009E6E80 caps the standoff at `+12B4h` minus the turn
  radius when `+12BAh`, the cached torpedo enable, is set. The host never writes `clearance_12b4`,
  and it calls `+12BAh` `clearance_valid_12ba`: src/ship_ai_approach_update.cpp:465 sets it with
  no image store behind it. Reading 009F1BC0's `+12B4h` arithmetic (009F2D46..009F2D8C) and
  binding the pair would let torpedo-enabled ships close in the way the image's do.
- **NavigatorForceTorpedo 008A7200** and **SubmarineAttack 00894440** bypass the mask. Neither is
  used by the reference missions; `vtable[1D8h]` and 00894440 are unread.
- **00A11B80,** CLOSEATTACK's and DEFENDPOSITION's middle call, is unread and unbound.

## 9. The ship AI's torpedo standoff, `nested+12B4h` / `+12BAh` (packet `cc9_torpedo_standoff`)

Worker cc9-ships2, on main `87b51d106`. Ghidra was read-only. Every body below was read from the
listing (`disasm-raw`), with Ghidra's decompile of 009F1BC0 as the cross-check for the stack slots.
The switch is `kTorpedoStandoffBound` in `include/bsp/ship_ai_approach_update.hpp`.

**Answer:** the pair is the torpedo half of the frame-state query block at `nested+127Ch`.
`+12BAh` is the block's torpedo gate byte (`+3Eh`) and `+12B4h` its word 14. 0095EB40 never reads
word 14; 009E6E80 does. With the gate set, the clearance above 0 and a ready torpedo barrel,
009E72F3..009E7338 caps the standoff at `clearance - turn radius`. So a torpedo-armed ship with its
director's torpedo enable set closes to about half its torpedo reach.

### What 009F2AC9..009F2E9B does

EDI is `[brain+0B20h]`, the raw target. EBX is EDI when it answers `vtable[5Ch](5)`. The ship
target (`vtable[5Ch](6)`) is kept at `[ESP+1Ch]` from 009F1DE8. Offsets are nested-relative.

**Arm without a target (009F2DF7..009F2E9B):**
- The four gate bytes `+12B8h..+12BBh` are the director's `+221h`, `+220h`, `+222h` and `+223h` as
  they are.
- Word 9 (`+12A0h`) is 5.0 (00CE3850) when the torpedo gate is set, else 0.
- `+12B4h` is `[unit+44Ch] * 0.8` (00CE3D40) when the gate is set, else 0. `unit+44Ch` is
  `unit+430h + 7*4`, the category-7 maximum range 00956C20 writes.

**Arm with a target (009F2AD1..009F2DF2), torpedo half:**
1. `+12BAh` = ship target and `+222h` (009F2BA8..009F2BC2).
2. `h = 00419010(0, 3.0, 0.4, 1.0, 00923BE0(unit))` (009F2BCE..009F2BFE): 3.0 at no health, 1.0 from
   40% up. It is taken on every target frame.
3. With the gate set, `+12BAh = [unit+6DCh]->00863920(EDI)` (009F2C1F). 00863920 walks the list
   00E0A520, which is `{7}`: the byte `+77h`, `[+60h]->vtable[4](7)` and 008633D0(7, target).
4. With the gate still set and EBX nonzero, `p = 00814350(unit, 7)` (009F2C45).
   - 00814350 returns `[[[[unit+3ECh]+8]+354h]+74h]+34h]`, the first torpedo tube's projectile
     class, when `[unit+3E8h] > 0`. Otherwise it returns 0.
   - With a class, word 9 = `EBX->0095E9A0(p, [nested+1218h]) * h` (009F2C5F..009F2C6B).
   - Then, if `0080DF40(unit) > 0`:
     - `e = 00814390(unit, EBX) * 0.5`.
     - When word 9 is above `e`, `e = max(e, EBX->vtable[1D4h]() / 5.0)`.
     - Word 9 becomes word 9 minus `e`. At 0 or below the gate is cleared (009F2D04).
5. With the gate still set:
   - `[unit+3E8h] <= 0` clears the gate (009F2DEB).
   - Otherwise `+12B4h = 008387B0(7, [nested+1280h], [unit+44Ch], acc)`. `acc` is
     `[[unit+3ECh]+8]->00729F40(7)` when that device exists, else 0.
   - Then `+12B4h = min(+12B4h, max(300.0, 00952530(nested+13B0h)) * h)` (009F2D92..009F2DDE).
6. When EBX is zero the gate stays set and `+12B4h` stays at the 0 009F2A10 stored.

`+1218h` is 0 at the read: its only nested-base writers are the ring constructor's clear 009E5600
and 009E7FD1's zero. Word 9 is scratch; nothing outside 009F1BC0 reads `+12A0h`.

### The callees, read whole

| address | what it is |
| --- | --- |
| 0080DF40 | `__fastcall(unit)`: over the category-7 list `unit+3ECh`, each operational device (00729F10) adds 00727D70(0.0), its ready barrels. Body 0080DF40..0080DF7D |
| 00814350 | `__thiscall(unit)(int category)`: the first node's projectile class, as above |
| 00814390 | `__thiscall(unit)(Entity* target)`: live torpedoes in the world list `[[00E188A8]+19CCh]+21Ch/+220h` whose owner `+4F8h` is `unit`, counting only those `target->vtable[1D0h]` accepts |
| 00814420 | the ship vtable's `[1D4h]` (00CFC5A4 in MDestroyer 00CFC3D0). **No Ghidra function:** 00814420..00814492 inclusive (RET at 00814492, INT3 from 00814493). Live torpedoes whose owner is NOT this unit and that its `vtable[1D0h]` accepts |
| 008173E0 | the ship vtable's `[1D0h]` (00CFC5A0). A torpedo threatens the unit unless: it is torn down; its `+488h` run time is not above 0; for a submarine `\|dy\| > 6.0`; for another unit its y is at or below `-[class+570h]`. Otherwise it is a threat when `wrap(+46Ch) - wrap(bearing to the unit)` is below `00419010(2.0, 80 deg, 6.0, 45 deg, distance / WaterTravelSpeed)`, or when the flattened distance is under half the class Length |
| 0095E9A0 | `__thiscall(target)(class* p, float offset)`: torpedo hits still needed. `remaining = health - offset`; 0 when torn down, no class or no remaining health. The armour is the class's vtable[24h] for sub-types 0Ah/0Bh, else `+4Ch`. `f = 00419010(max(+ACh,+B4h), 1, max(+B0h,+B8h), 0, armour)`, and a zero `f` returns FLT_MAX. The per-hit damage is `WaterTickDamage * +BCh * 0.5` plus the band excess, and the answer is `remaining / (per-hit damage * f)` |
| 00838530 | the accuracy profile's inverse: the range fraction at which the accuracy falls to `acc`, blended by the target length between the small and large rows. Its first float is a LENGTH (compared with TargetReferenceSizes), so the ledger's `(range, size)` naming of 008387B0's arguments was wrong |
| 00729F40 / 008FB530 | the tube's TorpedoBot `+39Ch` (0072C870 gives every Function-7 gun one) answers `[[00E1998C] + 14h*(level+1)]`, the level row's FireTargetAccuracy |

**Image quirk kept:** 008173E0's angle test is signed. The code casts `tolerance > difference` to
a float, masks its sign and compares with 0. A torpedo whose heading lies on one side of the bearing
counts as a threat at any angle.

### The binding (under `kTorpedoStandoffBound`)

- `bsp::ship_ai_torpedo_standoff_009f2ac9` (`src/ship_ai_approach_update.cpp`) is the block above,
  in the image's order. The ship AI host runs it every frame before the two curve refills
  (009F2F11/009F2FB1), and stores `+12BAh` and `+12B4h`.
- 009E8171/009E8178 store `+12BCh`/`+12BDh`. The earlier projection set `+12BAh` there; with the
  switch on only 009F1BC0 writes it.
- 0080DF40 answers the ready torpedo barrels through `FirepowerBinding`, for both 009F2C77 and
  009E731B. With the switch off it answers 0, as before.
- 0095E9A0, 008173E0, 007B4E90, 00838530 and 008387B0 are transcribed whole.
- **LABELLED SUBSTITUTIONS:**
  - 00863920 is recomputed in the ship AI host from the inputs the gunnery host's stance push uses:
    the `+222h` table, 00861D70's mask 3/0, the gate `00861BE0` (always 1), and the liveness, class
    and rank tests of the gunnery host's `score_candidate_00863990`. A live-state accessor would
    need `src/game_hosts_gunnery.cpp`, which cc9-units3 holds.
  - 008FB530's FireTargetAccuracy is this installation's robots.lua (lines 392..432, mtime
    2025-06-01), per level `{Stun 0.03, SPNormal 0.35, SPVeteran 0.055, MPNormal 0.04, MPVeteran
    0.045, Elite 0.055}`. The level is the units host's skill level; a ship defaults to 1 (0.35).
  - A torpedo's `+46Ch` is `atan2(vx, vz)` of the host round, as the torpedo response already takes
    it. A target without a ship depth input takes `class+570h = 0`.
  - WaterTickDamage is shipglobals.lua's 100, the value `FirepowerBinding` already uses.
- **The census:** `summary mission ship ai torpedo standoff frames=.. exits=.. cap_tests=..
  cap_gates=..`, and per row `enabled`, `clearance_min`, `clearance_last` and `cap_gates`.

### Predictions (written before the pairs; same tree, switch only, streams and the death table on)

Measured on OFF logs of cc9-ships's `ts_*` pair (main `87b51d106`'s parent tree):
- USN02 has 203 ship torpedo launches, 21 deaths and `Game Over` at 39.65 s.
- The Japanese script destroyers carry a 6136 m category range and choose standoffs of 1900..2300 m.
  The Allied tube ships carry 1852 m and choose 1450..1600 m.
- USN04 has 0 ship launches and `targeted=0`. The Lexington group (18 members, Fletcher-class01..04
  among them) is promoted to CLOSEATTACK between 95 and 100 s, and its tail then sets `+222h`.

| row | prediction |
| --- | --- |
| USN02 `pair_diff` | 3, gameplay moved |
| USN02 first divergence | before 10.4 s, on one of the eight TorpedoEnable'd Japanese destroyers: with a ship target, its clearance is about `0.45 * 6136` capped by the target curve times `h`, and minus its turn radius it falls under its 1900 m standoff |
| USN02 Allied tube ships after 10.4 s | clearance about `0.5 * 1852`, so the capped standoff falls to roughly 600..800 m from 1450..1600 m (`clearance_min` under 950 on Kortenaer, Electra, Encounter, Jupiter and Witte) |
| USN02 ship torpedo launches | more than 203 |
| USN02 deaths | differ from 21; direction not predicted |
| USN02 per-ship kills and hit records | move for the Allied destroyers and the Japanese script destroyers |
| USN02 mission end | still `Game Over` (failed), time moves off 39.65 s |
| USN04 `pair_diff` | 3, NOT identity: after the promotion (95..100 s) the Fletcher-class01..04 take the no-target arm with `+12B4h = 0.8 * range`, which minus the turn radius sits under their 1450..1550 m standoffs |
| USN04 first divergence | at or after 95 s, on a Fletcher-class standoff; nothing before |
| USN04 ship torpedo launches | 0 on both sides (no ship target, `targeted=0`) |
| USN04 deaths | 2 through 95 s on both sides; later ones may move through the escorts' positions |

### The pairs, measured

- OFF is this tree's `build\` at `5bb6fb14b`; ON is `pair_export --flip kTorpedoStandoffBound=true`
  of the same commit (SHA-256 `740E378D4C66`).
- Both sides ran with `BSP_GUNNERY_RNG_STREAMS=1 BSP_DEATH_TABLE=1`, lockstep 0.05 and an idle
  player. Logs: `local\tsd_{off,on}_{usn02,usn04}.log`.
- Each log was checked for its module directory and its final COM release line.

| row | OFF | ON | prediction | verdict |
| --- | --- | --- | --- | --- |
| USN02 `pair_diff` | - | exit 3 | 3 | holds |
| USN02 first divergence | - | the 55 s gunnery step (identical through 50 s; the log has no finer ship trace) | before 10.4 s on a Japanese script destroyer | **failed** (not visible before 55 s) |
| USN02 ship torpedo launches | 219 | 219 | more than 219 | **failed** |
| USN02 deaths | 22 | 21 (John1 and Alden survive, Witte dies at 407.92 s to Tokitsukaze) | differ | holds |
| USN02 hit records / hull hits | 579 / 290 | 656 / 330 | move | holds |
| USN02 death rows | - | 10 changed; Kortenaer 84.50 -> 72.90 s, Electra 120.45 -> 112.15 s, DeRuyter 183.71 -> 176.66 s | move | holds |
| USN02 mission end | Game Over 39.65 s | Game Over 39.65 s | Game Over, time moves | **half failed**: the time did not move |
| USN02 census | frames 0 | frames 16305; clearance 8716, capped 4171, not_ship_or_off 1794, no_devices 1624; cap gates 7610 | - | - |
| USN02 Allied `clearance_min` under 950 | - | 300.0 on every row | under 950 | holds, for another reason (below) |
| USN04 `pair_diff` | - | exit 1, gameplay identical | 3 | **failed** |
| USN04 census | frames 0 | frames 2265, all `not_ship_or_off`, cap tests 0 | the no-target arm | **failed** |
| USN04 ship torpedo launches | 0 | 0 | 0 | holds |

**Why the predictions failed:**
- **USN04:** the escorts always hold a raw target, a plane, after the promotion. So 009F2AC9 takes
  the target arm, and the gate needs a SHIP target (009F2BA8). The no-target arm (`0.8 * range`)
  is never reached.
- **USN02 launches:** the cap moves where the tube ships sit. Every launch still comes from the
  same torpedo bots, and the total is unchanged at 219. The moved kills come from gunfire at the new
  ranges: Jupiter's dealt damage goes 4930 -> 7364, and the ships' nearest distances shrink
  (John1 661 -> 241).
- **`clearance_min` = 300 on every row:** a ship's first target frames come before its target
  curve (`nested+13B0h`) is first filled at 009F2FB1. 00952530 then answers 0, so the cap is
  `max(300, 0) * h`. That is the image's order, since 009F2D98 runs before the refill in the same
  frame. Afterwards the clearances settle at 1665..1796 m for the Allied tube ships and 1850..2797 m
  for the Japanese script destroyers. These are closer to the target curve than to `0.5 * range`.
- **The Allied ships are enabled on every one of their frames.** Their approach frames begin only
  once their group is in CLOSEATTACK, and by then its tail has already set `+222h`.

### Verdict: `kTorpedoStandoffBound` ON

The block runs as read, and USN02 moves through the standoff alone:
- 4171 capped clearances;
- 7610 cap gates with ready barrels;
- deaths, kills and hit records moved;
- the same launch count.

USN04 is untouched. The failed rows are predictions of effect, not mismatches with the image.

### Open

- **The other three gate bytes** `+12B8h`, `+12B9h` and `+12BBh`, and the torpedo byte's copy in
  the query 0095F080 rates, are still the host's all-1 substitution
  (`ShipAiApproach::curve_query_allow_bytes`, 009F2AE7). They need 008637D0, 00863840 and 008638B0.
- 00863920 is recomputed rather than read from the gunnery host. It should be replaced by an
  accessor on `GameGunneryHost` once `src/game_hosts_gunnery.cpp` is free.
- The TorpedoBot descriptor (008FB530) is not loaded in this process; the six accuracies are
  constants.

## 10. The query block's gate bytes `+12B8h..+12BBh` (packet `cc9_torpedo_gate_bytes`)

Worker cc9-ships2, on main `bca91c9d8`. Ghidra was read-only. The switch is
`kShipAiQueryGateBytesBound` in `include/bsp/ship_ai_approach_update.hpp`, committed OFF.

**Answer:** 008637D0, 00863840, 00863920 and 008638B0 are one loop, each over its own category
list:
- 00E0A510, AA `{1, 5, 6}`;
- 00E0A4F8, artillery `{1, 2, 3, 4, 6}`;
- 00E0A520, torpedo `{7}`;
- 00E0A528, depth charge `{8, 9}`.

Each is `__thiscall(gunneryAi)(Entity* target)`, `RET 4`. It answers true at the first category
that passes three tests:
- its byte `+70h+c` is set;
- `[+60h]->vtable[4](c)` passes;
- 008633D0(c, target) accepts the target.

009F1BC0 fills the query block's four gate bytes from them, and the frame-state query (0095F080 at
009F2F11) and the ring query (009E7FC0 -> 009E5DA0) both read those bytes. The host has had all
four at 1.

| byte | query field | with a raw target | without one |
| --- | --- | --- | --- |
| `+12B8h` | `+3Ch`, category 1 | `+221h` and 008637D0 (009F2ADF..009F2B04) | `+221h` (009F2E0F) |
| `+12B9h` | `+3Dh`, categories 2, 3, 4, 6 | `+220h` and 00863840 (009F2B18..009F2B3D) | `+220h` (009F2E29) |
| `+12BAh` | `+3Eh`, category 7 | section 9 | section 9 |
| `+12BBh` | `+3Fh`, categories 8, 9 | ship target, `+223h` and 008638B0 (009F2B43..009F2B8A) | `+223h` (009F2E43) |

The target block `+1238h` really does hold four 1s (009F2733's EBX), so it is left as it is.

### The binding (under `kShipAiQueryGateBytesBound`)

- `GameGunneryHost::group_accepts_target_008637d0(unit, list, target)` is the loop on the gunnery
  host's live pass state.
  - The byte `+70h+c` is the enabled array, except for `+77h` and `+78h`. The host keeps those two
    as `torpedo_group_flag` and `depth_charge_group_flag` (00861DB7, 00861E07).
  - The gate `00861BE0` is always 1.
  - 008633D0 applies the liveness, class, rank and mask tests in the order of
    `score_candidate_00863990`.
- `bsp::ship_ai_query_gate_bytes_009f2ac9` fills the three bytes every frame, after the standoff
  block. Both queries then take all four bytes from the block.
- With the switch on, the standoff's 00863920 is the gunnery-host accessor. Section 9's
  recomputation stays on the OFF path.
- **The census:** `summary mission ship ai query gates frames=.. aa=.. artillery=.. torpedo=..
  depth_charge=..`, and per row `query gates <unit> frames aa artillery depth_charge`.

### Predictions (written before the pairs; same tree, switch only, streams and the death table on)

The ON baseline is section 9's ON:
- USN02: 4171 capped clearances, 219 launches, 21 deaths, 656 hit records;
- USN04: gameplay identical to its OFF.

Every row below is one the log prints.

| row | prediction |
| --- | --- |
| USN02 `query gates depth_charge` | 0: rows 8 and 9 of the preference table rank only 08h and 41h, and USN02 has no submarine target |
| USN02 `query gates torpedo` | within 5% of the standoff's enabled frames, since the torpedo byte is section 9's gate |
| USN02 `query gates artillery` | more than 90% of the gate frames: every ship target is ranked by categories 2..4 and 6, and `+220h` is on |
| USN02 `pair_diff` | 3: the own and ring ratings lose every torpedo and depth-charge mount they counted with the gates at 1 |
| USN02 standoff rows | Haguro and Jintsu (torpedo byte 0, masked) change `standoff last`; so do at least half of the Allied tube ships |
| USN02 ship torpedo launches | 219, unchanged: the launch path does not read the block |
| USN02 deaths / hit records | differ from 21 / 656 |
| USN02 `capped` (clearances) | within 10% of 4171 |
| USN04 `query gates depth_charge` and `torpedo` | both 0: every escort frame holds a plane target |
| USN04 `query gates aa` and `artillery` | equal to the gate frames: category 1 ranks planes, and both lists contain it |
| USN04 standoff rows | Fletcher-class01..04 (two category-7 and two category-8 mounts each) and York-class01/02 (two category-7) change `first` or `last`. Northampton-class01/02 (categories 1, 3 and 6 only, from the OFF log's mount lines) keep theirs |
| USN04 `pair_diff` | 3, carried by the Fletcher standoffs; air deaths may move through AA geometry |

### The pairs, measured

- OFF is this tree's `build\` at `ee0753acc`; ON is `pair_export --flip
  kShipAiQueryGateBytesBound=true` of the same commit (SHA-256 `3BC8815489D0`).
- Both sides ran with the streams and the death table on. Logs: `local\gb_{off,on}_{usn02,usn04}.log`.
- Every log was checked for its module directory and its final COM release line.
- **The OFF control is not section 9's ON.** Main moved in between, and `pair_diff` of the two
  USN02 logs exits 3: 220 launches against 219, 4160 capped clearances against 4171, 20 deaths
  against 21. The rows below are judged against this OFF.

| row | OFF | ON | prediction | verdict |
| --- | --- | --- | --- | --- |
| USN02 `query gates depth_charge` | - | 0 | 0 | holds |
| USN02 `query gates torpedo` vs standoff enabled | - | 13415 = 9101 + 4314 + 0 | within 5% | holds (equal) |
| USN02 `query gates artillery` | - | 16951 of 16951 | over 90% | holds |
| USN02 `pair_diff` | - | 3 | 3 | holds |
| USN02 standoff `last` | Haguro 2300, Jintsu 2300 | 900, 1000; Alden, John1..3, Jupiter, Witte and Java also move | Haguro, Jintsu and half the Allied tube ships | holds |
| USN02 torpedo launches | 220 | 221 | unchanged | **failed** (+1) |
| USN02 deaths / hit records | 20 / 640 | 21 / 652 | differ | holds |
| USN02 capped clearances | 4160 | 4314 | within 10% | holds (+3.7%) |
| USN02 standoff `refused` | 0 | 56 | - | the live 00863920 refuses where section 9's recomputation did not |
| USN04 `query gates torpedo` / `depth_charge` | - | 0 / 0 | 0 / 0 | holds |
| USN04 `query gates aa` / `artillery` | - | 2159 / 2159 of 2159 | all frames | holds |
| USN04 standoff rows | Fletcher-class01/02 last 1500 | 1450; Fletcher-class03/04 and York-class01/02 unchanged | Fletcher-class01..04 and York-class01/02 change | **failed** in part (only two of six changed) |
| USN04 Northampton rows | 2250 / 2250 | 2250 / 2250 | unchanged | holds |
| USN04 `pair_diff` | - | 3 (deaths 41 -> 44, hit records 812 -> 789) | 3 | holds |

**Why the failed rows failed:**
- **USN02 launches:** the moved positions change one torpedo bot's opening. The launch path still
  does not read the block.
- **USN04 standoff rows:** against a plane target the tubes and depth charges contribute nothing to
  the rating either way. 0095EB40's hit probability is zero past the class range, and neither
  category ranks a plane. So only the two Fletchers whose curve crossed a scan step changed. Every
  escort's choice count fell by about 14 because the formation moved.

The 56 refusals come from the live pass state. `torpedo_group_flag` follows the bridge's push,
and the target's liveness is the gunnery host's own `dead`; section 9's recomputation assumed the
push had already happened.

### Verdict: `kShipAiQueryGateBytesBound` ON

Every gate count is what the read predicts:
- depth charges 0;
- the torpedo byte equals the standoff gate;
- AA and artillery on every frame, since category 1 is in both lists.

Both missions move through the ratings alone. 00863920 now answers from the gunnery host.

### Open

- Section 9's recomputation (`torpedo_group_accepts_target_00863920` in the ship AI host) is dead
  code on the ON path. It can be removed with the switch once this lands.
