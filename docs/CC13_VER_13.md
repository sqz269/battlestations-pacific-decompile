# CC13 VER 13: when a dying ship stops being a target

Packet cc13-ver13 (Claude, cc13 lineage), 2026-10-13. A verification pass: for each consumer that picks
or keeps a target, which entity byte makes a sinking ship stop being chosen or kept, when that byte is
set relative to the kill, and what cc9's host tests instead. Sources: `bsp.py disasm-raw` (Capstone over
the PE on disk) and `local/v13_bytescan.py`, a census of every `byte ptr [reg+5Dh/5Eh/5Fh/60h]`
instruction in `.text` validated by decoding one instruction at each hit. No game runs; the log evidence
is from existing cc9 logs. Every name is a hypothesis, not a recovered symbol. Line numbers are from
this branch (origin/main 4bf73d3f4 plus the D2 commit).

## 1. The bytes and when they are set

| byte | writer | when, for a ship shot to death in fixed step N |
|---|---|---|
| `+60h` | Destroy `00926C80` (`00926CD5`), from `0077D1A0` = `vtable[70h](1)` at `00958DBE` | step N, at the health write |
| `+5Dh` | `00926390` (`0092639B`, with `+60h`), dispatched by FlushPending `009273A0` (`0092747F`) over the destroy list | step N, at the row-15 flush (`00875EC9`), after the step's subsystems |
| `+5Eh`, `+5Fh` | the kill-list block of `009273A0` (`009274CE`-`009274D6`, also `+5Ch = 0`) and `009263C0` | when the wreck reaches KillDepth (`00825F20` -> `00926D90(1)`), or another Kill |
| `+5Ch` | cleared with `+5Eh` at the kill list | the same, KillDepth |

So a dying ship is "dead" to every `+60h` reader from the kill step, to every `+5Dh` reader from the end
of that step, and stays registered (`+5Ch`) and un-removed (`+5Eh`) for the whole wreck descent.

Host mirrors: gunnery `UnitState::dead` / `GameGunneryUnitRow::sunk` (set in `kill_unit`, the `+60h`
moment); `GameUnitsHost::unit_flag_005d` (`state->simulate`, the `+5Dh` byte), set for ships by the row-15
flush (game_hosts_ready.cpp:564, `kSunkShipFlushBound` ON) and for planes by `kPlaneDeathFlagsBound` (ON);
`unit_active` / `GameUnitRow::active` is `+5Ch`, cleared only at the kill list.

## 2. Consumers, image against host

| consumer | image test | becomes true | host test | verdict |
|---|---|---|---|---|
| gun bots (turret, heading, lead, ballistic, depth charge) `008FFA20`, `008FFF20`, `00902920`, `009030C0`, `008FC080` | owner and target `+5Dh` clear and other party (`008FB890` inlined), else `vtable[38h](0)` | end of kill step | game_hosts_gunnery.cpp:6912 `unit_state[target].dead` | match (the host drops at the kill, same step) |
| TorpedoBot accept `008FC010` | `+5Dh` (`008FC03A`) among four tests | end of kill step | range only (docs/CC13_GPR_40.md 1.3) | known: docs/CC13_GPR_40.md 1.3 |
| unit gunnery think `00864FE0` / `00862820` | `+5Ch` set, `+5Dh`/`+60h`/`+5Eh` clear | kill step | `target_is_engageable_00862820` with gunnery `dead`, :5353 | match |
| director release `00835F60` | fire target `+238h` kept only if nav point (41h) or live hostile | kill step (`+60h`) | not modelled | **D2**, fixed behind `kDirectorDeadFireTargetReleaseBound` (OFF) |
| director attackmove arm `00836B45` | `0043F080` liveness, side | kill step | game_hosts_commands.cpp:2966 `live_0043f080` = gunnery dead | match |
| director push `0071D6D0` | `+5Dh` (`0071D712`) | end of kill step | `released_05d`, game_hosts_commands.cpp:3469 | match |
| ship auto-target scan `009F5B70` | `+5Dh` (`009F5BFA`) | end of kill step | `unit_flag_005d`, game_hosts_ship_ai.cpp:11159 | match |
| auto-target keep `009F5E37`-`009F5E60` | locked and `0071D6D0` accepts | end of kill step | game_hosts_ship_ai.cpp:11188 keeps a locked target while `row->active` (`+5Ch`) | covered by D2 when ON |
| ship AI approach `009F3277` | `+5Dh` | end of kill step | gunnery dead, game_hosts_ship_ai.cpp:6845 | match |
| plane neighbour finder `007E2090` / `007DEEC0` | `+5Ch`, `+5Dh`/`+60h`/`+5Eh` | kill step | `df_slot_live`, game_hosts_units.cpp:19873 | match |
| dive break-off `009C8A90` | target `+5Dh` (`009C8AB4`) -> break off | end of kill step | arm not fed (game_hosts_units.cpp:4689, :19621); the gunnery38 retest clears the target | D4 (minor) |
| torpedo break-off `009D4C10` | target `+5Dh` (`009D4C2C`) | end of kill step | `simulate`, game_hosts_units.cpp:19208 | match |
| approach target ref `009FADA0` / `009FAF90` | `[[sub+14h]+5Dh]` | end of kill step | only inside the retest | known: docs/CC13_VER_11.md |
| squadron land command `009F8295`-`009F82D1` | land target null, `+5Eh` or `+5Dh` -> latch `+3Ch` | end of kill step | not modelled | D5 (open) |
| Lua `UnitGetAttackTarget` `008A6DE0` | `+5Dh` (`008A6F58`) | end of kill step | `unit_active` (`+5Ch`), game_hosts_lua.cpp:5899 | **D1** |
| AI capture defenders `00A07D40` (via `00A24870`) | `+5Dh` (`00A07D73`) | end of kill step | `unit_active`, game_hosts_ai.cpp:4313 | **D3** |

## 3. Divergences

Each lists the instruction address, the host site, image against host, and the predicted effect.

**D1. `UnitGetAttackTarget` answers a sinking target.** Adoption note: game_hosts_lua.cpp is leased by
agent/cc13-fix4.
- Image: `008A6F58` `CMP byte [EAX+5Dh],0 / JNE` nil, on both arms (the ship's fire target `+238h`, and
  a squadron's command target through `0071EB60` / `00521EA0`).
- Host: game_hosts_lua.cpp:5899 `live = ... unit_active(...)`, which is `+5Ch` and stays true until
  KillDepth.
- Fix: `live = target_plus_one != 0 && !units->unit_flag_005d(target_plus_one - 1)` behind an OFF switch.
- Effect: commandhelpers.lua `luaAirfieldManager` (line 7966, `UnitGetAttackTarget(unit) == nil` ->
  "searching for new target") and `luaIdleUnitFilter(..., "code")` (line 2993) see the unit as idle as
  soon as its target dies. In the existing logs no stale answer was seen: `local/v13_logscan.py` over
  cc9-lua52 `l56_b1_osk.log` (us_osumi, 1418 calls, 528 deaths), cc9-ships44 `s47_b_n2.log`, cc9-physics1
  `p6u1_gj.log` and `p6e8_Br3Lcj.log` found 0 pushes of an already-dead target. The squadron arm's
  command is usually ended by the gunnery38 retest first, so the reach is small.

**D2. No `00835F60` release of a dead fire target.** Fixed behind `kDirectorDeadFireTargetReleaseBound`
(OFF), b0c03943b.
- Image: `00835F60` (first call of `00836920`) routes a `5Eh` (null, force 1; layout as `00835740`:
  id `+20h`, force `+23h`) to `00836240` unless the target is a nav point (41h) or a live hostile.
  The automatic think then rescans (`009F5E2C`-`009F5E66`).
- Host: no release. A locked target (SetFireTarget, forced SetCommand) that dies is kept until its row
  goes inactive at KillDepth (game_hosts_ship_ai.cpp:11188 tests `+5Ch`), and `009F5EFD` refuses the new
  choice while it is held. An unlocked dead one stays until a scan finds another.
- Evidence: the keep_locked release counter is 12 in `l56_b1_osk.log` (us_osumi) and 3 in `s47_b_n2.log`
  (ijn_06), each one delayed by the wreck's sink time.
- Effect ON: the ship re-targets during the descent. Gun assignment is unchanged (`00862820` already
  filters dead candidates); torpedo-category guns, which take candidates only from the director targets,
  get a live one sooner, so torpedo launches come earlier and their stream-0 spread draws move.

**D3. Capture defenders count wrecks.** Fixed behind `kCaptureDefenderDeadByteBound` (OFF), d3ba7265f.
- Image: `00A07D73` skips `+5Dh` entities. Host: game_hosts_ai.cpp `capture_defenders_00a24870` tests only
  `unit_active`.
- Effect ON: only where the planner's Capture path runs (USN13, USN01): a ship sunk near an enemy command
  building stops being its defender, so `00A0C650`'s group values and the assignments can change.

**D4. Dive break-off's dead arm is not fed (minor).** `009C8AB4` breaks off on target `+5Dh`; the host's
`DiveBombBreakOffInputs` never sets it (game_hosts_units.cpp:4689, :19621). The gunnery38 retest (bombs
held) and the spent-ordnance latch cover the outcome, apart from step timing.

**D5. The squadron `land` arm on a dead carrier (open).** `009F8295`-`009F82D1`: a land target (`00E08FA0`)
that is null, `+5Eh` or `+5Dh`, or live of another side passing `00927C90(0)`, sets `+3Ch`; `009F7C90`
then breaks every member off and `0084E010` issues `returntobase`. The host models only `009F8160`'s
ordnance arms. Modelled as `squadron_land_target_latches_009f8295` (9ecb65ead). What the host's land task
does with a sinking carrier was not traced, so the effect is unquantified.

Known from earlier packets, not repeated: the TorpedoBot accept's dead byte (docs/CC13_GPR_40.md 1.3) and
the approach target ref `009FADA0` (docs/CC13_VER_11.md).

## 4. Timing

The host sets gunnery `dead` in `kill_unit` (the `+60h` moment) and the `+5Dh` / `+60h` mirrors at its
row-15 flush, in the same fixed step. So `+60h` readers and `+5Dh` readers agree with the image to within
the step; no consumer waits for the sinking pose, and none tests health directly.

## 5. Status and handoff

- Reconstructed and build-tested: `squadron_land_target_latches_009f8295`. Host switches D2 and D3:
  build-tested, OFF, not game-validated.
- `./scripts/build.ps1` passed (2/2 tests) after each commit.
- D1 applied by cc13-fix6 behind `kLuaAttackTargetDeadNilBound` (OFF; game_hosts_lua.hpp) at
  8ca58876b (2026-10-13); OFF is byte-identical; build-tested (ctest 2/2), not game-validated.
- Open: D5 needs the host land task traced
  before a fix; `00927C90`'s table at `+188h` is unread.
