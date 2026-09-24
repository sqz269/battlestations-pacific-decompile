# Why no ordnance reaches the fleet in E2 (packet cc9_e2_zero_ordnance)

The reference row on main `416b17faa` (docs/GAME_EXECUTABLE.md, the section after the barrel
count) has no torpedo and no bomb dropped in USN04 over 9000 mission frames. The combined-state
row had 8 torpedo drops and 10 bomb drops. Deaths have stayed between 30 and 37 all day.

## 1. Runs and predictions (written before the runs)

All four runs are E2 (USN04, 9200/9000 frames) on main `710d3fc9b` merged into this branch
(`d2719e330`). All use `BSP_GUNNERY_RNG_STREAMS=1` and the read-only diagnostic
`BSP_DEATH_TABLE=1`, which prints one `death row` line per death and changes no term.

| run | switches | binary |
| --- | --- | --- |
| A | all as landed | `local\zA` |
| B | `kFlakProximityBurstBound` OFF | `local\zB` |
| C | `kMovetoSpeedBlendBound` OFF | `local\zC` |
| D | both OFF | `local\zD` |

Predictions:
- **A**: 0 torpedo drops and 0 bomb drops, 35-38 deaths, as in the death-flags ON run
  (`local/fl1_9000.log`, 37 deaths, 0 drops).
- **B**: 0-2 torpedo drops and 0-2 bomb drops. The flak burst's own pair left E2 torpedo drops at
  8 to 8, so turning it off should not bring releases back by itself. Deaths 33-37.
- **C**: releases come back: 3-8 torpedo drops and 0-10 bomb drops. The moveto blend is one of
  the three landings between the last run with drops (4, barrel count ON, main `02a946689`) and
  the first with none (death-flags OFF, main `ef9c27415`), and it is the only one of the three
  that changes the attack wings' speed. Deaths 32-37.
- **D**: as C or more: 4-10 torpedo drops.
- **Kate and Val deaths against aim entry.** Aim entry here means the attacker's last range to
  its ordered ship, from the `ordered` line. With the blend on (A, B), Kates die before they close
  inside their release range of about 1000-1200 m. With it off (C, D), some Kates live to release,
  and those die later against aim entry.

## 2. The four runs

| run | torpedo drops | bomb drops | deaths | refused releases from dead aircraft | prediction held? |
| --- | --- | --- | --- | --- | --- |
| A, all as landed | 0 | 0 | 37 | 9 | yes |
| B, flak burst OFF | 0 | 0 | 37 | 10 | yes (0-2) |
| C, moveto blend OFF | 0 | 0 | 37 | 11 | **no** (predicted 3-8) |
| D, both OFF | 0 | 0 | 37 | 10 | **no** (predicted 4-10) |

**Neither switch brings the drops back.** Section 1's narrowing was wrong. It reasoned from two
runs on different mains (4 drops on `02a946689`, 0 on `ef9c27415`) and took the moveto blend as
the cause. Turning the blend off moves which ship each Kate squadron attacks and when, but every
Kate still dies before it releases. The pass-side message, the traffic pass and the rudder-gate
store were the other candidates. They move the ships, not the aircraft, and they were not turned
off here.

## 3. The death table

Built by `local/deathtable.py` from the `death row` lines. Each line gives the victim, time,
altitude, first damaging hit, killer and its category, the killer's range, the nearest enemy
ship's horizontal range, and damaging hits by category. Run A in full:

- **Kates, 16 dead, none released.** They die 448-1074 m from the nearest ship (median 747 m).
  - The four Yorktown Kates (#4.1) die at 697-750 m from Yorktown, at 28-31 m altitude. Each dies
    to 7-8 category-1 hits, 0.1-0.35 s after its first damage. The killers are Yorktown's 28 mm
    quad (device 41, 4 barrels) and twin Bofors (device 42, 2 barrels).
  - The release envelope is 650 m in the first 15 s of the run and 450 m after it, times an
    aspect scale of 0.5 to 1.0 (docs/KATE_RELEASE_CONDITION.md). The Yorktown Kates die 50-100 m
    short of the 650 m distance.
  - Killers: category 1 for 10, category 6 for 5, category 5 for 1. Median killer range 808 m.
- **Vals, 19 dead, none released.** They die at 206-3368 m horizontal from the nearest ship
  (median 598 m). Twelve die at 858-1456 m altitude, before or at the dive entry; the other seven at 216-584 m. Killers: category 1 for 10,
  category 6 for 7, category 5 for 1, and one fighter kill. Median killer range 1289 m.

Per run, by `first damage to death` and range:

| run | Kate median first-damage-to-death | Kates dead within 0.5 s | Kate nearest-ship median (min) | Kate killers by category |
| --- | --- | --- | --- | --- |
| A | 1.65 s | 7 of 16 | 747 m (448) | 1: 10, 6: 5, 5: 1 |
| B | 0.25 s | 13 of 16 | 619 m (381) | 1: 15, 6: 1 |
| C | 5.35 s | 3 of 16 | 717 m (381) | 6: 9, 1: 6, 5: 1 |
| D | 0.30 s | 12 of 16 | 571 m (368) | 1: 13, 6: 3 |

**What moved.**
- With the flak burst on, dual-purpose and flak mounts kill the Kates 100-180 m farther out.
- With it off, the Kates close 130-180 m more, and then light AA kills them in one salvo, 0.25-0.3 s
  from the first hit to death.
- The blend changes the attack geometry. With the blend off, the Kate #4.1 squadron attacks
  Northampton-class03 at 135-161 s instead of Yorktown at 123-129 s.
- None of these changes the outcome.
- **Against the six-kill table** (docs/AA_LETHALITY_AUDIT.md section 1, before the flak burst and
  the barrel count): those Kates died at 820-1000 m after 7-8 hits in 0.15-0.6 s. That is the same
  burst shape. The flak burst now adds kills farther out, and the light AA bursts now land closer
  in, where the doubled and quadrupled light mounts fire.

## 4. The AA terms: what decides, and what is still substituted

**The deciding term is the light-AA salvo against an aircraft on its final run.** At the
SPVeteran skill every US ship is given, the flak and gunner bots have zero angle, distance and
throw error (docs/AA_LETHALITY_AUDIT.md section 2). Every round that fires at a straight-flying
torpedo bomber hits. 7-8 category-1 hits of about 30-39 applied damage each (MG classes 40-45 against armour 6) kill a 220-health Kate. A ship
with the 28 mm quad and two twin Bofors fires that many rounds in one barrel cycle, so the Kate
dies within a third of a second of the first hit.

**The host's fire-permission conjuncts for an AA gun, with their sources:**

| conjunct | host | source | faithful? |
| --- | --- | --- | --- |
| range | round's engagement range (`+60h`), the dual-purpose second ammunition against planes | 00729BC0, 006E9890, 00729BA9 | yes |
| minimum air range | FLAK and LIGHTARTILLERYFLAK against the round's MinRange | 005459E0, 00729B90 | yes |
| armour | light AA refused when max damage <= target armour | 008FBE17 | yes |
| fire window | the platform `Windows` arcs | 0085A9A0 / 007F60A0 | yes, but in the hull frame (no per-gun node) |
| **line of fire** | **always clear** | **0072F6E0 / 00729560 / 0072CDD0** | **no: substituted** |
| turning gate | `gun_can_fire_turning_0085a830` | 0085A830 | yes |
| reload and barrels | per-barrel timers, `BarrelDelayTime`, the model's barrel count | 007298D0, 0072CF00, 0072AB80 | yes |
| aim error | zero at SPVeteran; negative-vertical halving bound | 008FDBE0, 00902F62 | yes |
| ammunition | not counted | `Ammo = 9999` on 532 of 534 device rows | yes in effect: no mount comes near 9999 in 450 s |
| acquisition | the director's think time and the assignment passes | 0087E16B | yes |

**The line-of-fire predicate, read in this packet:**
- **Installed on every AA gun.** `00729560` (BSP_GunShotDecision_InstallLineOfFirePredicate) runs
  from `BSP_Gun_SetupFromDescriptor` (0072E702-0072E708). For weapon kinds 1, 5 and 6 it installs an
  object with vtable 00CFDC04 at gun+42Ch (007295D3). So the "gun+42Ch is zero" branch the host
  assumed is never taken by an AA gun.
- **Decision per target.** `0072F6E0` asks `00729670` once per target and caches the answer in a
  record `{target, clear, U(0.8, 1.2) x [GlobalConfig+8Ch]}`. The cached hit is returned without
  testing that third field, and whatever consumes it is unread.
- **The test.** `0072CDD0` (BSP_LineOfFirePredicate_Blocked) runs from the gun's own world
  position (gun+FCh) to the target, with both ends raised 5 m and the target's end clamped to at
  least 5 m. It answers "blocked" when:
  - a spatial-index segment query (flags 44h) hits static geometry; or
  - `0098B130` finds that the segment passes through the **firing ship itself** (the hit's `+54h`
    equals the owner's).
- **Direction of the substitution.** It can only refuse shots. Against a torpedo bomber at 20-30 m
  altitude and 700 m (about 2 degrees of elevation), a mount on the far side of the superstructure
  or deck from the target is looking through its own ship, and the image refuses it. The host
  fires it. So the host's AA is more lethal than the image's by this term. The size depends on
  where each mount sits.
- **It cannot be bound yet.** The test needs gun+FCh, each mount's world position, and the ship's
  collision shape. The host has neither: every gun fires from the unit origin raised by the class
  `Height`, which lies inside the hull, so any hull test from it would block every shot. This is
  the same hard dependency as the muzzle offsets (docs/GUN_BARREL_COUNT.md section 7.2 item 3):
  the ship-side platform attachment, which is unread.

**Verdict.**
- One AA term is still substituted in the more-lethal direction: the line-of-fire predicate. I
  have not bound it; binding it needs mount positions first.
- Every other conjunct on the list is the image's.
- The measured outcome, no ordnance delivered against an idle player's veteran fleet, can be the
  image's outcome or an artefact of that one term. That can be decided only by binding the
  predicate on real mount positions, or by a game-validated reference.
- No switch changed in this packet. The `BSP_DEATH_TABLE` diagnostic stays as a labelled,
  default-off measurement aid and changes no term.

## 5. Second set, on the lead's narrowed landings (predictions written before the runs)

The lead adjusted the packet: drop B, keep A and C, and add E (`kShipPassSideMessageBound` OFF,
the pass-side message and traffic pass, `446cbdd80`) and F (`kShipRudderGateStoreBound` OFF,
`3cc43e303`). Main moved again after sections 2-4. So all four are rebuilt on this branch at
`b61c82767` (main `44a2ac359`), E2 9000, with `BSP_GUNNERY_RNG_STREAMS=1` and
`BSP_DEATH_TABLE=1`. The binaries are `local\yA`, `local\yC`, `local\yE` and `local\yF`.

Predictions:
- **A and C:** as sections 2-3: 0 torpedo and 0 bomb drops, 37 deaths. Kates die at a median of
  700-750 m from the nearest ship; C moves the Kate #4.1 squadron onto Northampton-class03.
- **E (traffic pass off):** 0-2 torpedo drops, 0 bomb drops, 35-38 deaths. The traffic pass
  steers ships away from neighbours, which changes spacing and headings, but the light-AA salvo
  that decides every Kate kill in sections 3-4 does not depend on it. Kate death ranges stay
  within 100 m of A's.
- **F (rudder-gate store off):** as E: 0-2 torpedo drops, 0 bomb drops, 35-38 deaths.
- **Time from aim entry.** Kate aim entry is 2192-2199 m (docs/KATE_RELEASE_CONDITION.md). The
  approach is at about 75 m/s, so a Kate dying at 700-750 m dies about 19-20 s after aim entry.
  Vals die before or at dive entry in all four runs.

**Status, 2026-09-23 18:15: the second set is blocked by the session.**
- Runs A, E and F all died at mission frame 1412-1450, at 18:14. The log shows
  `present failed hr=0x88760868` (device lost), then exit 0xC0000005.
- Run C died at renderer initialisation.
- `query session` shows the user's session 1 as `Disc`.
- A 120-frame probe then failed at device creation (`device_hr=0x80004005`). Following the
  renderer-init rule, nothing more was launched.
- The partial logs (`local/y*_9000.log`) carry no drops, deaths or summary, and nothing is
  concluded from them.
- The first set (sections 2-4, on main `710d3fc9b`) is complete and stands. It has A and C: in
  both, no ordnance and 37 deaths.
- E and F still need their first runs. They need an active session.

## Correction, 2026-09-23 (packet `cc9_ship_platform_attachment`)

Section 4 misread the line-of-fire test. `0098B130` **excludes** both the firer's and the
target's collision objects, walks the spatial grid for kind-5 units, and returns the nearest
AABB hit. `0072CDD0` answers "blocked" when that unit's `+54h`, its side, equals the firer's.
So the predicate refuses a shot through a **friendly unit**, a ship of the same fleet or a
friendly aircraft, not through the firer's own hull. It still refuses only, so the direction of
the substitution stands. It no longer needs the hull's convex mesh: it is now bound on the
host's unit boxes, behind `kAaLineOfFireBound` (docs/SHIP_PLATFORM_ATTACHMENT.md section 3).
