# cc15-chg: the challenge rows start from the training list

Packet cc15-chg, 2026-10-23, at cc9's request. The goal is that `--menu-select CHG06` (and the other
challenge rows) start the way the image starts them. The new switch is `kChallengeGroupPageCc15Bound`
(`include/bsp/game_hosts_mission.hpp`, OFF). The code is the page-8 arm in
`GameMissionHost::advance` (`src/game_hosts_mission.cpp`, step `MissionList`). Descriptive names are
hypotheses.

## 1. Where the challenge rows live

This installation's `scripts/datatables/missiontree.lua` (modded, see the game-install note) puts the
CHG rows in the first mission group, `groupName = "Training grounds"` (line 65, comment
"Challenge Missions"). The rows are:
- CHG01 (line 457), CHG07, CHG02, CHG05 and CHG08.
- CHG06 (line 704, `sideMission = true`, `forcedDifficultyLevel = 2`).
- CHG09 and CHG04.

CHG03 (line 662) is inside a `--[[ ... ]]` block comment, so this installation has no CHG03 row and
`--menu-select CHG03` cannot be run.

00580940 sends group 0 to page 8 (`MainMenuPage::TrainingGrounds`, `docs/MAIN_MENU_MISSION_DETAIL.md`).
The OFF walk therefore publishes `group=0 mission=12 page=8` for CHG06, then calls 0058C010, which
rejects every group except 1..4 at 0058C092. With no detail page built, the walk stops:
"no mission-detail page was built for published group 0".

## 2. The image's path: 00598B60's page-8 arm

00598B60 (`BSP_MainMenu_OnListboxActivate`, ECX = screen+8) is the list-activation handler, and it
dispatches on the page at 00E08874. Pages 4..7 are the campaign arm, which ends at 00599318 in
0058C010, the mission-detail page. Page 8 has its own arm, read from the listing:

| Address | Instruction | Meaning |
|---|---|---|
| `00598BCF` | `CMP EAX,8 / JNZ 00598C06` | page 8 only |
| `00598BD4` | `MOV ECX,[EDI+1B0h]` / `CALL 00A9C990` | the activated row's data on the list at screen+1B8h |
| `00598BDF` | `LEA ECX,[EDI-8]` | ECX = the screen |
| `00598BE2`, `00598BE7` | `MOV [00E194DC],EAX` / `MOV [00E08878],EAX` | selected-row mirrors |
| `00598BEC` | `CALL 005922F0` | `BSP_MainMenu_PlaySelectedMission`, straight from the list |

The image therefore publishes no challenge "group page" or mission-detail page. A training-list row
starts through 005922F0, the same play action the campaign detail page's `globals.continue` footer
reaches through 00592640. Unlike the campaign arm, the page-8 arm does not clear screen+5Ch (00599315)
and does not write screen+564h. The host's `main_menu_flag_5c()` already answers false.

With the switch ON, a selection published on page 8 takes this arm. The host reads the row from the
published selection (the mission `selected_record()` returns), records 00598B60 as unimplemented
(the dispatcher itself is still not reconstructed), and calls the existing `start_briefing_005922f0()`.
The walk then continues at `BriefingStart`, as it does after a detail page. A campaign selection
(pages 4..7) never reaches the new branch. No harness-only route was needed.

## 3. Difficulty: the record's level wins over the pin

0058BF26 reads the record's `forcedDifficultyLevel` (record+0B4h, default 3 from the reader at
005C7641) and compares it with 3 at 0058BF2C (`src/mission_briefing_start.cpp`,
`run_start_selected_mission_0058bdf0`):
- When it is 3, 0058BF52/0058BF58 copy the chosen difficulty game+6B0h (the `--difficulty` pin, or
  profile+60h) into game+6ACh, guarded by the main-menu screen's +5Ch byte at 0058BF48.
- Otherwise, 0058BF37 stores the record's own value into game+6ACh, and game+6B0h is not read.

CHG06 holds 2, so the image runs it at effective difficulty 2 whatever `--difficulty` says. The ON
CHG06 log shows `--difficulty 1: chosen difficulty game+6B0h = 1` and then
`MissionStart::set_effective_difficulty game+6ACh=2`.

`tools/run_game.ps1` now records this in the manifest after the run. It reads `difficulty.effective`
from the log's `set_effective_difficulty` line, and `difficulty.effective_source` from the record's
level on the `side blocks:` line. For CHG06 the manifest reads:
`{"value":"1","source":"pin","effective":"2","effective_source":"record forcedDifficultyLevel 2"}`.
A campaign row reads `"effective_source":"chosen game+6B0h"`. No launcher argument changed.

## 4. Evidence

Builds:
- `local\bin_chgbase`: origin/main a7a80cd70.
- `local\bin_chgoff`: this change with the switch false.
- `local\bin_chgon`: a temporary ON build (full `scripts/build.ps1`, tests pass).

Ref env (`BSP_DEATH_TABLE=1`, `BSP_GUNNERY_RNG_STREAMS=1`). Each run went through the pinned launcher.

- CHG06 ON (`--frames 2600 --press-start-frame 30 --menu-select CHG06 --mission-frames 2400`): exit 0.
  - The run takes the page-8 arm and loads `Scripts/missions/Chg/chg_6_forcez.lua`.
  - It runs 2400 of 2400 mission frames: 228 world entities, 2 landscapes, an intro movie camera.
  - Its effective difficulty is 2.
- CHG06 on the origin/main build stops after the mission list, as described in section 1.
- Open item for cc9 (not this lane's area): CHG06's `luaForceZMission_think` fails 35 times at
  `scripts/global/commandhelpers.lua:330`, because `recon[targetUnit.Party][allegiance]` is nil.
  The recon unit lists are empty: the world summary shows `clears_recon=40` and counts `0/20/0/...`.
  This is mission-side, past the menu path.

Pairs (pair_diff, plus a strict index-by-index diff of dialog, message and Lua lines). Every log has
"rng streams: on".

| Pair | Rows | pair_diff | Sequence |
|---|---|---|---|
| OFF vs origin/main | USN04, USN02, LOMP06 | exit 0 each | identical |
| ON vs OFF | USN04, USN01, USN02, USN12, PRCP01, USN21, LOMP06, LOMP10, JM06, ESMP08, BSM09, BSM01 | exit 0 each | identical |
| ON vs OFF | IJN18 | exit 1, gameplay identical | identical (7367 lines) |

IJN18 moves only two ring-scan `clear_37c` cells (Kita2, Convoy1: 10000.0 and `~` trade places), the
known heap noise (SHIP_AI_OPEN_ITEMS 140). No campaign row reaches the new branch. The CHG06 manifest
sample is in section 3. A campaign sample (USN04) reads
`{"value":"1","source":"pin","effective":"1","effective_source":"chosen game+6B0h"}`.

The save files under Documents\Battlestations-Pacific (SHA-256, size and mtime) are unchanged from
the 2026-10-19 record after every run.

Not done:
- The 00598B60 dispatcher is still not reconstructed as a whole. Only its page-8 arm is followed.
- No Ghidra annotation was made in this packet.
