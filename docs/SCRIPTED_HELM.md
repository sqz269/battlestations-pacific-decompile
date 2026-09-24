# A scripted player helm: the image's path (packet cc9_scripted_helm)

2026-09-23. Read-only so far. Names are hypotheses; offsets are unit-relative unless stated.

## 1. Answer

**On USN04 the image refuses the player the Lexington's helm.** The role-1 transfer the HUD sends
(0077C470(unit, 2, 1) at 0064B9A6) is accepted only when the unit's role-1 permission word
unit+18Ch is 9 (PLAYER_ANY) or the player's own slot. usn_19_coralus.lua sets every Lexington role
to PLAYER_AI (line 458) and then opens only EROLF_AA_FLAK, EROLF_AA_MACHINEGUN and EROLE_CAPTAIN
(= 1, the captain's flag bit) to PLAYER_ANY (line 459). Role 1 is EROLF_PILOT (= 2,
scripts/global/luamw_init.lua lines 109-110). It stays at 8, and line 866 resets everything to
PLAYER_AI again. So the transfer fails at the 4Bh receive arm. The player never holds role 1, and
0064B870 never issues a helm order for the Lexington.

**The same read corrects docs/CONTROLLED_UNIT_HELM.md.** unit+184h, the byte that forces a ship
into `cruise` (009F3DF3), is set only when a player **takes role 1**, not role 0 (00780214, below).
On USN04 that never happens. So in the image the idle player's Lexington does not have +184h set.
It is not forced into cruise, and it follows its authored orders under the AI. The host sets its
player-controlled answer for the controlled unit and keeps the Lexington in cruise arm 2, where it
coasts to rest. That is the idle E2 reference's stationary carrier, and on this reading it is not
the image's behaviour.

No harness option was built. Both findings change what the packet should do; see section 5.

## 2. The transfer, 0077C470 and the 4Bh arm

**0077C470**, `__thiscall(unit, uint roleMask, int take)`, `RET 8`:
- Gated on `game+5D4h > 0Ch` or `game+216Ch`, and on session mode `game+1FE4h == 0` (or the client
  test).
- It builds message 4Bh through `BSP_SessionMessage_ConstructBase(4Bh)`, with vtable 00D02C90 and
  these fields:
  - +04h = 1;
  - +18h, +1Ah, +1Ch and +20h = 0;
  - **+24h = roleMask**;
  - **+28h = the local player slot `game+18ECh`**;
  - **+2Ch = take**;
  - +30h = 0.
- It routes the message with `BSP_Session_RouteMessage(msg, 0, 0)` on the unit.

**The HUD call site, 0064B97A-0064B9BD.** It calls `0077C470(unit, 2, 1)` when all of these hold:
- the thrust axis is above 0.1, or the held flag is set;
- unit+1130h == 0;
- `00927F30(unit, 0)`: the local player holds role 0;
- `!00927F30(unit, 1)`: the player does not yet hold role 1.

It then seeds the HUD levers without waiting for the answer: HUD+28h = unit+984h and
HUD+24h = unit+980h.

**The receive, 00780120's 4Bh arm (00780162-007803D9), read here.** With msg+20h == 0:
- **The +184h store (007801F2-00780235):**
  - take == 1, mask bit 2 (role 1) and `0059BBD0(unit, 1, slot)` give `unit+184h = 1` (00780214);
  - take == 0, mask bit 1 and `unit+1ACh == slot` give `unit+184h = 0` (00780235), then
    `0080E290` for a kind-6 unit.
- **The per-role loop,** i = 0..8 over the mask bits. For take == 1 it calls
  `unit->vtable[154h](i, slot)` and relays the message (0077C7B0) only when both hold:
  - the current holder `unit+1ACh+i*4` is 8 or AI-held (`BSP_PartySlot_IsAiHeld`);
  - the permission `unit+188h+i*4` is 9 or equal to the slot.
- Role 0 also rebinds the player record's unit (+4Ch).

**0059BBD0**, `__thiscall(unit, int role, int slot)`, body read whole: it returns
`unit+188h+role*4 == 9 || == slot`.

**The permission words.** They are written by `unit->vtable[148h](mask, value)`. For the instance
vtable 00CFC3D0 that slot is 0077F360, which calls 00927D20. That function stores `unit+188h+i*4 =
value` for every mask bit (`piVar4[-9]` with `piVar4 = unit+1ACh`). SetRoleAvailable (008AB850)
reaches it at 008ABA51 in session mode 0.

**The +184h writer census.** A byte-store scan for `[reg+184h]` over MOV/C6/88/89/C7 forms finds
these unit writers:
- 00928701, the constructor;
- 00780214, set;
- 00780235 and 00780439, clear;
- 009281D6, clear.

Every other hit is on part instances or screens. So role-1 acceptance is the only producer of 1.

## 3. The helm message, 0064B870 to 00816A40

- **The issue.** Each frame, while `00927F30(unit, 1)` holds, 0064B870 quantizes HUD+24h
  (thrust, quarter steps) and HUD+28h (turn, sixth steps). It issues `00816A40(unit, thrust, turn,
  kind 0)` at 0064BB12 (docs/UNIT_ORDER_RECORD.md).
- **00816A40** (00816A40-00816AF1, `RET 0Ch`) builds the 20h-byte record (00815440 clamps both to
  [-2, +2]).
  - It publishes the record through 0080DAD0 into ring slot `[unit+97Ch]`. 0080DAD0 clears the
    slot's +08h, which marks it authoritative, and mirrors it into unit+994h/+998h/+99Ch.
  - **Only in session mode 2** (00816A8D) does it also build message **8Eh** (0075B430 at
    00816AA1). The message carries the record at +1Ch and is sent through 0077C2A0.
- **008141A0** (through arm 00821EBE) applies an 8Eh on the receiving peer. It acts only when the
  sender holds the unit's helm slot, and it backfills with 00812FA0 at the lag-compensated age.
- **In single player (mode 0, this host's value) no 8Eh is sent.** The authoritative slot and the
  confirmed pair come straight from 0080DAD0.
- The existing `--order` option already models that part: `issue_into_ring` re-issues through
  00816A40 every step.
- With role 1 held, cruise takes arm 1, the helm bypass (009E13B4). It sets blk+3F5h, and
  009F3F80 then skips its ring write (009F3FF2). The host honours that byte
  (`drive_order_ring_009f3f80`).

## 4. What a scripted helm would have to fake

A faithful role-1 transfer on USN04 is refused. A harness that "performs the transfer as the image
does" would do nothing. Making the carrier move by helm needs one of these:
- **(a)** A scenario override that opens EROLF_PILOT to PLAYER_ANY on the Lexington, as a
  labelled scenario change equivalent to editing line 459. Then the transfer, the +184h store, the
  cruise arm 1 bypass and the per-frame 00816A40 all run as in the image.
- **(b)** Forcing the role-1 slot without the permission, which is not the image's behaviour.

## 5. Decision needed

- **The idle reference.** On this reading, the image's idle E2 Lexington is AI-driven, with no
  +184h. It is not the host's coasting carrier. The host's "controlled unit implies +184h" stand-in
  would need replacing with the 4Bh role bookkeeping. That is a host fidelity change to the
  reference, not a harness scenario, and it touches the controlled-unit and cruise-arm code.
- **The scripted helm.** It is only possible as scenario (a).

## 6. The role bookkeeping, bound (packet cc9_player_role_bookkeeping)

### 6.1 What is bound

The switch is `kPlayerRoleBookkeepingBound`, in `src/game_hosts_units.cpp`.

**Permission words.**
- unit+188h + role*4 starts at 9 for every role, from 00928630's `vtable[148h](1FFh, 9)`.
- SetRoleAvailable (008AB850, session mode 0, 008ABA51) now reaches `vtable[148h]` = 0077F360 ->
  00927D20. That writes the word for every mask bit.
- A kind-2 unit whose role is held and is closed to PLAYER_AI (8) gets a 4Bh release naming the
  holder.

**The 4Bh arm of 00780120 (msg+20h == 0, msg+30h == 0).**
- take 1:
  - mask bit 2 with 0059BBD0(unit, 1, slot) sets **unit+184h = 1** (00780214);
  - each mask bit's role is taken through `vtable[154h]` = 0077F480 -> 009281C0 when the holder is
    8 or AI-held and the permission is 9 or the slot.
- take 0:
  - mask bit 1 with role 0 held by the slot clears +184h (00780235);
  - each role the slot holds goes back to 8.
- 009281C0 clears +184h when role 0 is given 8.

**HUD page 27h's update, 0067BB50.** It is slot 20h of vtable 00CF7A38, constructor 0068A8A0, a
page INTF_CAPTAIN and the other unit interfaces open.
- It tracks the controlled unit, 00E188D8, at +30h and releases role 0 on a previous unit.
- For a kind-2 unit, it then takes role 0 with 0077C470(unit, 1, 1) when the player lacks it and
  0059BBD0(unit, 0, slot) passes.

**unit+184h**, read by 009F3DF3 (cruise forcing), 009E11C8 (cruise arm 2) and 009F5E06 (auto-target
suppression), is now the unit's own byte. It is no longer "the unit 004C0890 bound".

**Stand-ins, labelled in the source.**
- The local slot game+18ECh is 0.
- Every other slot is AI-held for 00927F10.
- 0077C470's route delivers to the unit at once.
- The page's shown byte +4h is taken as set while a unit is controlled, and the page runs once
  per fixed step.
- Not modelled:
  - the player record's unit rebinding (007802A0-0078030A);
  - 0080E290 on a kind-6 release;
  - 00927D20's per-player `vtable[2Ch]` tail;
  - 0077F360's and 0077F480's kind-5 suppress byte.

### 6.2 Predictions, written before the runs

The pairs run the switch OFF (the current reference) against ON, from the same tree, with
`BSP_GUNNERY_RNG_STREAMS=1`.

**The OFF side is the current reference** (VU0, section 5 of docs/ATTACKER_EVASION.md):
- E2: 35 deaths, 469 hit records, first hit 98.85 s;
- the Lexington moved 100.51 m;
- ship AI ai_owned=20 of 21;
- auto-target chose=0;
- no mission end.

**The Lexington under ON.**
- Roles: it holds role 0 = 0 (the player's captain seat) and every other role = 8. Its permission
  words read 9,8,9,9,8,8,8,8,8 (captain, machine gun and flak open). **+184h = 0.**
- It is no longer forced into cruise. Its director holds the script's
  `NavigatorMoveOnPath(CarrierPath1, PATH_FM_CIRCLE)` (usn_19_coralus.lua line 515), which the AI
  drives.
- Speed: about the Yorktown's 16.66 m/s on the same kind of order (its logged target_speed),
  between 12 and 17.1 m/s (reference_speed 17.105).
- Track: along CarrierPath1, not the spawn point. Displacement by 225 s is 2.5-3.9 km. The E2
  `moved` row, a displacement, is 1-7.7 km depending on how far round the circle it gets.
- Ship AI ai_owned rises from 20 to 21.
- **Torpedo response stays closed.** Cruise arm 3 stores 00521E70(unit, 0) into blk+3ECh, and with
  role 0 held by the (human) slot 0 that is 0. So the Lexington still asks for no torpedo
  avoidance, now from arm 3 rather than arm 2.

**Auto-target.**
- 009F5E06 no longer suppresses the Lexington, so its director may choose targets.
- Prediction: chose 0-50. The row is also held down by the other gates, which stayed 0 for
  every other ship.

**Kates and Vals.**
- In the reference both torpedo flights that reached aim target the Yorktown, not the Lexington.
- Aim-entry ranges and releases (0 / 0) are unmoved except through path coupling.

**Escorts.** Those keeping station on the Lexington now follow a moving leader. The ship AI's
station_keeping and replan counts move.

**AA.**
- The carrier moves under the attack, so path coupling moves the hits.
- E2 deaths 28-42 (35 reference), hit records 380-560.
- No ordnance release, as before.

**Other missions.**
- **USN04 4500:** the same Lexington change. Deaths 20-32 (26 reference), and moved 1-3.9 km at
  225 s.
- **USN01 3000:** the controlled unit is Airfield2, a non-ship on an unresolved dispatch. Its role
  0 may be taken, but no reader in this host reacts to it. Rows are flat: 5 deaths, 2 torpedo
  drops, first hit 55.65 s.

**Mission end:** none in all three.

### 6.3 The pairs, measured

The runs used same-tree binaries `local\rk0b` (switch off) and `local\rk1b` (on), with
`BSP_GUNNERY_RNG_STREAMS=1`.

**A first pass (`local\RK*`) exposed a second stand-in.** The director step (00836920) and the
cruise step (009E11C8) took "is the controlled unit" from their own `is_controlled` call rather
than from `unit_player_controlled_0184`. With the switch on, cruise arm 2 therefore still ran
(914 calls). Both now read the unit's byte through `player_flag_0184`, and the `RB*` pairs below
are from that tree.

**The OFF side:**
- RB0 matches RK0 on every summary row. It is the current main's reference (E2: 461 hit records,
  35 deaths).
- The moved row is the stationary carrier of section 1: 100.51 m.

| row | E2 off | E2 on | USN04 4500 off | USN04 4500 on | USN01 off | USN01 on | prediction | verdict |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Lexington +184h / roles held / open | - | 0 / 0,8x8 / 9,8,9,9,8x5 | - | same | - | Airfield2 role 0 taken | as written | held |
| controlled unit moved | 100.51 m | **6880.79 m** | 100.51 m | **3424.02 m** | 0.00 | 0.00 | 1-7.7 km / 2.5-3.9 km / flat | held |
| total path (all units) | 91025 | 135988 | 46762 | 68949 | 12212 | 12212 | - | - |
| ship AI ai_owned | 20 | 21 | - | 21 | - | - | 21 | held |
| cruise arm 2 (009E11E8) calls / cruise steps | 235 / 31 | 0 / 647 | - | 0 / 401 | - | - | arm 3 instead of arm 2 | held |
| auto-target scans / chose | 9020 / 0 | 9471 / 0 | 4520 / 0 | 4746 / 0 | 9211 / 0 | 9362 / 0 | chose 0-50 | held |
| hit records | 461 | **584** | 367 | 474 | 135 | 135 | E2 380-560 | **missed**: above |
| Japanese deaths | 35 | 35 | 27 | 27 | 5 | 5 | 28-42 / 20-32 / 5 | held |
| damage | 7700.0 | 7720.9 | 5940.0 | 6252.5 | 2250.0 | 2250.0 | - | - |
| fighter hits (E2) | 64 | 80 | - | - | - | - | coupled | - |
| releases (torpedo / bomb) | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 2 / 0 | 2 / 0 | unmoved | held |
| plane water contacts | 15 | 15 | 11 | 13 | 3 | 3 | - | - |
| first hit | 98.85 s | 99.05 s | 98.85 s | 99.05 s | 55.65 s | 55.65 s | - | - |
| mission end | none | none | none | none | none | none | none | held |

**Reading.**
- **The carrier sails its path.** It covers 3.4 km by 225 s and 6.9 km (displacement) by 450 s,
  about 15 m/s along CarrierPath1. That is the Yorktown's order speed.
- **The torpedo gate stays closed for it, now through arm 3.** Cruise arm 2 no longer runs, and
  the step takes arm 3. The Lexington's own slot, role 0, is slot 0, a human, so 00521E70 answers
  0 and blk+3ECh is 0. The host never took its "owner roles unavailable" record.
- **The Kates now reach aim on a moving carrier.**
  - On E2, Kate #2.1 .-4 and #6.1 .-2/.-3 enter aim against the Lexington at 314-422 m, with
    target_speed 15.4-16.2 m/s.
  - In the off run no Kate aims at the Lexington: all five aim entries target the Yorktown.
  - No Kate releases in either run; the release gap is upstream.
- **Hits and fighter hits rise.**
  - Hit records rise 461 to 584 on E2 and 367 to 474 on USN04. The carrier and its escorts now move
    under the attack, and the Lexington's own director is no longer suppressed.
  - Deaths are unmoved, so the extra hits land on surviving aircraft.
  - The per-kill split is RNG-coupled and is not attributed.
- **USN01 is flat** apart from Airfield2's auto-target scans. Its role 0 is taken, and no reader
  here acts on it.

**Switch state landed: `kPlayerRoleBookkeepingBound` ON.**

### 6.4 Reference rows with the role bookkeeping (for docs/GAME_EXECUTABLE.md)

This section is written here because docs/GAME_EXECUTABLE.md is leased to
`cc9_surface_gunnery_reference`. It is to be appended there as a dated section.

> **Mission reference baselines, 2026-09-24 (after the player role bookkeeping).**
>
> The binary is `local\rk1b`, built from agent/cc9-dogfight-engaged on main 35a065629. It is the
> packet cc9_player_role_bookkeeping tree, with `kPlayerRoleBookkeepingBound` ON and every other
> switch in its landed state. The runs used `BSP_GUNNERY_RNG_STREAMS=1`.
>
> **Every idle-player row before this section carried a host artefact:** the controlled Lexington
> stood still (100.51 m) because the host read "controlled unit" as unit+184h. In the image
> +184h needs an accepted role-1 take, which USN04's script never allows. So the image's carrier
> follows its CarrierPath1 under the AI (docs/SCRIPTED_HELM.md section 6,
> docs/CONTROLLED_UNIT_HELM.md correction).

| mission | frames | damage | deaths | queued_hits | torpedo drops | bomb drops | plane water contacts | first_hit | controlled moved | mission end | log |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| USN04 | 4500 mission | 6252.5 | 27 | 474 | 0 | 0 | 13 | 99.05 s | 3424.02 m | none | `local\RB1_usn04.log` |
| USN01 | 3000 mission | 2250.0 | 5 | 135 | 2 | 0 | 3 | 55.65 s | 0.00 (Airfield2) | none | `local\RB1_usn01.log` |
| USN04 (E2) | 9000 mission | 7720.9 | 35 | 584 | 0 | 0 | 15 | 99.05 s | 6880.79 m | none | `local\RB1_9000.log` |

The same-tree OFF rows are `local\RB0_*`:

| mission | damage | deaths | queued_hits | controlled moved |
| --- | --- | --- | --- | --- |
| USN04 4500 | 5940.0 | 27 | 367 | 100.51 m |
| USN01 3000 | 2250.0 | 5 | 135 | 0.00 |
| E2 9000 | 7700.0 | 35 | 461 | 100.51 m |

## 7. The scripted helm option (packet cc9_scripted_helm_option)

### 7.1 The option

**Syntax.** `BSP_PLAYER_HELM=<throttle>[,<rudder>]` is an environment variable, like
`BSP_GUNNERY_RNG_STREAMS`. It is unset by default. It is not a command-line flag because the
option parser lives in `src/game_hosts.cpp`, which this packet may not touch.

**What it performs**, in `src/game_hosts_units.cpp` (`player_helm_prepare_0064b870` and
`player_helm_issue_0064b870`). It needs `kPlayerRoleBookkeepingBound`.
1. **Open the pilot role.** At the first fixed step with a controlled unit it calls
   `SetRoleAvailable(unit, EROLF_PILOT, PLAYER_ANY)` exactly as the script binding does in
   session mode 0: `vtable[148h]` = 0077F360 -> 00927D20 with (2, 9). Only unit+18Ch changes.
2. **The role-1 transfer, every step, under 0064B870's conditions (0064B97A-0064B9BD).** The
   conditions are:
   - the thrust axis |a| > 0.1, where the option's throttle stands in for the axis;
   - unit+1130h == 0, taken as 0 because it has no host field;
   - the player holds role 0;
   - the player does not hold role 1.

   When they hold, it sends 0077C470(unit, 2, 1) into the bound 4Bh arm and seeds the HUD levers
   +28h/+24h from unit+984h/+980h (ring+14Ch/+148h).
3. **The helm, every step while the player holds role 1 (0064B9C0-0064BB16).**
   - The levers take the option's values, clamped to [-1, 1]. This stands in for the input
     integration, and the clamp bounds were not read.
   - They are quantized as 0064BAB5/0064BAEE do (floor with bias 0.49; quarter thrust steps,
     sixth turn steps).
   - They are issued through 00816A40. Its single-player publication is 0080DAD0, the
     authoritative slot and the unit+994h/+998h mirror. No 8Eh message is sent in session mode 0.
   - The issue sits where the `--order` standing order is refilled: after the slot promotion,
     once per fixed step instead of once per frame.
   - game+19C4h, which would release role 1 at 0064BB19, is taken clear.

### 7.2 Predictions, written before the pair

The pair is E2 9000 with the option unset (HO) against `BSP_PLAYER_HELM=1.0,0` (H1), from the same
binary, with `BSP_GUNNERY_RNG_STREAMS=1`.

**Option unset: identical to the 330b81cdc rows** (RB1, 6.3):
- 584 hit records, 35 deaths, 7720.9 damage;
- the Lexington moved 6880.79 m;
- auto-target scans 9471 / chose 0;
- releases 0/0 and no mission end.

**Option on, throttle 1.0, rudder 0.**
- **Roles.**
  - The Lexington's open words read 9,9,9,9,8x5 and it holds roles 0 and 1 (0,0,8x7).
  - **+184h = 1** (one transfer).
  - Every step issues (about 9000 issues).
- **Cruise.**
  - 009F3DF3 forces cruise.
  - 009E1170 takes **arm 1** (the role-1 slot is human, not AI), which sets blk+3F5h. So
    009F3F80 skips its ring write: the ring_gated_3f5 count is above 0 for the Lexington.
  - Cruise arm 2 is not taken.
- **Track.** A straight line on the spawn heading at full throttle, since rudder 0 holds the
  heading. The Lexington holds about 17.1 m/s (reference_speed 17.105), against the scripted
  path's 15 m/s. **Displacement is 3.6-3.9 km by 225 s and 7.0-7.7 km by 450 s**, larger than
  the 6.88 km scripted run because the line does not circle back.
- **Torpedo gate.** It stays closed: arm 1 stores blk+3ECh = 0.
- **Auto-target.** 009F5E06 suppresses the Lexington again, so scans fall back toward 9020.
- **Kates.** They aim at a faster, straight-running carrier:
  - the aim entries on the Lexington change;
  - 0-5 of them occur, each with target_speed of about 17 m/s;
  - releases stay 0/0.
- **Escorts.** They keep station on a leader that no longer circles, so the station requests move.
- **AA.** Deaths are 28-42, hit records 450-650, coupled.
- **Mission end:** none.

### 7.3 The pair, measured

The runs used the same binary `local\hl`, with the option unset (`local\HO_9000.log`) and with
`BSP_PLAYER_HELM=1.0,0` (`local\H1_9000.log`). `BSP_GUNNERY_RNG_STREAMS=1` was set on both.

**The unset side is identical to the tree without this packet.** `local\hbase` was built from the
same merge (main 5c126f14d) with this packet's source removed, and its run `local\HB_9000.log`
matches HO on every one of the 148 summary rows. Neither matches RB1 of 6.3, because main moved
between the two:
- hit records are 549 against RB1's 584;
- the Lexington moved 6905.23 m against 6880.79 m;
- the rest is equal: 35 deaths, 7720.9 damage, 0/0 releases.

The prediction "identical to the 330b81cdc rows" missed on that drift, not on the option.

| row | option unset (HO) | throttle 1.0, rudder 0 (H1) | prediction | verdict |
| --- | --- | --- | --- | --- |
| Lexington roles held / open / +184h | 0,8x8 / 9,8,9,9,8x5 / 0 | **0,0,8x7 / 9,9,9,9,8x5 / 1** | as written | held |
| transfers / issues | - | 1 / 9000 | 1 / about 9000 | held |
| levers seeded from +980h / +984h | - | 0.8769 / 0.0000 | - | - |
| cruise arm | arm 3 (647 cruise steps) | **arm 1 (009E13B4) 356 calls**, arm 2 none | arm 1 with blk+3F5h | held |
| Lexington moved (450 s) | 6905.23 m | **7689.92 m** | 7.0-7.7 km | held |
| Lexington speed seen by Kates | 15.3-16.2 m/s | **17.10 m/s** | about 17.1 | held |
| auto-target scans | 9471 | 9020 | back toward 9020 | held |
| Kate aim entries on the Lexington | 3 at 316-421 m | 3 at 334-352 m | 0-5 | held |
| torpedo releases | 0 | **2** | 0 | **missed** |
| bomb releases | 0 | 0 | 0 | held |
| Japanese deaths | 35 | 35 | 28-42 | held |
| hit records | 549 | 492 | 450-650 | held |
| damage | 7720.9 | **8848.9** | - | a torpedo hit |
| first hit | 99.05 s | 98.70 s | - | - |
| plane water contacts | 15 | 15 | - | - |
| mission end | none | none | none | held |

**The two releases.**
- Kate #6.1 .-3 dropped at 13 m and 73.8 m/s on the Lexington. The carrier ran 97.4 m during the
  torpedo's run, and the closest approach was 69.7 m, a miss. The torpedo expired after 1852 m.
- Kate #8.1 .-4 dropped on the Yorktown. The Yorktown's closest approach was 180.8 m, and the
  torpedo struck the escort Fletcher-class05 at 9.55 s. That is the extra 1128 damage.
- In the unset run these Kates reach aim but do not release. The faster straight carrier changes
  their approach geometry enough to satisfy the release gate. It is one scenario row, not a
  mechanism.

**Alternate reference row, "scripted helm, throttle 1.0", for docs/GAME_EXECUTABLE.md.** That doc
is leased to `cc9_surface_gunnery_reference`, so the lead routes this row. It is never the
reference.

| mission | frames | scenario | damage | deaths | queued_hits | torpedo drops | bomb drops | plane water contacts | first_hit | controlled moved | mission end | log |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| USN04 (E2) | 9000 mission | **scripted helm, throttle 1.0** (`BSP_PLAYER_HELM=1.0,0`, main 5c126f14d + cc9_scripted_helm_option) | 8848.9 | 35 | 492 | 2 | 0 | 15 | 98.70 s | 7689.92 m | none | `local\H1_9000.log` |
| USN04 (E2) | 9000 mission | the same binary, option unset (the idle reference on that tree) | 7720.9 | 35 | 549 | 0 | 0 | 15 | 99.05 s | 6905.23 m | none | `local\HO_9000.log` |
