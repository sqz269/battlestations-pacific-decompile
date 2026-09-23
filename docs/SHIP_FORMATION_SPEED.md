# Ship formation speed: the group speeds behind 009F4DA0

Packet cc9_ship_formation_speed, 2026-09-23. Base: main a3099a6f8. Switch
`kShipFormationSpeedBound` in src/game_hosts_ship_ai.cpp, which is on. Status: reconstructed,
build-tested and run-compared on USN04 and USN01. Not ABI-compatible and not game-validated.
Names are hypotheses.

Addresses: 009F4DA0 BSP_ShipAi_ThrottleCeilingStep, 0070D140 BSP_UnitGroup_ReduceMemberSpeeds,
0070D0F0 BSP_UnitGroup_GetSpeedCeiling, 0070DA00 BSP_UnitGroup_RefreshSpeedCeiling, 0070D100
BSP_UnitGroup_PublishMemberSpeed, 0070E3C0 BSP_UnitGroup_GetMaxFollowerTurnRadius, 0080FC30
BSP_UnitInstance_GetReferenceSpeed, 0082E850 BSP_ShipClass_GetTurnRadius, 00778890, 007788B0,
009F1420 (009F144F), 009DF2D0 (009DF6AA).

## 1. The group speeds

The member records sit at group+18h, 34h bytes each, with the count at +4F8h. Record +0h is
the member and record +30h is its published speed.

| Routine | Law |
|---|---|
| 0070D140 | The minimum of record+30h over records with a member, seeded 9999999.0f (00CFD6F4). An empty record is reset to 999.0f (00CF4888). |
| 0070D100 | Stores a speed into the caller's own record+30h. Its only caller is 009DF6AA in the follow update 009DF2D0: 0080FC30(unit) × 1.25 (the double at 00CF87C0) divided by a clamped station term. |
| Initial record+30h | 999.0f, from the join 0070EF30 (0070EF76) and the group init 0070D7B0 (0070D7F6). |
| 0070D0F0 | Returns group+504h. |
| 0070DA00 | Called from each join. Sets +504h = the minimum over the members of the ship class's MaxSpeed (class+500h), or for a non-ship [member+3D0h]'s class+188h, seeded 9999999.0f. |
| 0070E3C0 | The maximum of 0082E850 over the ship members other than the leader [group+14h], seeded 100.0f (00CE3D08). |
| 0080FC30 | unit+9C0h × the 008E6430 category-4 gameplay modifier when the modifier manager is live, else × 1.0f. |
| 0082E850 | class+520h × [00424C40+438h] (Navigator.TurnMultipliers.TurnMultiplierMaxSpeed[2], 2.0), unless the class descriptor answers vtable[18h](0Eh). |

## 2. brain+0AF0h

A census of the disp32 stores of +0AF0h finds these writers:

| Site | Where | Value |
|---|---|---|
| 009F122F | the brain constructor | 1.0f |
| 009F144F | the pre-pass, every pass | 1.0f (00D7A24C), beside clearing +0B38h and +3ADh |
| 009E1E48, 009E1FEF, 009E200C | land | varies |
| 009E229D, 009E2394 | kamikaze | varies |
| 009E262F | engage | varies |
| 009E2A1F, 009E2A33 | lead pursuit | varies |
| 009F392F, 009F3952 | tangent | 1.0 or 0.5 |
| 009DAEE6, 009DB658 | two trivial setters | varies |

The only reader is 009F4DD1, in 009F4DA0. The host runs none of the writing states: the
attackmove selector never reaches the engage, lead-pursuit or tangent members, and no land or
kamikaze state runs. So the faithful host value is the pre-pass's 1.0 (LABELLED).

## 3. 009F4DA0, bound

The read is from docs/SHIP_NATIVES_3.md. The block is brain+8h, so blk+344h is brain+34Ch
and blk+348h is brain+350h.

1. brain+350h = 1.0. A set brain+0B38h skips everything else.
2. brain+34Ch = min(brain+34Ch, brain+0AF0h) (009F4DC7).
3. **Leader** (00778890: the unit is [group+14h]):
   - brain+34Ch = min(brain+34Ch, float(min(0070D140, 0070D0F0) / 0080FC30))
   - brain+350h = float(0082E850(own) / (0070E3C0 × 1.2)), where 1.2 is the double at
     00CEC160; floored at 0.75 (00CEE07C) and capped at 1.0
4. **Follower** (007788B0):
   - 00863780(1) on unit+6DCh, a weapon side effect recorded for the gunnery host
   - when brain+3ADh is set, the station arm; see below
   - brain+350h = clamp(min(0070E3C0 × 1.25 / own turn, own turn / 200.0), 1.0, 1.25)

**The station arm is not bound.** brain+3ADh is written only by the pre-pass clear and by
009DA3B0's station request. In the image that request always zeroes brain+3A4h, so the arm would
set the follower's limit to ±0. The host records the request and runs no station-keeping arm,
so brain+3ADh keeps the pre-pass clear, and the (ceiling + 6.70421028) / reference term is not
applied (LABELLED).

**The host.**
- The units host publishes member records through the new `formation_member_unit(group, slot)`.
- The ship-AI host stores each unit's record+30h (999.0 until the follow state publishes).
- The rules run at the point the drive reads the two fields.

**Consumers.**
- blk+344h is read by the throttle ceiling cap `max(1.0, limit)` at 009F43EC, which never lowers
  the throttle. It is also read by the direction-mismatch branch 009F443F..009F448A, which does.
- blk+348h clamps the rudder only on the latched branch at 009F4511.

## 4. Predictions and measurement

Predictions were written first to local/fs_predictions.txt:
- a leader's limit below 1 only when a member is slower
- throttle moves only through the mismatch branch
- rare or no rudder changes
- gunnery unchanged

Runs used `BSP_GUNNERY_RNG_STREAMS=1` on both sides, with build/win32/fsC as the control and
build/win32/fsT as the treatment.

**USN04, 4700/4500** (local/fs_{ctl,trt}_usn04.log).
- Lexington-class01 leads the merged group. Its limit is 0.9474 at every step: the slowest
  member's MaxSpeed over its own reference speed. Its rudder limit spans 0.8526..1.0.
- The followers run at rudder limit 1.25, except Yorktown-class01 at 1.0..1.25.
- With addresses and threads masked, the only differing line is that limit in Lexington's
  summary row (0.947 against 1.000). The throttle, ship-AI, impact and death lines are identical,
  and gunnery is unchanged. Lexington cruises, and its throttle never took the mismatch branch.

**USN01, 3200/3000** (local/fs_{ctl,trt}_usn01.log).
- Convoy1 (FleetOilerJ) leads a convoy with limit 0.5879. The direction-mismatch branch now
  holds its throttle at 0.588, where the control ramped from 0.59 to 0.86. Its logged position differs by
  about 45 m at 150 s.
- Everything downstream follows from that one term: the two scout Dauntlesses targeting it close
  about 30 m less, the path-search and ring counters shift, and the refills counter is ignored.
- Hits (118), damage (4467.9) and deaths (5) are unchanged.
- Enterprise's group has no slower member, so its limit stays 1.0.

**Unimplemented calls, USN04:**

| Build | Unimplemented calls |
|---|---|
| control | 3,944,607 |
| treatment | 3,772,551 |
| fall | 172,056 |

The fall covers throttle_ceiling (81,000), throttle_ceiling_344 and publish_member_speed (10,056).

Decision: landed. The one term that moves, a convoy leader slowing to its slowest member, is the
image's law, and nothing else moved.

## 5. Open

- The follower station arm: 009DA3B0's request store and the station-keeping arm of 009ED6B0.
- The class-descriptor answer to vtable[18h](0Eh), and whether the 2.0 multiplier applies to
  each class.
- The settings object behind +438h and the 008E6430 modifier, which are defaults here.
