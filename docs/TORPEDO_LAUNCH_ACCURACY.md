# Why every launched torpedo misses

Addresses: `008FBB00`, `008FB8D0`, `00BF7030`, `008FFF20` (`00900202`..`0090025E`,
`00900380`..`009003DD` only), `0085AB50`, `007F6190`, `00857834` fragment.
Data `00D7A268`, `00D0D098`, `00D7A328`, `00D7A218`, `00D0C5E0`, `00D0C5E8`, `00D0C5EC`,
`00CEFF98`.

Report: `reports/cc7_torpedo_launch_accuracy.json`. **No header, no source**: see section 8.

This document follows `docs/TORPEDO_CATEGORY_ADMISSION.md`, which made category 7 fire at
all. On the 3200-frame USN02 mission the rebuilt `bsp_game.exe` now launches 16 torpedoes
and hits nothing: `entity_impacts` held at 163 while `water` rose by 12 and `expired` by 3.
The packet asks whether that is faithful. **It is not.** Four separate host defects are
proven below, and the largest of them is not in the intercept solver at all.

Counters quoted from the run are the ones supplied with the packet. This turn could not
produce a fresh run: `bsp_game.exe` aborts with an access violation
(`0xC0000005`) immediately after `Phase 5 load_game_settings` /
`settings resolution=2560x1440 ...`, before the renderer device exists. Both this
worktree's build and the main checkout's prebuilt binary abort at the same line, so the
condition is environmental and not this branch's. **Every claim below is therefore static:
the listing, the installed authored tables and arithmetic over both.** Nothing here is
run-validated. `docs/WORKER_VERIFICATION_CHECKLIST.md` rule 6 is **not satisfied** and the
prescriptions in section 7 must be measured before they are believed.

## 1. What `008FBB00` solves

`008FBB00` `__fastcall(ECX shooter, EDX target, float speed, const float3* target_velocity,
float3* out) -> bool`, `RET 0Ch`, body `008FBB00`-`008FBC08`. Coverage: **complete**.
It is a wrapper: `008FB8D0` (`RET 10h`, body `008FB8D0`-`008FBAFB`, coverage **complete**)
answers a root count and up to two times, and the wrapper turns the chosen time into

```
out = target + t * target_velocity                  ; 008FBB3C and 008FBB9E
```

**It is a plain constant-velocity lead.** The target's velocity is sampled once and
extrapolated linearly; there is no turn rate, no acceleration, no iteration and no gravity.
The failure return is `AL = 0` at `008FBC00` with `out` **not written**; the caller checks it
(`00900263 TEST AL,AL` / `00900265 JNZ`) and abandons the shot through `gun->vtable[1E8h](0)`.

`008FB8D0` builds, with `d = shooter - target` and `v = target_velocity`:

| site | value |
| --- | --- |
| `008FB8D3`..`008FB8EF` | `d = shooter - target` |
| `008FB933`..`008FB945` | `a = dot(v,v) - speed*speed` |
| `008FB949`..`008FB96B` | `b = dot(d,v)` |
| `008FB96F`..`008FB97D` | `c = dot(d,d)` |
| `008FB984`, `008FB98E` | the quadratic arm needs `a` outside `+/-1e-4` (`00D7A268`, `00D0D098`) |
| `008FBA02` | `disc = b*b - 4*a*c`; the `4` is the double at `00D7A328` |
| `008FBA1F` | a negative discriminant answers `0` |
| `008FBA21` | `sqrt` through `00BF7030` |
| `008FBA2C`, `008FBA40` | the divisor is `a + a`; the branch is `a` against `0.0f` (`00D7A218`) |
| `008FBA44` / `008FBA95` | the far root first, then the near one |
| `008FBA6D`, `008FBABE` | a negative far root answers `0`, after it has been written |
| `008FBAE4` | `2` when the near root is `>= 0`, otherwise `1` |
| `008FB99C`..`008FB9F7` | degenerate `a`: `t = c / b`, a negative `t` written back as `0.0f` with the answer still `1` |

The exact constant-bearing intercept is `a t^2 - 2 dot(d,v) t + c = 0`. The native applies
the textbook `(b +/- sqrt(b*b - 4ac)) / 2a` to `b = dot(d,v)`, which is **half** the correct
linear coefficient, so the native under-leads every target that is neither stationary nor
exactly abeam. `docs/GUN_BOT_REMAINDER.md` section 2 established this and
`tests/math_tests.cpp` pins it; this packet re-read the listing independently and agrees.
`src/gun_bot_remainder.cpp:26`-`100` reproduces both arms faithfully, including the sign
branch, the negative-root refusals and the root-count answer. **The port is correct; do not
"fix" it.**

## 2. The speed argument: `WaterTravelSpeed`, not `V0`

This is item 2 of the packet and it is settled by the producer on both sides.

**Native.** `0090022B` in `008FFF20` loads the speed:

```
00900222  MOV EAX,[EBX + 3F8h]      ; EBX = [bot+58h], the gun
00900228  MOV EAX,[EAX + 34h]       ; the projectile class descriptor
0090022B  FLD  float [EAX + 0E4h]   ; <- the speed argument
0090024A  FSTP float [ESP]          ; arg1 of 008FBB00
0090025E  CALL 008FBB00
```

`docs/WEAPON_CLASS_DESCRIPTOR.md` records the readers of that same descriptor object:
`006E8770` writes **`V0` at `+50h`**, and `MTorpedo`'s override `008566B0` writes
**`WaterTravelSpeed` at `+E4h`** (with `MaxWaterHitVel` `+DCh`, `MaxFall` `+E0h`,
`HeadingTurn` `+E8h` around it). The two fields are on one object and the bot deliberately
takes `+E4h`. The AAFlakBot's own solver call at `009031CF` takes `+50h` from the same
`[[gun+n]+34h]` node, which is what makes the choice deliberate rather than incidental.

**Host.** `src/game_hosts_gunnery.cpp:436` sets
`gun.muzzle_speed = flat_scaled(type_id, make("v0"), ...)`, and the flatten chunk at
`src/game_hosts_gunnery.cpp:296` is `f[q .. 'v0'] = num(bc.V0, 1000) or 0` — the **bullet
class `V0`**, i.e. `+50h`. `src/game_hosts_gunnery.cpp:1191` passes that into
`torpedo_intercept_point_008fbb00`.

**So the host passes the wrong field.** For the Mark 15 (section 4) it passes `13` where the
native passes `51.444`.

## 3. `00857834`: what the round actually swims at

The speed the *bot* assumes and the speed the *round* travels at are not the same number
either, and neither is `V0`.

A fragment with no Ghidra function, read from the raw listing, initialises the torpedo
record from its class descriptor:

```
0085782E  MOV EAX,[ESI + 314h]          ; the projectile class descriptor on the record
00857834  FLD  float [EAX + 0E4h]       ; WaterTravelSpeed
00857846  LEA EBP,[ESI + 474h]
00857850  MOVSS [EBP],XMM0              ; record+474h = 0.5999994f   (00D0C5EC)
0085785D  FMUL qword [00D0C5E0]         ; * 0.59999943 (double)
00857863  LEA EBX,[ESI + 478h]
00857869  MOVSS [EBX],XMM0              ; record+478h = 3.0000007f   (00D0C5E8)
00857875  FSTP float [ESI + 470h]       ; record+470h = WaterTravelSpeed * 0.6
```

Coverage: **partial fragment, `0085782E`..`00857875` inclusive, final instruction
`FSTP float [ESI+470h]`.** The enclosing routine has no Ghidra function and was not read.
`0085786F` is mid-instruction; the stream resyncs at `0085786D`
(`MOVSS XMM0,[00CE5380]`), which is why the two drag stores land where they do.

`record+474h` and `record+478h` are the axial and lateral coefficients
`docs/TORPEDO_TICK.md` step 5 names, and `record+470h` is the field its step 6 says the tail
reads. `record+470h = WaterTravelSpeed * 0.6 = 30.87 m/s` for the Mark 15, and

```
descriptor+60h = WaterTravelSpeed * FlyTime * 0.6   (00855A90, the same 0.6 at 00CEFF98)
               = (record+470h) * FlyTime = 30.87 * 60 = 1852 m
```

The engagement range is exactly the swim speed times the fly time. That closes the circle
and identifies `+470h` as the swim speed.

**Correction to `docs/TORPEDO_TICK.md`.** That document's section on the swim says: *"There
is no thrust term here and none in `00856BB0`, so `WaterTravelSpeed` (`classDesc+E4h`) does
not reach the swim: the torpedo coasts on the velocity it entered the water with."* The
clause **"`WaterTravelSpeed` (`classDesc+E4h`) does not reach the swim" is refuted**:
`00857834` loads `classDesc+0E4h` and `00857875` stores `WaterTravelSpeed * 0.6` into
`record+470h`, the very field that document's step 6 records the tick tail as reading. The
coasting reading was taken from a listing window that did not contain the initialiser.
What `record+470h` is *used for* inside the tick is still unread, so "the swim speed" is the
inference from the range identity above, not a read of the consumer.

## 4. The authored numbers, this installation

`scripts/datatables/classtables/arcade/bulletclasses.lua`, mtime **2026-05-09 23:04**
(this installation is modded, BSPRM/AlterBSP; the untouched datatables bulk is 2024-07-13).
`ArcadeTable[62]`, `"21. Mark 15 ship torpedo"`, the round of the US double/triple/quintuple
tubes (`deviceclasses.lua` 62, 63, 65, mtime 2026-05-09 22:37):

| key | value |
| --- | --- |
| `V0` | `13` |
| `WaterTravelSpeed` | `51.444` (= 100 kn) |
| `FlyTime` | `60` |
| `Range` | absent |

Derived: swim speed `30.87 m/s`, engagement range `1852.0 m`.
Target hull, `vehicleclasses.lua` (mtime 2026-05-09 21:52), Fletcher class:
`Length 110`, `MaxSpeed 18.777 m/s`.

## 5. The arc, and what 40017 refusals mean

**`arc_blocked` is a per-tick counter, not a shot counter.**
`src/game_hosts_gunnery.cpp:1294`-`1300` increments it once per gun per tick whenever the
gun has a target, its aim has settled and `gun_fire_allowed_007f60a0` refuses. 71 mounts over
3200 frames could contribute 227,200. 40017 is 18% of that ceiling, and it is not 40017
refused shots.

**The authored window.** Every torpedo platform in `vehicleclasses.lua` authors exactly one
window, and **every one of them has `MinVertAngle == MaxVertAngle == 0`**. Horizontally the
tubes get one broadside sector — `h[50,118]`, `h[62,114]`, `h[0,50]`, `h[-118,-50]` and
similar, 50 to 68 degrees wide. Submarine catapults (devices 66, 73) author
`h[0,0] v[0,0]`: a single point, dead ahead. Rest angles are `(0,0)` on 96 of the 205
torpedo platforms. So a fixed-sector tube legitimately refuses most headings, and a mount
parked at rest is outside its own firing sector.

**What the native does with a heading outside the sector — it does not refuse it.**
`008FFF20` step 8 calls `0085AB50(gun, h, pi/4)` at `00900380`; `0085AB50` (`RET 8`, body
`0085AB50`-`0085AB9A`, coverage **complete**) bounds-checks the platform index and delegates
at `0085AB91` to `007F6190(platform)(float horz, float limit)`, body `007F6190`-`007F64EB`,
coverage **complete**:

```
find the first arc record whose HORIZONTAL bounds hold horz (clamped to +/-pi,
  widened by 00D08B88 = half a degree).  No flag test, no vertical test.
none                       -> FLT_MAX
that record has the fire bit -> horz unchanged
otherwise: walk backwards, then forwards, through records that carry the traverse
  bit; at the first record on each side that also carries the fire bit, take its
  nearer bound (+4h min, +8h max) by wrapped angular distance; keep the nearer of
  the two candidates
  none, or |wrapped(candidate - horz)| > limit -> FLT_MAX
  else                                          -> candidate
```

So the native **snaps the commanded heading up to 45 degrees onto the nearest firing-window
edge** and fires along that edge; it abandons the shot (`00900392`..`009003A2` against
`FLT_MAX` at `00D7A278`) only when no window's horizontal bounds hold the heading at all, or
when the snap would exceed 45 degrees.

**What the native then commands vertically — zero.** At `009003CE`:

```
00900389  FLD  float [ESP+0Ch]      ; the filtered heading
0090038D  FST  float [ESI+60h]      ; bot+60h
009003A2  JP   009003CE             ; taken when it is not FLT_MAX; the heading stays in ST0
009003CE  FLDZ
009003D6  FSTP float [ESP+4]        ; arg2 = 0.0f, the vertical
009003DA  FSTP float [ESP]          ; arg1 = the heading
009003DD  CALL 0085ABA0
```

**The host does neither.** `src/game_hosts_gunnery.cpp:1272` passes the raw `want_horz` and a
`want_vert` of `asin(dot(unit_delta, up))` straight into
`gun_set_target_angles_0085aba0`, which (`src/gun_aiming.cpp:120`) **refuses outright** when
no window holds the pair — it does not clamp and it does not snap. Against a `v[0,0]` window
widened by half a degree (`kGunAimArcEpsilon`, `src/gun_aiming.cpp:88`) a non-zero
`want_vert` refuses whenever the depression to the target exceeds 0.5 degrees, i.e. whenever
`mount height / range > 0.0087` — which is every mount closer than roughly 115 times its
height above the waterline. That is the `no_window = 37734` column, and the mounts it leaves
parked at their `(0,0)` rest angles outside a `h[50,118]` sector are most of the
`arc_blocked = 40017` column.

**Verdict on item 3: 40017 is not faithful.** The narrow authored sector is real and a
correct model would still refuse a lot, but the refusals in this run are dominated by two
host modelling errors — a vertical command the native does not make, and a missing heading
snap — rather than by the authored arcs.

## 6. The expected hit rate

`local/torp_hit_bound.py` sweeps all 3600 aspect angles at each geometry, applies the native
solver exactly as section 1 recovered it, compares the lead it produces with the time the
round actually needs at its `30.87 m/s` swim speed, and calls it a hit when the cross-track
error is inside the presented half-width of a 110 x 11 m hull.

| range | target speed | host now, `s = V0 = 13` | native, `s = WaterTravelSpeed = 51.444` | half-b solver at the true swim speed |
| --- | --- | --- | --- | --- |
| 637 m | 18.78 m/s | 0.0 % | 7.1 % | 42.6 % |
| 637 m | 10.0 m/s | 1.2 % | 17.3 % | 100 % |
| 1000 m | 18.78 m/s | 0.0 % | 3.0 % | 38.3 % |
| 1000 m | 10.0 m/s | 0.7 % | 5.4 % | 80.1 % |
| 1852 m | 18.78 m/s | 0.0 % | 1.3 % | 13.8 % |
| 1852 m | 10.0 m/s | 0.3 % | 2.1 % | 58.2 % |

Two thresholds make the first column zero. With `s = 13` the coefficient `a = u^2 - s^2` is
**positive**, and `disc = b*b - 4ac >= 0` needs `cos^2(theta) >= 4 - 4 s^2 / u^2`, which has
no solution at all once the target is faster than `2s/sqrt(3) = 15.01 m/s`. A Fletcher at
18.78 m/s is past it, so **`008FB8D0` answers "no solution" for every aspect**, the host
takes `lead = at` (`src/game_hosts_gunnery.cpp:1194`) and aims at where the destroyer is
*now*. Over a 32 s run at 1000 m that ship has moved 600 m.

The third column is what a faithful model still gives up: even with the right speed the
half-b error leaves a beam-aspect deficit of 138 m at 637 m, 216 m at 1000 m and 401 m at
1852 m against a full-speed destroyer. **The shipped bot under-leads twice over** — once from
the half-b coefficient, once from leading at `WaterTravelSpeed` when the round swims at
`0.6 * WaterTravelSpeed` — and against a 36-knot destroyer at these ranges it should hit a
few per cent of the time, mostly on near bow-on and stern-on aspects where a lead error does
not become a cross-track error.

**Verdict on item 4.** 0 hits from 16 launches is *numerically* unsurprising, but it is not
faithful, because in this build no torpedo can reach any target at all:

**The fourth and largest defect.** `src/game_hosts_gunnery.cpp:1582` kills any projectile
that crosses `y = 0` downward, unconditionally:

```cpp
if (shot.position[1] <= 0.0f && from[1] > 0.0f) { ++summary.water_crossings; ... shot.alive = false; }
```

There is no swim phase. A torpedo launched from a deck a few metres up along a slightly
depressed vector drowns within tens of metres. `water` rising by 12 of 16 shots is exactly
that, and the other 3 `expired` because `src/game_hosts_gunnery.cpp:1591` runs them at
`row->muzzle_speed = 13 m/s` (`1852 / 13 = 142 s`) instead of the 30.87 m/s that makes the
authored range and `FlyTime` agree. **A correct solver would have changed nothing in this
run.**

## 7. Prescription for the host

The integrator owns every file below; this packet edits none of them.

1. **Keep the water speed on the gun row.** `src/game_hosts_gunnery.cpp:452` already reads
   `WaterTravelSpeed` to derive the range and then discards it. Store it
   (`gun.water_travel_speed`) alongside `gun.max_range`.
2. **Pass it, not `gun.muzzle_speed`, at `src/game_hosts_gunnery.cpp:1189`-`1191`**, and
   guard on it: `008FBB00`'s speed argument is `descriptor+0E4h` (`0090022B`), not
   `descriptor+50h`. For the Mark 15 that is `51.444`, not `13`.
3. **Command vertical zero for category 7.** `009003DD` passes `0.0f`; pass `0.0f` into
   `gun_set_target_angles_0085aba0` at `src/game_hosts_gunnery.cpp:1272`-`1273` when
   `gun.category == kUnitGunneryTorpedoCategory`, instead of the computed `want_vert`.
   Every torpedo window authors `v[0,0]`, so any other value refuses.
4. **Snap the heading instead of refusing it.** Reconstruct `007F6190` (section 5) and run
   `want_horz` through it with `limit = pi/4` before `0085ABA0`, for category 7 only
   (`00900380`). Abandon the shot on `FLT_MAX`, as `009003A2` does. This is the change that
   should move `no_window` and `arc_blocked`.
5. **Give the round a swim phase.** On the downward `y = 0` crossing
   (`src/game_hosts_gunnery.cpp:1582`), a category 7 shot must not die: flatten its velocity
   to the horizontal and set its speed to `WaterTravelSpeed * 0.6` (`record+470h`,
   `00857875`). The existing expiry at `src/game_hosts_gunnery.cpp:1591` then fires at
   exactly `FlyTime`, because `range / (WaterTravelSpeed * 0.6) == FlyTime` identically.
   This is the projectile step and overlaps `docs/TORPEDO_TICK.md`'s packet; it is the
   change that decides whether anything can hit, and it should be sequenced first.

Expected effect: (5) alone lets torpedoes reach the target; (2) turns "no lead at all" into
the native's lead; (3) and (4) open the launch windows. Even with all four the hit rate
against a full-speed destroyer stays in the low tens of per cent at 637 m and the low
teens at 1852 m, per section 6. **These are predictions, not measurements** — the run that
would check them could not be made this turn.

## 8. What is proven and what is assumed

**Proven from the listing.**
- `008FBB00` returns `target + t * target_velocity` and nothing else; the failure return
  leaves `out` untouched and `00900263` checks it.
- `008FB8D0`'s four coefficients, its `b*b - 4ac` discriminant, its `2a` divisor, its root
  ordering and its three return values.
- `0090022B` takes the speed from `descriptor + 0E4h`.
- `009003DD` passes `0.0f` as the vertical.
- `00900380` -> `0085AB91` -> `007F6190`, and `007F6190`'s snap-or-`FLT_MAX` contract.
- `00857834` loads `classDesc+0E4h`; `00857875` stores `* 0.6` into `record+470h`.

**Proven from the installed authored data.** The Mark 15's `V0`, `WaterTravelSpeed`,
`FlyTime` and absent `Range`; that every torpedo platform authors one window with a
zero-height vertical span; the Fletcher's length and speed.

**Proven from the host sources.** Which field `gun.muzzle_speed` holds; that
`gun_set_target_angles_0085aba0` refuses rather than clamps; that the projectile step kills
every downward water crossing.

**Assumed.**
- That `record+470h` is the swim *speed*. The identity
  `descriptor+60h = (record+470h) * FlyTime` is exact, and `docs/TORPEDO_TICK.md` records the
  tick tail reading `+470h`, but the consumer was not read.
- The hit-rate table's collision model: a rectangular presented width and a first-order run
  time `r / 30.87`. It bounds the order of magnitude, not the exact percentage.
- That the USN02 targets behave like a Fletcher. The mission's actual target classes were
  not enumerated.

**Not established.**
- Any run-time confirmation (section 0). The executable would not start this turn.
- Whether `008FFF20`'s heading pipeline (`00900328`..`00900380`: normalise, derived affine
  inverse `00414E10`, transform `0042D0D0`, `00521370` to a pair, negate) agrees with the
  host's `angles_from_world_direction_008FDAF0`. Only the filter at the end of it was read.
- What `00857834`'s enclosing routine is, and when it runs.

## 9. Follow-up packets

1. **`torpedo_swim_phase`** — read `00856BB0` and the `00857531`..`00857679` swim step for the
   consumer of `record+470h`, settle whether `+474h`/`+478h` are drags around a cruise speed
   or pure decay, and give the host a category 7 projectile branch. Blocks everything else.
   Owns `docs/TORPEDO_TICK.md`'s correction.
2. **`gun_platform_heading_snap`** — reconstruct `007F6190` and wire it at `00900380`'s
   position. Also covers `00959C20`'s kind 4 arm at `0095A238`, which abandons on the same
   `FLT_MAX`.
3. **`torpedo_bot_heading_pipeline`** — `00900328`..`00900380`, to confirm or replace the
   host's use of `008FDAF0` for category 7.
4. **`torpedo_record_init`** — bound `00857834`'s enclosing routine, name it, and record the
   full record layout around `+470h`..`+4A0h`.
