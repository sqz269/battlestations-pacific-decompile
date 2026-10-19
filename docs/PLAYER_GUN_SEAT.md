# The player's gun seat: message 79h on the gunnery side

Packet `cc9_player_gun_seat`, cc9-platform2, 2026-09-25. It is the gunnery side of
docs/SHIP_SCREEN_UPDATE.md section 32. Names are descriptive hypotheses, not recovered symbols. The
switch `kPlayerGunSeatBound` (`include/bsp/game_hosts_gunnery.hpp`) is **committed OFF**. Its pair
waits for the session reconnect.

## 1. The message, correcting section 32

005484F0 builds 79h at 005489DA with `00954A10(group, pos.x, pos.y, pos.z, yaw, pitch, held, pressed,
has_target, target_id)`. The argument order is taken from the pushes at 00548975..005489D5:

| field | value | producer |
| --- | --- | --- |
| +1Ch | the group (2 for the Lexington) | 2Eh +44h |
| +20h..+28h | the camera mover's world position | 00427EB0 on 2Eh's +40h |
| +2Ch | yaw of the mover's forward row | 00521370 on `0042D7E0()+20h`, the stack output (atan2) |
| +30h | pitch of the same row, clamped to +/-1.57 (00D1A640/00D1A638) | 00521370's EDX output, asin |
| +34h | **action 99h held** (004C5090) | section 32 had held and pressed swapped |
| +35h | 99h pressed (004C43C0) | |
| +36h, +38h | has-target, the target's +174h id | the target marks |

`BSP_Lighting_DirectionFromAngles` 004B4D80(pitch, yaw) rebuilds the direction as
`(sin yaw cos pitch, sin pitch, cos yaw cos pitch)`, the exact inverse of 00521370. So the message
carries the mover's forward row, apart from the pitch clamp.

## 2. The group 1/2 arm of 00959C20 (00959C91..00959F6D)

For each kind-20h gun on unit+48h that `BSP_Unit_GunAimMessageAcceptsDevice` 00954210(group) keeps.
That filter keeps an operational gun (00729F10) of Function 1, 5 or 6; group 2 also accepts
005459E0 (Function 5 or 6).
1. **The aim point** 00957BD0, when there is no target:
   - `d` is the direction from the two angles.
   - A 1000-unit segment from the camera through the spatial index (0098ADD0 at 00957DA0) aims at
     its hit when there is one.
   - Otherwise `C = P + d*dot(G-P, d)`, with G the gun's position, and
     `aim = C + d*sqrt(max(R^2 - |G-C|^2, R^2/9))`, where R is the bullet range
     `[[gun+3F8h]+34h]+60h` and 9.0 is 00CF0AB8. So the aim point is the camera ray's point at the
     gun's range.
   - When the camera is above the sea and the point below it, the segment is cut at y = 0.
2. **The angles.** 00955830 gives the gun's local angle pair toward the aim point, and 007F60A0
   tests it against the fire window.
3. **Out of the window:** a gun whose seat is AI-held (00521E70(gun, 0): `[gun+1ACh]` 8, or a
   slot 00927F10 reports AI) is left alone. A player-held gun gets 00729F70: `vtable[154h](0, 8)`,
   trigger `vtable[1E8h](0)`, `BSP_Gun_ClearBotFireTarget`.
4. **In the window, AI-held:** `vtable[154h](0, [unit+1B4h])` hands the seat to the unit's role-2
   holder. On that hand-over only, Function 6 calls 0084C500(0) and Function 1 drops its trigger.
5. **Every in-window gun** is turned with 0085ABA0 to the pair (00959E01), and 00859830 stores the
   aim point.
6. **The trigger** (00959E46..00959F68): with 99h held (+34h), a Function 1 mount fires, as does any
   mount within 3 degrees (00D1A8A0) of the pair on both axes. Otherwise `vtable[1E8h](0)`.

**The AI side.** The four bot ticks' side gate (008FFA99, 00902999, 00903136, 0090003B;
docs/GUN_BOT_TICKS.md step 4) runs a bot only while `[gun+1ACh]` is 8 or AI-held. A mount the
player holds therefore gets no bot target, no bot angles and no bot trigger. With the player idle
it does not fire.

**Dispersion.** 0073031D's default arm multiplies the throw by the seat's `BulletThrowMul` only
when 00521E70 finds both roles 2 (00730471) and 3 (00730482) AI-held on the owning unit. With the
player holding roles 2 and 3, every Lexington gun on that arm keeps its authored throw unscaled.
That includes the mounts the AI still fires. This host does not apply the throw magnitude at all
(`gun_throw_magnitude_0073031d` has no caller in `src/game_hosts_gunnery.cpp`), so this read has no
consumer yet. It is left as a contract.

## 3. The binding

- **HUD** (`src/game_hosts_hud.cpp`):
  - `build_fire_message_00954a10` fills a `GunAimMessage79` from the published Operator camera
    (forward row 2, position row 3). That is the mover's pose, because the mover publishes into it.
  - `route_fire_message_0077c2a0` hands the message to `GameGunneryHost::apply_gun_aim_message_00959c20`.
  - Both were records.
- **Gunnery** (`src/game_hosts_gunnery.cpp`):
  - The arm above runs over the unit's rows with category 1, 5 or 6.
  - It uses this host's own conventions: the muzzle point `gun_muzzle_point`, the hull-relative
    angle pair of 008FDAF0, `GameGunRow::max_range` for R, and `gun_fire_allowed_007f60a0`.
  - `GameGunRow::seat_1ac` holds the seat. In `run_gun_aim_and_fire`, a player-held gun takes no
    bot target, its angles come from `seat_horz/seat_vert`, and its trigger from `seat_trigger`.
- **Records:**
  - `PlayerGunSeat::segment_query`, once per message: the spatial-index hit, not modelled, so every
    gun takes the range point;
  - `PlayerGunSeat::target_intercept`, only with a target;
  - `PlayerGunSeat::flak_0084c500`, on a Function 6 hand-over;
  - `PlayerGunSeat::message_other_group`.
- **New summary line:** `summary mission gunnery player seat messages= handovers= returns=
  held_ticks= trigger_ticks=`. It is printed on both sides.

## 4. Predictions, written before the pair

One tree (main 85b73359e plus this), `local\bin\seat_off` against `local\bin\seat_on`,
`BSP_GUNNERY_RNG_STREAMS=1`, USN04 4700/4500 and E2 9200/9000. The role take (section 31) is on, so
the Lexington holds roles 2 and 3, and every 005484F0 call builds and routes one message:
N = 9,158 and 18,158.

**Records and the new line.**
- `HudWeaponGroupScreen::fire_message` and `::route_fire_message` go from N unimplemented to N
  concrete. `PlayerGunSeat::segment_query` N appears.
- The line reads `messages=N`. OFF prints all zeros.

**Which Lexington mounts.** The Lexington carries 22 AA mounts: 10 Function 1, 8 Function 5 and 4
Function 6 (the `gunnery: mount Lexington-class01` lines).
- With no input, the ShipCaptain camera keeps its seeded pitch of -10 degrees behind the ship. Its
  forward ray reaches the sea a few hundred metres ahead, so the water cut makes most aim points
  sea-level points ahead of the bow, a few degrees below each muzzle.
- A mount is handed over only if its horizontal window covers the bow direction **and** its
  vertical window reaches that depression.
- **Prediction:** handovers are between 4 and 16 in the first message (bow-facing mounts on both
  sides, fewer if the vertical windows start at 0). Returns stay under handovers + 10 over the run,
  as the ship turns and the camera follows the hull.
- **Alternative:** handovers of 0 (every AA vertical window above the aim depression) would make
  the whole pair a records-only change. That would itself be a finding about the windows, not a
  defect.

**Gameplay bands, when handovers > 0.**

| line | OFF (4500 / 9000) | ON band |
| --- | --- | --- |
| gunnery damage hit_records | 454 / 588 | falls 3 to 30 percent |
| deaths | 31 / 37 | same or up to 5 lower |
| plane death modes total | 31 / 37 | as deaths |
| torpedo_drop drops | 1 / 1 | unchanged or higher |
| Lexington damage / death | knife-edge | judged from the pair only (docs/SHIP_SCREEN_UPDATE.md sections 33, 36) |

- Every AA hit, death and damage line may move. The camera-aim change reaches the Lexington's AA
  engagement, and through it the planes' paths.
- Section 36 applies: the Lexington's path, and with it 29h's picks, is a knife-edge.
- **Mission end:** neither side ends the mission early in the host; both run their fixed frames.
- **Would falsify the binding:** any movement with handovers = 0; or hit records rising with
  handovers > 0, since player-held mounts fire nothing while the player is idle.

## 5. The pair

One tree (main 85b73359e plus 3c0b84e95), `local\bin\seat_off` against `local\bin\seat_on`,
`BSP_GUNNERY_RNG_STREAMS=1`. Logs: `local\seat_{off,on}_e2.log` and `local\seat_{off,on}_usn04.log`.

| line | E2 OFF | E2 ON | USN04 OFF | USN04 ON |
| --- | ---: | ---: | ---: | ---: |
| player seat messages / handovers / returns | 0 / 0 / 0 | 17,997 / 24 / 19 | 0 / 0 / 0 | 8,997 / 16 / 8 |
| held_ticks / trigger_ticks | 0 / 0 | 53,967 / 0 | 0 / 0 | 41,371 / 0 |
| shots (`gunnery aim`) | 4,932 | 4,588 (-7.0%) | 3,672 | 3,326 (-9.4%) |
| AA hit_records (`gunnery damage`) | 555 | 542 (-2.3%) | 448 | 435 (-2.9%) |
| deaths / total_damage | 37 / 7,832.4 | 37 / 7,832.4 | 30 / 6,487.4 | 30 / 6,487.4 |
| torpedo_drop drops | 0 | 0 | 1 | 1 |
| unimplemented total | 5,285,527 | 5,267,318 | 2,782,240 | 2,773,031 |

- **Records.** `fire_message` and `route_fire_message` turned concrete at N (18,158 / 9,158).
  `PlayerGunSeat::segment_query` N appeared.
- **The summary's `messages`** is 161 short of N. The gunnery summary is written at mission frame
  9000 (or 4500), and the HUD keeps routing through the run's last 200 frames.
- **The other moved lines** follow from the shots: flak bursts, projectile steps, targeted
  refusals, `aa_direct_aims`. Also one plane step (a plane survived one more step) and one torpedo
  gate tick.
- **The Lexington's role line** (`held=080088888`) is unchanged, and so is every other summary line.

**Verdicts.**
- **Held.**
  - The records turned concrete as predicted.
  - Returns stayed below handovers + 10.
  - The trigger stayed off with an idle player.
  - Deaths stayed within the band (identical), and torpedo drops are unchanged.
  - Nothing moved that the seat does not explain, and hit records fell, not rose.
- **Missed, on magnitude.**
  - Handovers are cumulative over the run: 24 and 16, against my 4..16 for the first message. The
    first message's own count is not logged.
  - Hit records fell 2.3 and 2.9 percent, just under the 3..30 percent band.
  - The AA shots fell 7 to 9 percent, but the silenced mounts' rounds were mostly misses, so the
    kill table did not move.
- **Result.** The binding does what the image does, with no contradiction in either form.
  **`kPlayerGunSeatBound` should flip ON.** The flip is one line in
  `include/bsp/game_hosts_gunnery.hpp`, currently leased by cc9-aa-targeting
  (`cc9_gun_horz_sign`), so it is handed to the integrator.

## 6. The segment query 00957DA0 (packet `cc9_player_gun_seat_segment_query`)

### 6.1 What the image does (00957BD0, body 00957BD0..009580DC, `RET 1Ch`)

00959C20 calls it once per kept gun at 00959D2A with ECX = the unit (EBP, written once, from ECX
at 00959C7C) and EDX = the gun (ESI, from the unit+48h list).
- **With a target** (the stack argument tested at 00957C29) it takes 00901C20's intercept and
  never casts.
- **The cast is skipped** when the camera's x and z are both 0.0 (00957CDA..00957D0A, the
  `UCOMISS`/`LAHF`/`TEST AH,44h` not-equal idiom on each).
- **The segment** (00957D10..00957D75): `to = camera + d*1000`. The 1000 is the 8-byte double
  00CE47A0 (`FLD double`). `d` is 004B4D80's direction from the two angles.
- **The call** (00957D79..00957DA0): 0042E630 gives the spatial index. `unit->vtable[20h]()` gives
  the exclusion, then `0098ADD0(camera, to, exclusion, record, 0)`.
  - The kind filter is 0, as in 009043A0.
  - The exclusion is `[unit+360h]` through a different slot than 009043A0 uses. Slot 20h is 006D1E30
    and slot B0h is 006D1DF0. Both are `MOV EAX,[ECX+360h]; RET` on all eleven unit vtables read
    (00CFC3D0, 00D0BF80, 00D01630, 00CFB738, 00CFA778, 00CFFA30, 00CF90B0, 00D0C648, 00D06638,
    00D06920, 00D09678). So the HUD form serves unchanged, and `SegmentQueryArgs` was not
    extended.
- **The hit** (00957DA5..00957DD2): with AL set and the hit's y above 0.0 (00D7A218, a 4-byte
  float read by `COMISS`), the hit point is the aim point. It jumps to 009580AE, past both the
  range sphere and the sea cut.
- **No hit, or a hit at y <= 0**, falls through to the range-sphere point of section 2.

### 6.2 The binding (switch `kPlayerGunSeatSegmentQueryBound`, committed OFF)

- The body of `GameGunneryHost::query_segment_units` moved into the file-local
  `query_segment_units_impl(Impl&, ...)`. The public entry delegates to it unchanged, so the HUD's
  contract (section 4 of docs/HUD_PICK_SEGMENT_QUERY.md) is untouched.
- `apply_gun_aim_message` casts when there is no target and the camera is off x = z = 0, with
  exclude = the seat unit (one based).
- **Once per message, not per gun.** The inputs do not depend on the gun, and the query is pure:
  it restores the two trace counters and draws no random number.
- A hit with y > 0 replaces every kept gun's aim point, skipping the range sphere and the sea cut.
- **Substitutions (labelled in the code):**
  - The world's spatial index is this host's units (`SegmentBinding`: ship meshes or hull
    extents). There are no islands and no terrain.
  - The excluded `[unit+360h]` is the unit itself.
- **Records:** OFF keeps `PlayerGunSeat::segment_query` as a record once per message. ON makes it
  concrete once per cast.
- **New summary line, printed on both sides:** `summary mission gunnery player seat segment
  casts= hits= aims= bound=`. `hits` counts the query's hits, and `aims` counts those above y = 0.

### 6.3 Predictions, written before the pair

Two builds of this tree differing only by the switch, `BSP_GUNNERY_RNG_STREAMS=1`,
`BSP_DEATH_TABLE=1`, E2 = USN04 9200/9000 and USN02 9200/9000.

**USN02.** The seat takes no message there (`messages=0` in the previous worker's
`af_on_usn02l.log`).
- The segment line reads 0 / 0 / 0 on both sides.
- The native table, the summaries and the per-entity tables are identical.
- `PlayerGunSeat::segment_query` does not appear on either side.

**E2, the records.**
- `PlayerGunSeat::segment_query` goes from 18,158 UNIMPLEMENTED to 18,158 concrete. The idle player
  never sets a target (no `target_intercept` row in `wt_on_e2.log`), and the camera is never at
  x = z = 0.
- `casts` equals the seat line's `messages`, about 17,997. Both are summarised at mission frame
  9000.

**E2, hits and aims.**
- The idle ShipCaptain camera sits behind the Lexington, pitched -10 degrees (section 4). Its ray
  crosses y = 0 a few hundred units ahead, so most of the 1000 units run under the sea.
- The query has no sea, so a hit on an escort's hull below the waterline counts in `hits` but is not
  an aim.
- **Prediction:** `aims` is at most 10 percent of `casts`, and `hits >= aims`. A value of 0 is
  plausible: nothing sits within a few hundred units ahead of the Lexington's bow above the water.

**E2, the downstream lines.**
- **If `aims` = 0:** the whole native table, every summary line and the per-entity death table are
  identical, except the segment_query row's status and the unimplemented total. That total falls by
  exactly 18,158.
- **If `aims` > 0:** only the seat line's `handovers`, `returns` and `held_ticks` may move first.
  Through the held set, the AA shots, hit records and deaths may also move. Deaths stay within 3,
  and `trigger_ticks` stays 0 (the idle player never holds 99h).
- **Would falsify the binding:**
  - any moved line with `aims` = 0;
  - `casts` differing from `messages` by more than the 161 late messages;
  - `trigger_ticks` above 0;
  - a moved line on USN02.

### 6.4 The pairs and the verdict

Tree `1cab77c2c`, `local\seg_off` against `local\seg_on` (the same tree, differing only by the
switch), `BSP_GUNNERY_RNG_STREAMS=1`, `BSP_DEATH_TABLE=1`, 9200/9000.
Logs: `local\seg_{off,on}_{e2,usn02}.log`.

| line | E2 OFF | E2 ON | USN02 OFF | USN02 ON |
| --- | ---: | ---: | ---: | ---: |
| `PlayerGunSeat::segment_query` | 18,158 UNIMPLEMENTED | 18,158 concrete | absent | absent |
| segment casts / hits / aims | 0 / 0 / 0 | 17,997 / 0 / 0 | 0 / 0 / 0 | 0 / 0 / 0 |
| seat messages | 17,997 | 17,997 | 0 | 0 |
| hit_records / deaths | 836 / 51 | 836 / 51 | 440 / 22 | 440 / 22 |

- **Both ways, the whole native table** (1,515 rows on E2, 1,417 on USN02), every summary line,
  and every `death row` and `plane death mode` line are identical. The exceptions are the one row
  on E2 and the new segment line's own values.
- **The 0 hits agree with the HUD pair.** docs/HUD_PICK_SEGMENT_QUERY.md section 6 found the same
  query never hits in USN04. The idle camera's segment runs from behind the excluded Lexington,
  and under the sea past a few hundred units.

**Verdict: held, every prediction.** `casts` equals `messages`, `aims` = 0, nothing else moved,
and USN02 is identical. `kPlayerGunSeatSegmentQueryBound` is ON.

## 7. The group 3 arm (packet `cc9_player_gun_seat_artillery`)

Worker cc9-gunnery14. Ranking #7 names the group 1/2 arm, but that arm has been bound and ON since
section 5. The row's reach (reference m: 9 rows, 47371 calls) is the `message_other_group` record.
It sums exactly over the nine rows whose controlled unit selects another group: USN01 2763, JM06
6158, JM08 6156, USN13 160, JM05 5478, USN12 5436, USNOS 4530, USNOS long 16530, IJN01 160
(reference p logs). A ship that is kind 6 and neither 8 nor 0Eh selects group 3 when it becomes the
group unit (`mission_camera.cpp`, 00549328..0054936F), so this packet binds **group 3**. Groups 4
and 5 are now separate records.

### 7.1 What the image does (00959F72..0095A1C7, `JMP 00959FF3` loop, exit to 0095A42C)

The kind is `[msg+1Ch]` itself: 00959C69 `ADD EAX,-1`, `CMP EAX,4`, table 0095A5C0.

1. **Flags.** 00959F72..00959FD0 set three flags for the hand-over:
   - `[ESP+11h]`: held (+34h) or pressed (+35h).
   - `[ESP+12h]`: the world `[00E188A8]+1FE4h == 1` and `+218Ch == 0`.
   - `[ESP+13h]`: `[unit+1ACh]` is not 8, not an AI slot (00927F10), and `00927F30(unit, 0)` is false.
2. **The direction.** 00959FEB `004B4D80(pitch = +30h, yaw = +2Ch)`.
3. **The device walk.**
   - The list is `[unit+48h]`, advanced by `[dev+44h]`.
   - Filter: `vtable[5Ch](20h)`, then `00954210(3, dev)`, which is operational and 005459B0.
   - 005459B0 (body 005459B0..005459D9) accepts a Function `[[dev+3F4h]+80h]` of 2, 3, 4 or 6.
4. **The hand-over** (0095A026..0095A05B): with all three flags and an AI-held seat
   (00521E70(dev, 0)), `vtable[154h](0, [unit+1BCh])`, the role-4 holder (`+1ACh + 4*4`; group 3's
   role mask is 10h). Flag `[ESP+12h]` needs a hosted session (`game+1FE4h == 1`), and `[ESP+13h]`
   a unit held by a non-AI slot that is not the local player, so this is a host handing a remote
   client's artillery over. It never runs in single player, where `game+1FE4h` is 0
   (cc9-gunnery53, SHIP_AI_OPEN_ITEMS 296.8). The single-player seat comes from the role take
   (7.5).
5. **The aim point** 0095A06E: `00957740(ECX unit, EDX &camera, &dir, &out, dev)`. Body
   00957740..00957BCC, `RET 0Ch`, now read whole:
   - A camera below y 0 returns the camera.
   - Otherwise the ray runs to `t = [[dev+3F8h]+34h]+60h`, the range. With `dir.y < 0`, `t` is
     `min(-cam.y / dir.y, range)`. `0098ADD0` queries the segment, filtered by the unit's
     `vtable[20h]`.
   - A hit with `y > 0` (00D7A218 is 0.0) is the point. With `[dev+3FCh] == 0` and a hit entity,
     the entity's lead is added at `[[dev+3F8h]+34h]+5Ch * +50h`: 00902290 for kind 6, else 00901C20.
   - No usable hit: with `dir.y >= 0`, it is `cam + normalise(dir.x, 0, dir.z) * 20000.0`
     (00CE3CB8, double). Otherwise it is the y = 0 crossing `cam - dir * cam.y / dir.y`. This is
     not clamped to the range.
   - Every component is clamped to [-99999, 99999] (00D1A648 / 00D119AC).
6. **The solve** 0095A0AA: `00955630(ECX unit, EDX &aim, dev+0FCh, [[dev+3F8h]+34h]+50h, &vert,
   &horz)`. AL goes to BL. `dev+408h..410h` takes the aim point.
7. **The turn** 0095A0EF: `0085ABA0(dev, horz, vert)`. A refusal clears BL.
8. **The trigger** (0095A0FA..0095A1C2):
   - It is skipped when the world `+1FE4h == 2`, and **skipped for an AI-held seat**.
   - Otherwise, with BL set and both `|00438B10(dev+484h, vert)|` and `|00438B10(dev+480h, horz)|`
     within 2 degrees (00D0C26C, 0.0349066f), `vtable[1E8h](+34h)`; else `vtable[1E8h](0)`.

**What it means for an idle player.** There is no hand-over, since there is no fire input. But
every accepted gun is turned toward the camera solution, the AI-held ones included. The message is
queued by 0077C2A0 and drained by 0076C600 from the session pump (fan-out row 9, 00875E91). That
runs after the gun waves (00875CDD, 00875D4D, 00875DBD) of the same fixed step. So the camera pair
is the last command before the next wave-1 step:
- the bot's own 0085ABA0 in wave 2 only moves `+494h/+498h` until the drain overwrites it;
- the bot's settle test still compares the gun against the bot's solution.

The player's ship's Function 2/3/4/6 guns therefore follow the view, and the AI fires them only
when its solution happens to match.

### 7.2 The binding (switch `kPlayerGunSeatArtilleryBound`, committed OFF in `729e87697`)

- **`apply_gun_aim_message_group3`** (`src/game_hosts_gunnery.cpp`):
  - It computes the aim point and the solve per accepted gun.
  - It stores the pair in `GameGunRow::seat_cmd_*`.
  - A player-held gun also takes `seat_horz/seat_vert` and the 2-degree trigger.
- **The gun tick** applies a pending pair with `gun_set_target_angles_0085aba0` just before the
  wave-1 step, for a gun that is not player-held. That is the drain's position in the image.
- **Substitutions:**
  - The segment query is this host's units plus the land query.
  - The solve is the world-frame (mount NULL) path, turned hull-relative as the bot's 006DFAD4 is.
  - The hit lead (009578C3) is a record.
  - The hand-over (0095A05B) is a record.
  - The world `+1FE4h` flags were taken as a single-player 1. That was wrong: single player is
    `game+1FE4h = 0`, so the image never takes the 0095A05B hand-over there (step 4 above).
- **Summary line:** `player seat groups 0..5=... artillery guns= unsolved= hits= turns=
  refusals=`, printed on both sides.

### 7.3 Predictions, written before the ON runs

**The OFF census.** The OFF build is `a3off`, exported from `729e87697` with no flip. Against
reference p it is gameplay-identical on all ten rows run (exit 1); only the record names moved.
Every other-group message is group 3:

| row | group 2 | group 3 |
| --- | ---: | ---: |
| USN04 | 7997 | 0 |
| USN01 | 0 | 2763 |
| JM06 | 0 | 5997 |
| JM08 | 0 | 5995 |
| USN13 | 4647 | 160 |
| JM05 | 0 | 5317 |
| USN12 | 0 | 5275 |
| USNOS | 605 | 4369 |
| USNOS long | 605 | 16369 |
| IJN01 | 0 | 160 |

The 300-frame ON smoke on USN01 (`a3on_smoke`) gives, per 160 messages, 800 accepted guns, 400
applied turns and 320 refusals, so most camera pairs fall outside the mounts' arcs.

**Predictions.**
1. **Mechanism:**
   - The group census is unchanged.
   - `artillery guns` > 0 on every row with group-3 messages, and 0 on USN04.
   - `turns` > 0, with a large share of `refusals`.
   - `hits` is 0 or small (the idle view's segment rarely meets a unit above y 0).
2. **USN04 (E2 not run):** gameplay-identical (exit 0 or 1). There are no group-3 messages.
3. **Rows with group-3 messages all mission long** (USN01, JM06, JM08, JM05, USN12, USNOS, USNOS
   long):
   - they move (exit 3);
   - the group unit's Function 2/3/4/6 guns hold the camera pair wherever the arcs allow;
   - its artillery shots and damage fall;
   - death rows involving that unit's fire may change.
   - Direction only; the size depends on how often the bot's solution lies inside the refused set.
4. **USN13 and IJN01** (160 messages, the first seconds only): gameplay-identical or a small move.
   Any move is limited to the first seconds' artillery and what follows from it through the shared
   RNG stream.

A move that is not the group unit's own artillery (another unit's targeting, plane paths) would be
a mechanism failure, apart from RNG coupling through changed shot counts
(`shared-rng-stream` noise, which `BSP_GUNNERY_RNG_STREAMS=1` limits to the gunnery streams).

### 7.4 The pair and the verdict

**Setup.** Same-tree pair at `729e87697`:
- `local\a3off` (SHA-256 prefix `ACEF03391F5E`) against `local\a3on`
  (`--flip kPlayerGunSeatArtilleryBound=true`, prefix `B55B9E7C8543`);
- `BSP_GUNNERY_RNG_STREAMS=1`, `BSP_DEATH_TABLE=1`, reference p's launch lines;
- logs `local\a3{off,on}_<row>.log`.

| row | exit | artillery guns / turns / refusals / hits | what moved |
| --- | ---: | --- | --- |
| USN04 | 1 | 0 / 0 / 0 / 0 | nothing |
| USN01 | 1 | 8609 / 4306 / 4226 / 0 | nothing |
| JM06 | **3** | 29985 / 14990 / 8979 / 2810 | Fletcher-class 08 (the group unit): shots 22 -> 19, dealt 419 -> 520. Row hit records 197 -> 201, damage 4112.8 -> 4239.2. Death rows identical |
| JM08 | 1 | 29975 / 14985 / 11988 / 0 | nothing |
| USN13 | 1 | 800 / 400 / 320 / 0 | nothing |
| JM05 | 1 | 21268 / 10632 / 10632 / 0 | nothing (every turn refused) |
| USN12 | 1 | 51950 / 25970 / 25326 / 8200 | nothing |
| USNOS | 1 | 55197 / 27592 / 27592 / 0 | nothing (every turn refused) |
| USNOS long | **3** | 211197 / 105592 / 84892 / 58136 | NH (the group unit): dealt 6035 -> 5363. Portland2 takes 3915 -> 3258 (consistent with GUNNERY_OPEN_ITEMS 63's friendly fire, not traced), TroopTrans1 deals 137 -> 185. Row damage 10635.1 -> 9964.0. One death row changes its per-category hits (unit #2.9); same victims |
| IJN01 | 1 | 800 / 400 / 320 / 0 | nothing |

**Prediction check:**
- **Mechanism held.**
  - The census is unchanged.
  - The counters are non-zero exactly where group-3 messages are, and zero on USN04.
  - Refusals are the large share.
  - Every gameplay move begins at the group unit's own artillery; the other units' changes follow
    from its fire.
  - USN04, USN13 and IJN01 are identical.
  - `unsolved` is 0 everywhere, so every pair is a solved arc.
- **Spread missed.**
  - Only JM06 and USNOS long of the seven all-mission rows move.
  - On JM06 the group unit's damage rises (419 -> 520) while its shot count falls.
  - Accepted turns do not change fire where the unit's bots had no target, or where the next bot
    solution was already inside the refused set.
- **Two ratios.**
  - `turns` is half of `guns`: the gun tick consumes one pending pair per gun per tick, and the HUD
    routes the message twice per tick (one message per entry unit, or two HUD updates per gun
    tick). Only the last pair per tick counts, as in the image, where the drain's last 0085ABA0
    stands.
  - Not resolved here: which of those two causes applies.
  - **Resolved later** (GUNNERY_OPEN_ITEMS 76): two HUD updates per gun tick. The host pumped the
    front-end screens twice per mission frame. `kMenuPumpYieldsToMissionFrameBound` (ON) leaves
    one pump, and `turns` then equals `guns`.

**Verdict: mechanism matches, spread missed; flipped ON** (`kPlayerGunSeatArtilleryBound = true`).
Ranking #7 is closed for group 3. Groups 4 and 5 (0095A1CC, 0095A441) carry no message on any
reference row, and stay records.

### 7.5 The single-player seat: the role-owner device walk (`kRoleOwnerDeviceWalkBound`)

cc9-gunnery53, 2026-10-19. With 7.2 the player's artillery stayed AI-held: no code path wrote a
Function 2/3/4/6 gun's seat, so the trigger at 0095A10C (skipped for an AI-held seat) never ran.
The image's path is the role take:

- Screen 2Eh's 00545410 sends 4Bh (0077C470) with group 3's mask 10h, take 1.
- 00780120's loop sets the role through `vtable[154h]` (0078037C). When the holder changed
  (00780393), 007803A8 calls 0077FE80(msg). For a kind below 53h its default 0078002F calls
  `entity->vtable[164h](msg)`, which is 00821E80. Its 4Bh arm 00821EEA forces the base 0095ABE0.
- 0095AC1D tests the mask, and the first match wins:
  - `& 4` or `== 8`: 00953F60, release only. It releases Function 1/5/6 through 00729F70, but keeps
    a Function 6 when `[unit+1BCh]` is the message's slot.
  - `& 10h`: 00954000. On a take, Function 2/3/4/6 get `vtable[154h](0, slot)`, and Function 6
    also gets 0072D5B0(0, 0). On a release, 00729F70.
  - `& 20h` / `& 80h` (009540D0 / 00954170) stay with the group 4/5 models.
- **Binding.** `GameUnitsHost::Impl::role_message_4b_00780162` calls
  `GameGunneryHost::role_owner_device_walk_0095ac1d` once per role in the mask whose holder
  changed. That covers every 4Bh sender the host routes there.
- **A HUD-site substitution was tried and dropped** (a69d739a1): it compared holders around the
  HUD's `role_request_0077c470`. It missed the release when the script moved the player to the
  ScoutDauntless (SetSelectedUnit at 1823 releases the roles through the units host, not the HUD).
  Northampton's turrets stayed player-held for the rest of the run: shots 113 to 6, held_ticks
  48798. The hook must sit at the role take itself. 00729F70 is
  modelled as in step 3 of section 2. 0072D5B0 is a record, because the host picks the ammo per
  shot. A new summary line, `gunnery role owner device walk takes= releases=`, is printed on both
  sides.
- **Preview pair** (working tree, USN01 16000 frames, reference env, `local\g53_ga_u1b.txt`: 0x9D
  then 0x99 held 300 frames at mission frames 500/900/1300).
  - ON: takes 8, releases 66, held_ticks 0 to 5661. The 8 takes are Northampton's three turrets
    and Dunlap's five Function 6 mounts, because Dunlap is a group entry on the screen. Its 5
    0072D5B0 records are those mounts. Dunlap's shots stay at 3 each, and four of its mounts now
    log 402 angle refusals.
  - The guns are held from about mission frame 410 to the 1823 ScoutDauntless hand-over.
  - trigger_ticks 0: the HUD camera looks 10 degrees down (dir y -0.174). It aims at the sea about
    214 m ahead, below the turrets' window. Every tick's 0085ABA0 refuses (1217 per turret), the
    guns hold, and the 2-degree trigger never passes.
  - Northampton's turret shots 113/113/109 to 95/95/90, because the AI no longer fires a
    player-held mount. Their hits 925 to 812.
  - Deaths and death rows identical (32). Hit records 4407 to 4294.
  - A pair that shows player shells needs a camera aim input, which the harness `input` line does
    not give.
- **The committed pair** (d91c9e54c, `local\g53_rw3_{off,on}`) reproduced the preview exactly. On
  USN04 idle the walk never runs (takes 0, releases 0), and gameplay, death rows and the unit table
  are identical.
- **Verdict: flipped ON by mechanism.** The walk is the image's, and an AI that does not fire a
  player-held mount is the game's behaviour.
- **Gap, now closed for shells:** cc13's `aim` line (SCRIPTED_HELM 18, main 4890e900a) steers
  the camera.
  - Check runs on aa5aaab54 (walk ON), USN01, orders `local\g53_aim2_u1.txt`. Each window has an
    aim, then 0x9D and 0x99 held 300 frames:
    - 450: `aim -77.3 -0.2`, the bearing from Northampton (6300, -3200) to Convoy1's spawn
      (-3500, -1000); the ray meets the sea about 10.8 km out;
    - 850: `aim 77.3 -0.2`;
    - 1250: `aim -77.3 -0.2` again.
  - "helm aim reached" logged yaw +/-1.3491 and pitch -0.0035 rad.
  - trigger_ticks went 0 -> 2234, and the artillery segment query found a point above the sea
    1197 times.
  - **Player shells.** The 1800-frame runs `g53_aim3_{noaim,aim}` end inside the hold window, so
    the turrets fire only for the player. Turret shots went 9/9/8 -> 26/26/24, which is 50 player
    salvos.
  - **Hits: none.** The turret hits are 8/7/9 on both sides. Convoy1's first damage is at 134 s,
    from aircraft.
  - The full 16000-frame run (`g53_aim2_u1`) has turret shots 111/111/106, against 95/95/90
    without aim.
  - Still open: a hit needs an aim held on a moving ship at 10 km, which a fixed yaw does not
    give. The 79h target id (`target_id_174`) is still a record, so the intercept path does not
    run either. Section 7.6 shows that the target id cannot help the turrets.
  - The 1197 artillery "hits" are not the ship lead: `seat artillery lead ship=0 other=0`. The
    camera ray crossed land and never a unit, so 009578C3 never ran.
- **The ray limit (cc9-gunnery53, with `aim unit`, SCRIPTED_HELM 18.1).**
  - The seat ray of 00957740 is `t = [[dev+3F8h]+34h]+60h` long, 2300 for Northampton's 8-inch,
    unless the sea cut is shorter. A ship is led (009578C3) only when that segment hits its hull
    above y = 0 (00957883..00957898).
  - USN01 `450 aim unit Convoy1`: tracking is exact (heading error 0), but Convoy1 is 10.19 km
    away. At about 2.2 km the ray meets a terrain ridge (land points y 23.5..27.7), at the
    waterline pitch and at -0.085 and -0.06 deg. So ship leads stay 0; no hull hit is possible
    in that window in the image either.
- **USN15, the surface fight.** The player's view is Fletcher-class10, and the group 3 seat
  routes to Mississippi's turrets. Fubuki-class04 closes from 3.4 to 1.9 km between 450 and
  533 s (`local\g53_th_fub4*.txt`: 9000 `aim unit Fubuki-class04 [pitch]`, 9010 0x9D, 9012 0x99
  hold 1700).
  - Auto pitch (the target's origin): the hull hit lies at y <= 0, which the y > 0 test drops,
    so ship leads = 0.
  - Pitch -1.0 deg: **009578C3 ship leads 1768 (all turned)**. Mississippi's player-fired shots
    go 0 -> 78, against the idle run (`local\g53_th2_idle.log` / `local\g53_th3_p10.log`).
  - Pitch -1.2 deg: ship leads 0 (the ray meets the sea first).
  - **Hits: 0.** Mississippi's turrets log 7330 turn refusals each, and the ship sinks to a
    Fubuki-class11 torpedo at 528.39 s, so the window is about 80 s. Its lead runs from its own
    muzzles toward a destroyer the camera tracks from Fletcher-class10.
  - **Result:** player shells and ship leads are confirmed; a player turret hit on a ship is not
    produced in the time box. Next row to try: a player battleship with a slower target inside
    its 2300 ray.
  - **Auto pitch after cc13's LOS-point fix (ecf447f17; cc9-gunnery54, `local\g54_auto_th.log`,
    tree a3a8b703e, reference env, `local\g54_th_fub4auto.txt` = the same lines with no pitch).**
    Tracking holds (heading error -0.001 rad, target pitch -0.0096 rad at 3.4 km). Ship leads
    0 -> **1216** (`seat artillery lead ship=1216 turned=1216`), so the raised aim point does put
    the hull hit above y = 0. Mississippi's turrets fire 9 shots each, hits 0, refusals 7245 / 7330
    / 7330. In this run Mississippi sinks earlier, at 485.70 s (killer Fubuki-class06, cat 7, blast), so the
    window is about 35 s. Still no player turret hit on a ship.
  - `8990 select Mississippi` before the same lines (`local\g54_auto_sel.log`) changes nothing: the
    select is the HUD click (SCRIPTED_HELM 10), not a view change, so the camera stays on
    Fletcher-class10 (range 3400 m to Fubuki-class04 at 450 s). The log is identical to the auto
    run in every gunrow and seat count. A ray from the camera on one ship with muzzles on another
    stays the open question. **Time box closed (cc9-gunnery54):** a row where the camera ship
    owns the group 3 turrets is still needed.
- **A player turret kill, USN02 (cc9-gunnery55, tree 6f682c696, reference env).** Houston is the
  camera ship and owns the group 3 turrets. Orders `local\g55_u2_aim.txt` = usn02_l68_u2_a plus four
  windows of `aim unit <ship>` (auto pitch), 0x9D, then 0x99 held: Yamakaze 1800 (hold 400),
  Asagumo 2300 (800), Yukikaze 3200 (600), Nachi 4100 (2400). Log `local\g55_pt_u2aim.log`; the
  baseline is the same plan without the aim lines (`local\g55_tgon_u2.log`, same code).
  - Baseline: Houston's turrets are player-held all run (held_ticks 18096, trigger_ticks 0) and
    fire **0** shots; Houston deals 0 damage.
  - Aim run: tracking holds (heading error under 0.001 rad at 300..1100 m), trigger_ticks 6822,
    `seat artillery lead ship=8787 turned=8784`. Houston fires 123 shots (gunrows plat 1 and 2:
    57 each, first at 90.75 / 90.85 s, 0.15 s into the Yamakaze window) and deals **1931**
    damage. Every Houston shot is therefore a player shot.
  - **Yukikaze dies to the player:** first damage 163.86 s (inside the Yukikaze window from
    160.6 s), death row `killer=Houston killer_gun=36 killer_cat=3 killer_range=275` at 177.36 s.
    In the baseline it dies at 192.11 s to John1.
  - The mission still fails (Game Over 305.05 s, Houston sunk by Nachi at 301.20 s; baseline
    330.04 s). **7.5's open item is closed:** a player turret hit on a ship with `aim unit` and
    auto pitch is produced when the camera ship owns the turrets.

### 7.6 The 79h target: AA only (`kPlayerGunSeatTargetInterceptBound`, cc9-gunnery53)

**What the image does.**
- **The HUD gate** (005488FE..00548975) sets has_target (+36h) and the id (+38h, `MOVZX [target+174h]`)
  only for a unit that:
  - is picked (screen +54h);
  - is not on the player's side: `[target+54h] != [[screen+20h]+14h]+54h`;
  - answers true to `vtable[10Ch]`.
- **Slot 10Ch**, read from the vtables:
  - 0074E1A0 on the seven plane classes whose slot I read (recon, large and small recon, fighter,
    bomber, dive bomber, torpedo bomber), and on 00D05F20 per the ledger: `[unit+B1Ch] > 3.0`
    (00CE3854; FCOMIP/JBE).
    [unit+B1Ch] is the cached speed that slot 38h (007B8E60) returns.
  - 0042BB40 (`XOR AL,AL; RET`) on MBattleship (vtable 00CF90B0), LandingShip, MSubmarine and the
    dummy kamikaze.
  - **So a ship is never sent as a target.**
- **Only the group 1/2 arm reads the id** (00959C91..00959CA8, 00521E30). With a target, 00957BD0
  skips the camera ray: 00957C29..00957CAB call 00901C20 with:
  - the shooter at gun+FCh;
  - the speed [[gun+3F8h]+34h]+50h;
  - the shooter velocity: the unit's vtable[34h] when [unit+4A0h] is set, else the zero vector.
    00953B80 writes [unit+4A0h] as the OR of `[[dev+3F4h]+95h]` (AddsSpeed) over the unit's
    Function 0/1/5/6 guns.
  - The aim is the intercept point; 00957CAB then jumps to the epilogue at 009580C2, past the range
    sphere and the sea cut.
- **The group 3 arm** (00959F72..0095A1CC, 00957740) never reads +36h or +38h. A turret leads a ship
  only through 009578C3, when the camera ray hits the hull (`kSeatArtilleryLeadBound`, ON).

**The binding** (OFF):
- `target_virtual_10c`: a plane-kind unit whose flight-controller world speed is above 3.0.
- `same_side_54`: `unit_side_0054`.
- `target_id_174`: the 1-based unit index. This is a SUBSTITUTION for the handle table, inverted
  in place of 00521E30.
- `seat_target_intercept_00957c29`: the AA bots' 00901C20 build.
- Counters: `seat target intercept messages/unresolved/aims/adds_speed`.
- The gate also feeds the HUD's target marks (`hud_weapon_group_screen.cpp`, the `target_virtual_10c`
  test at the marks), which move with it.

**The pair** (working-tree exes `local\g53_ti_{off,on}`, reference env, 9000 frames; logs
`local\g53_{tioff,tion}_{u4,u21}.log`):
- USN04, idle Lexington: 8497 seat messages on both sides. `target_virtual_10c` turned concrete
  at 7 calls, and all 7 answered false (the pick held a ship or a slow plane). The intercept saw
  0 messages. Gameplay, death rows, the unit table and the native table are identical apart from
  that one record.
- USN21: no seat messages; identical.
- **Reach was the open item.** With an idle or fixed camera, the pick never holds a fast enemy
  plane while group 1/2 is up. Exercising 00957C29 needs a camera that tracks a plane, e.g.
  cc13's aim-at-unit helm line, with group 2 selected (0x9C).

**The second blocker: the target marks cleared the pick.** With `aim unit "B5N Kate #2.1"`
(SCRIPTED_HELM 18.1), `target_virtual_10c` answered true (70 to 94 m/s), but no target reached
the gunnery side.
- The marks (00548E59..0054900D, `target_marks`) look for the nearest entry unit where the player
  holds role 2 or 3 and `[unit+49Ch] > |unit - target|^2`. With none found, 0054900D zeroes
  screen +54h, before the 005488FE gate reads it.
- Both sides were unimplemented stubs answering 0, so `0 > 0` failed every time.
- Now bound under the same switch:
  - 00548EA9..00548ECC: 00427EB0 on both units, then 00427C90 and 00427E30 (the squared length).
  - 00548ED0 `[unit+49Ch]`: the square of 00953B80's largest bullet range over the Function
    0/1/5/6 guns, read as `[[[dev+3F4h]+74h]+34h]+60h` (Function 6 reads +7Ch, its Bullet[2]).
    In the host that is `GameGunneryHost::unit_gun_range_sq_049c`: `gun.max_range`, and
    `SecondAmmo::max_range` for Function 6.

**The pairs, ON exe `local\g53_ti_on3` against OFF `local\g53_au`** (`local\g53_ti3{off,on}_*`):

| row | result |
| --- | --- |
| USN04, `local\g53_ti_kate.txt` (1600 `aim unit "B5N Kate #2.1"`, 1610 0x9C, 1612 0x99 hold 800), reference env | 235 targeted messages, 5170 gun aims. Kate #2.1 dies at 113.80 s to the Lexington's player-fired gun 14 (cat 1), against 116.65 s to Northampton-class02's flak. Its wingmen IV and III also fall to the Lexington. The Lexington's damage dealt goes 934 -> 1059. The same 29 deaths, and hit records 386 -> 384 |
| the same, default env | 200 messages, 4400 aims. The leader dies at 111.85 s (Lexington gun 14), against 116.05 s. The run's one torpedo drop (1 -> 0) does not happen. Deaths 30 -> 31 through the shared stream |
| USN04 idle, 9000 frames | gameplay identical; same_side concrete at 6 calls |
| USN21, 9000 frames | identical |

**Verdict: ON by mechanism.** The player's AA leads the plane it looks at, as 00957C29 does.
