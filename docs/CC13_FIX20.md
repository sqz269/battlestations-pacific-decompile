# CC13 fix20: Scoring_SetMissionCompleted and PlayBinkMovie behind OFF switches

Packet cc13-fix20 (2026-10-18). It answers `docs/CC13_VERIFY16.md` R7 and R8. Both natives run after
the mission outcome is decided, and neither changes whether a mission is won or lost. Both rows are
answered behind new OFF switches, so an OFF build stays byte-identical and the rows still log
UNIMPLEMENTED. No flips, no game runs, no Ghidra writes.

Status: reconstructed and build-tested. Not game-validated: no ON pair was run.

## R7 Scoring_SetMissionCompleted, 008B8AD0

**Image (read from the listing and the decompiler).**

- 008B8AD0 is the binding registered at 00E0C380. It is `__fastcall(LuaState*) -> int` and returns
  `00B66400` (the result count; it pushes nothing).
- The mode test is `[game+1FE4h] != 0`, and the default slot is `[game+18ECh]`.
  - Mode 0: there is no slot argument. The flag is arg 0.
  - Other modes: arg 0 is read with 00B66290 as the slot, and the flag moves to arg 1.
- The flag byte starts at 1. It is replaced by `lua_toboolean` (00B66250, no type gate) only when the
  argument count (00B663F0) exceeds the flag's position.
- It then calls 00906460 with ECX = `[game+21A0h]`.

00906460 (`set_slot_mission_completed_00906460`, src/mission_result.cpp) does three things:

1. It stores the clock 00F876A4 at record+10h, only on the edge where the old +0h is 0 and the flag
   is set.
2. It stores the flag at record+0h, as a zero-extended byte.
3. It walks `[[game+19CCh]+28h]` and calls 00927F60 for each value whose +180h is below 8.

00927F60 does the following:

- It computes `clock - entity+1D0h` (FSUB at 00927F73, FSTP to a local) and then stores the clock at
  entity+1D0h.
- When virtual +5Ch(0Fh) holds and the entity leads a formation (007B8AD0, entity+9D4h), it copies
  the stamp into each member's +1D0h. The loop runs up to the formation's +3CCh count, and slots
  past the fifth read as 0.
- When entity+1ACh is below 8, virtual +5Ch(5) holds and the leader flag (BL) is clear, it passes the
  elapsed time to 0090ED40 with ECX = `[game+21A0h]` and the slot entity+1ACh. 0090ED40
  (`scoring_add_unit_float_0090ed40`, include/bsp/cc13_seg59a.hpp) adds it into that slot's saved
  `unit_usage` map: record+114h (0090ED74 `LEA ECX,[slot*284h + scoring + 118h]`, the records start
  at scoring+4h), keyed by entity+170h's vtable[0]() (the 0090E740 subscript, then a float add). 0091A080
  merges that map into the progress record (0091AB5E `ADD ECX,114h`, 0090E740, a float add), and the
  progress archive writes it as section `unit_usage` (`MissionProgressRecord::unit_usage_114`,
  src/mission_progress.cpp; docs/MISSION_PROGRESS_ARCHIVE.md). The decompiler drops this float
  argument, but the listing at 00928006 shows it.

The native shows no message and makes no save or profile write. The profile write happens later, in
EndScene's commit 009205E0 and the storage paths after it.

**Host.**

- `kLuaScoringSetMissionCompletedBound` is in include/bsp/game_hosts_mission_result.hpp and is OFF.
- `GameMissionResultHost::lua_scoring_set_mission_completed_008b8ad0` builds
  `ScoringSetMissionCompletedArguments` from the session mode (`session_mode_`, game+1FE4h) and the
  local slot (`game_local_player_slot()`, game+18ECh). It then runs the existing
  `run_set_mission_completed_008b8ad0`.
- Fields written: `scoring_.slots[slot].mission_completed_00` (+0h) and `completion_time_10` (+10h).
  The clock is `mission_clock_`, which is the frame host's `world_clock` (labelled 00F876A4).
- Host reader: `run_end_scene_004d7970` -> `commit_mission_record_009205e0` reads
  `scoring_slot(commit_slot())` and copies it into `progress_`. That is the image's reader too: the
  commit at 004D7970.

**Divergences, labelled.**

- The 00927F60 walk stays one recorded step (`MissionScoring::notify_entities`, 009064B5). The host
  has no entity list at game+19CCh and no entity+1D0h, and it does not write the record's
  `unit_usage` map (record+114h) that 0090ED40 adds into.
- `scoring_slot` clamps a slot outside 0..7 to 0. The image has no such guard. This can only happen
  in a mode other than 0, which this process does not run.

**Exec guard.** commandhelpers.lua:10411 calls `os.execute("sus_prog.exe")` before
`Scoring_SetMissionCompleted(true)` at :10431. The guard (include/bsp/lua_exec_guard.hpp, commit
bb30ca8c7) returns nil and the script continues. This packet does not touch the os/io openers or the
guard.

## R8 PlayBinkMovie, 0089A480

**Image.**

- 0089A480 is registered at 00E0BA60. It reads arg 0 with 00B662B0 into a native string.
- 0089A5A2 is `XOR BL,BL`. When the count is above 2, it reads arg 2 with 00B66250 into BL
  (0089A5D1).
- 0089A5E0..0089A5E6 is `MOV DL,BL` then `CALL 0089A390`. The binding returns 00B66400 (no
  results).
- 0089A390 has two inputs: ECX = the name and DL = the debrief byte.
  1. It concatenates "movies/" with the name (0089A3FC) and calls 004CD390 (0089A40E). That
     replaces the 24h result object at game+7188h, with +8h = -200.0f and +21h = the byte.
  2. When BL is set (0089A45C), it calls 004D7970(0) at 0089A469.

The script does not wait: the call is synchronous and returns no value. Neither function sets a
pause or touches the time scale. The clip plays later, from the poll 004D7EA0 (when no debrief is
requested) or from the request-10h arm 004E458A, both through 004F8A20.

**Host.**

- `kLuaPlayBinkMovieBound` is OFF.
- `lua_play_bink_movie_0089a480` marks the binding implemented and enters
  `play_named_movie_0089a390`. That is the body the `--mission-complete-frame` injection already
  ran, split out so the injection still records the binding as not called by a script.
- Fields written: `end_movie_` (game+7188h: name, +8h, +21h). With the byte set, EndScene also
  writes `end_scene_body_done_`, the `progress_` commit and request 10h.
- Host readers: the poll `run_mission_completion_poll_004d7ea0` (`end_movie_slot()`), the
  request-10h arm `run_teardown_arm_004e458a` (`end_movie_`) and `result_projection()`.

**Divergences, labelled (unchanged from milestone 2g).**

- No clip is decoded or shown. The poll's `play_end_movie` (004D7F32) and the arm's 004F8A20 stay
  recorded.
- The executable runs the completion 004F89D0 itself.
- A non-string arg 0 reaches 0041E870 with whatever 00B662B0 returns. The wiring passes an empty
  name for nil.

## Wiring

The two rows are dispatched in `binding_trampoline` (src/game_hosts_lua.cpp) through
`GameMissionLuaHost::attach_mission_result`. src/game_hosts_mission_frame.cpp attaches it in the
frame host's constructor and detaches it in the destructor. It is not detached in `release_units`,
because the scene load calls that before the scripts run.

The row tests follow the trampoline's existing `const bool` form: `kLua...Bound && attached &&
address`. With the switch false, that folds to false at compile time, so `handled`, the
UNIMPLEMENTED record and the log are unchanged.

## RNG

The generator 00BD2F10 is absent from the static callee graph, walked eight levels deep, below 008B8AD0, 0089A480, 004D7970,
0090ED40, 00920A20 or 009205E0 (local/fix20_rng_reach.py over the call index). Virtual calls are not
covered. The host draws nothing on either path.

## Missions reached (this installation)

- **Scoring_SetMissionCompleted.** Every single-player mission end that goes through
  commandhelpers.lua: luaMissionCompleted :9967, luaMissionFailedNew :10162/:10372 (false) and
  luaMissionCompletedNew :10431 (true). The multiplayer branches are :9879, :10434-:10470 and
  :11608-:11614. 292 mission files call the end helpers (bsm, chg, COTP-IJN, COTP-USN, ijn, multi,
  traininggrounds, usn).
- **PlayBinkMovie(name, "", true).** It is reached only when the end helper got a movie (:9843,
  :10100, :10343, :10905, :11102, :11197). Calls with a literal `.bik` were found in bsm_01..bsm_11,
  COTP-IJN hawaii/jm14 (and PRCPIJN jm14), COTP-USN us_osumi, usn_14, usn_final_battle, PRCPUS
  usn_16, ijn_30_finale, ijn JM jm14, and USNRM usn_1_pearl. A movie passed through a variable is
  not counted.
- **One-argument PlayBinkMovie (mid-mission clip, no debrief).** bsm_01 :2299-:2311, bsm_02 :1448,
  bsm_04 :1934, bsm_06 :1203, bsm_07 :1054, bsm_10 :1520-:1532, PRCPIJN jm01 :2452, and the
  commandhelpers test call at :4448.

## Predicted ON effect

- **Scoring ON alone.** The local slot's record gets +0h = 1 for a win (0 for a loss), and +10h =
  the clock at the first set. Nothing reads it until EndScene runs, so in a normal run the only
  change is the new log line. Under `--mission-complete-frame` or PlayBinkMovie ON, the committed
  `progress_` record carries the flag and the time. The simulation does not move.
- **PlayBinkMovie ON.** A mission that ends with a movie runs EndScene (commit, request 10h) on the
  script's frame, then the teardown arm (state 11h) and the front-end request. The run leaves the
  mission the way the `--mission-complete-frame` injection does, instead of idling after the end
  text. A one-argument BSM clip sets the result object, and the next frame's poll records the play
  and releases it. Play continues, with no pause modelled.
- Missions without an end movie end through EndScene 008B01B0. That row is still recorded only
  (game_hosts_script_orders.cpp), so with only these switches ON they still never leave the
  mission.
