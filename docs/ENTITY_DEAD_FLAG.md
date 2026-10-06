# The entity's Lua `Dead` flag: when the image sets it, and binding it

Addresses: 00929800, 00929896, 009298C2, 009298DB, 00929B60, 0077D270, 006FD8B0, 00926390, 009273A0, 0092747F, 00926C80, 0077D1A0, 00958A30, 00958DBE, 00922FD0, 00928A00, 00E0CF04, 0084EC10, 0084BAD0

Packet `cc9_entity_dead`, 2026-09-23. It follows the open finding in `docs/GAME_DIFFICULTY.md`
section 6: Lexington sinks and USN04's failure branch never runs. The report is
`reports/entity_dead_flag.json`. All names are hypotheses, not recovered symbols. Nothing here is
ABI-compatible or game-validated.

**Status: bound and measured.** The switch is `kEntityDeadBound` in
`include/bsp/game_hosts_script_orders.hpp`, default true.

## 1. The writer, and when `Dead` flips

**The writer is 00929800** (`__fastcall`, ECX = the entity).
- It returns at once unless the dword at entity+178h, the Lua self-table slot, is nonzero.
- Otherwise it takes the mission-entity lock and gets the self object (00927B40).
- It sets `Dead` = true at 00929896 (`BSP_LuaObject_SetBoolean(.., 1)`; the key is `Dead` at 00CF829C).
- It sets `KillReason` to `[00E0CF04 + 4 * dword entity+70h]` (009298C2). The table at 00E0CF04
  reads, by cause: 0 `none`, 1 `harm`, 2 `soft`, 3 `sell`, 4 `exitzone`, 5 `landed`, 6 `editor`,
  and 7 null. Kill remaps cause 7 to 2, per docs/UNIT_DAMAGE_AND_DEATH.md.
- When the dword entity+1D8h, the think registration, is nonzero (009298DB), it builds a
  `{Message, ID}` variant list, calls the think once (009291D0), drops the entity from both think
  lists (00928300 at 00929AA7 and 00929AB2), and frees +1D8h.

**Its only caller is 00929B60**, the base mission entity's `vtable[+7Ch]` (ECX = ESI = entity).
It calls four routines in order: 00927F60 (scoring notify), 00928100, 00929800 and then, by tail
jump, 00923040. The slot was resolved from the PE's vtable-install stores (`local/vt_slot.py`):
- 11 vtables hold 00929B60 at +7Ch.
- 12 hold the unit override 0077D270 at +7Ch. That override posts `LastBanto` when the cause is 1
  and word +2C8h is nonzero, then calls 00929B60 at 0077D3CA.
- The ship class's vtable 00CFBA80 holds 006FD8B0 at +7Ch. It tail-jumps to 0077D270 at 006FD99F.

**Who dispatches +7Ch.** 00926390, `BSP_MissionEntity_OnDestroyedHook`, is `vtable[+74h]` in all
three vtables. If entity+5Dh is clear, it sets +5Dh = +60h = 1 (bytes), calls 00925C90, then
`MOV EDX,[EAX+7Ch]; JMP EDX` (009263A8-009263AE). 009273A0,
`BSP_EntityEventQueues_FlushPending`, dispatches `vtable[+74h]` at 0092747F for every entity on
the destroy list 00F899A8. A load-and-call scan found a second dispatcher of +7Ch. 0084EC10 is a
message handler that calls `vtable[+7Ch]` directly on an entity already torn down (+5Dh set), and
`vtable[+70h](1)` otherwise.

**When `Dead` flips, for a ship.** It flips at zero health, on the next frame's flush. It does not
wait for the sinking animation or for removal. The damage path is the one docs/UNIT_DAMAGE_AND_DEATH.md
already read (steps 1-9):
1. 00958A30 sees health <= 0 and calls `vtable[70h](1)` at 00958DBE.
2. That is 0077D1A0, then 00926C80. It sets +60h, sets cause +70h = 1, and queues the entity on
   00F899A8.
3. The next 009273A0 flush reaches `vtable[74h]`, then `vtable[7Ch]`, then 00929800.

So a ship shot to death reads `Dead = true`, `KillReason = "harm"`. The Sink binding (008110F0)
and the kill list (`vtable[80h]`) are separate and later.

**For a plane shot down, the same.** The destroy path is class-independent at `vtable[70h]`, and
the plane vtables hold 0077D270 at +7Ch. **A plane that hits the water** (007CB7F0's tail
007CB92C) posts a C3h state message there and does not destroy. Its destroy path was not read, so
it is not bound.

**What else scripts observe.**
- `KillReason`.
- The think-message branch, for entities with a think. The script-timer entities are the only
  ones with a think in this host.
- `LastBanto` (0077D270).
- `LastPosition`, which is set by the kill-list slot 00928C80, not here.
- `luaRemoveDeadsFromTable` (commandhelpers.lua:1582) reads only `.Dead`.
- `luaMissionFailedNew` reads `failEnt.Dead`, and later `TrulyDead` and `LastPosition` for its
  camera.

## 2. Host against image, and the binding

| term | image | host before | host now |
|---|---|---|---|
| self-table `Dead` | true on the flush after the lethal hit | false for ever | true at the next script frame after the gunnery host's `kill_unit` |
| `KillReason` | `harm` (cause 1) for a damage death | absent | `harm` |
| think message and deregistration | when +1D8h is set | n/a | not modelled (units have no think here) |
| `LastBanto`, scoring notify | yes | no | no |
| plane water contact | not read | no | no |

`GameScriptOrdersHost::publish_unit_deaths_00929800()` runs first in `run_script_timers`, before
the think walk. It reads `GameUnitsHost::destroyed_units()`, the new seam in
`src/game_hosts_units.cpp`. That seam reads the gunnery host's per-unit rows, `sunk` and
`sunk_seconds`, which are set in `kill_unit` (`src/game_hosts_gunnery.cpp`). It then writes
`thisTable["<index+1>"].Dead = true` and `KillReason = "harm"` once per unit.
- **No gunnery edit is needed.** The rows are already public through `unit_rows()`, so there is no
  line for the integrator.
- **One ordering assumption.** The flush is taken to run before the frame's thinks, so the bound
  is one frame.

## 3. Predictions (made before the runs)

- Lexington's `Dead` flips at about 220.3 s. The phase-1 tick at `usn_19_coralus.lua:589` then
  calls `luaMissionFailed` on its next pass:
  - `Mission.EndMission = true`.
  - `luaObj_Failed("primary", 1)`, which reaches the `Objectives_Failed` native.
  - `luaMissionFailedNew(Mission.Lex, "Game Over")`, which calls EnableMessages, EnableInput,
    CountdownCancel, the SetInvincible loops, Music, `Scoring_SetMissionCompleted(false)`,
    MissionNarrativeClear, `luaClearDialogs`, then `Blackout` and `MissionNarrative`.
- The planes' `Dead` flips shrink `IJNBombersLex` and similar tables. The difficulty-1 phase-1
  completion at line 576 would need every Lex bomber or every Lex Zero dead. No Zero dies, so there
  is no completion.
- `luaRemoveDeadsFromTable(Mission.USCVs)` at line 525 drops Lexington from the carrier list that
  the IJN fleet manager uses to pick strike targets. The manager is phase 3, so nothing moves in
  phase 1. The per-tick carrier paths that name Lexington stop.
- The host has no mission end on `EndMission`. It runs to the frame budget, so every census row
  stays comparable. **Phase 2 must not be reached in the 9000-frame run**, because the objective
  that line 576 completes is already failed.

## 4. Runs

Both binaries are built from this branch's commit (base `6774451b6`, the lead's merge of
cc9_difficulty). The control has `kEntityDeadBound = false` (`build\win32\ctl2\`) and the
treatment is `build\win32\treat2\`. Worktree root `J:\PROG\battlestations-pacific-decompile-cc9-difficulty`:

```
./tools/run_game.ps1 -Exe build\win32\<ctl2|treat2>\bsp_game.exe -Log local\dead_<ctl|trt>_usn04.log -- --frames 4700 --press-start-frame 30 --menu-select USN04 --mission-frames 4500 --mission-frame-seconds 0.05
./tools/run_game.ps1 -Exe build\win32\<ctl2|treat2>\bsp_game.exe -Log local\dead_<ctl|trt>_usn04_e9000.log -- --frames 9200 --press-start-frame 30 --menu-select USN04 --mission-frames 9000 --mission-frame-seconds 0.05
```

| run | damage | deaths | queued_hits | bomb/torpedo drops | dive releases | `Dead` published | EndMission | MissionPhase at end | script binding calls | log |
|---|---|---|---|---|---|---|---|---|---|---|
| 4500 control | 16020.1 | 9 | 116 | 18 / 13 | 18 | 0 | nil | 1 | 350 | `local\dead_ctl_usn04.log` |
| 4500 treatment | 16020.1 | 9 | 116 | 18 / 13 | 18 | 6 | **true** | 1 | 357 | `local\dead_trt_usn04.log` |
| 9000 control | 19087.5 | 17 | 187 | 26 / 16 | 26 | 0 | nil | **2** | 619 | `local\dead_ctl_usn04_e9000.log` |
| 9000 treatment | 19087.5 | 17 | 187 | 26 / 16 | 26 | 7 | **true** | **1** | 372 | `local\dead_trt_usn04_e9000.log` |

The control differs from the 2026-09-23 difficulty reference (16025.3 / 8 deaths) because main
moved between `e050a353c` and `6774451b6`. That is not this packet.

**Term by term, every prediction held.**
- **The gunnery and simulation rows are identical** at both lengths: damage, deaths, hits, drops,
  releases and per-entity deaths. The `Dead` flag has no simulation reader in this host. Script
  commands issued after 222 s are the only exception: one Lexington `moveonpath` fewer in the
  4500-frame run.
- **The flips.** Lexington publishes `Dead = true, KillReason = harm` at 220.31 s, the frame of its
  gunnery death. Planes publish at their death times: Kate #2.1 at 141.15 s, Vals #1.1, #1.1|.-2
  and #1.1|.-3 at 156.65-157.00 s, Kate #6.1 at 221.01 s, and in the 9000-frame run Kate #8.1 at
  332.79 s.
- **The failure branch runs at mission frame 4441, 222.05 s**, the next pass of the phase-1 check:
  - `EndMission = true`.
  - The `Objectives_Failed` native is called with argc 5.
  - `luaMissionFailedNew` reaches CountdownCancel, MissionNarrativeClear,
    `Scoring_SetMissionCompleted`, BannSupportmanager and `Blackout(true, "")`. Those natives are
    host records.
- **Then `luaClearDialogs` throws** (commandhelpers.lua:8459, `pairs(nil)`), because
  `GetActDialogIDs` (008CB730) is unimplemented and returns nothing. The timetable timer that
  carries it fails on every pass: 1 failure in the 4500-frame run and 76 in the 9000-frame run.
  The narrative, fail text and camera never run.
- **Phase 2 is not reached in the 9000-frame run.** The control reaches it; the treatment ends in
  phase 1. The whole phase-2 script workload is gone after 222 s: 619 binding calls become 372,
  and 21 timers become 14. So the script-side census rows are comparable only up to 222 s.
- **No phase-1 completion from plane deaths.** No Zero dies, as predicted.

## 5. The friendly-fire finding (for cc9_aa_targeting; read-only)

Source: the difficulty treatment log `local\diff_trt_usn04.log`. This packet's runs reproduce it:
Lexington dies at 220.31 s, again credited to York-class02.
- **The last hits on Lexington are flak blasts.** Just before the kill there are two
  `impact blast bullet=31` lines on Lexington, at `dist=0.0 base=65.0 range=57.0`. Each takes 15.0,
  bringing health from 213.2 to 99.8. Next to them is a blast from the same shell at 56.7 m that
  lands on Northampton-class01, another US ship, and takes 0 there. The lethal hit itself has no
  per-hit log line; it is a direct entity impact. Its credit, `last_attacker`, is York-class02.
- **York-class02 is side 0 and escorts Lexington.** It is a formation follower of Lexington at
  about 500 m. It deals 339 damage in the treatment and 0 in the control. It is credited with no
  aircraft kill in the 4500-frame window. Its gun table rows show it engaging the Vals and Kates
  that attack Lexington.
- **Reading.** York's anti-aircraft fire, class 31 flak with a 57 m blast radius, detonates at
  aircraft low over Lexington. The host's blast (0084BAD0) and direct-impact paths then apply damage
  to own-side ships, with York recorded as the attacker. The 4th and 5th Lex strike aircraft that
  difficulty 1 adds put more of those engagements over the carrier, which is why only the treatment
  shows it.
- **For cc9_aa_targeting.**
  - Whether the image's blast (0084BAD0) and projectile impact filter the firer's own side, or skip
    the firer's party, is the open question.
  - Whether a friendly hit updates the victim's attacker credit is the second.
  - Not fixed here.

## 6. Decision and open items

* **Decision: land it, with `kEntityDeadBound` true.** The flag flips at the image's moment:
  zero health, the next destroy-list flush. The failure branch it unlocks is the one the script
  authors, and the one thing that moves is the script side, as predicted. From now on, any
  reference run in which Lexington sinks ends in the phase-1 failure. The 2026-09-23 E-run's
  `MissionPhase=2` is superseded for this reason.

* **Kate wing members without a self table.** Three deaths (`B5N Kate #2.1|.-3`, `#4.1|.-3`,
  `#4.1|.-4`) have no `thisTable` entry in this host, so nothing is published for them. The Val wing
  members do have one. Whether the image gives every squadron member a self table was not checked.
- **Plane water contact** has no destroy path read, so it is not bound.
- **`GetActDialogIDs` (008CB730) is unimplemented** and returns nothing. So `luaClearDialogs`
  (commandhelpers.lua:8459) raises `bad argument #1 to 'pairs'`, and luaMissionFailedNew's own
  timetable call fails at mission frame 4450. The fail text, narrative and camera never run.
- **The mission end.** The host does not end the mission on `EndMission`.

## 7. Corrections, 2026-09-23 (packet `cc9_mission_end`, `docs/MISSION_END.md`)

* **Two of the six "published" flips in the 4500-frame run were not units.** In this host the scene
  markers were numbered straight after the scene's units, at ids 22-26 on USN04 (CarrierPath1-4 and
  IJNRetreat). Those are the ids the first spawned squadron takes. So `thisTable["23"]` and
  `["24"]` were CarrierPath2's and CarrierPath3's slots, and the lines for `D3A Val #1.1|.-2` and
  `|.-3` wrote `Dead = true` onto those two path markers. **No wing member had a slot of its own**,
  Val or Kate: the three "missing" Kates were the rule, not the exception. Fixed there: the markers
  number from their own base, every wing member gets its slot, and the publisher checks the slot's
  `Ptr` before writing.
* **The fail narrative does run.** `commandhelpers.lua` defines `luaMissionFailedNew` twice, and
  the live one is the second (:10360). Section 3's list followed the first. The live one runs
  `luaInitMissionEnd` (:13643), which includes `luaClearDialogs`. That only schedules the
  callback that raises `pairs(nil)` 0.01 s later, through `luaDelay`. The function itself then
  runs to its end: `luaObj_FailedAll(true)`, `Blackout(true, "", false, 0.25)` and
  `MissionNarrative("missionglobals.obj_fail", "luaMissionEnd_CamOnEnt")`. What never runs is the
  narrative's completion callback (the camera, then FadeAway and `EndScene`), because the host
  records MissionNarrative without playing it. See docs/MISSION_END.md section 3.

## 8. A squadron's `Dead` waits for its last plane (packet `cc9_squadron_dead`, cc9-lua42, 2026-10-06)

Switch `bsp::kSquadronDeadOnLastMemberBound` (`include/bsp/game_hosts_script_orders.hpp`), committed
**OFF**. Ghidra was read-only.

**The report.** cc9-ships36's USN01 win fired 1.45 s after the last LEADER (Nell5) died, while
Nell5|.-2 and Nell5|.-3 were still flying. The script's test is
`table.getn(luaRemoveDeadsFromTable(Mission.Nells)) == 0` (usn_1_marshall.lua, mtime 2024-07-13).
`Mission.Nells` holds `GenerateObject("Nell<n>")` (line 816), which is the squadron entity, and
`luaRemoveDeadsFromTable` (commandhelpers.lua:1582) reads only `.Dead`.

**The image.** The squadron entity dies only when its last plane leaves:
- 007F3970 `BSP_Squadron_RemovePlane` (__thiscall(squadron, plane, killed_by_landing)) compacts
  +3D0h and decrements +3CCh (007F39ED).
- At 007F3A16, `CMP [ESI+3CCh],0 / JNE`. Only when the count is zero does 007F3A2A call
  00926D90 (Kill, cause = plane+70h).
  - ECX is still the squadron there. It was set at 007F3A05 (`MOV ECX,ESI`), and the call to
    007ED260 between them never writes ECX (its listing has only `LEA ECX,[ECX]`).
- So the squadron reaches 00926D90 -> destroy list -> 00929800 (`Dead` = true) only after every
  member is gone.

**The host.** The squadron is fused with its wing-0 plane (`squadron_unit`), so the leader's death
published the squadron's `Dead`. With the switch on, `publish_unit_deaths_00929800` holds a fused
leader's death while another registered member (`member_units`, not yet in `destroyed_units()`) is
still alive. It publishes the death on the frame the last one dies.
- **SUBSTITUTION, labelled:** the leader plane has no table of its own here, so its own `Dead` waits
  with the squadron's.
- A member removed by landing (007F3970 with arg 1) is not modelled.

**Predictions (OFF -> ON).**
- Rows with no multi-plane squadron whose leader dies before its wingmen: identical.
- USN01 win run: the `entity dead` line for Nell5 moves to the death of the last Nell5 member,
  and the mission's win moves with it.
- Rows whose scripts count squadrons with `luaRemoveDeadsFromTable` or `.Dead`: a phase change or
  objective that waited on a squadron fires later or not at all. Each change is explained per
  entity in the measured section.

### 8.1 Measured: **ON** (cc9-lua42, 2026-10-06)

The OFF build is `15d1eadab` (main merged, the switch OFF). The ON build is its export with
`kSquadronDeadOnLastMemberBound=true` (`local\l42_d`). The launch form is lua41's (reference AB/AC),
and USN01 36000 uses ships35's phase-2 orders (`usn01o`). Logs are `local\l42_{off,d}_<row>.log`
and diffs are `local\l42_diffd_<row>.txt`. No run completed a mission, so no guard line was
expected.

| row | `pair_diff` | what moved |
| --- | --- | --- |
| smoke 300 | 0 | none |
| USN01 3000 | 1 | gameplay identical |
| USN01 36000 (`usn01o`) | 1 | gameplay identical (the mission stays in phase 2; see below) |
| USN04 | 1 | gameplay identical |
| JM05 long | 1 | gameplay identical |
| IJN11 | 0 | none |
| ESMP08 long | 1 | gameplay identical |
| USN13 long | 3 | phase 1 ends later (below) |
| USNRM01 | 3 | the intro clean-up kills two leaderless wingmen (below) |

**USN13 long.**
- Five `bruh #3.x` leaders die between 236.11 and 254.86 s. ON holds their `Dead` until the last
  member of each squadron dies, and the last is published at 275.01 s.
- `usn_13_truk.lua` line 801 ends phase 1 when `luaRemoveDeadsFromTable(Mission.AttackWave)` is
  empty, so `MAINPHASE` finishes at 285.15 s instead of 270.31 s.
- That 15 s shift is the source of every later change (death rows 2/2 only, 7 changed; hit
  records 2871 -> 3306). The death count stays 102.

**USNRM01.**
- `KateSpawn3` and `KateSpawn5` (leaders) die at 145.60 and 146.95 s. OFF publishes their `Dead` at
  once; ON holds it while `|.-2` flies.
- `luaKillOffIntroJaps` (`usn_1_pearl.lua` 2266-2288) then runs `Kill(unit,true)` on every
  `Mission.TorpTable` squadron that is not `.Dead`. ON reaches the two squadrons, and both `|.-2`
  wingmen die at 186.91 s. OFF skips them as dead, and the two wingmen fly on and fight.
- Everything later moves with that: deaths go 135 -> 132, `2PLANESDOWN` comes at 331.74 s instead
  of 403.52 s, and hit records go 1487 -> 1554. This is the script's own clean-up, not a host
  artefact.

**Verdict: ON.** The mechanism matches the read, 007F3970's kill at +3CCh == 0, in both moved rows.
Each move traces to a script reading `.Dead` on a `GenerateObject` squadron whose leader died first.

**The USN01 launch rows** (ships36's `s36_u1_w5.txt`) never leave phase 2 on this base. Phase 2 ends
only when every `Mission.Convoy` entry is dead (`usn_1_marshall.lua` 535). Since
`kBombDropScatterBound` went ON, the ConTBD1 drops miss, so no Nell is generated and every `launch`
line is refused (`no unit by the target name`). The win's caveat (a) is covered by the rule above,
not by a USN01 run.
