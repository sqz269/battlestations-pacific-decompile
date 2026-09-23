# The planner's target choice, and the range factor the host had wrong

Addresses: 00A1CB80, 00A1CC65, 00A1CD07-00A1CD4D, 00A1CD53-00A1CD9A, 00A1CDAB-00A1CDDA,
00A1CEAE, 00A0F970, 00A0C650, 00A0C3C0, 00A0C330, 00A07E40, 00A04560, 009FDF30, 00A08460,
00D7A2F0, 0077D600.

Packet `cc9_planner_kate_targeting`. Every name is a hypothesis, not a recovered symbol. The log is
`local\fdON_9000.log`: the faithful set at E2 9000, with `BSP_GUNNERY_RNG_STREAMS=1`.

## 1. The image's attack pick, 00A1CB80 (`ai_planner_choose_attack_target`)

For the planner's first owned group, each enemy group with members gets a score, and the strict
maximum wins (00A1CEAE). The winner is ordered through 00A1CEEF, which leads to 0077D600's member
orders. The score multiplies four terms:
* **base.** 00A1CC65 calls **00A0F970(own group, candidate, ...)**, the group target value (section
  3). The host substitutes the candidate's **population**.
* **range.** 00A1CD07-00A1CD4D form the planar range between the two groups' first members' poses
  (+FCh/+104h), as sqrt, or 0 under 1e-10 (00CE3820). Then **00A1CD53-00A1CD95 compute
  00419010(x0 = tuning+1D0h FreeAttack_NearDist, y0 = 1.0 (FLD1), x1 = tuning+1D4h
  FreeAttack_FarDist, y1 = 0.1 [00D7A2F0], x = range)**. That is 1.0 inside NearDist, falling
  linearly to 0.1 at FarDist.
* **objective.** tuning+1CCh FreeAttack_ObjectiveTargetMul, when 00A2C450 finds an objective member
  (00A1CDAB-00A1CDC1).
* **sticky.** tuning+1D8h FreeAttack_ExistingTargetMul, for the current target (00A1CDC7-00A1CDDA).

The authored values are NearDist 5000, FarDist 12000, ExistingTargetMul 2 and ObjectiveTargetMul 2
(`highlvlaiglobals.lua` line 114 and on; one mode uses 9000/15000).

**The host's divergence.** `range_interpolation(near, far, d)` returned
`near + (far - near) × clamp(d / 9 000 000, 0, 1)`. It treated the two distances as the two output
values and divided by the squared engagement radius. So every candidate scored about 5000 × (1 +
0.0003 per km). The range factor was almost flat and rose slightly with distance, which left
population to decide. **The image weighs a candidate at 12 km or beyond at 0.1 of one within 5 km.**

The host's own distance helper returns 0 inside 3 km (d² ≤ 9e6). That changes nothing under the
image's map, because anything within NearDist scores 1.0 anyway.

## 2. The decision in the log

* **The ship group.** The USN side's first planner group (18 members, leader Lexington-class01) is
  ordered onto movieval at 18.3 km. The line is `ai diag order_attack group_members=18 ...
  target_leader=movieval`, and `movetoattack dist=18309.5`.
* **The fighters.** The six fighters of both flights form one group led by
  Lexington-class01_sqn01, ordered onto the six-member D3A Val #3.1 group at 26.7 km
  (`movetoattack dist=26731.9`).
* Only two attack orders are issued in the whole run. The planner re-picks every tick, but the
  dedupe suppresses repeats of the same order.
* **Host scores.** Every candidate scores about 5000 × population. The six-member Val groups
  therefore beat the four-member Kate groups at any range. With the sticky 2, the #3.1 group holds
  the fighters for the whole mission.
* **Image scores, as first reasoned (before the pair).** If the fighter group's first member were
  near its carrier when the Kates close, a Kate group would score base_K × 1.0 against the #3.1
  Vals' base_V × 0.1 × 2. With the class weights equal (TorpedoBomber 8 = DiveBomber 8,
  `highlvlaiglobals.lua` lines 22-23), that is 4 against 1.2, and the Kates would win.
* **That premise is false** (section 6). The first member is the fighter leader, which by then is
  chasing Vals far from the carrier. With the image's range factor, the group takes the nearer Val
  groups (#1.1, #5.1, #3.1) and never a Kate group.

## 3. The group target value 00A0F970 / 00A0C650 (read; still substituted by population)

00A0F970(`__fastcall` ECX = attacker group, EDX = target group, five stack arguments, `RET 14h`):
* It returns 0 if either group's +5644h member count is 0.
* Otherwise it builds each group's entity records through 00A07E40, which calls
  BSP_Ai_EntityRecordBuild 00A04560 per member, and returns **00A0C650**.

00A0C650's own debug text states the formula:
`RESULT = ((AttackSumMul × AttackSum + AttackMaxesSum) / ReferenceWeight + SpeedBonus) -
(DontAttackPenalty + DontAttackedPenalty + RepeatPenalty)`.
* **AttackSum** is the sum over every attacker-target pair of 00A0C3C0 (per pair).
* **AttackMaxesSum** is the sum of each attacker's best pair.
* **The penalties** are tuning+21Ch × the attackers with no target, tuning+220h × the targets no
  attacker can hit, and tuning+224h × the repeated vehicles.
* **SpeedBonus** is min(tuning+22Ch × 00A07C10(), tuning+228h × base). This installation authors
  0, so it is off.
* The Lua values are ReferenceWeight 5, AttackSumMul 0.33, penalties 0.5 / 2.0 / 1.0
  (`highlvlaiglobals.lua` lines 139-145).

The per pair value is 00A0C3C0 = 00A0C330 × distmul, where:
* **00A0C330** = targetweight (00A08460 BSP_Ai_TargetWeight, which the host already models) ×
  rnd × fixweight.
  * rnd is the product of the two records' +18h: a per-entity spread from the **entity pointer
    modulo 79**, mapped into ValueRandomMul {0.95, 1.05}.
  * fixweight is the target record's +14h, from BSP_Ai_ClassWeightForClassId 009FDF30 × 00A04240.
  * It is zeroed for a kind-1Ch attacker record.
* **distmul**, for a plane attacker (kind 0Fh), is 00419010(tuning+F0h, +FCh, +F4h, +F8h, (d -
  tuning+ECh) / class+188h MaxSpd), a travel-time map. For a ship (kind 6) it is (d - tuning+D8h)
  / class+500h, with +DCh-+E8h.

**Not bound in this packet.** A faithful base needs:
* 00A04240;
* the ordering of 00A00020's seven arguments (`docs/AI_TARGET_WEIGHT_TERMS.md` marks it unsettled);
* the tuning block's +D8h-+FCh;
* a labelled stand-in for the pointer-modulo spread (the host has no stable addresses).

The population stand-in remains, and this doc names it as the next term.

## 4. Predictions for the pair (written before the runs)

The pair is E2 9000 with `BSP_GUNNERY_RNG_STREAMS=1`, built in this tree from 99f51dc34 (the
faithful set on), differing only by `kPlannerRangeInterpBound`: **K0** off, **K1** on.

1. **The fighter group** is still first ordered onto the nearest enemy group at the time the order
   is issued. Once the Kates close on the Lexington, it switches to a Kate group. The log should
   show more than one `order_attack` for the six-fighter group, the later one naming a B5N Kate
   leader.
2. **The ship group** (18 members) also weighs by range. Its first order may change from movieval
   at 18.3 km to a nearer group, and the `movetoattack` distances change. Ship paths move, and
   ship-AI rows move.
3. **Fighter trigger ticks** rise above K0's, with bursts at Kates.
4. **Kate deaths** rise by a few, torpedo drops on the Lexington fall, and the Lexington lives
   longer than in K0, or survives the 9000 frames.
5. **The Val dive rows** are flat unless the fighters' departure changes the Vals' near-field
   avoidance (`docs/AIMDIVE_ENTRY.md` 5). Releases stay 25 ± 3.

## 5. Secondary: the arc arm's helpers for 00954650 (read-only)

The arc arm of the plane prediction 00954650 (`docs/FIGHTER_GUNFIRE_RATE.md` 3) uses three helpers.

**0042B2F0 BSP_Vector3_LengthFloatThreshold.**
* `__fastcall(ECX = v) -> ST0`, plain `RET`.
* It returns sqrt(x² + y² + z²), stored and reloaded as a dword, or 0 when the squared length is at
  most 1e-10 (00CE3820).

**004F9B30 BSP_Vector3f_Cross.**
* `__fastcall(ECX = out, EDX = a, [ESP+4] = b)`; returns out in EAX.
* It computes **out = a × b**, component by component through dword stores.

**00952340**, a rotation about a point.
* `__fastcall(ECX = out, EDX = centre c, [ESP+4] = axis, [ESP+8] = point p, [ESP+0Ch] = angle)`,
  `RET 0Ch`.
* d = p - c goes on the stack (00952343-00952370). **0085C3F0**(ECX = &d, [ESP] = axis,
  [ESP+4] = angle) rotates d in place (0095237C). Then out = c + d (00952381-0095239C).

**0085C3F0**, the Rodrigues rotation.
* `__fastcall(ECX = out, EDX = v, [ESP+4] = axis a, [ESP+8] = θ)`, `RET 8`.
* par = (v·a) × a, and perp = v - par.
* If \|perp\|² < [00D7A268] (a double of about 1.0e-4), out = v.
* Otherwise **out = par + cos θ × perp + sin θ × (a × perp)**. The cross product is 004F9B30 with
  EDX = the axis, loaded at 0085C3FD from the first stack argument, and b = perp.
* This is the standard right-handed rotation, assuming the axis is a unit vector.

Unread in 00954650 itself: how the arc arm builds its axis and centre from unit+654h-65Ch and v
(009546F6-0095483B), and the angle's sign. 0095489A applies FCHS and then multiplies by t, so the
angle is -(rate) × t.

## 6. Results

The runs are E2 9000 with `BSP_GUNNERY_RNG_STREAMS=1`, built in this tree, differing only by the
switch.

| run | log | range factor | drops | kill credits | water | torpedo drops | fighter bursts / trigger ticks | mission end |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| K0 | `local\K0_9000.log` | host (flat) | 25 | 34 | 7 | 16 | 10 / 263 | failed 261.06 s |
| K1 | `local\K1_9000.log` | image | 24 | 30 | 5 | 16 | 4 / 107 | failed 261.06 s |

K0 equals the faithful-set ON run (`docs/FAITHFUL_DIVE_SET.md` 3) on every headline.

**The orders.**
* In K0 the fighters are ordered onto D3A Val #3.1 (6 members), and the ship group later onto the
  merged #3.1 group (12).
* In K1 the fighter group is ordered onto **D3A Val #1.1**, then, grown to 12 members, onto **#5.1**,
  then onto **#3.1** (18 members). The ship group keeps movieval.
* The fighters now take the nearer groups, as the image's range factor says. **They never take a
  Kate group.**

**Against the predictions:**
1. **Failed.** More than one order is issued, but none names a Kate leader.
2. **Held for the ships' first order** (movieval in both). The ship AI still moves: goal replans go
   from 258 to 981, brain targets from 8 to 9, and command-target units from 50 to 56. The
   planner's orders reach the ships' command targets. Not traced row by row.
3. **Failed.** Trigger ticks fall to 107 (4 bursts). The Lexington flight's leader group chases the
   nearer Val groups, which it does not catch.
4. **Failed.** Torpedo drops are 16 in both, and the Lexington is lost at 261.06 s in both.
5. **Held.** Releases are 24 against 25.

**Why no Kate group wins.** The group's position is its first member's. The first member is the
fighter leader, which is already away chasing Vals when the Kates close on the Lexington. The
Kates are near the carrier, not near the fighter group. The image's own position term would do the
same with this membership. What decides the outcome is **which group the fighters belong to, and
where its first member is**: the merge into one six- and then twelve-member group led by
Lexington-class01_sqn01.

## 7. Decision

* **The range factor is a host divergence with a read behind it** (00A1CD53-00A1CD95). It is bound
  as `kPlannerRangeInterpBound` in `src/game_hosts_ai.cpp` and lands **OFF**. Its pair moves the
  ship AI through the planner's command targets in ways not explained row by row, and it does not
  bring the fighters onto the Kates.
* **The image's planner would not send the Lexington's fighters after the Kates either**, with the
  groups composed as the host composes them.
* **The next term is the group composition:** the ComposeGroup pass that merges both fighter flights
  (and later six more aircraft) into one group, whose value is the same 00A0C650 formula (section
  3). The group base weight 00A0F970 is also still substituted by population.

**Flipped ON (packet cc9_val_squadron_registry, docs/VAL_SQUADRON_REGISTRY.md section 6):** the ship-side divergence traced back to the fighter group's new pick. Its pair on the leave-at-death state took torpedo drops from 8 to 0, because Yorktown stays on its path and its AA downs the Kates before release; that order split is untraced. The flip is a separate commit so it can be held.
