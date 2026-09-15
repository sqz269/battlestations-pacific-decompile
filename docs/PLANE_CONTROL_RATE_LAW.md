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

## Correction: the clamp is not a bound, and the factor is not in the step

Two claims published above are **wrong**, and both were wrong in the same way - a shape assumed from
a fragment rather than traced. Tracing the x87 stack through the clamp settles both.

### The clamp is the interval between the current value and the target

The published form said `clamp(..., -|bound|, +|bound|)`. There is no such bound. `007DACE0`-`007DAD01`
reads:

```
007dacdc  FLD   [ESP+68h]        ; cand
007dace0  FLD   [ESP+30h]        ; A          frame=-52
007dace4  FCOMIP ST0,ST1         ; A vs cand
007dace6  JBE   007dacef
007dacea  MOVAPS XMM1,XMM3       ;   A > cand  -> result = A
007dacef  FLD   [ESP+2Ch]        ; B          frame=-56
007dacf5  FCOMIP ST0,ST1         ; cand vs B
007dacf9  JA    007dad01         ;   cand > B -> result = B  (XMM1 still holds B)
007dacfb  MOVSS XMM1,[ESP+68h]   ;   otherwise -> result = cand
007dad01  MOVSS [ESI+48h],XMM1
```

and `A` and `B` are built immediately before it, at `007DAC59`-`007DAC9E`, by comparing the current
axis value against **the target**:

```
007dac59  FLD   [ESI+48h]        ; cur
007dac5c  MOVSS XMM3,[ESP+1Ch]   ; XMM3 = T   frame=-72, the pitch target
007dac6a  FCOMIP ST0,ST4         ; cur vs ST4
007dac6c  JBE   007dac76         ;   -> XMM1 = max(cur, T)  stored to frame=-56 at 007DAC7C
007dac8a  FXCH  ST4
007dac8c  FCOMIP ST0,ST4
007dac90  JBE   007dac98         ;   -> XMM3 = min(cur, T)  stored to frame=-52 at 007DAC9E
```

`ST4` is the target, and that is **traced, not assumed**. The three deltas at `007DAB52`-`007DAB75`
are each formed by `FLD target / FLD ST0 / FSUB [ESI+axis] / FSTP delta`, which leaves a copy of each
target on the x87 stack; after the three the stack holds, bottom to top, the pitch, yaw and roll
targets. `007DABB2`-`007DABE0` pushes and pops in balance, `007DABE4` pushes one more, and `007DAC59`
pushes `cur`, putting the pitch target at exactly `ST4`.

So the law's last step is

```
new = clamp(cand, min(current, target), max(current, target))
```

which is not a bound at all - it is a **non-overshoot guard**. The rate law moves an axis toward its
target at a constant rate and this stops it passing the target in a single step. Axes 1 and 2 repeat
the identical shape at `007DAE62`-`007DAE85` and in the third block, with the branch senses inverted
and the registers reallocated.

### `unit+BC4h` scales the yaw target, not the step

The published form said `FMUL [ESP+14h]` at `007DACC5` multiplies the sign by "the factor at
unit+BC4h". It does not. `[ESP+14h]` is `frame=-80`, a **scratch slot reused three times**, once per
axis, and the `+BC4h` value it holds early is dead by the time the step is formed:

* `007DA983`-`007DAA14` builds the `+BC4h`-derived value in `frame=-80`.
* `007DAA18` reads it out and `007DAA1F` stores it to `frame=-44`, where `007DAA4E` multiplies it
  into the **yaw target** alongside `YawSpd +1B0h` and `cos(+0C64h)`. That is its only consumer.
* `007DABE0` then **overwrites** `frame=-80` with a value of its own, and `007DAD6C` and `007DAEEF`
  overwrite it again for axes 1 and 2. Every write and read of the slot is accounted for; there is no
  path by which the `+BC4h` value reaches `007DACC5`.

The earlier finding that `+BC4h` resolves to 1.0 still stands. Its **placement** was wrong, and
because 1.0 is a no-op either way, nothing downstream would have exposed the error.

## The rate is a quadratic in the remaining error, and it is named

What `007DACC5` actually multiplies is built at `007DABB2`-`007DABE0`:

```
007dabb2  FLD   [00F872FCh]      ; A
007dabb8  FLD   [ESP+40h]        ; delta
007dabc4  FLD   ST0
007dabc6  FMULP ST2              ; A*delta
007dabc8  FMULP                  ; A*delta^2
007dabca  FLD   [ESP+30h]        ; |delta|
007dabce  FMUL  [00F87300h]      ; B*|delta|
007dabd4  FADDP                  ; A*delta^2 + B*|delta|
007dabd6  FADD  [00F87304h]      ; + C
007dabdc  FMUL  [ESP+2Ch]        ; x |accel term|   (|frame=-24|, the PitchAccel +1C0h product)
007dabe0  FSTP  [ESP+14h]        ; -> the rate
```

The three globals are read only here, once per axis. Their names are recovered: the loader that
fills them pushes `00CE608C`, `00CE60E4` and `00D08204`, and those are not names of fields but the
one-character strings **"A", "B" and "C"** - property-bag keys, read at `007E3CBE`, `007E3D00`
and `007E3D42`. The section string sitting beside "C" at `00D08208` is `RotationFactors`.

Then a floor, and only a floor - there is no ceiling:

```
BL == 0   ->  rate = max(rate, 0.15f)             ; 00CE3D30 = 3F19999Ah, .rdata, a real constant
BL != 0   ->  rate = max(rate, 1.5 * |current|)   ; 00CE3D78 = 1.5 as a double
```

and the second floor is guarded by a sign test at `007DABEA`-`007DAC06` which, decoded branch by
branch, applies it exactly when **the delta and the current deflection lie on opposite sides of zero**
- when the axis has to cross neutral to reach its target. A surface reversing direction gets a
minimum rate proportional to how far it is currently deflected, so the harder it is over, the faster
it comes back. `BL` is an output byte from `007DA380`, whose address `007DA717` takes in the prologue;
what it distinguishes is **not** established.

## The trap in A, B and C, worth recording on its own

Read statically, all three coefficients are **0.0**, and Ghidra finds no writer for any of them. The
obvious conclusion - a disabled tuning term, rate reduced to the floor - is wrong, and it is worth
setting down why, because the same shape has now cost this project time twice.

`00F872FC` is in `.data`, but `.data` has `vsize=0x297EDC` against `rawsize=0x10000`: the section is
mostly **uninitialised**, and `00F872FC` sits far past the file-backed part. The zeros are the
loader's zero-fill, not authored data, so reading them establishes nothing about runtime.

And there is a writer. It does not xref because it is a block copy:

```
007eaad7  MOV  ECX,4Eh           ; 78 dwords = 312 bytes
007eaadc  MOV  EDI,0F872F0h
007eaae1  REP MOVSD              ; bytes F3 A5 - a copy, not a STOSD fill
```

from `ESI = EBP+210h` (`007E2D03`), `EBP` being the `this` of `BSP_GameTuning_LoadFromPlaneGlobals`,
and `ESI` is provably untouched across the 6250 instructions in between - the function mentions it
fifteen times and writes it twice, `XOR ESI,ESI` and that `LEA`. So the whole plane tuning block is a
312-byte mirror of `this+210h..348h`, and A, B, C are entries 3, 4 and 5 of it.

This is the `class+50h` hazard again, and this time it is confirmed rather than suspected: **a
`REP MOVSD` writes 78 globals and xrefs only one of them.** A negative from Ghidra's xref table over
a `.data` address means "no literal-address writer", never "no writer". Two checks are needed: the
section's file-backed extent, and a scan for the region's base address as an immediate.

## The law, as far as it is established

```
delta  = target - current                                  ; 007DAB52..007DAB75
rate   = (A*delta^2 + B*|delta| + C) * |accelTerm|          ; 007DABB2..007DABE0
rate   = max(rate, BL ? 1.5*|current| : 0.15f)              ; 007DABE4..007DAC53
cand   = current + sign(delta) * rate * step                ; 007DAC9B..007DACD4
new    = clamp(cand, min(current, target), max(current, target))   ; 007DACE0..007DAD01
```

with `step` the single stack argument at `frame=4` - used directly for axis 0 at `007DACC9`, then
reused as scratch for the candidate at `007DACD8`, with axes 1 and 2 taking it from an x87 copy at
`007DAE55`. `this` is the control block: `ECX` at entry, axes at `+48h` pitch, `+4Ch` yaw, `+50h` roll.

**Open, and each is a value rather than a shape:** A, B and C's runtime values, which are
authored data reached through `RotationFactors`; the meaning of `BL`, which needs `007DA380`; and the
`|accelTerm|`'s full provenance beyond the `PitchAccel +1C0h` product at `007DAA7F`.

The structure is now complete and traced end to end. What is missing is three numbers and a flag, not
a form - which is a different kind of gap, and a wireable one, because a rate law with the wrong
coefficients moves an axis at the wrong speed toward the right place, where a law with the wrong
clamp or the wrong sign moves it to the wrong place entirely.

## The three numbers, from authored data

`A`, `B` and `C` are not in the executable at all. `BSP_GameTuning_LoadFromPlaneGlobals` is named for
where it reads them, and this repository had already recovered the field names and their property
paths - `include/bsp/game_tuning_singleton.hpp:172-174` carries
`Dynamics/RotationFactors/{A,B,C}` at `+21Ch`, `+220h`, `+224h`, which is exactly the block offsets
the `REP MOVSD` maps to `00F872FC`, `00F87300` and `00F87304`. The two halves were recovered
independently and meet.

The values are in `scripts/datatables/planeglobals.lua`:

```lua
["RotationFactors"]=
{
    ["A"]=0.5,
    ["B"]=1.5,
    ["C"]=0.1,
},
```

**A caveat that has to travel with the numbers.** This installation is modded, and
`planeglobals.lua` is not part of the untouched bulk: 29 of the 38 files in `scripts/datatables`
carry the install date `2024-07-13`, and this file is one of only two at `2024-10-29`. It is not
locally edited - the one file this user has changed carries `2026-05-09` - but it was replaced after
the original install, so these are **this installation's** rotation factors and not provably retail.

There is one piece of evidence that they are nevertheless the original values, and it is worth
stating because it is inference rather than proof. The reverse-crossing floor hard-coded in the
executable is `1.5 * |current|`, from the double at `00CE3D78`; the authored linear coefficient `B`
is also `1.5`. The floor is the linear term with `|current|` substituted for `|delta|` - the same
coefficient, applied to the deflection instead of the error, which is what you would write if the
axis has to treat its own deflection as the error while crossing neutral. A modder retuning `B` would
have no reason to also match a constant compiled into the exe, so the agreement is more easily
explained by `B` never having been retuned. Suggestive, not conclusive.

`C = 0.1` sits just under the `0.15f` idle floor at `00CE3D30`, so as an axis converges and `delta`
goes to zero the authored polynomial falls below the floor and the floor takes over. The two were
tuned against each other, which is a further small sign the pair is coherent.

## The floor selector, read rather than named

`BL` came from `007DA380`, and calling it "crossing" - which the shape of the branch invites - would
have been wrong. `007DA380` is a three-out-parameter helper (`RET 0Ch`; `&f1` to `frame=-68`, `&f2`
to `frame=-52`, `&flag` to `frame=-81`) that **switches on `this+0FCh`**:

```
007da38d  MOV EAX,[ESI+0FCh]
007da393  SUB EAX,0 / JZ 007da6e6      ; mode 0
007da39c  SUB EAX,1 / JZ 007da542      ; mode 1
007da3a6  SUB EAX,1 / JZ 007da3d0      ; mode 2
          default: flag = 0, f1 = f2 = [00D7A238]
```

So the flag is a **flight-mode output**, not a geometric predicate, and the mode field `this+0FCh` is
the input that decides it. Only the default arm is read here, and it clears the flag.

That matters, because the two conditions are nested and easy to collapse into one. Read correctly:

```
BL == 0                              ->  rate = max(rate, 0.15)
BL != 0  and delta, current are not strictly the same sign
                                     ->  rate = max(rate, 1.5*|current|)
BL != 0  and delta, current are both positive or both negative
                                     ->  no floor at all
```

The third case is a real branch - `007DABFC` and `007DAC06` jump past the floor entirely - and
writing `max(rate, BL ? 1.5*|current| : 0.15)` would have silently imposed a floor the game does not
apply while an axis is already moving the right way.

The sign test at `007DABEA`-`007DAC06`, branch by branch: `delta >= 0` with `current <= 0` floors;
`delta < 0` with `current >= 0` floors; `delta == 0` with `current > 0` floors; the two same-sign
cases skip.

## The law, complete but for one input

```
delta  = target - current
rate   = (0.5*delta^2 + 1.5*|delta| + 0.1) * |accelTerm|
if      (!flag)                        rate = max(rate, 0.15)
else if (!same_strict_sign(delta, current))  rate = max(rate, 1.5*|current|)
cand   = current + sign(delta) * rate * step
new    = clamp(cand, min(current, target), max(current, target))
```

per axis, `current` being `this+48h` pitch, `this+4Ch` yaw, `this+50h` roll, and `step` the single
stack argument. `flag` is `007DA380`'s third output, driven by the flight mode at `this+0FCh`; three
of its four arms are unread, which is what stands between this and a pure rule.

## The flag, closed for free flight

`007DA380` already had a name and an ABI in this ledger -
`BSP_PlaneFlight_ControllerModeFactors`, `__thiscall(ctl, float*, float*, unsigned char*)`,
`RET 0Ch`, recovered by packet `cc2_loose_ends_2` - and the ABI I walked to independently matches it
exactly. `docs/PLANE_FLIGHT.md:79` already had the mode field: `ctl+FCh`, **zeroed every step** at
`007DC841`, with `== 1` meaning on the ground.

Zeroed every step means the free-flight case is **mode 0**, and mode 0's arm is ten instructions:

```
007da6e6  MOVSS XMM0,[ESP+4]     ; the scalar 007D9A70(this) returned
007da6f8  MOVSS [ECX],XMM0       ; f1
007da6fc  MOVSS [EDX],XMM0       ; f2 - the same value
007da700  MOV byte ptr [EAX],1   ; flag = 1
```

So in free flight the flag is **1**, and the `0.15` idle floor never applies to a plane in the air;
the sign-guarded `1.5 * |current|` floor does, and only while the axis has to cross neutral. The
`0.15` branch belongs to the default arm (`007DA3AB`, which clears the flag) and to whichever of the
two remaining modes clears it.

That closes the last input for the case the reconstruction needs. It came out of two facts this
repository had already recorded and I had not connected - the name and ABI in the ledger, and the
per-step zeroing in `PLANE_FLIGHT.md` - which is worth noting, because I spent a call reading a
dispatcher whose answer was already written down.

## The accel term

`|accelTerm|` is `frame=-24`, built at `007DAA7F`-`007DAA9D`:

```
007daa7f  FLD  [EAX+1C0h]        ; PitchAccel, EAX = this+0Ch, the class descriptor
007daa93  FMUL ST1               ; bytes D8 C9 = FMUL ST(0),ST(1) - ST0 *= f1
007daa9d  FSTP [ESP+4Ch]         ; frame=-24 = PitchAccel * f1
```

The direction of that `FMUL` had to be read from the bytes: Ghidra prints `FMUL ST1` for both
`D8 C8+i` (`ST0 *= STi`) and `DC C8+i` (`STi *= ST0`), and the two give different answers here. `D8 C9`
is the first. The same check settles `007DAA66`, where `DE CA` is `FMULP ST(2),ST(0)`.

The yaw and roll terms follow in the same block - `class+1C4h` against `-f2` into `frame=-20`, and
`PitchAccel * f1 * class+1BCh` into `frame=-16` - and all three axes then run the **identical**
polynomial: `007DAD4E`-`007DAD6C` and `007DAED0`-`007DAEEF` read the same `A`, `B`, `C` globals and
multiply by their own accel term, exactly as `007DABB2`-`007DABE0` does for pitch.

## Free flight, as a rule

```
delta = target - current
rate  = (0.5*delta^2 + 1.5*|delta| + 0.1) * |accel_axis|
if (sign(delta) != sign(current))      rate = max(rate, 1.5*|current|)
cand  = current + sign(delta) * rate * step
new   = clamp(cand, min(current, target), max(current, target))
```

where `sign(delta) != sign(current)` is the strict reading decoded at `007DABEA`-`007DAC06` - either
being zero counts as a mismatch except `delta == 0` with `current < 0`, which skips.

This is wireable. The structure is traced end to end, the coefficients are authored data read from
this installation's `planeglobals.lua` with the provenance caveat above, and the one runtime flag is
resolved for the free-flight path. What is not yet verified is the rule's **behaviour** - nothing has
been run, and until a plane under a bot task actually turns toward its target in a mission, this is a
recovered law and not a validated one.

## Corrections from packet `cc7_plane_control_targets`

Three claims above are wrong, all mine, and all wrong the same way. I read a stack slot's first
producer and assumed it was still live at a later read, without checking for a write in between.
`tools/stack_frame_walk.py` exists precisely because a literal `[ESP+N]` names different storage at
different depths - and I applied that discipline to the offsets while skipping it for the values in
them. The slots in this function are scratch, reused three and four times each. **A slot's value has
to be traced from the read backwards to its nearest preceding write, every time.**

### `unit+BC4h` reaches the yaw target, but not the way the section above says

The conclusion stands - `+BC4h` does not multiply the step. The mechanism, the address, the slot and
the arithmetic in that section are all wrong:

* `007DAA12 FSIN` / `007DAA14 FSTP [ESP+14h]` **overwrites** `frame=-80` with `sin(unit+C68h)`
  before `007DAA18` reads it. So `frame=-44` holds the sine, not the `+BC4h` value.
* The `+BC4h` chain ends at `007DA9E3 MOVSS [EDI+0BC4h],XMM0`: it writes the **field** and goes no
  further.
* `007DAA4E`'s term is `SlideRatio(class+1B8h) * YawSpd * sin(bank) * cos(pitch)`, and it is
  **subtracted** at `007DAA56` (`DE E9 FSUBP`), not multiplied in.
* `+BC4h`'s real consumer is `007DA94B`, through `frame=-56`, 280 bytes earlier - inside the yaw
  raw product, which is where it does scale the yaw target.

### The roll acceleration term is `f1 * RollAccel`, and the oddity I flagged was my own slip

The section above reports `frame=-16 = PitchAccel * f1 * class+1BCh` and then flags the dependence
on `PitchAccel` as surprising, worth a second pair of eyes, possibly a shipped bug. It is none of
those. It is an arithmetic error of mine:

```
007daa7f  FLD  [EAX+1C0h]    ; stack: f1, PitchAccel        ST0 = PitchAccel
007daa93  FMUL ST1           ; D8 C9 = FMUL ST(0),ST(1)     ST0 = PitchAccel * f1
007daa9d  FSTP [ESP+4Ch]     ; frame=-24 = PitchAccel * f1, POP
                             ; ST0 is now f1 again - not the product
007daab3  FMUL [EAX+1BCh]    ; ST0 = f1 * RollAccel
007daab9  FSTP [ESP+54h]     ; frame=-16 = f1 * RollAccel
```

I tracked the `FSTP` as storing and forgot it pops, so I carried the product forward where the
listing carries `f1`. The three acceleration terms are perfectly symmetric -
`f1 * PitchAccel`, `-f2 * YawAccel`, `f1 * RollAccel` - which is what one would expect, and my
flagged anomaly was an invitation to go looking for intent behind a mistake I had made myself.
`docs/PLANE_CONTROL_AUTHORITY.md` repeats the same error in its table and is corrected there.

### The latched block is on the unit, not the controller

`007DA72D MOV EDI,[ESI+8]` makes `EDI` the unit, and **every** structure field this function reads -
`+BB0h`, `+BB4h`, `+BB8h`, `+BC4h`, `+838h`, `+900h`, `+5Dh`, `+C36h`, `+C37h`, `+C3Ch`, `+C64h`,
`+C68h` - is `EDI`-based. Writing them `ctl+BB0h`, as this doc did throughout, sends a reader to the
wrong object by one indirection. `include/bsp/plane_flight.hpp` never had this wrong; its comment
says "the pilot control block, **unit+9E4h**". Only the namespace's name suggests a controller base.

The three axis fields the law WRITES - `+48h`, `+4Ch`, `+50h` - are genuinely `ctl`-relative
(`ESI`-based), so the two bases really are mixed inside one function, which is how the confusion
started. It is not an excuse: `007DA72D` is the twelfth instruction.

### Two open items closed by the same packet

The **ground gate**: `007DA8D9 CMP [ESI+FCh],1` / `007DA8E3 JNZ 007DA8EB` zeroes the roll target
when the mode **is** 1, on the ground. The Ghidra plate comment inherited from packet
`cc2_plane_flight` has this backwards - "discarded when controller+FCh is not 1" - and is corrected
in the ledger by this packet.

The **flag**: `007DA9F5 JZ 007DAA7C` gates a block that adds the yaw slide term and the yaw-roll
coupling. Free flight sets the flag to 1, so in the air both apply. On the ground the flag comes
from `007DA542`, unread, so whether the coupling is added back onto a roll target `007DA8E5` has
just zeroed is open.

The three complete target expressions are in **`docs/PLANE_CONTROL_TARGETS.md`**.
