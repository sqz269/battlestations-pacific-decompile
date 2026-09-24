# The fighter gun's lead point and fine aim

Addresses: 009FC7C0 (009FC931-009FC9F6, 009FCADF-009FCE21), 00954650, 0042B2F0, 004F9B30,
00952340, 0085C3F0, 009F9FC0, 007C2610, 0099E756-0099E81A, 009FA7E0 (not read).

Packet `cc9_fighter_gun_lead`. Every name is a hypothesis, not a recovered symbol. Read from the
disk bytes (`bsp.py disasm-raw`); Ghidra was not written. The switch is `kFighterGunLeadBound` in
`src/game_hosts_units.cpp`. Its predecessor is `docs/FIGHTER_GUNFIRE_RATE.md` section 4, which
found the unled host fire and could not bind the lead without the muzzle-speed accessor.

## 1. What the image does

**The lead point +5Ch (009FC931-009FC9F6).** Once the finder has put a live target in +74h:

1. d1 = |target position (00427EB0) - own unit+FCh|, t1 = d1 / 007C2610().
2. +5Ch = target->vtable[48h](&out, t1).
3. d2 = |+5Ch - own position|, t2 = d2 / 007C2610(), and +5Ch = target->vtable[48h](t2).

007C2610 is the minimum muzzle speed over the unit's fixed guns. The gunnery host's accessor
returns FLT_MAX when the unit has none, so t is about 0.

**The prediction, 00954650.** It is the plane class's vtable[48h] (vtable 00D19D28),
`__thiscall(out, t)`, `RET 8`.

* v = vtable[34h] 007BBB70, which is unit+AC8h, the world velocity. P = unit+FCh..104h.
* **Straight arm** (009548C5-00954937): out = P + v·t. It is taken when |v|² ≤ 1e-10 [00CE3820],
  or |v| ≤ 1.0, or the turn rate is at most [00D1A630].
* [00D1A630] is the double 0.008726646502812704, which is π/360 (half a degree).
* **Arc arm.** acc = unit+654h..65Ch, the finite-difference acceleration: 007CEE05-007CEE5B
  takes (unit+648h - unit+6BCh) / dt, where unit+648h is (position - previous position) / dt.
  * n = v / |v|, a⊥ = acc - (n·acc)·n, w = |a⊥| (0042B2F0, 0 at |a⊥|² ≤ 1e-10).
  * d = a⊥ / w points at the turn centre. The axis is d × n (004F9B30, out = a × b).
  * rate = w / |v|. The centre is c = P + (|v| / rate)·d.
  * out = 00952340(c, axis, P, -rate·t), the negation being the FCHS at 0095489A.
  * 00952340 is c + 0085C3F0(P - c, axis, θ). 0085C3F0 is Rodrigues' formula, par + cos θ·perp
    + sin θ·(axis × perp), with an identity return when |perp|² < 1e-4 [00D7A268].
  * With these conventions -rate·t moves the point along +n, which is the physical turn.

**The envelope (009FCADF-009FCC9E)** uses +5Ch - own position. The host's
`dogfight_gun_tick_009fc7c0` already models it; only its input changes.

**The fine aim (009FCCA0-009FCDB2).** It runs before the fire decision and does not gate it.

* The cone term is 1 - (world delta / d)·+68h (009FCB96-009FCBBB).
* +68h is the unit's forward row when it is zero (009FC829-009FC871, 0042D7E0()+20h). The tick
  clears +68h and +40h at 009FCE5A-009FCE69, and the dogfight task writes only +40h (the aim
  state's `Angle_Strafe`, 15°), so +68h is always the forward row here.
* The condition is +40h > cone (009FCCAB) and +4Ch ≤ 0 (009FCCB4). Outside the aim state +40h is 0,
  so the fine aim never runs there.
* The angles are (x / d, y / d) of the lead in the unit frame, with d the 3-D distance
  (009FCCBA-009FCCCC). The divisor is d, not z.
* 009FA7E0 on gun+4 then adds a distortion (+20h, +24h), divided by tuning+648h/+64Ch for kind
  13h owners with a qualifying target. **Not read; the host takes it as 0.**
* 009F9FC0(this, &angles, dt), then +49h = 1.

**009F9FC0** (`__thiscall(gun, float* angles, float dt)`, `RET 8`), per axis, from the class
[[task+2F4h]+538h]:

* stick = sign(e) · min(|8e| / (Spd · Spd / Accel), 1, damp). 8.0 is [00CE3DB0].
* Yaw uses angles[0] with YawSpd +1B0h and YawAccel +1C4h. Pitch uses angles[1] with PitchSpd
  +1ACh and PitchAccel +1C0h.
* damp is 1 unless gun+4Ah is set. The host models the damping as off, because the setter of
  +4Ah is not read. 009FCDE5 is the only clear seen.
* Writes at 009FA21F-009FA248: plan+29Ch = the pitch stick, plan+2A0h = 1, plan+2D0h = 0,
  plan+284h = the yaw stick, plan+288h = 1, and plan+2D4h = 0.

**What plan+2D4h does (0099E756).** The pilot planner 0099D300 tests plan+2D4h. Non-zero runs the
yaw arm 0099E81A, which rewrites plan+284h from the heading. Zero skips it (0099E764-0099E80D
touches only plan+2ECh). 0099B450 seeds plan+2D4h = 1 on every think. So the fine aim's yaw stick
reaches the stick for that think, and the host now skips `plan_yaw_0099d300`'s yaw write when the
fine aim ran.

For a Wildcat (PitchSpd 0.436, PitchAccel 1.222, YawSpd 0.227, YawAccel 0.524 in this
installation's vehicleclasses.lua), the pitch stick saturates beyond |e_v| ≈ 0.0195 and the yaw
stick beyond |e_h| ≈ 0.0123.

## 2. What the host does now (switch ON)

* `df_set_lead` evaluates the two passes with `df_predict_00954650` (both arms) and
  `GameGunneryHost::min_fixed_gun_muzzle_speed_007c2610`, then projects +5Ch into the unit frame.
  Both the finder path and the task-target path use it.
* `df_fine_aim_009f9fc0` runs after the gun tick, with the image's condition and stick law, and
  writes the pitch and yaw slots and pitch mode 0. It sets a flag the planner reads as plan+2D4h.
* **Substitution: the target's acceleration.** The image reads the target's own per-fixed-step
  finite difference. The host differences the target's world velocity over the gun tick's dt,
  per fighter, and uses zero on the first tick after a target change.
* **Substitution: the distortion 009FA7E0 is 0.** Unread.
* **Substitution: the damping is off.** The setter of gun+4Ah is unread.
* A census line per fighter, `fighter gun lead`, reports lead ticks, arc ticks, fine-aim ticks,
  the mean and largest lead shift, and the muzzle speed used.

## 3. Predictions (written before the pair)

The control is the same tree with the switch off. It should reproduce K0
(`local\K0_9000.log`, main at 4ab52ffe0) line for line.

| row | K0 | prediction for switch ON |
| --- | --- | --- |
| fighter plane-gun hits | 0 | rise to 1-40 |
| lead shift | 0 | mean 40-120 m (t about 1 s at 800 m, Val 60-130 m/s) |
| fighter bursts / trigger ticks | 10 / 263 | change little: bursts 5-25, ticks 100-600 |
| Val deaths to fighters | 0 | 0-2 |
| Kate rows (torpedo drops 16) | 16 | unchanged |
| Lexington lost (entity dead) | 259.06 s | unchanged |
| dive rows (bomb drops 25, water 7) | 25 / 7 | flat, within ±2 if a Val dies before release |

The fine aim changes the fighters' paths in the aim state, so the trigger ticks may move more than
the lead alone would move them. Ship AA outcomes can move through the aircraft paths even with
`BSP_GUNNERY_RNG_STREAMS=1`, if a Val dies or its path changes.

## 4. The pair (E2 9000, `BSP_GUNNERY_RNG_STREAMS=1` on both sides)

Logs: `local\L0_9000.log` (switch off, binary `local\gl0`) and `local\L1b_9000.log` (switch on,
`local\gl1b`). Both come from this tree, main 4ab52ffe0 plus this packet, and differ only by the
switch.

**The control does not reproduce K0.** Main moved between the two: the plane death modes and the
dead-aircraft release gate (packet cc9_plane_death_modes) landed after K0. L0 is the reference
here. It releases 10 bombs and 8 torpedoes, against K0's 25 and 16, and the Lexington survives.
So the K0 column in the section 3 table is stale for every row except the fighter rows.

**A first treatment run was discarded.** In `local\L1_9000.log` the lead ran twice per tick with
the finder bound. It ran once for the task's target, a value the finder then overwrites, and once
for the finder's result. The second call saw a velocity history the first call had just updated,
so the arc arm lost its acceleration whenever both had the same target. The far task targets also
inflated the shift census, to a mean of 550 m. The fix takes the lead once, for +74h, as 009FC931
does. The switch-off path is unchanged, so L0 stands as the control.

| row | L0 (off) | L1b (on) | prediction | verdict |
| --- | --- | --- | --- | --- |
| fighter plane-gun hits | 16 (Lexington-class01_sqn01, unled) | 66 | 1-40 | rose more than predicted |
| fighter shots | 1499 | 5789 | - | - |
| lead shift, mean per fighter | 0 | 21-53 m (max 230 m) | mean 40-120 m | lower; hits are at 200-750 m |
| arc-arm ticks | 0 | 381 of 4101 lead ticks | - | - |
| fine-aim ticks | 0 | 1470 | - | - |
| bursts / trigger ticks | 8 / 251 | 32 / 971 | 5-25 / 100-600 | **missed**: about 4x |
| Val deaths to fighters | 0 | 2 (Val #3.1 at 100.30 s, #3.1\|.-3 at 102.80 s) | 0-2 | held |
| Val deaths, all causes | 14 | 13 | - | - |
| torpedo drops / Kate deaths | 8 / 14 | 8 / 14 | unchanged | held; Kate death times moved by up to 39 s (path-coupled AA) |
| Lexington-class01 | alive | alive | unchanged | held |
| bomb releases (aircraft) | 5 (#3.1 x4, #7.1\|.-3) | 2 (#3.1\|.-4, #7.1\|.-2) | flat ±2 | **missed** |
| water contacts | 18 | 10 | - | - |
| US fighter deaths | 3 (Lexington flight, 395-409 s) | 1 (Yorktown-class01_sqn02\|.-3, 114.15 s) | - | - |

**Hits.** The Yorktown flight made 50 of the 66 hits:
- sqn02: 22 hits
- sqn02|.-2: 12 hits
- sqn02|.-3: 16 hits

In L0 the same flight fired 562 rounds for 0 hits. Its second bursts close from 194 to 299 m on
Val #3.1 and #3.1|.-3. Those two Vals die at 100.30 s and 102.80 s, before any ship's first hit.
The log has no per-shooter kill credit, so the attribution rests on those times and targets.

**Why the triggers rose.** The fine aim steers the fighter onto the lead in the aim state, so the
envelope is met more often. The Lexington flight carries most of the rise: 423 and 339 ticks
against 157 and 0. Its lead fighter fired 2527 rounds for 0 hits, and its second fighter made 16.

**Why the dive rows moved.** The fighters shot down the lead of the one Val squadron that released
in the control, before its dive. Of its three survivors, #3.1|.-4 still released. #3.1|.-2
lived until 317 s without releasing. Whether a leaderless wingman should still attack is a
follow/formation question outside this packet. #7.1|.-3's release moved to #7.1|.-2 through path
coupling. The fighters are doing what the image's fighters do, so the lost releases are a
consequence of the faithful lead, not a new host defect. **One open question remains: #3.1|.-2
never releasing.**

**Switch state landed: `kFighterGunLeadBound` ON.** The lead and the fine aim are image law with
three labelled substitutions (section 2). The pair shows that the unled host fire was why the
Yorktown flight's fire missed.

## 5. Secondary: why the range-factor pair moved the ship AI

The pair is `local\K0_9000.log` (kPlannerRangeInterpBound off) and `local\K1_9000.log` (on), from
`docs/PLANNER_KATE_TARGETING.md`. Their ship-AI totals moved: goal replans 258 to 981, brain
targets 8 to 9, command-target units 50 to 56. The logs were compared line by line after removing
pointers, process ids and harness-slot text. No new runs were made.

1. **The first divergence in the whole log is the fighter group's planner order.** The group led
   by Lexington-class01_sqn01 (six fighters) is ordered onto the D3A Val #3.1 group in K0 and onto
   the D3A Val #1.1 group in K1. That is the range factor's new pick. Nothing else differs before it.
2. **The Vals' paths diverge next.** The two Val squadrons' formation geometry first differs at
   tick 1600 (80 s). The fighters reach and fire at a different squadron in each run. How the
   fighters perturb the Vals was not traced.
3. **The first ship-AI divergence is at ship-AI step 2440 (122 s).** The escort group (Northampton
   01/02, Fletcher 01-04, York 01/02) enters attackmove in both runs, with the same order: the
   group led by Lexington-class01 attacks the one-member movieval group in both. Only the target
   point moves, by 1.4 m (Northampton-class01: heading target 6.0583 against 6.0565, distance
   3597.58 against 3598.98). The steps at 2430 are identical. So the ships' order is the same; their
   approach point moved because the aircraft moved.
4. **The ship group's own orders then split.** At 190.85 s K0 orders the ship group onto the
   12-member Val group. K1 never issues that order. Two causes fit, and the logs cannot separate
   them without a planner score trace:
   * the range factor re-weighting the ship group's own pick, since the group planner scores every
     group with it;
   * the changed Val positions from step 2.
5. After that the escort group's attackmove samples go from 58 to 61 per ship, and their follow
   samples shift. The host does not log the replans per ship.

**Verdict.** The ship-AI movement follows from the range factor's picks. Its first link is the
fighter group's new target, and no ship-side divergence comes before the aircraft diverge. No
independent ship-side bug is implicated. That supports flipping kPlannerRangeInterpBound ON. The
one open point is which of the two causes in step 4 removes K0's second ship order. Settling it
needs a diagnostic, default-off trace of 00A1CB80's per-candidate score (base, range, objective,
sticky) for the ship group at about 190 s. It is not needed for the flip.

## 5. The friendly-in-line hold, 007B96D0 -> 007DEDB0 (packet cc9_fighter_accel_friendly_fire)

**The image has a protection the host lacked.** Step 6 of 009FC7C0 (docs/DOGFIGHT_GUN.md) fires only
when `!007B96D0(unit)`. When 007B96D0 is true, it re-aims at +0.25 through 009F9FC0 instead.

- **007B96D0** (body 007B96D0-007B96EB) is `unit+C50h != 0 && 007DEDB0(unit+C50h)`.
- **007DEDB0**, `__fastcall(neighbours)`, read whole from the listing:
  - While `+80h <= +90h` it returns the cached byte `+C8h`.
  - Otherwise it clears `+C8h`, subtracts `+90h` from `+80h`, and walks the `+70h` list: the
    same-party aircraft within the friendly radius that 007E11D0 keeps (docs/PLANE_GUNFIRE.md 3).
  - For each live entry (IsKindOf 0Fh, `+5Eh` clear), it transforms the friendly's position
    into the owner's frame. The frame is 00414E10 on `[[this+4]+28h]`, applied by 004142E0 at
    007DEE49.
  - It sets `+C8h` when `z > 1.0` ([00D7A24C], 007DEE54) and `x*x + y*y < +94h`.
- **Constants.** `+94h = 400.0` ([00CFD710], stored only by the constructor at 007E1F13), so the
  test is a **20 m radius cylinder ahead of the nose, at any range**. `+90h = 1.0` (007E1EFF),
  so the answer is re-evaluated once per second. The `+80h` clock starts at `U(0, 1) + 1.0`
  (007E1FD2-007E1FF2) and advances in 007E2010.
- **Consequence.** A fighter with a friendly anywhere ahead of it inside 20 m of its nose line holds
  fire for up to a second. It does not test the target, the range or the gun's reach.

**The host.**
- `DogfightGunInputs::finder_busy` already carries the input. Nothing set it, so the host always
  fired.
- `GameUnitsHost::Impl::friendly_in_line_007b96d0` now keeps the `+70h` list (007E11D0's
  same-party arm, 3 s refresh on its own clock copy), the 1 s re-check and the cached answer, all
  behind `kFighterFriendlyInLineBound`.
- **Stand-ins:** the clock start is taken at its midpoint 1.5, and the object's tick cadence is
  taken as the gun tick's.

**The call site is in the fighter aim hunk (cc9-gunnery-host's), as a request.**
Immediately before `bsp::dogfight_gun_tick_009fc7c0(unit_.df_gun, gi);`:

```
if constexpr (GameUnitsHost::Impl::kFighterFriendlyInLineBound) {
    gi.finder_busy = owner_.friendly_in_line_007b96d0(unit_, dt);
    if (gi.finder_busy) ++unit_.ff_busy_ticks;
}
```

The re-aim at +0.25 that 009FCDB6-009FCE01 performs instead of firing stays the open substitution
already labelled at that site.

**Would the image have fired the SA burst?**
- SA's friendly-fire kill at 107.90 s was Lexington-class01_sqn01 .-3 hit by its own flight leader.
- A wingman ahead of the leader within 20 m of its nose line is exactly what 007DEDB0 tests.
- If .-3 sat in that cylinder at the leader's re-check, the image held the burst. Section 5.2
  measures whether it did.

**The damage path.** It was not read in this packet. The gunnery host's projectile sweep is
cc9-gunnery-host's. Whether the image's round-versus-aircraft hit filters by side is left open, so
the hold above is the only protection this packet establishes.

### 5.1 Predictions, written before the pair

The pair is E2 9000 with `kFighterFriendlyInLineBound` off (F0) against on (F1), with the
acceleration term in its landed OFF state, from the same tree and with
`BSP_GUNNERY_RNG_STREAMS=1`. The call-site line is applied locally for the pair.

- **Busy ticks.** Above 0 for US fighter flights that fly in formation: 5-200 per flight. The US
  fighters are the only aircraft whose gun controller runs in E2.
- **Fighter fire.** Fire ticks fall from 129 by 0-40%, and hits from 80 by a similar share.
- **Friendly-fire kills.** None in F0 (there were none in S0) and none in F1.
- **Deaths and hits.** Japanese deaths 28-42, and AA hit records within +-15%, both RNG-coupled.
- **Releases** 0/0, and no mission end.

### 5.2 The pairs, measured

The runs are E2 9000 with `BSP_GUNNERY_RNG_STREAMS=1` and the call-site line applied locally.
Every log's module directory was checked.

| row | FF0: hold off, accel off | FF1: hold on, accel off | FFA: hold on, accel on | SA: hold off, accel on (sweep) |
| --- | --- | --- | --- | --- |
| hold busy ticks (sqn01 leader / .-3 / sqn02 leader / sqn02 .-2) | - | 0 / 110 / 870 / 1130 | 30 / 120 / 970 / 1040 | - |
| fighter fire ticks / bursts | 129 / 8 | 129 / 8 | 358 / 14 | 222 / 10 |
| fighter hits | 80 | 80 | 133 | 101 |
| hit records / deaths | 549 / 35 | 549 / 35 | 569 / 36 | 553 / 37 |
| US fighter deaths | 0 | 0 | 1 (sqn01 .-3, credited to its leader, 116.05 s) | 2 (sqn01 .-3 to its leader at 107.90 s; the leader to movieval .-2) |
| Lexington moved | 6905.23 m | 6905.23 m | 6780.57 m | 6517.06 m |

**FF1 equals FF0 on every headline row.** The hold is active, but no busy tick coincides with a
firing envelope. The sqn02 flight holds most, and in these runs it never fires. So the prediction
"fire ticks fall 0-40%" held at 0, and none of the other rows moved.

**The hold does not prevent the SA friendly-fire kill.** With the acceleration term on (FFA), the
same wingman dies to the same leader. At the time, the leader was firing at movieval .-3 at
372.7 m. The image's test is a 20 m cylinder about the nose line, answered once per second, so a
wingman crossing the line inside a cached "clear" second is not protected. On this evidence the
image fires that burst too. Which round geometry puts the wingman in the stream was not traced.

**Switch states landed:**
- `kFighterFriendlyInLineBound` ON: the predicate and its list are bound here.
- The call-site line is the request above, so the hold is inert until cc9-gunnery-host applies it.
  When applied, it is row-neutral on E2 (FF1 = FF0).
