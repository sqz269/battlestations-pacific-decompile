# CC13 luawire: unhosted mission Lua natives wired behind OFF switches

Packet cc13-luawire (cc13-stubcensus, 2026-10-19), from census rows 3-5 of
`docs/CC13_STUB_CENSUS.md`. The switches are in `include/bsp/cc13_lua_natives_wire.hpp`,
and all of them are OFF. With OFF, the trampoline row stays unhandled, so the native keeps
its UNIMPLEMENTED record line and pushes no value. Each summary line prints only when its
switch is ON. Status: reconstructed and build-tested; not game-validated. The pairs go to cc9.

| Switch | Native | Runner (`src/game_hosts_lua.cpp`) | Body checked against the listing |
| --- | --- | --- | --- |
| `kLuaHasFiredBound` | `008BE7E0` HasFired | `run_has_fired_008be7e0` | `src/unit_weapons.cpp` `lua_has_fired_008be7e0` (fixed by this packet) |
| `kLuaAddWaterLoadBound` | `00893380` AddWaterLoad | `run_add_water_load_00893380` | `src/cc13_left55_seg56a.cpp` `lua_add_water_load` (unchanged, matches) |
| `kLuaBreakShipBound` | `008914F0` BreakShip | `run_break_ship_008914f0` | `src/options_seg56_cc13.cpp` `lua_break_ship` (unchanged, matches) |

## HasFired, 008BE7E0

Stack shape: argument 0 is an entity read through `00888AA0` with no null test; argument 1 is
a string (`00B662B0`); argument 2 is a number (`00B66270`, left in ST0 as a double). One
boolean is pushed on every path (`008BE9FA`), and the native returns `00B66400`.

The sequence:
1. `vtable[5Ch](2)` (`008BE901`).
2. The director comes from `vtable[114h]`. When it is null, only a plane (`vtable[5Ch](0Fh)`,
   `008BE926`) falls back to `[unit+9D4h]`'s `vtable[114h]`, again with no null test on
   `+9D4h`.
3. The name's index comes from `006E8250` over the table at `00E08E58`. The compare is
   `_stricmp` (`00BF7FBF`), so it ignores case; no match gives 0.
4. The stamp is `[director+1D0h+index*4]` (`008BE99C`).
5. The answer is `stamp > float(double([00F876A4]) - arg2)`: `FSUBR` at 24-bit precision,
   then `FCOMIP`/`JA`, so a NaN answers false.

Earlier body, now corrected: it tested class 18h; it used the `+9D4h` fallback without the
0Fh test and with a null test the image does not have; and it folded the predicate into one
host call. Its ledger record was replaced with `--replace`. The old status was
`reconstructed_ledger_backfilled_from_source`; the new one is `reconstructed_build_tested`.

The stamps are seeded by `00720180` (`BSP_CommandControllerBase_Construct`, called by the
ship director and the squadron command-block constructors) with `D01502F9h` (-1e10f).
`009555A0` (`0095561B`) is the only writer. It runs at each projectile spawn: `0072BF10` at
`0072C155` (gun shots), and `006E4D50` at `006E55D2` (rack drops).

SUBSTITUTIONS (labelled in the runner):
- The director is a units-host slot: the unit itself, or for a plane its squadron slot in the
  plane-squadron registry.
- The stamp is the latest `last_fire_seconds` (gun+474h) over the director's guns of that
  sub-type. This does not cover dual-purpose second ammunition or rack drops.
- `[00F876A4]` is `spawn_world_clock_`.

## AddWaterLoad, 00893380

Arguments: entity, integer side (`00B66290`), and a number stored as a float. No value is
pushed.

- The hull-local point:
  - x = Width (`class+A4h`) × (1 when side is odd, -1 when even) × 0.5;
  - y = 0;
  - z = Length (`class+A0h`) × (1 when side < 2, 0 when side < 4, -1 otherwise) × 0.5.
- The native does nothing unless amount > 0 and the unit is not kind 8.
- count = trunc(clamp(float(amount / 10.0), 0, 63)). The constants are `00D7A280`,
  `00CE3DC0`, `00D099A0` and `00D099A8`.
- A non-zero count builds message 90h (`0080FF80`) and routes it with flags 7 (`0077C2A0`) to
  `008221A7`. That handler calls `0074F440`, which takes the point into the world through the
  pose at `unit+CCh` (`004142E0`) before `0074F090`.

SUBSTITUTIONS: the receiver is `GameUnitsHost::add_leak_0074f440`, called on the same tick
with the point already in world space. Width and Length come from
`vehicle_class_launch_keys`.

## BreakShip, 008914F0

The argument is an entity. The native acts only when the controller `[unit+1018h]`
(`0080E490`) has both halves: `+370h` (`0092BEA0`) and `+374h` (`0092BE90`) must be
non-zero. It then calls `00814520`:
- `004A5AA0` runs when `0080F9F0` answers true.
- Then it jumps to `00935D30` `BSP_UnitController_BreakInTwo`. That function:
  - takes 11 stream-1 draws;
  - calls `0092A240` (`BSP_TickableEntityAttachFires`) unconditionally on each piece, front
    (`00936905`, ECX = EDI), then back (`0093697F`, ECX = ESI);
  - clears `+370h`/`+374h`;
  - kills the unit with `00926D90(1)` (`00936C45`).

`0092A240` works from the piece's class `[piece+348h]`, which is the owner unit's class
`[unit+538h]` (`0092B185`):
- w = float(Length `+A0h` × 0.2) (`00CE6648`, a double);
- n = trunc(clamp(float(w / 10.0), 1, 2)) (`00CE3DC0`, `00D7A24C`, `00CE3958`);
- it draws `00BD2F10(1, -w, w)` once per puff (`0092A34A`).

It draws only when both of these are non-null:
- `[class+55Ch]`, the `DeathEfx` handle (`00831BE8`);
- `[[00E188A8]+19FCh]`, the game view node.

So the image takes 11 + 0..4 draws.

No value is pushed.

SUBSTITUTIONS:
- The halves are the units host's `front_back`, minus this host's own cleared mark. A breakup
  from the death message 98h is not seen.
- The draws come from `death_mode_draw_00bd2f10(1, ...)`. They follow the image's order:
  the eleven, then the front piece's puffs, then the back piece's.
- `0092A240`'s class is the unit's VehicleClass row. A non-zero `DeathEfx` id stands for a
  resolved `+55Ch` handle. The game view node is taken as present, since it is set in any
  rendered mission.
- The gunnery host's ship death 98h records "the same eleven" draws, so it likely lacks the
  same puff draws. That is a pre-existing record and is not changed here.
- `004A5AA0` and the piece and presentation steps are recorded, not modelled.
- The kill uses the Lua `Kill` runner's funnel (`kill_unit_00926d90(unit, 1)`): the stored
  cause is 1 (harm), and the ship death message is skipped (`explicit_kill_cause` is not 0).

## Predicted ON effect on the WINS missions

Only BSM01, BSM09 and JM06 call these natives; JM08, USN04, USN21, USN01, IJN08, USN15, USNOS
and LOMP06 do not.

- **BSM01, HasFired** (`bsm_01:813`):
  - The call is `HasFired(Henry, "DEPTHCHARGE", GameTime())`. It becomes true at Henry's
    first depth charge LAUNCH, because the stamp is set at the projectile spawn. That is
    about t=422 in `l69r_b1c_main_ref.log`: the detonation at t=425.67 (:109250) less its
    life of 3.70 s.
  - That plays the dialogs "TARGET" and "OPERATIONAL". Dialogue only.
- **BSM01, BreakShip** (`bsm_01:1192`; the script runs `ExplodeToParts`, then `BreakShip`,
  then `SetDeadMeat` on Phoenix):
  - Phoenix is killed by `00926D90(1)` in the same tick in which the OFF run's `SetDeadMeat`
    destroys it (t=117.05).
  - `SetDeadMeat` then finds Phoenix dead, so its sink is refused; it still takes its own
    pivot draw.
  - BreakShip takes 15 stream-1 draws: the eleven, then 2 + 2 puffs. Phoenix is
    `VehicleClass[252]` Brooklyn 1942, with DeathEfx 136 and Length 180, so w = 36 and
    n = trunc(clamp(3.6, 1, 2)) = 2 per piece.
  - ON reorders the draws as well as shifting them:
    - In the image, `ExplodeToParts`' `00934150` draws run at the call, before
      BreakShip's.
    - The host queues `ExplodeToParts` to its next gunnery step
      (`src/game_hosts_gunnery.cpp:15083-15087`), so BreakShip's 15 draws come first.
- **JM06, BreakShip** (`jm06:2521`, `luaDelay(luaJM6BreakShips, 9, ...)`, 9 s into the
  ambush movie):
  - The target is USTroopTransport 04, `Mission.TrgTbl[1]`, which `luaPickRnd(Mission.Cargos)`
    picks (`jm06.lua:2496`). `p23c_off_j6.log` shows the loop at `jm06.lua:2500-2505`:
    - `SetForcedReconLevel` on every live cargo (US Tanker 02, US Cargo Transport 01 and
      USTroopTransport 04, :40683-40689);
    - `AddDamage` on all but `TrgTbl[1]` (:40685 and :40688).
    - USTroopTransport 04 is the only one with no AddDamage. BreakShip is called once
      (:41951).
  - It is killed at about t=250. OFF, it dies to I-400 #2.1's torpedo at 413.82.
  - It is not a CargoTarget: the "Ambush" objective, with 3 units, completed before the call.
  - Once the Hospital Ship dies (293.65), every Cargos entry is dead, so
    `luaJM6CheckIJNSubs` has no cargo left to attack-move the subs to. The later I-400
    torpedo run and its draws change.
  - BreakShip takes 15 stream-1 draws. `VehicleClass[234]` US Troop Transport has DeathEfx
    138 and Length 180, so it gets 2 puffs per piece. They shift the shared stream.
- **BSM09, AddWaterLoad** (`bsm_09:1197-1202`, t≈39):
  - The six calls send six leaks of count 63 into Lexington's leak model.
  - With OFF, Lexington already floods, becomes a wreck at 155.00 and is killed at 255.91.
  - ON should flood it sooner. Lexington is held at its 0.01 invincibility floor until
    `luaLexSink`.
