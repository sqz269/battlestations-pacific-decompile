# The five attackmove sub-state steps and the three predicates that pick them

Addresses: 009F3240, 009E23B0, 009E26C0, 009F3670, 007B3DD0, 009E5CA0, 009E5530, 00852860, 009E85B0, 00D21994

Packet `cc_ai_attackmove_substates`, worker `agent/cc-ai-attackmove-substates`. Project
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`; Ghidra was read-only for this
packet. Every descriptive name below is a hypothesis, not a recovered symbol.

`009E8450` builds five sub-state objects inside the attackmove state and writes the **same owner**
into every one of them at `+4h` (`009E8473`, `009E848B`, `009E84A1`, `009E84B1`, `009E84C7`). That
owner is the ship AI brain, which is what makes `[sub+4h]` the brain in all five steps and is the
fact the rest of this document rests on. Throughout: `unit` is `[brain+0AA8h]`, `shipclass` is
`[brain+0AACh]`, `target` is `[brain+0B20h]`, `blk` is `brain+8h`, and the attackmove command's
destination point is `(brain+0B2Ch, brain+0B34h)`. `brain+0B30h` is never read by any routine here.

The reconstruction is `include/bsp/ship_ai_attackmove_substates.hpp` and
`src/ship_ai_attackmove_substates.cpp`; it builds at `/W4 /WX` and the existing
`reconstructed_math` test passes. Nothing here is fixture-tested or game-validated.

## Answer to the packet question

Four of the five steps produce a navigation goal, not three. Each picks a different point:

| sub-state | the goal it hands `009DE050` | keep_mode, final_leg |
| --- | --- | --- |
| `+8h` approach `009F3240` | `(sub+1230h, sub+1238h)`, produced by the nested ring object, not by this body | 1, 0 |
| `+14C0h` engage `009E23B0` | target position plus target velocity times a lead time | 0, 0 |
| `+14CCh` lead pursuit `009E26C0` | target position plus target velocity times `max(target_y / -10, 3)` | 0, 0 |
| `+14E0h` tangent `009F3670` | a tangent point on a circle around the destination | 0, 0 |
| `+14F4h` initial `007B3DD0` | none; the body is one `RET 4` | n/a |

The selector's three questions are answered under each predicate's heading below: `00852860` is an
altitude gate on the target, `009E85B0` is the approach-to-engage gate, and `009E5CA0`/`009E5530`
are the approach sub-state's construction, not a predicate at all.

## `009E5CA0` and `009E5530`, the approach sub-state's construction

`009E5CA0`: `__thiscall(sub)(owner)`, `RET 4`, body `009E5CA0-009E5CEC`, complete. It stores the
owner at `sub+4h` (`009E5CC1`) and vtable `00D21994` at `sub+0h` (`009E5CD0`), then constructs a
nested object at `sub+8h` (= `state+10h`) through `009E5530` (`009E5CC5`, `009E5CD6`). The packet
brief called this a nested object at `+10h`; both readings are the same object, `+8h` relative to
the sub-state and `+10h` relative to the state.

`009E5530`: `__thiscall(nested)(owner)`, `RET 4`, body `009E5530-009E5769`, complete. It writes the
owner at `nested+0h` (`009E5540`) and then fills **60 records of `4Ch` bytes** starting at
`nested+4h` in two passes, so the array runs `nested+4h .. nested+B44h`.

Pass one (`009E5550-009E55C1`, 60 iterations, `EBP` and `ESI` both advancing by `4Ch`) zeroes the
record's `+0h` and `+18h..+3Ch` and the byte at `+40h`, then writes `+44h = 1000.0f` (`00CE3804`,
`009E55B1`) and `+48h = 00BD2F10(stream 1, 0.0f, 2.0f)` (`009E55A1`), a random draw in `[0, 2)`.

Pass two (`009E5680-009E5758`) writes the geometry. For record `i`:

```
spoke   = (float)((i / 60.0) * 2*pi)          009E5688..009E56A0   (FMUL ST1 is D8 C9, ST0 *= ST1)
bearing = fmod(spoke, 2*pi)                   009E56AA             (00BF857A is _CIfmod, fmod(ST1, ST0);
                                                                    the FXCH at 009E56A8 puts them in that order)
bearing += 2*pi if -pi >= bearing             009E56C5, 009E56C9
bearing -= 2*pi if bearing > pi               009E56D9, 009E56DF
record+08h = bearing                          009E56F9
h = pi/2 - bearing; h += 2*pi if h < 0        009E56ED, 009E5708, 009E570C
record+0Ch = cos h, +10h = 0.0f, +14h = sin h 009E571E, 009E573C, 009E572E
record+00h = [owner+0AA8h], +04h = nested     009E5692, 009E5695
```

So the approach sub-state carries a precomputed ring of 60 bearings one sixth of a radian apart
with a unit XZ direction each, in the game's from-`+Z` heading convention. That is where the
segment's `circle` keyword comes from. Constants: `00CE3828` = 2*pi, `00CE3D68` = 60.0,
`00CE3D18` = -pi, `00CE3D28` = pi, `00CE3830` = pi/2, `00CE3958` = 2.0f, `00CE3804` = 1000.0f.

`009E55C3` and `009E55CE` call `00954940` on `nested+12C0h` and `nested+13B0h` and `009E55E1`
calls `009DF8F0` on `nested+14A0h`; none of those three bodies was read, so the records at those
offsets are outside this packet's coverage.

## `009F3240`, the approach sub-state step (`state+8h`, vtable `00D21994` slot `+0Ch`)

`__thiscall(sub)(float seconds)`, `RET 4`, body `009F3240-009F3664`, read complete from the
listing. Ghidra's body ends at `009F3664`; `009F3670` is a separate function.

**Early out.** `009F3262` loads the target and `009F3277` reads the byte at `target+5Dh`. A null
target or a non-zero byte takes `009F3645`, which calls `009E00A0` with `ECX = sub+4h`, the
**address** of the owner field, and returns. Nothing else runs.

**The nested update.** `009F328F` calls `009F3090(nested, seconds)`, `__thiscall`, `RET 4`, body
`009F3090-009F30E3`. That body is seven calls on the nested object in a fixed order: `009F1BC0`
(seconds), `009E7FC0`, `009E6E80`, `009E9190`(seconds), `009E74D0`(seconds), `009E76D0`(seconds),
`009E6A90`. None of the seven was read; the goal fields `009F3240` reads afterwards are written by
that sequence, so **`009F3240` does not compute its own approach point**. This is the follow-up
packet `ship_ai_attackmove_ring_update`.

**The throttle.** `009F3294-009F32EB`:

```
t = (float)(2 * (([unit+494h] + 500.0) - sub+11E8h))    009F32A6, 009F32AC, 009F32B2, 009F32B6
t = 0.0f     if !(0.0 <= t)                             009F32C2, 009F32C6
t = 1000.0f  if t > 1000.0                              009F32D5, 009F32DB
```

`00CE3840` = 500.0, `00CE47A0` = 1000.0, `00CE3804` = 1000.0f. The value goes to `brain+258h` and
`brain+2C0h` (`009F337B`, `009F3383`).

**The goal and the writes.** `009F332B` calls `009DE050(blk, &goal, keep_mode = 1, final_leg = 0)`
with `goal = {sub+1230h, sub+1238h}`. Then, in this order: `brain+370h` and `brain+368h` are
cleared only when `brain+1CCh != 3` (`009F3335`), `brain+1E0h = sub+1214h` (`009F335C`),
`00605070` wraps it in place (`009F3360`), `brain+1D4h = 0` (`009F336B`), `brain+1CCh = 3`
(`009F3375`), the throttle goes to `brain+258h`/`+2C0h`, and at the tail `brain+1D0h = 0`
(`009F361C`), `brain+1D8h = clamp(sub+1218h, -1, 1)` (`009F3635`) and `brain+1D4h = 0` again
(`009F363D`).

**The warn sweep.** `sub+14B4h` counts down by `seconds` (`009F338B`, `009F33A8`) and the sweep
runs only while it is strictly negative (`009F33E2`), the re-read target answers
`vtable[5Ch](1Ch)` (`009F33BF`) and `[unit+54h] != [target+54h]` (`009F3402`). It then resets the
timer to 1.0f (`009F340B`) and builds a candidate list: if `00778890` says the unit is its group's
leader (`MOV EAX,[ECX+284h]` then `[group+14h] == unit`, body `00778890-007788A7`), the list is the
group members that answer `vtable[5Ch](6)` and `[member+538h]->vtable[2Ch]()`; otherwise it is the
unit alone, if `[unit+538h]->vtable[2Ch]()` answers (`009F35D8`). For each candidate the sweep
tests `0092D730([c+1018h]) / 0080FC30(c) < 0.4` (`009F350C`), then the squared XZ distance from the
candidate to the target's refreshed position against `(float)([target+7C4h] * [target+7C4h])` --
an **integer** square (`IMUL` at `009F3546`, `FILD` at `009F357D`) -- then
`candidate->vtable[234h](target)`, and finally routes a message built by `0064A820` through
`0077C2A0(candidate, msg, 2, 0)`.

Two hazards in the original, reproduced in the doc but not in the C++: `009F3473` reads
`[member+538h]` after `009F3471` may have nulled `member`, and `009F3488` indexes a 40-slot stack
array with an unbounded group count. The reconstruction skips a null member and stops at 40.

## `009E23B0`, the engage sub-state step (`state+14C0h`, vtable `00D2174C` slot `+0Ch`)

`__thiscall(sub)(float seconds)`, `RET 4`, body `009E23B0-009E26BD`, read complete from the
listing this time. `seconds` is never used.

The sub-state's only storage is the byte at `sub+8h`, a two-mode latch. This is the `targetlock`
and `attackrun` pair the segment keywords name.

**Geometry.** `009E2429-009E2547`, every expression from the listing:

```
dx = target.x - unit.x ; dz = target.z - unit.z             009E2429, 009E2435
distSq = dx*dx + dz*dz                                      009E2441..009E2451
range  = distSq > 1e-10 ? sqrt(distSq) : 0.0f               009E245F, 009E2465, 009E2478
nx = dx / range ; nz = dz / range                           009E2491, 009E2499   (no zero guard)
closing = (unit.vx - target.vx)*nx + (unit.vz - target.vz)*nz  009E24DD..009E2501
divisor = 0.4 <= closing ? closing : 0.4f                    009E250F   (a floor, not a switch)
lead    = (float)(range / divisor - 2.0)                     009E2529, 009E252F, 009E2533
lead    = 0.0f if !(0.0 <= lead)                             009E2543, 009E254D
lead    = 12.0f if lead > 12.0f                              009E264F, 009E2658
intercept = target position + target velocity * lead         009E2594..009E25B8
```

Constants: `00CE3820` = 1e-10, `00CE65D0` and `00CE7804` = 0.4, `00D7A308` = 2.0,
`00CEB4B8` = 12.0f, `00CE3AE8` = 300.0f, `00CF8850` = 250.0.

Both velocities come from `vtable[34h]` with a hidden return buffer (`009E24CA`, `009E24DB`), and
the target's is fetched a **second** time at `009E2562` for the intercept rather than reused.
Reading `vtable[34h]` as a velocity is this packet's reading of its use, not a recovered contract:
the callee body was not read.

**Both arms** write `brain+3F8h = [unit+54h]` (`009E2588`), the avoidance request's side field.

**Close mode** (`sub+8h == 0`, `009E25BC`): `brain+3FCh = 1` (`009E2669`),
`009DE050(blk, &intercept, 0, 0)` (`009E267B`), then `009DA610(blk, &intercept)` (`009E268A`) and a
range under 250 (`009E269D`) set `sub+8h = 1` (`009E26A3`). `009DA610` is
`__thiscall(blk)(const float*)`, `RET 4`, body `009DA610-009DA66F`, read: false unless `blk+2FDh`
is set and the point is within 2500 squared units of `blk+1DCh/1E0h`, and on the far side it also
clears `blk+2FDh`.

**Run mode** (`sub+8h != 0`): `brain+3FCh = 0` (`009E25CC`), the heading to the intercept point
through `009DFF40(brain, h)` (`009E2620`), and `brain+0AF0h = 1.0f` (`009E262F`). The heading is
`h = pi/2 - atan2(dz', dx')` with one conditional `+2*pi` (`009E25EB`, `009E25F8`, `009E260C`);
`009E25E3` loads `dz'` first and `009E25E7` loads `dx'`, so the helper gets `ST1 = dz'`,
`ST0 = dx'`. That is the opposite operand order from `009E26C0`'s `atan2(dx, dz)` at `009E2814`,
and the extra `pi/2 -` rotation is exactly what makes the two produce the same heading convention.

**Exit.** `009E2483` and `009E24A9` drop the latch back to 0 as soon as the range exceeds 300,
and only while it is already set. The step writes nothing the selector reads.

## `009E26C0`, the lead-pursuit sub-state step (`state+14CCh`, vtable `00D2177C` slot `+0Ch`)

`__thiscall(sub)(float seconds)`, `RET 4`, body `009E26C0-009E2B58`, complete. The selector picks
it at `009E8756` when `brain+0B28h` is set. Storage: `sub+8h` a turn budget, `sub+0Ch` a two-mode
byte, `sub+10h` an arming distance.

**Early out.** `009E26D4`, a null target, then `009E00A0` with `ECX = sub+4h` and return. The three
pose refreshes at `009E26FA`, `009E2726` and `009E2752` are guards, not exits; the compiler hoisted
the position loads above their guards.

**Geometry.**

```
t    = (float)(target.y / -10.0) ; t = 3.0f if !(3.0 <= t)   009E275F, 009E278E, 009E279A
lead = target position + target velocity * t                 009E27BB..009E27E1
009DB6C0(sub, &lead, 10.0f) clamps the pair into the world box   009E27EF
dx = lead.x - unit.x ; dz = lead.z - unit.z                  009E27F4, 009E2800
desired = atan2(dx, dz)                                      009E280C, 009E2810, 009E2814
r = (float)(turnRadius * 1.5) ; r = 300.0f if !(300.0 <= r)  009E2853, 009E2867, 009E286D
error = wrap(desired - heading)                              009E2890, 009E28A0
a = error > 0 ? error : (-0.0f - error)                      009E28B3
r = 200.0f if sub+0Ch == 0 or !(1.5 > a)                     009E28D8, 009E28DD, 009E28DF
```

Constants: `00D0A198` = -10.0, `00CE3854` = 3.0f, `00CE3D78` = 1.5, `00CE3CA8`/`00CE3AE8` = 300,
`00CE386C` = 200.0f, `00CE380C` = 1.5f, `00CE3990` = 10 degrees, `00D05AAC` = 60 degrees,
`00CE3800` = 0.5f, `00CE3828` = 2*pi.

`009E283F` calls `00438AA0(desired, pi)` only in direct mode and **discards the result** at
`009E2844` (`DD D8`, `FSTP ST(0)`). It is a dead call, kept in the reconstruction because it is an
observable call.

**Arm choice.** `009E2918` sends `distSq > r*r` to the goal arm; `009E2922` falls to the heading
arm while `a <= 1.5`, and above the gate `009E2929` sends direct mode to the goal arm and leaves
budget mode on the heading arm.

**Goal arm:** `brain+1D0h = 1` (`009E2A41`) and `009DE050(blk, &lead, 0, 0)` (`009E2A53`).

**Heading arm:** the command starts as `desired` (`009E295D`) and is replaced only when the fresh
error leaves `[-1, +1]`, by `00438AA0(heading, +-1.0)` (`009E2993`, `009E2999`); then
`009E0040(sub+4h, command)` (`009E29AC`). The speed scale `brain+0AF0h` is
`00419010(10deg, 1.0, 60deg, 0.5, |error|)` in budget mode (`009E2A0E`, `009E2A1F`) and 1.0f in
direct mode (`009E2A33`).

**The two-mode cycle.** In budget mode `sub+8h += |yawRate * seconds|` (`009E2A97`, `009E2AD0`) and
the mode clears once the total passes 2*pi (`009E2AEA`, `009E2AEF`). In direct mode, once the
distance to the lead point exceeds `turnRadius * sub+10h` (`009E2B2D`), the mode arms
(`009E2B38`), `sub+10h *= 1.5` (`009E2B3D`) and `sub+8h = 0` (`009E2B41`). The step writes nothing
the selector reads.

## `009F3670`, the tangent-circle sub-state step (`state+14E0h`, vtable `00D217AC` slot `+0Ch`)

`__thiscall(sub)(float seconds)`, `RET 4`, body `009F3670-009F39B0`, complete. The selector picks
it at `009E876C` when `brain+0B28h` is clear. Storage: `sub+8h` the range captured on enter,
`sub+0Ch` a budget, `sub+10h` the time in the sub-state.

`sub+10h += seconds` happens **before** the null-target test (`009F3670`, `009F3684`); the null
test at `009F368E` then takes `009E00A0` with `ECX = sub+4h`. The sibling on-enter `009E2BB0` sets
`sub+8h = 009DB820(sub)`, `sub+10h = 0` and
`sub+0Ch = rand(30, 40) * 00419010(500, 1.0, 1000, 0.0, sub+8h)`, which is unclamped, so beyond
1000 units `sub+0Ch` starts negative and the tail below never runs.

**Weapon release.** While `sub+10h > settings+4D4h` (`009F36B1`) and the director's `+30h` is not 2
(`009F36D9`) and `sub+10h > settings+4D4h + 5.0` (`009F36F3`), it calls
`0071E430(director, 00E08F78, 1)` (`009F3718`). The two pushes at `009F370D` and `009F370F` belong
to that call and not to the getter at `009F3714`: `0071E430` is `RET 8` and there is no `ADD ESP`
after `009F3718`.

**Geometry.**

```
radius  = rand[1.2, 1.5) * max(turnRadius, 500)             009F3763, 009F3784, 009F37C1, 009F37C6
minStep = min(radius, 300)                                  009F37EE..009F380C   (always 300 in play)
point   = 009D68B0(centre = destination, radius, unit pos, minStep, side 1)   009F3829
009DB6C0(sub, &point, 10.0f)                                009F383F
dx, dz  = point - unit position                             009F3844, 009F385D
distSq  = dx*dx + dz*dz                                     009F386F..009F3885
heading = distSq > 1.0 ? atan2(dx, dz) : unit heading       009F3889, 009F3895, 009F3869
thr     = min((float)(speed * 1.5) squared, 80000.0f)       009F38C0, 009F38CA, 009F38E8
```

`speed` is `[[unit+538h]+0A0h]` (`009F38BA`). Constants: `00D7A370` = 5.0, `00CE3840`/`00CE397C`
= 500, `00CE3814` = 1.2f, `00CE380C` = 1.5f, `00CE3CA8`/`00CE3AE8` = 300, `00CE38B8` = 10.0f,
`00CE3D78` = 1.5, `00D21A98` = 80000.0f, `00CE7630` = 30.0, `00CE4D70` = 200.0.

**Arms.** `distSq > thr` (`009F3906`) takes the goal arm: `brain+1D0h = 1` (`009F390A`),
`009DE050(blk, &point, 0, 0)` (`009F3920`), `brain+0AF0h = 1.0f` (`009F392F`). Otherwise
`009E0040(sub+4h, heading)` (`009F3943`) and `brain+0AF0h = 0.5f` (`009F3952`).

**Tail.** `009F3966` leaves when `sub+0Ch < 0`. Otherwise `r = 009DB820(sub)` and the block runs
when `r > sub+8h + 30.0` (`009F3982`) or `r < 200.0` (`009F398E`); both `sub+0Ch -= seconds`
(`009F3998`) and `009E2B60(sub)` (`009F39A4`) are inside that gate. The step writes nothing the
selector reads.

## `007B3DD0`, the initial sub-state step (`state+14F4h`, vtable `00D2171C` slot `+0Ch`)

The whole body is `C2 04 00`, one `RET 4`, followed by `CC` padding to `007B3DDF`. It has no
Ghidra function; the boundary is in the table at the end of this document. **Twenty** vtables
reference `007B3DD0`, so it is a COMDAT-folded empty one-argument virtual shared across classes,
not an attackmove routine; naming it after this sub-state would be wrong. Every other slot of
`00D2171C` is a constant or a no-op as well (`+4h` `009DB590` `ret`, `+8h` `007B3DC0` `ret`,
`+10h` `007B3DE0` returns true, `+28h` `009DAA90` returns 5.0f), so `state+14F4h` is the fully
defaulted base sub-state.

It does nothing before a target is chosen, because it is never usefully stepped. `009E8820` runs
the selector first (`009E88D8`) and the member step second (`009E88F0`), and `009E84F8`/`009E84FD`
seed `state+1500h` to a negative random value, so the very first selector call re-picks and moves
the machine off `state+14F4h` before any step dispatch. The no-target behaviour lives in the
selector's `009E87EE` arm, which puts the machine on `state+8h`, not in this body.

## `00852860`, the altitude gate

`__thiscall(entity) -> bool`, `RET 0`, body `00852860-008528AC`, complete. Seven callers: the
selector at `009E873B` and six HUD marker routines (`00641910`, `00641E30`, `00642040`,
`00642C20`, `00643360`, `006434E0`), all with the same shape -- one entity, no argument -- so the
contract is not specific to the AI.

```
if (entity+0C8h == 0) 00414DB0(entity)                  00852863, 0085286C
return entity+100h <= ([entity+1204h] + [entity+1200h]) / 3.0    00852871..00852890
```

The `MOV EAX,1 / TEST AL,AL / SETZ CL / MOV AL,CL` tail at `00852892` is a logical NOT written
out, so the `JBE` side is the **true** side. `entity+100h` is world y, so the selector is asking
whether a kind-8 target is at or below a third of the sum of two fields at `+1200h`/`+1204h`.
Those two have no producer in the ledger; only the arithmetic is established. `00D7A2B0` = 3.0.

## `009E85B0`, the approach-to-engage gate

`__thiscall(sub) -> bool`, `RET 0`, body `009E85B0-009E86B5`, complete. Its only caller is
`009E86F0` at `009E87CD`, with `ECX = state+8h`, and the selector runs it only while the machine
already holds `state+8h`; a true answer moves it to `state+14C0h` (`009E87E3`). So this is the
approach-to-engage transition, and it is the only way `state+14C0h` is ever selected.

Three conjuncts, in body order:

1. `[brain+0AA8h]` non-zero (`009E85C1`) and one of `[[unit+538h]+510h]` / `+514h` strictly greater
   than 0.0f (`009E85D8`, `009E85E8`). Neither comparison is an `abs`.
2. The destination `(brain+0B2Ch, brain+0B34h)` is not inside an avoid zone (`009E8660`).
   `0082ADC0` fetches the zone list for `[unit+538h]+570h` from the avoid-zone singleton
   (`004218E0`, `004120D0`), and `004178F0` walks `[list+4h]` for `[list+8h]` entries returning the
   first that answers `00416B50` for the point, or 0 (`004178F0-00417939`, `RET 4`).
3. The squared XZ distance from the unit's refreshed position to that destination is strictly under
   `4000000.0` (`00D09FE8`), that is the unit within 2000 units of the point (`009E8698`).

The object at `unit+538h` also answers `vtable[2Ch]` for `009F3240`'s sweep and carries a group id
at `+570h` and a speed at `+0A0h` for `009F3670`. Calling it the unit's armament component is a
hypothesis; its class was not established.

## Host methods the executable must implement, in call order

Every row is a call site inside the named routine's Ghidra body; the machine-readable form with
`address` and `native` per row is `reports/ship_ai_attackmove_substates.json`.

| routine | order |
| --- | --- |
| `009F3240` | `009F3647` 009E00A0 (exit arm only), then `009F328F` 009F3090, `009F332B` 009DE050, `009F3360` 00605070, `009F33BF` vtable[5Ch], `009F3429` 00778890, `009F3457` 0070D060, `009F346B` vtable[5Ch], `009F347E` vtable[2Ch], `009F34B3` 00427EB0, `009F34EA` 0092D730, `009F34FD` 0080FC30, `009F3521` 00414DB0, `009F3592` vtable[234h], `009F359C` 0064A820, `009F35B3` 0077C2A0, `009F35E3` vtable[2Ch] |
| `009E23B0` | `009E26B2` 009E00A0 (exit arm only), then `009E23DB` 00414DB0, `009E2410` 00414DB0, `009E24CA` vtable[34h], `009E24DB` vtable[34h], `009E2465` 00BF7030, `009E2562` vtable[34h], then either `009E25EB` 00BF701A and `009E2620` 009DFF40, or `009E267B` 009DE050 and `009E268A` 009DA610 |
| `009E26C0` | `009E26DA` 009E00A0 (exit arm only), then `009E26FA`/`009E2726`/`009E2752` 00414DB0, `009E2773` vtable[34h], `009E27EF` 009DB6C0, `009E2814` 00BF701A, `009E283F` 00438AA0 (dead), `009E284E` 0082E850, `009E2890` vtable[50h], `009E28A0` 00438B10, then either `009E2A53` 009DE050 or `009E293C` vtable[50h], `009E294C` 00438B10, `009E2993` vtable[50h], `009E2999` 00438AA0, `009E29AC` 009E0040, `009E2A0E` 00419010; tail `009E2A85` vtable[38h] and `009E2A97` 0082ECB0, or `009E2B0D` 00414C60 and `009E2B18` 0082E850 |
| `009F3670` | `009F3692` 009E00A0 (exit arm only), then `009F369F`/`009F36E1` 00424C40, `009F36C3`/`009F36D9`/`009F3714` vtable[114h], `009F3718` 0071E430, `009F3731` 00414DB0, `009F375A` 0082E850, `009F37C1` 00BD2F10, `009F3829` 009D68B0, `009F383F` 009DB6C0, `009F3869` vtable[50h], `009F3895` 00BF701A, then either `009F3920` 009DE050 or `009F3943` 009E0040, then `009F396A` 009DB820 and `009F39A4` 009E2B60 |
| `007B3DD0` | none |
| `00852860` | `0085286C` 00414DB0 |
| `009E85B0` | `009E8600` 00414DB0, `009E864C` 0082ADC0, `009E8658` 004178F0 |
| `009E5CA0` | `009E5CD6` 009E5530 |
| `009E5530` | `009E55A1` 00BD2F10, `009E55C9`/`009E55D4` 00954940, `009E55E1` 009DF8F0, `009E56AA` 00BF857A |

## Coverage

| routine | coverage |
| --- | --- |
| `009F3240` approach step | complete (`009F3240-009F3664` read instruction by instruction) |
| `009E23B0` engage step | complete (`009E23B0-009E26BD`) |
| `009E26C0` lead-pursuit step | complete (`009E26C0-009E2B58`) |
| `009F3670` tangent step | complete (`009F3670-009F39B0`) |
| `007B3DD0` initial step | complete (one instruction) |
| `00852860` altitude gate | complete (`00852860-008528AC`) |
| `009E85B0` engage gate | complete (`009E85B0-009E86B5`) |
| `009E5CA0` construction | complete (`009E5CA0-009E5CEC`) |
| `009E5530` nested construction | partial: the two loops and the field writes are complete; the sub-objects built at `nested+12C0h` and `nested+13B0h` by `00954940` and at `nested+14A0h` by `009DF8F0` are not projected, and the record fields at `+18h..+3Ch` have no reader in this packet |
| `009F3090` nested update | partial: the call sequence and its ABI are established; none of the seven callee bodies (`009F1BC0`, `009E7FC0`, `009E6E80`, `009E9190`, `009E74D0`, `009E76D0`, `009E6A90`) was read |
| `009D68B0` circle tangent | not read; modelled as one host call from `009F3670` |
| `009DB6C0` world-box clamp | not read in this packet; modelled as one host call |
| `009E2B60` sibling notify | not read; modelled as one host call |
| `0064A820` / `0077C2A0` warn message | not read; modelled as one host call |
| `00416B50` zone containment | not read; `004178F0`'s loop is read, its leaf is not |

## Corrections

| was | is | evidence |
| --- | --- | --- |
| `009F3240` ledger evidence: "it calls 009DE050 at 009F332B with the final-leg flag 1 (009F3308)" | `keep_mode = 1`, `final_leg = 0` | `009DE050` reads its first stack argument as the goal pointer (`009DE063 MOV EDI,[ESP+18h]`, `009DE0A0 FLD [EDI]`) and its second as the keep byte (`009DE087 CMP byte [ESP+1Ch],BL`). The `PUSH 1` at `009F3308` is the middle push and the `PUSH 0` at `009F32F5` is the first, so 1 is `keep_mode` and 0 is `final_leg`. |
| `009F3670` ledger evidence: "Body NOT read and it is not among the callers of 009DE050" | it does call `009DE050` | `009F3920 CALL 0x009de050`, and `009F3670` appears in `ghidra callers 009DE050` |
| docs/SHIP_AI_STATE_STEPS.md: "Three of the five steps are among the callers of `009DE050`: `009F3240`, `009E23B0` and `009E26C0`" | four of the five, adding `009F3670` | same as above |
| docs/SHIP_AI_STATE_STEPS.md: `009E23B0` "arms 009DE050(blk,&goal,0,0) then 009DA610(&goal), **or** 009DFF40(brain, heading)" with `brain+3FCh` set to "1 or 0" | the two arms are selected by the latch at `sub+8h`, and the flag value follows the arm: 1 on the goal arm, 0 on the heading arm | `009E25BC JZ 009E2663`; `009E2669` writes 1, `009E25CC` writes 0 |
| The packet brief's "nested object at `+10h`" | `sub+8h`, which is `state+10h` | `009E5CC5 LEA ECX,[ESI+8]` with `ESI` = the sub-state at `state+8h` |

These are recorded here; the `009F3240` and `009F3670` ledger evidence strings are updated by this
packet's `ledger add-name --replace` calls, with the old value kept in git history.

## Follow-up packets

- `ship_ai_attackmove_ring_update`: `009F3090` and its seven callees `009F1BC0`, `009E7FC0`,
  `009E6E80`, `009E9190`, `009E74D0`, `009E76D0`, `009E6A90`. This is where the approach
  sub-state's goal, heading and throttle at `sub+1214h`/`+1218h`/`+1230h`/`+1238h` are produced and
  where the 60-slot ring is scored. Without it `009F3240` has no approach point of its own.
- `ship_ai_nav_circle_tangent`: `009D68B0`, `009D6550`, `004F3970`, `004F47B0`, the tangent-point
  primitive `009F3670` relies on.
- `ship_ai_unit_armament_component`: the object at `unit+538h`. It carries `+0A0h` a speed,
  `+510h`/`+514h` the readiness pair `009E85B0` tests, `+570h` an avoid-zone group id, and a
  vtable whose `+2Ch` gates `009F3240`'s sweep. Four routines in this packet read it and none
  established its class.
- `ship_ai_brain_speed_scale_0af0`: `brain+0AF0h` is written by three of the five sub-states with
  1.0, 0.5 or an interpolated value, and no reader was found. Its role is provisional.
- `entity_velocity_vtable_0034`: the hidden-return-buffer virtual both `009E23B0` and `009E26C0`
  use as a velocity. The reading is from its use, not from the callee body.

## no_ghidra_function

| start | inclusive end | evidence |
| --- | --- | --- |
| `007B3DD0` | `007B3DD2` | The three bytes at `007B3DD0` are `C2 04 00`, which decode unambiguously as `RET 4`; the routine has no branch, so control cannot reach past `007B3DD2`. `007B3DD3-007B3DDF` are all `CC` padding to the 16-byte boundary, and the next routine starts at the aligned `007B3DE0` (`B0 01 C3`) and is separately referenced as vtable data from `00D056E0`, `00D05790` and `00D057AC`. The routine above ends at `007B3DC0` with `C3` followed by `CC` padding, so there is no fall-through into it. `ghidra xrefs` reports no reference to `007B3DD1`, `007B3DD2`, `007B3DD3` or `007B3DD8`; all twenty references land on `007B3DD0` and are DATA. |

## Uncertainties

- Every float expression here is transcribed from the listing, but the image computes in x87 80-bit
  registers with double memory operands. The C++ projection uses `double` and rounds to `float` at
  the same stores the image does; the two agree to float precision on ordinary inputs and are not
  bit-identical in general. Nothing in this packet is fixture-tested against the image.
- `vtable[34h]` as a velocity, `vtable[50h]` as a heading, `vtable[5Ch]` as an entity-kind
  predicate, `vtable[2Ch]` and `vtable[234h]` as readiness and acceptance predicates, and
  `vtable[114h]` as a weapon director are all readings from use. None of those callee bodies was
  read in this packet, and each host method in the header says so.
- `sub+11E8h`, which sets the approach throttle's bias, is seeded to zero by `009E5530` and written
  by the unread nested update. Its play-time value is unknown.
- `brain+0AF0h`, `brain+1D0h`, `[target+7C4h]`, `[entity+1200h]`/`+1204h` and `[unit+538h]+510h`
  have no producer or reader established here; their names are provisional.
- The class of the global at `00E08F78` was not established: its on-disk vtable `00CFB378` has
  `__purecall` at `+0Ch`, so the real class is installed by a static initializer.
- No run-time evidence was gathered. `bsp_game.exe` milestone 2o runs the composite step and the
  selector, but this packet added no host implementations to it, so none of these paths was
  exercised in a run log.
