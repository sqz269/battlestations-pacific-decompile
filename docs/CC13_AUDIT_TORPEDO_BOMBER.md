# Audit of the torpedo-bomber aim and release path (cc13-audit10)

Addresses: 009D15F0 009D1C0B 009D1C51 009D1C84 009D1D02 009D1DD6 009D1E9A 009D1EDD 009D1F36 009D1F6B 009D20C4
009D2165 009D2209 009D2287 009D22C0 009D3420 009D3440 009D34A9 009D4850 009D4865 009D48FF 0099CAB0 0099CAF2
0099D300 0099EB89 0099EB95 0099EB9D 009A17D0 009A1990 009998A0 00419010

Packet `cc13-audit10`, branch `agent/cc13-audit10`, 2026-10-12. cc9 requested it through the cc13 lead. **Read only.**
This audit made no Ghidra writes, added no ledger records and edited none of cc9's files. Every change it proposes
is in the one unified diff at the end, and every hunk there sits behind an OFF switch. Base: `origin/main` at
`78bb7c0dd`.

**Method.**
- Each host term was compared with the disk listing (`bsp.py disasm-raw`), reading ESP by hand from each
  argument block. The host's `F=` slot names were checked against the listing's `[ESP+n]` at four loads:
  `009D1D62`, `009D1E53`, `009D1F03` and `009D2089`.
- Constants were read from the PE at the load's own width (`tools/pe_const_read.py`).
- Every `FCOMI(P)` and `COMISS` was read with its operand order and its `Jcc`.
- Displacement censuses use `local/au10_disp.py <disp> <lo> <hi>`, which tries every decode that covers the
  displacement bytes. Branch-target and register-writer sweeps use `local/au10_jt.py`. The robots rows were read
  with `local/au10_robots.py`.
- Rule 2 (boundary before attribution): `ghidra proto --brief` places each cited site in its function.
  - `009D1D02`, `009D1F6B` and `009D20C4` are in `BSP_BotStateTorpedoAim_Tick` (`009D15F0`..`009D2379`).
  - `0099EB9D` is in `BSP_PilotBot_PlanControls` (`0099D300`..`0099EBAB`).
  - `0099CAF2` is in `BSP_PilotBot_ShapeTerrainBand` (`0099CAB0`..`0099D054`).
  - `009A1990` is in `BSP_PilotBot_UpdateAvoidance` (`009A17D0`..`009A19A2`).
  - `009D4865` and `009D48FF` are in `BSP_BotTaskTorpedo_TickArm` (`009D4850`..`009D4967`).
- Rule 7 (argument counts): `00419010` is `RET 14h`.
- `tools/verify_report_calls.py` takes a `reports/*.json`. This audit has no host table, so there is nothing for it
  to check.

**Status vocabulary.** Every function named here is reconstructed and build-tested on main. This audit is a static
second reading. It adds no runs and no game validation. The diff was apply-checked and build-tested in this tree
with every switch OFF.

## The release-blocker verdict

**The gate that blocks US TBDs is the image's own altitude flag, `009D20C4`. The torpedo-bomber code computes it,
and every term feeding it, as the image does.** The altitude the TBD holds (106-140 m, docs/GUNNERY_OPEN_ITEMS.md
143.1, 148 and 157) comes from the planner's terrain-band repair. For the attitude the host flies, that repair is the
image's arithmetic, apart from one substitution. That substitution makes the host **more** permissive than the image,
not less (finding 3). For a TBD banked 0.19-0.33 rad, the image therefore computes the same hold or a higher one.

The holding at 106-140 m is **not a defect of the torpedo-bomber code**. What this audit does not settle is the
attitude itself: why the TBD holds a steady 0.19-0.33 rad bank on a 0.02-0.05 rad heading error while the Kate
flies wings level. That bank is the output of the planner's roll arm (`0099E2BA` / `0099D0A0`). The roll arm chases
a bearing that `009D3586` refreshes every tick toward the lead point. Every torpedo-task input to that arm errs
toward **less** bank in the host than in the image (findings 4 and 5). If the hold must change, look in the roll
arm. Nothing in this packet would make the image descend where the host does not.

### Which gate blocks, and why

`009D202C`..`009D2233` builds five flags. All five are needed for the ground arm at `009D2239`. The cone flag alone
opens the countdown.

| flag | listing | host | needs |
| --- | --- | --- | --- |
| state | `[ESP+13h]` from `state+24h` | `gate_state_24` | the state's own byte clear |
| lead | `009D2052`: `interp(0.5, 1, 1, approach+84h; |cos aspect|) * speed > F14` and `F14 + 80 > approach+A0h` | `gate_lead` | in the envelope |
| **altitude** | `009D20B4` `00419010(x0 0.4 [00CE7804], y0 40 [00CE685C], x1 1.0, y1 25 [00CE89CC]; x F0C)`, then `009D20B9` FLD the double altitude (`unit+100h`), FXCH, `009D20C4 FCOMI`, `JA` sets | `gate_altitude = alt_gate > unit_altitude` | **altitude below 25-40 m** |
| bank | `009D210A` `0.8 > |unit+C68h|` (else clear); then `009D2165` `unit+C64h > min(F14 * 0.006 - 0.8, -0.1)`, `JA` | `gate_bank` | wings within 0.8 rad, nose not below about -0.1 rad |
| cone | `009D2209`: `cone_deg * pi/180 > |F10|` | `gate_cone` | heading within 15-80 deg |

- The altitude flag compares a fixed band, 40 m at `F0C <= 0.4` easing to 25 m at `F0C >= 1.0`, against the
  aircraft's height.
- No robots value enters it. `TorpReleaseAlt` reaches the aim tick only through `approach+78h`, which shapes the
  floor `F=28h` (5-30 m) used by the pitch command, never this flag (item 5).
- Every TBD row of 143.1 reads `alt=0` with `min_alt` 106.0-139.7, and the other four flags open. **The altitude
  flag is the only one that blocks.**

### Why the altitude stays at 106-140 m

1. **The aim tick commands the dive, faithfully** (finding 1).
   - The command is `plan+2BCh = clamp(-F34 / max(F14 - 1200, MaxSpd * den), -DEG(80), +DEG(50))` with mode
     `plan+2D0h = 1`.
   - `F34 = (alt - floor) * max(1 - 1.6 |F10|, 0.1)`.
   - At 110 m and inside 1200 m of range this is about -0.38 rad. 143.1's -0.18 to -0.49 agrees.
2. **The planner's pitch arm saturates nose-down**, but `0099BF30`'s repair moves the command to `hi`, the upper edge
   of the pass-0 terrain band. The band comes from `0099CAB0` and is fed by `0099F1C0` (148, 157).
3. **Pass 0's band shaper is ramped in by the bank.**
   - `g = 00419010([pilot+264h], 0, 0.35 [00CF6560], 1; bankF)` (`0099CAD4`..`0099CB19`).
   - `g < 1` pulls `hi` toward -1.1, the steep end, which relaxes the band.
   - A wings-level Kate has `g` of about 0, so its pass-0 band vanishes. A TBD banked 0.19-0.33 rad has `g` of
     0.3-0.9, so its band survives with `hi` of +0.02 to +0.24.
   - The image reads `pilot+264h` = 0.0, where the host uses 0.12 (finding 3). The image's `g` is therefore larger,
     and its band stricter.
4. **The image does run the terrain pass during `aim`.**
   - `009A1974`/`009A1990` call `0099F1C0` when `plan+2E4h & 1` and the squadron's `+3A4h & 1`.
   - `BSP_BotTaskTorpedo_TickArm` sets `task+49Ch = 0FFh` (`009D4865`) and copies it to `plan+2E4h` after the state
     tick (`009D48FF`/`009D4905`). The only writes of `+49Ch` in `00999000`..`009E0000` are those two.
   - The host's gate comment (`game_hosts_units.cpp`, the `009A1865/009A194F/009A1972` block) agrees.

## Findings

| # | item | verdict | one line |
| --- | --- | --- | --- |
| 1 | aim descent command `009D1D44`..`009D1EE5` | FAITHFUL | threshold, damper, denominator, FCHS/FDIV order, clamps, constants |
| 2 | five-flag chain and ground arm `009D202C`..`009D2287` | FAITHFUL | senses and constants as the table; the arm refuses re-arm and zeroes the countdown |
| 3 | band shaper `pilot+264h` (`game_hosts_units.cpp`, `terrain_shape_band_0099cab0`) | DEFECT (permissive) | the image reads 0.0 (`0099EB9D`), the host 0.12; the 0.12 is `pilot+260h` (`0099EB95`) |
| 4 | aim plan stores dropped by the host | DEFECT | `+2E8h` = 1.4, throttle 1.0 and brake 0.0 with their bytes, `+2D8h` = 0, the `+264h` fade |
| 5 | aim class inputs stood in | DEFECT (labelled stand-ins, misdescribed) | TurnRoll 1.0 for 1.047, Width 1.0 for 15, TurnCircleRadius 0 |
| 6 | `torpedo_approach_update.cpp:589` `roll_limit_2e8 = 0.0f` | DEFECT, dead code | the image stores 1.4f; the function has no caller |
| 7 | the profile head `009D3433`..`009D34A9` | FAITHFUL | `+78h` zeroed unless `ctl+3ADh`; speed-ceiling rescale with FDIVR; `+74h = ctl+398h` when `< 100` |
| 8 | countdown stores `009D22AB`/`009D22C0`/`009D22CF` | UNMODELLED (not applied) | `[approach+1Ch]+40h = 0` and `approach+A4h = 3/0FFh`; outside the altitude question |
| 9 | robots `TorpRelease*` | INSTALLATION NOTE | SPVeteran 5/800/1200/0.5 against the backup's 10/480/1000/0.7; none of them enters the blocking flag |

**1, FAITHFUL.**
- **Threshold.** `009D1D54 FLD [unit+100h]`, `FSUB F=28h`. The turn factor is `FLD F=18h`, `FMUL qword [00CE3D48]`
  (1.6000000238), `FLD1`, `FSUBRP`, so it is `1 - 1.6 |F10|` in that order. The margin is floored at -0.5
  (`[00CEC9E0]` double, `[00CE69D0]` float), the factor at 0.1 (`[00D7A3A0]` double, `[00D7A2F0]` float). The
  product is `F=3Ch` (`009D1DED`).
- **Damper.** `009D1DD6 [00D7A208]` is -0.0f. `subss` gives `-0.0 - pitch`, and `COMISS` against `[00D7A218]` = 0.0
  with `JBE` runs the damper only while the nose is down.
  - `sel` is 2.5 (`[00CF87C8]`) for kinds 10h/16h, else 1.5 (`[00CE380C]`).
  - `den = (-pitch / [desc+1ACh]) * sel + 3.0` (`009D1E49 FADD qword [00D7A2B0]`), stored as a float.
  - The host rounds `(-pitch / PitchSpd)` and the product to float before the double add, where the image keeps them
    on the x87 stack. That is the precision class of docs/CC13_FPU_PRECISION.md, below a ulp of `den`. It is
    recorded, not proposed.
- **Denominator.** `009D1E53 FLD F=14h` (`[ESP+1Ch]` at that depth), `FSUB qword [00D1FAF8]` (1200). Then
  `009D1E64 FLD [desc+188h]` MaxSpd, `FMUL den`. `FCOMI` with `JBE` keeps `R - 1200` unless the product is larger:
  `max`. `desc` is `[approach+8h]`, reloaded at `009D1E57`.
- **Quotient.** `009D1E8E FLD F=3Ch`, `FCHS`, `FDIV F=50h`: `-F34 / den`.
  - The low clamp is the float `[00D21318]` = -1.3962635. `FCOMI lo ? x`, and `JBE` keeps `x` when `lo <= x`.
  - The high clamp compares `x` against the double `[00D057E0]` (0.8726646304) and stores the float `[00D05B40]`.
  - The host compares against the float. The two differ only at `x` within half a ulp of 0.8727, on the climb side.
    Recorded, not proposed.

**2, FAITHFUL.**
- **Altitude flag.** As in the table. The x is `[ESP+28h]` after `SUB ESP,14h` (`009D2082`), which is `[ESP+14h]` =
  `F=0Ch` at frame depth. That is the time-to-target, the slot `009D1F03` reads as `F0C`.
- **Bank flag.** `|unit+C68h|` is formed with `-0.0 - x` (`009D20EE`). `FLD qword [00CE3D40]` (0.8f widened),
  `FCOMI`/`FSTP ST1`, and `JBE` clears the flag.
  - `F14 * [00CEC390]` (0.006, double) `- 0.8`, then `min(., -0.1)`: `[00CE3928]` double, `[00CE3CB4]` float.
  - `JA` sets the flag on `pitch > limit`.
  - The x87 stack keeps the 0.8 across the arm; it is the image's own balance and has no host analogue.
- **Ground arm.** It needs `[00E188A8]` and its `+19CCh` to be non-null, then `1.0 > 00903860(pos)` (`009D2272
  COMISS`, `JBE` skips). It refuses when `[state+0Ch]` is already set (`009D2279`). The host's
  `release_timer_arm_009d2287` refuses re-arm. It does not model the null-world skip. That skip is moot whenever a
  scene is loaded.

**3, DEFECT (permissive; the fix makes the hold stricter).**
- `0099CAF2 FLD [EBP+264h]`, with `EBP = ECX` at `0099CAC0`, is pass 0's x0.
- `0099D300` ends with `0099EB89 XORPS XMM2,XMM2`, `0099EB8C MOVSS XMM0,[00CE81A8]` (0.12),
  `0099EB95 MOVSS [ESI+260h],XMM0` and `0099EB9D MOVSS [ESI+264h],XMM2`. The bytes at `0099EB9D` are
  `F3 0F 11 96 64 02 00 00` (`scan-bytes`).
- **Five branches enter at `0099EB8C`, past the XORPS:** `0099EA7F`, `0099EA8B`, `0099EAB5`, `0099EAF6` and
  `0099EB26` (`au10_jt.py`).
  - The compiler shares one XMM2 between the paths that zero it explicitly and those that arrive with it already
    zeroed by an earlier XORPS (`0099EA3B` is the nearest). Each path was not traced back individually.
  - A store of a non-constant could not be fed by a bare XORPS on one of its paths, so the value is the constant
    0.0 on every path.
  - No branch targets `0099EB8E`..`0099EBA5`, so every exit of `0099D300` passes the store.
- **Writers of `+264h`** in `00999000`..`009D6000` (au10_disp): the constructor `0099BEC1`, `0099EB9D` (0.0) and the
  aim tick `009D1F36` (the fade, only when `F0C < 1.5`).
  - `009B2FDB` (`BSP_BotApproachLand_ConstructComposite`) and `009B31CF` (`FUN_009B3040`) store `+264h` of a land-approach object, not the pilot. They were not read further.
- **Order.** `009998A0` runs the task arm (`vtable[64h]`, `0099995A`), then `009A17D0` (`0099999B`), then
  `0099D300`. The band therefore sees the aim's fade on the tick it is written, and 0.0 otherwise.
- **Host.** `terrain_shape_band_0099cab0` hardcodes `avoid_interp(0.12f, 0, 0.35, 1, bankF)`, and its comment
  attributes the 0.12 to `+264h` at `0099EB95`. docs/GUNNERY_OPEN_ITEMS.md 157's table row "`0099CAB0` `g` ...
  `[pilot+264h] = 0.12` | same" inherits the misreading.
- **Effect.**
  - For `bankF` in (0, 0.35), the image's `g = bankF / 0.35` exceeds the host's `(bankF - 0.12) / 0.23`.
  - At bank 0.25: image 0.71, host 0.57. At 0.19: image 0.54, host 0.30. At 0.10: image 0.29, host 0, where the
    host drops the pass-0 band entirely.
  - A larger `g` keeps `hi` nearer its raw value, which is a stricter band. A wings-level aircraft is barely affected.
- The diff binds `pilot+264h` as a slot field, zeroed where `0099D300` would zero it and written by the aim's fade,
  behind `kBandShaperPlan264Bound` (OFF). Expect it to raise banked holds, and to add pass-0 bands for aircraft
  banked 0.0-0.12 rad that the host now never bands. Pair it on USN04, a Kate mission (JM06 or USNRM01) and a
  fighter mission, since every terrain-avoiding plane runs this shaper.

**4, DEFECT.**
- After `009D1CF7` the aim tick stores `plan+2E8h = [00D06874]` (1.4f) unconditionally (`009D1D02`).
  - The host's live tick returns it as `r.roll_limit_2e8`, and nothing copies it into `plan_state.turn_scale_2e8`.
  - The planner's reset reseeds 1.0 every think (`0099B52C`), so the host's aim TBD banks at 1.0 x TurnRoll.
  - **Correction (part 2, docs/CC13_AUDIT_PLANNER_ROLL.md finding 3):** for a TBD wingman the cap binds both ways:
    TurnRollLimitLarge 56 deg = 0.977 < 1.0 x 1.047. The 1.4 is therefore neutral for TBDs, and the sentences below hold
    only for classes whose TurnRoll is under the cap.
  - In the image the roll arm's `max_bank = min(1.4 x TurnRoll, cap)`, and the bank request `raw * max_bank` scales
    with it (`plane_ai_control.cpp`, `0099DFAA`..`0099E1C9`). The image banks **more** for the same heading error.
- **The throttle block** `009D1F42`..`009D1F6B`: throttle desired 1.0 (`[00D7A24C]`) with its byte, air brake desired
  0.0 with its byte, and `plan+2D8h = 0`.
  - The host writes none of them, so the reseed (`kPilotPlanReseedBound`: `+2D8h = 1`,
    `+2B4h = TravelSpeed x NewTravelSpeedMul`) stays live through `aim`.
  - For this installation's TBD (`vehicleclasses.lua` VehicleClass[112], mtime 2026-05-09: MaxSpd 69.44,
    TravelSpeed 61.11), the demand is 97.8. That is above MaxSpd, so the demand arm also drives full throttle. The
    defect is therefore close to neutral for the TBD. Not measured.
- **The fade** `009D1F36` (finding 3's only non-zero writer) is likewise not applied.
- The diff applies all of these behind `kTorpedoAimPlanWritesBound` (OFF).

**5, DEFECT (stand-ins, misdescribed).** The host passes `turn_radius_268 = 0`, `cruise_speed_25c = 1.0` and
`alt_fold_a4 = 1.0`, with a comment that they feed "the turn-radius escape, the throttle and the pitch".
- `009D1C4E`/`009D1C51` load `[[approach+8h]+25Ch]`, TurnRoll (`game_hosts_lua.hpp`: `desc+25Ch`).
  `009D1C84 FLD [[approach+8h]+0A4h]` is Width (`class+A4h`).
- The product `time_fold x TurnRoll x 1.2 x bank_fold x alt_fold`, capped at 1.2 (`[00CE3814]`), is stored at
  `009D1D2E` into `plan+2C8h`, the **bank limit** (the host's own comment at the store says so).
- **TBD at 110 m.** The image gives `1.257 x 1.6`, capped at 1.2; the host gives `1.2 x 1.6`, capped at 1.2. Equal,
  so the stand-ins do not touch the hold.
- **Below 45 m (3 x Width).** The image's `alt_fold = interp(15 -> 0.1, 45 -> 1.6)` tightens the bank limit to
  about 0.75 rad at 25 m and 0.13 rad at 15 m. The host keeps 1.2 down to 3 m.
- `009D1962`..`009D1966` take `max([desc+26Ch], [desc+268h])`, where `+268h` is TurnCircleRadius, into the turn
  room `R_t sin(min(|F10|, 1.2)) + speed + 200`. That only moves the run-time update and probe skip at `009D19A0`.
- The diff feeds the three slot fields the host already carries, behind `kTorpedoAimClassInputsBound` (OFF).
  `desc+26Ch` stays 0, unread.

**6, DEFECT in dead code.** `src/torpedo_approach_update.cpp`'s `torpedo_aim_tick_009d15f0` writes
`roll_limit_2e8 = 0.0f` under a comment citing `[00D06874]`, which is 1.4f (`pe_const_read`). The listing stores it
unconditionally (`009D1CFA MOVSS XMM0,[00D06874]`, `009D1D02`).
- The function has no caller. `game_hosts_units.cpp` runs `bsp::torpedo_aim_tick_full_009d15f0`
  (`src/torpedo_aim_tick.cpp`, which has 1.4), and `reports/torpedo_run_in_velocity.json` already records "no
  caller in this host".
- It is also wrong elsewhere: `plan+2BCh` is the floor altitude instead of a pitch, and the throttle is a speed.
- The diff corrects the literal and marks the function superseded. Deleting it is cc9's call.

**7, FAITHFUL.**
- `009D3433 CMP byte [ctl+3ADh],0` / `JNE`, then `XORPS`, then `MOVSS [ESI+78h]`.
- The ceiling: `FLD [ctl+39Ch]`, `FLD +80h`, `FCOMI`/`FSTP ST1`. `JBE` skips. Otherwise `FDIVR [ESI+7Ch]` (late /
  early), store, then `x ctl+39Ch`, and `+80h = ctl+39Ch`.
- `009D3489..009D34A9`: `+74h = ctl+398h` when `100.0 (qword [00D7A220]) > ctl+398h`.
- The host's `torpedo_approach_apply_profile_009d3433` matches.
- **Consequence for item 5 below.** `TorpReleaseAlt` reaches `approach+78h` only at the reset (`009D046A`). Every
  approach update zeroes `+78h` unless the control block's `+3ADh` is set. The aim floor is
  `max(5.0, +74h + +78h, ground + 5)`.

**8, UNMODELLED.** When the cone is open and `F0C < 3.0`, the tick zeroes `[approach+1Ch]+40h` (`009D22AB`) and sets
`approach+A4h` to 3 (`F0C < 2.0`) or 0FFh (`009D22C0`/`009D22CF`).
- The host computes `clear_slot_1c_40` and `countdown_a4` but applies neither.
- `approach+A4h` has no reader in `009CF000`..`009D5000`. Its other writers are `009D0501`, `009D1344`, `009D2761`
  and `009D2B25`. The reader was not traced.
- Neither store bears on the altitude, so the diff does not include them.

**9, INSTALLATION NOTE.** `scripts/datatables/robots.lua` (mtime 2025-06-01 16:03:10) against `robots__.lua.bak`
(16:03:09), PilotBot rows (`au10_robots.py`):

| row (host index) | TorpReleaseAlt | DistNear | DistFar | DropCloserMul | robots.lua line |
| --- | --- | --- | --- | --- | --- |
| Stun (0) | 12 | 350 | 600 | 0.7 | 1248 |
| SPNormal (1) | 12 | 450 | 650 | 0.7 | 557 |
| SPVeteran (2) | **5** (bak 10) | **800** (bak 480) | **1200** (bak 1000) | **0.5** (bak 0.7) | 696 |
| MPNormal (3) | 10 | 800 | 1200 | 0.7 | 834 |
| MPVeteran (4) | 10 | 800 | 1200 | 0.6 | 972 |
| Elite (5) | 5 | 800 | 1200 | 0.5 | 1110 |

- SPVeteran also differs in `TorpTargetHError` 0 (bak 5), `TorpTargetVError` 0 (bak 10), `TorpCalcTargetPosError` 1
  (bak 6) and `TorpTargetPointSelectPrec` 0.25 (bak 0.9). It looks like an Elite copy.
- **Loading.** The host does not read the Lua at run time for these. `kTorpRows[6][4]` in `game_hosts_units.cpp`
  (the reset at `009D0380`) hard-codes the installed values in host order 0 Stun .. 5 Elite (luamw_init.lua,
  docs/IJN_SKILL_ROWS.md section 4). They match robots.lua, not the backup.
  - The row is `unit.pilot_skill_index` when `kSkillLevelBound`, and it falls back to 1 (SPNormal). The index comes
    from the scene entity's bag skill (`entity.bag_skill`) or from `SetSkillLevel` (`game_hosts_units.cpp`
    `pilot_skill_index = level`).
  - DistNear and DistFar seed the run slots `+7Ch`/`+80h` (scaled by `approach+24h`, then jittered). DropCloserMul
    seeds `+84h` unscaled. TorpReleaseAlt scales the `U(0, 0.25)` draw into `+78h`, which item 7 zeroes again
    unless `ctl+3ADh` is set.
- **Which row the scripted runs use.** docs/CC13_DIFFICULTY_VALUES.md has difficulty 1, Normal, selecting
  `SKILL_SPNORMAL` in the mission scripts. 143.1's TBD floor reading (5.2 m, "SPVeteran's TorpReleaseAlt") puts
  Yorktown's TBDs on row 2. That is consistent with the scenes giving US carrier squadrons a veteran bag skill.
  This audit did not trace the USN04 entity bags, so the row is **inferred, not verified**.
- **Effect on the blocker: none.** No `TorpRelease*` value enters `009D20C4`. Even a floor of 10 m (the backup's
  SPVeteran) is far below the 25-40 m flag. DistNear, DistFar and DropCloserMul move the lead and solution flags
  (`009D1FED`, `009D2034`), which already open for the TBDs in 143.1.

### Items (1) and (3) of the request

- **The approach and descent to the release altitude.**
  - Before `aim` the attackrun commands the altitude through `009FBA50` (docs/TORPEDO_DESCENT_LAW.md,
    docs/TORPEDO_RELEASE_ORDERS.md 7). TBDs enter `aim` at about 1050-1120 m.
  - Inside `aim` the only descent command is finding 1's `plan+2BCh` with mode 1. Its target is the floor `F=28h`,
    and its rate is set by `den`, at most `DEG(80)` nose-down.
  - The planner's pitch slew and `0099BF30` band repair sit between the command and the elevator. Of these, the
    band repair is what binds (above).
  - This audit re-read only the aim half. The attackrun law is cited, not re-verified.
- **The drop and the torpedo entry.**
  - The arm at `009D2287` sets `state+0Ch = 1` and `state+10h = 0`. The host binding's `arm_release_timer_009d2287`
    does the same and logs `timer arm 009D2287`.
  - The drop follows on the next tick through the release spawn (docs/TORPEDO_RELEASE_SPAWN.md,
    docs/TORPEDO_AFTER_THE_DROP.md). No TBD reaches it on USN04, so this audit did not re-read the spawn.

## The diff

The diff below was applied in this tree, built with `./scripts/build.ps1`, reverted, and confirmed with
`git apply --check` on a copy extracted from this doc. All three switches are OFF, so it changes no behaviour.

```diff
diff --git a/src/game_hosts_units.cpp b/src/game_hosts_units.cpp
index b23fab7eb..36aa2625a 100644
--- a/src/game_hosts_units.cpp
+++ b/src/game_hosts_units.cpp
@@ -1776,6 +1776,10 @@ struct GameUnitSlot {
     // another packet. docs/PILOT_THROTTLE_CUT_RAISER.md.
     float plane_desired_speed_2b4{0.0f};
     int plane_air_brake_mode_2d8{0};
+    // plan+264h, the start of 0099CAB0's pass-0 bank ramp. 0099D300 zeroes it
+    // at 0099EB9D on every think; only the torpedo aim tick (009D1F36) writes
+    // anything else. cc13-audit10, behind kBandShaperPlan264Bound.
+    float plan_band_bank_264{0.0f};
     // plan+2B0h, a byte. Its only reader in the plane-AI range is 0099D924
     // `CMP byte [ESI+2B0h],0` in 0099D300's speed-demand arm: ZERO lets
     // 0099D92D-0099D970 raise the speed-error divisor plan+2B8h to
@@ -2126,6 +2130,19 @@ constexpr bool kDiveAimLeadBound = true;      // ON: TORPEDO_AIM_LEAD 11.4
 // hull-point spread, the dive's section chance and weights, and the time error
 // on projtime (dive approach+C8h, torpedo approach+9Ch). Paired ON.
 constexpr bool kAimErrorDrawBound = true;     // ON: TORPEDO_AIM_LEAD 12.2
+// cc13-audit10 (docs/CC13_AUDIT_TORPEDO_BOMBER.md finding 4): the aim tick's
+// plan stores this host drops - plan+2E8h = 1.4f (009D1D02), throttle 1.0 and
+// air brake 0.0 with their bytes (009D1F42-009D1F64), plan+2D8h = 0 (009D1F6B)
+// and the plan+264h fade (009D1F36). Behaviour-changing; pair before ON.
+constexpr bool kTorpedoAimPlanWritesBound = false;
+// cc13-audit10 finding 5: desc+25Ch TurnRoll (009D1C51), desc+A4h Width
+// (009D1C84) and desc+268h TurnCircleRadius (009D1962) into the aim tick, in
+// place of the 1.0 / 1.0 / 0 stand-ins. Behaviour-changing; pair before ON.
+constexpr bool kTorpedoAimClassInputsBound = false;
+// cc13-audit10 finding 3: 0099CAB0's pass-0 ramp starts at pilot+264h, which
+// 0099D300 zeroes at 0099EB9D on every think; the 0.12 stored at 0099EB95 is
+// pilot+260h. Only the torpedo aim tick (009D1F36) writes +264h otherwise.
+constexpr bool kBandShaperPlan264Bound = false;
 // Packet cc9_ship_section_points (docs/TORPEDO_AIM_LEAD.md section 14): the pick
 // 009FA260 hands 00816650 the TARGET ship's section records, the engine room /
 // magazine / fuel tank points 0081F980 builds from the model's GeomMesh
@@ -9904,9 +9921,12 @@ static void terrain_shape_band_0099cab0(GameUnitsHost::Impl& host, GameUnitSlot&
                                         float lo, float hi, float dlo, float dhi, float r,
                                         float bank_f, float level_flight_speed, float speed) {
     if (!(lo < hi)) return;                                          // 0099CABC
-    // 0099CAD4-0099CB19; pilot+264h = 0.12 ([00CE81A8]), stored by 0099D300
-    // at 0099EB95 every think.
-    const float g = pass == 0 ? avoid_interp(0.12f, 0.0f, 0.35f, 1.0f, bank_f)   // [00CF6560]
+    // 0099CAD4-0099CB19; x0 is pilot+264h (0099CAF2). 0099D300 stores 0.0 there
+    // at 0099EB9D (XORPS 0099EB89) every think; the 0.12 ([00CE81A8]) stored at
+    // 0099EB95 goes to pilot+260h. The 0.12 below is the old reading, kept
+    // until the switch is paired (cc13-audit10 finding 3).
+    const float bank_lo = kBandShaperPlan264Bound ? u.plan_band_bank_264 : 0.12f;
+    const float g = pass == 0 ? avoid_interp(bank_lo, 0.0f, 0.35f, 1.0f, bank_f)   // [00CF6560]
                               : avoid_interp(1.3f, 1.0f, 1.7f, 0.0f, bank_f);   // [00CEB4B4] [00D1F3C4]
     if (1.0f > g) {
         lo = static_cast<float>((static_cast<double>(lo) - 1.1) * g + 1.1);     // double [00CE3DF0]
@@ -26092,6 +26112,17 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                         in.turn_radius_26c = 0.0f;
                         in.cruise_speed_25c = 1.0f;
                         in.alt_fold_a4 = 1.0f;
+                        // That comment is wrong for three of them: 009D1C51
+                        // [desc+25Ch] TurnRoll and 009D1C84 [desc+A4h] Width
+                        // shape plan+2C8h, the BANK limit, and 009D1962
+                        // [desc+268h] TurnCircleRadius sets the turn-room
+                        // escape. cc13-audit10 finding 5. desc+26Ch stays 0;
+                        // 009D1966 then picks +268h.
+                        if constexpr (kTorpedoAimClassInputsBound) {
+                            in.turn_radius_268 = unit_.plane_turn_circle_radius;
+                            in.cruise_speed_25c = unit_.plane_class_turn_roll;
+                            in.alt_fold_a4 = unit_.class_width_00a4;
+                        }
                         // CORRECTED, packet cc8_torpedo_release_timer: both of
                         // these were the stand-in 1.0f, and both are inputs to
                         // the aim tick's DIVE command.
@@ -26303,6 +26334,19 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                         // 009D20C4, easing to zero as the height above the
                         // floor closes. docs/TORPEDO_RELEASE_TIMER.md.
                         unit_.plan_state.pitch_target_2bc = r.commanded_altitude_2bc;
+                        if constexpr (kTorpedoAimPlanWritesBound) {
+                            // cc13-audit10 finding 4: the stores the aim tick
+                            // makes every think and this host dropped.
+                            unit_.plan_state.turn_scale_2e8 = r.roll_limit_2e8;              // 009D1D02
+                            unit_.plan_slots[bsp::kPilotSlotThrottle].desired = r.flag_278;  // 009D1F4A
+                            unit_.plan_slots[bsp::kPilotSlotThrottle].active = r.flag_27c;   // 009D1F55
+                            unit_.plan_slots[bsp::kPilotSlotAirBrake].desired = r.flag_2a8;  // 009D1F5C
+                            unit_.plan_slots[bsp::kPilotSlotAirBrake].active = r.flag_2ac;   // 009D1F64
+                            unit_.plane_air_brake_mode_2d8 = r.flag_2d8;                     // 009D1F6B
+                            if (r.fade_264_written) {
+                                unit_.plan_band_bank_264 = r.fade_264;                       // 009D1F36
+                            }
+                        }
                         unit_.torpedo_approach.aim_solution_130 = r.aim_solution_130;
                         if (r.aim_complete_2c && !unit_.torpedo_aim_complete_2c) {
                             unit_.torpedo_aim_complete_tick = unit_.torpedo_aim_ticks;
@@ -30076,6 +30120,11 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                                 // 009A17D0, before 0099D300.
                                 owner_.gunfire_avoidance_009a17d0(unit_, elapsed);
                             }
+                            if constexpr (kBandShaperPlan264Bound) {
+                                // 0099EB9D, at the end of 0099D300. Nothing
+                                // between here and there reads pilot+264h.
+                                unit_.plan_band_bank_264 = 0.0f;
+                            }
                             unit_.plane_think_dt = elapsed;   // 0099D300's argument
                             if constexpr (GameUnitsHost::Impl::kFighterLowTraceDiag) {
                                 if (unit_.dogfight_task_installed && !unit_.plane_death_c3a &&
diff --git a/src/torpedo_approach_update.cpp b/src/torpedo_approach_update.cpp
index ad89c0d70..ddc8bce1a 100644
--- a/src/torpedo_approach_update.cpp
+++ b/src/torpedo_approach_update.cpp
@@ -586,7 +586,10 @@ TorpedoAimCommand torpedo_aim_tick_009d15f0(const TorpedoAimTickInputs& in) noex
     // commanded speed and the ceiling at 00CE3814 and leaves the shaping out.
     cmd.commanded_throttle_2c8 = in.commanded_speed;
 
-    cmd.roll_limit_2e8 = 0.0f;          // 009D1D02, [00D06874]
+    // 009D1CFA MOVSS XMM0,[00D06874] = 1.4f, stored unconditionally at
+    // 009D1D02. The 0.0f this used to write contradicted its own citation.
+    // This function has no caller: the host runs torpedo_aim_tick_full_009d15f0.
+    cmd.roll_limit_2e8 = 1.4f;          // 009D1D02, [00D06874]
     cmd.flag_278 = 1.0f;                // 009D1F4A, [00D7A24C]
     cmd.flag_27c = 1;                   // 009D1F55
     cmd.flag_2a8 = 0.0f;                // 009D1F5C
```

## Handoff

- **The open question is the roll arm, not the torpedo task.** Why does a TBD in `aim` keep 0.19-0.33 rad of bank on
  a 0.02-0.05 rad heading error? Check the steady bank against the bearing rate of `009D3586`'s per-tick lead point
  (a pursuit curve). If the rates match, the hold is image behaviour for the geometry. If they do not, read the
  roll arm `0099E2BA`/`0099D0A0` against the listing.
- Pair `kBandShaperPlan264Bound` and `kTorpedoAimPlanWritesBound` separately. The first should make banked holds
  stricter. The second changes the TBD's bank authority (1.4 x).
- The two precision notes under finding 1 are recorded only.
