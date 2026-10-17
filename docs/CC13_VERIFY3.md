# CC13_VERIFY3: gunnery-host labelled substitutions

Verify-only pass for the cc9 gameplay lead (worker cc13-verify3, 2026-10-17, base 868bb503c).
No code was changed and no ledger records were added. Every line number below is against
that base. "Image" is battlestationspacific.exe as read with `bsp.py show`, `ghidra decompile`,
`disasm-raw` on disk bytes, `scan-bytes` and constants read with `ghidra bytes`.

Scope: the labelled substitutions in `src/game_hosts_gunnery.cpp` (comments that say the host
substitutes, approximates, stubs, simplifies, takes a fixed value, "stand-in", "not modelled",
"record only", LABELLED). The recon pass is EXCLUDED (cc9-gunnery51 is rebuilding it): the
labels at 351-356, 1094-1108 and 7205-7213 were not checked. Priority went to the per-shot fire,
reload, dispersion, aim and damage paths of a normal mission, then torpedo, submarine and depth
charge.

Rubric (cc13 common brief): MATCH; MISMATCH MINOR (edge guards, error paths, width with no
effect, synchronisation, ordering with no observable effect); MISMATCH REAL (wrong result,
missing or extra call, write or side effect on a normal path); STALE LABEL (the code already does
what the label says it does not); UNRESOLVED. "RNG" names draws on the shared generator 00BD2F10
(ECX = stream). The host's stream 1 is itself an LCG stand-in (`random_range_00bd2f10`, 2189), so
draw VALUES are never the image's; what can match is the bounds, the count and the order. A
missing or extra stream-1 draw on a normal path is graded REAL here because it changes which
value every later stream-1 consumer (pilot bots, plane tasks, gun bots) receives; the
distributions themselves are unchanged.

Precision: the gun fire chain (00730160) and the gun-bot ticks run inside the fixed-step unit
job, which runs at 24 or 53 bits depending on the claiming thread (docs/CC13_FPU_PRECISION.md).
Where a host expression differs from the image only by where it rounds, the item notes it; none of
those changes a branch.

## Census

57 hits for substitute/approximate/stub/simplify/deterministic/"not in the struct"/host
constant/record-only/placeholder/"in place of", plus about 40 for "stand-in"/"not modelled".
Grouped by image function:

| Group | Host lines | Image |
|---|---|---|
| Gun fire, reload, dispersion | 361-371, 1062-1073, 3767-3770, 4106-4166, 8945-8955, 9040-9100 | 00730160, 0072D520, 0073031D, 00730540, 007304A0 |
| Ship-artillery aim point | 1015-1028, 2598-2648 | 006DF520, 00816650 |
| AA aim and error | 1276-1304, 6472-6477, 7953-7990, 8016-8022, 8140-8205, 8240 | 00901C20, 00902920, 008FFBC6, 008FF040, 008FF200 |
| Torpedo bot, spread, swim | 330-347, 481-490, 578-590, 1029-1038, 1081-1091, 1184-1194, 2327-2350, 9068 | 008FFF20, 00901BA0, 00951FC0, 00856050, 00857480 |
| Depth charge | 8700-8770, 9972-10030 | 008FC080, 006FCD20 |
| Flak round | 10545-10600 | 0070C3AA, 0070C41C |
| Shot team, damage, death | 3940-3960, 11358-11372, 11500-11523, 12945-12955, 1471-1482 | 0072C0F2, 00470350, 00819A20, 00959450, 00827AB0 |
| Drops (torpedo, bomb) | 15305-15312, 15442-15448, 15618-15628 | 006E4D50 |
| RNG plumbing | 2196-2240, 14906-14913, 17004 | 00BD2F10, 00BD2FD0 |
| AI weight rows | 14588-14624 | 00A08460 |
| Mesh hit tests, wreck, misc | 896-955, 1385-1430, 1520-1531, 1664-1677, 1776-1785 | 0087BCC0, 009239A0, 0077CE60 |

## REAL findings (each sent to team-lead when confirmed)

### R1. Per-shot reload draw missing on stream 1 (every gun, every mission)

- Host: `game_hosts_gunnery.cpp:8945-8955` sets the fired barrel's timer to the scalar
  `gun.reload_time` (or `mode1->reload_time`, or `torpedo_stock_rearm_0072d520`'s value) with
  no draw. The label at 3767-3770 says "the draw's stream advance is not modelled, as on the fire
  path". Same at the SetTorpedoStock re-arm, 3799.
- Image: 00730160 BSP_Gun_Fire calls 0072D520(barrel = gun+44Ch, full = 1) at 007309E8. In
  0072D520, when the owner's vt[1F4h](class) answers a round (0072D538) and vt[1F8h] consumes it,
  the full arm (0072D555 CMP / JE) draws 00BD2F10(ECX = 1, [[gun+3F8h]+28h], [+2Ch]) at 0072D579
  and 0072CF00 stores it. The empty arm 0072D592 stores [00CFDBF8] with no draw (host matches).
  The re-arm at 0081F95A (BSP_UnitInstance_SetTorpedoStock) is the same call.
- Behaviour: the value is identical (every ReloadTime in this installation is a scalar, so
  +28h == +2Ch; docs/GUN_SHOT_CADENCE.md row 10 judged the value only). The image advances
  stream 1 once per shot; the host does not.
- LIVE: yes, every shot with a round available (Ammo 9999 on every artillery and AA row; torpedo
  ships through the 00810D80 spare provider).
- RNG: image 1 stream-1 draw per shot (after the two throw draws 00730557 / 00730575 of the same
  shot); host none.

### R2. Ship-artillery aim point redrawn on a live-to-live retarget

- Host: `game_hosts_gunnery.cpp:2601` (`artillery_aim_point`): `if (st.target != target) {
  st.target = target; st.timer_b4 = -1.0f; }` redraws the body point the tick the target changes.
- Image: 006DF520 is the only writer of bot+B4h found (decompiled exports of every 006D/006E
  function, both `param_1[0x2d] =` and `+ 0xb4) =` forms). It sets -1.0 (00D7A260) only in the
  arm where vt[44h] returns no target (the arm that also zeroes bot+90h). The setter 006DF170
  calls vt[30h] on a retarget; for this bot (vtable 00CFE000) that is 0072BD50
  BSP_GunBot_ClearFireTarget, which clears the bot+38h record only. On a retarget between two live
  ships the timer keeps running (006DF6D7 FSUB dt, 006DF6EF..006DF6F5 FLDZ/FCOMI/JBE: refresh only
  when 0 > timer, ordered) and the stored body-frame point, drawn from the OLD target's box or
  sections, is carried to the new target's matrix for up to TargetPointRefreshTime (5 s).
- LIVE: yes whenever an artillery gun switches between two live ship targets without a
  target-less tick between them (frequency not measured).
- RNG: the host makes 1 (section roll, when the row's chance > 0) plus 1 (section pick) or 3
  (box) aim_point draws per retarget that the image does not make.
- Also MINOR: the host refreshes at `timer_b4 <= 0`, the image at `timer < 0`.

### R3. AA gunner against a SHIP target: the hull-box draw is missing

- Host: `game_hosts_gunnery.cpp:8142-8185` (kAaGunnerSwingErrorBound refresh). Label 1289-1291:
  "the target point is the host's aim point for the target, not vtable[100h]'s hull-box draw".
- Image: 00902920, at each timer refresh, when the skill row's +0Ch (AngleDiffErrorRatio) is
  above 0 (00902B74..00902B84 COMISS/JBE), the indirect CALL at 00902BFD reaches target->vtable[100h] with (out, chance
  -1.0 (00D7A260), weights 1, 1, 1, box {0.8 (00CE74F8), 0.5 (00CE3800), 0.8}). For a ship that
  is 00816650: the chance is not positive, so no roll, but three box draws on stream 1, before
  the U(0, ConstAngleError) draw at 00902CF0. The spread e is then measured from that point.
- Host: no box draws; e is measured from `at`.
- LIVE: only when a category-1 AA gun on a ship engages a ship (small craft that pass 008FBE00's
  armour test). The host reaches it: the ship arm `e /= +750h/+754h` at 8178.
- RNG: per refresh against a ship the image draws U(3, 8), 3 box draws, U(0, c), the normals; the
  host U(3, 8), U(0, c), the normals.
- For PLANE targets vtable[100h] is 0042D810 (zero offset, no draw): MINOR only (the host's `at`
  is the Height-raised point, the image the origin).

## Fire, reload and dispersion

| # | Host line | Label / behaviour | Image evidence | Verdict | LIVE | RNG |
|---|---|---|---|---|---|---|
| F1 | 8945-8955, 3767-3770 | reload draw not modelled | R1 | **REAL** | yes | R1 |
| F2 | 4127-4166 | throw magnitude dispatch | 0073031D..00730498: plane and [gun+3F4h]+80h == 0 -> role 1, unit+DF4h; else by sub-type [[gun+3F8h]+34h]+8h: 0Ah -> gun+39Ch, role 5; 0Bh -> role 7, +3A4h else +3A0h; 10h -> role 3, +394h; 4..7 -> role 4, +398h; else +390h with roles 2 and 3; then FMUL the bot's vt[24h] at 00730498 | MATCH on the gates and their order. The host keys the bot by category where the image tests the slot pointer; for sub-type 0Bh the host's category 9 -> ArtillerySubDirector row and category 8 -> DepthChargeBot (+3A0h, per 0072C6A0) were taken as given, the bot at +3A4h was not read | yes | none |
| F3 | 1068-1069, 9083-9084 | unit+63Ch = 1.0 | 007305D6 multiplies the radius by unit+63Ch for sub-types 4..7 only. 0095DC40 stores 1.0 at 0095DD79 unless role 4 is the LOCAL player's (00927F30); then +63Ch decays by settings+40h per second to 0 once no fire input (action 153) has come for settings+30h + +34h seconds, and climbs back at +38h/+3Ch | label consistent. MINOR to REAL for the player's own artillery: a player-held role-4 ship that fires without input would fire with a narrower (to zero) cone in the image | UNRESOLVED: only if a locally held artillery seat fires while idle in a harness run | none |
| F4 | 1070, 9084 | 00470440(7) = 1.0 | 00470440 is FLD1 unless EnablePowerups (00E0C978) and the kind-7 list is non-empty; no AddPowerup call in the logged missions (docs/GAMEPLAY_MODIFIERS.md). Note: 00730622..0073064A multiply BOTH the angle +400h and the radius +404h by it; the host comment names only the radius | MATCH while no power-up is active | power-up missions only | none |
| F5 | 9080-9100 | the cone draw | 00730540: U(0, [00CE3D9C] = 0x40C90FDB = 6.28318548f), then U(0, 1) at 00730575; radius = tan(m) (00412E20) * U; 00730654..0073075E add r cos(t) row A + r sin(t) row B, no renormalise | MATCH (axes: label consistent, the draw is rotation-uniform). Precision: the host forms `tan(m) * u` in double and rounds once; at 24-bit PC the image rounds tan(m) first: at most 1 ulp | yes | 2 stream-1 draws per shot both sides |
| F6 | 9068-9078 | torpedo fan about the hull up | 007304A0..0073051A: fewer than 2 tubes -> 0; else m / (n - 1) * gun+44Ch - m * 0.5 (qword 00D7A280 = 0.5), then 0085C3F0 rotates the shot frame about the row at [ESP+58h] (the barrel frame's row) | MATCH on the offset (`gun_torpedo_fan_offset_007304a0`); the axis is MINOR (hull up vs the tube frame's row; tubes are level) | yes | none |
| F7 | 361-371 | flak mode chosen per shot from the target | 008FF200 (AAFlakBot SetFireTarget) calls 0072D5B0(1, 0) for weapon kind 6 at 008FF240; 006DF4C0 (ArtilleryGunnerBot SetFireTarget) calls 0072D5B0(0, 0) at 006DF4EC; both only for slot 8 or an AI-held slot | MINOR: the mode follows the last SetFireTarget, which the target kind decides; differs only for a player-held seat | rare | none |
| F8 | 3940-3960 | shot team: no +914h squadron-head replacement | 0072C0F2..0072C14B | label consistent; MINOR (a wingman and its head carry the same role-1 slot in single player; +914h's meaning not read) | yes | none |
| F9 | 11371 | `weapon_scale = 1.0f`, "006E7C90, gun+43Ch, default 1.0f" | gun+43Ch is an INT count, not a float scale. 0072C092..0072C0AE in BSP_Gun_CreateProjectile: when [gun+43Ch] != 1, shot->vt[20h]((float)count). Writers: the constructor (0072E666) and 00727E10 BSP_Gun_SetPair43C (+43Ch = group size, +440h = index), called only from 009575D0 (reached from BSP_Plane_ReadPropertyBag at 007D5DA5) for the plane's gun groups (+538h, list at +D8h). 006E7C90 itself reads ECX+240h | UNRESOLVED: a plane gun in a group of N may fire rounds whose damage is scaled by N in the image; whether only index 0 fires (00729D30 RebuildBarrelBuffers reads +43Ch and +440h) was not read | plane guns | none |

## Aim

| # | Host line | Label / behaviour | Image evidence | Verdict | LIVE | RNG |
|---|---|---|---|---|---|---|
| A1 | 1015-1027, 2598-2648 | target change re-draws at once | R2 | **REAL** | yes | R2 |
| A2 | 1023-1026 | "section points not loaded"; "bot+90h step not modelled" | `kShipSectionPointsBound` (1363) loads them; `ranging_offset_step` (2649) steps bot+90h at dt * 30 (00CE7630) | STALE LABEL | - | - |
| A3 | 7953-7956 | lead = V * distance / V0 | only the `aa_v0 < 2` fallback (8011-8014); 00901C20 aims straight at the target below 2.0 (00CE3958) | STALE for the bound path; the fallback is MINOR (no gun has V0 < 2) | no | none |
| A4 | 7987-7989 | 0085E4D0 turn average "not carried" | `aa_turn_average_00901c20` (3212) runs it behind `kAaTargetTurnAverageBound` (695, ON), called at 7983 | STALE LABEL | - | - |
| A5 | 1289-1291, 8142-8185 | AA swing error target point | R3 | **REAL** for ship targets, MINOR for planes | ship targets | R3 |
| A6 | 8016-8022 | tail gunner: no 00816650 draw on a ship target | 008FFBC6..008FFC03 call target->vt[100h] EVERY tick with the TailGunnerBot row's values | label consistent; REAL if a plane's tail gun ever takes a ship target (not established) | UNRESOLVED | image up to 4 stream-1 draws per tick against a ship |
| A7 | 6472-6477 | no per-gun node frame in the AA window | 0085A9A0 | superseded in part by `kAaKind6GateMountFrameBound` (197) and `kMuzzleOffsetsBound`; the remaining hull-frame use was not re-derived | yes | none |
| A8 | 8240-8245 | flak attach roll at the first flak tick | 008FDBE0 has three callers: 008FF055 in 008FF040 BSP_GunBot_AttachBallisticClass (vtable 00D18220 +1Ch, at attach), 008FF247 in SetFireTarget, 0090331B after a shot. The host's assign path (6701-6713) sets `fe.attached = true` and rolls, so the separate attach roll is never made once an assign precedes the first flak tick | MINOR: the image's attach roll happens at load; its seven draws only offset the start of stream 1 | at load | image 7 draws per flak bot at attach; host none |
| A9 | 1184-1194 | torpedo think timer and range gates not modelled | `kTorpedoBotThinkTimerBound` (503) and `kTorpedoBotAttackRangeBound` (492) model both | STALE LABEL | - | - |

## Torpedo, submarine and depth charge

| # | Host line | Label / behaviour | Image evidence | Verdict | LIVE | RNG |
|---|---|---|---|---|---|---|
| T1 | 2327-2350 | TorpedoBot accuracy rows from robots.lua | 00901BA0: row = 00E1998C + bot+34h * 14h; +14h FireTargetAccuracy when the arg is set, else +18h; then 008387B0(7, target, max range, accuracy). This installation's robots.lua TorpedoBot (2025-06-01) by skill 0 Stun .. 5 Elite: 0.03/0.035, 0.35/0.45, 0.055/0.065, 0.04/0.05, 0.045/0.05, 0.055/0.065 | MATCH (same values and index order as `kBulletThrowMul` row TorpedoBot 1, 1, 0, 1, 0.5, 0 and `kTorpedoAngleErr`) | yes | none |
| T2 | 1033-1035 | the gate's lead point lacks the +6D4h spread | the spread is added at 7920-7926 before gate B, and `torpedo_lead_xz` is captured from that lead at 8124 | STALE LABEL | - | - |
| T3 | 9007-9013 | spread flip | 00951FC0: JB on value < 0 -> 00D7A208 - v; else -v - qword 35.0 (00CF8608); then a result above 80.0 (00D19BE8, COMISS/JBE) -> 0 (the 00952017..00952039 abs/UCOMISS idiom stores 0 exactly when the compare was true) | MATCH | yes | none |
| T4 | 342-346, 10038-10066 | swimdepth -2.5 for every launch | 00856050 stores [00D0C314] = 0xC0200000 = -2.5 at +47Ch; 0072C970's 00852410 overwrite (sub at sub) not modelled, as labelled | MATCH on the constant; label consistent | sub-vs-sub only | - |
| T5 | 10062-10063 | the scan phase draw at water entry | 00856050 draws -U(0, 1) on stream 1 (00856132) in the CONSTRUCTOR, i.e. at launch; the host draws in `torpedo_swim3d_start` at water entry | MINOR: same draw, later in the stream; one draw fewer for a round that never reaches the water | yes | 1 stream-1 draw per torpedo, order differs |
| T6 | 1081-1091 | gyro turn only while swimming; 00852410 not modelled | not re-derived | label consistent | yes | - |
| T7 | 578-590 | the surface-plane vertical | superseded by `kTorpedoSwimDepthHomingBound` (348, ON; 10232) | STALE LABEL | - | - |
| T8 | 8700-8770 | DepthChargeBot: the target check and idle timer are the host's own | 008FC080: the resolve/vt[38h](0) clear, 008FBCE0 idle timer, the 00927F10 gate, the 0.1 s think (+5Ch), the sink time ((owner y - target y - [00CF3F20]) / desc+DCh) | label consistent. MINOR: `weapon_sink_rate` (8732-8736) returns 1.0 for DiveSpeed <= 0; the image divides by desc+DCh unguarded (inert if every depth charge authors DiveSpeed > 0) | yes (anti-sub) | none |
| T9 | 9972-10030 | the submarine list is the host's live MSubmarine units | 006FCDDD walks [[record+30h]+7Ch]; 008530E0 (vtable +134h, BSP_SubmarineUnit_DetachFromWorldLists) removes a sub from it at detach, not at death | MINOR: a wreck still on the list until detach can take a contact detonation in the image | rare | none |

## Damage, death and drops

| # | Host line | Label / behaviour | Image evidence | Verdict | LIVE | RNG |
|---|---|---|---|---|---|---|
| D1 | 1471-1482, 13052-13075 | WreckChance 0.3 from shipglobals.lua | 00827B33..00827B5D: U(0, 1) on stream 1 at 00827B4D, then FCOMI [settings+65Ch] vs U, JBE skips when WreckChance <= U. shipglobals.lua:496 `WreckChance = 0.3` (file 2024-07-13, untouched bulk) | MATCH | yes | 1 draw per ship death past the mass test, both sides |
| D2 | 11500-11523 | kamikaze direct arm 00915F20 recorded only | 00915F20 is the award/statistics record (string refs BW_DCM, BW_TM, BW_AM, BW_AAM, RUA_KU, RUA_LBU; callers 0077CE60 and 00819A20) | MINOR: scoring only, no damage | kamikaze missions | none |
| D3 | 12945-12955 | unit+70h = 1, world+4ACh = 1 | not re-derived | label consistent | yes | - |
| D4 | 15305-15312, 15442-15448 | drop from the plane origin at forward * speed | 006E4D50 (cc13_bombdrop.cpp): the round gets the rack's offset ([[rack+3F4h]+DCh] * rack +DCh..+E4h, 006E529E..006E5301) and the mover launch with the owner's vt[34h] velocity direction | MINOR (metres at release; equal in level flight) | yes | the image's drift draws 006E513C / 006E51AC are the rack path's |
| D5 | 1385-1430 | mesh hit records keep 0Ah / -1 | not re-derived | label consistent | yes | - |
| D6 | 1520-1531 | wreck window from the host death time | not re-derived | label consistent | yes | - |

## RNG plumbing

| # | Host line | Label / behaviour | Verdict |
|---|---|---|---|
| N1 | 2189-2194 | stream 1 is an LCG stand-in | known: values never the image's; bounds, count and order are what this pass checks |
| N2 | 14906-14913 | stream 0 is a function-local static LCG | label consistent. MINOR: the static outlives the host into the next mission (the same defect the G1 backfill fixed for the aim wander at 14917) |
| N3 | 2238-2239 | Draw enum | MINOR, measurement mode only: `sink_effects` and `auto_target_phase` both = 30, so under BSP_GUNNERY_RNG_STREAMS=1 they share keys |
| N4 | 17004 | reseed only stream 1 | label consistent |

## Other stale labels

| Host line | Label | Why stale |
|---|---|---|
| 14588-14615 | "the model is still NOT switched on" | 14620-14624 enable it (`row.inputs_complete = row_inputs_published`) |
| 10560-10563 | DistErr "is 0 unless kAaFlakAimErrorBound" | `kAaFlakAimErrorBound` is ON (1305); the wording reads as if it were off |

## Not covered (uncovered line ranges)

Not re-derived against the image, label read only: 896-955 (platform attachment, AA line of fire,
kind-6 gate), 1029-1061 (friendly crossing axis, muzzle offsets), 1165-1182 (meshless point
window), 1305-1330 (fire stance, plane hit notify), 1546-1560 (kamikaze contact stand-in, dead
code behind `kKamikazeDynContactBound`), 1664-1677 (kill box), 1776-1785 (turning-gun class),
3703, 7744-7760 (muzzle placeholder text, superseded by `kMuzzleOffsetsBound`), 10582-10600
(flak expiry: the life * V0 > FlyTime * V0 form is MINOR at the last ulp), 11720-11800 and
11966-12010 (plane crash sweep), 13238, 13345-13365 and 13512-13735 (player gun seat and
group-4 messages), 15140, 15419, 15618-15628 (dive-bomb stock). The recon labels are excluded
by the packet.
