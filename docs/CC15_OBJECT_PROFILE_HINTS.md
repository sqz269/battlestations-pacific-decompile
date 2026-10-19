# cc15 lane L2: the profile owner and the hint tracker

Two objects of the cc15 single-player bring-up (`docs/CC15_WIRE_FRONTIER.md`, ranking rows 7 and 8):
the GGame profile block at game+650h with its MissionProgress (+64h, game+6B4h), modelled as a
**new (empty) profile**, and the hint ("award") tracker singleton [00E198D0]. Both live in
`include/bsp/game_profile_owner.hpp` / `src/game_profile_owner.cpp` and are owned by
`GameMenuHost::Impl` (`src/game_hosts_menu.cpp`), declared before the HUD and mission hosts that
borrow them. Names are hypotheses; the bodies reused are the existing typed reconstructions.

## 1. Profile owner and MissionProgress (`kProfileOwnerCc15Bound`)

### Evidence and original ABI

| Address | Role | Source reused |
|---|---|---|
| 007FEE20 | profile constructor; calls 007FDB20 at 007FEF9F (only caller 004DDB90, game construction under 0073D410) | none (host construction stands in) |
| 007FDB20 | `ResetToDefaults`, ECX = game+650h, no stack arguments, RET. Destroys +64h (007FD780), `PUSH 24h` 007FDC11, new, 00920E10, store 007FDC41 | `reset_profile_007fdb20` (`src/profile_reset.cpp`) |
| 00920E10 | MissionProgress ctor, ECX = 24h object, three empty trees (+0, +0Ch, +18h) | `MissionProgressOwner::construct_mission_progress_00920e10` (`src/profile_persistence.cpp`) |
| 004C9AFA | OnInitTitle's 007FDB20 call | `MenuTitleInitHost::reset_player_profile` |
| press-start sign-in | 007FDB20 through the press-start host | `MenuPressStartHost::reset_profile_007fdb20` |
| 005C2F70 | map flag: `0090C560(ECX = [game+6B4h], &record+0)`, AL returned | `mission_map_flag_visible_005c2f70` (`src/mission_map_flag_policy.cpp`) |
| 0090C560 | lower-bound find in the +0 tree; true when the node's int (mission_completed) is non-zero | same |
| 009205E0 | mission-end record commit into [game+6B4h] | `GameMissionResultHost::mission_progress()` |

### Lifetime, as the image orders it

1. Game construction: 007FEE20 runs 007FDB20, so +64h exists from the start. Host: `GameMenuHost`
   constructor, ON only (log row `Game::construct_profile_reset 007fef9f`).
2. OnInitTitle 004C9AFA: 007FDB20 again (destroy +64h, new(24h) + 00920E10). Host: title-init hook.
3. Press-start sign-in: 007FDB20 again. Host: press-start hook.
4. Mission end: 009205E0 writes the record into [game+6B4h]. Host: `GameMissionResultHost` reads the
   profile owner at each use (it re-resolves, as the native re-reads the global), falling back to its
   own `progress_` only with the switch off.
5. Destruction: with GGame. Host: with `GameMenuHost::Impl`, after the mission and HUD hosts.

Not modelled: the real profile load (007FF100 -> storage -> 007FEFE0 -> 007FDF00 -> 00920000).

### The settings half of 007FDB20

007FDB20 also resets controls (008D4820) and gameplay settings (008D41C0) on the live settings
object, and asks XenonSystemManager [00F8ABE8] about a selected user. The host runs that half on a
private copy that is then discarded, so the host's live settings stay the ones phase 5 loaded. The
two Xenon queries log as unimplemented and answer false (the manager is not built). This is a
labelled divergence and belongs to whoever brings up the settings/profile load.

### Rows wired

- `MissionDetail::map_flag_visible` 005C2F70 (`game_hosts_mission.cpp`): ON returns
  `mission_map_flag_visible_005c2f70(group mission, *progress)`.
- `TitleInit::reset_player_profile` 007FDB20 and `PressStartScreen::reset_profile` 007FDB20.
- mission_result: `mission_progress()` is the profile owner's (the commit 009205E0 and the
  cc13-wire3 commit dump now read and write it).

### What a fresh profile shows, from the image

A new profile's +64h is three empty trees (00920E10), and 007FDB20 writes nothing into it (its other
writes, such as `RANK` = 1, go to profile+A0h). 0090C560 therefore finds no record and returns false
for every mission, so 005C2F70 hides every `mission_mapflag_<n>_Icon`. The earlier rejection ("ON
would hide every map flag the native shows") assumed a loaded profile.

This installation does have a saved profile, `Documents\Battlestations-Pacific\save\0\player`
(2,182 bytes, 2026-09-18, raw-deflate at offset 6, player "Opse"). Decoded, its `missionScores`
holds 75 records but only one, `"29"`, has `mission_completed` = 1; `"29"` is MULTI001 Dreadnought,
a multiplayer map. Every campaign record, USN04 included, has no `mission_completed` key (0). So even
a native run that loaded this save would hide every campaign map flag: ON matches the image for a
fresh profile and for this installation's save.

**What a player would see differently:** with the switch off, the host showed the completion flag
icon (`mission_mapflag_<n>_Icon`) over every mission of the campaign group on the world-map detail
page, as though all were completed. With it on, none is shown until a mission is completed in this
process (the commit then writes the record and a later visit to the page would show that one flag).
The map points (`mission_mappoint_<n>_Icon`), the unlock state and the row texts do not change.

## 2. Hint tracker (`kHintTrackerCc15Bound`)

### Evidence and original ABI

| Address | Role | Source reused |
|---|---|---|
| 004E1CA0 | getter, double-checked lazy singleton over [00E198D0]: new(A0h) + 004E18E0, registered with 00BD0C30 | `award_tracker_singleton_004e1ca0` (`src/seg10_cc13.cpp`, raw) stands as the reference; host `GameHintTrackerSlot::get_004e1ca0` |
| 004E18E0 | ctor, __fastcall(this), RET: vtable 00CE7F08, every tree/list/deque empty; +4h and +18h left alone | `GameHintTracker` members, offsets from `include/bsp/award_trackers.hpp` |
| 004E3EFB | OnInit 004E3AA0's first 004E1CA0, followed by 006921B0 ReloadHintTables | `MenuShellHost::game_on_init` |
| 0068EA00 | __thiscall(tracker), RET: 004D05E0 clears list +48h, 0068E780 pops deque +68h until +78h = 0 | raw `award_tracker_clear_hint_queue_0068ea00` (Fe5Host, not hosted); typed two-line equivalent `clear_hint_queue_0068ea00` |
| 00690CD0 | __fastcall(tracker), RET: rebuild history +94h from the profile list *(game+65Ch) | `hint_tracker_rebuild_history_00690cd0` (`src/gui_screens_3.cpp`) through `GameHintTracker` as its `HintStoreHost` |
| 004E1C80 -> 004E1A50 | destruction by the singleton-lifetime drain | host: the slot's `unique_ptr`, with `GameMenuHost::Impl` |

*(game+65Ch) is profile+0Ch, the head of the list at profile+08h (`ProfileResetState::string_list_08`,
saved as `AllSeenHints`), so 00690CD0 reads the profile owner of section 1. Both switches are
independent; with the tracker on and the profile owner off, the list is simply the never-reset
empty one.

### Rows wired

- `SetPendingScene::notify_award_tracker` 0068EA00 (`game_hosts_mission.cpp`).
- `MissionPlay::flush_award_tracker` 00690CD0 (0059230D getter, 00592314 call).
- OnInit builds the tracker; 006921B0 is recorded as unimplemented (`OnInit::reload_hint_tables`),
  so the definition map +30h stays empty. Loading it needs `HintReloadHost` over a Lua state.

The HintStoreHost hooks of routines this lane does not wire (00690B20, 00690C30, 0068EE00, 0068E600,
the 007F8E00/007FAE20/007FAE60 profile list edits, the player-unit queries) log as unimplemented if
reached. `game_hosts_mission_frame.cpp`'s own `hint_cooldowns` stand-in for the per-frame passes is
untouched; moving those passes onto this object is open.

## 3. Pairs (2026-10-19)

Builds: `build\win32\base` (a84b1ddac, the tree base), `build\win32\off` (c8991aa67) and
`build\win32\on` (the same tree with both switches true). Env `BSP_DEATH_TABLE=1
BSP_GUNNERY_RNG_STREAMS=1`. USN04: `--frames 4700 --press-start-frame 30 --menu-select USN04
--mission-frames 4500 --mission-frame-seconds 0.05`.

- **USN04 base vs OFF:** `pair_diff` exit 0 (native table 1552 rows unchanged, 780 summary lines
  unchanged).
- **USN04 OFF vs ON:** exit 1, gameplay identical (58 death rows, 58 plane death modes, 84-row unit
  table, summaries and other lines unchanged). The native table moved, all explained:
  - `+ Game::construct_profile_reset 007fef9f` concrete, 1 call: the construction reset.
  - `+ ProfileReset::xenon_state_is_two 00f8abe8` unimplemented, 2 calls: one per 007FDB20 (the
    construction reset and the title reset). `xenon_user_selected` is never reached because the
    native `&&` short-circuits.
  - `~ TitleInit::reset_player_profile 007fdb20` unimplemented -> concrete, 1 call.
  - `~ MissionDetail::map_flag_visible 005c2f70` unimplemented -> concrete, 36 calls. Each now
    returns false (the empty progress), so all 36 flag icons are hidden instead of shown.
  - `+ HintTracker::construct 004e18e0` concrete, 1 call, and `+ OnInit::reload_hint_tables
    006921b0` unimplemented, 1 call: the OnInit build.
  - `~ SetPendingScene::notify_award_tracker 0068ea00` and `~ MissionPlay::flush_award_tracker
    00690cd0` unimplemented -> concrete, 1 call each (the frontier brief expected two 0068EA00 calls;
    this run has one). Both act on empty containers.
  - `PressStartScreen::reset_profile` is not reached on this path.
- **Mission-end check, OFF vs ON** (same arguments but `--frames 1400 --mission-frames 1200
  --mission-complete-frame 900`): exit 1 with the same eight native rows and nothing else. Both runs
  commit (`GGame::EndScene ... commit=1`) and print the same `MissionRecord::commit_009205e0
  key=USN04 count_284=1 completed_00=0 ...` line; on ON that record lives in the profile owner's
  MissionProgress, so a later map-flag query in the same process would read it.

### Flipped ON: `kProfileOwnerCc15Bound` alone (cc15-flip, base 88b7eb3d1, 2026-10-19)

`kProfileLoadCc15Bound` and `kHintTrackerCc15Bound` stay false. OFF = 88b7eb3d1, ON = the same tree
with the switch true. Both builds run through tools/run_game.ps1 with `BSP_DEATH_TABLE=1
BSP_GUNNERY_RNG_STREAMS=1`, and every log has the in-process `summary mission gunnery per-consumer
random streams ON` line.

- **USN04** (`--frames 5000 --mission-frames 4800`): gameplay identical. Deaths 53, hits 691, damage
  12678.5, 81 units. Only the four section 3 profile rows moved: `+ Game::construct_profile_reset
  007fef9f`, `+ ProfileReset::xenon_state_is_two 00f8abe8` (2 calls), and `TitleInit::reset_player_profile
  007fdb20` and `MissionDetail::map_flag_visible 005c2f70` going from unimplemented to concrete. The
  summary lines and the other lines did not change.
- **Wins set** (`--stop-after-end 40`): `pair_diff` reports gameplay identical on all 12 rows. The
  end, the clock, the death rows, the unit table and the guard line match per row, and the native
  table moves only those four rows (map_flag_visible has 27-36 calls per mission):

  | Row | Orders | End (OFF = ON) |
  |---|---|---|
  | USN01 | usn01_l69_u1_a | completed 1200.05 s |
  | USN02 | usn02_l68_u2_a | failed 170.06 s (Game Over) |
  | USN12 | usn12_l68_u12_e8 | completed 1395.24 s |
  | PRCP01 | prcp01_p9_p01_k | completed 915.10 s |
  | USN21 | s46_u21_f | completed 2906.71 s |
  | ESMP08 | esmp08_p12_e8_rb | completed 1314.31 s |
  | LOMP10 | lomp10_p3_l10_b | completed 991.43 s |
  | JM06 | jm06_s45_j6_k | completed 1658.49 s |
  | BSM09 | bsm09_l68_b9_d1 | EndMission (nil) 285.05 s |
  | LOMP06 | lomp06_l70_l6_c | failed 653.11 s |
  | IJN18 | ijn18_l67_i18_d | failed 525.00 s (Game Over) |
  | BSM01 | bsm01_l69_b1_d | no end |

- The one other moved line is LOMP10's ring-scan `clear_37c` for Kaya (10000.0 -> FLT_MAX). This is
  the heap-dependent value of SHIP_AI_OPEN_ITEMS 140; see CC15_OBJECT_SCENE_DATABASE.md.
- No profile read or write: the load call site needs `kProfileLoadCc15Bound` as well
  (game_hosts_menu.cpp), and no ON log mentions a profile load. `Documents\Battlestations-Pacific`
  (`options.txt`, `save\0\player` 2,182 bytes, `quick`, `valid`) has the same sizes and mtimes
  (2026-09-18) before and after the runs.
- Not caused by the flip (both sides alike): on this main, USN02, LOMP06, IJN18 and BSM01 lose the
  wins docs/WINS_TABLE.md records for these order files. BSM09 d1 ends at 285.05 s, against 351.04 s
  at c95d107da. The env matches cc9's l70 'ref' runner and its manifests. On 4cc9872bc, where the ref
  env also gave 285.05 s, the same exe without `BSP_GUNNERY_RNG_STREAMS` ends at 429.02 s.

### Flipped ON: `kHintTrackerCc15Bound` (cc15-flip, base 46bdfac0a, 2026-10-19)

`kProfileOwnerCc15Bound` and `kProfileLoadCc15Bound` are already on, so the tracker now gets the real
save's 38 seen hints. OFF = 46bdfac0a, ON = the same tree with this switch true. Both run through
tools/run_game.ps1 with `BSP_DEATH_TABLE=1 BSP_GUNNERY_RNG_STREAMS=1`, and every log has both
`rng streams: on` and the `per-consumer random streams ON` summary line.

- **Tracker rows** (identical on all 13 rows): `+ HintTracker::construct 004e18e0` and `+
  HintTracker::reload_hint_tables 006921b0` (concrete, 1 call each). `MissionPlay::flush_award_tracker
  00690cd0` and `SetPendingScene::notify_award_tracker 0068ea00` go from unimplemented to concrete,
  1 call each. One new note: `hint tracker: 006921b0 ... entries=405 definitions=405 ... history94=13`.
  History +94h is no longer empty: it holds 13 Type keys, built from the seen names whose definition
  is not missionspecific. The summary line counts keys, not names.
- **USN04** (`--frames 5000 --mission-frames 4800`): gameplay identical, and only the rows above
  moved.
- **Wins set** (`--stop-after-end 40`): `pair_diff` reports gameplay identical on all 12 rows. The
  end, the clock, the death rows, the unit table, every summary line and the guard line match per
  row. On this base: USN01 completed 1079.93 s, USN02 failed 330.04 s, USN12 failed 859.91 s, PRCP01
  completed 814.92 s, USN21 completed 2941.74 s, ESMP08 completed 1274.27 s, LOMP10 completed
  1303.70 s, JM06 completed 898.90 s, BSM09 EndMission 282.05 s, BSM01 EndMission 812.93 s, LOMP06
  failed 668.11 s, IJN18 no end.
- **Dialogs, messages and Lua timing.** I made an ordered diff, with hex masked, of every line
  matching message, dialog, tutorial, hint, subtitle, speech, popup, objective, Lua or script, with
  its time. On every row it differs only by the five tracker lines above and the renderer worker's
  thread id. No dialog, message, objective or Lua line moved or changed its time. The per-frame
  trigger passes still do not run on the tracker (section 5 open items), so the loaded history
  cannot suppress a hint yet.
- **Other moved lines.** These are ring-scan `clear_37c` lines only (SHIP_AI_OPEN_ITEMS 140, the
  heap-dependent value): IJN18 Kita2 and Oma7, JM06 Lexington-class 01, and LOMP10 Kasumi.
- **Save untouched.** Every file under `Documents\Battlestations-Pacific` has the same SHA-256, size
  and mtime before and after the runs (player 58E93D77..., quick 54E3F992..., valid E3B0C442...,
  options.txt FF5CF395...).

- **Strict order re-check (base f906d5934).** The dialog/message check above used Compare-Object, which does not
  prove order. I re-ran it as a strict index-by-index diff, comparing `kHintTrackerCc15Bound=false` against main,
  ref env, USN04 plus the 12 wins rows. The diff covers activation, draw, power-up, weather, voice, warning, clip,
  message, dialog, hint, objective and Lua lines, with hex masked and the tracker's own lines dropped (`hint
  tracker:`, `HintTracker::*`, the two `award_tracker` rows). It finds 0 mismatched positions on all 13 rows, and
  gameplay is identical.

## 4. Uncertainties and open items

- The real profile load (and so the saved `AllSeenHints` and mission scores) is out of scope.
  A native run that signs in with this installation's save would show no campaign flags either
  (section 1), but its hint history +94h would hold the saved names.
- The settings half of 007FDB20 is withheld (section 1).
- Host construction order: the construction reset runs in the `GameMenuHost` constructor, after
  application initialize, not inside 004DDB90; nothing reads +64h in between.
- The tracker is owned by `GameMenuHost::Impl`, not by `game_hosts.cpp` (leased to another lane for
  the whole session); its lifetime still covers every borrower.
- History-map key comparison (005FD120) is taken as the case-insensitive native-string rule, like the
  definition map's 005FFB20; it is not exercised by an empty profile.

## 5. The definitions load 006921B0 (packet cc15-hint, 2026-10-19)

Under `kHintTrackerCc15Bound`, OnInit (004E3EFB) now runs `GameHintTracker::reload_hint_tables_006921b0`
on the tracker 004E1CA0 builds, instead of logging 006921B0 as unimplemented. It reuses the
existing typed bodies in `src/gui_screens_3.cpp`: `hint_tracker_reload_006921b0` (the sequence) and
`hint_tracker_load_definitions_0068f030` (one sub-table into the map +30h); the new code is only
their two host interfaces over a private Lua state (`HintTablesReload`, `src/game_profile_owner.cpp`).

### Evidence and original ABI

- 006921B0 `__fastcall(ECX = tracker)`, RET. 004DCDB0 empties +7Ch; 00B66BD0 constructs a private
  LuaStateOwner and 00692214 opens it with mask 1 (base library only); 00B69D40(path, 0) runs
  `Scripts/datatables/HintSystemMessages.lua` (literal 00CF7CB8, 29h bytes) and each VFS override of
  it. 004D7370 empties +30h. The globals table gives `HintSystemMessages`; `SYSTEM.Weapon`,
  `SYSTEM.Unit`, `SYSTEM.Gameplay` go to 0068F030 with `other_category` 0 and `OTHER.Single`,
  `OTHER.Multi` with 1 (call sites 0069233A, 0069238A, 006923DA, 00692460, 006924B1). Then 00690CD0
  rebuilds +94h, +60h and +1Ch are emptied, +18h = 13h, +24h = 0, +04h = 0, and 00B65E80 closes.
- 0068F030 `__thiscall(tracker, LuaObject table by value, char)`, RET 18h: per entry of the
  sub-table (00B67080 / 00B67190, Lua `next` order) a 128h-byte definition (layout in
  `include/bsp/gui_screens_3.hpp` `HintDefinition`) is filled and assigned to `map[id]`
  (005FFB20 + 005FDD90).
- Host mapping: the native LuaObject accessors are the stock Lua 5.1.1 calls (`lua_getfield`,
  `lua_tolstring`, `lua_toboolean`, `lua_tointeger`, `lua_next`); the file bytes come from
  `VfsLuaScriptFiles` over the host VFS (no content suffixes, x360comp 0, empty region, as
  `GameGlobalSubsystemsHost`). A non-table entry answers nil for every key (the native would read
  through its LuaObject wrapper); this installation has none.

### What it loads

This installation's `scripts/datatables/hintsystemmessages.lua` (343679 bytes) has 406 `["id"]`
lines after the header comment; one (the second `OHKA`, line 622) is inside the `--[[ ... ]]` block
at lines 619-639, so 405 entries are live. The ON run loads exactly those: `entries=405
definitions=405 replaced=0 other=340 hidden=18 critical=83 missionspecific=251`, i.e. 65 SYSTEM
definitions and 340 OTHER ones. `history94=0`: the profile is the empty (new) one, so 00690CD0 has
no shown names to file.

### Pairs (2026-10-19, tree base 328ee346b)

Builds: base (328ee346b), OFF (this change), ON (`kHintTrackerCc15Bound = true`;
`kProfileOwnerCc15Bound` stays false). Env and USN04 arguments as in section 3.

- **USN04 base vs OFF:** `pair_diff` exit 0 (native table 1555 rows unchanged, 786 summary lines
  unchanged).
- **USN04 OFF vs ON:** exit 1, gameplay identical (58 death rows, 58 plane death modes, 84-row unit
  table, all 786 summary lines unchanged). Every moved row:
  - `+ HintTracker::construct 004e18e0` concrete, 1 call: the OnInit build.
  - `+ HintTracker::reload_hint_tables 006921b0` concrete, 1 call: this packet. (The OFF build does
    not reach the `if constexpr` arm, so the old `OnInit::reload_hint_tables` unimplemented row is
    absent from both.)
  - `~ SetPendingScene::notify_award_tracker 0068ea00` and `~ MissionPlay::flush_award_tracker
    00690cd0` unimplemented -> concrete, 1 call each, as in section 3. 00690CD0 now walks 405
    definitions but the empty profile gives it no names.
  - One new other line, the `hint tracker: 006921b0 ...` counts above.
  - Host methods 1082/470 -> 1086/468.

### Open items

- Lua iteration order decides only which of two equal ids wins (`replaced` counts it); this file has
  no live duplicate, so the order is not exercised.
- `duration` and `cooldowntime` go through `lua_tointeger`; 00B66290's conversion path (with the
  SSE2 flag) is not reproduced, which matters only for non-integer values (the file has none).
- The per-frame trigger passes (TriggerHint and the queue, `hint_cooldowns` in
  `game_hosts_mission_frame.cpp`) still do not run on this object, so the loaded definitions are not
  yet consumed during a mission; with an empty profile nothing reads them in this run.
