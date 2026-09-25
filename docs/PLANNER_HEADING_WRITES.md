# Planner heading writes: what each pilot state gives 0099DE8A (packet cc9_planner_heading_writes)

2026-09-25. Names are hypotheses, not recovered symbols. `cmd` is the pilot control block
(`[approach+18h]`), `approach` the state's `[state+4h]`.

## 1. The question

`0099D300` `BSP_PilotBot_PlanControls` adds its yaw base term at `0099DE8A` only when `cmd+2CCh == 2`,
and `0099B450` reseeds `cmd+2CCh = 1` (and `cmd+2C4h = 0`, `cmd+2C8h = 20`) every think. The host gates
that term under `kPlannerYawBaseModeGateBound`, landed OFF because with it ON the Kates never reached the
fleet (docs/DOGFIGHT_MANEUVER_BODIES.md section 4, run DM1: 67,436 zeroed ticks, Kate deaths 16 -> 0).
With the gate OFF the host steers every mode toward the commanded target's bearing, which stands in for
whatever heading each state writes in the image.

## 2. Census: which host states reach the planner with `+2CCh != 2`

Run `local\PHC_9000.log` (gate OFF, E2 form: USN04 9200/9000, `BSP_GUNNERY_RNG_STREAMS=1`, module
directory `local\phc\` checked). A diagnostic (`kPlannerModeCensusDiag`, left on: it only adds summary
lines) counts every planner think that has a commanded target, keyed by task and state, split by
`+2CCh` on entry to `0099DE8A`.

| task : state | mode 0 | mode 1 | mode 2 | image writes its own mode? |
| --- | --- | --- | --- | --- |
| **torpedo : moveto (544h)** | 0 | **2173** | 0 | **no, host gap**: the image steers through `009F9E40` (mode 2) |
| torpedo : follow (580h) | 420 | 0 | 9852 | yes, `009BEE30` roll/bank/steer modes, bound |
| **torpedo : attackrun (6B4h)** | 0 | **1171** | 0 | **no, host gap**: the image writes mode 2 at `009D092D` |
| torpedo : aim (710h) | 0 | 0 | 2965 | yes |
| divebomb : goaway (704h) | 0 | 563 | 1958 | yes, bound (docs/DIVE_BOMB_GOAWAY*.md) |
| divebomb : aimdive (734h) | 2371 | 0 | 0 | yes, `009C5DB2`, bound |
| divebomb : aimglide (754h) | 0 | 46 | 3 | yes, `009C5408`/`009C5450`, bound |
| divebomb : flyabove (778h) | 0 | 5 | 2184 | yes, bound |
| divebomb : turndown (79Ch) | 103 | 543 | 0 | yes, `009C462F`-`009C46E9`, bound |
| divebomb : attackrun (7BCh) | 0 | 0 | 20010 | yes |
| dogfight : follow | 810 | 1043 | 26262 | yes, `009BEE30` |
| dogfight : moveto / aim | 0 | 0 | 13412 / 2128 | yes |
| dogfight : maneuver / avoid_roll / avoid_turn | 0 / 10 / 1 | 26 / 0 / 0 | 12 / 0 / 0 | yes, bound by cc9_dogfight_maneuver_bodies |

No planner think had a target without a task. So with the gate OFF, 9,285 thinks carry a mode other
than 2, and only **two states' 3,344 thinks** are there because the host never ran the image's write.
The others are the image's own non-heading modes (roll command, bank target), where the image's planner
also adds no base term; the host's bearing fallback was steering them anyway.

DM1's 67,436 zeroed ticks are larger than this because gating stopped the Kates in moveto: with no
heading term they never progressed out of it, so the moveto count grew for the whole mission.

## 3. The image's writers of `cmd+2C0h` / `+2C4h` / `+2C8h` / `+2CCh`

Scan: `python tools/store_census.py 0x2c0 0x2cc 0x2c4 0x2c8` over `.text` (every MOV imm/reg, MOVSS,
FSTP, FST, FISTP, INC/DEC/ADD/SUB form, disp8 and disp32). 284 stores in the image; the pilot range
`00990000`-`00A00000` holds the rows below. The owner column is the nearest preceding Ghidra function
(`store_census.py`); every torpedo row sits inside the named tick's body.

| state (torpedo task) | tick | image write | host before this packet |
| --- | --- | --- | --- |
| moveto (task+544h) | `009C18C0` | with a target at `+2Ch`: `009F9E40` at the target's world position (`009C18EC`-`009C1913`, call `009C1B23`), which writes `+2C0h` and `+2CCh = 2` (`009F9EB9`/`009F9EC1`). With no target: `+2C4h = 0`, `+2CCh = 1` (`009C1B71`/`009C1B7E`) | glide slope and speed only; **no heading write** |
| follow (task+580h) | `009C1FD0` -> `009BEE30` | steer point through the mode-2 pair (`009BF9EA`), or roll/bank modes 0/1 (`009BF752`/`009BF789`) | bound (`kPlaneFollowLawBound`) |
| attackrun (task+6B4h) | `009D07B0` | `+2C0h = AddWrapped(approach+94h, state+20h)` (`009D0915`/`009D0927`), `+2CCh = 2` (`009D092D`), on both the re-roll and the countdown paths (`009D097B` jumps to `009D0904`) | altitude only; **no heading write** |
| aim (task+710h) | `009D15F0` | `+2C0h` bearing plus sector turn, `+2CCh = 2` (`009D1D16`/`009D1D1E`) | bound |
| goaway (task+6D8h) | `009D0F10` | `+2C0h`/`+2C4h` with mode immediates at `009D10EA`, `009D1112`, `009D11AB`, `009D1203` | bound |
| done / prepare (task+618h / +740h) | `009D2720` | `+2C4h = 0`, `+2CCh = 1` (`009D2990`/`009D2998`), `+2C8h` at `009D277A`/`009D2B1D` | nothing, and the reseed already leaves `+2C4h = 0`, `+2CCh = 1`: equivalent for the gate |

The torpedo approach update `009D3420` itself stores none of the four fields; it produces
`approach+94h` (the bearing) and `+5Ch` (the sector-scan turn) that the aim and attackrun ticks consume.

### 3.1 `009D07B0` steps 1-3, read from the listing

- Constructor `009D06D0`: `+18h = 1.0` (`00D7A24C`), `+1Ch = -U(0,1)` (`00BD2F10`, `FCHS` at
  `009D072F`), `+20h = 0`. Enter `009D0790` (vtable `00D212C4+4`, xref `00D212C8`): `+20h = 0`,
  `+18h = 0.4` (`00CE7804`).
- `dt < +1Ch`: `+1Ch -= dt` (`009D097B`), then the heading.
- Otherwise the re-roll: `+1Ch = (+18h - dt) + +1Ch`; `007F0280` in **mode 0** (`PUSH 0` at `009D07FF`,
  extents 80/60/120 from `00CE5444`/`00CEB4B0`/`00D05804`); `raw = -out_a.x * out_b.y * out_b.z * pi/6`
  (`009D085C`-`009D0877`, the double `00CEC730`); `bias = clamp(00419010(3, 0, 0.5, 3, +90h / +88h) *
  approach+5Ch, -1.2, 1.2)` (`009D0888`-`009D08D6`, `009D0982`-`009D0999`; `00CE3854` = 3,
  `00CE3800` = 0.5, `00D05EA4` = -1.2, `00CE3814` = 1.2); `+20h = AddWrapped(raw, bias)` (`009D08FC`).
- Heading: `AddWrapped(approach+94h, +20h)` to `cmd+2C0h` (`009D0927`), `cmd+2CCh = 2` (`009D092D`) and
  `approach+60h` (`009D0939`).
- Stack offsets checked: `ESP0` is the frame after `PUSH ESI/EDI`; the extra `PUSH EBX` and
  `SUB ESP,14h` are undone by `00419010`'s `RET 14h` and the `POP EBX` at `009D08C3`, so `[ESP+10h]` at
  `009D090E` is `[ESP0+8h]` = `approach+94h` (`009D07D4`) and `[ESP+24h]` at `009D0888` is
  `[ESP0+0Ch]` = `approach+90h` (`009D07E4`).

## 4. The binding

One switch, `kPilotStateHeadingWritesBound` (`src/game_hosts_units.cpp`), binds the two missing writes:

- **Torpedo moveto** (`run_move_to_tick_009c18c0`): with a commanded target, `cmd+2C0h =
  heading_command_009f9e40(target, unit)` and mode 2. The `+2Ch` target is `009C2AC0`'s attack target,
  which this host holds as the commanded target. With no target the image writes `+2C4h = 0`, mode 1,
  which the reseed already leaves.
- **Torpedo attackrun** (`run_torpedo_attackrun_heading_009d07b0`, rule
  `bsp::torpedo_attack_run_heading_009d07b0` in `include/bsp/bot_task_states.hpp`): section 3.1 with the
  state's `+18h`/`+1Ch`/`+20h` kept per slot, the enter reset on the transition into the state, and the
  constructor draw at its midpoint (`+1Ch = -0.5`). **Substitution, labelled:** `007F0280` in mode 0
  walks `[approach+0Ch]+3CCh/+3D0h`, a list the host does not model (its probe is the mode-1 aircraft
  list), so the probe product is 0 and `+20h` is the sector-scan bias alone. On open water `+5Ch = 0`
  (docs/TORPEDO_RUN_IN_PATH.md section 4), so the heading is `approach+94h`, the bearing to the target
  point.

The pair flips `kPlannerYawBaseModeGateBound` ON together with it.

## 5. Predictions, written before the ON run

Pair: `local\PHW0_9000.log` (both OFF, binary `local\phw0`) against `local\PHW1_9000.log` (both ON,
binary `local\phw1`), one tree, E2 form, `BSP_GUNNERY_RNG_STREAMS=1`.

| row | OFF side | ON side prediction |
| --- | --- | --- |
| OFF side vs PHC (census run, same switches) | equal on every headline row | - |
| planner yaw base zeroed ticks | 0 | about 6,000-9,000 (the 9,285 image non-heading thinks, less the 3,344 now written, with coupling) - far below DM1's 67,436 |
| census torpedo moveto / attackrun mode 1 | 2173 / 1171 | 0 / 0 (all mode 2) |
| Kate deaths | 16 of 16 | 12-16 (they still reach the fleet) |
| Val deaths | 16 | 12-16 |
| hit records | ~553 | 400-700 |
| releases (idle player) | 0 | 0 |
| Lexington moved | ~6.5 km | 5.5-7.5 km |
| mission end | none | none |

The risk named in the brief: the states with image mode 0/1 (dive-bomb aimdive and turndown, the
follow roll/bank arms, dogfight avoid) lose the bearing fallback. In the image they get no base term
either, so if a row moves outside its band the cause is in one of those bindings, not in the gate.

## 6. The pair, measured

`local\PHW0_9000.log` (binary `local\phw0`) and `local\PHW1_9000.log` (binary `local\phw1`); the
module directory line was checked in both logs. PHW0 equals the census run PHC on every row below and
on every census cell.

| row | PHW0 (both OFF) | PHW1 (both ON) | prediction | verdict |
| --- | --- | --- | --- | --- |
| planner yaw base zeroed ticks | 0 | **10,071** | 6,000-9,000 | **missed, high**; still an order below DM1's 67,436 |
| census torpedo moveto / attackrun | mode 1: 2173 / 1171 | mode 2: 2320 / 1176 | all mode 2 | held |
| Japanese deaths (Kate / Val / movieval / Zero) | 16 / 16 / 3 / 0 | 16 / 16 / 3 / **2** | Kate 12-16, Val 12-16 | held |
| hit records / gunnery deaths | 592 / 35 | 588 / 37 | 400-700 | held |
| dive-bomb releases | 0 | 0 | 0 | held |
| torpedo releases | **6** | 4 | 0 | prediction wrong for the base: the OFF side already releases 6 on current `main`; the ON side stays close |
| Lexington moved | 6750.78 m | 5929.06 m | 5.5-7.5 km | held |
| mission end | none | none | none | held |

**Why the zeroed count is above the prediction.** The census was taken on the OFF side, and turning
the gate on changes how long the states last. The rise comes from three cells:

- torpedo follow mode 0 goes from 420 to 2,992 thinks (the `009BEE30` roll arm);
- dive-bomb aimdive goes from 2,371 to 3,169;
- a torpedo goaway cell (63 mode-1 thinks) appears after the changed releases.

All three are the image's own mode 0 or mode 1 writes, where the image's planner adds no base term
either. The Kates still close and die (16 of 16), which was the failure DM1 showed.

**The moved rows are RNG-coupled.** Aircraft path changes shift every later draw of the shared stream,
so the two Zero deaths and the 820 m on the Lexington are judged by band only.

**Switches landed:** `kPilotStateHeadingWritesBound` ON and `kPlannerYawBaseModeGateBound` ON.
`kPlannerModeCensusDiag` stays ON as a summary-only diagnostic.

## 7. Handoff: `kDogfightThrottleBound` (still OFF, not attempted here)

What blocks it, from the existing measurements (docs/PILOT_THROTTLE_SLOT.md section 6,
docs/PLANE_GUN_PASS.md, docs/DOGFIGHT_MOVETO.md run MB):

- With it ON (S1T), two Yorktown fighters still reach the water at |v| 100-106, in **aim**, chasing
  Vals. About 200 aim ticks of direct throttle at 0.46-1.0 precede both drownings.
- The aim tick's head-on flag is suspect: about half the aim ticks are flagged head-on while chasing
  from behind, which points at the forward-row substitution for the two units' `vtable[34h]` vectors.
  That read comes first (docs/PLANE_GUN_PASS.md).
- This packet does not move it: the census shows dogfight aim and moveto at mode 2 on every think, so
  the yaw base gate leaves the fighters' aim steering unchanged. The gate only touches the dogfight
  follow roll/bank arms (1,853 thinks) and maneuver/avoid (37 thinks).

Next step: read `vtable[34h]` for the aim tick's head-on test, then re-take S1T with the gate ON.
