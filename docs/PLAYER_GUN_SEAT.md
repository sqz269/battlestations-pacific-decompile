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
