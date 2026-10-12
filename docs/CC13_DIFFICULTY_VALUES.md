# What GetDifficulty returns, and the `difficulty=3` in cc9's USNOS log

Addresses: 008AE030, 008AE113, 008AE12A, 0058BDF0, 0058BF26, 0058BF37, 0058BF48, 0058BF58, 005C5600, 005C576F, 005C577A, 005C578D, 005C6A70, 005C7641, 00592640, 0059269B, 007FC490, 008D45D0, 008D466A, 0051B7B0, 007FDB20

Packet cc13-difficulty1, 2026-10-12, a read-only investigation for cc9. No C++, ledger or Ghidra
changes. All names are hypotheses, not recovered symbols. Builds on `docs/GAME_DIFFICULTY.md`,
`docs/CC13_PROFILE_SAVE.md` and `docs/GAME_PROFILE_ARCHIVE.md`. Evidence status: image facts are
from disk bytes (Capstone) and existing ledger records; host facts from `src/` at the tree base;
log facts from cc9's `s43_p6_osnf.log`, `s43_p6_osqf.log` and `s43_p6_osrf.log`
(`J:\PROG\battlestations-pacific-decompile-cc9-ships43\local\`, written 2026-10-12 00:01-00:04).

## Answer in five lines

1. `difficulty=3` in the log is the mission record's `forcedDifficultyLevel` (record+0B4h), not GetDifficulty's value.
2. 3 is the reader's default (005C7641) and means "use the player's chosen difficulty"; USNOS has `forcedDifficultyLevel = nil`.
3. The image's start routine (0058BF26) turns 3 into game+6B0h (the chosen difficulty); every image writer of game+6ACh stores 0, 1 or 2.
4. The host does the same: it logs `MissionStart::set_effective_difficulty game+6ACh=1` (log line 697) before the script's GetDifficulty call (line 5379), and GetDifficulty returns game+6ACh, so us_osumi.lua sees 1.
5. Verdict: image-faithful for the intended Normal setting; the `== 1` branches run (SKILL_SPNORMAL, RepairEnable true); nothing is skipped.

## 1. The native and its field (image)

`GetDifficulty` is row 00E0C0D8, body 008AE030 (`BSP_LuaBinding_GetDifficulty`). It reads the
game object `*(00E188A8)`, pushes the literal 2 when `+1FE4h` (the non-campaign / multiplayer flag)
is non-zero (008AE113 to 008AE123), and otherwise pushes the dword game+6ACh (008AE12A). So
multiplayer sessions always see 2 and single-player sessions see game+6ACh.

**Writers of game+6ACh.** `local/dif1_scan.py` decodes every instruction in `.text` whose memory
displacement is 6ACh or 6B0h (58 hits). Positive controls: the known stores 0058BF37 and 0058BF58
and the known read 008AE12A are all found. Integer stores through a `[00E188A8]` base:

| site | routine | value stored |
|---|---|---|
| 0058BF37 | 0058BDF0 `BSP_MainMenu_StartSelectedMission` | record+0B4h when it is not 3 |
| 0058BF58 | same | game+6B0h when record+0B4h is 3 and the main-menu byte `[[00E198AC]+58h]+5Ch` is clear |
| 005C577A | 005C5600 `BSP_MissionTree_StartSelectedMission` | record+0B4h when not 3 (005C576F `cmp esi,3`) |
| 005C578D | same | game+6B0h when record+0B4h is 3 |
| 0059269B | 00592640 (the main-menu `continue` footer command) | the first dword of the profile checkpoint record copied by 007FC490, i.e. the `__difficulty` saved from game+6ACh (`docs/CC13_PROFILE_SAVE.md`); 00592640 also sets byte +5Ch to 1, which is why 0058BF48 then keeps the checkpoint's value |
| 008D466A | 008D45D0 `BSP_Settings_RestoreSelectedUserControls` | the platform profile's difficulty mapped 2 -> 2, 1 -> 0, other -> 1 (008D465A to 008D4664) |

0051B7B0 also writes the widget index 0, 1 or 2 into both +6ACh and +6B0h, but it is unreferenced
(its ledger correction). The remaining scan hits are float stores (`fstp`) or other structs
(`[esi+...]`, `[ebp+...]`, misdecodes at 00424BAx). game+6B0h is the profile's `SelectedDifficulty`
(game+650h profile, +60h), default 1 after 007FDB20, otherwise the archived value.

**Range.** Every reachable writer stores 0, 1 or 2, or copies a value that was itself game+6ACh or
game+6B0h. So GetDifficulty returns 0 (easy), 1 (normal) or 2 (hard) in single player, and 2 in
multiplayer. Mapping: menu/profile pick -> game+6B0h; mission with `forcedDifficultyLevel` set ->
that value; mission without it -> game+6B0h; continue-from-checkpoint -> the checkpoint's saved value.

## 2. Can it be 3?

Not through any writer found. 3 exists only as the record value "ask the player", consumed by
0058BF26 and 005C576F before game+6ACh is stored. `BSP_MissionTree_ReadMissionFromLua` (005C6A70)
reads `forcedDifficultyLevel` into record+0B4h with default 3 at 005C7641.

This installation's `scripts/datatables/missiontree.lua` (mtime 2025-06-02 19:08, modded): 136
missions have `forcedDifficultyLevel = nil` (so 3), 4 have 0, 6 have 1, 5 have 2. USNOS (line 7514)
has nil and `briefingGuiLayer = "USN14"`, which is the `briefing="USN14"` in the same log line.

Scripts: 216 files call GetDifficulty, almost all as `Mission.Difficulty = GetDifficulty()` (173)
or `this.Difficulty = GetDifficulty()` (25). Comparisons of the stored value: `== 0` 1174,
`== 1` 1126, `== 2` 1186, plus ranges (`>= 1` 34, `< 2` 10, `<= 1` 9, `> 0` 7, `~= 0` 6, `~= 2` 3,
`> 1` 2). Only two compare with 3: `missions/COTP-IJN/PRCPIJN/jm13.lua:1103` and
`prcpjm13.lua:1114` (mod content, destroyer tweak), which is dead code given the range above.
`missions/COTP-USN/us_osumi.lua` (mtime 2024-10-29) stores GetDifficulty at line 356 and branches on
0, 1, 2 for `Mission.SkillLevel` (359-363) and per unit for TorpedoEnable/RepairEnable (2253-2261).

## 3. The host

- The log line `side blocks: ... difficulty=%u` is `src/game_hosts_mission.cpp:1533`, printing
  `record->screen.difficulty`, the record+0B4h field read by `src/mission_tree_data.cpp:164`
  (`forcedDifficultyLevel`, default `kMissionDifficultyFromPlayer` = 3).
- `src/mission_briefing_start.cpp:201` implements 0058BF26: on 3 and a clear main-menu flag it stores
  `chosen_difficulty()`, which is 1 (`src/game_hosts_mission.cpp:975`, the 007FDB20 reset value;
  `kSkillLevelBound` is true) into the process word `set_game_effective_difficulty_6ac`.
- `src/lua_binding_core.cpp:97` (GetDifficulty) returns `game_effective_difficulty()` when the
  non-campaign flag is 0, which `src/game_hosts_script_orders.cpp:322` answers with that word.
- All three logs show `MissionStart::set_effective_difficulty game+6ACh=1` at line 697 and the
  `binding GetDifficulty` call at line 5379 (luaStageInit), and the gunnery summary reports
  `level=1`. The harness does not set a difficulty option; the 1 is the profile-reset default.

## 4. Verdict

`difficulty=3` is image-faithful: it is the authored "use the player's choice" record value, and
both the image and the host resolve it to game+6B0h before scripts run. The scripts see 1 (Normal),
so us_osumi.lua takes `SKILL_SPNORMAL` and `TorpedoEnable/RepairEnable(unit, true)`. To test other
difficulties, change `chosen_difficulty()` (game+6B0h), not the record value. Open: the host never
models a loaded profile's `SelectedDifficulty` or the `continue` path; neither is on cc9's path.
Not game-validated (no in-game run compares GetDifficulty).
