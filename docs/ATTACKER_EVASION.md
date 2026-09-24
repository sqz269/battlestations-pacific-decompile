# Do the image's attackers react to fire? (packet cc9_attacker_evasion)

Addresses: 009A17D0, 0099EC40, 007C74A0, 007C75A0, 007CC2F0 (the call sites), 007D5D20 (the
property binding), 0099B670, 0099F1C0 and 0099B790 (identified, not read). Every name is a
hypothesis, not a recovered symbol. Ghidra was not written.

## 1. Answer

**The image's attackers fly straight into ship AA too.**
- The plane AI's only reaction to fire is a GUNFIRE avoidance. It dodges the gun line of an
  enemy AIRCRAFT that is firing its fixed guns. Nothing in the listing, and no key in this
  installation's `planeglobals.lua` or `robots.lua`, reacts to flak, to AA tracers or to taking
  damage.
- In the current E2 control (`local\H1_9000.log`, main plus the range factor) ship AA kills 33
  of the 35 Japanese aircraft that die, and US fighters kill 2.
- So the missing gunfire avoidance cannot explain the zero releases. It is not bound here (section
  4).

## 2. The reactions the listing has

| # | reaction | trigger | effect | constants and sources | host |
| --- | --- | --- | --- | --- | --- |
| 1 | gunfire avoidance, 009A17D0 -> 0099EC40 | own plane inside the forward cone of an enemy plane on the firing list (1.0 < z < [00CE3948] in the shooter's frame, lateral within [00D1F3F0] after a ±15 m dead band); own squadron skipped (+9D4h) | plan+26Ch bit 4; an avoidance control summed from the threats' side and blended with the normal control through 0099B790 | `Pilot/Avoidance/Gunfire`: MaxWeight 0.6, WeightInc 1.3/s, WeightDec 1.0/s, AvoidTime U(2, 3) s, WaitTime U(5, 8) s (tuning+5F8h..610h, timers pilot+3E8h/+3ECh) | **lacks**: 009A17D0 is one of the pilot's sub-object updates this host lists as a contract |
| 1a | exemptions of #1 | kinds 10h (level bomber) and 16h (large recon); a squadron flight leader with 2+ members (007B8AD0 and +3CCh) | level bombers instead scale their wobble by BomberVSGunfire 1.2 (tuning+614h) | 0099EC40, the tests after the timer draws (vtable[5Ch](10h), (16h), 007B8AD0) | lacks |
| 1b | who runs #1 | 009A17D0: an AI plane (+520h clear), alive (+5Dh clear), whose party differs from `[[00E188A8]+5FCh]+908h`; the arm is enabled by pilot+2E4h bit 4 and the squadron's +3A4h bit 4 | - | 009A17D0-009A19A2 | lacks |
| 2 | vehicle avoidance, 0099B670 (bit 2) | another vehicle on a collision course | avoidance blend | `Pilot/Avoidance/Vehicle` | lacks (not a fire reaction) |
| 3 | terrain avoidance, 0099F1C0 (bit 1) | ground ahead | avoidance blend | `Pilot/Avoidance/Terrain` | lacks (not a fire reaction) |
| 4 | attack-run weave, 009C4220 | a timer, not fire | a re-rolled lateral offset | robots rows | runs (`attackrun_weaves`) |
| 5 | goaway evasive turn | after a release, not fire | a banked turn away | `docs/DIVE_BOMB_GOAWAY.md` | runs |

**The firing list.** 007C74A0 sets plane+C40h = 1 and appends the plane to [00F87278] (count
[00F8727C]). 007C75A0 clears +C40h and swap-removes it. Both are called from 007CC2F0
`BSP_Plane_FreeFlightArm`, on plane+C35h (007CC345-007CC35A). 007D5D20 binds plane+C35h as the
Lua property `lastGunState` (the `{3, &+C35h}` pair followed by `{0, "lastGunState"}`, strings at
00D05C40 and 00D05C30). So the list holds the aircraft whose guns are firing now. Ships are not
on it.

## 3. How the attackers fly under fire now (H1)

Killers of the dead Japanese aircraft (the `killed_by` column):

| killer | kills |
| --- | --- |
| ship AA | 33 (Yorktown 6, Northampton-class03 6, Lexington 5, Fletcher-class03 4, Northampton-class01 3, Fletcher-class05 3, York-class01 2, Fletcher-class02 2, Northampton-class02 1, Fletcher-class08 1) |
| US fighters | 2 |

Time from the first flak blast to death, taken from the `impact blast` lines stamped with the
mission frame:
- **Kates** die 1.0-5.6 s after their first hit, after 1-13 blasts of 30 damage each. They are
  flying their straight torpedo run.
- **Vals** mostly die 0.1-6 s after their first hit. Five of them (the #3.1 and #7.1 flight leads
  and wingmen) survive 28-60 s from first contact, taking 9-33 hits through their dives and
  goaways.

**The image's gunfire avoidance could change only the 2 fighter kills and the fighter-hit
counts.** It cannot move the 33 AA kills, because ships never enter the firing list. The
question of why 35 of 36 attackers die is therefore on the AA side: its fire permission and
lethality, which is cc9-gunnery-host's audit.

## 4. Why nothing was bound

The largest missing reaction is #1. A faithful binding needs two unread pieces:
- 0099B790, the accumulator 0099EC40 steers through, 0099B790-0099B935;
- the blend in the pilot planner that consumes pilot+C8h/+190h/+258h/+25Ch with MaxWeight,
  WeightInc and WeightDec.

Binding the trigger without them would invent the control. Its measurable effect in E2 is bounded
by the 2 fighter kills and the fighter-hit count, and it does not bear on the zero releases. So
it is left as a follow-up packet: read 0099B790 and the planner's consumer, then bind 009A17D0's
gunfire arm with a pair measured on fighter hits and fighter-caused deaths.

No switch was added and no pair was run.
