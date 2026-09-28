# Ship AI for units generated after load (packet `cc9_generated_ship_ai_registration`)

Worker cc9-ships3, on main `b83cdbb90`. Ghidra was read-only. Descriptive names are hypotheses.

## 1. The gap

`GameShipAiHost::register_units` runs once, from the mission frame host
(`src/game_hosts_mission_frame.cpp`), and sizes the controllers to the units present then.
USN02's phase 2 generates four ships through `GenerateObject` (00944FD0) at about mission frame
3089 (154 s). They are Nachi (type 293, Myoko), Sazanami (73, Fubuki), Naka (70, Kuma) and
Ushio (73, Fubuki).

**They have no ship AI controller.**
- Their `NavigatorAttackMove` orders are recorded, but no ship AI step runs.
- They sit at their spawn points: (4200, -7500), (4700, -7500), (-4200, -7500) and
  (-4700, -7500).
- **They fire only what their own gun pass finds.** In the head run `local\s3_head_usn02.log`,
  Nachi fires 10 shots, Sazanami 12, Naka 8 and Ushio 9, all with 0 hits. Their nearest targets
  are 3631..4092 m away.

## 2. The image: a generated ship gets its brain on the load-time path

**The brain.** `BSP_Ship_EnsureAiOwners` `00810DD0` (body `00810DD0-00810E8A`):

```
00810DE9  CMP dword [ESI+740h],0 / JNZ 00810DFD
00810DF2  CALL 009F3F20                ; BSP_ShipAi_AllocateBrain (009F3BA0 inside)
00810DF7  MOV [ESI+740h],EAX           ; the outer bot, ship brain at +58h
00810E15  CMP dword [ESI+6DCh],0 / JNZ ; then the 558h gun object (00864580, attach 00864BD0)
```

`00810DD0` sits at vtable `+210h` of the ship vtables. `00CFB948 - 00CFB738 = 210h` for the
cruiser class.

**Its caller** is `00810F60` (`BSP_Unit_InitializeDirectorAndHullDimensions`), which sits at
vtable `+9Ch` of the same vtables (`00CFB7D4 - 00CFB738 = 9Ch`):

```
00810FA0  CALL 008366D0                ; the director, stored at +738h
00811074  MOV EAX,[ESI+0C0h] / TEST / JZ 008110C0   ; the spawn descriptor
0081107E  MOV EAX,[EAX+4h] / CMP EAX,1 / JNZ 008110A0
0081108D  CALL 0095A880                ; kind 1 (scene property bag): (0, 1, 0)
00811092  MOV EAX,[00E188A8] / CMP [EAX+1FE4h],2 / JMP 008110B2
008110A0  CMP EAX,3 / JNZ 008110C0     ; kind 3 (saved Lua table)
008110AB  CMP dword [ECX+1FE4h],2 / JZ 008110C0     ; not a network client
008110B4  MOV EDX,[ESI] / MOV EAX,[EDX+210h] / CALL EAX   ; 00810DD0
```

**Its caller** is `SEntity_InitAll` `00925F20` pass A, `[vt+9Ch]` at `0092604E`
(docs/CONSTRUCT_WORLD.md, "What it does").

**InitAll** runs over the pending-entity list from two kinds of site
(docs/CONSTRUCT_WORLD.md):
- the fixed-step row 12 at `00875EA2`, which is the load-time route;
- `BSP_Game_RunExtraFixedStep` `00874D79`, reached from the Lua `Spawn`, `GenerateObject` and
  `LaunchAirBaseSlot` bindings.

**`GenerateObject` creates from the scene record** (`0046DBE8`,
`BSP_SceneDatabase_CreateEntityByName`). The spawn descriptor is therefore the kind-1 scene
property bag, the same as a loaded ship's (docs/ENTITY_LIFECYCLE_TAILS.md, kind 1).

**So a generated ship reaches `00810DD0` through the same arm as a loaded one.** Its brain,
director and gun object are built during the `GenerateObject` call, before the next world tick.
The brain constructor `009F1160` makes its seven stream-1 draws there.

**The party brain's walk.** The AI coordinator host (`00A32350`) reads the units host live
(`src/game_hosts_units.cpp`, the comment at the coordinator guard), so a generated unit is
already visible to it. The ship AI host is the only per-unit table that was frozen at load.

## 3. The binding

The switch is `kGeneratedShipAiBound` in `src/game_hosts_ship_ai.cpp`, committed OFF.
- **The per-unit body is shared.** `register_units`' per-unit body moved unchanged into
  `register_ship_ai_unit`. It covers the 009E4330 navigation block, the depth and navigation
  inputs, the avoidance request and the think phase.
- **ON: late units are registered.** `controller_step` calls `register_generated_units` first
  on every step. That grows the controllers and rows to the units host's count and registers
  each new unit through `register_ship_ai_unit`. When at least one is a ship, it re-runs
  `seed_brain_draws_009f1160`, which seeds only unseeded controllers. That stands in for
  `009F1160`'s draws at generation.
- **A generated unit that is not a ship keeps no controller work.** Examples are USN01's
  scout squadron and USN04's SpawnNew bombers. The step loop and both fire-target stores skip
  it, as before, because `00810DD0` is a ship-vtable slot. This keeps the packet to ships.
  Whether a generated plane's director update belongs here is a separate question.
- **Labelled substitution: timing.** The image registers during the `GenerateObject` call.
  The host registers at the head of the next ship AI step, which is before any step of the new
  unit, so no controller tick is lost.
- **New log lines when ON:** `ship ai generated unit registered: unit=... ship=0|1` and
  `summary mission ship ai generated units registered=N ships=M`.

## 4. Predictions, written before any ON run

OFF is this tree's build. It should match the head run above, which was a plain run of main
`b83cdbb90` with streams and the death table on.

**USN02 9200/9000, pair_diff exit 3:**
- **The census.** `registered=4 ships=4` at about 154 s, for Nachi, Sazanami, Naka and Ushio.
  No generated non-ships.
- **The four ships steer.** Each enters `attackmove`: Nachi and Sazanami at Houston, Naka and
  Ushio at Exeter (luaMoveToPh2). Each gets a standoff row. Their speed rises above 0, and
  they close from about 5 km toward curve standoffs of about 2 km for the cruisers and about
  1.0..1.5 km for the destroyers. At about 15..18 m/s that takes about 3 minutes, so they
  reach the fight at about 330..380 s.
- **More shots and hits.** Their shots rise above 10/12/8/9 and their hits rise above 0.
  Hit records rise from 797.
- **Houston and Exeter take more fire.**
  - OFF, Houston takes 900 and ends with 6037.
  - OFF, Exeter takes 3859 and ends with 4178.
  - ON, both take more.
  - Exeter's loss is possible late in the run. If it happens, usn_2_java.lua:521 fails the
    mission at that time, where OFF has no mission end.
  - Direction only; the times are not predicted.
- **Deaths move.** OFF has 23. The FinalShips can themselves be sunk, so the direction is not
  predicted.
- **Nothing moves before about 154 s.**

**USN01 3200/3000: identity, exit 0 or 1.** Its GenerateObject is the ScoutDauntless squadron,
a non-ship. Only census lines differ: the registration line, the summary line and the unit hull
input line.

**USN04 4700/4500: identity, exit 0 or 1.** Its creations after load are SpawnNew bomber
squadrons and air-ops launches, all non-ships.

**JM06 3200/3000: identity, exit 0 or 1.** Any generated units are planes (the air-ops deck is
held back for GenerateObject); no ship is generated.

**Helm row, USN02 with `3135 takehelm Houston 1.0 EscapePoint`:**
- It is identical to OFF until about 154 s.
- OFF sinks Houston at 208.26 s by Haguro. ON, the FinalShips are still about 4 km away at
  208 s. Houston's loss time stays near 208 s or comes earlier, never later by more than the
  RNG coupling (docs/SHIP_AI_TAILS.md, the shared stream).

## 5. The pairs, and the flip

- **OFF** is `d7b44375a`, this tree's build. It is gameplay-identical to the head run (`pair_diff`
  exit 1 against `local\s3_head_usn02.log`).
- **ON** is `pair_export --commit d7b44375a --flip kGeneratedShipAiBound=true` into
  `local\gs_on`.
- **Logs:** `local\gs_{off,on}_{usn02,usn01,usn04,jm06,helm}.log`, run with streams and the death
  table on, lockstep 0.05, idle player.

| mission | pair_diff | result |
| --- | --- | --- |
| USN02 9200/9000 | exit 3 | shown below |
| helm `3135 takehelm Houston 1.0 EscapePoint` | exit 3 | shown below |
| USN01 3200/3000 | exit 1 | identical; no generated ship |
| USN04 4700/4500 | exit 1 | identical |
| JM06 3200/3000 | exit 1 | identical |

**USN02, OFF against ON:**

| measure | OFF | ON |
| --- | --- | --- |
| deaths | 23 | 26 |
| hit records | 797 | 847 |
| damage | 44672 | 56440 |
| shots | 1052 | 1117 |
| registered | none | 4 of 4 ships, at mission step 3000 |
| Exeter | survives, took 3859 | sunk at 210.81 s by an Ushio torpedo, took 6309 |
| mission end | none | `EndMission` failure at 212.91 s |
| Houston | survives, took 900 | sunk at 295.95 s by Jintsu, took 2845 |

- **The four ships enter `attackmove` and close.** Their nearest-target ranges fall:

  | ship | OFF | ON |
  | --- | --- | --- |
  | Nachi | 3631 | 2437 |
  | Sazanami | 4076 | 2534 |
  | Naka | 3631 | 3002 |
  | Ushio | 4092 | 3360 |

- **Their standoffs:**

  | ship | first | last |
  | --- | --- | --- |
  | Nachi | 900 | 1719 |
  | Sazanami | 900 | 1450 |
  | Naka | 1500 | 1795 |
  | Ushio | 900 | 900 |

- **Naka and Ushio go to `stop` at about 219 s,** after their target Exeter dies.
- **Nothing moved before phase 2.** The death list is identical to 196.96 s. The first moved
  death is Perth, 203.71 s against 203.86 s, killed by Ushio both times.

**Helm row.**
- Houston is sunk at 208.26 s by Haguro from 1878 m, the same in both runs.
- ON, Exeter is also sunk, at 211.56 s by an Ushio torpedo from 4606 m.
- Both runs fail at 212.91 s.

**Predictions:**
- **Held:**
  - the census, 4 of 4;
  - the four ships steer and close;
  - Houston and Exeter take more fire, and each is lost;
  - the mission failure appears where OFF had none;
  - deaths move;
  - the three identities;
  - Houston's helm-row loss time.
- **Failed on spread:**
  - **Timing.** Exeter's loss came at 210.81 s, not "late in the run". It came from Ushio's
    torpedoes launched on the approach, not from the gun fight at the standoff, which is
    about 3 minutes away.
  - **Gun hits.** The four ships' gun hits stayed 0. Ushio's damage, 6993, is torpedoes.
  - **Naka's shots** fell from 8 to 4 instead of rising.
- **The mechanism matched the image**: registration through the load-time body, then
  `attackmove` at the scripted target.

**Decision: `kGeneratedShipAiBound` is ON.** The predictions failed on spread only, which is
recorded above.

**For reference g:** USN02 now fails in phase 2 at 212.91 s, when Exeter is lost.
