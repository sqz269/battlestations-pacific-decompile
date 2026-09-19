# The torpedo release gate: the aspect scale, and the own-squadron exemption (packet `cc8_torpedo_release`)

Successor to `docs/TORPEDO_AIM_LEAD.md` (packet `cc8_torpedo_lead`). That packet settled that the
image's torpedo attack does **not** lead its target anywhere in the steering chain, and that the
image's one answer to a moving target is **when** it releases. This packet binds that gate, and the
own-formation exemption the entity sweep is missing.

Nothing in section 1 or 2 below is re-derived from the predecessor: every address was re-read from
the listing in this packet, and two of the predecessor's statements are corrected.

## 1. The aspect gate, re-derived and corrected

### 1.1 The gate is computed once and used TWICE

The predecessor, and the host comment at `src/game_hosts_units.cpp`, both said `approach+84h` shapes
only the aim-solution byte and "never the gate". That is **wrong**, and it is the reason this packet
matters. The image computes the aspect interpolation once and caches the product:

| address | instruction | meaning |
| --- | --- | --- |
| `009D1F78`-`009D1F99` | `FLD [ECX+134h]`, `FLD double [00CF3F20]`, `FCOMIP`, `JBE` | `elapsed_134 < 15.0` takes `+80h`, else `+7Ch`; result to `[ESP+28h]` |
| `009D1F9F`-`009D1FBF` | `FLD [ESP+34h]`, `FCOS`, `AND EAX,7FFFFFFF` | `|cos(aspect)|` |
| `009D1FE4` / `009D1FE0` / `009D1FDC` / `009D1FD0` / `009D1FC8` | the five stack slots | `InterpolateClamped(x0 = 0.5, y0 = 1.0, x1 = 1.0, y1 = approach+84h, x = |cos|)` |
| `009D1FED` | `CALL 00419010` | `BSP_Math_InterpolateClamped`, `RET 14h` |
| `009D1FF2` / `009D1FF6` | `FMUL [ESP+28h]` / `FSTP [ESP+28h]` | **the product is written back into the same slot** |

and that one cached product is then read by **two** comparisons:

| consumer | address | comparison | result |
| --- | --- | --- | --- |
| aim-solution byte | `009D1FFE` -> `009D2002` `FADD double [00CE4D70]` -> `009D2008` `FCOMIP` | `scaled + 200.0 > range` | `009D2021` `MOV [EDI+130h],AL` |
| **release chain** | `009D2034` -> `009D203D` `FCOMIP` | `scaled > range`, **no slack** | `009D2058`/`009D2061` set `[ESP+10h]`, the first release flag |

The slot is not clobbered between the two reads: a whole-function census of `[ESP+0x28]` in
`BSP_BotStateTorpedoAim_Tick` (`009D15F0`) shows writes only at `009D1F99` and `009D1FF6` in this
region. The frame is stable across the intervening `009D2018 PUSH ECX` / `009D2027 CALL 009FA3A0`
because `BSP_ReleaseTimer_Tick` ends `RET 0x4` at `009FA40A`, so `[ESP+28h]` at `009D2034` addresses
the same slot as at `009D1FF6`.

**So `approach+84h` gates the release itself.** The host reaches the same numbers by recomputing the
interpolation at both sites (`src/torpedo_aim_tick.cpp:303` and `:311`) instead of caching it; that
is numerically identical and the host's structure is not at fault. Only the comment was.

Constants, read at the width of the loading instruction (`tools/pe_const_read.py`):

* `00CE3800` = **0.5**, a float (`FLD dword`) - the knee.
* `00CE4D70` = **200.0**, a **double** (`FADD double ptr`). Read as a float it is `0.0`.
* `00CF3F20` = **15.0**, a double - the elapsed-time switch between the two release distances.
* `00CF1440` = **80.0**, a double - the `fall_lead_a0` slack on the second half of the release flag.

### 1.2 The seed, verified from the listing

`009D049D` `FLD float ptr [EAX + 0xc]` / `009D04A0` `FSTP float ptr [ESI + 0x84]`, in
`BSP_BotApproachTorpedo_Reset` (`009D0380`). Verbatim: a bare `FLD`/`FSTP` with **no scale** - unlike
the neighbouring `+7Ch`/`+80h`, which `009D0491`/`009D0497` do multiply by the speed ratio.

`EAX` provenance, by filtering the whole function listing for the register (not one idiom):
`009D046C` `MOV EAX,dword ptr [ESI + 0x14]` is the only `EAX` write between the function head and the
three reads at `009D0484`/`009D0494`/`009D049D`. So `approach+84h = (approach+14h)->+0Ch`.

**Correction to the predecessor.** It recorded `approach+14h` as `00F8A30C + index*248h + 0Ch`.
`00F8A30C` is a **pointer global**, not the table base:

    009f9d08: MOV EDX,dword ptr [EAX + 0xdf4]      ; the unit's pilot block
    009f9d0e: MOV EDX,dword ptr [EDX + 0x34]       ; the difficulty index
    009f9d11: IMUL EDX,EDX,0x248                   ; the row stride
    009f9d18: MOV ESI,dword ptr [0x00f8a30c]       ; the table base, LOADED FROM the global
    009f9d1e: LEA EDX,[EDX + ESI*0x1 + 0xc]        ; row + 0Ch
    009f9d22: MOV dword ptr [ECX + 0x14],EDX       ; -> approach+14h

The rows are built at runtime from the Lua table and **cannot be read out of the PE** at that
address. The offsets are unaffected: `approach+14h` points at row+`0Ch`, so its `+0Ch` is row+**18h**,
which `include/bsp/robot_config.hpp:82` names `torp_release_drop_closer_mul_018` (reader `009973B0`).

### 1.3 The authored value, and a correction to the row enumeration

This installation's `scripts/datatables/robots.lua` (mtime **2025-06-01 16:03:10** - a locally
modified file; this is this installation's value, not a claim about retail). The PilotBot section has
**six** difficulty rows, not the four the predecessor's partial enumeration implied:

| row | line | `TorpReleaseAlt` | `DistNear` | `DistFar` | `DropCloserMul` |
| --- | --- | --- | --- | --- | --- |
| **`SPNormal`** | 554 | 12 | 450 | 650 | **0.7** |
| `SPVeteran` | 693 | 5 | 800 | 1200 | 0.5 |
| `MPNormal` | 831 | 10 | 800 | 1200 | 0.7 |
| `MPVeteran` | 969 | 10 | 800 | 1200 | 0.6 |
| `Elite` | 1107 | 5 | 800 | 1200 | 0.5 |
| `Stun` | 1245 | 12 | 350 | 600 | 0.7 |

Two things follow. **No row authors 1.0**, so the host's forced `1.0` is outside the authored range
entirely, not merely off by one row's worth. And the whole authored band is `0.5`-`0.7`, so naming
`SPNormal` costs at most `0.2` against any other row.

The Lua authoring order at lines 557/558/559/560 - `TorpReleaseAlt`, `TorpReleaseDistNear`,
`TorpReleaseDistFar`, `TorpReleaseDropCloserMul` - matches the row offsets `0Ch`/`10h`/`14h`/`18h`
exactly. That is an independent confirmation of the offset assignment, from the data rather than from
the code.

The row's own Hungarian comment: *the torpedo release distance applies to the case where we catch the
target on the beam; if we want to torpedo it from the front or behind, the release distance is
reduced to this multiple.* `009D1739`/`009D1746` make the slot `|wrapped(target_heading -
own_heading)|`, so `|cos|` is **1 along the target's course** and **0 on the beam** - the same
convention the comment states, derived independently.

### 1.4 Which row this host gets

The difficulty index at `[[unit+DF4h]+34h]` (confirmed at `009F9D08`/`009F9D0E`) is unmodelled in this
host, so `SPNormal` is **named rather than chosen**. This follows the pattern the same host already
uses for the other three fields of the same row (`src/game_hosts_units.cpp:7147`-`7198`,
`kTorpReleaseAltSPNormal` / `kTorpReleaseDistNearSPNormal` / `kTorpReleaseDistFarSPNormal` in
`include/bsp/torpedo_release_spawn.hpp`). This packet binds the fourth field of that same row, by the
same substitution, and says so in the same words.

### 1.5 What the gate becomes

With `y1 = 0.7`, the interpolation is

    factor(c) = 1.0                        for c <= 0.5      (beam, the full release distance)
              = 1.0 - 0.6 * (c - 0.5)      for 0.5 < c < 1.0
              = 0.7                        for c >= 1.0      (dead ahead or astern)

and the release distance is `450` late / `650` early (the `15.0 s` switch at `00CF3F20`), times
`ratio_24h`, which is `1.0` for this installation's torpedo bombers.

## 2. PREDICTION, recorded before any run of this packet

Written before the before/after pair was launched. Every earlier absolute number in the predecessor's
document is stale (main changed ship motion and the dive-bomb goaway on 2026-09-19), so both arms are
my own, on this tree, same binary except for the change under test.

The census column `release_range` (and `abs_cos_aspect` beside it) is **new in this packet** and is
present in BOTH arms: the drop line prints the drop ALTITUDE, never a range, so the gate could not be
measured at all before. It is instrumentation, no behaviour.

### 2.1 The structural prediction, which the binding could fail

`factor` is **clamped to 1.0 for every `|cos| <= 0.5`**. So:

* **All five USN01 rounds are unchanged.** Their `|cos|` are 0.139, 0.238, 0.292, 0.477 and 0.520:
  four are below the knee and clamp exactly, and the fifth gives `factor = 0.988`, a 1.2 per cent
  change (about 8 m on a 650 m threshold) - at or below the resolution of the measurement.
* **USN04 trace 9 is unchanged**, at crossing 110.8 deg -> `|cos| = 0.355`, below the knee.
* **USN04 traces 3, 4, 5, 6, 10, 11, 12 tighten by 27.5 to 29.4 per cent**:

| trace | crossing | `|cos|` | `factor` | threshold change |
| --- | --- | --- | --- | --- |
| 3 | 172.0 deg | 0.990 | 0.706 | -29.4 % |
| 4, 10 | 163.7 deg | 0.960 | 0.724 | -27.6 % |
| 5, 11 | 163.2 deg | 0.958 | 0.725 | -27.5 % |
| 6, 12 | 163.9 deg | 0.961 | 0.723 | -27.7 % |
| 9 | 110.8 deg | 0.355 | **1.000** | **0 %** |

**Any movement at all on trace 9 or on the four clamped USN01 rounds falsifies this binding.** That
is the sharp half of the prediction, and it is a within-mission control: in the same USN04 run, seven
rounds must move and one must not.

### 2.2 The magnitude prediction, which is conditional and may well be wrong

The gate threshold falls by `(1 - factor) * dist`: about **180 m** if the early distance (650) is in
play, about **124 m** if the late one (450) is.

But the threshold is not necessarily what binds. The predecessor's USN01 release ranges, *back-solved*
from `t_cpa` rather than measured, were 327-355 m - far below even the un-scaled 650 m threshold.
If that is right, the range gate was already wide open at the drop and something else set the moment
of release: the randomised delay inside `BSP_ReleaseTimer_Tick` (`009FA3A0` calls
`BSP_Random_UniformFloatRange` and `BSP_Unit_CanReleaseOrdnance`), or one of the other four release
flags, most likely the altitude gate at `009D20B4`/`009D20C4`.

So the honest prediction has two branches, and the new `release_range` column decides between them:

* If the measured before-arm `release_range` on the along-course rounds is **above** the new
  threshold (~470 m early / ~326 m late), the gate binds and the after-arm release ranges should fall
  to roughly that threshold.
* If it is already **below**, the gate does not bind, the release ranges will **not move**, and
  binding 0.7 will be correct but inert in these two missions. That is a real possible outcome and it
  would be a finding, not a failure - it would say the aspect gate is masked in this host by whatever
  currently sets the release moment.

A third possibility worth watching: a tighter gate makes the aircraft fly ~180 m further in before it
may drop. If the break-off or goaway logic fires first, **drops can go DOWN and refusals up**. The
predecessor's USN04 run had `drops=12 refusals=0`; I will read those two counters first.

### 2.3 What a closer release should do to the hits

Zero-lead miss scales with the drop range (`miss = R * v_t * sin(theta) / |v_rel|`, the predecessor's
section 6). A shorter `R` therefore shrinks the miss on **moving** targets. Traces 4, 5, 6 are against
a **stopped** Lexington (`target_moved = 0.0`), so their recorded closest approach is the hull-strike
offset from the target centre and should be essentially unchanged whatever the release range. Trace 3
is against a moving Yorktown and already hits. So on the recorded USN04 set there is **little room for
the miss numbers to improve**, and the visible effect should be in the release ranges and in whatever
the escort screen does (item 3), not in a hit/miss flip.

### 2.4 MEASURED: the USN01 before arm, and a correction to the predecessor's range

`local/rel_usn01_before.log`, launched 13:43, finished with `native renderer final COM release`.
`torpedo_drop drops=5 refusals=0 water_entry_breakups=0`, `torpedo_closest_approach swims=5`.
Binary: the 13:40:13 build, census column only, `aspect_scale_84` still forced to `1.0`.

| round | ordered | `release_range` | `abs_cos_aspect` | ordered min | `target_moved` |
| --- | --- | --- | --- | --- | --- |
| Mav3 | Northampton | 436.9 m | 0.478 | 8.5 m | 0.0 m |
| Mav2 | Northampton | 440.8 m | 0.520 | 9.1 m | 0.0 m |
| Mav1 | Dunlap | 434.6 m | 0.139 | 210.3 m | 141.3 m |
| Mav5 | SaltLakeCity | 437.8 m | 0.239 | 182.6 m | 132.9 m |
| Mav4 | SaltLakeCity | 433.1 m | 0.293 | 175.2 m | 129.5 m |

The five `|cos|` reproduce the predecessor's five values exactly, so this is the right baseline.

**Correction to `docs/TORPEDO_AIM_LEAD.md` section 6.** That table *back-solved* the release range
from each round's `t_cpa` and got 327.0 / 333.7 / 338.5 / 351.9 / 355.0 m. Measured directly it is
**433-441 m on all five** - about 100 m high on every round, a systematic error rather than scatter.
That section is explicitly a consistency check with one fitted parameter, and the fitted parameter is
now shown to be wrong; its zero-lead **conclusion** is untouched, because that rests on section 7's
crossing-angle split, which needs no `R` at all.

**And the correction does not simply slot back into that model.** Feeding the measured `R` into the
same equations with the same `s = 30.87 m/s` over-predicts the time to closest approach: for
Mav1 -> Dunlap, `t_cpa = R (s - v_t cos θ) / |v_rel|²` with `R = 434.6`, `v_t = 19.09`, `θ = 98.0°`
gives **9.84 s** against the recorded **7.40 s**. So one of the model's other terms is wrong too -
the obvious candidate is that the round is *dropped* at 435 m but *enters the water* closer, after an
air fall the equation does not model, so the swim starts at a shorter range than the release range
this column measures. I am recording the inconsistency and **not** fitting it: the release range is
what this packet needed and what it measured, and the miss decomposition is the predecessor's, whose
residual it already left explicitly unexplained. Anyone reusing section 6's numbers should know that
its `R` is measured now, and that the rest of that model does not survive the substitution unchanged.

**This sharpens the magnitude prediction of section 2.2.** 435 m sits just under the `SPNormal`
`TorpReleaseDistNear` of 450 m. Since `009D1F78`-`009D1F99` selects the 450 m slot whenever
`elapsed_134 >= 15.0 s`, a release at ~435 m looks like the **late branch with the gate actually
binding** - the round leaves the rack as soon as the range gate opens, give or take the tick and the
randomised delay in `009FA3A0` - and not like a gate standing wide open at 650 m.

So, recorded before the after arm runs:

* Mav1, Mav3, Mav4, Mav5 (`|cos|` 0.139, 0.478, 0.293, 0.239) clamp to `factor = 1.0` and must
  **not move at all**.
* Mav2 alone (`|cos| = 0.520`) gets `factor = 0.988`, threshold `450 -> 444.6 m`, so it should
  release about **5 m closer**. That is small, but it is the only round permitted to move, and the
  direction is fixed.
* If nothing moves anywhere, the range gate is not what sets the release moment in this host, and
  section 2.2's second branch is the answer.

### 2.5 MEASURED: the USN04 before arm, and what it settles about the gate

`local/rel_usn04_before.log`, launched 13:47, finished with `native renderer final COM release`.
`torpedo_drop drops=12 refusals=0 water_entry_breakups=0`, `torpedo_closest_approach swims=8`. Same
13:40:13 binary.

| trace | ordered | `release_range` | `abs_cos` | ordered min | at | `target_moved` | crossing |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 3 | Yorktown-class01 | 436.7 m | 0.990 | 23.7 m | 7.30 s | 120.8 m | 172.0 deg |
| 4 | Lexington-class01 | 436.3 m | 0.960 | 55.4 m | 9.75 s | 0.0 m | 163.7 deg |
| 5 | Lexington-class01 | 435.7 m | 0.957 | 50.1 m | 9.90 s | 0.0 m | 163.2 deg |
| 6 | Lexington-class01 | 438.0 m | 0.961 | 49.5 m | 10.00 s | 0.0 m | 163.9 deg |
| 10 | Lexington-class01 | 436.3 m | 0.960 | 202.0 m | 5.00 s | 0.0 m | 163.7 deg |
| 11 | Lexington-class01 | 435.7 m | 0.957 | 192.1 m | 5.30 s | 0.0 m | 163.2 deg |
| 12 | Lexington-class01 | 438.0 m | 0.961 | 194.5 m | 5.30 s | 0.0 m | 163.9 deg |
| 9 | Yorktown-class01 | 435.1 m | 0.355 | 167.9 m | 7.75 s | 128.3 m | 110.8 deg |

Every `ordered min` and every crossing angle reproduces the predecessor's USN04 table to the digit,
so the harness is deterministic on these columns and this is the right baseline.

**The release range is flat at 433-438 m across BOTH missions and every aspect**, from `|cos| = 0.139`
to `|cos| = 0.990`. That is exactly the signature of a gate whose factor is identically 1.0: thirteen
rounds, five different ships, two missions, aspect ranging from nearly beam-on to nearly bow-on, and
the release distance does not vary by more than 8 m. It is also the clearest possible statement of
what the substitution was costing - the image's one compensation for not leading was flat.

Combined with the 450 m `TorpReleaseDistNear`, this settles the branch question of section 2.2: the
gate is on the **late** 450 m slot and it **binds**, with the round leaving the rack about 15 m
inside the threshold. So the after arm has a definite target:

| rounds | `factor` | threshold before | threshold after | predicted `release_range` |
| --- | --- | --- | --- | --- |
| USN04 3 | 0.706 | 450 m | 317.6 m | ~318 m |
| USN04 4, 5, 6, 10, 11, 12 | 0.723-0.725 | 450 m | 325.4-326.3 m | ~325 m |
| **USN04 9** | **1.000** | 450 m | **450 m** | **~435 m, unchanged** |
| USN01 Mav1, Mav3, Mav4, Mav5 | 1.000 | 450 m | 450 m | unchanged |
| USN01 Mav2 | 0.988 | 450 m | 444.6 m | ~5 m closer |

**One refinement, recorded before the result.** The before arm releases 12-15 m *inside* the 450 m
threshold, not at it, because `009D2287` only **arms** a timer: `BSP_ReleaseTimer_Tick` (`009FA3A0`)
counts it down - with a `BSP_Random_UniformFloatRange` term - and the aircraft keeps closing while it
does. At the 0.05 s mission tick that gap is about four ticks. So the after arm should land the same
12-15 m inside its new threshold: about **304 m** for trace 3 and **312-313 m** for the six at
`|cos| ~ 0.96`, rather than exactly at 318/326. I am predicting the threshold AND the offset, so a
result at the bare threshold would itself be a small surprise worth explaining.

## 2.6 FALSIFIED: the prediction was wrong, and the reason is that the gate is blind

`local/rel_usn01_after.log` and `local/rel_usn04_after.log`, 13:56:22 binary (`aspect_scale_84 =
0.7`), same parameters as their befores, both finished clean, `refusals=0` in both.

I predicted that every round with `|cos| <= 0.5` **could not move**, because the interpolation clamps
its output to `[y1, y0]`. **All of them moved.**

| USN01 round | `R` before | `R` after | `|cos|` | predicted | outcome |
| --- | --- | --- | --- | --- | --- |
| Mav3 | 436.9 m | 305.4 m | 0.478 | unchanged | **wrong**, -131.5 m |
| Mav2 | 440.8 m | 301.5 m | 0.520 | -5 m | **wrong**, -139.3 m |
| Mav1 | 434.6 m | 300.7 m | 0.139 | unchanged | **wrong**, -133.9 m |
| Mav5 | 437.8 m | 299.6 m | 0.239 | unchanged | **wrong**, -138.2 m |
| Mav4 | 433.1 m | 300.9 m | 0.293 | unchanged | **wrong**, -132.2 m |

USN04 did the same thing: all seven swimmers released at 301.1-305.9 m, **including** the round at
`|cos| = 0.300`, which was my designated control and was supposed to be immune.

### 2.6.1 The cause, from the source rather than from a story

`src/torpedo_aim_tick.cpp:122` seeds the aspect slot `float f2c = 0.0f;` and assigns it only at
`:134`, inside

    if (host.has_target_cc() && host.target_is_kind_vtable5c(kTargetKindProbe))

and in this host all three of those were stubs, at `src/game_hosts_units.cpp:7616`-`7619`:

    float target_heading_vtable50() override { return 0.0f; }
    bool  target_is_kind_vtable5c(int) override { return false; }
    bool  has_target_cc() override { return false; }

with a comment saying exactly that: `approach+CCh`'s target entity is not modelled, so the slot keeps
the `009D16AA` zero - "the no-target arm of the native branch". **So `f2c` is identically 0,
`|cos(0)| = 1.0`, and `009D1FED` returns its `y1` - `approach+84h` - on every tick, whatever the real
aspect is.**

That reproduces the measurement to better than a metre. Threshold `= y1 * 450`, and the release sits
the same 13 m inside it that the before arm showed:

| `y1` | threshold | predicted release | measured (USN01 mean of 5) |
| --- | --- | --- | --- |
| 1.0 (before) | 450.0 m | 437 m | 436.6 m |
| 0.7 (after) | 315.0 m | 302 m | **301.6 m** |

**My error was assuming the census's `|cos|` was the gate's input.** The `abs_cos_aspect` column is
the *gunnery* host's own computation from real unit headings; the aim tick never sees it. I checked
the consumer and not the producer - `WORKER_VERIFICATION_CHECKLIST.md` rule 4 - for the second time
in this packet.

### 2.6.2 What the flat gate did to the outcomes, which is the argument for fixing it

USN04, before against after, same binary except `approach+84h`:

* Traces 4, 5, 6 **stopped reaching the Lexington**. They were hits on `Lexington-class01` at
  `life` 9.75/9.90/10.00 s; they are now hits on **`Fletcher-class04`** at 3.50/3.55/3.70 s. Releasing
  135 m closer put the drop point inside the escort screen.
* Traces 10, 11, 12 still strike `Fletcher-class02`, now at `life` 1.55-1.75 s instead of 5.00-5.30 s.
* Trace 3 still hits `Yorktown-class01`, marginally better (18.4 m against 23.7 m).
* `swims` fell 8 -> 7; the four own-formation kills are unchanged (section 3).

So the flat 315 m arm is **worse than the flat 450 m arm**, and this answers item 3 for these two
arms: with the gate flat and closed down, the escort screen intercepts *more*, not less. Neither
arm is faithful, because the image's factor for USN01's beam-ish rounds is ~1.0 (450 m) and for
USN04's along-course rounds is ~0.72 (~324 m), and no single flat number is both.

**The gate's inertness was never the constant. It was `approach+CCh`.**

## 2.7 Binding the aspect, and the prediction that separates the two missions

`src/game_hosts_units.cpp`, the torpedo task's `AimTickBinding`: `has_target_cc` becomes "there is an
ordered target", `target_heading_vtable50` returns that target's `pose_heading_radians` - which is
exactly what `GameUnitsHost::unit_heading_radians` calls, and the same quantity the drop census takes
for *both* headings, so the gate's aspect is now commensurable with the `crossing` column (that check
is recorded at `src/game_hosts_gunnery.cpp:3270`-`3275`).

`target_is_kind_vtable5c` is a **labelled substitution**: `009D1718` probes the target with
`vtable[5Ch](0Fh)` and the entity kind table behind slot `5Ch` is not reachable from this host, so
every ordered target is taken to pass. The torpedo task's targets are ships, which is the case the
probe exists to admit; a kind the image would *reject* would be given an aspect here where the image
keeps the zero.

**PREDICTION, recorded before the run.** One binary must now send the two missions in *opposite*
directions, which no flat value can do:

* **USN01 goes back UP to ~435 m.** Four of its five rounds are below the 0.5 knee, so the factor
  returns to 1.0 and the threshold to 450. Mav2 (`|cos| 0.520`) should land ~5 m short of the others.
* **USN04 stays DOWN at ~305-315 m** for the six along-course rounds (`|cos|` 0.957-0.990, factor
  0.706-0.725, threshold 318-326).
* **USN04's `|cos| ~ 0.30` round goes back UP to ~435 m.** It is the within-mission control again,
  and this time it is a real one.
* Traces 4, 5, 6 should return to the **Lexington** rather than `Fletcher-class04`, because their
  release range returns to roughly where it was.

One caveat stated in advance: the gate is evaluated **every tick**, on the aspect at that tick, while
the census prints the aspect at the drop. Where an aircraft turns during the run-in the two differ,
so these are approximations, not identities - and the *direction* is what the prediction rests on.

### 2.7.1 MEASURED: USN01 returns, and only the above-knee round moves

`local/rel_usn01_aspect.log`, `drops=5 refusals=0`, clean.

| round | before | flat-0.7 | aspect-bound | `|cos|` | predicted | result |
| --- | --- | --- | --- | --- | --- | --- |
| Mav3 | 436.9 m | 305.4 m | **436.9 m** | 0.478 | unchanged | identical |
| Mav1 | 434.6 m | 300.7 m | **434.6 m** | 0.139 | unchanged | identical |
| Mav5 | 437.8 m | 299.6 m | **437.8 m** | 0.239 | unchanged | identical |
| Mav4 | 433.1 m | 300.9 m | **433.1 m** | 0.293 | unchanged | identical |
| Mav2 | 440.8 m | 301.5 m | **433.0 m** | 0.520 | ~5 m closer | **-7.8 m** |

Four of five are identical to the digit, and the one round above the 0.5 knee is the one that moved,
in the predicted direction. The closest-approach column is unchanged on those four (8.5 / 210.3 /
182.6 / 175.2 m) and improved on Mav2 (9.1 -> 7.7 m).

Mav2's predicted shift was 5.4 m and the measured one is 7.8 m. The excess is expected in sign and
size rather than explained away: the gate is evaluated **every tick**, so the operative `|cos|` is
whatever it is when the threshold is crossed, and the census samples 0.520 at the drop. A gate
running slightly above 0.520 gives slightly more than 5.4 m. This is the caveat above, doing what it
said it would.

### 2.7.3 MEASURED: USN04 falls, the control does not, in the SAME binary

`local/rel_usn04_aspect.log`, `drops=12 refusals=0 water_entry_breakups=0`, clean.

| round | ordered | before | flat-0.7 | aspect-bound | `|cos|` | predicted |
| --- | --- | --- | --- | --- | --- | --- |
| `#4.1\|.-2` | Yorktown | 436.7 m | 303.0 m | **303.0 m** | 0.990 | 304.6 m |
| `#2.1` | Lexington | 436.3 m | 301.7 m | **317.3 m** | 0.960 | 312.8 m |
| `#2.1\|.-2` | Lexington | 435.7 m | 301.1 m | **316.6 m** | 0.957 | 312.8 m |
| `#2.1\|.-3` | Lexington | 438.0 m | 303.4 m | **311.2 m** | 0.961 | 312.8 m |
| **`#8.1\|.-2`** | Yorktown | 435.1 m | 305.9 m | **435.1 m** | 0.355 | **unchanged** |

**The control holds exactly.** `#8.1|.-2` is back at 435.1 m - the before value to the digit - and its
closest approach is back at 167.9 m, also the before value. In the flat arm it had moved to 305.9 m,
which is what proved the gate blind. The along-course rounds sit where `threshold - 13 m` puts them:
`0.706 * 450 - 13 = 304.6` against 303.0 measured, and `~0.724 * 450 - 13 = 312.8` against
311.2-317.3.

**One binary sends the two missions in opposite directions**, which is the whole test: USN01 stays at
433-438 m and USN04 falls to 303-317 m, while USN04's own below-knee round stays at 435. No flat
value can produce that, and the flat arm demonstrably could not.

### 2.7.4 `swims` falls 8 -> 5, and it is not a loss or a truncation

All twelve rounds have an `exit=` line, so nothing was dropped late, lost, or cut off by frame 4800:

* four die on a squadron mate at `life = 0.05 s` (traces 1, 2, 7, 8) - section 3.3, unchanged;
* trace 3 hits `Yorktown-class01` at 5.35 s;
* traces 4, 5, 6 hit `Fletcher-class04` at 3.95-4.10 s;
* traces 10, 11, 12 hit `Fletcher-class02` at 1.45-1.60 s;
* trace 9 is the surviving Yorktown round at 435.1 m.

The counter fell because `swims` counts rounds that reached the **swim** state, and traces 10, 11, 12
now strike the escort **above the waterline, during the air fall**: their impact heights are
`y = 2.00`, `0.08` and `2.00`, at `life` 1.45-1.60 s. In the before arm the same three struck
`Fletcher-class02` at `y = 0.00` after 5.00-5.30 s of swimming. Releasing closer means the round is
still falling when it meets the destroyer's hull, so it never enters the swim and never counts.
`drops` is 12 in every arm and `refusals` is 0 in every arm.

### 2.7.5 Item 3: the escort screen, answered

Traces 4, 5, 6 were ordered onto `Lexington-class01` and still strike `Fletcher-class04`; 10, 11, 12
still strike `Fletcher-class02`. I predicted in 2.7 that they would return to the Lexington. **They
do not, and that is the correct behaviour rather than a miss.** Their `|cos|` is 0.957-0.961 - they
attack along the target's course, which is exactly the case the authored comment says must close to
`0.7` of the release distance - so the faithful gate tightens them to ~313 m, and at that range the
screening destroyer is between the bomber and the carrier. The interception is a destroyer doing its
job, not a defect: the gate is not there to get the round past the screen, it is there to stop a
zero-lead attack taking a long shot at a crossing target.

What the before arm showed was the opposite error - a flat 450 m release let three rounds reach the
carrier *because the gate was not working*, not because the attack was good.

### 2.7.6 The kind probe, and which form is committed

`009D1710` `PUSH 0x6` / `009D1714` `CALL EAX`, so the probe argument is **6**, and kind 6 is the ship
base throughout this project (`kKindKamikazeTargetShip` `attack_commands.hpp:71`,
`kKillCreditKindShipBase` `kill_credit.hpp:30`, `kUnitGunneryKindShipBase` `game_hosts_ai.cpp:792`).

It need not be a substitution at all: `src/game_hosts_units.cpp:2180` records that
`bsp::unit_is_kind_of` **is the same `0074E400` model that slot `5Ch` uses**, so the exact form is

    bool target_is_kind_vtable5c(int query) override {
        const GameUnitSlot* const t = aim_target();
        return t != nullptr && bsp::unit_is_kind_of(t->class_id, query);
    }

**That is not what is committed, and the reason is the rule about measuring what you ship.** Every
log this change rests on was produced with the predicate written `aim_target() != nullptr`. For a
ship target the two are identical by construction - and every ordered target in USN01 and USN04 is a
ship - but "identical by construction" is an expectation, not a measurement. Committing the stronger
form would ship code no run had exercised, so the weaker form is committed, labelled as a
substitution in the source, and the bound form is the **first** item of
`docs/HANDOFF_TORPEDO_RELEASE.md`, needing only one confirming pair.

What the committed form costs meanwhile: a **non-ship** ordered target is given an aspect here where
the image would keep the `009D16AA` zero. No round in either mission is affected.

## 3. Item 2: the round that kills its own formation

In the predecessor's USN04 run, traces 2, 7 and 8 never swam: each `entity_impact` a **friendly B5N
Kate** at `life = 0.05 s`, one tick after the drop, at y = 11.66 / 11.76 / 11.78. The round strikes an
aircraft of the dropping formation. `docs/TORPEDO_AFTER_THE_DROP.md` section 15 met the same defect
when formation members were co-located; spacing reduced it, but three of twelve still die this way.

### 3.1 The image does NOT prevent it - my first reading was wrong

**Retracted.** I first read step 6 of `0084BF00` as an own-squadron exemption that the torpedo host
was missing, and reported that to the integrator. It is not. The decisive operand is the SHOT's
sub-type, not the hit entity's class, and it gates the whole arm on **Kamikaze**.

`docs/TORPEDO_AFTER_THE_DROP.md` section 15.3 had already established this ("The image gives a
dropped torpedo no friendly-fire exemption, so this is not the fix") and it is correct. I reached the
listing before I read the doc that covered it, although `bsp.py docs-for 0084C203` named that
document in my first query on the address. The rule is grep the docs first; this is what it costs.

Step 6, the entity sweep whose call at `0084C314` pushes literal 1 for entities, `0084C1E6`-`0084C211`:

    0084c1e6: TEST BL,BL                     ; BL = the sweep said "entity hit"
    0084c1e8: JZ 0084c251
    0084c1ea: TEST ECX,ECX                   ; ECX = the hit entity
    0084c1ec: JZ 0084c251
    0084c1ee: MOV EDX,dword ptr [ECX]        ; its vtable
    0084c1f0: MOV EAX,dword ptr [EDX + 0x5c]
    0084c1f3: PUSH 0xf
    0084c1f5: CALL EAX                       ; hit->vtable[5Ch](0xF), a kind predicate
    0084c1f7: TEST AL,AL
    0084c1f9: MOV ECX,dword ptr [ESP + 0x78] ; reload the hit entity
    0084c1fd: JZ 0084c213                    ; predicate false -> no exemption
    0084c1ff: MOV EDX,dword ptr [ESP + 0x64] ; its class descriptor
    0084c203: CMP dword ptr [EDX + 0x8],0x11 ; classDesc+8h == 11h ?
    0084c207: JNZ 0084c213
    0084c209: CMP dword ptr [ECX + 0x9d4],ESI ; hit->+9D4h == the shooter's squadron ?
    0084c20f: JNZ 0084c213
    0084c211: XOR BL,BL                      ; EXEMPT: clear the entity-hit flag

`ESI` at `0084C209` is **not** the value it held at the function head - it is reassigned at
`0084C1D8`. Traced backward through the frame:

    0084bf34: MOV dword ptr [ESP + 0x44],EDI  ; pre-zeroed (EDI = 0)
    0084bf3f: MOV ESI,dword ptr [EAX + 0xcc]  ; the shooter-side object
    0084bf60: MOV EAX,dword ptr [ESI + 0x9d4] ; the SHOOTER'S squadron
    0084bf6d: MOV dword ptr [ESP + 0x44],EAX  ; cached in the frame
    ...
    0084c1d8: MOV ESI,dword ptr [ESP + 0x44]  ; reloaded
    0084c1dc: TEST ESI,ESI                    ; null guard: no squadron, no exemption

`include/bsp/air_operations.hpp:341` already names `+9D4h` `kPlaneSquadronBackPointerOffset`, and
that half of the reading holds: `ESI` really is the shooter's squadron, and `0084C209` really is a
squadron match.

**What kills it is `0084C1FF`/`0084C203`.** `EDX` is loaded from `[ESP+64h]`, and the only writer of
that slot in the entire function is `0084BFFC` `MOV dword ptr [ESP+0x64],EDI`, with `EDI` loaded at
`0084BFD8` from the incoming argument `[ESP+0xF4]` and used elsewhere in the function as a weapon
class row (`CMP byte ptr [EDI+0x74],0` at `0084C133`, `FLD float ptr [EDI+0xB0]`/`[EDI+0xAC]` at
`0084C184`/`0084C196`). So `[EDX+8h]` is the **round's** class descriptor, not the hit entity's, and
`include/bsp/projectile_kinds.hpp:30` names `+8h` `kWeaponClassOffSubType`:

* `kProjectileSubTypeKamikaze = 0x11` (`:137`) - the value the arm tests for;
* `kProjectileSubTypeTorpedo = 0x0A` (`:130`) - what a torpedo is.

So the true rule is: **a Kamikaze round does not hit a plane of its own squadron.** The arm cannot
fire for a torpedo at all. Binding it into this host would be neither faithful nor able to move the
measure.

The sweep's only real exclusion is the shooter itself. `0084C1B3`-`0084C1BF` takes
`EDI = shooter->vtable[0B0h]()` - the shooter's collision root, per `docs/HIT_NARROWPHASE.md` - and
`0084C1CF` pushes it as the sweep's single exclusion argument: one entity and its children, never a
formation.

**Therefore a torpedo that passes through a squadron mate hits it in the image too.** The mechanism
of traces 2, 7 and 8 is faithful. What is left open is whether the GEOMETRY that puts a wingman in
front of the muzzle at the moment of release is faithful, and
`docs/TORPEDO_AFTER_THE_DROP.md` section 15.4 already places that cause outside the torpedo chain,
in the missing formation offset.

### 3.2 The host matches the image here, so there is nothing to bind

`src/projectile_impact.cpp:205`-`208` does reconstruct step 6 for the projectile path, behind
`entity_hit_is_friendly_exempt`. The torpedo swim does not go through that path: it runs its own
sweep at `src/game_hosts_gunnery.cpp:2403`-`2410`, excluding only

    args.exclude_entity = reinterpret_cast<const void*>(shot.owner_unit);

one aircraft. Under the corrected reading that is **the same thing the image does** for a torpedo -
the image's own exclusion is the shooter's collision root and its children, and its squadron arm is
Kamikaze-only. So this host is faithful on this path and **no code is written for item 2.**

What this host may still get wrong is the collision *root*: the image excludes the shooter's root and
everything parented to it, while this host excludes one unit index. That distinction only bites if
an aircraft's root has children, and `docs/TORPEDO_AFTER_THE_DROP.md` section 15.3 already measured
that the six Kates each carry their own collision presence (the AA guns damage them individually), so
it does not bite here. Recorded as the one residual, not as a defect.

### 3.3 MEASURED observation: the two aircraft are about five metres apart, and they shoot each other

The integrator asked for the shooter's and the victim's separation at the drop, because a hit one
tick after release means the victim is within a few metres of the muzzle. **No new instrumentation
was needed**: the existing trace and drop lines already pin it, and the method is stated here so the
bound can be checked.

At `life = 0.05 s` a round has travelled about `0.05 * 84 = 4 m`, so the recorded `entity_impact`
point is within ~4 m of the muzzle, i.e. of the *shooter*, and it is on the *victim's* hull. The
separation between the two aircraft is therefore bounded by that impact point, to within a few metres.

From `local/rel_usn04_before.log` and `local/rel_usn04_after.log`:

| arm | trace | shooter | victim | impact point | pairwise separation |
| --- | --- | --- | --- | --- | --- |
| after | 1 | `B5N Kate #4.1` | `B5N Kate #4.1\|.-3` | (11270.1, 11.73, -11508.5) | **5.2 m** |
| after | 2 | `B5N Kate #4.1\|.-3` | `B5N Kate #4.1` | (11275.2, 11.78, -11507.4) | (same pair) |
| after | 7 | - | `B5N Kate #8.1\|.-3` | (10071.8, 11.60, -11184.4) | **4.4 m** |
| after | 8 | - | `B5N Kate #8.1` | (10074.0, 11.63, -11180.6) | (same pair) |
| before | 7 | - | `B5N Kate #8.1\|.-3` | (10026.6, 11.76, -11060.2) | **7.5 m** |
| before | 8 | - | `B5N Kate #8.1` | (10033.2, 11.78, -11063.7) | (same pair) |

**Traces 1 and 2 are the decisive pair.** The drop lines name their shooters: drop 1 by
`B5N Kate #4.1`, drop 2 by `B5N Kate #4.1|.-3`. Trace 1 killed `#4.1|.-3` and trace 2 killed `#4.1` -
**the two aircraft torpedoed each other**, on the same tick, each at the other's position. Their two
impact points are 5.2 m apart and at the same altitude (y = 11.73 and 11.78, against a drop altitude
of 12 m).

So the separation is **4 to 8 metres**, between aircraft of the same flight, at a common altitude.
The image's authored spacing for this is `Formation_UnitDist = 300.0` (`00CE3AE8`,
`docs/TORPEDO_AFTER_THE_DROP.md` section 15.4). Four to eight metres is not a formation shape that is
slightly wrong; it is two aircraft effectively **co-located**, which is a placement defect in the
formation hunk - a duplicate slot, or a shape whose offsets are unbound - and not anything the
torpedo chain can fix. That is the finding to route to whoever owns formation.

The count is **4 of 12 drops** in both arms (traces 1, 2, 7, 8), unchanged by item 1, which is what
section 3.3's expectation said: a closer release does not move the formation geometry at the instant
of the drop. The predecessor recorded three; this tree's main has four.

### 3.4 What is reported instead

No prediction and no run of its own: there is no change to test. The item 2 rounds are reported from
the item 1 arms as an observation - how many rounds die on a squadron mate at `life ~ 0.05 s`, and
whether item 1's closer release changes that number. A closer release does not move the formation
geometry at the instant of the drop, so the expectation is that it does **not** change, which also
keeps the two items separable.

The real cause is the formation offset (`docs/TORPEDO_AFTER_THE_DROP.md` section 15.4): three
aircraft of a `SpawnNew` squadron sitting on or near one point. That is not in the torpedo chain and
is not this packet's to fix.

## 4. Status

Filled in as the runs land. Nothing below is claimed until its log name is written beside it.

* **Proved from the listing**: sections 1.1 (the cached product read twice, the `RET 4` frame check,
  the `[ESP+28h]` census), 1.2 (the verbatim seed, `EAX` provenance, the `00F8A30C` indirection),
  1.5, and 3.1 (step 6 is Kamikaze-gated; the sweep's one exclusion is the collision root).
* **Authored content, this installation**: section 1.3 (robots.lua, mtime 2025-06-01 16:03:10, six
  rows, `SPNormal` 0.7, no row authoring 1.0).
* **Labelled substitutions**: the choice of the `SPNormal` row (1.4), and
  `target_is_kind_vtable5c` accepting every ordered target (2.7).
* **Measured**: 2.4 (USN01 before), 2.5 (USN04 before), 2.6 (both after arms), 3.3 (the
  own-formation separation). Logs named in each.
* **Falsified, my own prediction**: section 2.6. The clamped rounds all moved, because the gate's
  aspect input was stubbed to zero; the cause is proved in 2.6.1 and reproduces the numbers to
  within a metre.
* **Retracted, my own first reading**: section 3.1, the claim that step 6 is an own-squadron
  exemption the torpedo host was missing.
* **Withdrawn from another document**: `docs/TORPEDO_AIM_LEAD.md` section 6's back-solved release
  range, and section 9's premise that `approach+84h` touches only the aim-solution byte. Both noted
  in place, with the zero-lead conclusion explicitly left standing.
* **Changed**: `approach+84h` seeded from the robots row instead of forced to `1.0f`; the aim tick's
  aspect input bound from the ordered target; a `release_range` / `abs_cos_aspect` census column;
  one stale header comment withdrawn.
* **Not done**: no code for item 2 (the image does the same thing); no Ghidra annotation.
