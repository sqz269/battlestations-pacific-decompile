# Phase A of the follow geometry: the regime selector is an intercept planner

Addresses: 009BFEE0 (Phase A 009C0251-009C0EE0, read at block level; the 009C1455-009C1560 subtree
NOT read), 009BEE30 (unchanged).

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
