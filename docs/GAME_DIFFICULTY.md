# Game difficulty and per-unit skill level: verification, binding and re-baseline

Addresses: 008AE030, 008AE113, 008AE12A, 0058BDF0, 0058BF26, 0058BF37, 0058BF48, 0058BF58, 007FDB20, 00895250, 0089539A, 007B8AE0, 009565A0, 0095CCCC, 00927A80, 00927AC2, 006E6210, 00949750, 00949A53, 00945C30, 00945C92, 00945C9E, 009420A0, 00944210, 0094421B, 0043D8F0, 009F9D22, 00BD48F0

Packet `cc9_difficulty`, 2026-09-23. It follows `docs/PILOT_SKILL_LEVEL.md` section 5. The report is
`reports/game_difficulty.json`. All names are hypotheses, not recovered symbols. Nothing here is
ABI-compatible or game-validated. The runs are host runs of `bsp_game.exe`.

**Status: bound and measured.** Switch `kSkillLevelBound`, in
`include/bsp/game_hosts_script_orders.hpp`, defaults to true. False restores the old behaviour.

## 1. Verification (step 1)

* **GetDifficulty (008AE030)** reads `*(00E188A8)`. It returns 2 when `+1FE4h` is nonzero
  (008AE113 to 008AE123). Otherwise it returns `+6ACh` unchanged (008AE12A). The host's
  `game_non_campaign_flag()` answers 0, the single-player value, so the host returns game+6ACh.
  The existing rule `lua_binding_difficulty_value` already models the gate.
* **The writer (0058BDF0).** ESI is the mission record for the whole routine (0058BE11). EBX is 0
  from 0058BE2C to the epilogue, which a register filter of the listing confirms. The rule:
  - 0058BF26 loads `record+B4h` as a dword.
  - If it is not 3, 0058BF37 stores it to game+6ACh.
  - If it is 3 and the byte `[[00E198AC]+58h]+5Ch` is 0 (0058BF48), 0058BF58 stores the dword
    game+6B0h to game+6ACh.
  - If it is 3 and the byte is set, game+6ACh keeps its value.
  `src/mission_briefing_start.cpp` already implements this rule. The host answered 0 for game+6B0h
  and dropped the store.
* **The slot default.** The disk bytes at 0095CCCC read `mov dword [esi+390h], 1`, inside
  BSP_UnitGameObject_Construct. Being a constructor store, this is a default only. 009565A0 is the
  per-event writer.
* **The one writer the executable reaches.** 005C5600's launch arm, `bsp::MissionTreeScreensHost::
  set_effective_difficulty`, has no game-host implementation. 0058BDF0 is the only difficulty store
  on this executable's path.

## 2. The binding

| site | old | now |
|---|---|---|
| `chosen_difficulty()`, game+6B0h (`src/game_hosts_mission.cpp`) | 0 | **1**, the 007FDB20 profile-reset value; a loaded profile's `SelectedDifficulty` would replace it |
| `set_effective_difficulty` (0058BF37 / 0058BF58) | unimplemented record | stores game+6ACh into one process-wide word, `set_game_effective_difficulty_6ac` |
| `game_effective_difficulty()`, 008AE12A (`src/game_hosts_script_orders.cpp`) | 0 | the stored word |
| `entity_set_skill_level`, 0089539A | recorded only | resolves the entity to its units-host slot and calls `GameUnitsHost::set_skill_level_007b8ae0` |
| slot `pilot_skill_index` (unit+390h, bot+34h) | none | default 1; set by the call above, fanned out to live squadron members as 007ECF80 does |
| dive-bomb approach row (009F9D22) | SPNormal constants | `db_skill_row_14` captured once at arm construction; release altitude, aim error, pull gains, glide multiplier, power, brake and pitch ratio read from that row |

**The six rows.** `src/robot_config.cpp`'s reader (00901610) cannot be wired. The host records
004DC6A0 because it cannot build that reader's context. So the rows are this installation's
`scripts/datatables/robots.lua` PilotBot values, copied by line with the Lua line numbers in the
source comment. Only the dive-bomb fields are copied. The field names follow
`bsp::PilotBotParameters`, and a `static_assert` pins row 1 to the SPNormal constants the host used
before. The indices are 00901610's: Stun 0, SPNormal 1, SPVeteran 2, MPNormal 3, MPVeteran 4,
Elite 5. `luamw_init.lua:265-270` gives the scripts' `SKILL_*` the same numbers.

**Not modelled.** The weapon director and child re-skill that 009565A0 also performs. The launch
bag's `Skill = carrier+390h` for carrier launches. Neither is on the path of the two reference runs.
The gun bots read no skill (see section 6).

## 3. Where a plane's first skill comes from

`BSP_PropertyBag_GetSkill` (00927A80) reads the bag's `Skill`. When the bag has none, it reads
`Crew` (00D19264) and tail-jumps at 00927AC2 into **006E6210, BSP_Skill_FromCrewLevel**. In single
player that maps 0, 1 and 2 to themselves and 3 to Elite (5). Any other value, or no key at all,
gives 1. With `game+1FE4h` nonzero, 1 maps to 3 and 2 maps to 4.

**SpawnNew planes never reach the Crew arm.** BSP_SpawnManager_LuaSpawnNew (00949750) builds one
member element per `groupMembers` entry at 00949A53, through 00945C30 (RET 8). That routine
allocates the bag and calls **009420A0**, which seeds `Skill` = 1 in single player (3 otherwise),
then **00944210**, which copies the Lua member table over the bag through **0043D8F0**. 0043D8F0
is a generic table walk: IterateFirst and IterateNext dispatch to the bag setters by value type.
Its callee set was read but the body was not traced line by line, so treat it as partial. No
member in these scripts authors `Skill`. So a SpawnNew plane flies row 1, SPNormal, whatever its
`Crew` says.

**movieval.** Its scene record in `universe/scenes/missions/usn/usn_19_coralus.scn` (line 2439)
authors neither `Skill` nor `Crew`. Its property class `PlaneSquadronWNavpoint` defaults `Skill` to
SPNormal (`universe/library/plane.props:17`), and 00927A80 falls back to 1 anyway. **movieval flies
SPNormal.** That closes open item 1 of `docs/PILOT_SKILL_LEVEL.md`.

## 4. Predictions (made before the runs)

**USN01 (`usn_1_marshall.lua`, lines 178-360).** Difficulty 1 changes these things:
- Katori and the six convoy ships go from Stun to SPNormal.
- The US bombardment and carrier groups, seven ships, go from Elite to SPVeteran.
- The Katori and convoy ships get `TorpedoEnable(true)` and `RepairEnable(true)`, both false before.
- `Mission.JapAI` and `Mission.USAI` are set but never read.

The host honours none of these. TorpedoEnable (0089C8F0) is not wired in the game host.
RepairEnable is recorded (008AD4E8). Ship skill has no reader. **Prediction: every simulation line
unchanged.** Only the binding's own lines change: one difficulty store and 14 SetSkillLevel calls
applied.

**USN04 (`usn_19_coralus.lua`).** The first 225 s run phase 1, in `luaSpawnPh1Bombers`
(line 2572). `BomberWave < 2` gives two spawn waves 80 s apart, each with a Lex arm and a Town arm
of two groups.
- Difficulty 0: each group is three Vals (type 158) or three Kates (type 162), `Crew` 1, one
  member. That makes 8 SpawnNew calls and 24 aircraft.
- Difficulty 1: each group is four bombers plus a second member, two A6M Zeros (type 150). All are
  `Crew` 2 (lines 2649-2735). That makes 8 calls and 48 aircraft.
  - The Zero is `unit2` in `luaBombersSpawnedLex` and `luaBombersSpawnedTown`, with fire stance 2
    and `PilotMoveToRange` to the carrier.
- **Skill.** No SetSkillLevel reaches a phase-1 Japanese plane, and their bag Skill is 1 (section 3).
  So every Val flies SPNormal, as before. The dive mechanism cannot move.
- **US ships** get `SkillLevelOwn`, SPVeteran instead of Elite, through 18 calls. They have no host
  reader. `RepairEnable` goes from true to false, and it is recorded only.
- **Phase-1 completion.** Difficulty 1 also completes on `IJNFightersLex` empty (line 576). If every
  Lex Zero died, the mission would move to phase 2. With four per wave, that is not expected in
  225 s.
- **Expected squadron deltas.**
  - Dive-bomb aircraft: 12 spawned Vals plus 3 movieval become 16 plus 3.
  - Torpedo aircraft: 12 become 16.
  - Spawn batches: plus 8, the Zero batches.
  - Releases, damage and deaths move only through the extra aircraft and changed formations.
- Lines 1382-1600 (`luaIJNFleetManager`) are phase 3 and are not reached.

## 5. Runs

Both binaries were built from `e050a353c` (base `891bf426f`). The control is the same tree
with `kSkillLevelBound = false`. Binaries: `build\win32\ctl\` and `build\win32\treat\`. The runs
were made from the worktree root, `J:\PROG\battlestations-pacific-decompile-cc9-difficulty`:

```
./tools/run_game.ps1 -Exe build\win32\<ctl|treat>\bsp_game.exe -Log local\diff_<ctl|trt>_usn01.log -- --frames 3200 --press-start-frame 30 --menu-select USN01 --mission-frames 3000 --mission-frame-seconds 0.05
./tools/run_game.ps1 -Exe build\win32\<ctl|treat>\bsp_game.exe -Log local\diff_<ctl|trt>_usn04.log -- --frames 4700 --press-start-frame 30 --menu-select USN04 --mission-frames 4500 --mission-frame-seconds 0.05
./tools/run_game.ps1 -Exe build\win32\treat\bsp_game.exe -Log local\diff_trt_usn04_e9000.log -- --frames 9200 --press-start-frame 30 --menu-select USN04 --mission-frames 9000 --mission-frame-seconds 0.05
```

| run | damage | deaths | queued_hits | bomb_drops | torpedo drops | first_hit | dive-bomb aircraft/releases | torpedo aircraft/releases | log |
|---|---|---|---|---|---|---|---|---|---|
| USN01 control | 2892.0 | 0 | 35 | 0 | 5 | 62.55 s | - | - | `local\diff_ctl_usn01.log` |
| USN01 treatment | 2892.0 | 0 | 35 | 0 | 5 | 62.55 s | - | - | `local\diff_trt_usn01.log` |
| USN04 control | 13655.6 | 7 | 89 | 16 | 10 | 121.95 s | 15 / 16 | 12 / 10 | `local\diff_ctl_usn04.log` |
| USN04 treatment | 16025.3 | 8 | 116 | 18 | 13 | 123.30 s | 19 / 18 | 16 / 13 | `local\diff_trt_usn04.log` |

**USN01: as predicted, nothing moved.** After pointer and environment lines are stripped, the diff
is only these:
- `game+6ACh=1`.
- 14 `SetSkillLevel` lines: units 55-61 at 1 and 43-45 and 51-54 at 2.
- The two host-method rows that became concrete.
- `ship avoidance search refills`, 57 against 55. That counter is known to vary between identical
  runs.

## 6. USN04, squadron by squadron

The binding's own lines show `game+6ACh=1`. There are 18 SetSkillLevel calls, all level 2, all on
US ships. No Japanese unit receives one. `MissionPhase` stays 1 in both runs. Both runs make 8
SpawnNew calls, with 1 member per group in the control and 2 in the treatment. Spawn batches go
from 12 to 20, the eight new ones being the Zero pairs.

* **Zeros (`A6M Zero #1.2` to `#8.2|.-2`), 16 new aircraft.** None takes damage. None dies. No
  phase-1 completion by `IJNFightersLex`.
* **Lex dive group `D3A Val #1.1`.** Three Vals become four. The releases go from 6 (2+2+2) to 8
  (2+2+2+2). **The +2 in the mission's releases is `#1.1|.-4`, the new fourth aircraft.**
* **Town dive group `#3.1`.** The control makes 4 releases (2, 0, 2) and the treatment 4 (1, 1, 1, 1)
  over four aircraft. The total is the same and the split is new. The Town Vals' release lines
  name York-class02 as their target in the treatment.
* **Second-wave dive groups `#5.1`, `#7.1`.** 0 releases in both runs, since their arms are still
  in approach at 225 s (about 1190 arm ticks).
* **movieval.** 6 releases in both runs. Its row is SPNormal in both (section 3).
* **Kates.** Torpedo drops go from 10 to 13.
  - `#2.1` (Lex) 3 becomes 4 drops. The new `#2.1|.-4` runs at Lexington (45.4 m closest).
  - `#4.1` (Town) 3 becomes 4. The new `#4.1|.-4` runs at Yorktown.
  - `#8.1` 3 becomes 4.
  - `#6.1`'s one drop now reaches Lexington (79.8 m) instead of being nearest Fletcher-class02.
  - That makes +3, one per group whose fourth aircraft dropped inside the window.
* **Per-entity flips behind deaths 7 to 8.**
  - Three control deaths are gone. `B5N Kate #4.1|.-2` ends at 2 HP and `D3A Val #1.1|.-3` at
    8 HP; both are close calls in a denser formation. `movieval|.-3` takes 25 instead of 220.
  - Four treatment deaths are new: `B5N Kate #4.1|.-4` is a new aircraft, `B5N Kate #6.1` dies at
    221.01 s to Northampton-class01, `D3A Val #1.1|.-2` dies to Lexington, and **Lexington-class01**
    sinks.
  - `D3A Val #1.1`, `B5N Kate #2.1`, `#2.1|.-3` and `#4.1|.-3` die in both runs. Their times
    shifted by 1 to 4 s, which fits the denser formations.
* **Lexington-class01 sinks at 220.36 s.** It takes 8000 against 5992. The extra comes from the
  fourth Lex Kate's torpedo, `#6.1`'s torpedo and `#1.1|.-4`'s two bombs. That follows from the
  difficulty: four aircraft per group instead of three. **Finding, not traced:** the kill is credited
  to **York-class02**, a side-0 (US) escort. That ship deals 339 in the treatment and 0 in the
  control. The log has no per-hit source lines, so it is unproven whether York's AA rounds struck
  Lexington or whether the credit is an attribution slip. It is a host question independent of the
  difficulty; the difficulty only exposed it by pushing Lexington past 8000 HP.
  - `MissionFailedRan` stays nil. The script's `Mission.Lex.Dead` branch did not run in the
    remaining 4.6 s.
* **`first_hit` 121.95 s to 123.30 s.** The first hit is Kate `#4.1|.-3` on Yorktown in both runs.
  The four-aircraft `#4.1` formation drops about 1.4 s later.

**Every moved number traces to the branch change.** The branch adds aircraft and changes formation
sizes. No row moves through a skill change, as predicted. The Lexington kill credit is the one
finding, and its mechanism is not part of this packet.

## 7. The E-run (USN04, 9000 mission frames, treatment)

Log `local\diff_trt_usn04_e9000.log`: 19109.5 damage, 15 deaths, 26 bomb drops, 16 torpedo drops
and `MissionPhase=2`. The per-group releases and the 12 water contacts are in `docs/GAME_EXECUTABLE.md`,
"Mission reference baselines, 2026-09-23 (difficulty 1)".
* **Every death inside 225 s matches the 4500-frame run to the hundredth of a second**, which
  confirms the run is deterministic across the two lengths.
* **Lexington sinks at 220.36 s and the mission still reaches phase 2.** `MissionFailedRan` stays
  nil. The phase-1 tick (`usn_19_coralus.lua:586`) calls `luaMissionFailed` when `Mission.Lex.Dead`
  is set, so the host apparently never sets the sunk carrier's `Dead` field. **Open finding**, not
  traced here.

## 8. Decision and open items

* **Decision: keep `kSkillLevelBound` true.** Difficulty 1 is the image's fresh-session value. The
  reference rows are re-baselined on it.
* **Gunnery (for the integrator).** US ships now carry SPVeteran in their slot, but no gun bot reads
  it. The one-line hookup belongs to the owner of `src/game_hosts_gunnery.cpp`: give the gun bot's
  skill index `units.skill_level(unit_index)`, BSP_GunBot_Attach's `unit->vtable[12Ch]()` in the
  host. It was left out on purpose.
* **Launch bags.** A carrier launch's `Skill = carrier+390h` (006EC98D) is not carried into the
  launched plane's slot. It matters from USN04 phase 3 on (lines 1382-1600), which the reference
  runs do not reach.
* **The profile.** `save\0\player` is the archive container, zlib block framing, whose native
  routines are 00BD48F0, 00BD49B0, 00BD4A70 and 00BD4860 (`docs/ARCHIVE_COMPRESSION_BUFFER.md`).
  It is compressed, not enciphered. Its `SelectedDifficulty` (profile+60h, see
  `docs/GAME_PROFILE_ARCHIVE.md`) was not decoded in this packet.
* **Lexington's credited killer**, from section 6.
