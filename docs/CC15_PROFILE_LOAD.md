# cc15-prof: the real profile load, read-only

Packet cc15-prof, 2026-10-19. This builds on lane L2 (`docs/CC15_OBJECT_PROFILE_HINTS.md`), whose
`GameProfileOwner` (game+650h, MissionProgress at +64h) held only a fresh profile. With the new
switch `kProfileLoadCc15Bound` (`include/bsp/game_profile_load.hpp`, OFF), together with
`kProfileOwnerCc15Bound`, the host now reads this installation's saved profile into that owner, and
it never writes. The code is in `src/game_profile_load.cpp`, with the call site in
`src/game_hosts_menu.cpp` (`GameMenuHost::Impl::load_signed_in_profile_0067cc60`). Descriptive
names are hypotheses.

## 1. Where and how the image loads the profile

| Address | Role | Body reused |
|---|---|---|
| 0067D439.. | press-start screen, `SignInPhase::Completed` arm: when 00A3E510 reports a selected user, it calls 0067CC60 | `run_completed_arm` (`src/press_start_screen.cpp`) |
| 0067CC60 | `ApplySignIn`. It resets the profile with 007FDB20, then calls 004374F0 and the name and XUID setters, then 00BD3450. The save name is `sprintf("%llx", 00A3E5D0())`. On PC `[0109CECC]+21h` is 1, so `+1Ch(name, 1)` is asked; when that is true it calls 007FF100(name, 0067CA40), and when it is false it calls +14h and then 007FA710, the *write* of a new profile. | `apply_sign_in_0067cc60` |
| 00A3E5D0 | online XUID; XUserGetSigninInfo(slot, 2). This is 0 for a local (offline) profile, so the save name is `"0"`. The installation's only save directory is `save\0`. | (not hosted) |
| 007FF100 | `RequestRead`, `__thiscall(profile, NativeString* name, callback)`, RET 8. It sets +34h = name and asks the query. When the query is true it calls 00BD3D70(name, 1) and drives 006ADB50 to 007FEFE0. | `request_profile_read_007ff100` (`src/profile_reset.cpp`) |
| 00BEB1E0 | PC +1Ch: `<root>\<name>\valid` and `<root>\<name>\player` both open with GENERIC_READ | `PcStorageBackend::storage_query_1c` |
| 00BD3D70 / 00BEB9B0 op 2 | Queued read: 00BD3EC0 reads the file, then **stamps `<name>\valid`** (CREATE_ALWAYS + SetFileTime). On an error it raises `globals.loadfailed`. | `PcStorageBackend` |
| 00BD3EC0 | Reads the whole file. A file whose first four bytes are `Opti`, `Play`, `BSP_` or `Save` is plain text; otherwise it is decoded block by block. It then runs 00B6A020(1) and loads the result as Lua `DoBuffer`. | `read_archive_00bd3ec0` |
| 007FEFE0 | `CompleteRead`. State 1 resets the profile. Any other state builds the 004425C0 reader over the storage globals, runs 007FDF00, frees +30h, closes the storage Lua, calls `[00F8A2FC]`+A0h, then 008D7A50 (settings, using the callback), 008D5B50, and 007FAE70. | `complete_profile_read_007fefe0` |
| 007FDF00 | archive traversal into game+650h | `read_profile_archive_007fdf00` (`src/profile_archive.cpp`) |
| 00920000 | the `Score` section into +64h | `read_mission_progress_00920000` (`src/mission_progress.cpp`) |

The other two callers of 007FF100 are the console command queue (004E2200) and the `command_line_mission`
branch of 004E27E0, which reads the newest slot. Neither is on the menu path.

**Archive format** (`<CSIDL_PERSONAL>\Battlestations-Pacific\save\0\player`, 2,182 bytes). The file
is a chain of blocks. In each block, the first 6 bytes are XORed with `C7 04 0F 48 FE 4C`; after
that XOR, the block holds a little-endian DWORD length followed by a zlib stream of that length.
Each block inflates to at most 10000h bytes. The inflated text is Lua 5.1 source,
`PlayerProfile = { ... }` (56,812 bytes). `quick` (1,806 bytes) is the same container, holding
`Options = { ... }` (16,581 bytes). `valid` is an empty marker.

## 2. What is loaded (this installation's save)

The load logs a summary line in the profile owner. The values, read in place from the real file:

| Field | Value | Consumer in the host |
|---|---|---|
| `Version` (+E8h) | 2 | 00920000's record reader version gate |
| `SelectedMissionID` (+68h) | `TRN1` | none on this path |
| `AllSeenHints` (+08h; game+65Ch is its head) | 38 names | 00690CD0 rebuilds history +94h from them. The definition map +30h is empty (006921B0 is unimplemented), so every name is skipped and +94h stays empty. |
| `Unlocks` / `Bonus` | 0 / 0 | none |
| `Achievements` (named counters) | 8 (`RANK` = 1, ...) | none on this path |
| `Score.missionScores` (+64h +0) | 75 records, 1 with `mission_completed` = 1: `"29"` (MULTI001) | 005C2F70 / 0090C560 map flags |
| `score_total_30` (0090BF50 over `MultiScores`) | 0 | rank page (not reached) |

The map flags therefore stay as lane L2 had them for a fresh profile. All 36 calls query the campaign
group, and none of those records is completed, so every flag is still hidden. Only a page that
lists MULTI001 would now show a flag.

**Settings.** Natively, 007FEFE0 then calls 008D7A50, which reads `0\quick` into the live settings,
and 008D5B50 applies them. That file holds the volumes (0.5), `rumbleOff`, `swapSticks`,
`swapMapSticks`, the full `keyboardSetup`, `gamma`, `markerAlpha`, `CockpitMode` = true,
`XboxCompatibilityMode` and `DownloadedContent`. A native run that signs in with this save would
apply them, so they *should* apply once the settings bring-up is hosted. They are **not applied**
here: 008D7A50 and 008D5B50 log as `withheld`, the same as the settings half of 007FDB20, which
still runs on L2's throwaway copy. The bindings and `CockpitMode` are the values that could move a
run, so applying them needs its own pair.

## 3. No-write proof

1. **Storage calls.** `src/game_profile_load.cpp` calls only these `PcStorageBackend` members:
   `reset_storage_operation_00bd3450` (memory only), `storage_query_1c` (GENERIC_READ opens),
   `immediate_read_00bd4380` (GENERIC_READ + ReadFile, no marker, no prompt), `has_storage_buffer_30`,
   `free_and_clear_storage_buffer_30`, `close_storage_archive_00b65e80`, `storage_lua_38`, and
   `operation()` (read of `state_08`).
   The only `CreateFileA(GENERIC_WRITE)`, `DeleteFileA`, `RemoveDirectoryA` and `CreateDirectoryA`
   calls in `src/storage_backend.cpp` are in `stamp_marker`, `clear_marker`, `delete_slot`,
   `write_archive_00bd37b0`, and the op 3/4 arm of `update_00beb9b0`. Each of those is reached only
   from `update_00beb9b0`, and this file never calls `update_00beb9b0` or any `request_*` member.
2. **The native writes on this path are diverted.**
   - The queued read's `valid` stamp is replaced by the immediate read: `request_read_00bd3d70` only
     records the name, and `run_storage_operation_006adb50` reads with 00BD4380.
   - 0067CC60's 007FA710 arm (no save present) is not hosted; the call site only runs the read.
   - 004374F0's checkpoint read (kind 2, which would stamp `valid` too) is not issued. Only its
     read-only query runs, and `0\game` does not exist here.
   - `request_write_00bd3dc0`, `write_mission_progress_0090cb40` and any task other than
     `ReadCompleted` throw `std::logic_error` instead of running.
3. **The save's Lua cannot write.** It runs in a `PcStorageLuaOwner` opened with mask 1, which
   loads only `luaopen_base` (no `io`, no `os`; `src/lua_state_owner.cpp`).
4. **No other writer in the host.** Across `src/`, the profile writers 007FA710/007F9540
   (`request_profile_write_007fa710`, `write_profile_archive_007f9540`) are called only by
   `profile_reset.cpp`, `profile_write.cpp` and `pc_profile_io.cpp`, and only through
   `PcProfileIoHost`/`TextProfileWriteHost`. No `bsp_game` host constructs either. The
   `request_single_write_00bd3e10` host hooks (`cc13_sweepA.cpp`, `options_hardware_cc13.cpp`) have
   no implementer outside `PcStorageBackend`. No registry key is touched by this code.
5. **The root.** `read_only_save_root()` calls `SHGetSpecialFolderPathA` with fCreate = FALSE. The
   pre-existing phase-3 00BEB2C0 still issues its two CreateDirectoryA calls; on existing
   directories those change nothing.
6. **Measured.** I recorded the files before and after the base, OFF and ON runs. In both cases the
   SHA-256 values were `58E93D77...` (player), `54E3F992...` (quick) and `E3B0C442...` (valid, empty).
   The mtimes (UTC) were also identical: player and valid at 2026-09-19 06:27:02, quick and
   `options.txt` at 06:11:06, and directory `save` at 2026-05-04.

## 4. Pairs (2026-10-19)

Builds: `local\bin_base` (the tree base 328ee346b), `local\bin_off` (this change, switches OFF) and
`local\bin_on` (the same source with `kProfileOwnerCc15Bound`, `kHintTrackerCc15Bound` and
`kProfileLoadCc15Bound` true). Env `BSP_DEATH_TABLE=1 BSP_GUNNERY_RNG_STREAMS=1`. USN04: `--frames
4700 --press-start-frame 30 --menu-select USN04 --mission-frames 4500 --mission-frame-seconds 0.05`.
The ON run read the real save.

Before that, I made a parser check on a copy (`BSP_PROFILE_LOAD_ROOT` = `local\prof_copy\save`,
400 frames). It printed the same summary line as the real run.

- **USN04 base vs OFF:** `pair_diff` exit 0. The native table has 1555 rows, unchanged; there are
  786 summary lines, unchanged. Only the known `pretranslate` noise differs.
- **USN04 OFF vs ON:** exit 1, gameplay identical. Deaths 58, hits 882, damage 13985.7, shots
  14872, first hit 87.35 s, all unchanged. The death, plane-death and 84-row unit tables are
  unchanged, and so are the 786 summary lines. The moved rows are all explained:
  - L2's own rows: `+ Game::construct_profile_reset`, `~ TitleInit::reset_player_profile`, `~
    MissionDetail::map_flag_visible` (36 calls, all still false, section 2), `+ HintTracker::construct`,
    `+ OnInit::reload_hint_tables`, `~ SetPendingScene::notify_award_tracker` and `~
    MissionPlay::flush_award_tracker`.
  - `+ ProfileReset::xenon_state_is_two` has 3 calls, one per 007FDB20: construction, title, and
    the load.
  - Fourteen `ProfileLoad::*` rows and `PressStartScreen::apply_sign_in profile load 0067cc60`, 1
    call each. Six of them are unimplemented: `hints_owner_field_08`, `refresh_profile_manager`,
    `manager_00f8a2fc`, `commit_content_state`, and the two withheld settings rows.
  - Other lines: the four `profile load:` notes (root, checkpoint absent, completed record `"29"`,
    and the summary `queried=1 read=1 version=2 selected_mission=TRN1 seen_hints=38 unlocks=0
    achievements=8 mission_records=75 completed=1 score_total=0`). Also, `native Lua after raw
    singleton drain: getters` moves from 128 to 130: the profile load's storage Lua takes its
    environment through `owner_environment`, which reads the cached `fundamentals.lua` through the
    same getter.
- **Order.** The load runs after `TitleInit::reset_player_profile`, which is OnInitTitle 004C9AFA,
  so the title reset does not erase it. It also runs before `main-menu path PressStartPoll ->
  TitleSkip`, as natively the read and its continuation 0067CA40 precede 0068D8A0.

### Flipped ON (cc15-flip, base ec8fc67e1, 2026-10-19)

`kProfileOwnerCc15Bound` is already on (main eca2d69b7) and `kHintTrackerCc15Bound` stays false.
OFF = ec8fc67e1, ON = the same tree with this switch true. Both run through tools/run_game.ps1 with
`BSP_DEATH_TABLE=1 BSP_GUNNERY_RNG_STREAMS=1`, and every log has the in-process `per-consumer random
streams ON` line. The ON runs read the real save.

- **USN04** (`--frames 5000 --mission-frames 4800`): gameplay identical. Deaths 53, hits 691, damage
  12678.5, 81 units. The death, plane-death and unit tables are unchanged, and so are the 786 summary
  lines. The moved rows are exactly the section 4 load rows: 14 `ProfileLoad::*` rows and
  `PressStartScreen::apply_sign_in profile load 0067cc60` (1 call each; six unimplemented), and
  `ProfileReset::xenon_state_is_two` 2 -> 3 calls. The four `profile load:` notes (`seen_hints=38
  mission_records=75 completed=1`) are new, and `getters` moves 128 -> 130. `map_flag_visible` did
  not move (36 calls, all false: no campaign record is completed). There are no HintTracker rows,
  because that switch is off.
- **Wins set** (`--stop-after-end 40`): `pair_diff` reports gameplay identical on all 12 rows. The
  end, the clock, the death rows, the unit table, the summary lines and the guard line match per
  row, and each native table moves only the same 16 rows. The ends are the same as in
  CC15_OBJECT_PROFILE_HINTS.md's ProfileOwner table (USN01 completed 1200.05 s, USN02 failed
  170.06 s, USN12 1395.24 s, PRCP01 915.10 s, USN21 2906.71 s, ESMP08 1314.31 s, LOMP10 991.43 s,
  JM06 1658.49 s, BSM09 EndMission 285.05 s, LOMP06 failed 653.11 s, IJN18 failed 525.00 s, BSM01
  no end).
- The one other moved line is USN12's ring-scan `clear_37c` for Montpelier (FLT_MAX -> 10000.0).
  This is the heap-dependent value of SHIP_AI_OPEN_ITEMS 140.
- **Difficulty is unchanged.** The save's `SelectedDifficulty` (0) stays in the profile; no host
  code reads profile+60h. `chosen_difficulty()` (game_hosts_mission.cpp, game+6B0h at 0058BF52)
  returns the `--difficulty` override or 1. In BSM09 OFF and ON alike, the record difficulty is 3,
  and the log reads `MissionStart::set_effective_difficulty game+6ACh=1`.
- **Seen hints have no consumer.** The 38 `AllSeenHints` names (profile+08h) are read only by
  `GameHintTracker::profile_shown_names_65c` (00690CD0, gui_screens_3.cpp). The tracker is built only
  under `kHintTrackerCc15Bound`, and no ON log has a HintTracker row. In BSM09 the hint, tutorial
  and seen lines differ only by the three load lines. The summary lines (904) and all other lines
  outside the `profile load:` notes and the getters count are identical, so the Lua timing is
  unchanged.
- **Save untouched.** Every file under `Documents\Battlestations-Pacific` was recorded before the
  first run and after the last: `options.txt` (226 bytes, FF5CF395...), `save\0\player` (2182,
  58E93D77...), `save\0\quick` (1806, 54E3F992...) and `save\0\valid` (0, E3B0C442...). The SHA-256
  values, sizes and mtimes (UTC 2026-09-19 06:27:02 / 06:11:06; directory `save` 2026-05-04,
  `save\0` 06:27:02) are identical.

## 5. Uncertainties and open items

- **Substituted sign-in.** The host has no XenonSystemManager, so the press the host takes to the
  title skip stands in for the completed sign-in. The save id is taken as 0, which is what a local
  profile's online XUID formats to. The gamertag and XUID setters (007F9290, 007F9340, +48h) are
  not run. `Name` = `Opse` is in the file, but 007FDF00 does not read it.
- **Immediate read.** The read uses 00BD4380 instead of the queued 00BD3D70: there is no
  progress operation, no `valid` stamp and no `globals.loadfailed` prompt. A read failure resets
  the profile through 007FEFE0's state-1 arm, as natively.
- **Unimplemented.** 004374F0 (the `BSP_Chk_Save` game table and the checkpoint import), 007FAE70
  (content-state commit) and `[00F8A2FC]`+A0h are not hosted. The hints owner +08h answers 0 (its
  constructor's value); after 007FDB20 has cleared the list, either value gives the same list.
- **Settings.** The loaded settings (`0\quick`) should apply natively, but are withheld (section 2).
- **Hint history.** Hint history +94h stays empty until 006921B0 loads the definitions. After that,
  the 38 saved names will start suppressing hints, and that will need its own pair.
- **Lane L2 doc.** `docs/CC15_OBJECT_PROFILE_HINTS.md` still says "Not modelled: the real profile
  load"; it is held by lane cc15-hint and is left for that lane or the integrator to point here.

## 6. Applying the quick settings (cc15-prof2, `kQuickSettingsApplyCc15Bound`, OFF)

Packet cc15-prof2, 2026-10-19. The switch is in `include/bsp/game_profile_load.hpp`; the code is in
`src/game_profile_load.cpp` (`LiveSettings`, `QuickSettingsReadHost`, `QuickSettingsApplyHost`). The
startup host binds the live state in `src/game_hosts.cpp` through
`GameMenuHost::bind_profile_settings_sinks`. With the switch OFF, nothing is bound and the load runs
exactly as in section 1. Descriptive names are hypotheses.

**What the image runs.** At the sign-in, 0067CC60 runs 007FDB20 on the live settings 00F88980, so its
settings half applies there: 008D4820 (the control defaults +40h..+44h), the Xenon query for
008D45D0, then 008D41C0 (the game defaults). 007FEFE0 then calls 008D7A50, which reads
`<name>\quick` (storage kind 0) and deserializes it with 008D6DC0, and then 008D5B50. The 008D7A50
read is synchronous: 006ADB50's first update runs the queued read and calls 008D79A0 before 008D7A50
returns, so 008D5B50 sees the loaded values.

**What the host runs (ON).**
- **007FDB20's settings half** runs on a typed view of the raw BCh owner, in the native order. The
  construction and title calls to 007FDB20 still use the owner's throwaway block. Natively they write
  the same constants that the 00CD2D80 constructor stores, and options.txt touches none of those
  fields, so the live values are the same.
- **008D7A50** uses the existing `restore_profile_settings_008d7a50`. Its host answers
  `[0109CECC]+21h` = 1 and game present, then queries `0\quick` with GENERIC_READ. The queued 00BD3D70
  read is replaced by the immediate 00BD4380 read, as for the player read, because its completion would
  stamp `0\valid`. 008D6DC0 then runs `read_settings_archive_008d6dc0`. Its shader-model tail reads the
  renderer's PixelShaderVersion (renderer+1B40h, the value 008D7671 reads as virtual +104h -> +28h).
- **The write-back.** After 007FDB20's half, after 008D7A50, and at the end of the load, every scalar
  the bodies store is written back into 00F88980 at its native offset, and each change is logged. The
  +B4h ClanText and +98h DownloadedContent fields own native allocations, so a change to either is
  reported, not stored. This save holds `""` and `{}` for them, so nothing is reported.
- **008D5B50** runs `apply_all_settings_008d5b50` in its title-time state: the game is present; the
  scene, ocean, foliage and clouds (game+19E8h..19FCh), the download manager 00F8A304 and the
  in-mission interface 00E198C4 are absent. Its sinks:

| Sink (native) | Host state | Status |
|---|---|---|
| 008D5483 / 008D5490, 00F8BBCC +218h / +21Ch | the sound alternate publication, as the startup copy does | applied (0.5, 0.5: unchanged) |
| 00A94C50 rumble, `!game+634h && !rumbleOff` | `GameInputRuntime::set_rumble_enabled_00a94c50` (00E12F2C and every device's motors) | applied: enabled 1 -> 0 |
| 008D6132, the 00439100 region into 0108FF24 | the canonical Lua globals | applied: `USA` on this machine |
| 008D44C0 inside 008D6DC0: +B0h, the 0108FF20 mirror | the canonical Lua globals | applied (false: unchanged) |
| 008D44C0's console command 006B8AD0, 00F8BBD8 +70h, 00A7A440, 00A864F0, 00A7ABF0, 00A7ACF0, 004C1710 +10h/+14h, 00B1FFB0, renderer +F0h (gamma), 00699B80 (stick modifiers), 004DCDF0, 007371D0, 00AA09D0, 00B0D580, renderer +80h, 00B29E60, 0109CF04 +0Ch, XLiveOnResetDevice, 00B107F0, 004C12B0, 00B0D080 | none in the host | one unimplemented row each |

**Keyboard bindings are not applied.** 008D6DC0 hands the `keyboardSetup` table to 006ABA50 on the input
settings that 00698A10 builds from `Scripts\datatables\Inputs.lua`. The host builds no such
configuration: its press-start screen logs 00698A10 as unimplemented, and the host drives actions by
id, not by key. The table is therefore entered with empty settings. 006ABA50 reads only the device and
input names those settings hold, so it reads nothing, and 006AA640 stops at its 00A92260 check. The
save's bindings (four devices, `DevInputs`, `FE_pc.cs_air`, `FE_pc.cs_command`, `FE_pc.cs_sealandaa`)
and sensitivities stay unread. They can move a run only once the input configuration is hosted, and
then that needs its own pair.

**What this save changes** (read in place, logged by the ON run):

| Field | Before (constructor + options.txt) | After | Host reader |
|---|---|---|---|
| +40h `rumbleOff` | 0 | 1 | 00A94C50 (applied; no device in a harness run) |
| +80h `markerAlpha` | 0 | 0.2 | none. 00643C9C's objective-sweep gate (00F88A00 > 0.03) is `HudMarkers::objective_detail_level` in `src/game_hosts_hud_world.cpp`, which answers a constant 0.0 |
| +95h `HardwareReported` | 0 | 1 | none (the hardware probe needs `BSM_HWD`, which is absent here) |
| all others | | unchanged | the volumes 0.5, swapSticks 1, invertCameraY 0, invertPlaneY 0, swapMapSticks 1, gamma 0, CockpitMode 1, XboxCompatibilityMode 0, and the absent keys at their 008D6DC0 defaults, which equal the reset values |

`CockpitMode` = true equals the constructor and 008D41C0 value, so it cannot move a run.

**Pairs (2026-10-19).** Builds: `local\bin_base` (tree base 7f4f75b85), `local\bin_off` (da68a2635,
both switches OFF), `local\bin_quick` (the same source with `kQuickSettingsApplyCc15Bound` true).
Reference env (`BSP_DEATH_TABLE=1 BSP_GUNNERY_RNG_STREAMS=1`, no `BSP_FRAME_JITTER`); every log has
`rng streams: on`. The launcher is `local\prof2_run.ps1`, with the cc15-app row arguments. ON runs read
the real save. Before them, a 400-frame run on a copy (`BSP_PROFILE_LOAD_ROOT=local\prof2_copy\save`)
printed the same lines.
- **USN04, base vs OFF:** `pair_diff` exit 0, with only the `pretranslate` noise. 1589 native rows, 793
  summary lines, 53 deaths, 762 hits.
- **USN04 and all 12 wins rows, OFF vs ON:** `pair_diff` exit 1 on every row, gameplay identical. The
  death rows, plane-death modes and unit table are identical on every row, and so are the summary
  lines except USN21's one (below). The strict
  index-by-index sequence of dialog, message and Lua lines (`local\prof2_seqdiff2.py`, dropping only
  the switch's own `QuickSettings*` and `quick settings:` lines) is identical on every row. The
  moved rows are the same on every row:
  - 36 added native rows: `ProfileLoad::reset_settings_half`, `ProfileLoad::restore_settings` and
    `ProfileLoad::apply_settings` (concrete), eight `QuickSettings::*` rows (the read and 008D6DC0's
    host calls, three of them unimplemented), and 25 `QuickSettingsApply::*` rows, one per 008D5B50
    sink call in the table above (five concrete). The foliage, ocean, clouds, shadow and
    download-content sinks are not called in the title-time state.
  - 2 removed: the two `(withheld)` rows.
  - `ProfileReset::xenon_state_is_two` 3 -> 4 (the settings half's own query).
  - 13 `quick settings:` notes.
  - **IJN18, JM06 and LOMP10:** one ring-scan `clear_37c` cell each (Kita2 10000.0 -> FLT_MAX,
    Lexington-class 01 10000.0 -> FLT_MAX, Asashimo 3259.5 -> 10000.0). These are the known
    heap-dependent cells (SHIP_AI_OPEN_ITEMS 140).
  - **USN21 only:** the summary line `mission ship ai free` moves corner_fwd 18099 -> 18087 and
    corner_back 9413 -> 9425, and 21 `ship ai step` lines for Intrepid (steps 39450..39750, mode
    heading) move only their `target` column, for example 2.6651 -> 2.6630. Throttle, rudder,
    desired_1d8 and hull are the same in those lines, and the deaths (514), units (595) and summary
    lines are identical. The free-bearing scan reads the avoid-zone searcher cache that 009D7050
    refills. That cache's bounds are never initialised, in the image too (SHIP_AI_OPEN_ITEMS section
    140, docs/AVOIDANCE_REFILL_DETERMINISM.md), so its first refill depends on the heap. The ON
    load opens and closes one more storage Lua state and allocates the views before the mission, which
    shifts the heap. The `clear_37c` cells in the three rows above come from the same cache and
    move the same way. This is attributed to that known heap noise, not to a setting: no setting
    this switch changes has a ship-AI reader. No run has yet separated the two causes.
- **The region.** Every Lua owner opened after the sign-in now runs `REGION="USA"`. Among this
  installation's scripts only `datatables\credits.lua` reads `REGION`, and the Lua sequences are
  identical.

**Save untouched.** See section 8.

## 7. Profile difficulty (cc15-prof2, `kProfileDifficultyCc15Bound`, OFF; scheduled by the user and cc9)

**Image.** game+6B0h *is* the profile's +60h: the profile block lives at game+650h, so +60h is
game+6B0h. No other cell is involved. 0058BF26 copies the record's +B4h into game+6ACh when it is not
3 (0058BF37). Otherwise, when the main-menu byte `[[00E198AC]+58h]+5Ch` is clear, 0058BF4D..0058BF58
load `[00E188A8]+6B0h` and store it into +6ACh. The image has no command-line override:
docs/CC13_DIFFICULTY_VALUES.md lists every writer of +6B0h (007FDB20 stores 1, the profile reader
stores the archived value, and the unreferenced 0051B7B0).

**Host (ON).** `chosen_difficulty()` (`src/game_hosts_mission.cpp`) returns `--difficulty` when it
is given, a harness rule kept on purpose. Otherwise it returns the bound profile owner's
`selected_difficulty_60` and logs `MissionStart::chosen_difficulty game+6B0h=profile+60h=N`. With the
profile load OFF that is 007FDB20's 1, the value the host uses today.

**Pairs.** `local\bin_diff` is da68a2635 with `kProfileDifficultyCc15Bound` true, run in the reference
env on USN04.
- OFF vs base: section 6, exit 0.
- OFF vs ON (`pair_diff` exit 3, gameplay moved, as expected): the ON log reads
  `chosen_difficulty game+6B0h=profile+60h=0` and `set_effective_difficulty game+6ACh=0` (OFF: 1).
  The save's `SelectedDifficulty` is 0. GetDifficulty (008AE030) returns 0. All 18 `SetSkillLevel`
  calls move from level 2 to level 5.
  `Scripts\missions\USN\usn_19_coralus.lua` maps difficulty 0 to `SkillLevel = SKILL_STUN` and
  `SkillLevelOwn = SKILL_ELITE` (lines 70-82), `RepairEnable(unit, true)` (159, 198), a four-wave
  bomber phase instead of five (569-586), and different plane-launch branches (638, 1382). So deaths
  fall from 53 to 27, hits from 762 to 324, shots from 12535 to 4520 and units from 81 to 57, and the
  first hit moves from 87.40 s to 90.70 s. Every moved row follows from the script's difficulty-0
  branch.

### Flipped ON with a harness pin (cc15-prof2, approved by -62; cc9 to ack before landing)

The switch is true. Every shared launcher passes `--difficulty 1` unless the caller already passes
`--difficulty`, so harness runs keep the pre-flip value:
- `tools/run_game.ps1` adds it after its config defaults. `BSP_DIFFICULTY_PIN=<n>` pins another value,
  and `BSP_DIFFICULTY_PIN=off` runs unpinned. The manifest gains `"difficulty": {value, source}`, where
  the source is `pin`, `caller` or `BSP_DIFFICULTY_PIN`.
- `scripts/run.ps1` applies the same rule.
- `tools/run_replay.py` sets `BSP_DIFFICULTY_PIN=off` for a manifest that has no `difficulty` field,
  so an older run replays with its own arguments. A newer manifest already records its
  `--difficulty` in `game_args`.

No existing argument or parameter was renamed, moved or removed.

The log shows which applied. A pinned run prints the existing
`--difficulty 1: chosen difficulty game+6B0h = 1 (default 1)`. An unpinned ON run prints
`MissionStart::chosen_difficulty game+6B0h=profile+60h=0` and then `set_effective_difficulty game+6ACh=0`.

Evidence. Builds: `local\bin_main` (the tree base with the switch false) and `local\bin_flip` (the
same base with the switch true). Both ran through the pinned launcher (`local\prof2_pin.ps1`).
- USN04 and BSM09, main vs flip, in the ref env and in the default env: `pair_diff` exit 0, and the
  strict Lua/dialog/message sequence is identical, on all four pairs.
- Wins rows, ref env:
  - PRCP01 and LOMP06: exit 0.
  - JM06, BSM01, USN12, LOMP10 and USN21: exit 1, gameplay identical, with an identical sequence. Each
    moves only one or two `clear_37c` ring-scan cells, or, in USN21, the free-bearing corner
    counts and one `target` value. That is the known searcher-cache heap noise (section 6): the two
    binaries differ only in this constant, which changes the code layout.
  - ESMP08, IJN18, USN01 and USN02: not yet paired. Their 2026-10-23 reruns failed at FMOD startup
    (create_result 78), the disconnected-session failure.
- Unpinned sample (`BSP_DIFFICULTY_PIN=off`, flip, USN04): the manifest records
  `{"value":"unpinned","source":"BSP_DIFFICULTY_PIN"}`, and the log shows profile+60h = 0 and game+6ACh = 0.

## 8. cc15-prof2 save safety

Every file under `Documents\Battlestations-Pacific` was recorded before the first run and after the
last (`local\prof2_save_before.txt`, `local\prof2_save_after.txt`). `options.txt` (226 bytes,
FF5CF395...), `save\0\player` (2182, 58E93D77...), `save\0\quick` (1806, 54E3F992...) and `save\0\valid`
(0, E3B0C442...) are identical, and so are the mtimes (UTC 2026-09-19 06:11:06 / 06:27:02) and the
directory mtimes (`save` 2026-05-04 14:59:43, `save\0` 2026-09-19 06:27:02). The quick read added
the storage calls `storage_query_1c` and `immediate_read_00bd4380` only; no writer is reachable (section 3).
The copy under `local\prof2_copy` is also unchanged after its run, including `valid`.

## 9. cc15-prof2 open items

- **Keyboard bindings** need the input configuration (00698A10, `Inputs.lua`) hosted first (section 6).
- **markerAlpha** (0.2) reaches no host reader. `HudMarkers::objective_detail_level` (00F88A00,
  `src/game_hosts_hud_world.cpp`, cc9's HUD) answers a constant 0.0. With the live value, 00643C9C's
  objective sweep (> 0.03) would run, which is a HUD change for cc9.
- **008D5B50's audio, renderer, input and GUI sinks** have no host implementation (the table in
  section 6). Natively GGame::OnInit runs 008D5B50 at boot too; the host runs neither call.
- **USN21's target column** is attributed to heap noise without a discriminating run (section 6).
