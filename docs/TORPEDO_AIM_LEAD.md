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
| the call | `009D3517` | `this->vtable[0](&local)` - the update asks itself for the aim point. |
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

**What this does not close, stated plainly.** "No writer names the displacement" is still not "no
writer". A block copy, or a pointer aliased into a register by some form not enumerated above, would
be invisible. **I did not find the writer of the torpedo task's `+D0h`, and I am not claiming the
field is dead.** The lead answer does not rest on it: section 2 is a complete census of the only
routine in the image that fetches a target velocity for a bot, so whatever writes `+D0h` did not
obtain a velocity to bake a lead into it.

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

* `00D213C0` is the **task** vtable, installed by `009D3050 BSP_BotTaskTorpedo_Construct` at
  `009D30A9`, not an approach vtable. The adjacent `00D213B8` is installed by
  `009D2DA0 BSP_BotTaskTorpedo_ConstructStates`. Both carry `009D0670` at slot 0.
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

## 7. Status

* **Proved from the listing**: sections 1, 2, 3, 5.
* **Corroborated, different class**: section 3.1.
* **Open**: the writer of the torpedo task's `+D0h` (section 3), and which of H1/H2 holds
  (section 3.2).
* **Measured**: section 6, against the existing `docs/SHIP_ESCORT_SCREEN.md` section 6.3 trace.
* **Not changed**: no source file, no constant, no Ghidra annotation.
