# CC13_VERIFY11: labelled substitutions in game_hosts_units.cpp lines 1-20000

This is a verify-only pass for the cc9 gameplay lead, done by worker cc13-verify11 on 2026-10-17 against
base 56f4b923a. It covers `src/game_hosts_units.cpp` lines 1-20000 (cc13-verify12 takes 20001 to the
end). The subjects in that range are the switch block, air operations (deck, elevator, takeoff, touchdown,
flight-state transitions), squadron bookkeeping (retreat, spent ordnance, kill), ship hydro and
submarine bindings, unit-instance bindings and pilot avoidance. No code was changed and no ledger
records were added. Line numbers are against the base. "GHU" is `src/game_hosts_units.cpp`. "Image"
means battlestationspacific.exe, read with `bsp.py disasm-raw`, `bsp.py ghidra bytes` and
`tools/pe_const_read.py`. Script references are to this installation's `scripts/` tree, which is
modded.

Excluded, as briefed: `squadron_free_attack` wiring and the dying-spin / stall / rack switches of
cc9-physics14/15. Items already in CC13_VERIFY1..10 and CC13_VER_* are not repeated, except where this
pass resolves one (C1 of VERIFY8, below).

Rubric (cc13 common brief):
- MATCH.
- MISMATCH MINOR: edge guards, error paths, NaN-only arms, width with no effect, presentation, or RNG
  draw count only.
- MISMATCH REAL: a wrong result, or a missing or extra call, write or side effect on a normal path.
- STALE LABEL: the comment says something is missing or substituted, but a bound arm or another host
  already does what the image does.
- UNRESOLVED.

## Census

A keyword sweep over lines 1-20000
(`substitut|approximat|stub|simplif|fixed value|pins? (a|the) draw|no producer|not in the struct|host
constant|record-only|unbound|not bound|placeholder|stand-in|stands in|unimplemented(|record(|TODO|not
modelled|not yet|omit|skips the|host keeps|host uses|assum`) matched 321 lines. Each hit was tagged
with the image addresses it names that already appear in a CC13_VER* doc. The OFF switches
(`static constexpr bool k... = false`) in the range are `kPlaneFormationPlacementEnabled`,
`kDiveBombMoveToOriginBound`, `kDiveHitClockBound`, `kLandAbortGroundArmBound`,
`kRoleScreenFixedStepCall`, `kRackUnequippedEmptyBound` and four trace switches. VERIFY8 judged all
but `kRoleScreenFixedStepCall`, which is deliberately OFF (it must be the opposite of
`bsp::kHudRoleScreenPumpBound`). A further sweep for "OFF until" premises found the one at `:6042`.

About 60 of the 321 hits were read against their code and, where the claim is per-tick or
script-visible, against the image. Most of the rest are notes on how a body was read, live-arm
records that perform the write and only log it (for example `:17941-17953`), or presentation
records. "Not covered" at the end lists what was not read.

## Summary: MISMATCH REAL (each one sent to team-lead when confirmed)

| # | Host | Image | Live in a normal mission? | RNG |
|---|---|---|---|---|
| R1 | GHU `:6043` `kDiveHitClockBound = false` (URGENT, goaway). Resolves VERIFY8 C1 | 009C4AA4-009C4AC6: a hit less than 1 s ago (approach+C4h < 1.0f) and elapsed +28h > +2Ch + 6.0 store -1.0f to +24h, which forces the goaway re-roll | any dive bomber hit by AA during its goaway more than 6 s past the bank window (USN01, USN04 Vals) | the forced re-roll leads to the image's stream-1 draws, which the host pins anyway (VERIFY8 R1) |
| R2 | GHU `:6243-6244` label, feed at `:28600-28601`: the torpedo done/disarmed arm measures from the target origin (URGENT, torpedo) | 009D29F8-009D2A35 measure from approach->vtable[0] = 009D0670 = approach+D0h, the 009FADA0 point (hull point plus lead) written at 009D34BB every arm tick | every torpedo plane's post-release done ticks while its squadron's first member is below 300 m (USN04 torpedo rows) | none |
| R3 | GHU `:16418-16427`, `:16471-16477`: 009F7C90's moveto re-issue to a command building is not modelled | 009F7D17-009F7D75 issue moveto 00E08F68 on the command's object when it IsKindOf(1Ch) (MCommandBuilding), the command type is 1 or 2 and the queue holds one command | only where a squadron's command ends on a command-building target; which test rows do was not established | none |

### R1 kDiveHitClockBound is OFF, but its premise is gone

`kDiveHitClockBound` (`:6043`) is OFF "until paired with the hit notice's caller" (`:6042`). The caller
has been ON since f262e60a8 (2026-09-29): `kPlaneHitTaskNotifyBound = true`
(`game_hosts_gunnery.cpp:1336`) sends every plane hit to
`GameUnitsHost::plane_hit_task_notify_00999aa0` (`game_hosts_gunnery.cpp:3494`). For a dive bomber with
the task installed that zeroes `db_approach_clock_c4` (`:17944-17948`), and `:4329-4330` advances it by
dt. The goaway timer feed still passes the floor: `tin.approach_clock_c4 = kDiveHitClockBound ?
unit_.db_approach_clock_c4 : 3600.0f` (`:27071-27072`).

Image, 009C4A6D:
- 009C4A91 loads 1.0f (00D7A24C). 009C4AA4 `COMISS xmm0, [ECX+C4h]` and 009C4AAB `JBE` skip unless
  1.0 > approach+C4h (an unordered clock skips).
- 009C4AAD-009C4ABC: elapsed +28h is compared with +2Ch + 6.0 (00CE6628, a double). Above it,
  009C4ABE-009C4AC6 store -1.0f (00D7A260) to +24h.

- **MISMATCH REAL.** In the image a hit in the last second ends the goaway's bank window early. The
  host never does.
- **LIVE:** every dive-bomb goaway under AA. The hit frequency in goaway was not measured.
- **RNG:** none extra on the host side. The image's forced re-roll takes the 009C4D13 / 009C4D33
  stream-1 draws, which the host pins (VERIFY8 R1).
- Fix: flip the switch and pair it.

### R2 The torpedo disarmed arm measures from the target origin

`kTorpedoDoneDisarmedArmBound` is ON (`:6247`). Its label (`:6243-6244`) says "the aim point
approach->vtable[0] is taken as the target's position (substitution)". The feed sets
`din.aim_x/z = tgt->motion.position` (`:28600-28601`, in verify12's range; only the label is in this
range).

Image:
- 009D29F8-009D2A04 call approach->vtable[0]. That is 009D0670, which copies approach+D0h..D8h
  (bytes read).
- approach+D0h is sub+1Ch of the 009FADA0 sub-object at approach+B4h, written by 009D34BB
  `CALL 009FADA0` (ECX = approach+B4h): the hull point plus target->vtable[48h](projtime) minus the
  origin.
- 009D2A06-009D2A52: ratio = |aim.xz - leader.xz| (00414C60) / (record+8h x approach+24h), then the close
  test, the bank cap cmd+2C8h and +8Ch, as in `include/bsp/torpedo_task_arm.hpp:298-313`.

The premise is STALE. The host already computes that point for the goaway
(`kTorpedoGoAwayAimPointBound`, `:2366-2373`, through `aim_point_009fada0`).

- **MISMATCH REAL.** The ratio is taken against a point that is off by the hull offset (up to 0.45 x
  Length) plus the lead (target speed x projtime). That moves the close test and the bank cap.
- **LIVE:** every torpedo plane's done ticks after release while the squadron's first member is below
  300 m.
- **RNG:** none.
- Fix: feed the arm from `aim_point_009fada0`, as the goaway does.

### R3 009F7C90's moveto re-issue to a command building

The labels at `:16424` and `:16477` say "the kind-1Ch moveto re-issue is not modelled". Image
009F7D01-009F7D75:
- +3Dh set and 007EE720 true (009F7D01-009F7D15).
- d = 00521EA0([sq+4]+58h), the command's resolved object. d->vtable[5Ch](1Ch), so the object is an
  MCommandBuilding (`src/entity_class_ids.cpp:40`) (009F7D17-009F7D35).
- The current command's vt[0Ch] is 1 or 2 (009F7D37-009F7D50), and 0071BE60 == 1 (009F7D52-009F7D5D).
- Then 0071ECF0(00E08F68, 004684F0(d)), a moveto with the building as the object target
  (009F7D5F-009F7D75), before 0071E430(current, 1) at 009F7D7B.

Host: the command ends, and the members' rows are retired with the target cleared.

- **MISMATCH REAL** on that path: the squadron gets no follow-up moveto over the building.
- **LIVE:** a squadron striking a command building (base capture, airfield HQ). No log census was
  made. JM08 has HQ buildings, so it is the first row to check.
- **RNG:** none.

## Other findings

| # | Host | Class | Evidence |
|---|---|---|---|
| S1 | GHU `:170-174` `kTorpedoReferenceSpeedAuthored`, "the PilotBot tuning block is not reachable from this host" | STALE LABEL, same value | The plane globals are loaded (`load_plane_globals_007e2a20`, `:12114`). `:41424-41425` already reads `plane_globals().pilot_torpedo_reference_speed`. The installed planeglobals.lua authors KMH(300), which is 83.333336f, the constant. The constant is still used at `:28360` and `:28609` (verify12's range). No behaviour change while the script keeps KMH(300). |
| S2 | GHU `:3614-3617` `kPilotDiveBombReferenceSpeed`, "Logged, not yet bound" | STALE LABEL, same value | Same loader: tuning +4D8h. 280.0f / 3.6f rounds to the same float as KMH(280) from Lua. |
| S3 | GHU `:17803-17805` "the dive it leads to (the hull move to bands[level]) is not modelled" | STALE LABEL | `kSubmarineDiveBound` ON (`game_hosts_units.hpp:387`): `run_submarine_dive_00936dc0` (`:10310-10336`) reads `row.submarine_depth_level`. The unposted A2h message is the echo of the boat's own post (00852C7D applies `SetDepthLevel(payload)`; docs/SUBMARINE_MODEL.md section 16), so not posting it is a MATCH. |
| S4 | GHU `:10305-10307` "needAir (+1281h) ... read false; there are no seabed samples, so the clearance never clamps" | STALE LABEL | `kSubmarineAirBound` and `kSubmarineSeabedBound` are ON (`game_hosts_units.hpp:394`, `:411`). `:10326` reads `sub_air.need_air`, and `:10332-10336` clamps on `sub_scan.clearance`. The kamikaze and 008522C0 parts stay as VERIFY4 R3 described. |
| S5 | GHU `:11365-11369` "This host has no ground model ... branch A returns at its first test, before any band or +25Ch write" | STALE LABEL | `kAvoidTerrainWaterProbeBound` is ON (`:6621`). `terrain_avoidance_water_009a1420` (`:11279-11359`) probes through `world_segment_blocked_00903bc0` and inserts bands and +25Ch. |
| S6 | GHU `:17287-17292` "This is a STAND-IN ... leaves the writer to packet ship_buoyancy_element_producer" | STALE LABEL | `kShipBuoyancyElementsBound` is ON (`:6907`). The image list from 0082D040 is used. The stand-in is only the fallback for a class without the Aux nodes. |
| S7 | GHU `:3540-3550` and `:5413-5420`, "the task's release slot ... UNREAD, so the spawn is not reproduced from its own site" | STALE LABEL (describes the OFF arm) | `kReleaseIssueStageBound` and `kReleaseIssueStageValsBound` are ON (`:6521`, `:6535`). The live arm defers to the rack tick. |
| S8 | GHU `:6835-6839`, `:6886-6891`, `:6916-6919`: KillDepth -200, MaxLeakPercent 0.02, EnnyiVizEsKeszPercent 0.2, DologSzorzo 2 | MATCH | Equal to this installation's shipglobals.lua `:377`, `:381`, `:383`, `:388` (2024-07-13). The premise holds: `load_gameplay_settings_0083b5e0` (`:12106`) loads only the rudder, AutoThrust and sound records, not VizbeomlesDolgok. `src/gameplay_settings.cpp` has a loader that the units host does not call. |
| S9 | `src/unit_instance.cpp:149-157` with GHU `:9712-9721`: the bubble timer, gated on "the ocean object's own world Y", which the host answers 0.0 | MISMATCH MINOR, wrong premise | 0082568F-0082571D: the gate is unit+5Dh, then `[[00E188A8]+19FCh]+124h < 0`. game+19FCh is the CAMERA node (docs/CC13_GUI_SCREENS_3.md:238, docs/CC13_PKT642.md:132), not the ocean. With the camera under the sea, a +5Dh unit draws U(settings+64Ch, +650h) from 00BD2F10 with ECX = 1 (0082571D) and spawns the bubble effect. The host has no underwater camera, so the answer is right for the harness. The label names the wrong object. A run with an underwater camera would take one stream-1 draw per expiry that the host never takes. |
| S10 | GHU `:8762-8766` retreat leave-map PARTIAL: the survivors keep their retreat task | MISMATCH MINOR | 007F32CB-007F340F: with survivors, +361h clear and 007EE2E0(0) true, the image sets fire stance 0 on sq+348h (0071BE80(0)), sq+370h = 0, posts a message, and orders moveto 00E08F68 to c + 1.1 x (p - c) in xz at the plane's y. Here c is the map box centre (004B4850) and p is the leaving plane's position. The x87 order is read from bytes `DE EA` at 007F3370 (FSUBP ST(2), ST(0) = p - c). The point lies beyond the exit point, so both sides leave the map. The differences are the route, and squadron fire stance 0 (fire predicate 0084D910 is false for stance 0, so the survivors hold fire in the image). |
| S11 | GHU `:11421-11423` pitch for 0099F1C0: atan2 when \|v\| > 1, else `plane_pitch_angle_c64` | MISMATCH MINOR | The image reads unit+C7Ch, written by 007C18B0 only when \|v\| > 0.1 (double 00D7A3A0, 007C1C4B-007C1C59) and otherwise kept. It differs only below 1 m/s. |
| S12 | GHU `:11376-11386` (0099F1F3-0099F20D) | MISMATCH MINOR | The image returns when sq+34Ch, the avoid-zone layer, is null (0099F20D `JE 009A17A1`). The host continues with a 100 m cell when `reg.layer()` is null. It differs only in a scene with no `.nav` layer. |
| S13 | GHU `:10710-10720` `kShipPostMotionRepairOrder`: 008160D7's 0093CA20 is counted | MISMATCH MINOR (order) | The repair tick runs in the gunnery host's `run_damage_control` after the projectile pass, not in the motion tail. This is labelled. |
| S14 | GHU `:6281-6282` "no rack here answers 31h" (the paratrooper point 007BCCF0 -> 006E3F90) | MISMATCH MINOR, same class as VERIFY8 L6 | 31h is MParatrooper (`include/bsp/ordnance_kinds.hpp:23`). This installation has C-47 Paratrooper classes, and JM05, JM11 and JM13 (with their PRCP copies) script paratroopers. None of the five test rows does. |
| S15 | GHU `:14799-14833` ground retakeoff: `007C17D0` recorded on the takeoff/Takeoff arm | UNRESOLVED | The label in docs/SQUADRON_LAND_TASK.md 5bp ("does nothing unless +904h is set, which it is not for a launched plane") is about the launch. A retakeoff can carry +904h: the touchdown sets it with state 4. In single player 007C17D0 posts C3h(+904h, 4). The 007CCFA0 arm 007CD02C sends state 4 or 5 to 007C7430 (5 -> 4 only), and any other state not 7 to 007CA3F0(0). So landed in state 4: no-op. Landed in state 6: 007CA3F0, which needs a holder (+BF4h) and otherwise takes its explosion arm. Whether a landed state-6 retakeoff happens (`retakeoff_state6`) was not checked in logs. |
| S16 | GHU `:6835-6919`, `:7966-7987`, `:9937-9948`, `:12121-12164`, `:17923-17957` | MATCH | DummyAIEnabled default 3 (no SP scene authors it, docs/TERRAIN_ARM_US_FIGHTERS.md). The unit-order message is posted only in session mode 2 (00816A8D). Hit-notice records perform their writes. The settings loader records are paths taken only when a script is missing. |
| S17 | GHU `:3111-3118` 00815E20 +FA0h/+FA8h not carried | MISMATCH MINOR (presentation) | 00815E20-00815ECF writes the new leader's +FA0h/+FA4h/+FA8h from the old leader's wake offsets. The readers are the wake ring and 00811820. No formation-steering reader was found in GHU. |
| S18 | GHU `:13881-13891` 007C16F0's 0090F6C0 arm | MISMATCH MINOR | 0090F6C0 is BSP_MissionScoring_PlaneAchievement (statistics), after the +5Ch/+5Dh checks. The state, +C04h, +904h and +C18h writes match. The `!= 0.0f` test agrees with LAHF/TEST AH,44h/JNP, which also takes the arm for NaN. |

## Not covered

- Read only for labels, with no image check: `:1-160`, `:500-2200`, `:4400-5390`, `:5560-6000`,
  `:7700-7960`, `:8000-8700`, `:8830-9440`, `:12180-13550`, `:13600-13880`, `:14360-14700`,
  `:14850-16000`, `:16480-17160`, `:18100-20000` (the ship terrain contact, exit-zone role 0081BEB0,
  which lies in the struck 0081B000-0081EFFF range, and the plane water-contact / depth-kill block
  around 007CB7F0 and 007CE3A7, which is frozen for cc9-physics13).
- The per-tick arithmetic spot checks were limited to 0099F1C0's head (look_t, sa, v_look: MATCH),
  007C16F0, 008528B0 and 009D29E0's head.
