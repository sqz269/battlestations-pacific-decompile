# AA lead: the fighters' guns, the intercept, and the dual-purpose air round

Addresses: `007CE9A0` (the plane fixed step's trigger loop, `007CE9A0`-`007CE9F4`), `0072D2C0`
(BSP_Gun_SetTriggerHeld), `00901C20` (BSP_GunBot_InterceptSolution), `00902920`, `009030C0`,
`008FBE00`, `008FBFC0`, `00729BC0`, `00729B90`, `005459E0`, `006DF520`.

Packet `cc9_aa_lead`, 2026-09-23. All pairs have `BSP_GUNNERY_RNG_STREAMS=1` on both sides.

## 1. The fighter hook (`kPlaneGunfireHooked`)

`docs/PLANE_GUN_PASS.md` section 2 read the path. The plane's fixed step `007CE040` hands its
latched `gunFire` (`unit+BC9h`) to every enabled fixed gun's `SetTriggerHeld` (`007CE9A0`-`007CE9F4`).
The units host publishes that byte as `units.plane_gun_trigger_bc9()`. From the trigger onward the
gunnery host already models the path: `0072D2C0` latches, `0072D130` sends one fire message per
step, `FireIfReady` and `CanFire` check the reload, and `BSP_Gun_Fire` spawns the round. Plane guns
never fired only because the host formed `want_fire` from bot targeting, and a plane's forward
guns have no bot.

**Binding.** For a category 0 (PLANEGUN) gun on a plane, `want_fire` is the published trigger.
The weapon-group enable byte (`[[unit+538h]+94h][gun+38Ch]+0Ch`) is taken as on, which is
labelled.

## 2. The intercept `00901C20`, read

`__fastcall(const float3* shooterPos ECX, Entity* target EDX)(float projSpeed, const float3*
shooterVel, float3* outAimPoint, float* outDist)`, `RET 10h`. The body is `00901C20`-`0090227F`.
The gun-bot callers are the AAGunnerBot `00902920` at `00902A74` and the AAFlakBot `009030C0` at
`009031CF` and `00903219`. Both pass the round's V0 (`+50h`). Ghidra lists seven more call sites,
not read here and not a complete census: `00526F27` and `005270FA` in `00526A40`, `00640B15` in
`00640A20`, `00902379` in `00902290`, `00957A00` in `BSP_Aim_ResolveRayToWorldPoint`, `00957CA6` in
`00957BD0`, and `0070C680` in `BSP_FlakProjectile_TickAdvance`.

1. **Speed floor.** When `projSpeed < 2.0f` (`00CE3958`), the aim point is the target's pose
   position `+FCh..+104h`, and `outDist` is the distance to it.
2. **Velocity.** `V = target->vtable[34h]()`. When the target is a plane (`vtable[5Ch](0Fh)`) with
   an angular rate at `+AF8h..+B00h` whose square exceeds `0.001f` (`00D7A23C`), `0085E4D0` builds
   a turn-rate matrix. V is then averaged with its rotated image times `0.5` (`00D7A280`).
3. **Relative motion.** `V -= shooterVel`. A ship target (`vtable[5Ch](6)`) has its vertical
   component zeroed.
4. **Geometry.** `D = targetPos - shooterPos`, `d = |D|`. When `d <= 1.0` the aim point is the
   target's position.
5. **The time.** Let `a = V·D/d` and `vv = |V|^2`.
   - If `vv - a^2 >= 0.01f` (`00D7A238`), the motion is not radial. Then, when
     `vv * 1.5 < s^2` (`00CE3D78`, double), it solves `(vv - s^2)t^2 + 2da·t + d^2 = 0`, with
     discriminant `(2da)^2 - 4.0 (00D7A328) (vv - s^2) d^2`. It takes the root `t1` unless that
     is negative or the other root `t2` is non-negative and smaller; a negative result becomes 0.
     When the speed gate fails, t stays 0.
   - Otherwise it is the linear `t = d / (s - a)`, floored at 0.
6. **Clamp.** `BSP_Math_ClampInPlace` bounds t to `[0, 12]` (`00CEB4B8`).
7. **Biases.** `t += settings+758h` and `t += (d / 1000.0 (00CE47A0)) * settings+75Ch`. These are
   `ShipGlobals.AAGunnerErrorModifier.CalcTargetPosTimeAddFix` and `.CalcTargetPosTimeAddMul`
   (`docs/GAMEPLAY_SETTINGS.md`): loader defaults 0, installed 0.05 s and 0.1 s per km. The host
   now reads both from the live `ShipGlobals` and prints them at load.
8. **Output.** The aim point is `targetPos + t*V`, and `outDist = |aim - shooterPos|`.

There is no gravity, no `FSQRT` other than the library `sqrt`, and no iteration. The host
before this packet used `targetPos + V_target * d / V0`: no shooter velocity, no quadratic and
no settings biases.

**Binding (`kGunInterceptBound`).** Steps 3 to 8 run in the AA branch (categories 1 and 5, and
6 against a plane once item 3 is on). Step 2's turn-rate averaging is a labelled substitution,
because the host carries no plane angular velocity. The plane's velocity is its body axis times
`0092D730`'s speed, as in `docs/GUN_BALLISTICS.md`.

**The artillery bot's raised point.** `006DF520` step 5 aims at `target origin + ErrorOffset`,
where `ErrorOffset` (`bot+84h..+8Ch`) steps toward a point `target->vtable[100h]` picks. That is a
section or lead point chosen with `descriptor + 18h..24h`: TargetPointRefreshTime,
SectionTargetChance and the three section weights. It is not the Height-raised visibility point
the host uses. Binding it needs `vtable[100h]`'s section picker, which is not read here, so the
host's artillery aim point stays a substitution.

## 3. The dual-purpose second ammunition (`kDualPurposeSecondAmmoBound`)

- **Authoring.** A LIGHTARTILLERYFLAK device's `Bullet` table holds two records, for example
  `Bullet[1].Bullet = 3` (Artillery) and `Bullet[2].Bullet = 44` (Flak) on the King George 5.25"
  DP mount. The weapon descriptor points to them through `[+74h]`, stride `48h` (`00729C4B`: `LEA ECX,[EDI+EDI*8]`, then `[EDX+ECX*8+34h]`), class at `+34h`, so the second class is `[+74h]+7Ch`.
- **Selection.** `00729BC0` takes variant 1 (the second record) when the weapon kind is 6 and the
  target answers `IsKindOf(0Fh)`. It then dispatches on that round's projectile kind: Flak (10h)
  goes to slot `+394h`, the AAFlakBot. `00729B90` reads the same variant's `MinRange` (`+58h`),
  and `00729BC0` its range (`[proj+60h]`).
- **The host** loaded only `Bullet[1]` and counted the two records as two barrels (the flatten's
  `bn`). The barrel count is left as it is; the open item is in section 7.
- **Binding.** For a category 6 gun against a plane, the host now uses `Bullet[2]`:
  - its derived range in the acceptance test;
  - its `MinRange` in the kind-6 minimum-air-range skip, which the RNG-stream packet had left at 0;
  - the AAFlakBot law, meaning the intercept lead with no arc and no aim-error envelope;
  - its V0 and class at spawn, so its `NoGravity`;
  - its class for hit and blast damage, through a round-class override around `apply_hit` and
    `apply_impact_blast`.

## 4. Predictions, written before each treatment build

The controls are `local\l0_4500.log` and `local\l0_9000.log`: all three new switches off, on top
of main `3412839fd`. In the 9000 control, Yorktown-class01_sqn02|.-3 has one burst of 6 trigger
ticks on D3A Val #3.1|.-3 at about 845 m, and its six PLANEGUN rows (558-563) fire 0 rounds.

**T1, fighter hook, 9000.**
- About 18 rounds, 12 to 36: six guns and 6 trigger ticks, with a reload near 0.1 s.
- **0 hits**, because the burst is at about 845 m, beyond the Bullet classes' 800 m range, so
  the rounds expire first.
- Rows 558-563 move and nothing else, since their fire-stagger draws are on their own stream keys.

**T2, T1 plus intercept.**
- Category 1 and 5 rows move, plus downstream rows.
- For an approaching aircraft the quadratic gives less lead than `d/V0`, and the settings biases
  add 0.05 s plus 0.1 s per km, about 0.12 s at 700 m.
- **Direction uncertain; hit rate within ±25% of T1.** Categories 3, 7 and 8 do not move.

**T3, T2 plus the second ammunition.**
- **Category 6 rows that engage aircraft change law.** Their V0 goes 300 to 800, gravity off,
  and they take the intercept, no arc and no envelope, so **category 6 hits on aircraft rise**.
- Category 6 guns aimed only at ships (none in USN04) would not move.

**T2x, a measurement build: T2 with the two settings biases dropped.** Written after T2 failed its
prediction and before the run. The environment option `BSP_AA_TIME_BIAS_OFF=1` is never set in a
reference run. If the biases cause T2's drop, category 1 hits at 4500 frames return to 190 or more,
near T1's 204. If the quadratic causes it, they stay near T2's 152.

**T4, the halving of negative verticals, on top of the landed switches.**
- Only category 1 rows whose target sits below the mount's elevation zero move: low torpedo
  bombers inside a few hundred metres, and Vals late in their pull-out.
- A halved negative vertical aims above the target, so **those rows' hits fall**. The count of
  halvings is small, under 5% of category 1 aim ticks.
- No other category moves except through kills and survivals.

## 5. Pairs

USN04, `--frames 4700 --press-start-frame 30 --menu-select USN04 --mission-frames 4500
--mission-frame-seconds 0.05`, and the same with 9200/9000. Every log is under the worktree's
`local\`. Hits are gun-row hits; deaths are the damage summary's.

**T1, the fighter hook: `l0_*` against `l1_*`. The prediction held.**

| Run | Gun rows moved | Category 0 rounds | Category 0 hits | Unit rows moved | Deaths |
|---|---|---|---|---|---|
| 4500 | 6 of 726, all category 0 | 0 to 35 | 0 | 0 of 71 | 24 to 24 |
| 9000 | 6 of 726, all category 0 | 0 to 35 | 0 | 0 of 71 | 35 to 35 |

- The six moved rows are 558-563, Yorktown-class01_sqn02|.-3's PLANEGUNs.
- The trigger is held on 72 ticks. 35 rounds is inside the predicted 12 to 36.
- No round hits: the burst is at about 845 m and the rounds expire first.

**T2, the intercept: `l1_*` against `l2_*`. The prediction failed.**

| Run | Category 1 shots | Category 1 hits | Hits per shot | Category 5 hits | Deaths |
|---|---|---|---|---|---|
| 4500, T1 | 1776 | 204 | 11.5% | 10 | 24 |
| 4500, T2 | 2654 | 152 | 5.7% | 10 | 19 |
| 9000, T1 | 5773 | 254 | 4.4% | 12 | 35 |
| 9000, T2 | 10732 | 243 | 2.3% | 28 | 33 |

- The rate per shot halves, far outside the predicted ±25%.
- Shots rise because fewer aircraft die: category 1 target assignments go 4346 to 5339 at 4500.
  The category 6 shot rise, 473 to 871, is the same consequence; its law did not change.
- The first 9000 treatment run died with 0xC0000005 at mission frame 764 while the session was
  switching to Remote Desktop (`local\stale\l2_9000_crash.log`). The re-run completed normally.

**Attribution, two measurement builds at 4500.** Each drops one term through an environment
option that no reference run sets.

| Build | Option | Category 1 shots | Hits | Hits per shot |
|---|---|---|---|---|
| T2 | none | 2654 | 152 | 5.7% |
| T2x, no settings biases | `BSP_AA_TIME_BIAS_OFF=1` | 2648 | 142 | 5.4% |
| T2v, shooter velocity kept | `BSP_AA_NO_SHOOTER_VEL=1` | 2158 | 179 | 8.3% |

- The settings biases do not cause the drop. Removing them lowers hits slightly.
- About half the loss comes from subtracting the shooter's velocity. 006E8430 launches a round
  at `speed * direction` and does not add the shooter's velocity (`include/bsp/projectile_impact.hpp`),
  so the image leads for motion its rounds do not have. This is an inconsistency in the image,
  and the host now reproduces it.
- The rest comes from the time law, the quadratic in place of `d / V0`. The host's plane
  velocity substitution and the missing turn-rate average are shared by both sides, so they do not
  explain the difference by themselves, but they may interact with it.

**T3, the second ammunition: `l2_*` against `l3_*`. The prediction held.**

| Run | Category 6 shots | Air rounds on `Bullet[2]` | Category 6 hits | Category 6 dealt | Deaths |
|---|---|---|---|---|---|
| 4500, T2 | 871 | 0 | 3 | 220.0 | 19 |
| 4500, T3 | 828 | 828 | 93 | 1188.3 | 18 |
| 9000, T2 | 2248 | 0 | 12 | 880.0 | 33 |
| 9000, T3 | 2872 | 2872 | 493 | 1965.9 | 32 |

- USN04 has no enemy ship, so every category 6 round is an air round and every category 6 row
  changes law. The artillery arc and the aim-error envelope stop running: `artillery_arc_aims`
  and `aim_error_rerolls` fall to 0 at 4500.
- The 241 minimum-range skips at 4500 are the Flak class's own `MinRange`, which the kind-6
  air skip had read as 0.
- Category 1 and 5 rows move through changed kills and survivals. At 4500, category 1 hits go
  152 to 119 on 2654 to 2269 shots.
- **At 9000, category 1 hits go 243 to 2412 while damage dealt falls 6027 to 4824.** The
  extra hits land on friendly hulls for no damage: the unit table's hits taken rise by 330 on
  Northampton-class01, 336 on Northampton-class02, 597 on Fletcher-class03 and 501 on
  Fletcher-class04, and no aircraft takes more than 20. D3A Val #1.1|.-4 died at 226 s in T2
  and survives in T3. In all three 9000 runs it fails its pull-out and touches the water
  (`plane water contact ... state 7 -> 6`, whose surface law 007DCDD0 is still a host contract).
  In T3 nobody kills it first, so it sits on the surface as a live target. The mounts that fire
  on it aim below the horizontal, and the likely result, not verified round by round, is rounds
  into the hulls around it. Category 1's law is the same on both sides, so this follows from a
  changed survival. It makes category 1 hit counts after about 225 s unusable as an accuracy
  measure in this mission, and it is an open item.

**T4, the halving of negative verticals: `l3_*` against `l4_*`. The prediction partly failed.**

| Run | Halvings | Category 1 shots | Category 1 hits | Category 1 dealt | Deaths |
|---|---|---|---|---|---|
| 4500, T3 | switch off | 2269 | 119 | 2990.5 | 18 |
| 4500, T4 | 9514 | 2341 | 118 | 3148.7 | 18 |
| 9000, T3 | switch off | 16841 | 2412 | 4823.8 | 32 |
| 9000, T4 | 291773 | 17565 | 909 | 5003.9 | 32 |

- At 4500, 128 category 1 rows move, but their hits stay level at 119 to 118 while damage rises.
  The predicted fall in hits did not appear. The first hit moves from 110.90 s to 118.15 s.
- The halving count is under 5% of aim solutions at 4500, as predicted: 9514 of 355177.
- At 9000 the count reaches 291773, almost all of it after 225 s against the Val on the water.
  Halving those downward commands likely sends fewer rounds into the friendly hulls around it: hits fall
  2412 to 909 while damage dealt rises. Fletcher-class04 still takes 649 of them.
- Categories 5 and 6 move only through kills and survivals.

## 6. Decisions

- **`kPlaneGunfireHooked`: landed, default true.** Every moved row is a fighter round, and no
  other row or unit moves.
- **`kGunInterceptBound`: landed, default true, although its prediction failed.** Each term is
  read from 00901C20 with its constants. The two measurement builds attribute the hit-rate
  drop to terms in the image: about half comes from the shooter-velocity subtraction, which the
  image's rounds do not inherit, and the rest from the quadratic time. The settings biases do not
  cause it. The lead may prefer to hold this switch until the plane velocity substitution and
  the turn-rate average are bound; both are labelled in the code.
- **`kDualPurposeSecondAmmoBound`: landed, default true.** The prediction held: every category
  6 air round now uses `Bullet[2]`, and category 6 hits on aircraft go 3 to 93 at 4500.
- **`kAaGunnerErrorBound`: landed, default true, as the halving step only.** The step is read at
  00902F62..00902F76. Its aggregate effect at 4500 is flat, so the pair neither confirms nor
  contradicts it beyond the moved rows. The swinging error pair, 00902B38..00902EF7, is not bound:
  it needs the unread `vtable[100h]` sample and the 00BD2F90 split.
- The two measurement options, `BSP_AA_TIME_BIAS_OFF` and `BSP_AA_NO_SHOOTER_VEL`, stay in the
  host, default off, labelled as measurement options that no reference run sets.

## 7. Open items

- **A plane on the water stays a live AA target.** D3A Val #1.1|.-4 touches the water at about
  225 s in USN04 and, when nothing kills it first, draws AA fire for the rest of the mission. The
  water surface law 007DCDD0 is a host contract. Until it is bound, AA hit counts after the
  first water contact do not measure accuracy.
- **The DP barrel count.** The flatten counts a device's `Bullet` records as barrels, so a DP
  mount reads as two barrels. The real barrel count is not read here.
- **The artillery bot's aim point.** 006DF520 aims at a section point from `target->vtable[100h]`,
  not at the Height-raised point the host uses. See section 2.
- **The turn-rate average.** 00901C20's `0085E4D0` step for a turning plane needs the angular
  velocity at `+AF8h`, which the host does not carry.
- **The swinging error pair** in 00902920, as above.
- **The other 00901C20 callers**, seven sites listed in section 2, including
  `BSP_FlakProjectile_TickAdvance`, are not read.
