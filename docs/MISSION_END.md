# Mission end: dialogs, wing-member self tables, and what ends a failed mission

Addresses: 008CB730, 008CB825, 008CB834, 00450750, 008B0540, 008B0688, 008B0940, 008B0A5F, 008B0C10, 00734870, 008B01B0, 00531B00, 004D7970, 007F4580, 007F45A7, 007CDF20, 0077E830, 00928A00, 00925F20, 00926054

Packet `cc9_mission_end`, 2026-09-23. It closes the three open items of `docs/ENTITY_DEAD_FLAG.md`.
The report is `reports/mission_end.json`. All names are hypotheses, not recovered symbols. Nothing
here is ABI-compatible or game-validated.

## 1. The dialogs

**GetActDialogIDs (008CB730)**, `__fastcall(lua_State* ECX)`.
- It loads `ECX = [[00E188A8]+21E4h]` (008CB825), the dialog and panel owner of
  docs/PANEL_SEQUENCE.md, and calls 00450750 at 008CB834 with a local vector.
- It builds a new table (`BSP_LuaObject_NewTable`) and stores element i at index i+1 through
  00B672F0.
- It pushes that one table, **even when it is empty**.

**00450750**, `__fastcall`, ECX = owner, one stack argument (`RET 4`). It walks the owner's map
(head at +20h, the case-insensitive NativeString map at +1Ch) in order and appends each node's key
(node+0Ch) to the vector (00450540). **The registry is keyed by dialog id.**

**StartDialog (008B0540)** reads argument 1, the id string (008B0660), and calls 00451A90 on
`[game+21E4h]` (008B0688). **KillDialog (008B0940)** reads the id and calls 004514A0 on the same
owner (008B0A5F). The insert and erase bodies were not read; their names follow from the pairing
with the collector.

**What the script does with them.** `luaClearDialogs` (commandhelpers.lua:8445) only schedules
`luaClearDialogsCallback` 0.01 s later. That callback does
`for _, id in pairs(GetActDialogIDs()) do KillDialog(id) end`. With the host's nil this raised
`pairs(nil)` on every timetable pass.

**The binding.** `active_dialogs_` is a case-insensitive `std::map` keyed by id. StartDialog inserts,
KillDialog erases, and GetActDialogIDs pushes the keys as an array table. Playback is render and
audio work, the panel sequence, and is not modelled. So a started dialog stays active until killed.
That is exact for USN04, whose one dialog, `INTRO`, is still listed when the fail path kills it.
**Some other missions poll the count** (chg_3_hunt.lua:710 `table.getn(GetActDialogIDs())`), and
there the absent playback means the count never falls on its own. That is recorded, not solved.

## 2. The missing self tables

**The image.**
- Every plane class carries a slot-39 (+9Ch) attach override: 007CDF20 calls 0077E830 at 007CDF24,
  which calls 00928A00 at 0077E834. See docs/MISSION_ENTITY_LUA_ATTACH.md for MPlaneDiveBomber,
  MPlaneTorpedoBomber and the rest.
- The squadron's own slot 39 is 007F4580, `BSP_PlaneSquadron_AttachLuaSelfAndSpawnPlanes`. It
  attaches the squadron (0077E830 at 007F45A7) and then creates the wing planes.
- `BSP_SEntity_InitAll` (00925F20) dispatches +9Ch on each node of its entity list at 00926054. A
  scan of every `mov r,[r+9Ch]; call r` and `call [r+9Ch]` in .text finds that as the only mission
  dispatch.
- **So every plane, wing members included, gets its own `thisTable` slot**, keyed by its own id.
  PARTIAL: that 00925F20 reaches the planes created inside 007F4580 is inferred from the list walk,
  not traced node by node.

**The host missed them.** `GameMissionLuaHost` attached a slot only to the entity that
`create_unit_from_scene_record_0046db4b` or `create_air_ops_squadron_006c5050` returned, which is
the leader unit. **No wing member had a slot**, whether Kate or Val.

**A second defect made it look selective.** The scene markers were numbered from `units + 1`, which
is 22-26 on USN04 (CarrierPath1-4, IJNRetreat). Those are exactly the ids the first spawned squadron
takes (unit id = index + 1). So:
- the first SpawnNew attach overwrote CarrierPath1's slot;
- `thisTable["23"]` and `["24"]` were CarrierPath2 and CarrierPath3, which is where c8883c235's
  "Val #1.1|.-2/.-3 Dead" landed;
- **the carriers' `NavigatorMoveOnPath(..., FindEntity("CarrierPath1"/"CarrierPath4"))` resolved
  the path to the planes `D3A Val #1.1` and `D3A Val #1.1|.-4`.**

**The fixes.**
- `attach_wing_member_tables()` gives every other unit the creator just made its slot. It is called
  from the SpawnNew member loop and the air-ops squadron path.
- The markers number from `kSceneMarkerIdBase` = 50000.
- The Dead publisher checks the slot's `Ptr` against the unit id before writing.

## 3. What ends a failed mission

**The script path that runs.** `commandhelpers.lua` defines `luaMissionFailedNew` twice, and the
later definition (:10360) is the live one. It runs these steps:
1. `luaInitMissionEnd` (:13643): EnableMessages(false), EnableInput(false), CountdownCancel,
   `SetInvincible(unit, 0.1)` on every living unit of the three parties, MissionNarrativeClear,
   and `Mission.MissionEndParams = {Text, Ent, Movie}`.
2. `luaObj_FailedAll(true)`, `MissionStatus = false`, MUSIC_DEFEAT, `MusicEndTime = GameTime()+40`,
   `Scoring_SetMissionCompleted(false)` and `BannSupportmanager()`.
3. `Blackout(true, "", false, 0.25)` and
   `MissionNarrative("missionglobals.obj_fail", "luaMissionEnd_CamOnEnt")`.

**The narrative's completion is the gate.** MissionNarrative (008B0C10) only enqueues
(text, callback, args) through 00734870. The display, duration and callback firing live in the
narrative panel, which was not read. The callback `luaMissionEnd_CamOnEnt` (:10616) sets skip-movie
and black bars and moves the camera, then schedules `luaMissionEnd_FadeAway` in
`MusicEndTime - GameTime() - 2.5` s. That is 37.5 s after the failure less the narrative's display
time. FadeAway then runs `luaFadeAway("luaMissionEnd_Finale")`, which leads to
`luaMissionEnd_EndScene` (:10915) and `EndScene()`.

**EndScene (008B01B0), for a failed single-player mission,** reads the commit slot's scoring
record, which `Scoring_SetMissionCompleted(false)` left clear, and `game+1FE4h == 0`.
- It **raises the restart prompt** through `BSP_MenuPromptScreen_Raise` (00531B00): slot 4, kind 1,
  text `globals.restartmission|FE_xbox.ingame_quitting|globals.areyousure`. The checkpoint variant
  is used when 007F8D60 answers true.
- It **returns without calling `BSP_Game_EndScene` (004D7970)**.

So the image does not end or unload a failed mission by itself: it waits on the player's answer to
the prompt. The prompt's slot-4 kind is one of the two menu "title/pause gates"
(docs/FRONTEND_PROMPT_SCREEN.md, menu+188h). Whether that gate stops the simulation was **not
established**; the reader of +188h was not found here.

**The host.** Behind `kMissionEndBound`, the script host stamps the frame `Mission.EndMission` first
reads true. It records `MissionStatus`, `MissionEndParams.Text` and `Ent`, and every objective's
Active and Success, and writes `summary mission end: ...`. It does **not** freeze. The image's
freeze is unproven, and the step that would freeze is EndScene's prompt, which the host cannot
reach while MissionNarrative's completion is a render-side record. MissionNarrative and the camera
natives stay as they were: logging records, the render-side contract described above.

## 4. Predictions (made before the runs)

For USN04 at 4500 frames, control = base `81c798b47` (this branch before the
packet), treatment = this packet:
- No script errors: the control has 1 `luaTimetable` failure, the treatment 0.
  `luaClearDialogsCallback` gets the active set and kills it.
- The dialog set at the failure is USN04's one started dialog, if it was never killed.
- Every wing member of the 8 SpawnNew groups (48 aircraft, 16 group entities) and of the 4 air-ops
  squadrons (12 aircraft, 4 leaders) gets a slot. That is 32 + 8 = 40 `wing_member_tables`.
- The three Kates are published. `Dead` publications go from 6, of which 2 were markers, to 9, one
  per death.
- The mission-end row is at about 222.05 s: failed, "Game Over", Lexington, primary 1 failed.
- Every simulation row is identical. One expected exception: the two carriers' path argument now
  resolves to CarrierPath1 and CarrierPath4 instead of two planes.

## 5. Runs

Worktree root `J:\PROG\battlestations-pacific-decompile-cc9-difficulty`. Control
`build\win32\ctl3\`, treatment `build\win32\treat3\`.

```
./tools/run_game.ps1 -Exe build\win32\<ctl3|treat3>\bsp_game.exe -Log local\<end_ctl|end_trt2>_usn04.log -- --frames 4700 --press-start-frame 30 --menu-select USN04 --mission-frames 4500 --mission-frame-seconds 0.05
```

| run | damage | deaths | releases | script errors | `Dead` published | wing tables | dialogs start/kill | mission end row | log |
|---|---|---|---|---|---|---|---|---|---|
| control | 15970.7 | 9 | 18 | 1 | 6 (2 on markers) | 0 | records | none | `local\end_ctl_usn04.log` |
| treatment | 15970.7 | 9 | 18 | **0** | **9** | **40** | 1 / 1 (`INTRO`) | failed at **222.06 s**, "Game Over", Lexington-class01 | `local\end_trt2_usn04.log` |

`local\end_trt_usn04.log` is a first treatment run whose reader still looked for
`MissionFailParams`, so its row printed an empty text and entity. It is identical to
`end_trt2` on every other line (44801 lines each).

**Term by term, against the predictions.**
- **Script errors, 1 to 0.** `luaClearDialogsCallback` now gets `{"INTRO"}` and kills it, so the
  dialog registry ends empty. The timetable's failure count is 0, and its entity 100010 now dies
  normally (`deletes` 8 to 9).
- **The self tables.** 40 wing-member slots, exactly as predicted. The marker slots moved to 50000
  and up (CarrierPath1-4, IJNRetreat), so `self_table_entities` goes from 46 to 86.
- **`Dead` publications.** 9, one per death: Kate #2.1, #2.1|.-3, #4.1|.-3, #4.1|.-4, #6.1, Val
  #1.1, #1.1|.-2, #1.1|.-3 and Lexington. None is on a marker.
- **Mission-end row at 222.06 s.** The predicted 222.05 s plus float clock rounding.
  `status=failed`. `primary:1 Active=true Success=false`, set by luaObj_FailedAll through
  luaObj_Failed. The other six objectives were never activated.
- **Simulation.** Damage, deaths, hits, releases and every per-entity row are identical, and not
  one `ship ai step` or world-motion line differs. The carriers' `NavigatorMoveOnPath` now names
  CarrierPath1 and CarrierPath4 instead of `D3A Val #1.1` and `#1.1|.-4`. That changes only
  ship-AI bookkeeping inside 225 s: goal refreshes 15321 to 15489, the target-kind queries, and one
  movetopos row count. Positions, throttles and rudders do not change. Over a longer run the
  carriers do follow the authored paths now, which is a correction.

## 6. Decision and open items

* **Decision: land it.** The failure path is script-clean, every spawned unit has its self table,
  the Dead flag lands only on the unit's own slot, and the mission end is recorded. Every
  simulation row is identical inside the reference window.
* **Open: the narrative panel.** MissionNarrative's display and completion (the reader of
  00734870's queue) are unread. Until they are, the camera, FadeAway and EndScene steps never run in
  the host, and neither does the restart prompt that ends a failed mission in the image. The
  failure-to-prompt delay in the image is `narrative + (37.5 s - narrative) + FadeAway` from
  `MusicEndTime`, so roughly 40 s after the failure plus the fade. That is a reading of the
  script, not a measurement.
* **Open: does the slot-4 prompt pause the simulation?** menu+188h is a "pause gate" by name only
  here.
* **Open: dialog playback.** A started dialog never completes on its own. That matters for
  missions that poll `GetActDialogIDs()`, but not for USN04 or USN01.
* **Open: 007F4580's wing loop against 00925F20.** Whether InitAll reaches each wing plane is
  inferred, not traced.
* **Noted: the scene-marker FindEntity results change** for every mission whose markers used to
  share ids with spawned units. Correctness improves, and other missions' references should be
  re-run to take the new rows.
