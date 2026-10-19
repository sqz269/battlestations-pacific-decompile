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
