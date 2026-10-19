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
