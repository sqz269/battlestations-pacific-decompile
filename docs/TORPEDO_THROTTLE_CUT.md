# There is no throttle cut: step 4's second value is dead, and the 12 m is the image's own

Addresses: 009D07B0, 009D07BD, 009D07D4, 009D07DC, 009D07E4, 009D07FE, 009D0868, 009D0888,
009D08A4, 009D08AE, 009D08BB, 009D08C3, 009D0951, 009D0957, 009D095D, 009D096B, 009D09C2,
009D09CC, 009D09D6, 009D09F0, 009D09FC, 009D0A08, 009D0A1F, 009D0A23, 009D0A26, 009D0A3C,
009D0A46, 009D0A50, 009D0A5A, 009D0A63, 009D0A68, 009D0A6B, 009D0A71, 009D0A79, 009D0A81,
009D0A92, 009FBA50, 009FBAD2, 009FBAC5, 009FB800, 009D3489, 00939E77, 00939E83, 009C8920,
009C89CE, 007BB6E0, 007BB7B9, 007BB82E, 007BB83A.

Packet `cc8_torpedo_throttle_cut`, owner `agent/cc8-torpedo-run-in`, on `5ee71603f`.

**Negative result.** The packet set out to bind the attack-run throttle cut so a 60-degree dive
would hold a speed the release gate could be crossed at. There is no throttle cut. Step 4 of
`009D07B0` commands no throttle, the value that looked like one is computed and discarded, and the
commanded run-in altitude of 12 m that produces the steep dive is the image's own number for a
torpedo bomber rather than a host gap. What is actually missing is named in section 4.

## 1. Step 4's second value is `009FBA50`'s `scale`, not a throttle

**The deciding instruction is `009D0A6B`.**

`docs/BOT_TASK_STATES.md` "The torpedo run" row 4 reads: *altitude `approach->+78h +
approach->+74h` through `009FBA50`, **throttle** from `InterpolateClamped([00D7A2F0], [00CF6560],
[00CE7804], [00CE74F8], .)` over the height margin `[00D1F8D0] - unitY`...*

The listing puts that interpolation's result into `009FBA50`'s **fourth float argument**:

```
009d0a3c  FLD [0x00ce74f8] -> [ESP+0xc]    ; y1 = 0.8
009d0a46  FLD [0x00ce7804] -> [ESP+0x8]    ; x1 = 0.4
009d0a50  FLD [0x00cf6560] -> [ESP+0x4]    ; y0 = 0.35
009d0a5a  FLD [0x00d7a2f0] -> [ESP]        ; x0 = 0.1
009d0a63  CALL 00419010                    ; t = InterpolateClamped(0.1, 0.35, 0.4, 0.8, x)
009d0a68  SUB  ESP,0x10
009d0a6b  FSTP [ESP+0xc]                   ; 009FBA50's 4th float -> `scale`
009d0a71  FLD  [ESP+0x18] -> [ESP+0x8]     ; 3rd float, rangeHigh
009d0a79  FLD  [ESP+0x1c] -> [ESP+0x4]     ; 2nd float, rangeLow
009d0a81  FLD  [EDI+0x78]
009d0a84  FADD [EDI+0x74]
009d0a8f  FSTP [ESP]                       ; 1st float, the base altitude
009d0a92  CALL 009fba50
```

Every constant checked against the data section: `00D7A2F0` = `0.1f`, `00CF6560` = `0.35f`,
`00CE7804` = `0.4f`, `00CE74F8` = `0.8f`, and the three doubles the doc's row names are
`00D1F8D0` = `1400.0`, `00CF0DD8` = `2000.0`, `00CF3F20` = `15.0`. The interpolation's `x` is
`(1400 - unit+100h) / min(approach+90h, 2000)` (`009D0951`-`009D095D` for the margin,
`009D09CC`-`009D09F0` for the divisor, `009D0A26` for the divide), and the result is clamped into
`[0.35, 0.8]`.

`009FBA50` uses `scale` in exactly one place, its own step 4: `base += span · scale · class+518h`,
**and only when `span = max(rangeHigh - rangeLow, 0)` is positive**. Otherwise the argument is
untouched except that `009FBAC5`'s `FLD` leaves it in `ST0` at the `RET`, which every call site
discards.

## 2. `span` is always zero here, so the value never reaches anything

**The deciding instructions are `009D07E4` and `009D0A08`.**

Both of `009FBA50`'s range arguments are the same field:

```
009d07bd  MOVSS XMM0,[EAX+0x94]   ; approach+94h
009d07d4  MOVSS [ESP+0x8],XMM0
009d07dc  MOVSS XMM0,[EAX+0x90]   ; approach+90h, the range
009d07e4  MOVSS [ESP+0xc],XMM0    ; -> rangeLow
...
009d09fc  MOVSS XMM0,[EDI+0x90]   ; approach+90h again
009d0a08  MOVSS [ESP+0x8],XMM0    ; -> rangeHigh
```

`[ESP+0xc]` is `009FBA50`'s `rangeLow` (`009D0A79` reads it as `[ESP+0x1c]` after the `SUB
ESP,0x10`) and `[ESP+0x8]` is its `rangeHigh` (`009D0A71`, `[ESP+0x18]`). Both hold
`approach+90h`, so **`span` is zero on every tick** and the `scale` term is unreachable from this
call site.

**The frame arithmetic was checked rather than assumed**, because `[ESP+n]` shifts here twice.
`009D07FE` pushes `EBX` and `009D08C3` pops it, and `009D0868`'s `SUB ESP,0x14` is balanced by
`00419010`'s `RET 14h`. So the stores at `009D08A4` and `009D08AE`, which look like writes to the
same two locals, are inside that window and are the **interpolation's own argument slots** - which
is confirmed by `009D0888 FLD [ESP+0x24]` reading the function's `[ESP+0xc]` from inside it, the
`approach+90h / approach+88h` that `docs/BOT_TASK_STATES.md` records as step 2's input. Nothing
between the pop and `009D09FC` writes either local.

So the interpolation at `009D0A63` is **computed every tick and used by nothing**. Whether that is
dead code in the original or a term some other caller of `009FBA50` reaches is not settled here;
what is settled is that this call site cannot reach it.

## 3. The throttle's real path, and why no torpedo state touches it

`007BB6E0` is where a plan value becomes the live throttle `unit+9F0h`, and
`docs/PILOT_PLAN_SLOT_PIPELINE.md` "Throttle is not always quantised" has the shape: the request
byte `cmd+14h` gates the axis, and only the `cmd+14h == 0` path at `007BB82E`-`007BB83A` takes the
planned `cmd[3]`. For a bot that is the normal path, because `0099B450` clears `plan+2E4h` every
think.

So the throttle a bot flies is the plan's throttle slot. **No routine in the torpedo chain read so
far writes it**: `009D07B0` writes `cmd+2C0h`, `+2CCh`, `+278h`, `+27Ch`, `+2A8h`, `+2ACh` and
`+2D8h` and, through `009FBA50`, `+2BCh` and `+2D0h`; the aim tick `009D15F0` writes `+2C0h`,
`+2CCh`, `+2C8h` (the bank cap, `docs/PLANE_POSE_THROTTLE_ALTITUDE.md` section 3), `+2BCh`, `+2D0h`,
`+2E8h` and the five constant bits. None of them is the throttle slot. `0099B450` seeds it from the
live value, so it holds whatever the aircraft already had.

**There is nothing to bind.** This host's plane flies the run-in at the throttle it was created
with, and so, on this evidence, does the image's.

## 4. The 12 m is the image's own number, and what is actually missing

The dive is steep because `009FB800` saturates: its demand is
`-min(DropAngle · t, max(DropAngle · 1.6, DEG(60)))` with `t = clamp(-weighted / DropDist, 0,
reference)` and `weighted = err · (reference + 1) · 0.5`. With the commanded altitude at 12 m and
`DropAngle` 0.4014, the `DEG(60)` cap binds for any error above about **80 m**. An aircraft 788 m
above its commanded altitude therefore dives at 60 degrees for the whole descent, which is what the
run measured.

The obvious suspicion is that the commanded altitude is wrong - that `approach+74h` should carry a
cruise altitude and the host leaves it at zero. **It should not.** `009D3489` fills `approach+74h`
from the control record's `+398h` when that is below the double `100` at `00D7A220`, and `+398h`:

* is constructed **zero** - `00939E77 XORPS XMM0,XMM0` then `00939E83 MOVSS [ESI+0x398],XMM0` in
  `BSP_UnitController_ConstructVariantB`; and
* has exactly one gameplay writer in the image, `009C89CE` inside
  `009C8920 BSP_BotTaskDiveBomb_UpdateCruiseProfile`, which sets it to a uniform random in
  `[0, 00CE5380]` plus the tuning float at `+4CCh`, and is the **dive bomb** task's.

So for a torpedo bomber `approach+398h` stays zero, `approach+74h` is zero, and the commanded
run-in altitude is `0 + TorpReleaseAlt` = **12 m exactly as this host computes it**. The host's
`74h=0.00` in the census is the image's value, not a gap.

**What is missing is upstream of `attackrun`.** The aircraft enter it at their 800 m spawn
altitude. `kTorpedoCruiseProfile` (`src/bot_task_states.cpp`) points at `Pilot/Torpedo/CruisingAlt`,
authored **500**, and it is the `moveto` tick `009C18C0` that would command it. This host runs the
state machine's per-state tick for `aim` and, since `docs/TORPEDO_RUN_IN_DESCENT.md`, `attackrun` -
**and for nothing else**. `moveto` and `follow` never tick, so no aircraft is ever brought to its
cruise altitude before the attack run begins.

That does not by itself fix the arrival speed - a 60-degree dive from 500 m still reaches the sea
fast - but it is the next real gap on this path, and it is the same shape as the one the descent
packet closed.

## ABI

* `009D07B0` `BSP_BotStateTorpedoAttackRun_Tick`, Ghidra body `009D07B0`-`009D0B0E`. Prologue `SUB
  ESP,3Ch` / `PUSH ESI` / `PUSH EDI`; `EBX` is pushed at `009D07FE` and popped at `009D08C3`.
* `009FBA50`, `float __thiscall(this, float base, float rangeLow, float rangeHigh, float scale)`,
  `RET 10h`. `scale` is read only inside `if (span > 0)` at `009FBAD2`, and `009FBAC5` leaves it in
  `ST0` at the `RET`.
* `009C8920` `BSP_BotTaskDiveBomb_UpdateCruiseProfile`, the only gameplay writer of `+398h`.

## Uncertainty

* Whether some **other** caller of `009FBA50` passes a real range pair, which would make the
  `scale` term live and the `009D0A63` interpolation meaningful for them. `009FBA50` has four call
  sites by `include/bsp/bot_tasks.hpp:279` - `009C18C0`, `009A3770`, `009A4010` and `009D07B0` -
  and only this one was read here.
* `009C8920`'s conditions, and whether a torpedo-task twin exists that this packet did not find.
  The census over `398h` was exhaustive, so any twin would have to write through a pointer rather
  than a displacement.
* `class+518h`, `009FBA50`'s per-class gain, is still unmodelled and still unreachable here.

## Host methods

**None.** This packet changes no host code: there is no throttle command in the chain to bind, and
the commanded altitude the host already computes is the image's own. The two files the binding
would have touched were not needed and were not claimed.

## Corrections

Appended to the docs they amend, and verified present there.

* `docs/BOT_TASK_STATES.md` "The torpedo run" row 4 calls the second value a **throttle**. It is
  `009FBA50`'s `scale` argument, and it is unreachable from this call site because both range
  arguments are `approach+90h`.
* `docs/TORPEDO_RUN_IN_DESCENT.md`'s "step 4's throttle half ... is not run" and its follow-up 3,
  and `docs/PLANE_ALTITUDE_HOLD_AND_SURFACE.md`'s follow-up 0, which made the throttle cut the next
  gate. There is no throttle cut. Passing `rangeLow = rangeHigh = 0` in that binding gives the same
  commanded altitude as the native's own arguments, so the binding is right for a reason it did not
  know.

## no_ghidra_function

None. `009D07B0`, `009FBA50`, `009FB800`, `009C8920`, `00939E50` and `007BB6E0` all have Ghidra
functions.

## Validation

No run. This packet changes no behaviour, so a before and after pair would be the same binary; the
measurements it reasons from are `docs/PLANE_ALTITUDE_HOLD_AND_SURFACE.md`'s, whose census printed
`base=12.00 (74h=0.00 78h=12.00)` and a `-1.0472` rad demand held from 800 m to the water. Section 4
explains both of those numbers as the image's own rather than as host gaps, which is the result.

## Follow-up packets

1. **The `moveto` and `follow` state ticks**, `009C18C0` above all, so an aircraft reaches
   `attackrun` at `Pilot/Torpedo/CruisingAlt` rather than at its spawn altitude. This is the gate.
2. **The other three callers of `009FBA50`** - `009C18C0`, `009A3770`, `009A4010` - to find whether
   any passes a real range pair and makes the `scale` term live.
3. **What sets a bot's throttle at all**, if anything does. The plan's throttle slot has no writer
   in the torpedo chain, and `0099B450` only reseeds it from the live value.


## Correction from packet `cc8_torpedo_moveto_tick`: the first open question is answered

Appended, not rewriting the sections above.

The Uncertainty section asks "whether some **other** caller of `009FBA50` passes a real range pair,
which would make the `scale` term live and the `009D0A63` interpolation meaningful for them".

**`009C18C0` does.** Its call at `009C1B17` passes `this+38h` as `rangeLow` and the live distance to
the target as `rangeHigh`, so `span` is positive for the whole approach and `009FBAD2`'s term drives
a glide slope: `span * t * class+518h`, with `class+518h` derived as `tan(desc+1F0h DropAngle)` at
`007C4A44`. So the interpolation shape this doc found dead at the attack-run site is the same
computation that shapes the approach descent at the move-to site.

That does not change anything here: the attack-run site still passes `approach+90h` as both
arguments, its `scale` is still discarded, and passing a zero range pair there is still exactly
right. `009A3770` and `009A4010` remain unread. `docs/TORPEDO_MOVETO_TICK.md`.


## Correction from the `cc8_torpedo_moveto_tick` follow-up: the planner writes the throttle

Appended, not rewriting section 3.

Section 3 says "**No routine in the torpedo chain read so far writes it**" and the contracts list
"what sets a bot's throttle at all" as open. The first half is right and stays right; the question
is now answered, and the answer is that no *task* arm writes it - the **planner** does.

An exhaustive `store_census` over the throttle slot's `desired` at `plan+278h`
(base `plan+274h`, stride `0Ch`, slot 0) returns four writes inside
`0099D300 BSP_PilotBot_PlanControls`, on the `ESI` that `0099D305 MOV ESI,ECX` makes the plan:

* `0099D399`, the centred-stick arm, writes `[00D7A24C]` = **1.0** and raises the slot's `active`
  byte at `0099D3A1` - which is the pair that proves the record is the plan;
* `0099D8CF`, the speed-hold arm, writes a demand that exceeded `plan+2B4h`, with the air brake
  beside it at `0099D8DD`;
* `0099DC31`, a floor through `00415550 BSP_Math_MaxFloatByRef`;
* `0099DC8F`, a **ceiling that pins the throttle to `[00CE3D30]` = 0.6**.

**So the native does cut a bot's throttle, and this doc looked for it in the right spirit and the
wrong place.** `docs/PILOT_BOT_THROTTLE_ARM.md`.

## Correction from packet `cc8_torpedo_descent_law`: the span is not always zero, and the interpolation is the descent scale

The negative result stands: **step 4 commands no throttle**, and `009D0A63`'s
`InterpolateClamped(0.1, 0.35, 0.4, 0.8, .)` is not a throttle value. What it is, and the claim that
it reaches nothing, are corrected here.

* **was**: "span is ALWAYS zero from this site: `009D07E4` writes `approach+90h` into the rangeLow
  local at the head and `009D0A08` writes `approach+90h` into the rangeHigh local just before the
  call ... So the interpolation runs every tick and reaches nothing", and therefore "a host that
  passes `rangeLow = rangeHigh = 0` there reproduces the native's commanded altitude exactly".
* **is**: `009D07E4 MOVSS [ESP+0xc],XMM0` and `009D0A2A MOVSS [ESP+0x20],XMM0` are the **same slot**.
  `009D0A23 SUB ESP,0x14` sits between them, so `[ESP+0x20]` after the subtract is `[ESP+0xc]`
  before it, and `009D0A2A` **overwrites** the head's `approach+90h` with the release distance
  `approach+7Ch` or `+80h` that `009D0A0E` selects on the 15-second switch - three instructions
  before the call. `009D0A79 FLD [ESP+0x1c]`, after `009D0A68 SUB ESP,0x10`, reads that slot, so
  `rangeLow` is the **release distance** and `rangeHigh` is `approach+90h`. The span is
  `max(range - releaseDist, 0)`, it is large for the whole run-in, and `009FBA50`'s bias term
  `span * scale * class+518h` with `class+518h = tan(DropAngle)` is the **glide slope** that brings
  the aircraft down to the release altitude at the release distance.
* **evidence**: the frame is `SUB ESP,0x3C` + `PUSH ESI`/`PUSH EDI`, with `PUSH EBX` at `009D07FE`
  balanced by `POP EBX` at `009D08C3` and every `SUB ESP` in between belonging to a callee that
  cleans its own arguments (`00419010 RET 14h`, `00438AA0 RET 8`), so `ESP` at `009D07E4`,
  `009D0A08` and `009D0A23` is the same value. Decisively, a run with the four real arguments prints
  `low=650.0` at 4183 m range and `low=450.0` after the 15-second switch, with `span=3533.4` and a
  commanded altitude of 761.9 m - not the 12.0 m a zero span gives.
* **consequence**: the recommendation "a host that passes `rangeLow = rangeHigh = 0` there
  reproduces the native's commanded altitude exactly" is **struck**. That substitution is what made
  `src/game_hosts_units.cpp` command a flat 12 m from 4 km out, and with the saturated pitch
  reference it put all five of USN01's torpedo bombers in the sea eleven seconds into the mission.
  `docs/TORPEDO_DESCENT_LAW.md`.

The fourth argument's name is also corrected: it is neither a throttle nor a discarded return value.
`009FBB06 FSTP [ESP+4]` hands it to `009FB800` as the clamp on `t`, which is what decides how much
of the `DEG(60)` dive cap the aircraft may ask for.
