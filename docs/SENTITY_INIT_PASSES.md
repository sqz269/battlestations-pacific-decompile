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

