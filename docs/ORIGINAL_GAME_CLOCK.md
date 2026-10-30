# Native original-game clock sampling

Schema 3 of `tools/original_motion_capture.py` adds game-clock brackets to the
schema 2 pose/velocity sampler. This is still external, unsuspended process
reading. The tool opens only query/read rights, makes no native calls, and does
not start or stop a game. Native motion objects remain documented in
`docs/ORIGINAL_MOTION_ABI.md`.

## Native cells and writers

The local target configuration names `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`. The saved live program was verified before the
read-only disassembly/byte queries on 2026-10-30. These addresses and widths
were checked against that program, rather than inferred from reconstructed
C++ struct member offsets. All addresses are original x86 virtual addresses;
all cells below are little endian.

| CSV meaning | Native cell | Width and producer evidence |
| --- | --- | --- |
| `game_ptr` | `[00E188A8]` | 32-bit pointer loaded at `00875BB1`; the sampler rechecks it |
| `game_time` | `[00F876A4]` | float32; Lua `GameTime` reads it at `008A93FE`; reset/set `MOVSS` at `0087464C`; step-driver `FSTP float` at `00875C21`, `00875C80`, `00875F26` |
| `game_frame` | `[game+648h]` | uint32, `ADD dword ptr [ESI+648h],EDI` at `004E4FF5`, with `EDI=1` |
| `engine_dt` | `[game+21F0h]` | float32 scaled **OnMove frame** delta, `FSTP float` at `004C6F7F`; time controls rewrite it at `004C700D`; cinematic stop writes zero at `004C7020` |
| `fixed_step_count` | `[00F876B0]` | uint32, `ADD dword ptr [...],1` at `00875C6E`; reset/set DWORD store at `0087466B` |
| `fixed_step_dt` | `[00D0DE84]` | float32 loaded at `00875C8D` for fixed-step consumers; saved bytes `CD CC 4C 3D`, approximately `0.0500000007451` seconds |
| fixed-step active flag | `[00F876A1]` | byte, set to one at `00875C86`, cleared at `00875F0E` |
| diagnostic `game_elapsed` | `[game+64Ch]` | float32 undilated accumulated frame time, `FSTP float` at `004C6F76`; differs from simulation time under time controls |
| diagnostic accumulator | `[00F876A8]` | float32, `FSTP float` at `00875C3D`; reset/set `MOVSS` at `0087465A` |
| diagnostic remainder | `[00F876AC]` | float32, `FSTP float` at `00875C31`; reset/set `FSTP float` at `0087467C` |
| diagnostic interpolation | `[00F876B4]` | float32; reset/set `FSTP float` at `00874688`; step/interpolation driver also writes it |
| diagnostic unscaled frame delta | `[game+21ECh]` | float32 `MOVSS` at `004C6F68` |

`00D7A270` is the fixed-step driver's binary64 comparison/addition operand,
not an elapsed-sample delta. Its saved bytes are `00 00 00 A0 99 99 A9 3F`,
the exact binary32 step promoted to binary64. `00F876B0` is a **simulation-step
count**, not the OnMove frame counter at `game+648h`.

The source routes used to locate the native evidence were
`src/game_frame_control.cpp`, `src/mission_state_frame.cpp`,
`src/in_mission_subsystem_tick.cpp`, `src/native_mission_clock_reset.cpp`, and
`src/lua_binding_mission.cpp`. Those reconstructed interfaces are routing
evidence; the instruction operands establish the original cell selection.

## Clock lifecycle and alignment

`game_time` is the native mission/simulation clock used by Lua `GameTime`,
not process uptime. The fixed-step driver temporarily subtracts the old
remainder, advances by whole fixed steps, then adds the new remainder back at
`00875F20..00875F26`. Outside that phase it therefore includes fractional
simulation time. A reconstructed fixed-step trajectory timestamp may differ
by this phase/remainder; equal timestamp values alone do not establish equal
update order.

`00874640` accepts a float argument and sets both time cells, derives the step
count, and computes remainder/interpolation. Its indexed callers include
mission load, session teardown, load finish, and session start gates. The
driver also zeroes `00F876AC/A4/A8` at `00876003/0B/13` when its player/session
gate is closed. Consequently clock values can reset or be set to another
value. The sampler starts a new per-unit `game_clock_epoch` when a clean
`game_time` or `game_frame` regresses, or the game pointer changes. It never
computes a sample delta or a single slowdown ratio across that boundary. Counter wrap is also treated as
a boundary, rather than guessed to be continuous.

The simulation consumes the scaled OnMove delta; zero scaled delta does not
advance the simulation clock. A frame counter or repeated pose alone cannot
establish pause. Time controls and cinematic stepping can change the stored
delta without changing the fixed motion step. No pause/restart UI experiment
was performed by this repair lane.

## Schema 3 semantics

For each fixed unit, read clock cells, then pose/controller/body/motion state,
then clock cells again. Retry immediately once when any observed clock cell
changes or either bracket has the fixed-step active byte set. The final
attempt is retained:

- `clock_torn` is numeric `0` or `1`; `clock_retry_count` is `0` or `1`.
- `clock_torn_reason` names changed clock fields and/or `fixed_step_active`.
- The before/after time, frame, engine delta, fixed-step count, phase,
  undilated elapsed time, and remainder remain as diagnostic columns.
- A final torn row retains its pose/orders/velocity and diagnostics, but its
  canonical `game_time`, `game_frame`, `engine_dt`, `fixed_step_count`, and
  `fixed_step_dt` are empty. Comparison must exclude it.
- `sample_game_dt` is the difference from the previous **clean same-unit**
  game-time observation within the same clock epoch. It is empty for the
  first clean row, torn rows, and the first row after a clock regression.
- `engine_dt` is the last stored native **frame** interval. It is neither
  `sample_game_dt` nor the fixed motion step. It does not assert that this
  particular pose came from exactly one update with that interval.
- Legacy `t`, `sample_dt`, `monotonic_ns`, and `sample_span` retain wall-clock
  meanings. `dt` remains empty because external rows are not individual
  engine ticks. Existing schema 2 columns and single-unit CLI arguments stay
  available; consumers must explicitly request game-time alignment.

Equal brackets still do not prove atomic pose capture. In particular, the
native flag clears before interpolation jobs run, and external reads can
miss intervening changes that return to the same values. This is clock
consistency filtering, not a tick-boundary hook or deterministic trace.

The JSON sidecar's `game_clock_summary` reports clean/torn/retried samples
and endpoint game/wall seconds per unit and clock epoch. The ratio is
`(last_game_time-first_game_time)/(last_wall_midpoint-first_wall_midpoint)`.
For one epoch it also appears as the unit's `game_to_wall_ratio`. A ratio
below one measures slower simulation progression per wall second over that
window; it does not by itself identify the cause. The schema 2 attempt-6
capture has no game clock, so its roughly six-percent speed/distance gap
cannot be settled retroactively. A new clocked original capture must provide
that measurement alongside its motion data.

## Fixed named units

The legacy single-unit capture and `--discover-controlled` remain supported.
For two or three fixed ships, use `--units-file PATH` instead of `--unit`:

```json
{"units": [
  {"unit_address": "0x1234AB00", "label": "USS Lexington"},
  {"unit_address": "0x1234CD00", "label": "Second verified ship"}
]}
```

Each unit is sampled sequentially on each requested wall deadline, with its
own pointer identity, clock epoch, sample deltas, and summary. `unit_id` is
still the supplied address; `unit_label` is supplied text, whose native name
and ID evidence belongs in the separate unit-discovery receipt. The sampler
does not follow selection changes or automatically replace a disappeared
unit. `--unit-label TEXT` optionally labels a legacy single-unit capture.
The total success count includes all retained rows, including flagged rows;
the clock summary separately counts usable clock observations.

## Repair validation

Python compilation and whitespace checks passed. A focused synthetic check
covered one successful retry, final torn-row blanks, an unchanged but active
fixed-step flag, the difference between sample and native frame deltas,
clock-reset boundaries, and endpoint game/wall ratios. A synthetic two-unit
capture wrote six rows with distinct supplied labels and per-unit summaries;
its ignored receipts are under
`local/clock-checks/1793323687167643500/` in the `codex-sampler-clock` worktree.
These are synthetic checks, not an original-game timing measurement. This
lane did not launch an original process or alter install/profile data.
