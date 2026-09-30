# Phase A of the follow geometry: the regime selector is an intercept planner

Addresses: 009BFEE0 (Phase A 009C0251-009C0EE0 and the whole fly-to arm 009C0026-009C1846, read
and checked against the image in section 8; the 009C1455-009C1560 subtree is section 8.4),
009BEE30 (unchanged), 004F4840, 007DB4D0 (class+270h).

Packet `cc9_follow_phase_a`. Analysis only: nothing is bound and nothing lands in the host (see
section 5). Descriptive names are hypotheses.

## 1. Method

Phase A is about 1200 instructions of x87 code with three loops. Reading it by eye failed twice
in earlier packets, because the linear x87 depth trace mixes paths. This packet used two local
tools, both in this tree's ignored `local/` directory:

* **`local/sym.py`**, a symbolic SSA walker over `tools/x87trace.py`'s trace (`local/trall.txt`,
  made with the committed `tools/callee_effects_009bfee0.json`). It carries the x87 stack and
  frame slots as named values and prints every slot store and branch condition along a path.
  Capstone's operand forms are used, so `DC C9`-style destinations are right. Slot names are
  x87trace's canonical `base` offsets.
* **`local/blocksA.txt`**, the walker's output for each of Phase A's 99 blocks from
  `edges --lo 0x9c0251 --hi 0x9c0ee0` (the predecessor's `cfg.py`). Each block is walked with
  symbolic incoming stack entries `s0..s7`.

One finding about the tools: `x87trace` reports depth 3 at `009C0251`, but every path walked
from `009C0026` reaches Phase A with an **empty** x87 stack. The difference is x87trace's join
conflict at `009C00C0`. Its absolute depths inside Phase A are therefore offset, while the
walker's per-path depths are exact.

## 2. The inputs Phase A builds (009C0251-009C0323)

| slot | value | evidence |
| --- | --- | --- |
| `base+44h`, `base+48h` | leader velocity X, Z | `009C025E` vtable `+34h` (`007BBB70`, `unit+AC8h`) |
| `base-08h` | leader horizontal speed, `sqrt(vx^2 + vz^2)`, or 0 below 1e-10 | `009C0291` `00BF7030` |
| `base-18h` | `1 / ω`, with `ω = TurnMul * classDesc+270h` | `009C02FF`, `009C0323` |
| `base-20h` | turn radius `r = classDesc+18Ch (TravelSpeed) / ω` | `009C031B` |

`TurnMul` is `block+0Ch` or `block+10h` (`Pilot/Follow/SmallPlaneTurnMul` /
`LargePlaneTurnMul`, 1.1 each), chosen by the member's vtable `+5Ch(10h)` / `+5Ch(16h)`.
The frame values from §5.10 of `docs/PLANE_FOLLOW_LAW.md` are also used: `A` = `base-1Ch`
(heading error against the lagged track), `V` = `base+0Ch` (cross-track), `along` = `base+10h`,
and the V-sign byte `base-21h`.

## 3. What `p` and `e` are

At the first guard, `009C08B5`-`009C08C7`, the walker gives on all four paths from `009C07B6`:

```
e = |base+24h + V|          009C088B / 009C08AF (V = base+0Ch)
p = base+28h                FLD [ESP+5Ch] at 009C07D2 or 009C0829, carried on the x87 stack
JA 009C0ECB  when  e > 0.05 * p      double [00D7A270] = 0.05000000074505806
```

The twins read the same way from the raw bytes. At `009C0B96`, `0 > base+28h` jumps to
`009C0ECB`. At `009C0BC1`, `|base+24h + base+0Ch|` (through `0042BE90`) greater than
`0.05 * base+28h` jumps to `009C0ECB`.

`base+24h` and `base+28h` come from the quadrant's turn plan. In quadrant 1 with `V >= 0`
(`009C033F`/`009C03A5`, then `009C0409`), `θ = A ± π/2` gives:

```
base+18h = ∓r·sin θ               base+1Ch = r·(1 - cos θ)
base+24h = base+18h ± r           base+28h = base+1Ch + r
base-10h = (π/2 + θ) / ω          (the manoeuvre time; +π in the later arcs)
```

Then `009C0508` makes `p` relative to the moving station:

```
base+28h = base+28h - base+04h * base-08h + along
```

That is the along-track advance of the manoeuvre, minus the distance the leader flies meanwhile
(time × leader speed), plus where the member is now. So:

* **`p`** is the member's along-track position relative to its moving station at the end of the
  planned rejoin turn. Positive means it ends ahead.
* **`e`** is its cross-track error at that moment.

The guard says: if the turn would leave the member behind the station (`p < 0`), or off the
track by more than 5% of the along-track distance, use the bit-8 lead-in; otherwise use
lead pursuit. The loops (`009C07D6`/`009C081F`, `009C09FC`/`009C0A7F`,
`009C0CA6`/`009C0D15`) shorten the turn by `asin_clamped(distance / 2r)` steps
(`009C0463`, `009C0A7F`, `009C0D15`) until the arcs fit.

## 4. The regime table

The quadrant byte from `009C01D3`-`009C024F` selects the half. `CMP BL,1` is at `009C0327`,
`CMP BL,2` at `009C0903`, and `CMP AL,4` at `009C0BDD`.

| quadrant | blocks | exits | condition | regime (BL at `009C0EE1`) | `base-0Ch` into the dispatch |
| --- | --- | --- | --- | --- | --- |
| 1 | `009C032D`-`009C08FB` | `009C08CD` | `p >= 0` and `e <= 0.05p` | 1, lead pursuit | `base+04h` (time), `009C0873` |
| 1 | | `009C08D8`/`009C08C7` taken | `p < 0` or `e > 0.05p` | bit 8, `009C1328` lead-in | `base+04h` |
| 1 | | `009C0814` / `009C08F5` | the arc test at `009C07FC` / `009C0853` fails | 4 / 2, abeam by the V sign | `base-10h` (time), `009C0800` / `009C08E9` |
| 2 | `009C0909`-`009C0BC7` | `009C0BC7` | `p >= 0` and `e <= 0.05p` (`009C0B96`, `009C0BC1`) | 1 | `base-10h`, `009C0B88` |
| 2 | | `009C0B96`/`009C0BC1` taken | `p < 0` or `e > 0.05p` | bit 8 | `base-10h` |
| 2 | | `009C0B75` -> `009C08E7` | the arc test at `009C0B73` fails | 4 / 2 by the V sign | `xmm0` |
| 4 | `009C0BE3`-`009C0E48` | `009C0E42` taken -> `009C0800` | `base-14h > base-0Ch` | 4 / 2 by the V sign | `base-10h` |
| 4 | | `009C0E48` -> `009C0EB6` | otherwise | bit 8 | `min(...)`, `009C0EC7` |
| 3 | `009C0E4E`-`009C0EB6` | `009C0EB6` | always | bit 8 | `min(...)`, `009C0EC7` |

`base-0Ch` is always a **time** on the way out. `009C0EE1`-`009C0F00` then multiplies it by
`base-08h`, the leader's horizontal speed. So the dispatch's lead distance, and the abeam
altitude offset of §5.12.1, is **the distance the leader flies during the planned manoeuvre**.
That completes the second channel §5.12.1 named.

**Not established.** Quadrant 4's exit test at `009C0E42` (`base-14h` against `base-0Ch`) was
read only as an expression. The two `min(...)` arguments at `009C0EC2`, the exact arc sequence
per quadrant beyond quadrant 1, and the `009C1455`-`009C1560` subtree were not read.

## 5. Host against image, and why nothing was bound

The host's `run_follow_law_009bfee0_009bee30` carries five substitutions
(`docs/HANDOFF_PLANE_FOLLOW_REGIMES.md`):

| substitution | replaced by Phase A? |
| --- | --- |
| regime selector: always lead pursuit | **yes**. This is exactly Phase A (section 4), but only as a block-level reading |
| good-position gate: fly-to arm always | no. That is `009BFD70` / `+85h` and the HOLD arm (`docs/PLANE_FOLLOW_HOLD_ARM.md`) |
| `007D7DA0` turn rate = 0 | no. That is the frame (`009C0109`), before Phase A |
| `state+88h` = 1e30 | no. That is the tail's band floor |
| `007C47F0`, `classDesc+188h` stand-ins | no. That is `009BEE30`'s fly-to speed |

Binding the selector needs all four quadrant planners transcribed, with their loops, and the
dispatch fed `base-0Ch`. The block-level SSA is enough to name every quantity, but it was
produced with symbolic incoming stacks per block. Transcribing it into a law without a per-path
check of each loop would repeat the kind of plausible, wrong binding §5.11 withdrew. So
`kPlaneFollowPhaseA` was **not** added, and there were no Phase A runs. The measured cost of
leaving the substitution in place is small: in `docs/PLANE_FOLLOW_PITCH.md`'s run C2, no member
drowns in `follow` under lead pursuit, and the remaining E2 defects are in the dive and goaway
arms.

## 6. Runs

| run | configuration | binary | log | result |
| --- | --- | --- | --- | --- |
| A3 | main at `0af2f50eb`, default configuration | `local\binA3` | `local\A3_default.log` | see section 7 |

## 7. Control result

A3's digest (`local\A3_default_digest.txt`) is identical to B2's
(`local\B2_default_pitch_digest.txt`): 30 releases, 16 water contacts, `#3.1|.-2` / `#7.1|.-2`
transitions 7 / 13, 0 `follow law` rows. So main at `0af2f50eb`, with the packets merged since
B2, is neutral on USN04's default configuration. Nothing from this packet is in that binary.

## 8. Phase A read whole, checked against the image, and bound (packet `cc9_follow_approach_arm`, cc9-lua21, 2026-09-30)

Sections 1-7 read Phase A at block level and left the per-quadrant arcs, quadrant 4's exit test,
the `min(...)` arguments and the `009C1455`-`009C1560` subtree open. This section closes all of
them. Every claim below was checked against the image's own bytes, not only read.

### 8.1 Method: the image as an oracle

The follow geometry step `009BFEE0` was executed instruction by instruction on its bytes from the
PE on disk, by a small x86/x87/SSE interpreter in this worker's ignored `local/`
(`l21_emu.py`, `l21_harness.py`).
- x87 register forms are decoded from the opcode bytes, so `FSUBP`/`FSUBRP` cannot swap.
- The follow state, the owner, both units, the class descriptor and the `Pilot/Follow` block are
  fake objects. Reading an unset byte of one of them stops the run and names the field, so every
  input the arm reads is known.
- Every leaf callee runs on its own bytes: `0042CF10`, `00419010`, `00438B10`, `00415510`,
  `00415550`, `0042BE90`, `00414C60`, `0042B2F0`, `00419260`, `00419510`, `004F4840` and its
  callees.
- Only these are hooked:
  - the CRT x87 entries `00BF7030` (sqrt), `00BF701A` (atan2) and `00BF8490` (atan);
  - the virtuals: leader and own `+50h` (heading), leader `+34h` (velocity) and own `+5Ch`;
  - `007D7DA0` (the turn rate, an input);
  - `0042E740` (the singleton pointer).
  - `00414DB0` is never reached: the pose-clean byte `+C8h` is set.

5000 sampled geometries were run (`l21_paths.py`, seed 2), with the station, the member within
0-4000 m, any headings, the leader's speed, climb and turn rate, and the class turn terms all
varied. The sampled runs covered every instruction of the fly-to arm except:
- the `00414DB0` refresh calls;
- two degenerate floors (`009C0F5B`, `009C0F76`, `009C13B8`);
- the quadrant-1 branch `009C0829`-`009C0861` / `009C08DF` (section 8.3).

A Python model of the whole arm (`l21_model.py`, `l21_model2.py`) was then written from the
listing. It matches the emulated image on all 5000 cases: BL and `base-0Ch` at `009C0EE1`, and
every state field the arm writes (`+34h`, `+44h`-`+4Ch`, `+50h`-`+5Ch`, `+60h`-`+68h`), within
1e-3 relative. The C++ in `src/plane_follow_law.cpp` is a transcription of that model. It was
checked the same way through a probe executable (section 8.6).

### 8.2 The inputs (settled)

| slot | value | evidence |
| --- | --- | --- |
| `base-08h` | the leader's horizontal speed `len2(v.x, v.z)` (0 below 1e-10) | `009C025E` vtable `+34h` = `007BBB70`, `unit+AC8h` |
| `w` | `TurnMul * classDesc+270h`; `TurnMul` = `block+10h` Large when the own unit answers `+5Ch(10h)` or `+5Ch(16h)` (the `0047B850` pair), else `block+0Ch` Small | `009C02CF`-`009C02FF` |
| `base-18h` | `1 / w` | `009C0323` |
| `base-20h` | `r = classDesc+18Ch (TravelSpeed) / w` | `009C031B` |

`classDesc+270h` is written only by `007DB4D0` (`007DB616 FST [ESI+270h]`, the one class-side
store in a whole-`.text` displacement scan). It is the class turn rate that
docs/SQUADRON_LAND_TASK.md reads: `class+26Ch = TravelSpeed / rate`. So `r = class+26Ch /
TurnMul`, the class turn circle shrunk by the tuning multiplier (1.1 in this installation).

### 8.3 Phase A, quadrant by quadrant

Notation: `A` heading error, `V` cross-track, `AL` along-track, `s` = `V < 0` (the byte
`base-21h`), `iw = 1/w`, `Vl` the leader's horizontal speed. Every "time" is in seconds. `p` is the
along-track position, relative to the moving station, at the end of the plan.

**Quadrant 1** (`009C032D`-`009C08FB`):
1. The first arc turns through `th` (`A + pi/2`, or `pi/2 - A` when `s`):
   - `T = (pi/2 + th) iw`;
   - `x18 = -+r sin th`, `x1C = r (1 - cos th)`, `x24 = x18 -+ r`, `x28 = x1C + r`.
2. If `t = x24 + V` is past the track (`t < 0`, or `t > 0` when `s`), the arc is cut to
   `phi = min(th, asin_c(|t| / 2r))` (`00415510`). Then `x24 = -V`,
   `x28 -= 2r (1 - cos phi)` and `T4 = T - 2 phi iw`.
3. `p = x28 - T4 Vl + AL` (`009C0508`). A negative `p` goes to the lead test (below) with time `T4`.
4. Otherwise, a reversal:
   - `T2 = T + pi iw`, `x1C' = x1C - r`, `h = (x18 -+ r + V) / 2`.
   - Unless `h >= r` (or `-r >= h` when `s`), a second arc:
     - `psi1 = asin_c((h + r)/r` or `(r - h)/r)`, and `T2 += 2 psi1 iw`;
     - the alternative `T3 = (beta + 2pi + 2 psi2) iw`, with `beta = -A` or `A` and
       `psi2 = asin_c(|x44 + V| / 2r)`.
   - `009C07D6`: `q = x1C' - T2 Vl + AL`.
     - If `-q > p`: the lead test with time `T4`.
     - Else: **abeam** with time `T2`, exiting through `009C0800`.
   - `T3 <= T2` (`009C0829`) was never reached. A 400000-draw search of the model's own inputs
     found no case either, so the branch is transcribed from the listing only.

**The lead test** (`009C086D` / `009C0B82`, one shape):
- `p < 0` gives the **turn circle**;
- otherwise `|x24 + V| > 0.05 p` also gives the **turn circle**;
- otherwise **lead pursuit** (`BL = 1`).

**Quadrant 2** (`009C0909`-`009C0BC7`). No loop.
- `th = A` or `-A`; `T = (th + pi) iw`.
- `x24 = +-(r (1 - cos th) - 2r)`, `x28 = r sin th + 2r`.
- The same cut-short step as quadrant 1, without the `min`.
- `p` as above. A negative `p` goes to the lead test.
- Else: `gam = pi/2 -+ A`, `T5 = (gam + 3pi/2) iw`, `x48 = r (1 - cos gam) - r`,
  `q = x48 - T5 Vl + AL`.
  - If `-q > p`: the lead test.
  - Else: **abeam** with time `T5`, exiting through `009C08E7`.

**Quadrant 4** (`009C0BE3`-`009C0E48`):
- `gam = A - pi/2` or `-(A + pi/2)`; `T6 = (gam + 3pi/2) iw`.
- `x38 = (3 - (1 - cos gam)) r`, cut short as above.
- `p = x38 - T6 Vl + AL`.
- `dl = pi -+ A`, `T7 = (dl + pi) iw`, `q = -r sin dl - T7 Vl + AL`.
- `|q| > |p|` (`009C0E42`) gives **abeam** with `T6` through `009C0800`.
- Otherwise the **turn circle** with `min(T7, pi iw)`.
- Section 4's "`base-14h > base-0Ch`" is `|q| > |p|`.
- The constant at `009C0CCA` is `[00CE3D28]` = pi, not 2pi.

**Quadrant 3** (`009C0E4E`-`009C0EC7`): always the **turn circle**, with
`min((pi/2 + gam) iw, pi iw)`, where `gam = -(A + pi/2)` or `A - pi/2`.

**The abeam side.** Two joins test the V-sign byte with **opposite** polarity:
- `009C0800` (quadrant 1, quadrant 4): `V >= 0` gives `BL = 2`, which is LEFT.
- `009C08E7` (quadrant 2, the unreached quadrant-1 branch): `V >= 0` gives `BL = 4`, which is
  RIGHT.

This corrects PLANE_FOLLOW_LAW.md section 5.11, which read one join and concluded that the abeam
manoeuvre is always a break-away to the side the aircraft is on.

**There are no loops.** The backward edges `009C0AF2 -> 009C09FC`, `009C0D97 -> 009C0CA6` and
`009C0827 -> 009C07D6` are joins of the cut-short step, each taken at most once. Sections 3 and 7
called them loops.

### 8.4 The turn-circle regime (`BL & 8`), read whole (`009C124D`-`009C15BB`)

This was called "the `009C1328` regime", with `station + base-0Ch * U` as its steer point. That is
only its first store. The whole regime:
1. `state+5Ch = r` (`009C1262`). Every other regime leaves `-1` (`009C0EF4`).
2. `P = station + L U`, with `L = time x Vl` (`009C0F00`), stored at `+44h..4Ch`.
3. `n` is the horizontal track direction `(ux, uz) = (h.x, h.z) / lh`, turned a quarter:
   `(uz, -ux)` when `V >= 0`, else `(-uz, ux)`. Then `C = P.xz + r n`, the centre of a turn
   circle beside the point (`009C1273`-`009C134E`).
4. `d = own.xz - C`, with the length floored at 0.01. If it is shorter than `r + 1`, the
   aircraft point `q` is pushed out to `r + 1` (`009C13C8`-`009C142C`).
5. `004F4840(C, q, r, out0, out1)`: the two tangent points of the circle seen from `q`.
   - `V < 0` (`BL & 2`) takes `out0`, else `out1`.
   - `state+60h/64h/68h = (T.x, own Y, T.z)`.
   - If `004F4840` declines, the out slots keep `L U.x, L U.y` and `U.x, U.y`.
6. `v = T - q`. If shorter than `FollowedPointDist`, it is scaled to `FollowedPointDist / max(|v|,
   0.1)` (`009C14E8`-`009C152B`, `00415550`).
7. The steer point is `own.xz + v`, with its Y still `P.y` (`009C1552`, `009C1578`).
8. `state+50h/54h/58h = (C.x, own Y, C.z)`. The regime jumps to the tail and skips the
   `009C16C0` copy.

In words: when the member cannot make a plain pursuit of its moving station, it heads along the
tangent of a turn circle placed at the point the leader will have reached. It enters that circle
on the side the track needs.

### 8.5 The tail's lower bound

`009C1734`-`009C175A` computes `floor = max(min(leaderY + LeaderFollowAlt, state+88h), leaderY -
120)`, with `[00D1F3F8]` = 120.0 as a double. The old binding (`clamp_into_band_009c17c6`'s
caller) had only the `min`. The emulated image showed it when `state+88h` was small.

### 8.6 The binding

- **`src/plane_follow_law.cpp`:**
  - `plane_follow_phase_a_009c0251`: Phase A, pure;
  - `circle_tangent_points_004f4840`: moved from the moveto host's static copy, identical math;
  - `plane_follow_geometry_009bfee0`: with `PlaneFollowGeometryInputs::run_phase_a`, Phase A picks
    the regime. Lead distance, turn-circle regime, abeam side, tail floor, and the auxiliary
    `+50h`-`+68h`.
  - Without `run_phase_a` the old substitutions stand, bit-identical.
- **The probe** `local/l21_probe.cpp` links the real `plane_follow_law.cpp` and ran the 5000
  cases: 0 mismatches against the emulated image (1e-3 relative on every field).
- **The host** (`src/game_hosts_units.cpp`), behind **`kFollowPhaseABound`**, committed OFF:
  - it fills Phase A's inputs: `plane_world_velocity` (`unit+AC8h`), `unit_is_kind_of` `10h` /
    `16h`, `pilot_follow_small/large_plane_turn_mul`, and the class rate `007DB4D0` as the Impl
    member `plane_class_turn_rate_270_007db4d0`, which the land task's radius now shares;
  - it also fills `plane_travel_speed` and pose row 2.
  - With the switch OFF, Phase A runs as a shadow for the summary line `summary follow phase-a
    lead= abeam= circle= mean_time= applied=`.

**Coverage:** `009C0026`-`009C1846` complete, except the substitutions the host already carried:
`state+88h` = 1e30, the leader turn rate from `007D7DA0`, and the pose-clean assumption at the
`00414DB0` sites. The hold arm `009BFEFC`-`009C0025` is not part of this packet.

### 8.7 Predictions, written before any ON run

**The pair:**
- OFF is the no-flip export of `838701681` (`local\l21_p0`, `BA99FA9E2C7E`).
- ON is the same commit with `kFollowPhaseABound=true` (`local\l21_pa`, `D95DED205C5F`).
- Both use the reference launch rows (lockstep 0.05, idle player, `BSP_GUNNERY_RNG_STREAMS=1`,
  `BSP_DEATH_TABLE=1`).
- The OFF logs are in. Their shadow line says what Phase A would pick on each fly-to tick:

| row | follow ticks | fly-to (lead / abeam / circle) | latched | station error mean / max (m) | mean plan time (s) |
| --- | --- | --- | --- | --- | --- |
| USN04 3000 | 12709 | 313 / 0 / 7618 | 38% | 163.3 / 507.0 | 2.14 |
| E2 (USN04 9000) | 16425 | 313 / 0 / 9588 | 40% | 160.1 / 507.0 | 1.94 |
| JM05 9000 | 22284 | 4418 / 1630 / 13274 | 13% | 553.4 / 2117.7 | 9.01 |
| JM08 3000 | 6534 | 19 / 0 / 3813 | 41% | 108.8 / 250.4 | 2.17 |
| LOMP10 3000 | 67 | 22 / 0 / 9 | 54% | 96.7 / 286.7 | 3.23 |

**So the host's "lead pursuit always" is the image's choice on only 4% of USN04's fly-to ticks.**
The image almost always takes the turn-circle lead-in. That is the regime a member takes when it
would end its plan behind its moving station, or more than 5% of that distance off the track.

**Mechanism (the switch's own lines):**
- `applied=1`, and the regime split keeps the OFF shape: circle dominant on USN04, E2 and JM08,
  and all three regimes present on JM05 9000.
- The latched share (`1 - fly-to / follow ticks`) rises on USN04, E2 and JM08. The prediction
  is 45-70%.

**The station error** (`summary follow station error`) is the packet's target. It falls, because
the circle regime steers at a tangent point near `station + L U`, with `L` about 2 s of the
leader's travel. Lead pursuit steered at `station + (D + 250) U`, which runs ahead of a member
that is already ahead.

| row | OFF mean | ON mean predicted |
| --- | --- | --- |
| USN04 | 163 | 80-140 |
| E2 | 160 | 80-140 |
| JM05 9000 | 553 | 250-500 |
| JM08 | 109 | 60-105 |
| LOMP10 | 97 (67 ticks) | any |

**Gameplay** (path-coupled, so spreads only):
- **USN04:** deaths 27 ± 4, torpedo drops 2 ± 2, plane water contacts 8 ± 4.
- **E2:** deaths 51 ± 6, water contacts 19 ± 6.
- **JM05 9000:** deaths 5 ± 3. The idle player's USS Phelps is unaffected.
- **JM08:** exit 3 with deaths 10 ± 2.
- **LOMP10:** exit 1 or 3; there are only 67 follow ticks.

**Verdict rule, fixed now:**
- Flip ON if the station error mean falls on at least three of USN04, E2, JM05 9000 and JM08,
  and nothing pathological appears. Pathological means water contacts up by more than half,
  members orbiting (fly-to share rising), or a crash.
- A mechanism failure keeps it OFF: `applied` not 1, or a regime missing where the shadow had it.

### 8.8 Measured (pairs on `838701681`) and the verdict: kept OFF

- OFF logs: `local\l21_p0_<row>.log`. ON logs: `local\l21_pa_<row>.log`.
- Diffs: `local\l21_diff_<row>.txt`. `pair_diff` exits 3 on all five rows.

| row | fly-to lead / abeam / circle, ON (OFF shadow) | latched share OFF -> ON | station error mean OFF -> ON (m) | max OFF -> ON (m) |
| --- | --- | --- | --- | --- |
| USN04 3000 | 313 / 0 / 7624 (313 / 0 / 7618) | 38% -> 38% | 163.3 -> 163.8 | 507 -> 514 |
| E2 | 313 / 0 / 9595 (313 / 0 / 9588) | 40% -> 40% | 160.1 -> 160.8 | 507 -> 514 |
| JM05 9000 | 4787 / 1630 / 25848 (4418 / 1630 / 13274) | 13% -> 9% | 553.4 -> 431.1 | 2118 -> 1790 |
| JM08 3000 | 0 / 0 / 4247 (19 / 0 / 3813) | 41% -> 35% | 108.8 -> 124.8 | 250 -> 255 |
| LOMP10 3000 | 22 / 0 / 9 (22 / 0 / 9) | 54% -> 54% | 96.7 -> 96.8 | 287 -> 287 |

JM05 9000 has 35351 follow ticks ON against 22284 OFF. Its members stay in `follow` longer, so its
lower mean is over a different population.

**Gameplay** (every row inside its predicted spread):

| row | deaths | torpedo drops | plane water contacts | per-entity |
| --- | --- | --- | --- | --- |
| USN04 | 27 = 27 | 2 = 2 | 8 -> 7 | same 27 dead, 16 rows changed (time or killer) |
| E2 | 51 = 51 | 2 = 2 | 19 = 19 | same 51 dead, 40 rows changed |
| JM05 9000 | 5 = 5 | - | 3 = 3 | death rows identical, 20 unit rows moved |
| JM08 | 10 = 10 | - | 2 = 2 | death rows identical, 1 unit row moved |
| LOMP10 | 10 = 10 | - | 1 = 1 | same dead, 5 rows changed |

**Verdict: kept OFF**, by the rule fixed in 8.7.
- **The mechanism held.** `applied=1` on every row, and the regime split repeats the shadow almost
  tick for tick on USN04, E2 and LOMP10.
- **The station error fell on only one of the four rows** (JM05 9000, over a larger population).
  It is flat on USN04 and E2, and worse on JM08, whose fly-to share also rose (59% -> 65%). That
  meets the rule's "orbiting" exclusion.

**What this says about the gap:**
- The image's own regime choice does not close the 100-600 m station error. On USN04 the circle
  regime replaces lead pursuit on 96% of fly-to ticks, and the mean moves by 0.5 m.
- So the error is not a steering-regime substitution. The remaining host-side candidates are:
  - the fly-to speed (`009BEE30`'s catch-up ramp and its level-flight / `classDesc+188h`
    terms, PLANE_FOLLOW_LAW.md section 15);
  - the latch recomputed without hysteresis;
  - the station itself (`007F23A0`'s live slot 0).
- **Next measurement:** an along-track / cross-track split of the error on fly-to ticks. It would
  show whether members sit behind their stations (a speed deficit) or beside them (a steering
  one).
- The binding stays as the image's reading. It is exact against the image's bytes, and it is the
  base for flipping once the speed side is settled.

**Flipped ON, 2026-09-30, the lead's decision on fidelity.** This is a spread miss with the
mechanism matching, which the pair rule allows:
- The binding is exact against the image's bytes (section 8.1).
- `applied=1`, and the regime split repeats the shadow.
- Deaths, drops and water contacts stayed inside their spreads on all five rows.

The station-error criterion measured a gap that this pair showed lies outside the steering
regime. So it is not a test of this binding. The OFF path (lead pursuit always) stays in the code
and is not run.

### 8.9 The error split (packet `cc9_follow_error_split`)

A diagnostic now runs on both sides:
- `summary follow error split` splits the station error on every follow-law tick in the leader's
  frame (its horizontal forward, not the lagged track).
- It reports:
  - the along mean;
  - the |cross| and |dy| means;
  - counts behind, ahead and beside (50 m and dominant axis);
  - own |v| minus leader |v|, overall and when behind;
  - on fly-to ticks, `009BEE30`'s commanded speed `+2B4h` minus own |v|.

The per-400-tick `follow law` rows now key off a counter for every follow-law tick (`fw_law_ticks`).
Before, they keyed off the dive-bomb follow counter and never printed on the torpedo and dogfight
seams.

**Measured on `c73b7efc1`** (Phase A ON; export `local\l21_s1`, `918C55118E53`, logs
`local\l21_s1_<row>.log`, reference launch rows):

| row | follow ticks | error mean (m) | along mean (m) | \|cross\| mean | \|dy\| mean | behind / ahead / beside | own - leader \|v\| (m/s) | same, behind | fly-to cmd - own (m/s) |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| USN04 | 12225 | 156.6 | -137.6 | 15.8 | 50.1 | 8633 / 0 / 12 | -1.30 | -2.24 | +75.9 |
| E2 | 15550 | 150.9 | -134.3 | 13.0 | 47.8 | 10188 / 0 / 12 | -1.10 | -1.98 | +75.3 |
| JM05 9000 | 32083 | 527.7 | -233.2 | 302.4 | 98.2 | 15349 / 2992 / 12791 | -6.19 | -8.98 | +48.2 |
| JM08 | 6534 | 124.8 | -117.4 | 22.4 | 11.8 | 4757 / 0 / 131 | +0.11 | +1.14 | +75.1 |
| LOMP10 | 1009 | 247.4 | -240.3 | 29.7 | 26.2 | 991 / 0 / 18 | +0.53 | +0.54 | +80.1 |

LOMP10's follow population grew from 67 to 1009 ticks with main's changes since `838701681`.

**The gap is a speed deficit, not steering.**
- On every row but JM05 the member sits straight BEHIND its station: the along mean is -117 to
  -240 m, with |cross| of 13-30 m and not one tick ahead.
- It flies at its leader's speed or slower (-2.2 to +1.1 m/s while behind). So it never closes.
- Meanwhile `009BEE30`'s fly-to arm commands about 75 m/s MORE than the member is doing.

So the command is there and is not being realised. The loss is between `+2B4h` and the airframe:
the speed-hold / throttle slot path, the class speed ceiling, or the leader already flying at the
member's top speed.

JM05 9000 adds a real cross-track component (|cross| 302 m, 40% of ticks beside), on top of the
same deficit.

**Next:** the existing `follow trace` diagnostic (`kFollowTraceEvery`, in the shared units file)
prints the throttle slot, `want`, `v` and the speed mode on the follow seams. One OFF-only run
with it at 400 would show which stage drops the 75 m/s.

## 9. The speed loss: the follow step's turbo request (packet `cc9_follow_turbo`, cc9-lua21, 2026-09-30)

### 9.1 Where the 75 m/s goes

The diagnostic `summary follow speed ceiling` was added at `caeaaa4c4` and runs on both sides.
Over every follow-law tick it reports:
- the member and leader |v|;
- their class MaxSpd (`desc+188h`);
- the leader's commanded `+2B4h`;
- both throttle slots, and how often each is at 0.99 or above.

Measured on `caeaaa4c4` (export `A3B86BC078A2`, logs `local\l21_s1_<row>.log`):

| row | member v | leader v | MaxSpd | leader want | throttle member / leader | at full, member / leader |
| --- | --- | --- | --- | --- | --- | --- |
| USN04 | 72.6 | 73.9 | 75.7 | 102.7 | 0.982 / 0.976 | 91% / 94% |
| E2 | 74.3 | 75.2 | 76.1 | 102.6 | 0.984 / 0.976 | 92% / 94% |
| JM05 9000 | 52.7 | 58.9 | 70.8 | 106.1 | 0.831 / 0.906 | 72% / 87% |
| JM08 | 80.7 | 80.5 | 83.3 | 94.0 | 0.968 / 0.955 | 78% / 34% |
| LOMP10 | 90.1 | 89.5 | 87.6 | 122.1 | 0.988 / 0.993 | 96% / 98% |

- The leader is commanded `TravelSpeed x NewTravelSpeedMul` (1.6), above its MaxSpd, as in the
  image (PLANE_FOLLOW_LAW.md 16.2). So it flies flat out at MaxSpd.
- The member is also at full throttle, and the same class caps it at the same speed.
- The throttle path is not the loss. The airframe is: in this host a member at full throttle can
  never out-run a leader at full throttle.

### 9.2 The image's answer: `009BEE42`, the turbo request

`009BEE30` opens `MOV byte [[approach+18h]+2E5h],1` at `009BEE42`, before the `+85h` branch, so it
runs on both arms. The chain, each link read from the listing:

| step | address | effect |
| --- | --- | --- |
| request | `009BEE42` | `plan+2E5h = 1` on every follow command step (never for the leader: the tick returns at `009C1FF1` first) |
| reseed | `0099B572` | `plan+2E5h = DL`, with `DL = 0` from `0099B46E XOR EDX,EDX`, at every think |
| release | `009C2347` | `plan+2E5h = 0` when latched and `plan+268h & 1`. `+268h` is only ever stored as 0 (DOGFIGHT_MANEUVER_BODIES.md), so this does not fire |
| command buffer | `0099BF0F` | `cmd+15h = plan+2E5h`, which is `unit+A11h` |
| commit | `007BB8D0`-`007BB8D6` | `unit+9F9h = cmd+15h`. The `unit+61h` gate at `007BB8D3` has no writer |
| control copy | `007DC84F` | `ctl+4h = unit+9F9h`, every step |
| thrust | `007D9062` | `a *= tuning+330h` `Dynamics/SpdMultipliers/TurboMultiplier` (1.95) when `ctl+4h` |
| follow exit | `009BDE40` (vtable `00D20AB8` +8) | `unit+9F9h = 0` via `007B8A90` |

- Drag is `Accel / MaxSpd^2 x v^2` (`007C4990`). So a member on turbo settles at
  `MaxSpd x sqrt(1.95)`, about 1.4 MaxSpd: 106 m/s on USN04 against the leader's 74.
- `include/bsp/pilot_command_path.hpp` calls `cmd+15h` / `unit+A11h` "dead". That is a
  correction owed: `007BB8D0` reads it (PILOT_PLAN_SLOT_PIPELINE.md already has the row).
- Other writers of `plan+2E5h` remain unread: `009A4F1A` (in `009A4DC0`) and `009A5143` (in
  `009A5000`).

**The binding:** `kFollowTurboBound`, committed OFF, in `src/game_hosts_units.cpp`:
- `plan_turbo_2e5` is set at the top of `run_follow_law_009bfee0_009bee30` and cleared in the
  think reseed.
- `plane_turbo_9f9` is copied at the pilot commit (`007BB920` site).
- The thrust multiply is added beside `007D9052`'s `Accel x throttle`.
- **Substitution, labelled:** the follow exit's immediate clear (`009BDE40`) is not bound. The
  byte drops at the next think's commit instead, one think late.
- A new summary line: `summary follow turbo steps= bound=`.

### 9.3 Predictions, written before any ON run

Pair: OFF = the no-flip export of the binding commit, ON = `kFollowTurboBound=true`. Same five
rows.

**Mechanism:**
- `turbo steps` is 0 OFF and non-zero ON on every row with follow ticks.
- Member |v| rises above leader |v| while behind: `behind_speed_diff_mean` goes from about -2 to
  +5..+30 m/s.

**Along-track error falls:**

| row | along mean OFF -> ON | station error mean OFF -> ON | latched share OFF -> ON |
| --- | --- | --- | --- |
| USN04 | about -138 -> -10..-80 | about 157 -> 40-110 | 38% -> 55-85% |
| E2 | about -134 -> -10..-80 | about 151 -> 40-110 | 40% -> 55-85% |
| JM05 9000 | about -233 -> -30..-180 | about 528 -> 250-480 | rises |
| JM08 | about -117 -> -10..-70 | about 125 -> 30-100 | rises |
| LOMP10 | about -240 -> -20..-150 | about 247 -> 60-200 | rises |

- The `ahead` count becomes non-zero (overshoot), but stays below `behind`.
- The cross-track and dy means barely move.

**Gameplay** (formations close up, so AA exposure and arrival times move):
- **USN04:** deaths 27 ± 5, torpedo drops 2 ± 2, water contacts 7 ± 4.
- **E2:** deaths 51 ± 8, water contacts 19 ± 6.
- **JM05 9000:** deaths 5 ± 3.
- **JM08:** deaths 10 ± 2.
- **LOMP10:** deaths 10 ± 3.

**Verdict rule:**
- Flip ON if the mechanism holds (turbo steps non-zero, member faster than leader while behind)
  and the along-track error falls on at least three of USN04, E2, JM08 and LOMP10.
- Keep it OFF, and record the reason, if water contacts rise by more than half or the ahead count
  exceeds the behind count.

### 9.4 Measured (pair on `4bdf19712`) and the verdict: ON

- OFF: `local\l21_p0` (`4E906856EB2D`).
- ON: `local\l21_pa` with `kFollowTurboBound=true` (`E97EA0B3EC07`).
- Logs are `local\l21_{p0,pa}_<row>.log` and diffs are `local\l21_tdiff_<row>.txt`.

| row | turbo steps | station error mean OFF -> ON (m) | along mean | \|cross\| | behind ticks | latched share | pair_diff |
| --- | --- | --- | --- | --- | --- | --- | --- |
| USN04 | 0 -> 24073 | 153.8 -> 22.4 | -134.9 -> -9.4 | 15.9 -> 11.6 | 8649 -> 328 | 40% -> 96% | 3 |
| E2 | 0 -> 31209 | 147.4 -> 20.5 | -131.1 -> -9.6 | 13.0 -> 10.2 | 10351 -> 410 | 44% -> 97% | 3 |
| JM05 9000 | 0 -> 61643 | 527.7 -> 389.7 | -233.2 -> -203.1 | 302.4 -> 208.9 | 15349 -> 17017 | 6% -> 12% | 3 |
| JM08 | 0 -> 12982 | 124.8 -> 32.8 | -117.4 -> -21.2 | 22.4 -> 17.9 | 4757 -> 691 | 35% -> 95% | 1 |
| LOMP10 | 0 -> 1870 | 247.4 -> 98.5 | -240.3 -> -84.6 | 29.7 -> 28.1 | 991 -> 427 | 0% -> 60% | 3 |

- The ahead count is 0 on every row but JM05 (2992 -> 2535).
- Members are faster than their leaders while behind on E2 (+2.5), JM08 (+13.9), LOMP10 (+20.9)
  and JM05 (-9.0 -> +5.1).
- On USN04 the 328 remaining behind ticks are transients at -0.9. The prediction there was
  +5..+30, a miss on a residue of 3% of the ticks.
- The latched share overshot the predicted 55-85% on USN04, E2 and JM08 (95-97%).

**Gameplay:**

| row | deaths | torpedo drops | water contacts | per-entity |
| --- | --- | --- | --- | --- |
| USN04 | 27 -> 26 | 2 -> 3 | 6 = 6 | A6M Zero #4.2 survives; 23 death rows moved |
| E2 | 51 = 51 | 2 -> 5 | 19 = 19 | 48 death rows moved |
| JM05 9000 | 5 = 5 | - | 3 = 3 | death rows identical |
| JM08 | 9 = 9 | - | 1 = 1 | gameplay identical (exit 1) |
| LOMP10 | 7 = 7 | - | - | 5 death rows moved |

Every row is inside its predicted spread. E2's torpedo drops rise 2 -> 5, because wingmen now
arrive with their leaders; no spread was predicted for that.

**Verdict: ON.**
- The mechanism held on all five rows.
- The along-track error fell on all four named rows, by 65-93%.
- No exclusion fired: water contacts are unchanged, and ahead never exceeds behind.

The 100-600 m follow gap (SQUADRON_LAND_TASK 5ai item 1) is closed on USN04, E2, JM08 and
LOMP10. JM05 9000's remainder is its cross-track and population, still open.

### 9.5 Who else requests turbo: nobody (cc9-lua21, 2026-09-30)

A whole-`.text` displacement scan for `+2E5h` found six writers besides the reader `0099BF0F`.
Each was read at its site:

| site | function | object | value | meaning |
| --- | --- | --- | --- | --- |
| `009BEE42` | `009BEE30` follow command step | pilot plan (`[approach+18h]`) | 1 | **the follow turbo request** (9.2) |
| `0099B572` | `0099B450` plan reseed | pilot plan | 0 (`DL`, `0099B46E XOR EDX,EDX`) | the per-think clear |
| `009C2347` | `009C1FD0` follow tick | pilot plan (`[[state+4]+18h]`) | 0 | release when latched and `plan+268h & 1`; `+268h` is never set, so this is dead in practice |
| `0099B3E4` | `0099B3E0`-`0099B3ED` (`RET 4`) | pilot plan (`ECX`) | the byte argument | a setter with **no reference**: no rel32 call, no absolute dword, no Ghidra xref |
| `009A4F1A` | `009A4DC0` (called by `BSP_BotTaskDepthCharge_Construct` at `009A5281`) | the **task** object (`ESI = ECX`) | 0 (`009A4DFA XOR EBX,EBX`) | zero-initialisation of the depth-charge task's own `+2B0h`..`+2E5h`; not the plan |
| `009A5143` | `009A5000` (called from `009A6820`) | the task object (`ESI = ECX`) | 0 (`009A5038 XOR EBX,EBX`) | the same, for the second depth-charge constructor |

**So the follow state is the only live turbo request** in the image, and it is raised only for
wing members, because the leader's tick exits at `009C1FF1` first.
- Dogfight, moveto, attack runs and landing never request turbo.
- A plane on turbo in any other state is a host error, and the host has none:
  `plan_turbo_2e5` is set only in `run_follow_law_009bfee0_009bee30`.
- The two depth-charge sites are same-offset fields of a different object. That is the
  constructor-store pattern, not a writer of the plan.
