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

## 6. The vehicle and terrain arms, read and bound (packet cc9_pilot_vehicle_terrain_avoidance)

009A17D0 runs three arms in a fixed order: the gunfire arm (section 5), then the vehicle arm, then
the terrain arm. Read whole from the listing, 009A17D0-009A19A2:

- **Entry.** Before any gate it stores +26Ch = 0, **+25Ch = 1.0** ([00D7A24C]), and the three set
  counts +C8h, +190h, +258h = 0 (009A17E9-009A1803). +25Ch is the throttle limit that 0099BF30's
  throttle arm reads. +258h is only the roll set's count.
- **Gates.** An AI plane (+520h clear), alive (+5Dh clear), in free flight or unit+900h == 6, and not
  of the local player's party (009A180F-009A1871).
- **Arms.**
  - Gunfire: free flight only. Mode 6 stores +3E8h = -1 (009A193F) and falls through.
  - Vehicle, 009A194F: pilot+2E4h bit 2, and the squadron's +3A4h bit 2 when a squadron exists, then
    0099B670.
  - Terrain, 009A1972: bit 1 of both, then 0099F1C0.
  - Both run in mode 6 as well as in free flight.
- **Exit.** 009A1995 clears pilot+260h, the vehicle arm's one-tick skip slot. It has no known writer.

**The enable mask does not exempt the dive.**
- pilot+2E4h is the task's per-tick mask:
  - the pilot think stores 0FFh before the task arm (00999944);
  - the dive-bomb tick sets its scratch +4C4h = 0FFh (009C87A3) and copies it to +2E4h after the
    state tick (009C8855);
  - the torpedo tick does the same through +49Ch (009D4865, 009D48FF).
- A byte-pattern census found no clearing write. It covered every MOV, AND, OR and register store
  at displacement 4C4h or 49Ch, dword and byte, image-wide.
  - The only other hits are CMPs (009AAFDF, 009AB189, 009D321C, 009A9CA8).
  - One accessor takes the address: 009C8D20 returns task+478h for a flight leader and task+4C4h
    otherwise. It has no code or data reference.
- So all three arms stay enabled through the dive-bomb and torpedo tasks. The squadron's +3A4h has
  no host producer and is taken as set, labelled, as in section 5.

**Nothing reads +26Ch but a debug string.** The vehicle arm sets bit 2 with every insert. The only
reader, 009A1A90, turns bits 4, 2 and 1 into status names.

### 6.1 The vehicle arm, 0099B670 -> 007DF4F0

0099B670 (body 0099B670-0099B688) is `if (unit+C50h) 007DF4F0(pilot)`.
- **ABI.** ECX = the unit+C50h neighbour object and the pilot is the one stack argument:
  `__thiscall(neighbours, pilot)`, `RET 4` at 007E073C, body 007DF4F0-007E073E.
- **The C50h object.** docs/PLANE_GUNFIRE.md section 3 has its creation and refresh. Its +30h list
  holds every entity answering IsKindOf(6) or (0Fh), of either party, within R = 1080. It is
  refreshed every 3 s by 007E11D0.
- **Tuning.** The body reads no tuning key. The Avoidance/Vehicle keys in planeglobals.lua have no
  reader here: MaxWeight, WeightInc, WeightDec, AvoidSpdMul, MinCollTime, MinPlaneSpd,
  MaxDistMultiplier, MinDistMultiplier, UseRollStrength and BomberVSSmallPlane.

**Preconditions (007DF500-007DF53B).** The unit is `[this+4] - 310h`. The unit must be in free flight
or mode 6, and the list must be non-empty.

**The own frame M (007DF5E0-007DF959).**
- If speed > 10 ([00CE38B8]) and |unit+B04h| / speed > 0.1 (double [00D7A3A0]):
  - M is built from forward = normalize(velocity + unit+B04h) and right = the pose's right row;
  - 0085D770 re-orthonormalises the pair and 00B632D0 inverts;
  - if |forward . right| >= 0.99 (double [00CED5D0]), 00414E10 supplies the cached inverse instead.
- Otherwise M is the inverse pose at unit+110h.
- The own half extents from class+50h's box (+28h..+3Ch, min xyz then max xyz):
  - hx = 1.2 x max(-minX, maxX), double [00CEC160];
  - hy = 1.5 x max(-minY, maxY), double [00CE3D78].

**The walk (007DF96C).**
- Null entries are skipped, and so is the entry equal to pilot+260h.
- An entry answering IsKindOf(0Fh) takes the aircraft branch. Every other entry takes the ship
  branch.
- Nothing tests party, squadron, flight leader or task state, so wingmen are tested against their
  own leader.

**Aircraft branch (007DF99C-007DFE85), verified term by term on the listing.**
- Setup:
  - Lp = M P1 (004142E0), Lv = M (V1 - V0) (0042D0D0).
  - Closing only: Lp . Lv < 0 (007DFAB6).
  - d = |Lp|, and R = the two classes' +500h summed.
    - +500h is the largest box half extent, max over axes of max(-min, max), written at
      007C4CDC-007C4D78.
  - s = clamp(|Lv|, 2.0, 80.0): 00415620, [00CE3958], [00CE5444].
- Range test. When d > R: skip if s < 1e-4 (double [00D7A268]) or (d - R) / s > 5.0
  (double [00D7A370]).
- Closest approach. When d > R: t = -(Lp . Lv) / s^2, C = Lp + t Lv and dca = |C|. Otherwise
  C = Lp, dca = d and t = 0.
- Weight:
  - w = interp(0.8 -> 1.25, 1.6 -> 0; dca / R) x interp(0.5 -> 1.6, 5.0 -> 0; t).
  - Constants: [00CE74F8] [00CF29A8] [00D06BB4]; [00CE3800] [00CE3850].
  - If Lp.z < 0 (behind), w -= 0.5 (double [00D7A280]).
  - Skip unless w > 0.
- Direction. n = C / max(dca, R).
- Bands, with x = n.x +- w and y = n.y +- w:
  - x goes into the yaw set pilot+4h (007DFD75) when the x band excludes 0 or the y band contains 0;
  - y goes into the pitch set pilot+CCh (007DFDC4) when the y band excludes 0 or the x band
    contains 0;
  - so a threat off one axis is dodged on that axis alone, and one dead ahead on both.
- Throttle (007DFDD3-007DFE7F):
  - Condition: both bands reach into the central +-0.3 ([00D06888], [00CE69C8]), s < 35
    ([00CE4D90]), Lp.z > 0 and t < 2.0.
  - Action: pilot+25Ch = interp(3 -> 1, 15 -> -1; s) ([00CE3854], [00CE5380], [00D7A260]).
  - It is a plain store, so a later contact in the walk overwrites an earlier one.

**Ship branch (007DFE8A-007E070B), verified the same way.**
- Filters:
  - Closing: (V0 - V1) . (P1 - P0) > 0 (007DFF68).
  - Altitude: skip when unit+9B4h > (the ship's maxY - minY) + 5.0 (007DFF74-007DFF92).
    - unit+9B4h has no literal per-tick writer (only the constructor's MOVUPS at 0081EEBB).
    - Every reader (007BE58E, the accessor 007C4D90, the dogfight maneuver pitch in docs/DOGFIGHT_ENGAGED.md, and 0099F1C0) uses it as height above the surface.
- Gap:
  - gap = |P1 - P0| - (own +500h + 5) - |P1 - Q|, floored at 1.0 (007DFFAA-007E002C).
  - Q is 00816410's point, where the ray from the ship's origin toward P0 leaves its box.
- Time test. tc = gap / max(1, |V0 - V1|); skip if tc > 1.5 / own PitchSpd (class+1ACh) (007E0080).
- Target point. L = M P1 (00414D10), its x and y moved toward 0 by hx and hy, with a dead zone
  inside (007E00B5-007E012E).
- The ship in M:
  - its right, up and forward rows, and its velocity Vo, are brought into M, x and y only;
  - k = clamp(1.25 tc, 0.7, 2.0) (double [00CF87C0], [00CE3E18], [00CE3958]);
  - Cmax = maxX r + maxY u + maxZ f;
  - Cmin = minX r + minY u + minZ f.
- Scale. Z = (0.5 / PitchSpd) x gap (double [00D7A280]), divided by 1.4 in mode 6
  (double [00D045F0]).
- Bands:
  - x band = sort((Vo.x + L.x + k Cmax.x) / Z, (L.x + k Cmin.x) / Z); the y band likewise;
  - lo is floored at -1.1 ([00D06BB0]) and hi capped at 1.1 ([00CE6448]).
- Mode 6, when the x band straddles 0:
  - the x band is widened to at least [-0.2, 0.2] (double [00D06BA8] and [00CE3D10]);
  - pilot+25Ch = interp(3 -> 1, 8 -> -1; own speed) ([00CE3918]).
- Inserts:
  - yaw, x band into pilot+4h (007E0693): when xhi > -1, xlo < 1 and the y band straddles 0;
  - pitch, y band into pilot+CCh (007E0706): in free flight only, when yhi > -1, ylo < 1 and the
    x band straddles 0.
- It never touches the roll set, any timer or any unit field.

**0099B790, the insert, in full (0099B790-0099B935).**
- The set holds up to 24 pairs at set+4h, with the count at set+C4h.
- The new band is clamped first (lo <= -1 becomes -5, hi >= 1 becomes 5). The walk then widens a
  running union over every pair it overlaps.
- The union replaces the first overlapped pair, and the other overlapped pairs are removed by moving
  the last pairs in.
- With no overlap the band is appended while fewer than 24 are held.
- 0099B940 moves a value inside a pair to that pair's nearer edge. It returns false only for a pair
  wider than [-1, 1].

**0099BF30's throttle arm (0099C130-0099C1FF).** When +25Ch < 1:
- if the limit is not positive, throttle = 0.01 ([00D7A238]) and air brake = max(cmd[4], -limit);
- otherwise throttle = min(cmd[3], limit).

### 6.2 The terrain arm, 0099F1C0 with 0099CAB0

The body is 0099F1C0-009A17CB, with `__thiscall(pilot)`. The shaper 0099CAB0 has
`__thiscall(pilot, pass, lo, hi, dLo, dHi, R, bankF)`, `RET 1Ch`, body 0099CAB0-0099D03B. The terms
were read from the listing and cross-checked with the decompile. The C++ in `src/game_hosts_units.cpp`
follows the listing's order.

**Branches.**
- **A, on the water (unit+900h == 6), 009A1420.** A heading probe of +-30 degrees
  ([00CEC724]/[00CEC728]):
  - 00903BC0 tests both probe ends at y = 0.1, over a length interp(0 -> 250, MinControlSpeed ->
    140) from TakeOffMaxLength and TakeOffMinLength.
  - When the probe is clear the branch returns. When blocked, it searches headings +-k pi/12 and
    inserts one yaw band, [-1.1, 0.9] or [-0.9, 1.1].
  - It then stores +25Ch = interp(1 -> 1, 8 -> -1; forward speed).
- **B, free flight.** It needs the squadron (unit+9D4h) and its avoid-zone layer ctl+34Ch, chosen by
  slope 1.5 (docs/TORPEDO_RUN_IN_PATH.md).

**Height.**
- h = lerp(layer sample 0041BC20, ground 00903860, pilot+268h).
- pilot+268h is only ever stored as 0: the constructor at 0099BEC1, and 0099D300 at 0099EB9D (base
  pilot+4h). So the world ground height is never consulted; the layer is the height.

**Safe altitude sa (0099F249-0099F46A).**
- It starts at W / 5 (W = class+A4h Width). This installation's Terrain/SafeAlt {12, 30} and
  TimeTick are never loaded (docs/GAME_TUNING_SINGLETON.md), and nothing else here is tuned.
- The folded |bank| over pi/2 multiplies it by 8 and marks the plane inverted. Otherwise it is
  multiplied by interp(0.1 -> 1, 1.2 -> 8; |bank|).
- It then adds four terms:
  - 4 W |pitch| when pitch < 0 (double [00D7A328]); pitch is unit+C7Ch, an elevation angle 007C1CA8
    writes;
  - interp(80 -> 0, 460 -> 2W; unit+9B4h);
  - interp(1.4 MaxSpd -> 0, 2.5 MaxSpd -> 3W; forward speed 007D99C0).
- It is floored at 10.

**Look-ahead.**
- D = V T, a double, where:
  - V = max(StallRangeMax x StallSpd (007C4830), unit vtable[204h]);
  - T = max(1.5 / PitchSpd, 3).
- The origin is P0 = pos - 10 F', with F' = F + c L, where:
  - L = -right row;
  - c = interp(-0.15 -> 0, 0.2 -> 2.5; -pitch) sin(bank) cos(pitch) SlideRatio YawSpd.
  - The normalize at 0099F62F writes a slot nothing reads, so F' stays unnormalized.

**Pass 0 (yaw set pilot+4h).**
- R = interp(0.3 -> 0.6, 1.2 -> 1.2; |bank|) D.
- Edge points A = P0 + 0.8 R (F' + L) and B = P0 + 0.8 R (F' - L); centre M = P0 + R F'.
- The origin clearance is c0 = max(P0.y - (h + 10), 0.1).

**Pass 1 (pitch set pilot+CCh).**
- F is fresh, and U' = 1.25 x up row.
- Inverted: F += 0.15 cos(bankF) cos(pitch) U'.
- Otherwise with pitch < 0: F.y *= 1 - interp(10 -> 0, 50 -> 1; unit+9B4h) x
  interp(-0.4 -> 0, -0.15 -> 1; pitch).
  - So below -0.4 rad the fan points along the nose.
- R = 1.1 D, A = P0 + 0.75 R (F - U'), B = P0 + 0.75 R (F + U'), M = P0 + R F. P0 and c0 are pass 0's.

**Sampling (0099FF50-009A13E4).**
- Steps:
  - N = trunc(extent / min(layer+0Ch, 120)) + 1 steps j, with t = j / N;
  - s starts at min(1 / N, 1) and grows by 1 / N.
- Each step samples two half-lines:
  - from E = P0 + t (M + s (A - M) - P0) through C = P0 + t (M - P0) (stick -1 to 0);
  - then from C to E' (stick 0 to +1);
  - n2 = trunc(1.2 max(|dx|, |dz|) / cell) + 1 samples per half.
- A sample is blocked when y - (h + sa) < 0.
- Runs of blocked samples become stick intervals:
  - each crossing is interpolated linearly between the neighbouring samples;
  - an interval starting blocked at E is open at -1.1;
  - one still blocked at E' is closed at +1.1;
  - dLo and dHi are the crossing points' distances from P0;
  - an open end takes c0 / (c0 - c) of its sample's distance.
- Each closed run goes to 0099CAB0.

**0099CAB0.**
- Return if lo >= hi.
- Fade g:
  - interp(pilot+264h -> 0, 0.35 -> 1; bankF) for yaw, where pilot+264h = 0.12 ([00CE81A8]), stored
    by 0099D300 at 0099EB95;
  - interp(1.3 -> 1, 1.7 -> 0; bankF) for pitch.
  - If g < 1, lo = 1.1 + g (lo - 1.1) and hi = g (hi + 1.1) - 1.1.
- The ratio dLo / dHi is clamped to [0.56, 1.8].
- For a band covering stick 0, m = min(-lo, hi):
  - pitch with m > 0.3: tgt = interp(0.3 -> 1.8 MaxSpd, 0.7 -> LevelFlight + 20; m). When
    tgt - speed < 30, +25Ch = min(+25Ch, interp(-20 -> -1, 30 -> 1; tgt - speed));
  - yaw with m > 0.4: unit+BC4h = max(unit+BC4h, 2 (m - 0.4) sin|bank| + 1), through 0099BB00.
- R scaling:
  - pitch returns when both dLo and dHi exceed R, then R *= 1.1;
  - yaw takes R *= 0.7.
- Each inner edge moves by e = dHi / min(R, 200) - base, where base = interp(0 -> 0.3, 0.08 -> 0.5;
  m) for a covering band, else 0.3:
  - if e >= 0, the edge shrinks by e^2;
  - otherwise it widens by interp(0 -> 0.001, 0.1 -> 1.1; m) |e|, or by 0.001 for a band that does
    not cover 0.
- The band is inserted into pilot+4h + pass x C8h.

**No exemption.** Nothing in either body reads a task state, a dive flag or a unit kind. Only pitch,
bank, unit+9B4h and speeds shape the fan. A steep dive raises sa by 4 W |pitch| and keeps the pitch
fan along the nose. So a Val diving toward the sea at a few hundred metres gets a pitch band covering
stick 0, and 0099BF30 moves its pitch command to the band's upper edge.

### 6.3 The host binding

`kPilotVehicleAvoidanceBound` and `kPilotTerrainAvoidanceBound` are separate switches because the
arms act on different aircraft and rows. Both run inside the landed 009A17D0 update, in the image's
order.

**Band machinery.**
- The sets hold the merged 24 pairs of 0099B790, and 0099B940 walks them.
- 0099BF30's +25Ch throttle arm is bound. It is inert at 1.0, which is all the gunfire arm leaves.

**Stand-ins, all labelled in the source.**
- The squadron's +3A4h bits are taken as set.
- pilot+260h is never set.
- unit+B04h is taken as zero, so the frame is the inverse pose.
- The class+50h model box is Width/Height/Length, symmetric about the origin, and class+500h is
  derived from it.
- unit vtable[38h] and [204h] are |velocity|.
- unit+C7Ch is the velocity's elevation angle.
- **Water surface for the missing heights.** Both the avoid-zone layer height and the ground under
  unit+9B4h are taken as the water surface. The layer's cell is taken as the scene's 100 m.
- The C50h list refresh runs on the think interval. "In the world" means a live state object not
  marked destroyed.
- Branch A (on the water) returns at its first test, because the host has no ground for 00903BC0.
- unit+BC4h (0099BB00) has no host consumer and is not modelled.

### 6.4 Predictions, written before the pair

The pair is V0 (both new switches off) against V1 (both on), E2 9000, stream option on, on main
20f04e555 plus this packet. The reference is J1: releases 0/0, Japanese deaths 35, fighter hits 64,
15 water contacts, and the Lexington alive.

**Who flags vehicle avoidance.**
- Only Japanese aircraft, because US planes are on the player's party.
- Mostly wingmen against their own flight. With R near 14 m and s clamped at 2, a closing mate
  within about 25-40 m gets bands.
- Predicted plane-flag ticks: 100-3000.
- Ship flags need height above the sea under the ship's Height + 5. Only low Kates can meet that,
  near ships: 0-100 ticks.

**Terrain flags.**
- **Val dives: flagged.** J1's Vals leave aimdive at 170-240 m ("db aim exit" lines). At those
  heights the pitch fan's centre ray reaches the sea, so dive-state terrain ticks will be above 0.
- This is the case the brief says to stop on. If dive-state flags appear, the terrain switch is not
  landed on and the finding is reported.
- **Low Kate runs: flagged.** All of Kate #8.1's flight met the water in J1.
- Water contacts: 15 predicted to fall to 0-5.

**Band repairs.** Above J1's 103, predicted 1000-20000.

**Headline rows.**

| row | J1 | predicted |
| --- | --- | --- |
| releases | 0 / 0 | 0 / 0; the release gap is upstream |
| Japanese deaths | 35 | 28-40; AA kills are coupled to paths through the shared random stream |
| fighter hits | 64 | 30-90; coupled the same way |
| Lexington | alive | alive |

### 6.5 The pair, measured (V0 against V1)

`local\V0_9000.log` has both new switches off, and `local\V1_9000.log` has both on. Both finish the
loop normally.

**V0 reproduces J1 on every row.** That includes 35 deaths, 469 hits, 103 repairs and 15 water
contacts, so the merged band sets change nothing for the gunfire arm alone.

| row | V0 (off) | V1 (on) | prediction | verdict |
| --- | --- | --- | --- | --- |
| vehicle flags (plane / ship ticks) | 0 / 0 | 194 / 0 | 100-3000 / 0-100 | held |
| vehicle bands / throttle stores | 0 / 0 | 336 / 14 | - | - |
| terrain flag ticks / bands | 0 / 0 | 1099 / 2980 | - | - |
| terrain flags in a Val dive state | 0 | **0** | above 0 | **missed**: no Val ever flags terrain |
| terrain flags on low Kate runs | 0 | every Kate flight | flagged | held |
| band repairs (all arms) | 103 | 333 | 1000-20000 | **missed**: fewer |
| water contacts | 15 | 15, the same units and speeds | 0-5 | **missed**: see below |
| releases (torpedo / bomb) | 0 / 0 | 0 / 0 | 0 / 0 | held |
| Japanese deaths | 35 | 35 | 28-40 | held |
| AA hit records / hull hits | 469 / 183 | 461 / 176 | coupled | path coupling |
| fighter hits | 64 | 64 | 30-90 | held, unmoved |
| Lexington | alive | alive | alive | held |

**Who flags.**
- **Vehicle.**
  - Kate wingmen against their own flight: #2.1 and #6.1 .-4 flag 59 and 77 ticks, with 6 throttle
    stores each; the .-2 wingmen of #2.1, #4.1, #6.1 and #8.1 flag 4-9.
  - Val #7.1 .-2 and .-3: 15 and 12 ticks.
  - Two Zero leaders: 2 each.
  - No ship flags. No aircraft flew below a ship's Height + 5 while closing within the time test.
- **Terrain.**
  - Every Kate, 24-92 ticks each, at heights of 15-105 m above the sea. These are the torpedo
    approaches.
  - One Zuiho fighter, 29 ticks at 100 m.
  - No Val and no terrain throttle store (+25Ch).

**Why no Val dive flags.**
- The host's Vals dive slowly, about 53 m/s unpowered (docs/PILOT_THROTTLE_SLOT.md), and J1's leave
  aimdive at 170-240 m.
- At that speed the pitch fan's reach is about 3 s x max(1.6 StallSpd, speed), roughly 160 m by estimate. So the
  centre ray stays above sea + sa.
- The image has no exemption (6.2). A faster dive would be flagged. This pair shows the binding does
  not pull any host Val out of its dive.
- The brief's stop condition was not met.

**Why the water contacts do not move.**
- Every one of the 15 is a wreck. The "entity dead" line precedes each contact, for example Kate
  #8.1 dies at 208.8 s before its contact, and 009A17D0's +5Dh gate skips dead planes.
- The prediction misread J1's contacts as live aircraft.

**The AA picture.** Deaths are 35 on both sides. The hit records move 469 to 461 through path
coupling on the shared random stream. Fighter hits are unmoved, because the fighters' target flight
(Val #1.1) never flags either new arm.

**Switch states landed: `kPilotVehicleAvoidanceBound` ON, `kPilotTerrainAvoidanceBound` ON.** Both
follow the listing at every term read, with the labelled stand-ins of 6.3. The terrain arm's
behaviour over islands waits on the avoid-zone layer and ground height, which this host replaces
with the sea.
