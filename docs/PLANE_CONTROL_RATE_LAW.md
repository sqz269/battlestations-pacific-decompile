# `007DA710 BSP_PlaneFlight_ControlRateLaw` — structure, partial

`__thiscall(controller /* ECX */, float step)`, body `007DA710`-`007DAFCD`, roughly 2.2 KB, x87 and
SSE mixed. Read-only analysis; no Ghidra mutation. Every name is a hypothesis, not a recovered
symbol.

**This document is deliberately partial.** It records the shape of the law and the sites that
establish it, and stops before the arithmetic, because the arithmetic needs a stack-slot walk with
frame tracking that this pass did not do. See "What is not established" — the gap is named, not
papered over.

## Why it matters

It is the last unreconstructed hop in the chain from a mission-script order to a turning aircraft.
Either side of it exists:

* Above — the order reaches a weapon director and installs a bot task
  (`docs/PILOT_ORDER_BINDINGS.md`, `docs/PILOT_BOT_TASK_OBJECT.md`), and the planner's yaw law has
  every input closed (`docs/PILOT_BOT_PLAN_CONTROLS.md`).
* Below — `007D9C80` produces `ctl+24h..2Ch` from the body-frame angular velocity at
  `ctl+48h..50h`, the rotation sense is proved, and the pose advance is reconstructed
  (`docs/PLANE_ANGULAR_VELOCITY.md`, `docs/PLANE_ADVANCE_POSE.md`).

`007DA710` is what puts a value in `ctl+48h..50h`. Without it a plan slot can carry a yaw command
and nothing turns.

## What is established

**Three axes, three target rates, three deltas.** At `007DAB52`-`007DAB75` the routine takes three
target rates and subtracts the current rate of each axis, keeping both:

```
007dab52  FLD  [ESP+1Ch]      ; target rate, axis 0
007dab56  FLD  ST0            ; keep a copy
007dab58  FSUB [ESI+48h]      ; delta = target - current
007dab5b  FSTP [ESP+40h]      ; store the delta

007dab5f  FLD  [ESP+5Ch]      ; axis 1
007dab63  FLD  ST0
007dab65  FSUB [ESI+4Ch]
007dab68  FSTP [ESP+44h]

007dab6c  FLD  [ESP+18h]      ; axis 2
007dab70  FLD  ST0
007dab72  FSUB [ESI+50h]
007dab75  FSTP [ESP+48h]
```

The `FLD ST0` before each `FSUB` is what makes this readable: the **target** survives on the x87
stack after the delta is popped into its slot, so both quantities are live afterwards.

**Each axis is a rate-limited approach with a symmetric clamp**, and axis 0's tail shows the whole
shape:

```
007dacc5  FMUL [ESP+14h]      ; the step term
007dacc9  FLD  [ESP+68h]
007daccd  FLD  ST0
007daccf  FMULP ST2
007dacd1  FLD  [ESI+48h]      ; the current rate
007dacd4  FADDP ST2,ST0       ; candidate = current + step term
007dacd8  FSTP [ESP+68h]
007dacdc  FLD  [ESP+68h]
007dace0  FLD  [ESP+30h]      ; upper bound
007dace4  FCOMIP ST0,ST1
007dace6  JBE  007dacef
007dacea  MOVAPS XMM1,XMM3    ; saturate high
007daced  JMP  007dad01
007dacef  FLD  [ESP+2Ch]      ; lower bound
007dacf5  FCOMIP ST0,ST1
007dacf9  JA   007dad01       ; saturate low (XMM1 already holds it)
007dacfb  MOVSS XMM1,[ESP+68h]; otherwise take the candidate
007dad01  MOVSS [ESI+48h],XMM1
```

So per axis: **`new = clamp(current + step_term, lower, upper)`**, stored back over the current rate.

**The bounds are a signed pair built by an absolute-value idiom.** At `007DAB79`-`007DAB8A` the
routine tests a value against zero and either stores it or stores `(-0.0f) - value`, with `XMM4`
loaded from `00D7A208` — the `-0.0f` this project has already recorded at that address. That is the
standard `fabs` sequence, so the clamp is `±|limit|` rather than two independent bounds.

**The three store sites** are `007DAD01` (`ctl+48h`), and the corresponding tails for `ctl+4Ch` and
`ctl+50h` — `007DAD77` reads `ctl+4Ch` in the second axis's block.

## What is not established

**The arithmetic of the step term, and therefore the law itself.** `[ESP+14h]` and `[ESP+68h]` are
read at `007DA983`/`007DA963` early in the body and again at `007DACC5`/`007DACC9` in the tail, and
**the prologue pushes between those points**, so the same literal displacement need not name the
same slot at both sites. Resolving them needs a reaching-definition walk with the frame delta
tracked per read — the method `docs/PILOT_BOT_PLAN_CONTROLS.md` used on `0099D300`, which caught
exactly this class of error there (`[ESP+1Ch]` was frame slot `[ESP+14h]` because a `SUB ESP,8` was
live at the read).

Guessing it would be the most expensive mistake available in this chain: a wrong rate law turns
aircraft at the wrong speed or in the wrong direction, and every gunnery number downstream would
look validated without being the game's behaviour.

Also unread: which axis is which (the `ctl+48h`/`4Ch`/`50h` order is not yet matched to
yaw/pitch/roll); where the three target rates at `[ESP+1Ch]`, `[ESP+5Ch]` and `[ESP+18h]` come from;
the class fields the body scales by, though `docs/PLANE_CLASS_FIELDS.md` already attributes
`RollSpd +1A8h` at `007DA7C2`, `PitchSpd +1ACh` at `007DA8EB`, `YawSpd +1B0h` at `007DA926` and
`NegativePitchRatio +1D8h` at `007DA918` inside this body; and the whole region before `007DAB3A`.

## Update: the stack walk, and `ctl+48h` identified

`tools/stack_frame_walk.py` was written for this and normalises every `[ESP+N]` in the body to an
entry-relative frame offset, so two reads of the same literal at different depths are visibly
different slots. It resolves the open questions above in order.

**`[ESP+68h]` is the `step` argument.** Every one of its eight sites - `007DA963`, `007DA96D`,
`007DACC9`, `007DACD8`, `007DACDC`, `007DACFB`, `007DAD31`, `007DAD40` - resolves to `frame=4` at a
stable `depth=100`, and `frame=4` is the first stack argument. So the tail's accumulator and the
step are the same storage, which a literal-offset reading could not have told apart.

**The three target rates are distinct frame slots**, read at `007DAB52`/`007DAB5F`/`007DAB6C`:

| store | target slot | delta slot |
| --- | --- | --- |
| `ctl+48h` | `frame=-72` | `frame=-36` |
| `ctl+4Ch` | `frame=-8` | `frame=-32` |
| `ctl+50h` | `frame=-76` | `frame=-28` |

**`ctl+48h` is the PITCH rate**, established three independent ways in the block that writes its
target at `007DA912`:

```
007da8eb  FLD   [EBP+1ACh]        ; PitchSpd
007da8f1  MOVSS XMM0,[EDI+0BB4h]  ; kLatchedPitch, the previous-step snapshot
007da8f9  COMISS XMM2,XMM0        ; 0 against the latched pitch
007da912  FSTP  [ESP+1Ch]         ; -> frame=-72, the ctl+48h target
007da916  JBE   007da926          ; taken when the latched pitch is not negative
007da918  FLD   [EBP+1D8h]        ; NegativePitchRatio, applied only when it is
007da922  FSTP  [ESP+1Ch]
```

`PitchSpd +1ACh`, `kLatchedPitch +BB4h` and `NegativePitchRatio +1D8h` all feed the same slot, and
the ratio is applied exactly when the latched pitch is negative - nose-down. That matches
`docs/PILOT_BOT_PLAN_CONTROLS.md`'s independent finding that the planner applies the same constant
on the inverted-flight branch.

The block immediately after, `007DA926`-`007DA95F`, opens with `YawSpd +1B0h` and multiplies by
`kLatchedYaw +BB0h`, landing in `frame=-64` - a working slot rather than one of the three targets,
so the yaw path runs through at least one more step before reaching its store.

## Still not established

The **step term's arithmetic**, and therefore the law. `[ESP+14h]` resolves to `frame=-80` at the
`007DACC5` multiply. Traced one level further: the slot is initialised at `007DA983` from
**`[EDI+0BC4h]`**, then reshaped (a multiply by the double at `00D7A280`, a subtract, a comparison
against `1.0`) and clamped at `007DAC30`-`007DAC53` before it multiplies into the step.

`+BC4h` is **not a field this repo has recorded**. It sits between `kLatchedAirBrake = 0xBC0` and
the latched bytes at `0xBC8`..`0xBCA` in `plane_flight.hpp`'s latched block, which names nothing
there. So the step term is now blocked on one named unknown field rather than on an untraced slot -
a smaller and better-posed gap, but still a gap, and still not guessed.

~~**Which of `ctl+4Ch` and `ctl+50h` is yaw and which is roll.**~~ **CLOSED - all three axes are
identified, each by its own class constants and then confirmed as a set by the body-axis
convention.**

| store | axis | evidence |
| --- | --- | --- |
| `ctl+48h` | **pitch** | `PitchSpd +1ACh` (`007DA8EB`), `kLatchedPitch +BB4h` (`007DA8F1`), and `NegativePitchRatio +1D8h` (`007DA918`) applied exactly when the latched pitch is negative |
| `ctl+4Ch` | **yaw** | `YawSpd +1B0h` (`007DA926`) times `kLatchedYaw +BB0h` (`007DA934`) into `frame=-64`, which `007DAA8D` negates against the `-0.0f` at `00D7A208` and `007DAA97` stores to `frame=-8`, this axis's target |
| `ctl+50h` | **roll** | `RollSpd +1A8h` (`007DA7C2`) multiplied into `frame=-76` at `007DA7D6`, and `YawRollRatio +1B4h` (`007DAA68`) added into the same slot at `007DAA70` |

The independent check is that this is exactly the body-axis order. `include/bsp/plane_flight.hpp`
records the body frame as `(x lateral, y up, z forward)`, so angular velocity about x, y and z is
pitch, yaw and roll in that order - which is what the class constants independently say. Two
unrelated lines of evidence agreeing is what makes this safe to state; either alone would not be.

`YawRollRatio` appearing in the **roll** target rather than the yaw one is the coupling a
coordinated turn needs, and it is consistent with `docs/PILOT_BOT_PLAN_CONTROLS.md`, where the
planner's yaw arm blends by bank angle.

The **yaw target is negated** on its way to the store. That sign is recorded rather than smoothed
over: it is a real `(-0.0f) - value` at `007DAA8D`, and anything wiring this axis has to carry it.

The region before `007DAB3A` is read only where it writes the three target slots.

## Next

Trace `frame=-80` (the step-term multiplier) and `frame=-64` (the yaw working slot) with
`tools/stack_frame_walk.py`, then match `ctl+4Ch`/`ctl+50h` against `RollSpd +1A8h` at `007DA7C2`
and `kLatchedRoll +BB8h` the same way the pitch axis was matched. The tool makes each of those a
filter rather than a read.

## The law's form, and the one thing still missing

The step term is **not proportional**. `007DAC9B`-`007DACD4` reads:

```
007dac9b  COMISS XMM0,XMM2
007daca4  JBE  007dacac
007daca6  MOV  [ESP+24h],ECX     ; ECX = -1   (OR ECX,0FFFFFFFFh at 007DAC98)
007dacaa  JMP  007dacc1
007dacac  COMISS XMM2,XMM0
007dacaf  MOV  [ESP+24h],1
007dacb7  JA   007dacc1
007dacb9  MOV  [ESP+24h],0
007dacc1  FILD [ESP+24h]         ; the sign, as an integer loaded to the x87 stack
007dacc5  FMUL [ESP+14h]         ; x the factor at unit+BC4h
007dacc9  FLD  [ESP+68h]         ; x step
007dacd1  FLD  [ESI+48h]         ; + the current rate
007dacd4  FADDP ST2,ST0
```

A three-way `{-1, 0, +1}` selection, `FILD`-ed as an integer, times a factor, times the step. So each
axis approaches its target at a **constant rate** and the clamp is what stops it overshooting:

```
new = clamp(current + sign * factor * step, -|bound|, +|bound|)
```

**The factor is `unit+BC4h`, and it resolves to 1.0.** Its whole lifecycle is now known:
`BSP_Plane_ReadPropertyBag` sets it to `1.0f` (`007D6167`, from the float at `00D7A24C`), the
constructor writes it twice, and the rate law itself maintains it at `007DA978`-`007DA9E3` as a
value that **relaxes toward 1.0** by `0.5 * step` each tick - the double at `00D7A280` is `0.5` -
clamped on both branches so it never crosses 1.0. Initialised at 1.0 and relaxing to 1.0, it is 1.0
throughout in this host unless something not yet found drives it away.

## What is still missing

**The sign's polarity.** `XMM0` and `XMM2` at `007DAC9B` are set outside the window read here, and
`tools/stack_frame_walk.py` tracks `ESP` only - it does not trace SSE registers. So which comparand
is the delta and which is zero, and therefore whether a positive delta yields `+1` or `-1`, is
**not established**.

This is the single most dangerous thing in the function to guess. The whole law is otherwise known,
and a reader who assumed the obvious polarity would produce something that compiles, runs, and turns
every aircraft the wrong way - with the gunnery numbers downstream looking exactly as validated as
they would if it were right. It stays open.

**The bound's source.** `frame=-52` and `frame=-56` feed the clamp; they are built by the `fabs`
idiom at `007DAB79`-`007DAB8A` from a value not yet traced.

## Next

An SSE register trace across `007DAC00`-`007DACC1` to settle the two comparands, then the bound's
producer. The form above means the remaining work is two register questions rather than a law.

## The polarity, settled

`XMM0` is **zero** and `XMM2` is **the delta** at the `007DAC9B` comparison, both established by
exclusion rather than by reading the obvious:

* `007DAB40 XORPS XMM0,XMM0` zeroes it, and a scan of every SSE write form - `MOVSS`, `MOVAPS`,
  `XORPS`, `SUBSS`, `ADDSS`, `MULSS`, `CVTSI2SS`, `DIVSS` with `XMM0` as destination - over the
  whole span from there to the comparison finds **none**. Every intervening mention is a `COMISS`
  read.
* The same scan finds `XMM2`'s last write at `007DAB90 MOVSS XMM2,[ESP+40h]`, which the frame walk
  resolves to `frame=-36` - the delta this function stored for `ctl+48h` at `007DAB5B`.

So `COMISS XMM0,XMM2` compares **zero against the delta**, and the three-way selection reads:

```
delta <  0   ->  -1     ; 007DACA4's JBE not taken, 007DACA6 takes ECX = -1
delta >  0   ->  +1     ; 007DACAF sets 1, 007DACB7's JA keeps it
delta == 0   ->   0     ; 007DACB9
```

**`sign = sign(target - current)`** - the natural polarity, so an axis moves toward its target. It
is worth being explicit that this is the answer a guess would also have given: the value of
establishing it is not that it differs, but that the law can now be wired without anyone having to
trust that the obvious reading was right. A polarity assumed is a polarity that fails silently.

## The complete form

```
delta = target - current                       ; 007DAB52..007DAB75, per axis
sign  = sign(delta)                            ; 007DAC9B..007DACC1
new   = clamp(current + sign * factor * step,  ; 007DACC5..007DACD4
              -|bound|, +|bound|)              ; 007DACE0..007DACFB
```

with `factor` = `unit+BC4h`, which is 1.0 (set by `BSP_Plane_ReadPropertyBag`, relaxing to 1.0 at
`0.5 * step`), and the three axes as `ctl+48h` pitch, `ctl+4Ch` yaw (negated at the target), `ctl+50h`
roll.

## What remains

**The bound.** `frame=-52` and `frame=-56` feed the clamp. `frame=-56` is `|XMM1|` built by the
`fabs` idiom at `007DAB79`-`007DAB8A`, where `XMM1` traces to `frame=-24` - the product `007DAA7F`
forms from `PitchAccel +1C0h`. `frame=-52` traces to the delta itself at `007DAB9B`. So the clamp is
built from an accel-derived term and the delta, which is the shape of an overshoot guard, but the
exact expression is not traced and is **not** being inferred from that shape.

No pure rule is written yet for this reason: the law is four-fifths established and the clamp is the
remaining fifth. Everything above is evidence; the clamp would be a guess.
