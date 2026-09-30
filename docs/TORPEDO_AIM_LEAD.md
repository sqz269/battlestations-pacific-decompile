# The aircraft torpedo attack does not lead its target (packet `cc8_torpedo_lead`)

Owner: `agent/cc8-torpedo-lead`. This packet was set one question: does an aircraft torpedo attack
lead its target, and where? The answer is **no**, and this document proves it from the listing,
names the three places a lead could have lived and shows it is in none of them, and then checks the
reading against the recorded USN01 trace.

**There is no code change in this packet.** The host already aims at the target's present position
with zero lead, which is what the image does, so the five-round USN01 outcome is faithful.

Read `docs/TORPEDO_AFTER_THE_DROP.md` sections 5, 6.3, 11 and 13 first for the history; this
document supersedes section 6.3's open question and retracts one of its claims (section 5 below).

## 1. The chain, end to end

Addresses are from the raw listing (`bsp.py disasm-raw`, disk bytes) because `009D15F0` and
`009D3420` have no Ghidra function body.

| step | address | what happens |
| --- | --- | --- |
| the aim point | `009D0670` `BSP_BotApproachTorpedo_GetAimPoint` | `FLD [ECX+D0h]/[+D4h]/[+D8h]` -> the out-param at `[ESP+4]`. Eight instructions, `RET 4`, **no arithmetic**. |
| own position | `009D34EA`, `009D34F8` | `unit+FCh` (X) and `unit+104h` (Z), the translation row of the entity world matrix that starts at `unit+CCh`. |
| the call | `009D3517` | `this->vtable[0](&local)` - the update asks itself for the aim point. It calls the same slot twice more: `009D36E4`, feeding a line-of-sight query at `007DF360` with the target pointer, and `009D3DC8`, whose result is read as X and Z at `009D3DCA`/`009D3DD7`. All three are the same eight-instruction copy; none of them leads. |
| the difference | `009D3519`, `009D3523` | `aim.x - own.x` and `aim.z - own.z`. |
| range | `009D3552` sqrt -> `009D357E` | `approach+90h`, with an epsilon at `00CE3820` that collapses it to zero. |
| bearing | `009D3586` atan2, `fsubr` `00CE3830`, wrap by `00CE3828` -> `009D35C0` | `approach+94h`, a compass bearing. |
| steered at (aim) | `009D161A` reads `+94h`; `009D1B7A`, `009D1B95`, `009D1BCC`; `009D1D16` | commanded heading -> `cmd+2C0h`, mode `cmd+2CCh = 2`. |
| steered at (run-in) | `009D07BD` reads `+94h` | the attack run's own re-roll, `009D0927` `FST [EAX+2C0h]`. |

**The bearing is a pure `atan2` of (aim point - own position).** No velocity term, no time term, no
target-motion term of any kind enters it. A lead therefore cannot live in the update or in either
steering site; it could only be baked into the aim point itself.

### 1.1 The frame walk behind the aim tick row

`[ESP+24h]` is read at `009D1D10` and written at `009D1BD4`, and those are the same slot only
because the two pushes at `009D1BBC`/`009D1BBD` shift `ESP` by 8 for both. The intervening
`SUB ESP,8` / `SUB ESP,14h` blocks are all restored by their callees (`00438AA0`, `00438B10`,
`00419010` are `RET 8`/`RET 14h`), so no `ADD ESP` appears and the frame is stable across them.
Writing F for the post-prologue `ESP`, the commanded heading is `F+1Ch`, and it is
`AddWrappedAngle(F+20h, SubtractWrappedAngle(F+1Ch, F+20h))` over a value seeded at `009D1622` from
`approach+94h`. The only thing added along the way is the sector turn offset, which
`docs/TORPEDO_AFTER_THE_DROP.md` section 13.5 already showed is **zero on open water** by the
routine's own all-clear rule.

## 2. A lead needs the target's velocity, and the chain never asks for it

This is the decisive argument, and it does not depend on finding who writes the aim point.

The only virtual calls made on the ordered target (`approach+CCh`) anywhere in the torpedo chain:

| site | slot | what it returns |
| --- | --- | --- |
| `009D1714` | `+5Ch` (arg 6) | a predicate, `TEST AL,AL` |
| `009D1721` | `+50h` | the target's **heading** |
| `009D175F`, `009D176E` | `+5Ch` (args 10h, 16h) | the same predicate, other tags |
| `009D1E08`, `009D1E17` | `+5Ch` (args 10h, 16h) | the same predicate |

The `+50h` result is consumed at `009D1739`-`009D174C` as
`abs(SubtractWrappedAngle(own_heading, target_heading))`. That is an **angle-off magnitude** - a
crossing-angle gate - not a lead: an absolute value of an angle difference cannot displace an aim
point. No `vtable[34h]` dispatch (the velocity getter slot named in
`docs/TORPEDO_FLY_TO_SOLVER.md`) occurs anywhere in `009D15F0`, `009D3420` or `009D07B0`.

And the image's one actual lead mechanism is censused and does not reach the torpedo:
**`009FA2E0`, the velocity getter forwarder, has exactly two callers image-wide** -
`009C61F9` and `009C62E8`, both inside `BSP_BotStateDiveBombFlyAbove`, which is the
`aim - 3.0 * (v_own - v_target) - pos` site. None in `009D0000`-`009D5000`.

A caution about the 3.0 constant, because it is easy to misread: the torpedo aim tick **does** load
`qword [00D7A2B0]` at `009D1CA9` and `009D1E49`. Neither is a lead. At `009D1CA9` it scales
`config+A4h` into a lookahead time fed to `BSP_Math_InterpolateClamped`; at `009D1E49` it is the
additive term of a descent-time estimate `(00D7A208 - unit+C64h) / config+1ACh * k + 3.0`. No
target velocity is anywhere near either. **The constant is shared and is not diagnostic of a lead.**

## 3. The aim point's writer: a complete displacement census, and what it does not close

Sixteen encodings were scanned image-wide for each of `+90h`, `+94h`, `+CCh`, `+D0h`, `+D4h`,
`+D8h`, every one with a healthy positive control so that no negative is vacuous:

| form | encoding | image-wide hits on `+D0h` |
| --- | --- | --- |
| x87 load/store | `D9 ?? D0 00 00 00` | 109 |
| x87 double | `DD ?? D0 00 00 00` | 108 |
| MOVSS store / load | `F3 0F 11 ??` / `F3 0F 10 ??` | 16 / 4 |
| MOVUPS store / load | `0F 11 ??` / `0F 10 ??` | 16 / 4 |
| MOVAPS store | `0F 29 ??` | 1 |
| MOV store / load | `89 ??` / `8B ??` | 59 / 145 |
| MOV immediate | `C7 ??` | 8 |
| LEA, no SIB / with SIB | `8D ??` / `8D 84 ??` | 29 / 17 |
| MOVQ store / MOVLPS store | `66 0F D6 ??` / `0F 13 ??` | 1 / 1 |
| ADD r32,imm32 / ADD EAX,imm32 | `81 ?? D0 00 00 00` / `05 D0 00 00 00` | 44 / 4 |

**Inside `009D0000`-`009D5000` the only hit on the whole `+D0h/+D4h/+D8h` triple, across all
sixteen forms, is the `FLD` at `009D0674`/`009D067C`/`009D0685` inside `GetAimPoint` itself.**
There is no store, in any form, including the four pointer-construction forms (`LEA`+SIB, `ADD`)
that the earlier census in `docs/TORPEDO_AFTER_THE_DROP.md` section 6.3 never ran and that were the
named escape route for a disp8 write through an aliased pointer.

### 3.0 The second displacement, which no earlier census had

Section 6.3 of `docs/TORPEDO_AFTER_THE_DROP.md` names the escape route for its own negative: "the
producer holds the approach at some other offset". It does.

**Credit where it is due: the layout below was already in the ledger**, on `009D0670`'s own record
("found from `009D3050`'s `LEA EDI,[ESI+3F8h]` at `009D3080` and `MOV [EDI],0D213C0h` at
`009D30A7`"). I re-derived it here without having read that record first. What is new in this packet
is not the layout but the *consequence nobody had drawn from it* - that `+D0h` is therefore also
addressable as `task+4C8h`, and that this second displacement had never been scanned.

`009D3050 BSP_BotTaskTorpedo_Construct` lays the task out as follows:

| address | instruction | what it establishes |
| --- | --- | --- |
| `009D3080` | `LEA EDI,[ESI+3F8h]` | a sub-object at `task+3F8h` |
| `009D3091` | `CALL 009D2DA0` with `ECX=EDI` | that sub-object is constructed by `ConstructStates` |
| `009D30A1` | `MOV [ESI],0D213C8h` | the **task's own** vtable is `00D213C8` |
| `009D30A7` | `MOV [EDI],0D213C0h` | the sub-object's vtable is `00D213C0`, whose slot 0 is `009D0670` |
| `009D30AD` | `MOV [ESI+530h],0D213BCh` | a third sub-object at `task+530h` |

`00D213C8` slot 0 is `009D4E10`, a `CG_scalar_deleting_dtor` - the ordinary MSVC slot 0 for a class
with a virtual destructor. So `009D3420`'s `ESI` cannot be the task: `009D3517` would be calling a
destructor, and the return value is dereferenced as a vec3 at `009D3519`. **`ESI` is the approach
sub-object at `task+3F8h`**, and its `+D0h` is therefore also reachable as **`task+4C8h`**.

That second displacement was scanned too - `C4 04 00 00`, `C8 04 00 00`, `CC 04 00 00`,
`D0 04 00 00` over the x87, MOVSS-store, MOVUPS-store and MOV-store forms, image-wide (controls
15/12/15/15 on the x87 form). The only hits anywhere near the bot bands are **loads**, in the dive
bomb: `009C8A20` (`BSP_BotTaskDiveBomb_UpdateCruiseProfile`), `009C8B40`
(`BSP_BotTaskDiveBomb_ShouldBreakOff`) and `009C3F39`/`009C3F51`. No store, and nothing in
`009D0000`-`009D5000`.

**So both candidate displacements are now closed**: approach-relative `D0 00 00 00` and
task-relative `C8 04 00 00`. That is strictly more than any earlier census had, and it retires the
specific escape route section 6.3 named.

### 3.0.1 The base-class hypothesis, tested and refuted

The natural remaining hypothesis is a base-class `SetTarget`/refresh taking `this` = the approach,
which would use exactly the `+D0h` displacement but live outside the `009D` band. Classifying every
image-wide `+D0h`/`+D4h`/`+D8h` **store** hit by enclosing function and looking in the two bands
where such a method would sit:

* **`007B4000`-`007B5000`** (the base band): the only hits are `007B4339` `FSTP [ESI+0D0h]` and
  `007B4341` `MOV [ESI+0D4h],imm32`, both in `007B41E0` - which is **not** a vec3 triple (there is
  no `+D8h` store) and which installs vtable `00D0577C` at `007B4237`, a different class.
* **`009F9C00`-`009F9E40`** (the approach base constructor's neighbourhood): **no hits at all.**

And the three calls `009D3420` makes *before* `009D3517` are `009FADA0` (which writes only small
offsets on its own `this` - `+1Ch`/`+20h`/`+24h` from `BSP_Vector3f_TransformAffinePoint`),
`007B93F0 BSP_WeaponController_HasTorpedoOrdnance` (a predicate) and `00414DB0` (a pose refresh that
writes the entity, not the approach). None of them is the writer.

**What this still does not close, stated plainly.** "No writer names either displacement" is still
not "no writer". A block copy, or a pointer aliased into a register by some form not enumerated
above, would be invisible. **I did not find the writer of the approach's `+D0h`, and I am not
claiming the field is dead.** The lead answer does not rest on it: section 2 is a complete census of
the only routine in the image that fetches a target velocity for a bot, so whatever writes `+D0h`
did not obtain a velocity to bake a lead into it.

### 3.1 Corroboration from the sibling classes

Sibling bot-task classes do write the same triple. `009B44F0` refreshes the entity pose through
`00414DB0 BSP_EntityPose_RefreshWorld` (guarded by the `entity+C8h` dirty byte) and then copies
`entity+FCh/+100h/+104h` straight into `this+D0h/+D4h/+D8h` at `009B462E`-`009B464E`: the present
world position, verbatim, zero lead. Other writers of the triple are `009B4D00`, `009B5C80`,
`009B6670`, `009B7C90`, `009CA4A0`, `009CCED0`.

**Uncertainty, not hidden:** `009B44F0` is a different class (it ends `MOV EAX,ESI` / `RET 8`, an
MSVC constructor returning `this`, and its own `+CCh` is a *byte* at `009B45E8`, not the pointer the
torpedo task keeps there). So this is corroboration for what the field means, not proof for the
torpedo, and a constructor store is never authoritative about a per-tick value.

### 3.2 Two live hypotheses, and why both give the same answer

Either the torpedo task's `+D0h` is refreshed per tick by an unfound writer with the target's
present position (H1), or it is seeded once when the target is assigned and never refreshed (H2).

* Under **H1** the host is faithful.
* Under **H2** the image aims at a **stale** point - where the target was when the order was given -
  and this host, which re-reads the target every update, is *more* accurate than the image.

Neither is a lead, and neither supports adding a lead term. H2 would be a divergence in the opposite
direction, and it is the one worth testing if this stream is picked up again.

## 3.3 What the image does instead of leading

The crossing angle of section 2 is not discarded. `009D174C` stores
`abs(SubtractWrappedAngle(own_heading, target_heading))` into the frame slot `F+2Ch`, and
`009D1F9F` reads it back (`[ESP+34h]` there, because `EBX`/`EBP` are pushed at
`009D1BBC`/`009D1BBD` and not popped until `009D22E2`/`009D22E9`), takes `FCOS` at `009D1FA3`, and
takes the bit-level absolute value at `009D1FBF`. That `|cos(aspect)|` is the last argument of the
`BSP_Math_InterpolateClamped` call at `009D1FED`, whose result scales the range threshold that the
comparison at `009D2008` tests, writing the aim-solution byte `approach+130h` at `009D2021`.

**So the image answers target motion with a release *gate*, not with an aim offset**: aspect decides
*when* the aircraft may declare a solution and drop, while *where* it aims stays the target's
position with no lead. That is a coherent design, and it is the reason a no-lead attack is not
simply broken.

This is confirmation of existing work, not a new finding: the host already binds this chain, in
`src/torpedo_aim_tick.cpp:91` `torpedo_aim_solution_009d2021`, with the constants
`kAspectX0 = 0.5f` (`00CE3800`), `kAspectY0`/`kAspectX1` from the two `FLD1`s, and `approach+84h`
read at `009D1FD0`. I re-derived it from the listing while looking for a lead and record it here
because it is the piece that makes the no-lead reading make sense.

### 3.3.1 The direction of the gate, now established

The five arguments of the `009D1FED` call, assembled from the two stack windows
(`00419010` is `RET 14h`, so the callee clears them):

| stack slot | loaded at | value |
| --- | --- | --- |
| `[ESP]` | `009D1FE4` | `00CE3800` = **0.5** (`f:`, the load is `FLD dword`) |
| `[ESP+4]` | `009D1FE0` | `FLD1` = **1.0** |
| `[ESP+8]` | `009D1FDC` | `FLD1` = **1.0** |
| `[ESP+0Ch]` | `009D1FD0` | `approach+84h` |
| `[ESP+10h]` | `009D1FC8` | `|cos(aspect)|` |

so `InterpolateClamped(x0 = 0.5, y0 = 1.0, x1 = 1.0, y1 = approach+84h, x = |cos|)`, and the result
multiplies a distance before `009D2002` adds `00CE4D70` = **200.0** and `009D2008` compares against
the range (`AL = 1`, in range, when `threshold > range`).

**The convention.** `009D1739` computes `SubtractWrappedAngle(target_heading, own_heading)` and
`009D1746` takes the absolute value, so the slot holds the angle **between the two headings**. Then
`|cos|` is **1 when the aircraft runs along the target's course** (bow-on or stern-on) and **0 on the
beam**. So the interpolation reads: on the beam (`|cos| <= 0.5`, i.e. aspect between 60 and 120
degrees) the factor is `1.0`, the full release distance; running along the target's course
(`|cos| -> 1`) the factor goes to `approach+84h`.

**And `approach+84h` is authored.** `009D049D`/`009D04A0` seed it in
`BSP_BotApproachTorpedo_Reset` as `(approach+14h)->+0Ch`, taken verbatim with no scale.
`009F9D1E` makes `approach+14h` = `00F8A30C + index*248h + 0Ch`, so this is the robots row's
**`+18h`**, which `include/bsp/robot_config.hpp` names `torp_release_drop_closer_mul_018` (reader
`009973B0`).

In **this installation**'s `scripts/datatables/robots.lua` (mtime 2025-06-01 16:03:10 - this
installation is modded, so this is its value, not a claim about retail), the `SPNormal` row line 560:

    ["TorpReleaseDropCloserMul"] = 0.7,   -- F -- a torpedo ledobasi tavolsag arra az esetre
    vonatkozik, ha oldalba kapjuk a celpontot. ha szembol/hatulrol akarjuk megtorpedozni, akkor a
    torpedo oldasi tavolsag az ennyiszeresere csokken.

Other rows author 0.5 (line 699) and 0.7 (line 837). The developers' own comment translates as: *the
torpedo release distance applies to the case where we catch the target on the beam; if we want to
torpedo it from the front or behind, the release distance is reduced to this multiple.*

**So the answer is: only from CLOSER.** A beam-on attack may release at the full distance; a bow-on
or stern-on attack must close to 0.7 of it. The authored comment states the same convention I
derived from `009D1739`-`009D174C`, independently, which is as good a confirmation of the reading as
this packet has.

**This is the image's only compensation for not leading, and it is the sensible one**: a crossing
target is where zero lead costs the most, so the beam attack is allowed the long shot, and the
along-course attack - where a straight runner barely needs a lead at all - is made to close.

## 4. The host, and why there is nothing to bind

`src/game_hosts_units.cpp:4741` `approach_target_point` is the host's stand-in for
`009D3517`'s `vtable[0]` call. It returns the ordered target's present world position and is already
labelled a substitution in its own comment. `src/torpedo_approach_update.cpp:356`-`369` consumes it
into `range_90` and `bearing_94` through `torpedo_approach_range_009d3519` and
`torpedo_approach_bearing_009d3586`.

**The defect this packet went looking for does not exist.** The host aims at the target's present
position with zero lead; the image's chain provably contains no lead; the misses are faithful.

## 5. Retraction

`docs/TORPEDO_AFTER_THE_DROP.md` section 6.3 reports the `+D0h` writer census as a set of clean
negatives with stated image-wide controls. Those counts are right, but the method that produced the
in-band negatives is unsafe as run: **`bsp.py scan-bytes` silently caps its result list at 20** and
prints `... N more matches` only on the last line. A filtered pass over the head of a capped list
returns an empty in-band result whether or not in-band hits exist. My own first `+94h` pass came
back empty against 57 real matches for exactly this reason. Every count in section 3 above was
re-run with `--limit 4000`. The section 6.3 conclusions happen to survive the re-run; the reasoning
that reached them did not establish them.

Two naming corrections while this is being recorded:

* `00D213C0` is the vtable of the **approach sub-object at `task+3F8h`**, installed at `009D30A7`;
  its slot 0 is `009D0670`. The **task's own** vtable is `00D213C8` (installed at `009D30A1`),
  whose slot 0 is the `CG_scalar_deleting_dtor` at `009D4E10`. The adjacent `00D213B8` is installed
  by `009D2DA0 BSP_BotTaskTorpedo_ConstructStates` at `009D2DFE`, and `00D213BC` goes on a third
  sub-object at `task+530h` (`009D30AD`).
  *I reported the first half of this to the integrator in an earlier form - "`00D213C0` is the task
  vtable" - before reading `009D4E10`. That was wrong and this is the correction: `00D213C0` is the
  approach's, `00D213C8` is the task's.*
* The approach base class is not at `009F9C00`-`009FA400`. The non-`009D` slots of `00D213C0` point
  at `007B40C0`, `007B40F0` and `007B4100`, so the base band is `007B4xxx`.

One offset trap worth recording: `+CCh` and `+D0h` mean different things in different classes. On an
**entity** `+CCh` begins the 4x4 world matrix whose translation row is `+FCh`/`+100h`/`+104h` - that
is why `009FADA0` takes `LEA EAX,[EDI+CCh]` and hands it to
`BSP_Vector3f_TransformAffinePoint`. On the **torpedo task** `+CCh` is the ordered-target pointer and
`+D0h` is the aim point. A census across classes at a fixed offset mixes the two.

## 6. Check against the recorded USN01 trace

Source trace: `docs/SHIP_ESCORT_SCREEN.md` section 6.3 (five drops, five swims). Both columns were
checked against their printing code before being used, per the standing rule:

* `drop_crossing_angle` = `|wrapped_angle_subtract(owner_heading, target_heading)|` at the drop
  (`src/game_hosts_gunnery.cpp:3280`) - the aircraft's heading against the target's course, in the
  same heading space (the comment at `:3272` records that check).
* `target_travel` = the ordered target's straight-line displacement **from release to closest
  approach** (`src/game_hosts_gunnery.cpp:164`-`167`).
* Distances are **centre to centre and horizontal** (`:3601`). Read each against the target's own
  length, not as a miss from the plating.

Constant-velocity closest-approach geometry for a straight runner launched along the bearing to the
target's drop-time position, with `s = 1853.5 / 60.05 = 30.87 m/s`:

    |v_rel|^2 = s^2 + v_t^2 - 2*s*v_t*cos(theta)
    t_cpa     = R * (s - v_t*cos(theta)) / |v_rel|^2
    miss      = R * v_t * sin(theta) / |v_rel|

`R`, the drop range, is not in the recorded table, so it is back-solved from the measured `t_cpa`.
**This is therefore a consistency check with one fitted parameter, not a free prediction.**

> **2026-09-19, packet `cc8_torpedo_release`: the fitted `R` in the table below is WITHDRAWN.**
> That packet added a `release_range` column to the closest-approach census (the drop line printed
> only the drop altitude, so no run had ever recorded the release range) and measured it directly on
> this same mission: **433-441 m on all five rounds**, against the 327.0 / 333.7 / 338.5 / 351.9 /
> 355.0 m back-solved here. The error is about `+100 m` on every round, systematic rather than
> scatter. `local/rel_usn01_before.log`.
>
> What this withdraws is the fitted parameter and every number in the table that depends on it - the
> `zero-lead miss` column and therefore the `residual` column. What it does **not** touch is this
> document's conclusion: the zero-lead reading rests on section 7's crossing-angle split (traces 3
> and 9, same ship, same speed, differing only in aspect), which uses no `R` at all.
>
> The correction does not simply slot back in, either. With the measured `R` and the same
> `s = 30.87 m/s`, `t_cpa` for Mav1 -> Dunlap comes out **9.84 s** against the recorded **7.40 s**, so
> another term of this model is also wrong - most likely that the round is *dropped* at 435 m but
> *enters the water* closer, after an air fall these equations do not model. That is recorded, not
> fitted. `docs/TORPEDO_RELEASE_GATE.md` section 2.4.

| round | v_t (m/s) | R fitted (m) | zero-lead miss | recorded | residual |
| --- | --- | --- | --- | --- | --- |
| Mav1 -> Dunlap | 19.09 | 327.0 | 160.7 | 210.3 | +49.6 |
| Mav4 -> SaltLakeCity | 16.60 | 333.7 | 135.5 | 175.2 | +39.7 |
| Mav5 -> SaltLakeCity | 16.61 | 338.5 | 142.2 | 182.6 | +40.4 |
| Mav3 -> Northampton | 0.00 | 351.9 | 0.0 | 8.5 | +8.5 |
| Mav2 -> Northampton | 0.00 | 355.0 | 0.0 | 9.1 | +9.1 |

**The zero-lead geometry accounts for 77, 77 and 78 per cent of the three misses**, and predicts
zero for the two stationary rounds, which hit. The reading in sections 1 and 2 survives a
quantitative test it could have failed.

**The residual is real and is not explained here.** It is systematic - `+40` to `+50 m` on all three
movers against `+8.5`/`+9.1 m` on the two stationary rounds - so it is not a constant angular
release error: at the fitted ranges that would be about 7 degrees for the movers against 1.4 degrees
for the stationary pair. Because it scales with target motion, the candidates are that
`target_travel` is a straight-line **chord** while the escorts are **turning** (so the real
perpendicular displacement exceeds what `v_t` derived from the chord implies), and that the escorts
are still accelerating over the run. Neither was measured. **I am recording the residual rather than
fitting it away**; no constant in this packet was tuned.

What the table does *not* license: it does not say a lead term would convert these to hits. A
correct lead would remove the 135-160 m geometric term and leave the 40-50 m residual, and whether
that is a hit depends on the hull box this host does not have.

## 7. USN04, out of sample - and a retraction of section 6's residual guess

One run, no code change: `local/aimlead/usn04_torpedo.log`,
`--frames 8000 --press-start-frame 30 --menu-select USN04 --mission-frames 6000
--mission-frame-seconds 0.05`, launched 13:08:53 on launcher slot 1, finished with
`native renderer final COM release`. `torpedo_drop drops=12 refusals=0 water_entry_breakups=0`,
`swims_started=8`, `torpedo_closest_approach swims=8`.

| trace | shooter -> ordered | exit | ordered min | at | target moved | drop crossing |
| --- | --- | --- | --- | --- | --- | --- |
| 3 | Kate #4.1\|.-2 -> Yorktown-class01 | **hit `Yorktown-class01`** | 23.7 m | 7.30 s | 120.8 m | 172.0 deg |
| 4 | Kate #2.1 -> Lexington-class01 | **hit `Lexington-class01`** | 55.4 m | 9.75 s | 0.0 m | 163.7 deg |
| 5 | Kate #2.1\|.-2 -> Lexington-class01 | **hit `Lexington-class01`** | 50.1 m | 9.90 s | 0.0 m | 163.2 deg |
| 6 | Kate #2.1\|.-3 -> Lexington-class01 | **hit `Lexington-class01`** | 49.5 m | 10.00 s | 0.0 m | 163.9 deg |
| 10 | Kate #6.1 -> Lexington-class01 | hit `Fletcher-class02` at 5.00 s | 202.0 m | 5.00 s | 0.0 m | 163.7 deg |
| 11 | Kate #6.1\|.-2 -> Lexington-class01 | hit `Fletcher-class02` at 5.30 s | 192.1 m | 5.30 s | 0.0 m | 163.2 deg |
| 12 | Kate #6.1\|.-3 -> Lexington-class01 | hit `Fletcher-class02` at 5.30 s | 194.5 m | 5.30 s | 0.0 m | 163.9 deg |
| 9 | Kate #8.1\|.-2 -> Yorktown-class01 | `expired`, range 1852.0 | 167.9 m | 7.75 s | 128.3 m | 110.8 deg |

Traces 10-12 were stopped by the **escort screen**: the ordered target was Lexington, and they
struck `Fletcher-class02` a third of the way there. Traces 2, 7 and 8 are not in the table because
they never swam - they `entity_impact` a **friendly B5N Kate at life=0.05 s**, at y = 11.66, 11.76
and 11.78, i.e. the round strikes an aircraft of the dropping formation immediately on release.
Both of those are outside this packet and are reported to the integrator as observations, not
diagnosed here.

### 7.1 The out-of-sample test the model could have failed

The section 6 model, run on USN04 with **nothing refitted** (same `s = 30.87 m/s`, same equations,
`R` back-solved from each row's own `t_cpa`):

| trace | target | v_t | crossing | zero-lead miss | recorded | outcome |
| --- | --- | --- | --- | --- | --- | --- |
| 3 | Yorktown, moving | 16.55 | 172.0 deg | 16.8 m | 23.7 m | hit |
| 9 | Yorktown, moving | 16.55 | 110.8 deg | 130.1 m | 167.9 m | miss |
| 4 | Lexington, stopped | 0.00 | 163.7 deg | 0.0 m | 55.4 m | hit |
| 5 | Lexington, stopped | 0.00 | 163.2 deg | 0.0 m | 50.1 m | hit |
| 6 | Lexington, stopped | 0.00 | 163.9 deg | 0.0 m | 49.5 m | hit |

**Traces 3 and 9 are the strongest single piece of evidence in this packet.** Same mission, same
ship class, same target speed, same torpedo - the *only* material difference is the crossing angle.
At 172 degrees the perpendicular component of the target's 120.8 m of travel is
`120.8 * sin(172 deg) = 16.8 m` and the round **hits**; at 110.8 degrees the perpendicular component
of 128.3 m is 130 m and the round **misses and expires**. The hit/miss split follows
`target_travel * sin(crossing)` - which is exactly and only what a zero-lead aim produces. A leading
aim would have brought both to zero.

### 7.2 Retraction: section 6's explanation of the residual is wrong

Section 6 offered, as unmeasured candidates for its `+40` to `+50 m` residual, that `target_travel`
is a chord while the escorts turn, and that the escorts are still accelerating - both of which make
the residual **scale with target motion**. USN04 refutes that: traces 4-6 have `target_moved = 0.0`
and a residual of `+49.5` to `+55.4 m`, while trace 3 moves 120.8 m and has a residual of `+6.9 m`.
The residual does not track motion at all.

The decomposition that does fit both missions is the measurement definition, which section 6 quoted
and then failed to apply: **distances are centre to centre** (`src/game_hosts_gunnery.cpp:3601`).

* For a **hit**, the recorded number is where the round met the hull *relative to the target's
  centre*, so it is not model error at all. It tracks the target's size: Northampton (180 m class
  length) 8.5 and 9.1 m; Yorktown 23.7 m; Lexington, a far longer carrier, 49.5 to 55.4 m.
* For a **miss**, the model under-predicts by `+37.8` to `+49.6 m` across four independent rounds in
  two missions (USN01 Mav1/Mav4/Mav5, USN04 trace 9). That band is tight and is the real residual.

I am recording that the real residual is therefore a consistent `+40 m`-ish under-prediction on
misses, and that **its cause is still unmeasured**. What section 6 got wrong was not the number but
the explanation, and the explanation was refuted by the first out-of-sample data it met.

## 8. Status

* **Proved from the listing**: sections 1, 2, 3, 5.
* **Corroborated, different class**: section 3.1.
* **Open**: the writer of the torpedo task's `+D0h` (section 3), and which of H1/H2 holds
  (section 3.2).
* **Measured**: section 6 against the existing `docs/SHIP_ESCORT_SCREEN.md` section 6.3 trace, and
  section 7 out of sample on USN04 (`local/aimlead/usn04_torpedo.log`).
* **Retracted by my own later evidence**: section 6's explanation of the residual (section 7.2), and
  the claim I first sent the integrator that `00D213C0` is the task vtable (section 5).
* **Withdrawn by a later packet (2026-09-19, `cc8_torpedo_release`)**: section 6's back-solved `R`,
  and the `zero-lead miss` and `residual` columns that depend on it. The release range is now
  measured, not fitted, and is about 100 m longer on every round. The zero-lead conclusion stands,
  because section 7 does not use `R`. See the note in section 6.
* **Superseded**: section 9's follow-up. `src/game_hosts_units.cpp` no longer forces
  `aspect_scale_84 = 1.0f`; it is seeded from the robots row as `009D049D` does. That packet also
  corrects the claim, repeated in section 3.3 here, that `approach+84h` shapes only the aim-solution
  byte: `009D1FED`'s product is cached at `009D1FF6` and read again at `009D2034`, so it gates the
  release chain too. `docs/TORPEDO_RELEASE_GATE.md` section 1.1.
* **Not changed**: no source file, no constant, no Ghidra annotation.

## 9. The one thing worth a follow-up packet

`src/game_hosts_units.cpp:7887` sets `in.aspect_scale_84 = 1.0f`, with a comment saying
`approach+84h` "is not in the approach struct yet". It is a labelled substitution, not a silent
stub, so it breaks no project rule - but it makes section 3.3's mechanism **inert**, and that is
worth stating because the mechanism is the image's whole answer to a moving target.

`009D1FED` calls `InterpolateClamped(0.5, 1.0, 1.0, approach+84h, |cos aspect|)`. With
`approach+84h` forced to `1.0` both interpolation endpoints are `1.0`, so the result is `1.0` for
every aspect and the permitted release range stops depending on the crossing angle at all. In the
image it presumably does depend on it.

**That question is now answered (section 3.3.1): `approach+84h` is `TorpReleaseDropCloserMul`, and
this installation authors it as 0.7, not 1.0.** So the stub is not merely a placeholder of unknown
value - it is known to be wrong by 30 per cent in the one direction that matters, and the gate is
inert where the image would bite.

The concrete, testable consequence for whoever takes the follow-up: the factor only departs from 1.0
when `|cos(aspect)| > 0.5`. Checked against the two runs in this document:

* **USN01's five rounds are unaffected.** Crossings 98.0, 107.0, 103.8, 118.6 and 121.3 degrees give
  `|cos|` of 0.139, 0.292, 0.238, 0.477 and 0.520 - four below the 0.5 knee and one barely above it.
  Binding `0.7` would not move the three misses. **The USN01 misses stay faithful either way**, which
  is why this packet still closes with no code change.
* **USN04's rounds are affected.** Crossings 163 to 172 degrees give `|cos|` of 0.96 to 0.99, so the
  image would require those bombers to close to about 0.7 of the release distance, while this host
  let them release at the full distance. Traces 3, 4, 5, 6, 10, 11 and 12 all sit in that band.

So the follow-up has a ready before/after: bind `aspect_scale_84` to the row value and re-run USN04
at the section 7 parameters. **Prediction to be recorded before that run**: the seven along-course
rounds release roughly 30 per cent closer, which should *reduce* the escort-screen interceptions
(traces 10-12 struck `Fletcher-class02` a third of the way to the ordered Lexington) because the
bomber flies further in before the round is in the water. I am not making that change here: it is
outside this packet's addresses, I hold no lease on it, and it needs its own before/after window.

## 10. The aim point's writer, found (integrator's note, 2026-09-19)

Section 3 closed its census honestly: no store names `approach+D0h/+D4h/+D8h`, nor the
task-relative `task+4C8h`, anywhere in the image, "which is not 'no writer'". The writer exists and
the census could not have found it, because the aim point is not a field of the approach proper.

Packet `cc8_dive_aim` found the mechanism on the dive-bomb side (`docs/DIVE_BOMB_AIM_POINT.md`): a
**target-reference sub-object** (`009FB200` constructs it and installs vtable `00D21CB4`), refreshed
every tick by `009FADA0`, whose tail `009FAEDF CALL 004142E0` transforms a body-frame point by the
target's world matrix and stores the result to the sub-object's `+1Ch/+20h/+24h`
(`009FAEEA` / `009FAEF5` / `009FAF00`). The body-frame point is chosen by the TARGET through
`target->vtable[+100h]` (`009FA260`) and re-picked every 1.5-2.5 s.

The torpedo approach embeds the same sub-object at `approach+B4h`, verified from the bytes:

```
009d34ae  fld  dword ptr [ebp+8]        ; dt
009d34b1  push ecx
009d34b2  lea  ecx,[esi+0B4h]           ; this = approach+B4h
009d34b8  fstp dword ptr [esp]
009d34bb  call 009FADA0                 ; BSP_BotApproachTargetRef_Update
```

inside `009D3420 BSP_BotApproachTorpedo_Update`, before the `vtable[0]` call at `009D3517`.
`B4h + 1Ch = D0h`, so `009FADA0`'s three stores ARE the writes to `approach+D0h/+D4h/+D8h`; they
address the field as `[sub+1Ch]`, a disp8 displacement on a different base, which is why sixteen
encodings at two displacements found nothing. `tools/callsite_census.py 009fada0` lists eleven call
sites: the torpedo (`009D34BB`), the dive bomb (`009C7A9F`), the depth charge (`009A6043`) and six
more approach classes, plus two inside the sub-object's own code.

What this changes in this document, and what it does not:

* Section 3's "the writer is unfound" is **closed**. Section 3 itself listed `009FADA0` among the
  three calls `009D3420` makes before `009D3517` and set it aside because it "writes only small"
  offsets on its own `this`; that observation was correct and was the answer, read without its
  base.
* The no-lead conclusion is **unchanged and strengthened**: the writer has no velocity term and no
  time term either. The aim point is the target's live pose applied to a hull point.
* The host's `approach_target_point` (section 4) returns the target's ORIGIN. The image aims at a
  target-chosen HULL POINT. That is a host gap shared with the dive-bomb task, scoped as packet
  `cc8_hull_aim_point` in `docs/HANDOFF_DIVE_BOMB_AIM.md`; an offset of tens of metres on a
  180-270 m hull has the shape of section 7's unexplained +37.8 to +49.6 m under-prediction on the
  misses, which stays unmeasured until that packet binds the point.

## 11. Correction: the aim point DOES lead, through `sub+44h` "projtime" (packet `cc9_approach_target_lead`)

cc9-planes1, 2026-09-29 (stamped 17:10 UTC). This section retracts the title of this document, and
the "no lead" conclusions of section 10, `DIVE_BOMB_AIM_POINT.md` section 1 and the header of
`include/bsp/approach_target_ref.hpp`. All of them read `009FADA0` up to `009FAF00` and set aside
its tail as "off until something sets `sub+44h`". Two things set it.

**The tail, `009FAF05`-`009FAF7D`** (disk bytes; `009FADA0` runs to `009FAF88` exclusive, `RET 4`
at `009FAF85`, then INT3):

```
009faf05  movss  xmm0,[esi+44h]
009faf0a  comiss xmm0,[00D7A218]        ; 0.0
009faf11  jbe    009FAF80               ; projtime <= 0: no lead
009faf26  mov    ecx,[esi+18h]          ; the target
009faf29  fld    [esi+44h]
009faf2e  mov    edx,[edx+48h]          ; target->vtable[48h]
009faf31  push ecx / fstp [esp]         ; t = projtime
009faf39  push eax                      ; &tmp
009faf3a  call   edx                    ; __thiscall(out, t), RET 8
009faf3c  tmp - target+FCh/+100h/+104h  ; the predicted displacement
009faf62  sub+1Ch/+20h/+24h += it       ; added to the hull point just stored
```

**The name.** `009FB3E0` (the second constructor, four approach classes) registers the
sub-object's fields through `[arg]->vtable[10h]`: `+48h` "precision", `+54h` "bullpos", `+34h`
"error", **`+44h` "projtime"** (`009FB516 LEA EDX,[ESI+44h]`, name at `00D21C8C`), and "vehicle".

**The writers**, found by scanning the approach code for stores at the embedded displacement, the
scan the earlier censuses did not run:
- **Torpedo**, sub at `approach+B4h`, so `sub+44h = approach+F8h`. It is written at `009D3D2F`,
  `009D3D52` and `009D3D65`, and these are the three arms of the engagement estimate already
  reconstructed as `torpedo_engagement_eta_009d3c93` (TORPEDO_APPROACH_UPDATE): `clamp(+98h +
  2*max(+90h - speed, 0) / (unit speed + +70h), 0, 30)` seconds. The host keeps it as
  `torpedo_approach.eta_f8` and feeds it nowhere.
- **Dive bomb**, sub at `approach+30h`, so `sub+44h = approach+74h`. It is written at `009C7D65`
  (0) and `009C7E72`/`009C7E85`: `clamp(tf + approach+C8h, 0, 30)`, where `tf` is the
  `007BCC80(...) + 0.1` fall time of the predicted impact point, kept in the second argument slot
  `[ESP+38h]`. `approach+C8h` is a construct-time jitter, `009C3E8C` `Uniform(-row+2Ch,
  +row+2Ch)` on stream 1.

Both writes come after the approach's own `009FADA0` call (`009D34BB`, `009C7A9F`), so each tick
leads by the previous tick's estimate.

**`target->vtable[48h]`** is `008120E0` on all nine hull-sampling vtables. I read each from
`.rdata` at `vtable+48h`; the plane's `00D05F20` carries `00954650`. Its slot `+34h` is `00812090`
and its `+38h` is `0080E0F0`, on all nine. Read whole (`008120E0`-`0081230D`, `RET 8` twice):
- It takes a turning arc when `|00811890(unit+984h) x t| > 0.1` and speed `> 0.8333`. The chord is
  `s x t` long, along `heading(v) - A/2`, where `A = yaw x t` clamped to `+-1` rad.
- Otherwise it is straight: `pos + v x t`.

Reconstructed as `bsp::ship_predict_position_008120e0` and
`bsp::approach_target_ref_lead_tail_009faf05` (`src/approach_target_ref.cpp`), build-tested only.

**The size.** At the aim entry (about 2200 m) the estimate is tens of seconds, clamped at 30. A
Lexington at about 16 m/s (reference o's USN04: 3572 m in 225 s) moves 300-480 m in that time.
So the image's Kates aim well ahead of the carrier, and the host's aim at the hull point itself
is a lost lead of that size. Section 7's along-course under-prediction on the misses has the
same sign.

**What this does not change.** Section 1's bearing is still a pure `atan2` of (aim point - own
position); the lead lives in the aim point, not in the bearing.

### 11.1 The switch and its predictions, written before any ON run

`kTorpedoAimLeadBound`, to be committed OFF in `src/game_hosts_units.cpp`. The exact edit is
`local\p1_units_edit.txt` in the cc9-planes1 tree. It sits in
`TorpedoApproachBinding::approach_target_point`, after `hull_aim_world_point`. For a target that
samples its hull, it takes the previous tick's `torpedo_approach.eta_f8` as projtime and adds
`ship_predict_position_008120e0(target, projtime) - origin`.

It carries two labelled substitutions:
- `00812090`'s body axis is replaced by the hull's heading direction, the same substitution
  `neighbour_world_velocity` makes.
- The torpedo goaway tick (`009D0F10`) still reads the origin. Its own aim-point gap is not
  part of this switch.

Predictions, each against the same-tree OFF run:
- **Mechanism, on every torpedo row.** The `torpedo aim lead` lines show a projtime of 5-30 s at the
  2200 m entry, falling toward 1-5 s at release. The lead lies along the target's course and is
  roughly `projtime x speed` (Lexington about 16 m/s).
- **USN04 4500, and E2 (USN04 at 9200/9000).**
  - The Kates' aim and run-in tracks shift ahead of the carrier.
  - Releases: 6 of 16, +-3.
  - Deaths among the 16 Kates move by up to +-3, because the AA exposure changes with the
    geometry.
  - Aerial torpedo impacts on the ordered carriers do not fall.
  - Ships' own torpedo traces are unchanged except by RNG coupling.
- **JM05 9200/9000.** The torpedo planes are still shot down before release, so releases stay
  at or near 0 and the deaths move by at most a few.
- **Rows with no torpedo task:** identical apart from the known noise.

The verdict rule is the contract's. A mechanism failure (no lead lines, or a projtime outside
0-30) keeps the switch OFF. A spread miss while the mechanism matches may flip, recorded.

### 11.2 The dive-bomb side: the second switch, and its predictions (written before any ON run)

The pure writer is `bsp::dive_bomb_projtime_009c7e3c(impact_arm, tf, c8)` in
`src/approach_target_ref.cpp`, build-tested only.
- Outside the impact arm it stores 0 (`009C7D65`).
- Inside the impact arm it stores `clamp(approach+C8h + tf, 0, 30)`.

`tf` is `009C7D71`'s fall time: `007BCC80(...) + 0.1`, kept in the argument slot `[ESP+38h]`. The
host already carries it as `db_impact_fall_time`.

`approach+C8h` is `009C3DA0`'s `Uniform(-row+2Ch, +row+2Ch)` at `009C3E8C`. It is drawn at the
task seed and at every fly-over enter.
- **SUBSTITUTION, labelled:** the host draws none of `009C3DA0`, so the switch uses the draw's
  mean, 0. The row base `[approach+14h]` is unread (FOLLOWER_ATTACK_HANDOVER section 6).
- The two readings of `row+2Ch` are `torp_targetv_error_02c` and, under the `+0Ch` shift,
  `dive_bomb_calc_target_pos_error_038`. The name "calc target pos error" fits a jitter on the
  target-prediction time, which leans toward the shift.

The binding is `kDiveAimLeadBound`, committed OFF, in `local\p1_units_edit.txt` edits 4 and 6. It
carries three edits:
- The snapshot at the host's `009C7A9F`.
- The writer after the impact-point block.
- Every dive consumer of the fed aim point routed through `aim_point_009fada0`. Those are the
  approach update, the fly-over lead point, the aimdive and aimglide heights, the goaway turn and
  the aimdive tail.

Predictions, each against the same-tree OFF run with the torpedo switch at its verdict value:
- **Mechanism.**
  - The `aim lead` lines for dive bombers show a projtime of about 3-12 s. That is the fall time
    from the dive's release height, 0 before the dive and the range latch.
  - The lead lies along the target's course: roughly `tf x speed`, 50-200 m against a
    15-16 m/s ship and 0 against a stopped one.
- **USN01 3000.** ScoutDauntless keeps its 2 of 2 releases. The recorded aimglide lead window
  (`-117.4..-5.0 m`) sees the aim point move ahead by the lead, so the bomb's along-course miss
  against a moving target falls. That is DIVE_BOMB_AIM_POINT's "no-lead signature" closing.
- **USN04 4500 and E2.** Dive-bomb releases are 0 of 19 and 1 of 19 in reference o, held back by
  AA and the fly-over tolerance (ranking #1). They stay within +-2. Any release that happens
  scores a smaller along-course error.
- **Rows with no dive-bomb task:** identical apart from the known noise.

### 11.3 The torpedo pair, measured: `kTorpedoAimLeadBound` ON

cc9-planes1, 2026-09-29. Both builds are exports of `1dc0d4bf8`: the OFF build (`70E1BAABA0EB`)
flips nothing, and the ON build (`E28985CFC860`) flips `kTorpedoAimLeadBound=true`. The runs are
reference-o launches with `BSP_GUNNERY_RNG_STREAMS=1` and `BSP_DEATH_TABLE=1`, and a 300-frame
USN01 smoke ran first. The logs are `local\p1_{off,ton}_<row>.log` in the cc9-planes1 tree.

**Mechanism: holds.**
- The `aim lead` lines appear on every torpedo row: 39 samples on USN04, 121 on JM05 9000.
- Every projtime lies in `[0, 30]`. It sits at the 30 s cap while the range is long, then falls
  on the run-in: Kate #6.1|.-4 on Lexington reads 28.86, 23.48, 15.86, then Kate #6.1|.-2
  reads 4.99.
- The lead is `projtime x speed` along the target's course: 16.19 m/s x 15.86 s = 257 m, logged
  as (181.5, 181.9).
- The arc arm is rare, because `|yaw x t|` stays under 0.1 on these straight-running carriers.

**Outcome: moved in both directions.**

| row | releases OFF -> ON | aerial torpedo impacts | deaths | damage |
| --- | --- | --- | --- | --- |
| USN04 4500 | 6 of 16 -> 2 of 16 | 6 (Lexington 3, Yorktown 3) -> 2 (Lexington 2) | 46 -> 46, 42 rows changed | 15008.9 -> 12050.6 |
| E2 (USN04 9000) | 6 -> 2 of 16; dive 1 -> 0 of 19 | 6 -> 2 | 51 -> 51, 47 changed | 15802.4 -> 12797.5 |
| JM05 9000 | 3 of 24 -> 6 of 24 | Zuikaku 3 -> Shokaku 3 + Zuikaku 3 | 27 -> 30: Shokaku, Zuikaku and Japan Troop Transport 04 sink only ON | 29057.2 -> 31543.8 |
| USN13 3000 | 0 of 60 -> 4 of 60 | 0 -> 4 | 31 -> 31, 31 changed | 9130.0 -> 10941.9 |
| USN01 3000 | 0 of 5 both | - | 5 -> 5 | 2786.4 both |

**Why USN04 loses releases.** Every released torpedo hits, on both sides. The loss is in
survival. The per-entity death table shows each Kate dying 1-4 s EARLIER and FARTHER out:
- #4.1: 791 -> 1116 m;
- #2.1: 658 -> 822 m;
- two kills move from Lexington's guns to `Northampton-class02`.

Aiming 300-480 m ahead of the carrier moves the run-in off the carrier's own bearing and across
the escort screen. The Yorktown group (#4.x) is now shot down before release, so Yorktown takes
0 damage (was 1916).

**Against the predictions (11.1).** The mechanism prediction held. The spread predictions missed:
- USN04's releases fell by 4, outside +-3, and its aerial impacts fell with them.
- JM05 at 9000 released 3 more instead of staying near 0. The prediction came from the
  3000-frame row; at 9000 frames there are releases on both sides.

The contract's rule is that a spread miss with the mechanism holding may flip, recorded. The
listing is unambiguous that the image leads (section 11). **The switch goes ON.** The
AA-lethality finding (ranking #1) now applies to a strike that aims ahead, as the image's does.

### 11.4 The dive pair, measured: `kDiveAimLeadBound` ON

The OFF side is the torpedo-ON export (`E28985CFC860`, logs `local\p1_ton_<row>.log`). The ON side
flips both switches (`E9AB45EF5DA9`, `local\p1_don_<row>.log`).

**Mechanism: holds.** The dive bombers' `aim lead` lines show projtime falling through the dive:
- USN01 ScoutDauntless on Convoy1 at 13.74 m/s: 13.24, 10.35, 7.76, then 4.44 s near release.
- USN04 Vals: 15.24 to 12.55 s.

The lead is along the course, 58-183 m. At release the projtime equals the bomb's 4.3-4.5 s fall
time, as `tf + 0` should. Prediction 11.2's projtime band of 3-12 s was slightly low at its top
(15.2 s early in the dive), because the fall time is long from the dive's entry height.

**Outcome.**

| row | dive releases | deaths | other |
| --- | --- | --- | --- |
| USN01 3000 | 2 of 2 -> 2 of 2 | 5 -> 5, death table identical | Both bombs still miss. "vs target at release" rises 62.2 -> 114.1 m and 77.0 -> 119.7 m, by about the lead. At release the bomb is now aimed where the convoy will be after the fall, not where it is. The impact-time target position is not logged, so the along-course closure is inferred, not measured. |
| USN04 4500 | 0 -> 0 of 19 | 46 -> 46; D3A Val #7.1 survives, D3A Val #5.1\|.-2 dies | torpedo releases 2 -> 1 (the AA picture moves with the Vals' tracks) |
| E2 | 0 -> 1 of 19 | 51 -> 51 | torpedo 2 -> 1 |
| JM05 9000 | 0 -> 0 | 30 -> 30, 16 rows changed | - |
| USN13 3000 | - | identical | no dive task: GAMEPLAY identical |

The prediction's spread held: releases within +-2, USN01 keeps 2 of 2, and rows without a dive
task are identical. **The switch goes ON.**

Open, recorded:
- `approach+C8h` is substituted by its mean, 0.
- A re-target zeroes projtime for one tick.
- The torpedo goaway tick (`009D0F10`) still reads the target's origin.
- The lead uses the hull's heading for `00812090`'s body axis.

## 12. The aim-error redraw: `009C3DA0` and `009D02A0` (packet `cc9_aim_error_draw`)

cc9-planes1, 2026-09-29.

**The row base is settled.** `009F9D0E`-`009F9D22` in the base approach constructor `009F9CE0` do
`approach+14h = [00F8A30C] + [[unit+DF4h]+34h] * 248h + 0Ch`. So `[approach+14h]+N` is the
`robot_config.hpp` field with suffix `N+0Ch`. That settles FOLLOWER_ATTACK_HANDOVER section 6's
open contract in favour of the shifted reading.
- The dive draws `row+30h`/`+34h`/`+2Ch`, which are `dive_bomb_targeth_error_03c`,
  `_targetv_error_040` and `_calc_target_pos_error_038`, not torpedo fields.
- The same shift already names `approach+A8h`'s `row+38h`/`+3Ch` as `dive_bomb_release_alt_1_044`
  and `_2_048`.

**The two bodies**, read whole from disk bytes, are in the `approach_target_ref.hpp` block
"The aim-error redraw". Each draws, on stream 1:
- `bias = (U(+-HError), 0, U(+-VError))` into `sub+34h` through `009FA380`;
- the hull-point spread `sub+48h..50h = TargetPointSelectPrec` on all three axes, with the dirty
  byte set;
- (dive only) the named-section chance and weights `sub+64h..70h`;
- a time error: the dive's `approach+C8h`, which feeds projtime at `009C7E3C`, and the torpedo's
  `approach+9Ch`, the run-time bias `009D1360` adds into `+98h` and so into the engagement
  estimate.

This installation's robots.lua comments name each one. TargetHError and TargetVError are the miss
across and along the target's long axis. CalcTargetPosError is "time to impact plus random this
much: where the target will be then, it sends it there". That is the projtime jitter, and it
confirms section 11's reading from the data side.

**Callers:**
- dive: `009C4083`, the approach constructor's tail call, and `009C6276`, every fly-over enter;
- torpedo: `009D062B`, the reset's tail, and `009D15D6`, every aim enter.

**The rows** are this installation's robots.lua PilotBot blocks (mtime 2025-06-01),
`kTorpedoAimErrorRows` and `kDiveBombAimErrorRows`. The strike squadrons on USN04 and JM05 launch at
skill 2, SPVeteran (the `air ops launch skill` lines):
- torpedo errors 0 / 0 / +-1 s, spread 0.25;
- dive errors 0 / 0 / 0, spread 0.5, section chance 1.0.

SPNormal (level 1, the default) is torpedo 10 / 16 / +-10 s, spread 0.9, and dive 10 / 5 / +-10 s,
spread 0.8, section chance 0.

**Substitutions, labelled:**
- The draws use the `release_altitude_draw_00bd2f10` stream-1 stand-in, keyed per unit under
  `BSP_GUNNERY_RNG_STREAMS=1`.
- The ship's named-section points (`ship+A68h..A94h`) are not carried by this host, so a positive
  section chance still falls through to the hull box (00816650's own fallback when no section is
  available). Skill 2 dive bombers would aim at engine rooms, magazines and fuel tanks in the
  image. That remains open.

### 12.1 Predictions for `kAimErrorDrawBound`, written before any ON run

The OFF side is `db5276810` plus this packet with the switch OFF. The ON side flips it.

- **Mechanism.** The `aim error draw` lines appear once per torpedo reset and aim enter, and once
  per dive seed and fly-over enter. They show:
  - USN04, E2 and JM05 skill-2 attackers: bias 0 and time within +-1 s (torpedo) or exactly 0
    (dive), spread 0.25 or 0.5;
  - a skill-1 attacker: bias up to +-10/16 m and time up to +-10 s.
- **USN04 4500 and E2.** The Kates' hull point pulls toward the hull centre (spread 0.9 -> 0.25),
  and the eta moves by at most 1 s, so the lead moves by at most 16 m.
  - Torpedo releases 2 of 16, +-2.
  - Every drop still hits.
  - Dive releases 0 of 19, +-2.
  - Deaths 46 (51 on E2), +-3, with the per-entity rows moving through the AA picture.
- **JM05 9000.**
  - The skill-1 airfield squadrons draw real errors: torpedo +-10 s on the eta, which is a lead
    change of up to about 100 m against 10 m/s carriers.
  - The skill-2 carrier squadrons change as above.
  - Torpedo releases 6 of 24, +-3.
  - The Shokaku and Zuikaku sinkings may flip, because a +-10 s eta error can turn a hit into a
    miss.
- **USN01 3000.** ScoutDauntless is the player's own unit and is not in the launch-skill lines, so
  it takes level 1: bias +-10 m across and +-5 m along, projtime +-10 s, spread 0.8.
  - A +-10 s jitter moves the lead by up to +-137 m at the convoy's 13.74 m/s. Both bombs'
    "vs target at release" distances therefore move by tens to about 140 m.
  - 2 of 2 releases are kept.
  - Hit or miss: still expected to miss, at under 1-in-3 odds of a hit.
- **USN13:** GAMEPLAY moves only where a torpedo task runs (4 of 60 releases on the ON side of
  11.3); +-2.

### 12.2 The pair, measured: `kAimErrorDrawBound` ON

cc9-planes1, 2026-09-29. Both sides are exports of `ec176d648`: OFF `DB644D545010`, ON `F179E3072D4B`.
The logs are `local\p1_{off,ton}_<row>.log`, and a 300-frame USN01 smoke ran first.

**Mechanism: holds, with one correction to the prediction.** The `aim error draw` lines appear on
every attacking row.
- The JM05 carrier squadrons draw the skill-2 torpedo row: bias 0, spread 0.25, time within +-1 s
  (-0.06, 0.62, 0.83, -0.67).
- USN04's dive bombers (`movieval`, the Vals) draw the SPNormal dive row, spread 0.80 with
  errors up to 6.3 m and 6.6 s, not the skill-2 row the prediction assumed. Their
  `db_skill_row_14` is level 1. The launch-skill lines name the torpedo squadrons, not these.
- USN01's torpedo bombers `Mav1`-`Mav4` draw SPNormal: bias up to 11 m, time up to +-9.5 s.

| row | OFF -> ON | predicted | verdict |
| --- | --- | --- | --- |
| USN04 4500 | torpedo releases 1 -> 3 of 16, impacts 1 -> 3 (every drop hits); deaths 46 -> 45 (D3A Val #5.1\|.-3 survives); dive 0 -> 0 | releases 2 +-2, deaths +-3 | held |
| E2 | torpedo 1 -> 3, dive 1 -> 2 of 19; deaths 51 -> 51 | +-2 | held |
| JM05 9000 | torpedo releases 6 -> 8; Shokaku 3 -> 6 impacts, Zuikaku 3 -> 2; deaths 30 -> 29 (Kuma-class 01 survives); both carriers sink on both sides | 6 +-3, sinkings may flip | held; no flip |
| USN01 3000 | ScoutDauntless 2 of 2 on both sides; "vs target at release" 114.1 -> 84.9 m and 119.7 -> 56.8 m; both miss | 2 of 2, move up to about 140 m, still miss | held |
| USN13 3000 | torpedo 4 -> 3 of 60 | +-2 | held |

The OFF column here is `db5276810`'s ON side, and it moved against 11.3 and 11.4 (for example
USN04's torpedo releases 2 -> 1). That comes from lua18's gun-controller change merged in
between, not from this switch. **The switch goes ON.**

## 13. Handoff queue (cc9-planes1, about 75% context)

1. **Ship section points for the dive bombers (next).**
   - `00816650`'s named-section path needs `ship+A68h..A94h`, the engine room, magazine and fuel
     tank points with their flag bytes. It also needs `0093A570`'s section-id vector at
     `ship+A20h`.
   - Skill 2-5 rows carry a section chance of 0.5-1.0 (`kDiveBombAimErrorRows`), so in the image
     most veteran dive bombers aim at a section, not a random hull point.
   - The units host carries none of it. `approach_target_ref_pick_009fa260` passes empty
     sections, so the chance falls through to the box.
   - Find the writers of `ship+A68h`/`+A78h`/`+A88h` (the ship constructor or its section
     loader), bind the points behind a switch OFF, and pair on a row with skill-2 dive bombers.
2. **The torpedo goaway aim** (`009D0F10`): it still reads the target's origin, not
   `aim_point_009fada0`.
3. **`009D0160`**, the torpedo reset's own `+98h` seed from `+9Ch`. It is unmodelled; the host
   starts `+98h` at 0 until `009D1360` runs.
4. **Recorded substitutions:**
   - the stream-1 draws use `release_altitude_draw_00bd2f10`'s stand-in;
   - a re-target zeroes projtime for one tick;
   - the lead uses the hull's heading in place of `00812090`'s body axis.

## 14. The ship section points in the pick (packet `cc9_ship_section_points`, cc9-planes2)

### 14.1 What the image does

- **The pick's callee reads the target.** `009FA2A0` calls `target->vtable[+100h]`, which is
  `00816650` for the nine hull-sampling ship vtables. `this` is the TARGET ship, so the three
  section records it reads are the target's own:
  - engine room: point `ship+A88h`, byte `+A94h`, id 5;
  - magazine: point `+A68h`, byte `+A74h`, id 8;
  - fuel tank: point `+A78h`, byte `+A84h`, id 6.
- **The producer is the ship loader `0081F980`.** It is already read in
  docs/SURFACE_GUNNERY_REFERENCE.md 8.1 (packet `cc9_hull_sections`).
  - It clears the three bytes, then walks the model instance's GeomMesh elements.
  - Kind 8 goes to `+A68h` (`00820566`), kind 5 to `+A88h` (`008205D7`) and kind 6 to `+A78h`
    (`00820648`). The last element of a kind wins.
  - The point is `00723030(element)`, the midpoint of the element's root box, in the model's
    frame. The pick then adds the bias `sub+34h` (`009FA2B3`) and `009FAEEA` poses the sum, as it
    does for a box point.
- **The id vector is the repair task's failure list.** `ship+A20h` is the damage-control task
  (docs/UNIT_FIRE_AND_REPAIR.md). `0093A570` scans its `10h`-byte records `[task+18h, task+1Ch)` for
  `+00h == id`, which closes the open question in docs/GUN_BOT_REMAINDER.md section 10.
  - `0093BED0` pushes `{hit+30h, name, duration}`, and `hit+30h` is the element kind the hull trace
    hit. So a section drops out of the draw while its failure is active: EngineJam for 20 s,
    Explosion (the magazine) for its 240 s cooldown, Fire (the fuel tank) for 15 s
    (docs/COMPONENT_FAILURES.md 1).
  - `0093C300`'s random pick pushes id `-2` instead, and the Lua `EngineJam` binding `0093BD80`
    pushes id 5.
- **Only dive rows reach it.** `009C3DA0` stores the dive row's chance and weights into
  `sub+64h..+70h` (`009C3E56`..`009C3E6B`). Torpedo rows leave the constructor's -1.0.
  - Skill 1 (SPNormal) has chance 0.0 and never rolls.
  - SPVeteran and Elite have chance 1.0 with weights 0.5 / 1.0 / 1.0. That gives the engine
    room 20 %, the magazine 40 % and the fuel tank 40 %.
- **Why a section matters for damage.** A bomb whose segment reaches a section element's triangles
  carries that kind in `hit+30h`. `0093BED0` then rolls `p = damage / 100` (FailureChance 100,
  threshold 100), so any bomb of 100 damage or more starts that section's failure:
  - magazine: Explosion, 35 % of health (`0083E379`);
  - fuel tank: Fire, 400 points over 10 s;
  - engine room: EngineJam.

### 14.2 The binding: `kApproachSectionPointsBound` (src/game_hosts_units.cpp), committed OFF

- `bsp::ship_section_points_0081f980` (src/approach_target_ref.cpp) builds the records from the
  class's `Mesh` model. It runs once per class, as the gunnery host's copy does.
- `approach_target_sections` adds 0093A570's answer per section. It only runs when the shooter's
  `sub+64h` is positive.
- `approach_target_ref_pick_009fa260` passes both to `00816650`. The pick draw is the stand-in's
  `unit[0] * (total - 1e-4)`.
- **Labelled substitutions:**
  - The element root box is taken as its triangles' bounding box in model space, as in 8.1.
  - `0093A570` is answered by `GameGunneryHost::unit_failure_active` under the kinds' Failures
    names (EngineJam / Explosion / Fire), not by id. This is the same answer while `0093BED0` is
    the host's only producer of those names; `0093C300` is not called by the game host.
  - The pick draw comes from the hash stand-in (section 12's stream-1 substitution), not from a
    second draw on the image's stream.
- **Census:** a `section points class` line per class, up to 40 `section aim` lines, and the
  `summary mission approach section points` line.

### 14.3 Predictions (written before any run)

The rows come from cc9-planes1's 12.2 logs.
- USN04 and E2: every dive draw is SPNormal, chance 0.0, and every torpedo draw is -1.0.
- JM05: the 13 skill-2 dive draws (the Lexington `sqn09`, Yorktown `sqn10` and `sqn12`
  squadrons) have chance 1.0. JM05's dive task released 0 bombs of 33 aircraft on that side.

| row | prediction |
| --- | --- |
| USN04 4700/4500 | identical, exit 0 or 1: no pick has a positive chance; the summary line reads 0 picks and 0 classes |
| E2 9200/9000 | identical, exit 0 or 1, for the same reason |
| JM05 9200/9000 | section picks near 13 per aim draw, split about 20/40/40, no unavailable section; the aim points move, so plane paths move. Dive releases stay 0, and deaths move by at most 3 through the shared RNG stream (memory: shared RNG couples pairs). A carrier sinking may flip. |

**Mechanism test:** the `section aim` lines name skill-2 dive bombers against ship targets with a
body point equal to the class's section point plus the bias (0 for SPVeteran). **Verdict rule:**
if USN04 or E2 move, or JM05 shows no section picks, the mechanism failed and the switch stays OFF.

### 14.4 The pair, measured: `kApproachSectionPointsBound` ON

cc9-planes2, 2026-09-29. Both sides are exports of `fc93ff307`: OFF `BB7A09434752`, ON
`C1DCB3AF7C34`. The logs are `local\p2_{off,on}_<row>.log`, launched through
`local\p2_runs.ps1` in o's launch form with `BSP_GUNNERY_RNG_STREAMS=1` and `BSP_DEATH_TABLE=1`.
A 300-frame USN01 smoke of the ON binary ran first (exit 0, final COM release). Every log shows
`present interval immediate`, its own module directory and the final COM release.

**Mechanism: holds.**
- JM05 builds one class, 257 `models/ships/japan/Zuikaku.mmod`, which both carriers use. The
  engine room is at (0.4, 8.8, -24.0), the magazine at (1.6, 9.0, 69.7) and the fuel tank at
  (0.1, 8.6, -79.7), in model space: magazine forward, fuel tank aft, engine room amidships.
- The picks: `engine_room=66 magazine=111 fuel_tank=133` (21 / 36 / 43 %, against a predicted
  20 / 40 / 40), `chance_to_box=0` and `with_unavailable=0`. The 310 picks are every re-pick of the
  Lexington `sqn09` and Yorktown `sqn10` / `sqn12` Dauntlesses against Shokaku and Zuikaku.
- Each `section aim` line's body point is the section point exactly, because SPVeteran's bias
  is 0.
- `0093A570` never removed a section in this row: no failure of those three kinds was active
  at a pick. That arm stays unexercised.

| row | `pair_diff` | measured | predicted | verdict |
| --- | --- | --- | --- | --- |
| USN04 4700/4500 | exit 1 | gameplay identical; only the summary line's `bound=` differs; 0 picks, 0 classes | identical | held |
| E2 9200/9000 | exit 1 | the same | identical | held |
| JM05 9200/9000 | exit 3 | deaths 29 -> 29, with 0 only-ON and 0 only-OFF rows and 14 rows moved in time or killer (the US aircraft over the carriers, 246-275 s); dive releases 0 -> 0 of 33; torpedo 8 -> 8 of 18; hull hits 776 -> 776; shots 8476 -> 8133; damage 32230 -> 32265; Shokaku sinks at 257.61 -> 257.51 s, Zuikaku at 279.56 -> 279.16 s | releases 0, deaths +-3, a sinking may flip | held; no flip |

The JM05 movement is the Dauntlesses' new aim points: their paths change, and so does the AA
fire at them, which draws on the shared stream. None of their bombs is released on either side,
so no section hit is measured yet. **The switch goes ON.**

## 15. The torpedo goaway's break-off point (packet `cc9_torpedo_goaway_aim`, cc9-planes2)

### 15.1 What the image does

- **The goaway tick reads the stored aim point.** Each arm tick `009D4850` runs the approach update
  `009D3420` first (`009D486F`), then the state tick through `state->vtable[0Ch]` (`009D48F6`).
  - `009D3420` calls `009FADA0` on `approach+B4h` at `009D34BB` every time it runs
    (docs/TORPEDO_APPROACH_UPDATE.md, the host table).
  - That call leaves the hull point plus the lead in `sub+1Ch`, which is `approach+D0h`.
- **The goaway reads that point through the approach's slot 0.** The goaway tick `009D0F10`
  calls the geometry `009D0C10` at `009D0F46`, and `009D0C10` calls `approach->vtable[0]`
  (`009D0C46`-`009D0C4F`). That slot is `009D0670 BSP_BotApproachTorpedo_GetAimPoint`, 31 bytes,
  which copies `approach+D0h..D8h` into the return buffer.
- **So the fly-to solver `009FD570` is fed this tick's aim point**, the same point the approach
  states steer at. It is not the target's origin. The standoff and the side then turn that point
  into the break-off bearing (docs/TORPEDO_FLY_TO_SOLVER.md 3).
- **The projtime of that lead.** It is `approach+F8h` (section 11), read before this tick's
  estimate is written. It keeps being refreshed after the drop, because the approach update runs
  on every arm tick.

### 15.2 The binding: `kTorpedoGoAwayAimPointBound`, committed OFF

`run_goaway_tick_009d0f10` asks `aim_point_009fada0` for the solver's `point`, exactly as
`TorpedoApproachBinding::approach_target_point` does for `009D3517` / `009D36E4` / `009D3DC8`.
The host recomputes the point at each consumer rather than storing it once per tick. The inputs
are the same within one tick, so the answer is the same.

Census:
- a `goaway aim` line every 200th tick;
- `summary mission torpedo goaway aim point`, giving the ticks and the mean and max XZ offset
  from the origin.

### 15.3 Predictions (written before any run)

The goaway only starts after the drop, so no torpedo release or hit can change directly. What
moves is the climb-away bearing. Through the shared RNG stream, that moves the AA fire at the
bombers and whatever it kills.

| row | prediction |
| --- | --- |
| USN01 3200/3000 | The Mav torpedo bombers, each with a goaway lasting several hundred ticks. The point sits 10-200 m off the origin: the hull offset (up to half the length) plus the lead at the target's speed. Goaway ticks on every Mav. Torpedo releases the same on both sides. Deaths +-2; the surviving Mavs may flip. |
| USN04 4700/4500 | Torpedo goaways for the 1-3 droppers; releases unchanged +-2, deaths +-3 |
| JM05 9200/9000 | About 8 goaways; releases 8 +-2; deaths +-3; a carrier sinking may flip only through the RNG coupling |

**Mechanism test:** `ticks` > 0 on every row with a drop, and a positive `mean_off_origin`.
**Verdict rule:** keep OFF only if the census shows the point is not reached, or if a row
without torpedo goaways moves.

### 15.4 The pair, measured: `kTorpedoGoAwayAimPointBound` ON

cc9-planes2, 2026-09-29. Both sides are exports of `03e2e91e8` (main `31de7f88a` merged): OFF
`893038836610`, ON `2A02089C4FFE`. The logs are `local\p2_{off,on}_<row>.log`, in the same launch
form as 14.4. A 300-frame USN01 smoke of the ON binary ran first (exit 0, final COM release).

**Mechanism: reached, and inert on these rows.**
- The goaway takes 009D0670's point on USN04 (112 ticks, mean 65.3 m and max 77.3 m off the
  origin) and on JM05 (268 ticks, 52.0 / 79.7 m).
- Every sampled `goaway aim` line shows projtime 0.00. `approach+F8h` is zero once the torpedo is
  gone, so the offset is the hull point alone, with no lead.
- The point only reaches gameplay through `state+18h`, the break-off heading. **No arm consumed
  it**: `heading_ticks=0` on every torpedo aircraft of all three rows.
  - The aircraft ran only the window's high arm and the post-window low arm. The host's own
    `heading_ticks` counter shows that neither commands a heading.
  - The heading arms (`cmd+2CCh = 2`, docs/TORPEDO_AFTER_THE_DROP.md 4) never ran.
- The side `state+2Ch` could also move, but only through the solver's avoid term, and this host
  passes an empty obstacle list.

| row | `pair_diff` | measured | predicted | verdict |
| --- | --- | --- | --- | --- |
| USN01 3200/3000 | exit 1 | gameplay identical, death rows identical (5); **0 goaway ticks** on every aircraft | the Mavs' goaways, deaths +-2 | missed: no torpedo aircraft reaches the goaway on this row now |
| USN04 4700/4500 | exit 1 | gameplay identical, death rows identical (45); 112 goaway ticks, heading_ticks 0 | releases +-2, deaths +-3 | held on the mechanism; smaller than predicted, because no heading is consumed |
| JM05 9200/9000 | exit 1 | gameplay identical, death rows identical (29); 268 goaway ticks, heading_ticks 0 | releases 8 +-2, deaths +-3 | the same |

The prediction assumed the break-off heading was consumed; it is not, on any of these rows. The
mechanism matches the image (the point is 009D0670's), and nothing moved on rows without a goaway.
**The switch goes ON**, recorded as gameplay-inert until an aircraft reaches a heading arm.

### 15.5 Why no USN01 Mavis reaches the goaway any more (cc9-planes2, lead's request)

docs/TORPEDO_AFTER_THE_DROP.md section 4 had all five Mavis in the goaway, three of them still
alive at the end of the run. On main `32eeb5204`, none of them reaches it (15.4). **This is not a
plane-switch regression. The Mavis die before they are low enough to release, and a dead
aircraft's bot no longer thinks.**

**The history, from the reference logs already on disk.** Each figure below is the per-aircraft
`goaway 009D0F10: ticks=` sum.
- `cc8-gunnery-host/local/rb_usn01.log`, near section 4's time: 399 / 250 / 251 / 723 / 719.
- The AA-targeting passes (`rb2`, `rbB`, `rbF`) and `rb3`-`rb9`: two to four Mavis, each with
  7-322 ticks. The AA chain was being bound over these passes, and the Mavis die sooner with
  each one.
- `rb10` (reference j): 22 ticks on one Mav. Reference j attributes USN01's whole move to
  `kAiGroupSeedPerEntityBound`. Its leave-one-out `rb10ngs_usn01.log` restores rb9's
  62 / 41 / 43.
- `rb11` (reference k) onward: 0. Reference k attributes USN01's lost releases to
  `kDeadPlaneBotThinkBound`, and its leave-one-out `rb11ndpb_usn01.log` restores 46 ticks, on a
  dead Mav. docs/PLANE_DEATH_MODES.md 7.5 showed that the lost releases were dead planes'.

**The bisect on current main (by switch, exports of `32eeb5204`).**

| export | Mav deaths (s) | goaway ticks | lowest altitude in the aim state | torpedo releases |
| --- | --- | --- | --- | --- |
| main as is (`p2_off_usn01`, 15.4's OFF side) | 70.85-79.60 | 0 on all five | 69-194 m | 0 |
| `kAiGroupSeedPerEntityBound` and `kDeadPlaneBotThinkBound` both OFF (`08C161EE2519`, `p2_bis2_usn01`) | 63.55-88.20 | Mav2 57, Mav3 110, Mav4 13 | 14-160 m | 1, by a dead plane (`live=0 dead=1`) |
| the seed OFF alone (`p2_seed`) | not run: the renderer failed at CreateDevice, hr `0x8876086a`, with `logonui=1` (a locked session, an environment fault) | | | |

- **With both switches OFF, every goaway belongs to a dead aircraft.** No Mav is alive at the
  release altitude, and the one release is counted as a dead plane's.
- So the goaways that return are the ghost states that `0099ACD0` suppresses in the image. That
  `kDeadPlaneBotThinkBound` removes them is correct, from its listing reading in
  docs/PLANE_DEATH_MODES.md 7.1.
- **What really moved is how early the ship AA kills the Mavis:** now 63-88 s, where section 4 had
  129.7 s and 134.9 s, with three survivors.
  - That is the product of the AA chain's bindings (targeting, the group seed and gunnery),
    each paired and accepted on its own evidence.
  - Whether the image's AA is this lethal to a Mavis at 700 m or more cannot be settled
    statically. It is a question for a recorded original run, not for a plane switch.
- **Recorded, not a defect in my lane.** The goaway, its aim point (15.4) and the dead-plane
  suppression all match the image. USN01 has stopped being a goaway row. The rows that still
  exercise the goaway are USN04 and JM05.

## 16. The fly-to solver's obstacle list is the enemy AA envelopes (packet `cc9_fly_to_obstacles`, read)

cc9-planes2, read-only while the session was locked. The edit is prepared as
`local\p2_edit_obstacles.py` in the cc9-planes2 tree, for sequencing on src/game_hosts_units.cpp.

**The rebuild, `009FD5FF`-`009FD743`, read from the raw listing.**
- The cache is the calling state. `009FD5FF ADD [EDI+8],-1` / `JNS 009FD7D9` reuses the list.
  Otherwise `MOV [EDI+8],14h` (`009FD630`) and `007B4500(0)` clear it.
  - So the walk runs on the first call and then every 21st call.
  - The dive goaway's state starts with `+8h` = 0 (`009C74E8`, EBX = 0).
- **The walk.** `[[00E188A8]+19CCh]+64h` is world list 6, the ships, with `{+4h next, +8h unit}`
  nodes.
- **The team test.** `EBX` is the flyer's `unit+54h`, loaded at `009FD5E3`. `009FD656 CMP
  [ESI+54h],EBX / JE` skips its own team.
- **The range test** (`009FD66F`-`009FD6F9`). The distance to the lead point is taken in XZ,
  with `FLDZ / FMUL ST0` for y. The radius is `max(+448h, +444h)`, then that against `+434h`,
  plus 100.0 (`00D7A220`, `FADD qword`). The unit is kept when radius^2 > distance^2.
- **What those fields are** (docs/GUNNERY_TABLES.md, `00956C20` step 11):
  - `unit+430h + cat*4` is the longest live gun range of category `cat`, floored at 10.
  - `unit+460h + cat*4` is the sum over that category's guns of (DamageMin + DamageMax) / 2
    (`00956E94`..`00956E9B`).
  - The categories read are 1 AAMACHINEGUN (`+434h` / `+464h`), 5 FLAK (`+444h` / `+474h`) and
    6 LIGHTARTILLERYFLAK (`+448h` / `+478h`).
  - So each obstacle is an **enemy ship's AA envelope**: its radius is the longest AA range plus
    100, and its weight is the AA damage sum. The solver pushes the steer away from it
    (docs/TORPEDO_FLY_TO_SOLVER.md 4).
  - `include/bsp/plane_fly_to_solver.hpp` calls the two fields `extent_max` and `extent_sum`.
    They are AA range and AA damage, not geometric extents.
- **Where it matters.** Two goaways feed the solver, and so both steer round enemy AA:
  - the torpedo goaway (`009D0C54`), whose heading no aircraft consumed in 15.4;
  - the dive-bomb goaway (`009C4810`), which does command its heading (the USN04 Vals log 212
    heading ticks).
  The host passes an empty list at both sites.

**The prepared binding (`kFlyToObstacleListBound`, to be committed OFF).**
- One cache per goaway state on the unit slot, with a countdown starting at 0. It walks
  `world_lists.entries[6]` with `row.party` as `+54h`.
- The ranges come from the gunnery host's `category_ranges`. The damage sums are rebuilt from its
  gun rows and bullet classes.
- It adds a summary census line.
- **Labelled substitutions:**
  - It omits `00956C20`'s second ammunition record for a kind-6 gun in category 6 (`+48h`).
  - It takes every gun with a bullet class as live (`desc+78h > 0` is unread here).
  - The cache is not reset when a task is rebuilt.
- **Uncertain:** whether list 6 drops a sunk ship. The rebuild itself does not test for death.

**Predictions** (to be committed with the binding):
- The dive-bomb goaway turns change on every row with dive bombers near enemy ships (USN04, E2).
  The side can flip (`side_writes` > 0).
- The torpedo goaway stays inert until a heading arm runs.
- Deaths move by up to 3 through the shared stream.

### 16.1 The pair, measured: `kFlyToObstacleListBound` ON (cc9-planes3)

**Binaries:** `pair_export.py --commit dad065393`, OFF `local\p3_off` (SHA-256 prefix
`87BED43FA2AE`) and ON `local\p3_obs_on` (`--flip kFlyToObstacleListBound=true`). Rows were run in
14.4's launch form with `BSP_GUNNERY_RNG_STREAMS=1` and `BSP_DEATH_TABLE=1`. A USN01 300-frame
smoke passed first, at 20:35 UTC, after the renderer-init failures had cleared. Every log has
`present interval immediate`, `frames_presented` equal to the frame count minus one, and the
final COM release.

**Mechanism census** (the same on USN04 and E2, because E2's first 4500 frames are USN04's):

| cache | rebuilds | kept | avoid_ticks | side_writes |
| --- | --- | --- | --- | --- |
| dive goaway | 28 | 139 | 510 | 0 |
| torpedo goaway | 11 | 76 | 168 | 0 |

**Results per row:**

| row | pair_diff | deaths OFF -> ON | per-entity death rows | torpedo-task releases | dive-bomb-task releases | shots |
| --- | --- | --- | --- | --- | --- | --- |
| USN04 4700/4500 | 3 | 45 -> 45 | 1 only-OFF (Val #7.1\|.-4), 1 only-ON (Val #5.1\|.-3), 21 re-timed | 3 -> 5 of 16 | 0 -> 1 of 19 | 8656 -> 9619 |
| E2 9200/9000 | 3 | 51 -> 51 | none only one side, 28 re-timed | 3 -> 5 of 16 | 2 -> 1 of 19 | 9450 -> 10265 |
| JM08 3200/3000 (control) | 1 | 10 -> 10 | identical | - | - | 2867 -> 2867 |
| USN12 3200/3000 (control) | 1 | identical | identical | - | - | identical |

**Against the predictions:**
- **The Vals' goaway turns change: held.** 139 obstacles were kept and the avoid term ran on 510
  dive-goaway ticks. The first death row to move is a Zero at 161 s.
- **Side flips: missed.** `side_writes` is 0. The avoid term ran without ever writing the side at
  `009FDC48`. This is a spread miss: the mechanism ran.
- **The torpedo goaway stays inert: held.** Every `goaway 009D0F10` line still has
  `heading_ticks=0`. The ON torpedo goaway ran more ticks (goaway aim ticks 112 -> 168) because two
  more Kates released and reached it.
  - Those two extra releases come through coupling: the Vals now fly different paths, so the AA
    fire and the Zeros' fights move with them. This is not a torpedo-side mechanism. The AA RNG streams are decoupled, but
    the targets and ranges are not.
- **Deaths move by up to 3: held.** Both aggregates are equal. On USN04, one Val death moved from
  one Val to another; E2 has no one-sided row.
- **Rows with no goaway do not move: held.** JM08 and USN12 are gameplay-identical (exit 1).

**Verdict:** the mechanism matched, the only miss was the side-flip spread, and no control row
moved, so the switch **flips ON**. The flip is one line in src/game_hosts_units.cpp, applied under
the shared-file rule. Logs: `local\p3_{off,obs_on}_{usn04,e2,jm08,usn12}.log`; diffs:
`local\p3_diff_obs_*.txt` in the cc9-planes3 tree.

## 17. `009D0160`, the reset's run-time seed, and two reset fields the host is missing

**`009D0160`** has no Ghidra function. Its body is `009D0160`-`009D0292`: `RET` at `009D0291`,
then `INT3`. It is `__thiscall(approach)`, and its only caller is `009D0632` in
`009D0380 BSP_BotApproachTorpedo_Reset`, right after the aim-error draw `009D02A0`
(`009D062B`).
- It is `009D1360`'s run-time arithmetic with different inputs, traced through the x87 stack:

```
fall  = 007BCC80(unit, +78h + +74h)          fall time from the planned release altitude
lead  = fall * +70h                           (009D01DA-009D01E4)
dt    = (+70h - run) / 80.0                   run = 007BCFA0(unit); 00CF1440 qword; NO floor
dd    = dt * (+70h + run) * 0.5               00D7A280 qword
L     = +7Ch - (lead + dd)                    009D01EC-009D01F6: FADDP then FSUBR [ESI+7Ch]
t     = L < 0       ? fall
      : dd <= L     ? fall + dt + (L - dd) / run
      :               fall + dt * L / dd
+98h  = max(0, t + +9Ch)                      009D0255-009D0285
```

- It differs from `009D1360` (`torpedo_run_time_009d1360`) in four places:
  - it uses `+70h` in place of the unit speed;
  - its fall height is `+74h + +78h`, not the unit altitude;
  - its leg is `+7Ch` (TorpReleaseDistNear) rather than `min(range, +7Ch)`;
  - it has no floor on `dt`.
- `dd` is subtracted from the leg twice (in `L`, then in `L - dd`). That is taken as the image's
  own arithmetic, not a misreading. It is worth a second reader, because it is x87 stack work.
- docs/TORPEDO_AFTER_THE_DROP.md section 5's census names only `009D14E7` as a `+98h` store. It
  missed `009D0272` and `009D0285` (`MOVSS [ESI+98h]`), which sit in a body Ghidra has no
  function for.

**Why it is not bound yet:** two of its inputs are wrong in the host's reset.
- **`+70h` is not read-only** (TorpedoApproachState calls it `closing_speed_bias_70`, "read
  only", and holds 0).
  - `009D0449` stores `U(0.9, 1.1) * min(desc+18Ch TravelSpeed, 0.75 * 007BCE20(unit))`. The
    draw is stream 1: lo `00CE3860` 0.9f, hi `00CE6448` 1.1f. The 0.75 is `00CEC9D8`, a double.
  - `007BCE20` is the minimum of `bullet+DCh` over the unit's devices whose bullet kind `+8h` is
    0Ah; `+DCh` is unread.
  - So `+70h` is a planned attack speed. The engagement estimate adds it to the closing speed
    (`torpedo_approach_update.cpp:119`), so it also moves `+F8h` and the torpedo lead's projtime.
- **`+78h` is a draw, not the row value.** `009D0451`-`009D0475` store
  `U(0.0, 0.25) * record+0h` (TorpReleaseAlt). The draw is stream 1: lo from `FLDZ`, hi
  `00CE3868` 0.25f. The host stores the full TorpReleaseAlt (12 m for SPNormal), where the image
  gives 0-3 m.
  - `+74h` gets 67.0f (`00D212A0`) here. The host takes it from `009D3489` later, which is
    unchanged.

**Proposed order:**
1. A reset packet binds `+70h` and `+78h` with their stream-1 draws. These are `units.cpp` hunks
   and change the torpedo descent and release on every torpedo row.
2. Then `009D0160` seeds `+98h` from them.

## 18. Handoff queue (cc9-planes2, about 75% context)

This session's switches:
- `kApproachSectionPointsBound` is ON (14.4).
- `kTorpedoGoAwayAimPointBound` is ON and gameplay-inert on the reference rows (15.4).
- `kFlyToObstacleListBound` is committed OFF and not yet paired (`a9a1a4051`, section 16).
- USN01 no longer reaches the goaway, and that is recorded as not a regression (15.5).

**Environment at handoff:** runs fail at renderer start (`CreateDevice` hr `0x8876086a`,
`logonui=1`). Check `query session` and run one 300-frame smoke before any pair.

1. **The obstacle-list pair (next).**
   - Run `python tools/pair_export.py --commit <tip> --flip kFlyToObstacleListBound=true` on a
     synced tree.
   - Rows: USN04 4700/4500 and E2 (USN04 9200/9000), in 14.4's launch form. JM05 is optional; its
     torpedo goaways are inert.
   - **Mechanism census:** the `summary mission fly-to obstacles` line. Expect dive_goaway
     `rebuilds` > 0 and `kept` > 0 near the enemy ships. `avoid_ticks` > 0 means the avoid term ran,
     and `side_writes` counts `009FDC48`.
   - **Predictions (section 16):**
     - The Vals' goaway turns change, with some side flips.
     - The torpedo goaway stays inert (heading_ticks 0).
     - Deaths move by up to 3 through the shared stream. Diff the per-entity death table.
   - **Verdict rule:**
     - Keep it OFF if `kept` = 0 on rows whose Vals break off beside an enemy ship, or if a row
       with no goaway moves.
     - Otherwise flip it. The flip is one line in src/game_hosts_units.cpp, so claim that file for
       it alone.
2. **Torpedo reset draws** (units.cpp hunks; section 17 has the reading).
   - `+70h` at `009D0449`: `U(0.9, 1.1)` on stream 1, times
     `min(desc+18Ch TravelSpeed, 0.75 * 007BCE20(unit))`.
     - `007BCE20` is the minimum `bullet+DCh` over the unit's devices whose bullet kind is 0Ah.
       Read `+DCh`'s authored key first.
     - The host holds `closing_speed_bias_70 = 0`, and the engagement estimate adds `+70h` to the
       closing speed (src/torpedo_approach_update.cpp:119). So this also moves `+F8h` and the torpedo
       lead's projtime.
   - `+78h` at `009D0451`-`009D0475`: `U(0.0, 0.25)` on stream 1, times `record+0h` TorpReleaseAlt.
     The host stores the full row value.
   - **Then `009D0160`** seeds `+98h = max(0, t + +9Ch)` with section 17's formula, called after
     `009D02A0` in the reset (`009D0632`).
     - **Get a second reader for the x87 stack**: the deceleration distance is subtracted twice
       (`009D01EC`-`009D01F6`, then `009D0227`), and `dt` is not floored.
     - `009D0160` has no Ghidra function. Its body is `009D0160`-`009D0292` (RET at `009D0291`,
       then INT3); send the definition to the integrator.
   - **Predict before running.** The draws change descent, release altitude and time on every
     torpedo row: USN04, E2, JM05, USN13.
3. **Carried over from section 13:**
   - the stream-1 stand-in draws (`release_altitude_draw_00bd2f10`);
   - a re-target zeroes projtime for one tick;
   - the lead uses the hull heading in place of `00812090`.
4. **Recorded, not queued:**
   - `0093A570` has never removed a section in a run (14.4).
   - The artillery draw's "never destroyed" is routed to gunnery.
   - `FlyToObstacle`'s field names `extent_max` / `extent_sum` in
     include/bsp/plane_fly_to_solver.hpp should be `aa_range_max` / `aa_damage_sum` (section 16).

Scripts in the cc9-planes2 tree's `local\`:
- `p2_runs.ps1`: the launcher (`-Variants`, `-Rows`, `-Tag`);
- `p2_edit_obstacles.py`: the applied edit, with an optional root argument;
- `p2_{off,on}_*` logs.

## 19. The torpedo reset's draws and its run-time seed (packet `cc9_torpedo_reset_draws`, cc9-planes3)

cc9-planes3, stamped 2026-09-29 20:24 UTC. Written while game runs failed at renderer init
(`CreateDevice` hr `0x8876086a`, `logonui=1`; smoke `local\p3_smoke.log` at about 20:17 UTC), so
everything below is a reading and a prediction until the pairs in 19.4 run.

### 19.1 What the reset draws (`009D0380 BSP_BotApproachTorpedo_Reset`, disk bytes)

| field | store | image value | host before this packet |
| --- | --- | --- | --- |
| `+70h` | `009D0449` | `U(0.9, 1.1) * min(desc+18Ch TravelSpeed, 0.75 * 007BCE20(unit))` | 0 (`closing_speed_bias_70`) |
| `+74h` | `009D0457` | 67.0f (`00D212A0`); `009D3489` replaces it on the first approach update | untouched until `009D3489` |
| `+78h` | `009D0475` | `U(0, 0.25) * record+0h` TorpReleaseAlt | the whole TorpReleaseAlt |
| `+7Ch` | `009D05ED` | `+7Ch + U(-0.1, 0.5) * (+80h - +7Ch)` | the row value times `+24h` |
| `+80h` | `009D0625` | `+80h - U(-0.1, 0.5) * (+80h - new +7Ch)`, old `+80h` kept as a double (`009D05F6`) | the row value times `+24h` |
| `+98h` | `009D0272`/`009D0285` in `009D0160` | section 17's formula, after `009D02A0` | 0, never written |

- **Draw bounds, read as bytes.** 0.9f `00CE3860`, 1.1f `00CE6448`, 0.25f `00CE3868` (low
  bound from `FLDZ`), -0.1f `00CE3CB4`, 0.5f `00CE3800`. The 0.75 is the double at `00CEC9D8`.
  - Every draw is `00BD2F10` with `ECX = 1`, stream 1. The first argument, `[ESP]`, is the low
    bound.
- **The min at `009D03FD`.** `FCOMI ST1` / `JBE` keeps the product `0.75 * 007BCE20` when it is at
  or below TravelSpeed. The product was rounded to a float at `009D03E7` first.
- **`007BCE20` and `007BCFA0` have the same shape**, each `__fastcall(unit)`:
  - They walk `unit+974h` (count `+994h`) and read `[[dev+3F8h]+34h]`.
  - They keep the minimum of `+DCh` (`007BCE20`) or `+E4h` (`007BCFA0`) over the devices whose
    `+8h` is 0Ah. They start from FLT_MAX (`00D7A248`).
  - `+8h == 0Ah` is the Torpedo sub-type (docs/AI_TARGET_WEIGHT_TERMS.md, `006EA4F0`).
  - `+DCh` is `MaxWaterHitVel`: `008566B0` reads the key and stores it raw at `+DCh`.
  - `+E4h` is `WaterTravelSpeed`, with no 0.6 factor.
  - So the planned attack speed is capped at three quarters of the slowest water-entry limit the
    aircraft's torpedoes allow.
- **Not bound in this packet:** the reset also draws `+88h` (`U(1.25, 1.5) * desc+268h + +80h`)
  and `+12Ch` (`-U(0, 1)`), where the host holds the engage range and 0. They are queued in 19.5.

### 19.2 `009D0160`, second reading of the x87 stack (asked for in 18, item 2)

I re-traced `009D0160`-`009D0292` instruction by instruction from the disk bytes, and **I agree
with section 17**. The opcodes the argument turns on, read as bytes because Capstone's register
forms are easy to misread:
- `009D01AE DE EA` is `FSUBP ST2,ST0`: ST2 = ST2 - ST0, so `+70h - run`.
- `009D0219` and `009D022B`, `DE F3`, are `FDIVRP ST3,ST0`: ST3 = ST0 / ST3.
  - At `009D0219` that gives L / dd.
  - At `009D022B` it gives (L - dd) / run.
- `009D0227 DE EA` is `FSUBP ST2,ST0`: ST2 = ST2 - ST0 = L - dd, where L is `+7Ch - (lead + dd)`
  from `009D01F6`.

So dd is subtracted twice on the middle arm. The comparison at `009D0211` also tests dd against
the L that already has dd taken off. `009D1360` subtracts only its fall lead
(`009D146E FSUB [ESP+14h]`), so the two routines really do differ. Other details:
- `dt` is not floored.
- `run` is not guarded against 0.
- `007BCC80` is `__thiscall(unit, h)`. `ECX` is still `[ESI+4]` from `009D0169`, and the
  `PUSH ECX` at `009D016F` only reserves the argument slot, which `009D0178` then overwrites
  with h.
- `+74h` at this point is the reset's 67.0.

### 19.3 The binding (committed OFF)

- **`src/torpedo_approach_update.cpp`** adds four functions:
  - `torpedo_reset_attack_speed_009d03d9`
  - `torpedo_reset_near_leg_009d05ed`
  - `torpedo_reset_far_leg_009d0625`
  - `torpedo_reset_run_time_009d0160`
  - Build-tested only. The run-time divide is guarded as the `009D1360` port guards it.
- **`src/game_hosts_units.cpp`**, in the torpedo task install after `+84h`:
  - `kTorpedoResetDrawsBound` sets `+70h`, `+74h`, `+78h`, `+7Ch` and `+80h`.
  - `kTorpedoResetRunTimeSeedBound` seeds `+98h` after the aim-error draw.
  - The seed is meant to run with the draws ON, since it reads their fields.
  - `torpedo_device_min_007bce20` walks the gunnery host's gun rows for the unit with
    `bullet_sub_type == 0Ah`.
  - The census line is `summary mission torpedo reset draws`, and the first 24 resets log
    `torpedo reset draws` / `torpedo reset seed` lines.
- **Labelled substitutions:**
  - Each draw has its own keyed stand-in stream (`#t70`, `#t78`, `#t7c`, `#t80`), so the
    existing draws keep their order.
  - The devices are the gunnery host's gun rows.
  - `007BCC80`'s vy is the host's `motion.linear_velocity.y`, as the dive side already takes it.
  - **The seed is never refreshed.** The image rewrites `+98h` through `009D1360` on every aim
    tick (`009D19A4`) and in done/prepare (`009D27D1`). In the host both calls are counters, so the
    reset's value stands for the whole task. Binding `009D1360` is queued in 19.5.

### 19.4 Predictions (written before any ON run)

**Numbers for USN04's torpedo class** (the OFF logs' trace: `hit_limit=100.0`, `swim=30.9`, so
WaterTravelSpeed 51.5):
- `0.75 * 100 = 75` exceeds any torpedo bomber's TravelSpeed. So `+70h` = `U(0.9, 1.1) *
  TravelSpeed`, about 55-77 m/s.
- `hit_vel_min` must print 100.0 on USN04, never FLT_MAX (3.4e38).
- With SPNormal, `+78h` falls from 12 m to 0-3 m. The commanded band `+74h + +78h` therefore drops
  by 9-12 m on every torpedo run.
- `+7Ch` moves by -20..+100 m and `+80h` by up to 100 m from 450/650.
- The seed: `h` is about 67-70, so `fall` is about 3.4 s and `run` = 51.5.
  - `dt` is about 0.1-0.3 (it can go negative for a slow draw).
  - `L` is about 200-320.
  - That gives **`+98h` of about 7-10 s**, plus the `+9Ch` time error.

**Draws alone (pair A, `kTorpedoResetDrawsBound=true`):**
- Mechanism:
  - `draws` equals the number of torpedo task installs.
  - Every `torpedo reset draws` line has a finite `hit_vel_min`.
  - The `+78h` values lie in [0, 0.25 * TorpReleaseAlt].
- Torpedo descent and release:
  - The commanded release altitude drops by about 10 m on every torpedo row (USN04, E2, JM05,
    USN01, USN13).
  - The release distance shifts with `+7Ch`.
  - The `+F8h` estimate shrinks at mid range, because the closing speed roughly doubles. So the
    lead shrinks there. It is unchanged at 30 s beyond about 1400 m.
- Torpedo-task releases and water contacts move on USN04 and E2. JM05, USN01 and USN13 release
  nothing now, and may stay at 0.
- Rows with no torpedo task (USN02, JM06, JM08, BSM01, LOMP06, LOMP10, USN12) stay gameplay
  identical.
- Deaths move by a few at most. Diff the per-entity table.

**Seed on top (pair B, both true against draws only):**
- Mechanism: `seeds` equals `draws`, and `mean_run_time_98` falls between 5 and 15 s, never 0 or
  NaN.
- `+F8h` at the release range rises from about 0 to about `+98h`. So the aim point leads the
  target by about `+98h` x target speed (about 100-160 m for a 16 m/s carrier) at the drop.
- Torpedo hit records move on USN04 and E2.

**Verdict rules:**
- **Pair A.**
  - Keep it OFF if `hit_vel_min` is FLT_MAX on a row with a torpedo task (the device mapping
    failed), if `draws` = 0 there, or if a row with no torpedo task moves.
  - Otherwise flip it, even if releases fall (spread miss with the mechanism matching, recorded).
- **Pair B.**
  - Keep it OFF if `+98h` is 0, negative, NaN or above 30 on any seed line.
  - Otherwise flip it.

### 19.5 First pair, measured: a mechanism failure caused by a host stand-in, now fixed

**Binaries** are `pair_export.py --commit deeb6573b`: r0 has no flip (`0640FFC4D188`), ra flips
the draws (`BF4DE40209C7`) and rb flips the draws and the seed (`91222581FD35`). The rows were
USN04 4700/4500, E2 9200/9000, JM05, USN01, USN13 and JM08, each 3200/3000, in the reference
launch form. All 18 logs are complete.

**The draws matched their reading.**
- `draws` equals the number of torpedo task installs (16 / 16 / 12 / 5 / 60 / 0).
- `hit_vel_min` is 100.0 on every line, so the device mapping works.
- `+70h` equals `U(0.9, 1.1)` times TravelSpeed (61.11 for the Kates and the TBDs, 66.67 for the
  USN01 Mavises), because 75 is above every TravelSpeed.
- `+78h` lies in 0.05-2.96, and `+7Ch`/`+80h` are jittered within their gap.
- The seed ran on every task. `mean_run_time_98` is 8.90 s on USN04 (predicted 7-10), 16.19 on
  JM05, 6.40 on USN01 and 9.71 on USN13. The lines range from 0.0 to 18.5 s.

**The gameplay failed.**
- In pair A, torpedo-task releases fell to 0 on every row that had any: USN04 5 -> 0, E2 5 -> 0,
  USN13 3 -> 0.
- The aim census shows why:
  - The altitude gate never opened (`alt=0` on every aim line).
  - `min_alt` rose from about 12 m to 68-71 m.
- The E2 death table agrees: the Kates die at 78-83 m instead of 20-25 m.

**Cause: this host's stand-in for the pilot control block.**
- `009D3489` copies `ctl+398h` into `+74h` only when it is below 100.
- The host's `read_control_block` supplied `ctl+398h` = Pilot/Torpedo/CruisingAlt = 500, so the
  store never ran.
- With the draws OFF, `+74h` stayed at the host's default 0, and the band was 0 + 12. With the
  draws ON, the reset's 67.0 stood.
- The image's producer is `009D4A70` step 5 (docs/BOT_TASKS.md): `+398h = *(task+40Ch)`, with 5.0f
  (`00CE3850`) replacing any value below the double 5.0 (`00D7A370`). `task+40Ch` is
  `approach+14h`, the run-profile record pointer, so this is **record+0h TorpReleaseAlt**.
  - For SPNormal that is 12, which is below 100, so the image's `+74h` is 12.
  - The image's band is therefore `12 + U(0, 3)`. The host's old `0 + 12` was right only by
    accident.

**Verdicts:**
- **Pair A: keep OFF (mechanism failure), recorded.** A reported "the commanded release altitude
  drops by about 10 m", and it rose by 57 m. That prediction was wrong because it assumed the host's
  `+74h` came from the image.
- **Pair B is void**, because it stacked on the broken A. For the record, its seed lines were sound.
  - One line is `+98h = 0.000`: USN01 Mav2, where `+9Ch` = -8.52 against t of about 8.3. That is the
    image's own `max(0, ...)` floor.
  - My 19.4 rule ("keep OFF if `+98h` is 0 on any line") was mis-specified: it should have excluded
    a zero produced by the floor. I restate it before the re-run instead of applying it after the
    fact.

**The fix (committed with this section).** Under `kTorpedoResetDrawsBound`, `read_control_block`
supplies `ctl+398h = max(TorpReleaseAlt row, 5.0)`. The row is stored on the slot at the reset as
`torpedo_release_alt_row`.

**Predictions for the re-run** (pairs A' and B', from one new commit):
- **A'.**
  - The band becomes `+74h` 12 plus `U(0, 3)` on SPNormal, so the commanded altitude rises 0-3 m
    over OFF. `min_alt` on the aim lines lies within about 3 m of OFF's.
  - Torpedo-task releases move by a few in either direction, but do not collapse to 0 on USN04, E2
    or USN13.
  - JM08 is gameplay-identical.
  - Deaths move by a few at most.
- **B'.**
  - `seeds` equals `draws`, and the means are as before (the seed does not read `+74h` from the
    control block).
  - Torpedo hit records move on USN04 and E2.
  - Keep B' OFF if a seed line prints NaN or inf, if a line above 30 s appears, or if a `+98h` of 0
    appears with `+9Ch` above -3 (a zero the floor cannot explain). Otherwise flip it.
- **A' verdict:** keep it OFF if releases collapse to 0 again, or if `min_alt` differs from OFF by
  more than 5 m. Otherwise flip it.

### 19.6 The re-run, measured: A' and B' both flip

**Binaries** are `pair_export.py --commit ac16e89ac`: s0 has no flip (`1889CDC9C4D6`), sa flips the
draws (`9BE233F9F054`) and sb flips the draws and the seed (`A52B1E3189FC`). The rows and the
launch form are 19.5's. All 18 logs are complete, and the runs finished at 21:40 UTC.

| row | A' (s0 -> sa) | torpedo-task releases | deaths | B' (sa -> sb) | torpedo-task releases | deaths |
| --- | --- | --- | --- | --- | --- | --- |
| USN04 4700/4500 | 3 | 5 -> 5 of 16 | 45 -> 44 (1 only-OFF Zero) | 3 | 5 -> 8 of 16 | 44 -> 48 (4 only-ON, among them a Zero and two Vals) |
| E2 9200/9000 | 3 | 5 -> 5 | 51 -> 51, none one-sided | 3 | 5 -> 8 | 51 -> 51, none one-sided |
| USN13 3200/3000 | 3 | 3 -> 3 of 60 | 31 -> 31 | 3 | 3 -> 4 | 31 -> 31 |
| USN01 3200/3000 | 3 | 0 -> 0 of 5 | 5 -> 5 | 3 | 0 -> 0 | 5 -> 5 |
| JM05 3200/3000 | 1 | 0 of 12 | identical | 1 | 0 | identical |
| JM08 3200/3000 (control) | 1 | - | identical | 1 | - | identical |

**A', against the restated predictions:**
- **The band held.** Per aircraft, `min_alt` moved by -3.9 to +3.2 m on USN04 (16 aircraft) and
  by -1.7 to +4.5 m on USN13 (4 aircraft). That is inside the 5 m rule, against a prediction of
  +0 to 3.
- **The releases did not collapse**; they are unchanged.
- **The control is identical.**
- **Verdict: flip.**

**B', against the restated predictions:**
- **The seed line counts match.** `seeds` equals `draws` on every row (16 / 12 / 5 / 60 / 0), and
  the means are unchanged (8.90 s on USN04).
- **The seed-line rule held.** Of 73 seed lines, two are 0.000:
  - USN01 Mav2 with `+9Ch` = -8.52;
  - one with `+9Ch` = -9.50.
  Both are floors that `+9Ch` below -3 explains. No line is NaN, inf or above 30.
- **Torpedo releases rose.** They went from 5 to 8 of 16 on USN04 and E2, and from 3 to 4 on USN13.
  - This is the lead at the drop that 19.4 predicted: `+F8h` no longer falls to about 0 at release.
  - Damage on USN04 rose from 13514 to 16652, and on E2 from 14409 to 17174.
- **Deaths.**
  - USN04's 4500-frame window gains four deaths at its end.
  - E2, the same mission run to 9000 frames, has no one-sided row.
  - So those four are timing inside the shorter window, not new kills.
- **Verdict: flip.**

**Still standing:**
- The seed is never refreshed (`009D1360` is unbound). It is the next packet.
- `+88h` and `+12Ch` are not drawn.

## 20. Who reads the pilot control block's `+398h`, and what this host supplies (packet `cc9_ctl398_audit`)

cc9-planes3, read-only. This is the lead's audit of the `ctl+398h = 500` stand-in that 19.5 found.

**Census method.** I ran `scan-bytes` with `--limit 4000` over `.text` for every disp32 form at
`+398h`:
- `D9`, `D8` and `DC ?? 98 03 00 00`;
- `F3 0F 10`, `11`, `58`, `59` and `5C`;
- `0F 2F`, `0F 2E`, `8B`, `89` and `C7`.

153 hits came back (`local\p3_scan398.txt` in the cc9-planes3 tree). Most sit on other
structures: `gun+398h`, the ship AI nav block (`009DDBC0`, `009DA0D0`, `009ED6B0`), the mission
record, and the pose matrices in `007BEEE0` (`+364h`/`+394h..+39Ch` next to `+608h..+610h`). The
pilot control block is reached as `approach+0Ch` or `task+404h`. The hits on it are:

| routine | role | reads or writes `ctl+398h` | host |
| --- | --- | --- | --- |
| `009D4A70` torpedo cruise profile | writer, step 5 | `max(*(task+40Ch) = record+0h TorpReleaseAlt, 5.0)` | **was CruisingAlt 500**; `max(TorpReleaseAlt, 5)` since `ac16e89ac`, ON with `kTorpedoResetDrawsBound` (19.6) |
| `009D3420` torpedo approach update, `009D3489` | reader, copied into `+74h` when below 100 | - | reads the above through `read_control_block` |
| `009C8920` dive cruise profile, `009C89CE` | writer | `U(0, 15) + BeginAltRange/1` (leader only), gated by `+38Ch` / `+37Ch` / `+3AAh` | modelled: `kDiveProfileDrawBound`, `kSquadronAttackAltBound` (`src/game_hosts_units.cpp` near 3262) |
| `009C7A80` dive approach update, `009C7AA7` | reader, copied into `approach+ACh` | - | `db_begin_alt_ac` from the squadron block's `sq_profile_398` / `sq_alt_398` |
| `009C62B0` flyabove, `009C4A40` goaway, `009C7F00` goaway-complete | readers | - | read `db_begin_alt_ac` (the same copy) |
| `009A4DC0`, `009A5000` (depth-charge constructors) | `MOV [ESI+398h],EBX`, where ESI is probably the task, not the block (not checked) | - | depth-charge task not hosted |
| `009A1D60`/`009A1DC0`/`009A1A90`/`009A29F0` (close-to-ship), `009AB850`, `009ADE00`, `009B7C90` (level bomb), `009CA3B0` (strafe), `007B3EA0` (rocket) | approach readers; the base `approach+0Ch` is checked at `009AB857`, `009B7CA6` and `007B3EA6` only | - | these tasks are not hosted, so there is no stand-in to correct |

**Findings:**
- **The 500 stand-in reached one reader, `009D3489`.** It is corrected by the ON switch, so no
  other host path saw it. The dive family has its own model of the same block, taken from its own
  writer `009C8920`, and does not use the torpedo binding's stand-in.
- **One real gap: the script override on the torpedo path.** `008A22B0 SquadronSetAttackAlt`
  writes the squadron block's `+398h` and sets `+3AAh`. `+38Ch` is set when forced. `009D4A70`
  step 5 then keeps the script value exactly as `009C8920` does for the dive. The host's torpedo
  `read_control_block` ignores `sq_alt_398`.
  - **Inert on the reference rows:** no `squadron attack alt` line appears in any 19.6 log (USN04,
    E2, JM05, USN01, USN13, JM08).
  - In this installation, `SquadronSetAttackAlt` appears in 81 mission scripts (for example
    JM02's Betties at 500, forced). IJN01's call at `ijn_1_pearl.lua:1011` is on A7M fighters.
  - A torpedo squadron given an attack altitude of 100 or more makes `009D3489` skip its store.
    `+74h` then keeps the reset's 67.
  - **Not bound here.** No row in the reference set exercises it, so a pair could only show
    "identical". I have queued it, to be bound when a row with a torpedo squadron and a script
    attack altitude is found.

**No binding change comes out of this audit.**

## 21. `009D1360` bound: the run time and fall lead refreshed (packet `cc9_torpedo_run_time_update`)

cc9-planes3. The predictions below were written before any ON run.

### 21.1 The reading

- **Callers.** `009D1360` has exactly two callers (`ghidra xrefs`):
  - `009D19A4` in the aim tick `009D15F0`, when `turn_room > f14_range` at `009D19A0` (already
    ported as `update_run_time_009d1360`);
  - `009D27D1` in the done/prepare tick `009D2720`.
- **The done/prepare call site was ported to the wrong branch.**
  - The image's structure:
    - `009D2753 JBE` sends a disarmed countdown (`state+98h <= 0`) to `009D29E0`.
    - The committed branch steps the countdown down (`009D2782`-`009D2794`).
    - It releases through `009D25A0` when the countdown reaches zero, then stores `[00D7A260]`.
    - `009D27BE`-`009D27C9` re-tests the countdown, and `009D27D1` calls `009D1360` while it is
      still above zero, before any target test.
  - `torpedo_task_arm.cpp` called the hook on the idle outcome with the countdown at or under
    zero. That is the disarmed branch, which never reaches `009D27D1`.
  - The fix: `TorpedoDoneTickResult::run_time_updated` is set right after the countdown step, and
    the arm calls the hook on that flag.
  - The fix is unswitched. Today the hook only counts (`record`), so the only change it makes to
    an OFF run is the unimplemented-call count.
- **The body, re-traced on the x87 stack** (opcode bytes checked: `009D13D7 DE EA`,
  `009D148C DE F2`, `009D149A DE E9`, `009D149C DE F2`). It agrees with
  `torpedo_run_time_009d1360`:
  - `fall = 007BCC80(unit, unit+100h)` (the altitude) and `lead = fall x speed`, stored at `+A0h`.
  - `dt = max((speed - run) / 80, 0)` and `dd = dt x (speed + run) / 2`.
  - `L = min(+90h, +7Ch or +80h by the 15 s switch at 00CF3F20) - lead`.
  - `t` is `fall` when L < 0, `fall + dt + (L - dd)/run` when dd <= L, and `fall + dt x L/dd`
    otherwise.
  - `+98h = max(0, t + +9Ch)`.
  - Unlike `009D0160`, dd is subtracted only once.
- **The reset.** `009D04E9` stores `+A0h = 10.0f` (`00CE38B8`), and the host had 0.

### 21.2 The binding: `kTorpedoRunTimeUpdateBound`, committed OFF

- `GameUnitsHost::Impl::torpedo_run_time_009d1360(slot, site)` supplies the inputs:
  - the speed as `unit_speed_vtable38` computes it (the length of the live velocity);
  - the fall time from `weapon_fall_time_007bcc80(position.y, velocity.y)`;
  - the run speed from `torpedo_device_min_007bce20(slot, true)` (`007BCFA0`).
- It writes `+A0h` and `+98h`.
- It is called from the aim-tick override (site 0) and from the done/prepare hook (site 1).
- Under the switch, the reset also stores `+A0h = 10`.
- There is a census line, `summary mission torpedo run time`.

### 21.3 Predictions

**`+98h`.**
- At the aim updates a Kate flies at about 12-15 m and about 61 m/s, so `fall` is about 1.3-1.5 s
  and `lead` is about 80-90 m.
- `L` is about 350-500 m less the lead, so `t` is about 7-10 s. The mean `+98h` over updates is
  therefore **7-11 s**, close to the reset seed's 8.9 s on USN04.
- So `+F8h`, and the lead, move by about a second at most on the Kates that reach the update.

**`+A0h`.**
- It is 10 from the reset and about 80-90 m after an update.
- It feeds the aim release gate at `009D2052`: `f14_range + 80 > +A0h`. That fails only when
  `f14_range` is under about 10 m.
- So **the lead gate should not change a release** on these rows.

**Counts.**
- Aim-site updates are greater than 0 on USN04, E2 and USN13 (the rows with aim ticks near the
  release range).
- Done-site updates are greater than 0 where a committed countdown ran, which is every row with a
  release.

**Gameplay.**
- USN04, E2 and USN13 move a little, through the lead: torpedo-task releases within plus or minus
  2 of 8 / 8 / 4.
- JM05, USN01 and JM08 stay gameplay-identical, because they have no torpedo release.
- Deaths move by a few at most. The per-entity table is the one to read.

**Verdict rule.**
- **Keep OFF** if any of these holds:
  - the aim-site updates are 0 on a row whose aim census has ticks near the release range;
  - any update prints NaN, inf, or a `+98h` above 30 s;
  - torpedo releases fall by more than 3 on any row.
- **Otherwise flip.**

### 21.4 The pair, measured: `kTorpedoRunTimeUpdateBound` ON

**Binaries** are `pair_export.py --commit 0794659c8`: t0 has no flip (`5F1254E19E8D`) and t1 flips
the switch (`D06194F4DDD1`). The rows and the launch form are 19.5's. All 12 logs are complete,
and the runs finished at 22:05 UTC.

| row | pair_diff | aim updates | done updates | mean `+98h` at the aim | mean `+A0h` at the aim | torpedo-task releases | deaths |
| --- | --- | --- | --- | --- | --- | --- | --- |
| USN04 4700/4500 | 3 | 437 | 0 | 9.92 s | 104.7 m | 8 -> 9 of 16 | 48 -> 48, none one-sided |
| E2 9200/9000 | 3 | 437 | 0 | 9.92 s | 104.7 m | 8 -> 9 | 51 -> 51, none one-sided |
| USN13 3200/3000 | 3 | 243 | 0 | 10.46 s | 186.3 m | 4 -> 3 of 60 | 31 -> 31, none one-sided |
| USN01 3200/3000 | 3 | 49 | 0 | 3.49 s | 268.1 m | 0 -> 0 of 5 | 5 -> 5, 3 re-timed |
| JM05 3200/3000 | 1 | 0 | 0 | - | - | 0 of 12 | identical |
| JM08 3200/3000 (control) | 1 | 0 | 0 | - | - | - | identical |

**Against the predictions:**
- **Mean `+98h` 7-11 s: held** on USN04, E2 and USN13. USN01's Mavises reach the update higher
  (their mean `+A0h` is 268 m), so their fall lead eats most of the leg and the mean is 3.5 s.
  Across all updates `+98h` ranged from 2.93 to 16.36 s, with no NaN, inf or value above 30.
- **`+A0h` of 80-90 m: close.** It is 105 m on USN04 and 186 m on USN13, because the updates happen
  higher than the 12-15 m band. The lead gate did not stop a release; releases moved by one.
- **Done-site updates greater than 0: missed.** They are 0 on every row. The cause is not the
  binding. No committed countdown runs on these rows: every aircraft's `drop_timer_98` stays -1
  and `prepare_entries` is 0 (the `orders` and `attack mode` census lines). The releases come
  from the aim state, so `009D27D1` is never reached. The 21.1 call-site fix therefore remains
  untested at run time.
- **Releases within plus or minus 2: held** (+1, +1, -1).
- **USN01 identical: missed.** Its aim ticks do reach the update, so the lead and the flight path
  move. No release changed on it, and no death became one-sided.
- **JM05 and JM08 identical: held.**

**Verdict:** none of the three keep-OFF conditions holds. The aim site ran wherever aim ticks
reached the release range, no value is out of range, and no row lost more than one release. The
switch **flips ON**. The two misses are recorded: the done site is unexercised here, and USN01
moved without a release change.

## 22. Handoff queue (cc9-planes3, about 75% context)

Stamped 2026-09-29 22:06 UTC.

**This session's switches, all in src/game_hosts_units.cpp:**
- `kFlyToObstacleListBound` is ON (16.1).
- `kTorpedoResetDrawsBound` and `kTorpedoResetRunTimeSeedBound` are ON (19.6). The draws carry
  `ctl+398h = max(TorpReleaseAlt, 5)` (19.5).
- `kTorpedoRunTimeUpdateBound` is ON (21.4).
- The done/prepare `009D27D1` call site was moved to the committed branch, unswitched (21.1).

**Environment:** runs worked from 21:07 to 22:05 UTC. The 20:17 and 20:56 batches failed at
renderer init (`hr 0x8876086a`, `logonui=1`); one smoke run 10 minutes later passed each time.

1. **The reset's last two draws (next).** Both are in `009D0380`, with the constants read as bytes
   in 19.1.
   - **`+88h`** is `U(1.25, 1.5) * desc+268h + +80h`. The draw uses lo `00CF29A8` 1.25f and hi
     `00CE380C` 1.5f on stream 1. The field is `param_1[0x22]` in the pseudocode.
     - The same value times `[00D21298]` then goes into `+8Ch` (the engage range, later clamped by
       `009D4AC4`) and `+90h`. Read those stores whole in the listing (after `009D047D`) before
       binding.
     - The host sets `scan_radius_seed_88` to the engage range instead.
     - `desc+268h` is unread. Start with `rg -n "268h" docs include/bsp`.
   - **`+12Ch`** is `-U(0, 1)` (`009D0581`-`009D0590`, `FCHS`), where the host holds 0
     (`replan_timer_12c`). It shifts the first replan.
   - Bind both OFF with predictions and pair on the six rows of 19.5. Use the keyed stand-in
     streams `#t88` and `#t12c`.
2. **The script attack altitude on the torpedo path (section 20).**
   - `008A22B0 SquadronSetAttackAlt` sets the squadron block's `+398h`, and `009D4A70` step 5 keeps
     it, gated by `+38Ch`/`+37Ch`/`+3AAh` as `009C8920` is for the dive.
   - The host's torpedo `read_control_block` ignores `sq_alt_398`.
   - It is inert on every reference row. Bind it when a row with a scripted torpedo squadron is
     found: 81 scripts in this installation call it, so search them for a torpedo-task squadron.
3. **The done/prepare site is unexercised.**
   - No row starts the committed countdown: `drop_timer_98` stays -1 and `prepare_entries` is 0 on
     all 109 aircraft lines (21.4).
   - The 21.1 call-site fix and the `009D27D1` update need a row where a torpedo task enters
     prepare. The attack-mode census names its blockers (`blocked_0099af53`).
4. **Leftovers from sections 13 and 18.**
   - The FlyToObstacle fields `extent_max`/`extent_sum` are the AA range and the AA damage sum
     (section 16). Renaming them touches `include/bsp/plane_fly_to_solver.hpp`.
   - The `speed_late_7c`/`speed_early_80` names are distances (the header says so; the rename is
     still due).
   - The substitutions in section 13.

**Scripts** are in the cc9-planes3 tree's `local\`, with the `p3_` prefix:
- `p3_runs.ps1` is the launcher (`-Variants`, `-Rows`; rows are usn04, e2, jm05, usn01, usn13,
  jm08, usn12, jm06).
- `p3_edit_units*.py` are the applied edits, each taking an optional path for a dry run.
- `p3_diff*_<row>.txt` are the pair diffs.
- `p3_scan398.txt` is the section 20 census.

## 23. The reset's engage draws (packet `cc9_torpedo_reset_engage_draws`, cc9-gunnery15)

Section 22's item 1.

### 23.1 The image, `009D0380` (disk bytes, `disasm-raw 009D0497 --length 0x110`)

**The scan seed and the engage range.**
- `009D04A6`-`009D04B9` pushes hi `00CE380C` 1.5f and lo `00CF29A8` 1.25f (both read as bytes)
  and calls `00BD2F10`.
  - ECX is `1` from `009D0478`, and nothing writes ECX before the call, so the draw is on stream 1.
- `009D04BE FMUL [EDI+268h]`: EDI is `[ESI+8]` (`009D046F`), the class descriptor, and `+268h`
  is TurnCircleRadius (`include/bsp/dive_bomb_task.hpp`).
- `009D04D3 FADD [ESI+80h]`: the far leg as seeded at `009D0497`, before the jitter at
  `009D0625`.
- `009D04F9 FSTP` rounds the sum to a float, and `009D050B FST [ESI+88h]` stores it.
- `009D0517 FMUL qword [00D21298]` multiplies by `1.2999999523162842` (the bytes `00 00 00 C0 CC
  CC F4 3F`).
  - `009D052B FSTP` rounds to a float.
  - `009D0539 FST [ESI+8Ch]` and `009D053F FSTP [ESI+90h]` store the same value to both fields.
- `+8Ch` is then clamped up by `009D4AC4` to `AttackDist * ratio`. The host holds 2200 with a
  ratio of 1.0.

**The replan delay.**
- `009D0555`-`009D0579` pushes lo `FLDZ` and hi `FLD1`, sets `ECX = 1`, and stores
  `+128h = 1.0f`.
- `009D0581` calls `00BD2F10`, `009D0586 FCHS` negates it, and `009D0590` stores it to `+12Ch`.
- **Result:** `+12Ch = -U(0, 1)`.

**The host before this packet:**
- `+88h` was the engage range (2200).
- `+90h` stayed 0 until the first approach tick.
- `+12Ch` was 0.

**The readers of `+88h`:**
- the approach cap in `src/torpedo_approach_update.cpp` (`cap = +88h * ...`);
- the state-tick ratio `range_90 / scan_seed_88` in `src/bot_task_states.cpp`.

### 23.2 The binding (`kTorpedoResetEngageDrawsBound`, committed OFF)

- **In `src/game_hosts_units.cpp`,** the reset block, before the `009D0625` jitter:
  - `+88h` comes from `torpedo_reset_scan_seed_009d04be`;
  - `+90h` and the clamped `+8Ch` come from `torpedo_reset_engage_range_009d0517`, then
    `009D4AC4`;
  - `+12Ch = -U(0, 1)`.
- **Substitution, labelled:** keyed stand-in streams `name#t88` and `name#t12c`, as for the
  section 19 draws, so the draws the host already makes keep their order.
- **Logged:** the first 24 resets each write a `torpedo reset engage` line.

### 23.3 Predictions (written before any run)

**Magnitudes.** The far leg `+80h` is 617-642 m on USN04, USN13 and USN01, and 1057 m on JM05
(the R logs' `torpedo reset draws` lines).
- With a TurnCircleRadius of a few hundred metres, `+88h` falls from 2200 to roughly 800-1500 m.
- `+88h * 1.3` stays below 2200, so **`+8Ch` is unchanged** (the clamp holds it at AttackDist).
- The ON log lines will show the actual values.

**Rows,** against reference r:
- **Moved:** the torpedo-task rows USN04, E2, USN13, USN01, JM05 and JM05 long. Their approach
  cap and state ratio use the smaller `+88h`, and their first replan comes up to 1 s later.
- **Identical:** JM08, USN12 and JM06, which have no torpedo task.

**Caveat.** Under the party gate (R), only USN04 and E2 still release a torpedo (1 of 16 each), and
USN13 releases none. So the release counts can move only on USN04 and E2. Elsewhere the change
shows in the aircraft paths.

### 23.4 The pair: the mechanism held, the magnitude missed; flip ON

**Setup.**
- Exports of `18979299d` (main `9ff123eef` plus this packet): `local\g15_eoff`, SHA-256 prefix
  `1CBB523307C0`, and `local\g15_eon`, `5B5C9A7C3622`.
- Rows USN04, E2, USN13, USN01, JM05, JM05 long, JM08, USN12 and JM06, in the reference launch
  form.
- A smoke passed at 01:46 UTC, and the runs ended by 02:11 UTC.

**The values, from the ON log lines.**
- Every torpedo class on these rows has TurnCircleRadius **1200 m**: the Kates, the TBDs and
  Lexington's squadrons.
  - The Kate far leg `+80h` is 650, so `+88h` is 2250-2450.
  - JM05's `+80h` is 1200, so `+88h` is 2716-2971.
- `+90h = +8Ch = +88h * 1.3` is 2926-3863. That is **above** AttackDist 2200, so the `009D4AC4`
  clamp keeps the drawn value: **the engage range widens by 700-1650 m.**
- `+12Ch` falls in (-1, 0].
- Every value follows 23.1's formula.

**pair_diff.**
- Exit 3 (moved): USN04, E2, USN13 and USN01.
- Gameplay-identical: JM05 and JM05 long (exit 1), JM08 (exit 1), USN12 (exit 0) and JM06 (exit 1).

**Effects** (death tables, per entity):

| row | torpedo releases | deaths |
| --- | --- | --- |
| USN04 | 1 of 16 on both sides | 50 -> 51 (`D3A Val #7.1\|.-2` only ON); 43 rows re-timed; water contacts 14 -> 16 |
| E2 | 1 of 16 on both sides | 51 -> 51 with 44 re-timed |
| USN13 | 0 of 60 on both sides | 22 -> 25 (`bruh #1.4`, `#1.9\|.-2`, `#1.9\|.-3` only ON: Kates shot down on the longer approach) |
| USN01 | 0 of 17 on both sides | 5 -> 6 (`KatTBD\|.-3` only ON) |

No ship death changes on any row.

**Misses, recorded:**
- **The magnitude.** 23.3 assumed a TurnCircleRadius of a few hundred metres and predicted `+8Ch`
  unchanged. At 1200 m the draw exceeds AttackDist, so the engage range itself grows.
- **JM05 and JM05 long do not move.** Their twelve torpedo tasks reset, but no approach tick
  reads the new values within the window. This matches 21.4, where JM05 was identical.
- The extra plane deaths are path changes. Through the shared generator 00BD2F10 they are also
  coupled to the ships' fire draws, so they cannot be attributed kill by kill.

**Verdict: flip ON.**
- The mechanism matches the image's formula on every logged reset.
- The moved rows are the torpedo rows, and the controls are identical.
- The size is a spread miss.
