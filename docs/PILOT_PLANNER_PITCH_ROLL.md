# The planner's pitch and roll arms (packet `cc7_planner_pitch_roll`)

`0099D300 BSP_PilotBot_PlanControls`, read backwards from the three `desired` stores that the yaw
packets left open. This closes the two arms that keep a bot's nose up and its wings level.

## Headline

1. **There is a fourth roll store the packet brief did not list: `0099E39D`.** It is the computed
   roll law, and it is the one that matters. `0099D384`/`0099D416`/`0099D6B7` are the neutral plan,
   the low-airspeed wings-level gate and the stick override respectively — none of them is a law.
2. **The roll law is a proportional bank-angle servo**:
   `slot(roll).desired = clamp(-A / t, -1, +1)`, where `A` is a soft-zoned bank error against a
   target held in `plan+2C4h` and `t = class+1A8h² / (class+1BCh · Pilot/General/WaggleLimit)`.
3. **The roll target `plan+2C4h` is driven by the heading error** — the same deadbanded,
   step-limited `[ESP+6Ch]` the yaw arm consumes. That is the coordinated turn: heading error →
   bank command → roll command. A reconstruction that wires only the yaw arm gets a flat skid, not
   a turn.
4. **The pitch demand is a normalised pitch-rate command**:
   `demand = (E / dtScale) / (class+1ACh · (0.9R+0.1) · cos(bank) · k · Pilot/General/PitchCtrlSetTimeMul)`
   with `E` the wrapped pitch error. `plan_pitch_0099e68d` then clamps it; that half was already
   reconstructed and is unchanged.
5. **The nose-up hold exists and it is in the pitch arm**: `0099E4E8`-`0099E512` raises the pitch
   target to a floor `T = Pilot/General/PitchTurnMaxPitch - 2.5·(1 - q)` on every pass. It is an
   *attitude* floor, not an altitude hold — see "Altitude" below.
6. **`plan+2BCh` is the PITCH target, not a bank target.** `docs/PILOT_BOT_PLAN_CONTROLS.md` and
   `include/bsp/plane_ai_control.hpp:296` (`kBankTargetLimited`) both name it a bank target. They
   are wrong, and the same header already contradicts them at `kPitchMode = 0x2D0`. Proof below.

## Method, and a frame-walk hazard worth recording

`python tools/stack_frame_walk.py 0099d300` prints a `depth=` per instruction; the entry-relative
slot is `disp - depth`, not `disp + depth`. **The walker's depth drifts by +4 from `0099DA09`
onward**: `0099DA05 PUSH 0x6` / `0099DA07 CALL EAX` is an indirect call whose callee pops the
argument, and the walker cannot know that ("INDIRECT CALL - cleanup unknown"). Every slot computed
from an address `>= 0099DA09` therefore needs `+4` added back. The check that settles it: the pitch
side product reads `cos(pitch)` as `[ESP+2Ch]` at `0099E6A4` (depth 12), and the frame stores it as
`[ESP+2Ch]` at `0099D50E` (depth 8) — those are the same slot only with the correction.

Without the correction `0099E2BA FLD [ESP+34h]` resolves to the `cos(bank)` slot instead of the
`bank` slot, and the roll law reads as differencing an angle against a cosine. That is the exact
shape of a plausible-looking wrong answer, so it is called out here rather than buried.

Slot numbering below is entry-relative with the correction applied; the per-tick frame of
`docs/PILOT_BOT_PLAN_CONTROLS.md` maps as:

| slot | content | store |
| --- | --- | --- |
| 8 | `unit+C64h` pitch, later reused | `0099D4DC` |
| 12 | (unused at entry) the pitch arm's measured angle | `0099DCAC` |
| 16 | scratch: roll limit, then roll error, then pitch error | — |
| 20 | `\|bank\|` | `0099D4E2` |
| 24 | `sin(bank)` | `0099D4FC` |
| 28 | scratch | — |
| 32 | `1 / dtScale` | `0099D4E6` |
| 36 | `cos(pitch)` | `0099D50E` |
| 40 | `cos(bank)` | `0099D506` |
| 44 | `bank` = `unit+C68h` | `0099D4D6` |
| 48 | `dtScale = max(unit+340h·0.4, 1.0)`, dead after `0099DFE5` | `0099D4AD` |
| 52, 56, 60, 68, 72, 76, 84 | scratch | — |
| 100 | `dt / dtScale`, then the heading error | `0099D4F2` |

ABI unchanged: `__thiscall(plan* ECX, float dt)`, `RET 4`, `EBX = BSP_GameTuning_GetSingleton() +
538h`, `ESI = plan`, `plan+2F0h = unit`, `plan+2F4h = class descriptor`.

## Which slot is which axis — settled inside this function

The brief's premise and the published slot table both hold, and this packet can now prove the axis
identity without leaning on the `unit+998h`/`+99Ch`/`+9A0h` naming:

* The arm gated on `task+2CCh` stores to `+290h` and clears `task+2CCh` (`0099E3B5`). Its error
  term is `SubtractWrappedAngle(plan+2C4h, unit+C68h)` at `0099E2CE` — it servos the **bank angle**.
  `+290h` is therefore the roll slot, and `+2C4h` / `+2CCh` are its target and mode.
* The arm gated on `task+2D0h` stores to `+29Ch` and clears `task+2D0h` (`0099E748`). Its target is
  `plan+2BCh` and its measured angle reaches it through `unit+C84h`.
* `0099D700`/`0099D711` and `0099D745`/`0099D756` are a matched pair: one writes `plan+2C4h` and
  sets `task+2CCh = 1`, the other writes `plan+2BCh` and sets `task+2D0h = 1`, each guarded on its
  own `unit+9A_h` field (`0099D717` reads `unit+9ACh` for the pitch one).

So `{+290h, +2C4h, +2CCh, unit+C68h}` and `{+29Ch, +2BCh, +2D0h, unit+C84h}` are the two axes, and
whichever of them is pitch, `+2BCh` and `unit+C84h` are on the same axis as `+29Ch` and `+2D0h`.
Every existing doc calls `+29Ch` pitch and `+2D0h` the pitch mode; `plan+2BCh` is a **pitch**
target and `unit+C84h` a **pitch-axis** angle.

---

## (1) The roll arm

### 1a. The gate

```
0099DDBE  ECX = task+2CCh
0099DDC6  JNZ 0099DE8A                ; non-zero -> skip the roll re-plan block
0099DE8A  CMP ECX,2 / JNZ 0099E26E    ; only mode 2 recomputes the bank target
0099DE93..0099E25C                    ; recompute plan+2C4h, then task+2CCh = 1 (0099E264)
0099E26E  CMP [ESI+2CCh],1 / JNZ 0099E3BF
0099E27B..0099E39D                    ; the law
```

* `task+2CCh == 2` — recompute `plan+2C4h` from the heading error, set the mode to 1, run the law.
* `task+2CCh == 1` — run the law against the standing `plan+2C4h`.
* `task+2CCh == 0` — the roll slot is not written at all this pass.

**The law clears the mode to 0 after storing** (`0099E3B5`). The `desired` persists in the slot with
`active = 1` (`0099E3AE`); the planner does not re-derive it until a command re-arms the mode word.
The pitch arm does the same at `0099E748`. Host code that sets the mode once and expects a law
every tick will get one pass only.

### 1b. The bank target `plan+2C4h`, `0099DE93`-`0099E25C`

The input is the heading error already documented as `[ESP+6Ch]` (slot 100): `0099DE9E` calls
`unit->vtable[50h]()` for the heading, `0099DEB8` wraps `plan+2C0h - heading`, `0099DEBD` divides by
`dtScale`, `0099DEC5`-`0099DF03` applies the `Pilot/General/SoftHdgZone` (`tuning+3Ch`) deadband and
`0099DF09`-`0099DF87` the step limit. Taken on trust from
`docs/PILOT_BOT_PLAN_CONTROLS.md` §"`[ESP+6Ch]`, the base numerator"; call the result `h`.

```
0099DFAA  maxBank = plan+2E8h * class+25Ch
0099DFCC  cap     = 0047B880(unit) ? tuning+64h : tuning+68h     ; TurnRollLimitSmall / Large
0099DFF3  maxBank = min(maxBank, cap)                            ; slot 8

0099E02B  e       = |h|                                          ; slot 28
0099E07E  hdgRamp = InterpolateClamped(tuning+8Ch, 0, tuning+90h, 1, e)   ; PitchTurnHdgRange, slot 56
0099E087  w       = min(|bank| * 1.5, maxBank)                   ; 00CE3D78 = 1.5 (double)
0099E0C1  L0      = 0099D0A0(plan, w)                            ; __thiscall, x87 return  ** not read **
0099E0CA  C       = (|L0| > 0.01f) ? L0 : (L0 < 0 ? -0.1f : +0.01f)
                                                                 ; 00D7A238 = 0.01f, 00D7A300 = -0.01f,
                                                                 ; 00CE3CB4 = -0.1f
0099E17B  s       = InterpolateClamped(-C, -0.3*C, +C, +0.3*C, h) ; 00CE3DC8 = 0.3 (double)
0099E180  raw     = (h - s) / C
0099E199  raw     = ClampInPlace_00415620(raw, -1, +1)
0099E19E  raw    *= maxBank                                      ; slot 28

0099E1CC  Lc      = InterpolateClamped(tuning+6Ch, tuning+78h, tuning+70h, tuning+74h, pe)
                                                                 ; TurnRollPitchLimit{Pitch,Roll}/{1,2}
0099E1D5  L       = max(Lc, |bank|)
0099E218  L       = ClampInPlace(L, -2.0f, +2.0f)                ; 00CE7D7C = -2.0f, 00CE3958 = +2.0f
0099E23E  plan+2C4h = ClampInPlace(raw, -L, +L)
```

`pe` (slot 52) is the **pitch** error `SubtractWrappedAngle(m, plan+2BCh)` computed at
`0099DD64`-`0099DD85`, i.e. the roll limit is scheduled on how far the pitch is from its target —
which is what the tuning key name `TurnRollPitchLimit…` says it should be.

Then, still inside the law's block:

```
0099E27B  if (plan+2C8h < pi)  plan+2C4h = ClampInPlace(plan+2C4h, -plan+2C8h, +plan+2C8h)
                                                                 ; 00CE3D28 = pi (double)
```

`plan+2C8h` is a per-task bank-angle limit; its producer is outside this function and was not read.

### 1c. The law, `0099E2BA`-`0099E39D`

```
0099E2CE  v = SubtractWrappedAngle(plan+2C4h, bank)              ; bank = unit+C68h, slot 44
0099E2D3  v = v / dtScale                                        ; slot 32 = 1/dtScale
0099E2ED  a = |v|                                                ; -0.0f - v idiom, 00D7A208 = -0.0f
0099E30E  if (tuning+40h > a)        A = v * tuning+44h          ; SoftRollCtrl, SoftRollMul
          else                     { plan+2ECh = 0.1f
0099E332                             A = v - sign(v) * tuning+48h }   ; 00E0E2F4 = 0.1f
0099E357  t = class+1A8h * class+1A8h / (class+1BCh * tuning+0h) ; tuning+0h = WaggleLimit
0099E390  slot(roll).desired = InterpolateClamped(-t, +1.0f, +t, -1.0f, A)
0099E3AE  slot(roll).active  = 1   (byte)
0099E3B5  task+2CCh          = 0
```

`BSP_Math_InterpolateClamped(x0, y0, x1, y1, x)` with those endpoints is exactly
`clamp(-A / t, -1, +1)` for `t > 0`, and returns `y0 = +1.0f` on the ordered-equal endpoints
(`t == 0`) — the degenerate case matters, because `t` is a pure class/tuning product with no runtime
guard. **The sign is inverted**: a positive bank error (target above current bank) yields a negative
roll command.

Inside the soft zone the command is `v · SoftRollMul`; outside it the constant `tuning+48h` is
subtracted with the sign of `v`. The two pieces join continuously iff
`tuning+48h == SoftRollCtrl · (1 - SoftRollMul)`; that is a hypothesis about the data, not something
this packet measured, and `0x580` (= `tuning+48h`) has no row in the
`src/game_tuning_singleton.cpp` setter table.

---

## (2) The pitch arm

### 2a. The target `plan+2BCh` and the measured angle, `0099DC9E`-`0099DD85`

This is the block `docs/PILOT_BOT_PLAN_CONTROLS.md` §"The bank-target arm" describes. Its algebra is
confirmed here; only its *name* is wrong (it is the pitch axis — see above), and two details differ:

```
0099DC9E  EAX = task+2D0h
0099DCA6  slot52 = 0 ; slot12 = 0
0099DCB2  if (task+2D0h == 0) goto 0099DD94                      ; no pitch target this pass
0099DCB8  if (task+2D0h == 2 && unit->vtable[38h]() < 2.7778f) task+2D0h = 1   ; 00D1F3D8
0099DCE0  if (task+2D0h == 2) {
0099DCFA      slot12 = unit+C84h                                 ; the held pitch-axis angle
0099DD2C      x0  = 007C4810([00CEDF5C]=5deg, class+18Ch, [00CE398C]=20deg, speed)
0099DD35      inc = InterpolateClamped(x0, 5deg, class+18Ch, 20deg, speed)
0099DD42      sum = inc + slot12
0099DD4C      if (unit+C64h > sum) { plan+2BCh = sum ; slot12 = unit+C64h }
          } else slot12 = unit+C64h                              ; 0099DD54
0099DD78  slot52 = SubtractWrappedAngle(slot12, plan+2BCh)       ; the roll limiter's schedule
```

Two corrections to that section: the comparand at `0099DD46` is slot 8 = **`unit+C64h`, the live
pitch angle**, not an untraced `[ESP+10h]`; and `t` from `007C4810` is `InterpolateClamped`'s
*first* argument (`x0`) after the `PUSH ECX` at `0099DD31` shifts the four already-stored floats up,
not its last. `007C4810` is modelled as popping nothing, which is what makes the shift work out —
`0099DD35`'s `RET 14h` then balances the whole sequence back to the entry depth.

### 2b. The gate

```
0099E3BF  ECX = task+2D0h
0099E3D1  JNZ 0099E490      ; the law
          ; zero -> the pitch re-plan block 0099E3D7-0099E483, then 0099E488 JZ 0099E754 (no store)
```

So the pitch law runs **iff `task+2D0h != 0`**. On the zero path `0099E3CB` also sets slot 8 to
`0.0f`, which is the yaw arm's turn numerator — no pitch plan means no turn term.

### 2c. The nose-up floor, `0099E490`-`0099E512`

```
0099E496  r  = |bank| / class+25Ch
0099E4AA  r' = min(r, 1.0f)
0099E4CC  q  = r' * hdgRamp                    ; hdgRamp = slot 56 from 0099E083
0099E4DC  T  = tuning+88h - 2.5 * (1 - q)      ; PitchTurnMaxPitch; 00CE3DE0 = 2.5 (double)
0099E4FA  B  = max(T, plan+2BCh)
0099E512  plan+2BCh = B
```

`T` rises with bank (toward `class+25Ch`) and with heading error (through `hdgRamp`), so the harder
the commanded turn, the higher the floor under the pitch target. With `q = 0` the floor sits
`2.5` below `PitchTurnMaxPitch`; with `q = 1` it is `PitchTurnMaxPitch` itself.

`hdgRamp` is slot 56, and it is **path-dependent**: it holds the `0099E083` interpolation only when
the roll arm took the `task+2CCh == 2` recompute. On the `task+2CCh != 0` path the block from
`0099DDC6` jumps to `0099DE8A` and slot 56's reaching definition is `0099DDA3`, which stores `0.0f`
— so a pass that does not re-plan the bank target computes `q = 0` and the lowest floor,
`T = PitchTurnMaxPitch - 2.5`. That coupling is real, not an artefact of the reading: the two arms
share the slot deliberately.

### 2d. The demand

```
0099E51A  corr   = sin(bank)^2 * cos(pitch) * class+1B8h * class+1B0h * tuning+A0h
                                             ; SlideRatio, YawSpd, PitchCtrlSetTimeMul
0099E53D  m      = slot12 - corr                               ; slot 12
0099E554  E      = SubtractWrappedAngle(B, m)                  ; slot 16
          ; re-plan interval, on |E|:
0099E593    |E| >  12deg          -> plan+2ECh = 0.1f          ; 00CE3E00 = pi/15
0099E5B5    3deg < |E| <= 12deg   -> plan+2ECh = 00415510(&plan+2ECh, 00E0E2F0)  ; 00D1A8A0 = pi/60
            otherwise               plan+2ECh unchanged
0099E5D7  R      = BSP_PlaneFlight_ControlAuthority(unit+AB0h)
0099E5FC  S      = R * 0.9 + 0.1                               ; 00D7A390, 00D7A3A0
0099E5EB  inv    = sign(cos(bank)) < 0                         ; slot 60, from 0099DDCA
0099E61C  k      = inv ? class+1D8h : 1.0f                     ; NegativePitchRatio
0099E604  N      = E / dtScale
0099E63A  X      = class+1ACh * S * cos(bank)
0099E644  D      = X * k * tuning+A0h                          ; PitchCtrlSetTimeMul
0099E66A  demand = (|D| > 0.001f) ? N / D                      ; 00D7A23C = 0.001f
                                  : N * 100.0 * sign(cos bank) ; 00D7A220 = 100.0 (double)
0099E689  [ESP+44h] = demand
```

`demand` is a required pitch rate divided by the pitch rate one unit of elevator buys — the inverse
of the axis authority. `D` collapses toward zero near knife-edge (`cos(bank) -> 0`), which is what
the `0.001f` guard and the `N * 100 * sign(cos bank)` fallback exist for; at exactly `cos(bank) == 0`
the fallback is `0`.

**This is the `demand` that `plan_pitch_0099e68d` was declared against.** `0099E68D`-`0099E741` is
unchanged from `docs/PILOT_BOT_PLAN_CONTROLS.md`: `slot(pitch).desired = clamp(demand, -1, +1)`,
`active = 1` (byte, `0099E741`), `task+2D0h = 0` (`0099E748`).

### 2e. The yaw arm's turn numerator — confirmed, not new

`X` above is the live x87 value; `0099E64A` and `0099E689` both pop values pushed above it. The
`+1` / pass-through branch leaves `slot 8 = p - k·X` (`0099E6CE FSUBR`) and the `-1` branch leaves
`slot 8 = p + class+1D8h·X` (`0099E725 FADD`) — note the opposite sign — with
`p = sin(bank)^2 · cos(pitch) · class+1B8h · class+1B0h`. Independently re-derived here and it
agrees with `docs/PILOT_BOT_PLAN_CONTROLS.md` §"`[ESP+10h]`, the turn numerator"; the `FMUL ST1`
at `0099E642` is `D8 C9` = `FMUL ST(0),ST(1)`, and `0099E69F`/`0099E707` are `DC C8` = square.

---

## (3) Altitude, and what actually holds the nose up

There is **no altitude term in the pitch arm**. Nothing in the chain from `task+2D0h` to the store at
`0099E739` reads a world position, a Y coordinate, or a terrain/sea height. The two things that
hold a bot's nose up are both attitude terms:

1. `B = max(T, plan+2BCh)` at `0099E4E8`-`0099E512` — the floor of §2c, applied unconditionally on
   every pass of the pitch law. Wire this or a planned bot will follow whatever `plan+2BCh` was last
   set to, including downward.
2. `plan+2BCh` itself, whose slew (`unit+C84h + inc`, §2a) only ever *raises* it toward the live
   pitch: `0099DD4C` writes `sum` only when `unit+C64h > sum`.

A named caveat: this is a scoped negative over the chain I traced, not a census of all 1451
instructions. `Pilot/General/MaxAltOffset`, `ClimbDist` and `DropDist` exist at singleton `+540h`,
`+544h`, `+548h` (= `tuning+8h`, `+Ch`, `+10h`) and none of them is read by either arm — if an
altitude hold exists it is in whoever *sets* `plan+2BCh` and `plan+2C0h` from outside, i.e. the bot
state machine, not the planner.

## Tuning offsets (`EBX = GameTuning + 538h`)

| `tuning+` | singleton | name (`src/game_tuning_singleton.cpp`) | used by |
| --- | --- | --- | --- |
| `+0h` | `538h` | `Pilot/General/WaggleLimit` | roll `t` divisor, `0099E35D` |
| `+3Ch` | `574h` | `Pilot/General/SoftHdgZone` | heading deadband, `0099DEC5` |
| `+40h` | `578h` | `Pilot/General/SoftRollCtrl` | roll soft zone, `0099E30B` |
| `+44h` | `57Ch` | `Pilot/General/SoftRollMul` | roll soft gain, `0099E314` |
| `+48h` | `580h` | **no row in the setter table** | roll outer offset, `0099E332`/`0099E337` |
| `+64h` | `59Ch` | `Pilot/General/TurnRollLimitSmall` | max-bank cap, `0099DFD5` |
| `+68h` | `5A0h` | `Pilot/General/TurnRollLimitLarge` | max-bank cap, `0099DFDC` |
| `+6Ch` | `5A4h` | `Pilot/General/TurnRollPitchLimitPitch/1` | roll limit curve `x0`, `0099E1C6` |
| `+70h` | `5A8h` | `Pilot/General/TurnRollPitchLimitPitch/2` | roll limit curve `x1`, `0099E1B8` |
| `+74h` | `5ACh` | `Pilot/General/TurnRollPitchLimitRoll/1` | roll limit curve `y1`, `0099E1B1` |
| `+78h` | `5B0h` | `Pilot/General/TurnRollPitchLimitRoll/2` | roll limit curve `y0`, `0099E1BF` |
| `+7Ch`/`+80h` | `5B4h`/`5B8h` | `Pilot/General/YawTurnRollRange/1`,`/2` | yaw base gain (already documented) |
| `+88h` | `5C0h` | `Pilot/General/PitchTurnMaxPitch` | the nose-up floor, `0099E4C2` |
| `+8Ch`/`+90h` | `5C4h`/`5C8h` | `Pilot/General/PitchTurnHdgRange/1`,`/2` | `hdgRamp`, `0099E075`/`0099E065` |
| `+A0h` | `5D8h` | `Pilot/General/PitchCtrlSetTimeMul` | pitch authority and turn correction |

`tuning+34h`/`+38h` (`56Ch` `SoftHdgMul`, `570h` `SoftHdgLimit`) are the heading step limit's, as the
existing doc records.

## Contradictions with existing documents — **loud**

1. **`include/bsp/plane_ai_control.hpp:296` `kBankTargetLimited = 0x2BC; // the slewed bank target,
   0099DD4E` is wrong.** `plan+2BCh` is the **pitch** target. Its arm is gated on `task+2D0h`, which
   the same header calls `kPitchMode`, and it terminates in the store to `+29Ch`, the pitch slot.
2. **`docs/PILOT_BOT_PLAN_CONTROLS.md` §"The bank-target arm, `0099DCA4`-`0099DD64`" is
   mis-titled**, and its table row "`unit+C84h` bank | `+2D0h` | `2` = hold | the bank-target arm"
   mislabels `unit+C84h` as the held **bank**. It is a pitch-axis angle. The algebra in that section
   is right; only the axis is wrong.
3. Same section: "`[ESP+10h]`, the alternative target" is listed as **not established**. It is
   `unit+C64h`, the live pitch angle, stored at `0099D4DC` and never overwritten before `0099DD46`.
4. Same section: "`inc = InterpolateClamped(..., t)`" places `007C4810`'s result as the
   interpolation's `x`. It is `x0`, the first argument.
5. Same doc §"The pitch arm's terminal law" says the demand subtracts
   "`cos(pitch) · SlideRatio · YawSpd · tuning+A0h · <factor>`" from the bank target at `0099E53D`.
   Two errors: the factor is `sin(bank)²·cos(pitch)·…` (the `sin²` is missing), and it is subtracted
   from the **measured** angle (slot 12), not from the target `B`.
6. The brief's own list of roll write sites omits `0099E39D`, the only computed one.

None of this touches the yaw arm, whose published account I re-derived where it overlapped and
found correct.

## What is **not** established

* **`0099D0A0`** — not read. It is `__thiscall(plan, float)` with an x87 return and it produces `C`,
  the normalising scale of the whole bank command. Everything about the roll target's *magnitude*
  depends on it; only its role is established here.
* **`00415620`** — read as `clamp(value, lo, hi)` returning `ST0` purely from the call shape
  (`ECX = &value`, `EDX = &lo`, pushed `&hi`, the two constants being `-1.0f` and `+1.0f`, and the
  x87 stack balancing only if it returns a value). Not read. `BSP_Math_ClampInPlace` (`00415690`) is
  its by-reference sibling and *is* in the ledger.
* **`00415510`** (`0099E5C4`, the `plan+2ECh` blend) and **`0099BA10`** (`0099D700`/`0099D745`, the
  direct angle command) — not read.
* **`0047B880`** — the predicate choosing `TurnRollLimitSmall` vs `Large`. Not read.
* **`plan+2E8h`** (the max-bank scale) and **`plan+2C8h`** (the bank cap) — producers outside this
  function, not traced.
* **`unit+C84h`** — proven to be on the pitch axis by its mode word; its *producer* was not found,
  so whether it is the pitch attitude, a flight-path angle or a held command is open.
* **Class descriptor fields** `+18Ch`, `+1A8h`, `+1ACh`, `+1BCh`, `+1C8h`, `+25Ch` are unnamed.
  `+25Ch` behaves as a maximum bank angle in both arms (normaliser at `0099E496`, scale at
  `0099DFAA`); that is a reading of its use, not a recovered name.
* **`tuning+48h`** (singleton `+580h`) has no row in the recovered setter table.
* **Sign conventions downstream.** This packet establishes the algebra of the two `desired` values.
  Whether a positive `+290h` rolls right is a property of `007DA710`, not of `0099D300`, and was not
  checked here.
* Nothing in this packet was built, compiled or run. It is a listing reading only; no Ghidra
  mutation, no ledger record, no lease.
