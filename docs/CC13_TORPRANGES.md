# cc13-torpranges: census rows 15 and 18 (torpedo moveto ranges, run profile row)

Packet cc13-torpranges, from docs/CC13_STUB_CENSUS.md rows 15 and 18. Names are hypotheses,
not recovered symbols. No game runs; the target heights come from current-main OFF logs.

| Row | Site | Verdict |
|---|---|---|
| 15 | 009BDE80 SetRanges at 009D48CF | LIVE, small. The base misses the target-height lift. New OFF switch `kTorpedoMoveToRangesBound`. |
| 18 | 009D0484 run profile row | Equal at the harness difficulty (1). The census row was stale. This installation's saved profile selects difficulty 0, which was routed to the lead. |

## Row 15: what the image stores and reads

**The store.** In 009D4850 BSP_BotTaskTorpedo_TickArm (ESI = task, EDI = approach at task+3F8h):
- 009D486F calls 009D3420, the approach update.
- 009D4874-009D4895 load +134h, then the double 15.0 at 00CF3F20. FCOMIP and JBE then take
  approach+7Ch when 15.0 <= +134h or the compare is unordered, and +80h otherwise.
- 009D48AC-009D48CC compute A = `FLD [EDI+78h]; FADD [EDI+74h]; FSTP float` twice.
- 009D48CF calls 009BDE80(task+544h; A, A, range).

009BDE80 is the whole body: three MOVSS stores to +30h, +34h and +38h, then RET 0Ch.

**What runs between the store and the read.**
- 009D48DE calls 009D4030 TransitionRule. Its only state changes go through 009D3E40 SetState,
  which calls the old state's exit (vtable +8h) and the new state's enter (+4h).
- The torpedo state vtables (read from the PE) are 00D20AEC (moveto), 00D20AB8 (follow),
  00D212C4, 00D212E0, 00D212FC and 00D21320. A moveto tick follows a transition only into
  moveto, whose enter is 007B3DB0, a bare RET.
- The exits that can precede it are 007B3DC0 (RET), 009BDE40, 009D0C00 (trivial), 009D1350 and
  009D2570. Between them they write [state+4]+A8h, cmd+4Ch and [state+4]+2Ch, and call
  007B8A90 and 007BBBA0 on the unit. None of them writes approach+74h, +78h, +7Ch, +80h or +134h.
- 009D48E7-009D48F6 then call the current state's vtable[0Ch].

**The read.** 009C18C0 is the moveto tick (vtable 00D20AEC +0Ch):
- 009C18CB: ESI = [state+2Ch], the observed target. 009C2AC0 stores it from its second
  argument, which 009D2E55 passes as the task target (ConstructStates' second argument).
- 009C18FD-009C1905: [ESP+20h] = target+100h, the target's world Y. 009C18E7 refreshes the
  pose first when the dirty byte +C8h is set.
- 009C19EB: when +2Ch is 0 the glide is skipped (JZ 009C1B69).
- 009C19F1-009C1A19: `FLD [EDI+34h]; FADD [ESP+20h]` gives L, and `FLD [EDI+30h]; FCOMIP` with
  JBE keeps L. So base = +30h only when +30h > L, and L otherwise.
  - The stack is balanced between the two uses. 009C1993 PUSHes one argument for the
    vtable[1Ch] call 009C1850, and 009C1850 ends with RET 4.
- 009C1B09: rangeLow = +38h, passed to 009FBA50 at 009C1B17.

Since +30h = +34h = A, the image's base is A + max(targetY, 0).

**Other readers and writers.**
- The listing confirms the earlier docs/CC13_MOVETO_AUDIT.md scans.
- The only readers of +30h/+34h/+38h in the moveto class are in 009C18C0.
- The only other writer is the constructor 009C2AC0, at 009C2B17, 009C2B22 and 009C2B41.
- The task-relative displacement scan (task+574h/+578h/+57Ch) has one hit outside the class,
  in 009CC8F0, the strafe debug draw.

## Row 15: the host and the difference

`run_move_to_tick_009c18c0` (src/game_hosts_units.cpp) reads the approach fields directly:
- base = `alt_floor_74 + alt_margin_78`;
- rangeLow = `elapsed_134 >= 15.0f ? speed_late_7c : speed_early_80`.

rangeLow is the image's value except when +134h is NaN; the host's +134h accumulator starts at
0. The base lacks the target lift. The earlier audit called this equal on the assumption that
target origins sit at or below 0. The current logs show they do not.

Target origin Y from the `hull_aim` lines (`origin=(x y z)`), current-main OFF logs of
2026-10-19 (local/tr_shipy.py):

| mission | log | samples | Y > 0 | max |
|---|---|---|---|---|
| USN21 (usn_sibuyan.lua) | cc9-lua52 l70r_n_vu21_main_ref | 533 | 123 | +1.3 Tone; Chikuma +1.1, Nagato +1.0, Kumano +0.5 |
| USN21 | cc9-physics1 p24n_off_u21f | 533 | 123 | +1.3 Tone |
| USN21 | cc9-ships44 s60_m1off_u21 | 444 | 100 | +1.3 Tone |
| USN01 (usn_1_marshall.lua) | cc9-lua52 l70r_n_u1s_main_ref | 97 | 19 | +0.2 Convoy4 |
| USN01 | cc9-physics1 p24n_off_u1i | 85 | 7 | +0.2 Convoy4 |
| JM08 (PRCPJM08.lua) | cc9-gunnery52 g54_boff{s,d}_j8 | 0 | - | no samples |

So in USN21 the torpedo moveto base runs up to about 1.3 m low while the target heaves above
its rest height. LIVE, but small. It was routed to the lead when found.

## Row 15: the switch

`kTorpedoMoveToRangesBound` (OFF):
- **The arm.** The torpedo arm binding's `refresh_move_to_ranges` (009BDE80 at 009D48CF) stores
  A, A and the +7Ch/+80h pick into the new slot fields `torpedo_moveto_alt_30/_34/_range_38`.
  This runs after the approach update, as the image does. The +134h test is written so that an
  unordered compare takes +7Ch, as the image's JBE does.
- **The tick.** `run_move_to_tick_009c18c0` reads the three fields. base = +30h if
  +30h > (+34h + targetY), else +34h + targetY. rangeLow = +38h. targetY is the world Y of the
  slot `torpedo_task_target_plus_one` names, the task target.
  - With no target the image skips the glide entirely. The host keeps the glide and drops only
    the lift. That case cannot be reached, because the host's task always has its target.
- **Precision.** The main thread runs x87 at PC24, so each FADD/FSTP pair is one float
  addition, which is what the host computes.
- **OFF** keeps the old reads and records 009BDE80 as unimplemented.

**ON data lifetime.**
- **Writer.** The three slot fields have one writer: the arm binding's `refresh_move_to_ranges`.
  `bsp::torpedo_task_arm_009d4850` calls it on every arm tick, straight after the approach update
  (src/torpedo_task_arm.cpp:331-338) and before any return.
- **Reader.** `run_move_to_tick_009c18c0` is the only reader. Its only call is the state dispatch
  after the arm call in `run_torpedo_task_arm_009d4850`, so each read sees the values stored
  earlier in the same tick.
- **No reset.** Nothing resets the fields on a state change or a task re-install. None is needed,
  because a store always comes before the read in the same tick.
- **The image's lifetime** is the same in effect.
  - The fields live in the moveto state object at task+544h. They are set by the constructor
    009C2AC0, through ConstructStates 009D2E19-009D2E57, using the same formula.
  - 009BDE80 then overwrites them on every TickArm before the state tick.
  - A retarget builds a new task (009D3EF0 retire, then install), and with it a new state object.
- **[state+2Ch] on the ON path** is `torpedo_task_target_plus_one`. It is set at install from
  `command_target_plus_one` (src/game_hosts_units.cpp, the install block). When the two differ,
  the task is retired and re-installed (`kAttackTaskTargetRetireBound`), as 009D2E55 passes the
  task's target to 009C2AC0.
- **Uncertainty.** In the image, +2Ch is an observer link: 009C2B4B and 00694A60 register it,
  and 006952A0 unregisters it. If the target object is destroyed during a run, the image may
  clear +2Ch, which would skip the glide at 009C19EB. The host keeps the dead target's last
  position. That callback was not read.

The flip is cc9's call. The expected effect is a lift of up to about 1.3 m in the torpedo glide
base in USN21. USN01 lifts at most 0.2 m.

## Row 18: 009D0484, the run profile row

**The census row is stale.** The host already selects the row by the unit's skill index:
`kTorpRows[pilot_skill_index]` under `kSkillLevelBound` (ON). pilot_skill_index is unit+390h,
the pilot bot's +34h (docs/GAME_DIFFICULTY.md, docs/IJN_SKILL_ROWS.md). The leftover
`log.unimplemented("TorpedoApproach::run_profile_record_14h")` was what kept the row in the
census, and this packet marks the site implemented.

**Where the image takes the difficulty.** docs/CC13_DIFFICULTY_VALUES.md has the details.
- GetDifficulty (008AE030) returns game+6ACh in single player.
- The three WINS missions have `forcedDifficultyLevel` = nil, so record+0B4h = 3 and the logs
  print `difficulty=3`.
- 0058BF58 then copies game+6B0h, the profile's `SelectedDifficulty` (profile+60h).
- Writers of game+6B0h:
  - 007FDB20's profile reset stores 1 (007FDFE2/007FE01B).
  - The profile reader (docs/GAME_PROFILE_ARCHIVE.md) stores the archived value.
- The scripts map the difficulty to skill rows: usn_sibuyan.lua:227-241,
  usn_1_marshall.lua:178-192 and PRCPJM08.lua:73-92. The rows are:
  - Mission.SkillLevel (enemy): 0 gives Stun, 1 SPNormal, 2 Elite (SPVeteran in PRCPJM08).
  - SkillLevelOwn: 0 gives Elite, 1 SPVeteran, 2 SPNormal.
- SetSkillLevel writes them into unit+390h.

**The harness.** The host loads no profile, so `chosen_difficulty()` returns 1, the reset value
(src/game_hosts_mission.cpp). All three WINS logs print
`MissionStart::set_effective_difficulty game+6ACh=1`. At 1, the host's rows are the image's.

**This installation's default.** C:\Users\sqz269\Documents\Battlestations-Pacific\save\0\player
(2182 bytes, 2026-09-18) was read read-only. It is a raw deflate stream at offset 6
(local/tr_profile2.py) and holds `["Difficulty"] = 0` and `["SelectedDifficulty"] = 0`. The
installed game therefore starts these missions at difficulty 0. The torpedo rows that follow:

| side | difficulty 1 (harness) | difficulty 0 (this profile) |
|---|---|---|
| own (SkillLevelOwn) | SPVeteran {5, 800, 1200, 0.5} | Elite {5, 800, 1200, 0.5}: the same torpedo row |
| enemy (SkillLevel) | SPNormal {12, 450, 650, 0.7} | Stun {12, 350, 600, 0.7} |

Every other `Mission.Difficulty` branch in the scripts also changes at 0. That makes the
difficulty a harness-wide choice, not a torpedo fix. cc9 decided to keep 1 (the image's
fresh-profile value) as the default and to add an opt-in `--difficulty N` harness flag that
feeds `chosen_difficulty()`. The flag is a separate commit. No switch was built for row 18.
This packet changes only the log record at the 009D0484 site: `unimplemented` becomes `done`.
That is log text only; no behaviour changes.

## The --difficulty flag (harness option)

`bsp_game --difficulty N` (N from 0 to 2) sets the player's chosen difficulty, game+6B0h.
- **The path.** `GameMissionHost::set_chosen_difficulty` makes `chosen_difficulty()` return N.
  On the briefing start path, 0058BF26 in src/mission_briefing_start.cpp copies that into
  game+6ACh when the record's forcedDifficultyLevel is 3, as 0058BF58 does in the image. A
  record that forces 0, 1 or 2 keeps its own value. GetDifficulty then returns game+6ACh.
- **Bad values.** A value outside 0..2, a non-number or a missing value exits 2 with
  `--difficulty needs a level from 0 to 2`.
- **The log.** When the flag is set, startup logs one line:
  `--difficulty N: chosen difficulty game+6B0h = N (default 1)`.
- **Without the flag,** `chosen_difficulty()` still returns 1 and nothing else changes.
- **N = 3 is rejected.** The image never stores 3 in game+6ACh: 3 is only the record's "ask
  the player" value. The image cannot run at 3. The three-entry HPMultipliers,
  PlayerCheatMultipliers and LockRadiusMultipliers tables are indexed by game+6ACh (loaded at
  00442B23, 0095DA4B and 00910417) with a bounds check: 00442B32 calls the checked operator[]
  0043F880, and 0095DA6E and 0091043A inline the check and call 00BF6713 (terminate). The host
  would instead silently drop the multipliers: gunnery.cpp skips the 1.8 HP scale, and
  lock_radius_multiplier(3) is 0. Every Lua `Difficulty == 0/1/2` arm would also miss.
- **Use 0** to play the rows the way this installation's saved profile does.

## Status

- Row 15: reconstructed under an OFF switch and build-tested. Not game-validated.
- Row 18: verified equal at the harness default. The profile divergence is open with the lead.
