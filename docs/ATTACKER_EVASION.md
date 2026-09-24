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

## 5. The gunfire avoidance, read and bound (packet cc9_gunfire_avoidance)

### 5.1 The three bodies

**009A17D0 BSP_PilotBot_UpdateAvoidance**, `__thiscall(pilot, float dt)`, body 009A17D0-009A19A2.
- It clears plan+26Ch, sets +25Ch = 1.0, and zeroes the three band-set counts at +C8h, +190h and
  +258h.
- It returns unless the unit exists and is not suppressed (+520h), is not torn down (+5Dh), is in
  free flight (+72Ch vtable[38h], mode 7) or mode 6, and its party differs from
  `[[00E188A8]+5FCh]+908h`, the local player's.
- Not in free flight, it sets +3E8h = -1.0.
- Otherwise, while `+3ECh <= +3E8h`, it does `+3E8h -= dt`. With +2E4h bit 4 (and the squadron's
  +3A4h bit 4) it calls 0099EC40 when `+3E8h > 0` or `+3E8h < +3ECh`.
- Afterwards, when bit 4 of +26Ch is clear and +3E8h > 0, it sets `+3E8h = -0.0 - +3E8h`.

**0099EC40 BSP_PilotBot_AvoidGunfire**, body 0099EC40-0099F1B3.
- **The shooters.** For each plane on [00F87278], other than the own squadron (+9D4h):
  - L = the own position in the shooter's frame, required to satisfy 1.0 < L.z < 800.0 (double
    [00CE3948]).
  - V = the own world velocity (vtable[34h]) rotated into the shooter's frame.
  - (x, y) = L.xy + V.xy, each shrunk by a ±15 m dead band (doubles [00CF3F20] and [00CE3D58]).
  - The plane is flagged when (x/L.z)² + (y/L.z)² < 0.0144 (double [00D1F3F0], about 6.9°). That
    sets plan+26Ch |= 4.
  - It then sums the own-frame x and y of (the shooter's line point at range L.z, minus the own
    velocity) into [ESP+0Ch]/[ESP+10h] (0099EE11-0099EF02).
- **The timers.** When flagged and +3E8h < 0: +3E8h = U(AvoidTime 2, 3) and
  +3ECh = -U(WaitTime 5, 8) (tuning+604h..610h), through 00BD2F10. A kind-13h plane scales both by
  its +DF4h factor; not modelled here.
- **The exemption.** Kinds 10h and 16h, and a squadron flight leader with 2+ members, go to
  0099F197, which sets unit+840h = BomberVSGunfire 1.2 and inserts no band.
- **The bands.** Otherwise the sum is normalised (004F2F40) and scaled by 0.8 (double [00CE3D40])
  to (x', y'). Then, through 0099B790:

| set (`this`) | x' > 0 / y' > 0 | otherwise | constants |
| --- | --- | --- | --- |
| task+4h, repairs cmd[0] yaw | [-x', 0.8] | [-0.8, -x'] | [00CE74F8], [00D05E14] |
| task+194h, repairs cmd[2] roll | [-0.8x', 0.71] | [-0.71, -0.8x'] | [00CED058], [00D1F3EC] |
| task+CCh, repairs cmd[1] pitch | [-y', 0.7] | [-0.7, -y'] | [00CE3E18], [00D1F3E8] |

**0099B790** (body 0099B790-0099B935) is an interval-set insert: up to 24 [lo, hi] pairs, the
count at +C4h. It extends lo <= -1.0 to -5.0 and hi >= 1.0 to 5.0 ([00D7A260]/[00CFBC84],
[00D7A24C]/[00CE3850]) and merges every overlapping band into one.

**0099BF30 BSP_PilotBot_RepairCommandBands** (`this` = task+4h) runs after 0099BC00 over the
six-dword buffer in 0099BC00's order: yaw, pitch, roll, throttle, air brake. It uses 0099B940:
- 0099B940 keeps a value outside every band.
- A value inside a band that covers all of [-1, 1] makes it return 0.
- Otherwise it moves the value to the nearer edge.

Per axis:
- cmd[1] pitch, gated by +72Ch vtable[38h], against bot+C8h: a blocked value becomes 1.1 when
  |bank| <= pi/2, else -1.1.
- cmd[0] yaw, ungated, against bot+0: -1.1 or +1.1 by the sign of the bank.
- cmd[2] roll, gated, against bot+190h: ±1.0 by its own sign.
- Each is clamped to [-1, 1].
- The throttle arm: when bot+258h < 1.0, it limits throttle and air brake. Only another arm could
  lower it; this one never does.

**MaxWeight, WeightInc and WeightDec (tuning+5F8h..600h) have no reader** apart from the Lua load
(`docs/GAME_TUNING_SINGLETON.md`). The image never blends. The avoidance is these hard bands on the
stick.

### 5.2 The host binding

`kPilotGunfireAvoidanceBound` in `src/game_hosts_units.cpp`. 009A17D0 runs before
`plan_yaw_0099d300`, and 0099BF30 after 0099BC00, where the old comment said "0099BF30 would run
here". Labelled substitutions:
- The gunFire byte +BC9h stands in for lastGunState +C35h on the firing list.
- The controlled unit's party stands in for the local player's.
- The +2E4h and +3A4h enable bits are taken as set; the host has no producer for either.
- One band per set. Only this arm inserts, once per think, into sets cleared that same think.
- The kind-13h timer scaling and 007D7A40 are not modelled.

The vehicle arm (0099B670) needs the plane-proximity list and its own band rule, and is not read.
The terrain arm (0099F1C0) needs the ground height and its own band rule, and is not read.

### 5.3 Predictions, written before the pair

The pair is J0 (switch off) against J1 (on), E2 9000, stream option on, on main d72c163ce plus this
packet.
- **Who dodges.** Only the Japanese aircraft, because the US planes are on the player's party. They
  dodge only while inside a firing US fighter's 6.9 degree cone within 800 m. Flight leaders with
  wingmen are exempt, so it is the Val, Kate and Zero wingmen, and the Zeros only if a US fighter
  fires at them.
- **Flagged ticks:** 50-400 over the run; band repairs somewhat fewer.
- **Fighter hits:** H1 47, predicted 25-47 (a drop).
- **Fighter-caused deaths:** H1 2, predicted 0-2.
- **AA deaths, releases (0 torpedoes, 0 bombs) and the Lexington:** flat apart from path coupling.

### 5.4 The pair, measured (J0 against J1)

`local\J0_9000.log` has the switch off and `local\J1_9000.log` has it on. Both runs finish the loop
with `exit_code=1` and no mission end, which is how current main ends in this configuration.

| row | J0 (off) | J1 (on) | prediction | verdict |
| --- | --- | --- | --- | --- |
| planes checked per think / flagged ticks | 0 / 0 | 105372 / 53 | flagged 50-400 | held |
| exempt ticks (flight leader with wingmen) | 0 | 18 (Val #1.1) | - | - |
| band repairs | 0 | 103 (Val #1.1\|.-2 44, .-3 44, .-4 15) | fewer than flagged ticks | **missed**: up to three axes are repaired per flagged think |
| fighter hits | 47 | 64 | 25-47 | **missed**: rose |
| fighter-caused deaths | 0 | 0 | 0-2 | held |
| Japanese deaths / killers | 35, all ship AA (Northampton 10, Fletcher 10, Lexington 7, Yorktown 6, York 2) | 35, all ship AA (Northampton 18, Lexington 10, Yorktown 4, Fletcher 3) | flat apart from coupling | held; the killers redistribute through path coupling |
| torpedo drops / bomb drops | 0 / 0 | 0 / 0 | flat | held |
| damage | 7700.0 | 7700.0 | - | - |
| Lexington | alive | alive | alive | held |

**Reading.**
- Only the Val #1.1 flight ever sits in a firing US fighter's cone, because the fighters engage
  that flight. Its three wingmen dodge through the bands, and its leader is exempt.
- Fighter hits rise, 47 to 64. The bands push the wingmen's stick to a band edge for one think at a
  time, and the flight's new path puts it where the fighters' fine aim holds. The mechanism was not
  traced tick by tick.
- The AA picture is unmoved, as section 3 said it would be: every Japanese death is still ship AA.

**Switch state landed: `kPilotGunfireAvoidanceBound` ON.** It is the listing's form at every term
read, with the labelled substitutions of 5.2.
