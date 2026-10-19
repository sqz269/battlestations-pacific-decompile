# CC13 sub dive: the submarine depth level 008528B0 and session message A2h

Packet cc13-subdive (cc9-approved; pairs go to cc9-ships60). Header
`include/bsp/cc13_sub_dive.hpp`; host `src/game_hosts_units.cpp`
(`GameUnitsHost::set_submarine_depth_level_008528b0` and the 00853630 seed). One switch,
OFF; the A2h post is record-only. Status: reconstructed, build-tested; not game-validated (this packet makes no runs).

## What the image does

`008528B0` `BSP_SubmarineUnit_SetDepthLevel`, `__thiscall(ECX = submarine, int level)`,
`RET 4`, SEH frame (00C945B8), body 008528B0-0085296E.

1. Clamp: `level < 0` gives 0 (008528D5), `level > 3` gives 3 (008528DE).
2. Kamikaze test (008528E5..00852908): `class = [unit+538h]`; `MOVSS` class+510h
   (KamikazeDamage), `COMISS` with 0.0, `JA` to the force; else class+514h
   (KamikazeBlastDamage), `COMISS`, `JBE` past it. Either above 0 (ordered) forces level 1.
   NaN reads false at both.
3. `CMP [unit+1268h], level` (0085290D), `JZ` to the return. On a change: store +1268h
   (0085291E), `0075B430(A2h)` builds a stack message (00852924; it writes only the message:
   vtable 00D02C68, +4h = 3, +8h/+Ch = 0, +10h = A2h, +14h = the game+18CCh slot of
   game+18ECh), the caller then sets vtable 00D02E70, +4h = 1, bytes +24h..+28h = 0 and the
   payload dword +2Ch (message +20h) = level, and calls `0077C7B0(ECX = unit, &msg, 0)` at
   00852956. No destructor call follows (the message is a stack object; EH state 0 only).

`0077C7B0` `BSP_SessionOwner_BroadcastMessage`, `__thiscall(ECX = entity, msg, exclude)`
`RET 8`: `CMP [[00E188A8]+1FE4h], 1` at 0077C7B6 and `JNZ 0077C87B` (the return) at
0077C7C0. Only in session mode 1 does it walk the entity's peer list (+2A8h) and call
`00770B50` per peer, which sends through the transport (`00783DC0`) and skips the local peer.
Nothing on this path appends to the local session-message FIFO (`0076E520`) or is drained by
the pump (`0076C600` at 00875E91).

`00852C60` `BSP_SubmarineUnit_HandleMessage` (vtable +164h of 00D0BF80) is the A2h handler:
kind A2h calls `008528B0(msg+20h)`; kind A1h steps the level by one; others go to 00821E80.
It changes nothing else: no hull write, no sonar/visibility, no torpedo gate.

The hull move to `bands[level]` is NOT made by A2h. The dive controller `00936DC0` reads
+1268h every tick and drives the hull toward `bands[effective level]`; the host models it as
`run_submarine_dive_00936dc0` under `kSubmarineDiveBound` (ON). The one-shot teleport in
00853630 stage 2 (00853B55) is `kSubmarineDiveTeleportBound` (ON). The earlier host comment
("the dive it leads to is not modelled") was stale and is corrected.

`00853630` stage 1 (00853A31..00853A81): the same class test with two `JA`s (NaN false);
`+1268h = (kamikaze ? 1 : 0)` at 00853A81, before stage 2's `Dive` / `TargetDive` overrides.

No shared-RNG draw (`00BD2F10`) occurs in 008528B0, 0075B430, 0077C7B0, 00770B50 or
00852C60, so there is no RNG sub-switch.

## Switches

| Switch | Covers | OFF | ON |
| --- | --- | --- | --- |
| `kCc13SubDiveKamikazeBound` | the class test in 008528B0 and in 00853630 stage 1 | reads false at both (the labelled substitution before this packet) | reads `VehicleClass[type_id]` KamikazeDamage, then KamikazeBlastDamage only if the first is not above 0, once at the seed; the result is kept on the slot (`sub_kamikaze_class`) and used by both sites |

The kamikaze switch covers both sites together on purpose. Fixing 008528B0 alone would let a
kamikaze boat seeded at 0 be moved to 1 by its first 008528B0 call even with level 0
requested, a transition the image never makes (it seeds 1 and stays at 1).

## OFF identity

OFF compiles to the previous code: `kamikaze` is the constant false passed to the existing
`submarine_clamp_depth_command_008528b0`, the seed stores 0 (`sub_kamikaze_class` is never
written and defaults to false), and the change path calls `host.record` with the same name and
address as before. No Lua read is made OFF (the `note_vehicle_class_read` census is
unchanged).

## ON-only lookups

| Lookup | Container | Filled by | Cleared or moved before the read? |
| --- | --- | --- | --- |
| `host.lua.read_vehicle_class_number(row.type_id, "KamikazeDamage" / "KamikazeBlastDamage")` at the seed | the Lua global `VehicleClass` table (`scripts/datatables/autoload/vehicleclasses.lua`) in the units host's `GameMissionLuaHost` state | the datatable autoload, before `create_units`; the same state already serves `read_vehicle_class_row(row.type_id)` a few lines above the seed (the dive bands) and `ship_death_message_class` | no: nothing in the host reassigns or clears `VehicleClass`; the read happens inside the same seed block that already depends on it |
| `slot_.sub_kamikaze_class` in the dive law binding (`run_submarine_dive_00936dc0`, `band.kamikaze_class`) | the same slot field | the seed | no: the dive binding runs only for a slot with `sub_dive_ready`, set in the same seed block before it |
| `slot.sub_kamikaze_class` in 008528B0 | `GameUnitSlot` (host.slots[unit_index]) | the seed above, in `create_units`, for every submarine-kind slot | no: slots live for the mission; 008528B0 returns before the read for a slot whose `submarine_depth_seeded` is false (so a default false is never consulted for a non-seeded slot) |

## A2h: record-only (no switch)

The lead's re-scope asked whether A2h loops back through the local session FIFO (as the
5Dh / 77h kill messages do through 0077C2A0) and is handled one drain later. It does not:
008528B0 calls 0077C7B0, not 0077C2A0, and 0077C7B0 has no local arm. In single player
(game+1FE4h = 0, docs/CONSTRUCT_WORLD.md, the LobbySettings line of this process) it returns
at 0077C7C0. Even in mode 1 it only reaches remote peers through 00770B50, which skips the
local peer. The handler 00852C60 would only call 008528B0 again with the same level. The post
therefore changes nothing in a single-player run, and it stays the record
`SubmarineUnit::depth_message_a2`. The session-mode value itself is audited in
docs/CC13_SESSION_MODE_AUDIT.md.

## Reach (census logs, cc13-stubcensus's eleven, read 2026-10-19)

Counts from each log's host table (`SubmarineUnit::set_depth_level` = 008528B0 calls,
`SubmarineUnit::depth_message_a2` = level changes, which is where the A2h post happens) and
the per-unit seed / Lua lines (local/subdive_count.py).

| Mission | Log | 008528B0 calls | Level changes | Submarines (seed level) |
| --- | --- | --- | --- | --- |
| JM08 | cc9-gunnery52 g53_lfoff_j8 | 0 | 0 | none |
| USN04 | cc9-physics1 p23h_off_u04 | 0 | 0 | none |
| USN21 | cc9-physics1 p23j_off_u21f_def | 1982 | 14 | Darter 1, Dace 1 |
| USN01 | cc9-physics1 p23j_off_u1i_def | 0 | 0 | none |
| BSM09 | cc9-physics1 p23j_off_b9_def | 0 | 0 | none |
| BSM01 | cc9-lua52 l69r_b1c_main_ref | 6933 | 10 | Tautog 0, MiniSub 0 (one Lua SetSubmarineDepthLevel, stored) |
| IJN08 | cc9-lua52 l70r_w_i08_main_ref | 4095 | 12 | JapSub1-3, all 1 |
| USN15 | cc9-lua52 l70r_u15g_main_ref | 0 | 0 | none |
| USNOS | cc9-physics1 p23h_off_os | 87 | 2 | Gato 0; twelve Kaitens (type 4, units #3.1-#3.6, #6.1-#6.6) seeded 0 |
| LOMP06 | cc9-lua52 l70r_w_l6_main_ref | 1791 | 9 | Narwhal 1 |
| JM06 | cc9-physics1 p23c_off_j6 | 12744 | 76 | Narwhal 3, Gato 1, PlayerSub 01-03 1, I-400 #2.1 0 |

Total: 27632 calls, 123 changes, in six of eleven missions (matches cc13-stubcensus's 123).
All but 7 of the calls come from the ship-AI sub-attack states (`ShipAiSubAttack::
set_depth_level_008528b0`); the rest from Lua `SetSubmarineDepthLevel` (JM06 6, BSM01 1).

The only kamikaze submarine class in `vehicleclasses.lua` is class 4, Kaiten
(KamikazeDamage 3000, KamikazeBlastDamage 3000). Of the eleven missions only USNOS has any,
twelve, and no sub-attack summary or 008528B0 call names one of them.

## Expected ON effect

- `kCc13SubDiveKamikazeBound`: only USNOS changes. Each Kaiten's
  `submarine depth seed: unit=unit #3.x level=` line reads 1 instead of 0, and its
  `submarine depth trace` lines read `level=1`. The dive law then targets `bands[1]` = -3.0
  (periscope) instead of `bands[0]` = 0.0 (class rate keys from the `submarine dive bands`
  line: down=1.00, up=5.00, stop=5.00, accel=0.25). In the OFF log the Kaitens already run
  out of air (`unit #3.1` reads `air=-0.505 need=1` by step 3801), and OFF that need forces the
  effective band to 0. ON exempts them (00936E5C..00936E80), so after air-out they keep
  targeting -3.0 instead of 0.0. Any reader of +1268h follows: the Kaiten contact detonation
  path (`summary mission gunnery kamikaze contacts=0` in the OFF log) may change with the
  hull's depth. Details below; GetSubmarineDepthLevel
  (00894100) keeps its own labelled kamikaze substitution (game_hosts_script_orders.cpp, not
  this packet). The other ten missions are unchanged (no kamikaze class seeded).
  The seed half of this switch matches the image only for a Kaiten whose seed Y is
  nearest band 1. The image's last seed stage (the nearest band, section below) can
  overwrite the kamikaze value, and every USNOS Kaiten seeds at -3.66..-4.37, where the
  nearest band is 1. The 008528B0 pin and the dive-law exemption do not depend on it.

### What a Kaiten at periscope depth changes downstream (USNOS)

The OFF baseline is `p23h_off_os.log` (local/subdive_kaiten.py). The Kaitens' bands are
(0.0, -3.0, -40.0, -80.0).

- **Today (OFF).** The law targets the surface. With the boat more than a metre under, step
  6 leaves the rate at 0, so buoyancy decides. The twelve Kaitens bob between about +0.1 and
  -2.1 m: 4 to 12 of their ~25 trace samples are above -1.0. Their air runs out
  (`need=1` first at step 2201..3601, i.e. 110..180 s, against RunOutTime 120). Then all twelve
  sink without limit, to -140..-200 m by the end of the trace. They run `kamikaze_attack` at
  throttle 1.0 from 4500..6500 m, and the mission records `kamikaze contacts=0`.
- **ON.** The target is -3.0 from the first step, so the law removes upward velocity and holds
  them near -3. That is the image's state for a Kaiten.
- **After air-out.** The exemption keeps the target at -3. The sinking after air-out is not
  made by the law (its rate is clamped at 0 and only removes upward velocity), so it is not
  predicted to stop. Whatever drives it (the air or flooding model) is outside this packet.
- **Sensor category / visibility** (00852B90, `kSubmarineSensorCategoryBound`, ON). The surface
  limit is (b0 + b1)/3 = -1.0 and the periscope limit is b1 - 5 = -8.0. OFF, a Kaiten flips
  between `surface` and `periscope_in` as it bobs across -1.0. ON at -3 it reads
  `periscope_in` steadily (a Kaiten has no mast, so never `periscope_out`). Spotting ranges by
  category follow (sensor tables); expect fewer or later sightings of Kaitens.
- **Ship torpedo bots** (008FFFB2, `kTorpedoSubmergedTargetDropBound`, ON). A ship's torpedo bot
  drops a submarine target that is not above (b0 + b1)/3 = -1.0 (00852820). OFF, a Kaiten is
  targetable on the samples where it bobs above -1.0. ON at -3 it is dropped every time, so no
  ship torpedo is aimed at a held Kaiten.
- **Guns / AA.** The submerged-fire tests (00729A80 with 00728A90 / 005459B0) are about the
  *gun's* muzzle being under water, not the target, so they do not change. Gun and AA
  engagement of a Kaiten changes only through the sensor category above (what is sighted).
  I did not search the gunnery host exhaustively for a target-depth gate on gun fire.
- **Contact detonation** (009377E0 / 008145B0 / 00819A20, `cc9_kaiten_contact_detonation`). This
  needs a hull contact. A Kaiten body (extent 1.04 x 1.21 x 11.1 m) centred at -3 is still
  within a warship's draft, so contact remains possible. OFF reached no contact at all because
  the boats sank first. Whether ON reaches one depends on the post-air-out sinking above. That
  is not predicted.

## Dive-path check (lead's item 1)

The host dive law `submarine_dive_step_00936dc0` was traced step by step against 00936DC0 in
docs/SUBMARINE_MODEL.md section 12 (cc9-lua3) and held in pairs. For this packet I re-read
00936E34..00936EAB (the effective band, quoted above) and the binding:

- The bands come from 00853A90..00853AC5 (cc13-gpr53 checked).
- The rate keys are the class row's.
- The structure (steps 4..16) matches.

Two gaps:

- **Precision (24-bit).** The law runs on the main thread at PC = 24
  (docs/CC13_HOST_PRECISION_CENSUS.md row 13 already flags it). The binding computes these
  multi-op x87 chains in double and rounds once:
  - `error*gain/v` (2 ops)
  - `vy + (candidate - vy)*curve` (3 ops)
  - the forward dot product (5 ops)
  - `forward / refspeed / 10 * direction`

  The image rounds after every op. Single-op stores (`vy - UpSpeed`, `vy + DownSpeed`,
  `target - hullY`) are unaffected: a double result rounded once equals the 24-bit result for
  one float op. Expect 1-ulp differences in `rate` and the pitch target. Not fixed here
  (src/submarine_model.cpp, a precision packet).
- **The kamikaze exemption** in the effective band (00936E5C..00936E80) is now under this
  packet's switch (above). The other labelled inputs are unchanged: 008522C0 (read false) and
  the ring writes (step 1).

## The post-air-out Kaiten sink is the image's behaviour (no build)

The lead asked why all twelve USNOS Kaitens sink without limit after their air runs out
(OFF, `p23h_off_os.log`). They sink because they are dead. Each Kaiten dies first and then
sinks as a wreck. The host reproduces each step of the image's chain, so nothing is built.

**The deaths (OFF log).**
- Five were shot: #3.1 at 143.75 s, #3.3 at 162.16 s and #3.5 at 185.31 s by Portland1
  (cat 3); #3.2 at 178.71 s by NH (cat 6); #6.2 at 810.28 s by TroopTrans3 (cat 6).
- Seven drowned: #3.4, #3.6, #6.1, #6.3, #6.4, #6.5 and #6.6. Each has a
  `submarine air: ... drowned (vtable[70h](1))` line and a death row with no killer.
- For #3.1, trace step 2201 reads `level=0` with `dir=-1` and a positive rate. The effective
  band is therefore 3, not 0, so the boat is dead. It sinks at vy = -1.000 (the class
  DownSpeed). Crush pulses start at -71 m, and
  `sunk ship kill depth: ... KillDepth=-200.0` removes it at 246.56 s.

**The image chain.**
1. **Air, 00855250.** The breathing line comes first:
   - 00855256..0085525C: `FLD [unit+1204h]` (bands[1]), `FADD double 00D7A2B0` (3.0),
     stored as a float.
   - 0085526A / 00855270: it is compared with the double at 00CEE4E0, re-read for this note
     as bytes `00 00 00 00 00 00 10 C0` = **-4.0**. `JBE` keeps the sum when -4.0 <= sum;
     otherwise the line is the float at 00CF1430 (bytes `00 00 80 C0` = -4.0). So the line
     is max(sum, -4.0). For a Kaiten (bands[1] = -3.0) it is exactly 0.0.
   - 008552A4: `FCOMIP` hull Y against the line, `JBE 008552CB`. At or below the line the
     air drains by dt / RunOutTime (120 s for the Kaiten); above it, it refills
     (008552AA, `FDIV [class+83Ch]`).
   - 0085537A..008553B8: air above 1.0 is clamped. Air below 0 is stored as 0 and the
     drown kill `vtable[70h](1)` follows.
   - 00855250 reads no class+510h / +514h, so a kamikaze boat drains and drowns like any
     submarine.
2. **The dive law, 00936DC0.** 00936E84 `CMP byte [ESI+5Dh],0` and 00936E8A `MOV EBX,3`
   force the effective band to 3 (-80 m) for a dead boat, after the kamikaze test. The
   dead write wins, so the law drives the wreck down.
3. **Removal, 00826628.** The wreck is removed at KillDepth -200 (packet
   cc9_sunk_ship_kill_depth, hosted).

**Why the OFF Kaitens live 110..180 s.** At level 0 (the host's false kamikaze test) the
law targets the surface and leaves the rate at 0 once the boat is more than a metre under.
The boats bob between +0.1 and -2.1 m, across the 0.0 breathing line, so they drain and
refill in turn. At the `surface` sensor category, Portland1's guns hit them.

**Predicted with kCc13SubDiveKamikazeBound ON (USNOS, pairs to cc9-ships60).**
- The Kaitens are held near -3, always below the 0.0 breathing line. The kamikaze
  exemption keeps them from surfacing to refill, which is the image's design.
- All twelve should drown about 120 s after spawn. Group #3 dies around 120 s instead of
  143..188 s.
- Gunfire kills on Kaitens should mostly vanish: they read `periscope_in`, and ship
  torpedo bots drop them (00852820). The death rows should go from five shot plus seven
  drowned to about twelve drowned.
- Kamikaze contacts stay at 0. The boats start 4.5..6.5 km from TroopTrans3 (d32c
  4548..6496 m), and even at the reference speed of 15.43 m/s they cover at most 1.85 km in
  120 s. Group #6 died 681..2791 m from the transports OFF, after a much longer life.
- The wreck lines (crush pulses, the KillDepth -200 removal) remain, at earlier times.
- Not measured: the spawn times of group #6, so "spawn + 120 s" is a prediction for them.

**A latent host constant mismatch (no reach).** `submarine_air_breathing_line_00855250`
(src/submarine_model.cpp) tests `sum < 0.0f`, but the image compares the sum with -4.0
(00CEE4E0). The two differ only for a sum strictly between -4.0 and 0.0, that is a
PeriscopeDepth strictly between 3 and 7. The shipped classes use 3, 7, 10.2, 12.8, 13.0
and 14.0 (`vehicleclasses.lua`). At 3 and 7 both forms give the same line (0.0 and -4.0),
and the rest fall below -4.0 in both, so no shipped class is affected. The header comment
near `kSubAirBreathingLineFallback` ("discarded whenever it falls below zero") describes
the host test, not the image. Fixing it belongs to the owner of src/submarine_model.cpp; it
is not changed here.

## The nearest-band seed stage, kSubSeedNearestBandCc13Bound (packet cc13-subdive, rev-convoy N1)

00853630's last depth stage (bytes read for this packet):
- **When it runs.** 00853B0F `JNZ 00853BEB` skips the property stage for a holder that is not
  kind 1. Otherwise `Dive` sets the flag `[ESP+13h]`, and a found `TargetDive` goes straight
  to 00853BB0. At 00853BE2 / 00853BE6, a found `Dive` also goes to 00853BB0. Only a boat with
  neither key (or a non-kind-1 holder) falls into 00853BE8.
- **What it computes.** `y` = the hull world Y +100h (after 00414DB0 when +C8h is clear,
  00853BEB..00853BFD). `best = |y|` (00853C0F `AND 7FFFFFFFh`). For i = 1..3
  (00853C23..00853C6C): `d = |float(y - bands[i])|` (`FSUB float [EDX]`, `FSTP float`,
  `AND`). If `best > d` (00853C4B `FCOMIP`, `JBE` skips on equal or unordered), then
  `best = d` and `+1268h = i`.
- **What it does not do.** Band 0 is not compared by subtraction (`|y|` stands in for it), and
  `+1268h` is not written unless some band beats `|y|`. Then 00853C70 jumps to 00853BB3, the
  kamikaze periscope-node test.

**Order against the kamikaze test.** Stage 1 (00853A81) writes `kamikaze ? 1 : 0` first. The
nearest stage runs later and wins whenever a band beats `|y|`. For a Kaiten the kamikaze
value survives only when `|y|` is at most every `|y - bands[i]|`, i.e. a seed at or above
-1.5. So the two switches are independent and the image needs both: nearest-band ON alone
seeds the USNOS Kaitens at 1 but still lets them surface at air-out, and kamikaze ON alone
seeds 1 only by coincidence of their seed Y.

**Host.** Under `kSubSeedNearestBandCc13Bound` (in `include/bsp/cc13_sub_dive.hpp`), the
seed runs the stage when neither `Dive` nor `TargetDive` was found. Every host submarine is
a kind-1, scene-sourced holder. The stage reads `slot->motion.position[1]` (local is world
for a host submarine, as in the teleport) and the seeded `sub_bands`. Each step is one
rounded float op, so 24-bit and 53-bit agree. OFF leaves the earlier value.

**Reach (the eleven census logs, local/subdive_nearest.py).** Y is the first
`submarine depth trace` sample (step 1, vy 0). The host stage itself reads the seed-time hull
Y (`slot->motion.position[1]`, from the placement `entity.world[13]`), not the trace. In these
logs the two are equal to the logged precision, because nothing moves the hull before step 1
except the `Dive` teleport, and these boats have no `Dive`.

| Mission | Sub | Dive / TargetDive | Seed Y | Bands | Host | Image |
| --- | --- | --- | --- | --- | --- | --- |
| BSM01 | MiniSub | none | -30.0 | 0, -7, -40, -80 | 0 | **2** |
| USNOS | Gato | none | -15.0 | 0, -12.8, -40, -80 | 0 | **1** |
| USNOS | twelve Kaitens (#3.1-#3.6, #6.1-#6.6) | none | -3.66..-4.37 | 0, -3, -40, -80 | 0 (1 under the kamikaze switch) | **1** |
| JM06 | I-400 #2.1 | none | -49.53 | 0, -20, -40, -80 | 0 | **2** |
| the twelve `Dive` holders: USN21 Darter, Dace; BSM01 Tautog; IJN08 JapSub1-3; LOMP06 Narwhal; JM06 Narwhal-class Submarine 01, Gato-class Submarine 01, PlayerSub 01-03 | each has `Dive` | - | - | - | stage skipped | stage skipped |

Fifteen boats in three missions differ. JM08, USN04, USN01, BSM09 and USN15 have no
submarine.

**Expected ON effect (pairs to cc9-ships60).**
- `submarine depth seed` lines read the new level for these fifteen boats, and the dive law
  targets the new band from the first step.
- BSM01 MiniSub holds at -40 instead of rising toward the surface, until its Lua
  SetSubmarineDepthLevel(1) (stored).
- JM06 I-400 #2.1 holds at -40 until its Lua level 1.
- USNOS Gato starts at periscope depth -12.8, until the sub-attack AI's own requests
  (depth=0/21/66/0).
- The USNOS Kaitens seed 1. Without the kamikaze switch they still surface at air-out.
- Sensor categories, air drain and early hits on these boats move with their depth. The
  GetSubmarineDepthLevel answers follow +1268h.

## Uncertainties

- (Closed.) The class loader stores the Lua numbers unscaled. 008319F6 pushes the key
  `KamikazeDamage` and 00831A3B pushes `KamikazeBlastDamage`. Each goes through 00B67800 and
  then 00B66330 with a 0.0 default (`FLDZ` at 00831A0A / 00831A4F). The result is FSTP'd
  straight to +510h (00831A22) and +514h (00831A67), with no arithmetic in between. The
  host's `read_vehicle_class_number(type_id, key, 0.0f)` is that value.
- 00894100's own kamikaze test (a third site of the same class fields) stays labelled false in
  `src/game_hosts_script_orders.cpp`; it matters only for a dead / out-of-air kamikaze boat.
- 00853630 also skips the periscope node lookup (00853C90) for a kamikaze class; the host's
  scene-contents find for the node is unchanged by this packet. A Kaiten mesh with a
  `periszkop` node would differ; not checked.
- The A2h verdict holds for single player only; a hosted session would need the peer list.
