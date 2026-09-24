# The in-mission camera: what produces the matrix the HUD projects with

Packet `cc9_mission_camera` (worker cc9-platform, 2026-09-23). Part 1 is the read (sections 1 to 3);
part 2 is the full update, the binding and its predictions (sections 4 to 8).
Addresses: 00B70490 00B6DB70 0043A660 004DE73F 00B71A80 004BC410 0042A920 0064DA40 0064B650
00519290 00432ED0 0042DEF0 004329D0 0042DCD0 0042D5A0 0042C610 0042F0C0 0042ED50 00415510 00432650
00651370 00651760 0068ACA0

## 1. Who asks for the camera

- **The markers.** `0043A660` (`BSP_Camera_ProjectWorldToScreen`) loads `[[00E188A8]+19FCh]`
  (0043A697), calls 00B70490 on it and transforms the point with 00B62D10.
- **The minimap.** It refreshes the same node with 00B6DB70 before reading its axes.
- **The node.** `game+19FCh` is the **Operator** camera node. `004DE73F` creates it with the
  camera constructor `00B71A80` (458h bytes, name `Operator` at 00CE7E04;
  `docs/GAME_WORLD_CONSTRUCT.md` row 5). It survives `004D2BB0`.

The view-projection side is already reconstructed:
- `get_camera_view_projection_00b70490` (`src/camera_transform.cpp`) reads a `CameraState`: a
  transform (world at +F0h) and a projection (fov +1C4h, aspect +1C8h, near +1D4h, far +1D8h).
- `refresh_native_camera_world_00b6db70` refreshes the world from local and parent.
- `build_projection_00b642f0` builds the perspective.

What the host lacks is the **producer of the node's pose** each frame.

## 2. The node's pose comes from a camera mover

- **`004BC410` sets the mover** (`__thiscall(game, mover)`, RET 4). It stores the mover at
  `game+1ED4h`, calls the new mover's virtual +124h with the old one (the hand-over), and destroys
  the old one through 00926D90. It then calls `0042A920(mover, [game+19FCh])`, which is
  `mover+1BCh = node`, and registers the mover with `[game+21C4h]` through 00445790.
- **Each frame the mover pushes its pose into the node.** The mover's common tail `004329D0`,
  reached from the update, ends at 00432B19..00432B2B with `[mover+1BCh]->vtable[34h](mover+CCh)`.
  That call hands the mover's own entity pose block to the node. Node virtual +34h is the
  world-matrix setter that `00B6DAE0` tail-jumps to (`docs/CAMERA_POSE_DISPATCH.md`).
- **The world update ticks the mover.** It is an entity: `00923870(0, [game+19CCh], identity)`
  places it in the world.
- **The sound listener reads the mover directly.** `docs/INTERFACE_SOUND_LISTENER.md` reads
  `[game+1ED4h]` through virtual +120h.

## 3. Which mover a ship gets: "ShipCaptain"

USN04's player unit takes interface **25h**. The host log shows `applied as 25h`. Its arm
(`docs/IN_GAME_INTERFACE_SCREEN_SETS.md`) calls `0064DA40` on `interface+7Ch` with the unit.

`0064DA40` does five things:
- **Creates the mover** once: 404h bytes, constructor `0064B650`, primary vtable `00CF5CE8`, class
  string `ShipCaptain` at 00CF5E1C.
- **Places it in the world.**
- **Sets the pitch limits** from the gameplay settings:

| mover field | value | source |
| --- | --- | --- |
| +3ECh | `settings+458h * pi / 180` | `ShipCamera.MinCameraAngle` (constants 00CE3D28 = pi, 00CE3D20 = 180.0) |
| +3F0h | `settings+45Ch * pi / 180` | `ShipCamera.MaxCameraAngle` |

- **Aims the camera at the unit** unless it already targets it: `00432E60(unit, 1)`, then refreshes
  the unit's pose.
- **Seeds the orbit, only when +3E8h is zero:**

| mover field | value | source |
| --- | --- | --- |
| +384h | `-0.0 - yaw` | yaw from `BSP_Direction_ToPitchYaw` of the unit's pose |
| +388h | -0.1745329 (-10 degrees) | 00CECA08 |
| +38Ch, +390h, +394h, +398h | `class+A0h * -2.0` | the ship's Length times 00CF60F8 = -2.0 |

It then calls `004BC410`, 0051E730 and 00549260. It copies `unit+980h`/`+984h` (throttle and
rudder) into `interface+24h/+28h`.

**The constructor `0064B650`** calls the base constructor `00519290`, then:

| mover field | value | source |
| --- | --- | --- |
| +3ECh | -0.5235988 | 00CEC728 |
| +3F0h | 1.5533431 | 00CEC400 |
| +3F4h | 0 | |
| +400h | 0 | |

**The vtable `00CF5CE8`**, against the unit-view mover `00CF6250`:

| slot | ShipCaptain | purpose |
| --- | --- | --- |
| +DCh | `00432ED0` | the per-frame update, the same slot as the unit update |
| +120h | `0042DEF0` | the matrix getter |
| +124h | `0042EAC0` | the hand-over |
| +12Ch | `0064B6B0` | |
| +130h | `0064B6D0` | |

**`0042DEF0`** (`__thiscall(mover, float out[16])`, RET 4): when `+1BCh` (the node) and `+3ACh`
(the target) are both set, it copies the 64 bytes at `mover+2F8h`. Otherwise it writes identity.

## 4. The update, `00432ED0`, read in full

`__thiscall(mover, float dt)`, RET 4, body 00432ED0..00433560. The reconstruction is
`update_ship_captain_00432ed0` in `src/mission_camera.cpp`. Every FSTP to a dword is a float
store there.

**What binding sets first.** `0064DA40` calls `00432E60(unit, 1)` before seeding.
- It stores its second argument in **+3F4h = 1**, so a ship's camera runs in mode 1: yaw fixed in
  the world, seeded from the heading.
- 0042F650 sets the target and clears the focus fields +3C4h, +3D4h and +3D8h.
- With +3E8h clear, `+3A4h = ZoomOffset * class Length`, and both sway fields are zeroed.
- The yaw seed is `-0.0 - atan2(forward.x, forward.z)` (00521370's stack output, 005213AC).

**The update, in order:**
1. **Countdown.** +3E8h counts down.
2. **Sway (00432F01..00432F81).** `t = min(2*dt, 1)`, 1 when NaN.
   `+3FCh = a + (rudder*throttle*3.0 - a)*t`, with rudder at unit+984h and throttle at unit+980h.
   `+3F8h = b + (+3FCh - b)*t`.
3. **Gate `0042DCD0(dt, mode == 0)`.** It takes its early path, zeroing +3E0h and +3E4h, unless
   +3D4h > 0. Only the focus request at vtable +11Ch (`0042EA40`) sets +3D4h. The host never makes
   that request, so the focus branch 0042DD2E..0042DEE9 is not transcribed.
4. **Death re-base, mode 0 only.** When unit+5Dh is set, the mode becomes 1 and the yaw is
   re-based through 0042D2E0 and 00438AA0.
5. **Pivot** `P = (unit.x, class CameraMinHeight, unit.z)`.
6. **Orientation +2B8h.**
   - Mode 0: 0042D5A0 writes row 2 = (forward.x, 0, forward.z) and row 1 = the world up vector
     00F8758C. That vector is (0, 1, 0), copied from 00E0B68C by the static initializer 00CD2280.
     0085DC80 then orthonormalises. Next comes `RotY(-yaw) * M` (00B646E0, 00413920), and the
     pivot is moved by `-sway * row 0`.
   - Mode 1: `M = RotY(-yaw)`.
7. **Pitch below zero.** `M = RotX(-pitch) * M` (00B64640).
8. **The offset (004331F6..0043345F).** R is row 2 of M. V is the normalised
   (forward.x, 0, forward.z) (0042B260).
   - Front = `V * (V.R)`.
   - Up = `G * (R.G)`, with G the world up.
   - Side = `R - (Up + Front)`.
   - Scaling: Front by `CameraDistanceFront * LengthMult`, Up by
     `CameraDistanceVertical * LengthMult`, Side by `LengthMult * CameraDistanceSide`.
   - `Offset = (Side + Up) + Front`.
   - `+3A4h = |(Offset.x*ZoomOffset, 0, ZoomOffset*Offset.z)|` (0042B2F0).
9. **Position.** `C = P - Offset`.
10. **Pitch at or above zero.** `M = RotX(-pitch) * M`.
11. **Output.** Row 3 of M is C, and `+2F8h = M`, the vtable +120h matrix.
12. **Zoom, `0042C610`.** +3A8h snaps to `+3A0h ? +3A4h : 0` when within 20000.0, and otherwise
    steps 20000.0 toward it. Row 3 of M then moves by `normalise(row 2 with y floored at -0.1) * +3A8h`.
13. **Publish, when +380h is set.** `+74h = M`, then `004329D0(dt)`:
    - Its shake block needs +1C0h > 0.
    - The phases +1E8h..+1F0h advance by +1DCh..+1E4h times dt.
    - `0042F0C0` runs.
    - 0042ED50 and 00414DB0 refresh the pose.
    - `node->vtable[34h](mover+CCh)`, which is `00B71460` `BSP_Camera_SetWorldMatrix` on the
      Operator camera.

**`0042F0C0`, keeping the camera out of the water.**
- T is the local translation. B is the unit's position plus its up row times `max(h, 1)`, where h
  is T's height above the unit along its up row.
- It walks the five probes at 00E08088: (0,0,0), (-1,-1,0), (1,-1,0), (1,1,0), (-1,1,0). Each is
  placed through the local rows.
- The kind from vtable +128h is 2 (0042A8E0). When the water under a probe (0078CF20) is above
  it and B is not below the water under B, the probe is lifted to that water height. Its x and z
  become the midpoint with B.
- A ray from B to the probe is then tested against the unit's collision (vtable +B0h, 0042E630,
  0098B370). T accumulates each probe's correction.

**No input.** None of these routines reads input. Orbit input would reach the mover through other
virtuals, which nothing in the host calls.

## 5. The binding

Switch `kMissionCameraBound` in `include/bsp/game_hosts_hud_world.hpp`.

- **Create and bind.** `src/game_hosts_hud.cpp` binds the mover when the 25h arm hands
  interface+7Ch the unit. `screen_receive_unit(7Ch)` stands for `0064DA40`.
- **Tick.** It ticks once per in-game interface frame and publishes into the Operator node, a
  `CameraState` held by `bsp::mission_camera_publication()`.
- **The markers.** `project_clip_space` runs `get_camera_view_projection_00b70490` and
  `transform_native_vector4_00b62d10` on that node.
- **The minimap.** `renderer_basis` runs `refresh_camera_world_00b6db70` and reads world +110h and
  +118h.
- **Inputs from Lua:**
  - ShipGlobals["ShipCamera"] ZoomOffset, LengthMult, MinCameraAngle and MaxCameraAngle through
    `read_ship_camera_settings_0083b5e0`.
  - VehicleClass[type] CameraDistanceFront, Side and Vertical, CameraMinHeight,
    CaptainCameraHeight and Length through `read_ship_class_camera_00831e0d`, then
    `ship_class_camera_00831e0d`'s fallbacks with 00CE38B8 = 10.0.

  This installation authors ShipCamera as 1.25, 1.0, -89 and 89. The destroyer class authors 250,
  150, 50 and 45. `vehicleclasses.lua` is the one locally modified datatable here.
- **Records the ON side adds:**
  - `MissionCamera::phase_draw` (00BD2F10), three times at bind.
  - `MissionCamera::collision_ray` (0098B370), five per tick.

**Labelled substitutions.**

| term | image | host |
| --- | --- | --- |
| phases +1E8h..+1F0h | three 00BD2F10 draws in 00432750 | not drawn: they would advance the shared stream the ship AI uses, and they never reach the pose |
| ray against the unit's collision (0042F4A8..0042F503) | a hit replaces the probe | no collision shape is built for a unit, so there is no hit |
| tick point | the world update ticks the mover as an entity | once per in-game interface frame, before the markers or minimap update, with that screen's delta |
| fov +1C4h | 00B71A80's 40 degrees (00CE7D20). 004DCDF0 at world construction re-applies the current fov through 004DC940. The ship view's zoom path (0064E2EE) sets `FOVs.Ship (30) * [00F889B4] * zoom scale`. | 40 degrees. The FOVs table, 00F889B4 and the zoom path are not bound. |
| aspect +1C8h | 004DCDF0 writes `[platform+10h]` | 4/3 (00D5BD98): the platform state reaches the HUD host only through `src/game_hosts.cpp`, which cc10 leases. It is 640/480 at this resolution. |
| near +1D4h, far +1D8h | 1.0 (00B71A80); 20000.0 written by 004DCDF0 (00CE3CC0) | the same values |
| node velocity +1B8h and previous position +1A4h | 004329D0's tail, for the sound listener | not modelled |
| mover world | 00414DB0 on a world-root entity | the local matrix |

## 6. Predictions, written before the pair

The pair is one tree on main `576a8065a`, built twice differing only in `kMissionCameraBound`, with
`BSP_GUNNERY_RNG_STREAMS=1`. The 25h interface is applied in the first mission frames, and from then
on the node is ready.

- **Rows that leave UNIMPLEMENTED** (as done rows, from the bind on):
  - `HudMarkers::view_projection_matrix` (00B70490), about 73,264.
  - `HudMinimap::refresh_renderer_basis` (00B6DB70), about 18,320.
  - The few calls before the bind stay records.
- **Rows added:**
  - `MissionCamera::collision_ray`: about 5 x 4,500 = 22,500 UNIMPLEMENTED.
  - `MissionCamera::phase_draw`: 3 UNIMPLEMENTED.
  - Done rows: `MissionCamera::update` and `publish_pose` (about 4,500 each),
    `MissionCamera::ocean_height` (about 45,000), `bind_ship_view` (1).
- **Total.** The unimplemented total falls by about 73,264 + 18,320 - 22,503 = 69,081 from the OFF
  side. The OFF side should be about 2,210,184, unless main moved.
- **HUD lines that move:**
  - The minimap's icon rotations and the heading, now the camera's forward instead of the ship's.
  - The mission-markers summary (`on_screen`, `collapsed`, `widgets` and the corner bounds), now
    under a perspective camera behind the ship.
  - The minimap `placed` count only if a rotation takes an icon off the page.
- **Gameplay lines expected identical:** every gameplay summary line. That covers damage, deaths,
  queued hits, releases, mission end and the RNG-coupled rows, because no draw is taken.

## 7. The pair: not run

Both launches on 2026-09-23 died before the mission with
`device_created=0 device_hr=0x80004005` (`local\cam_probe.log`, and the 120-frame probe
`local\cam_probe120.log`).

`query session` shows session 1 (the user's) **Disc**. This is the known disconnected-session state
in which no run can create a device. Per the brief, launching stopped after the probe. The switch
is **landed OFF**, and the build is tested both ways.

**The missing pair:** USN04 4500, switch off and on, on this branch, once the session is connected
again.

## 8. No Ghidra function

`ghidra proto` answers `?`. The bodies sit after INT3 padding that ends the previous function, whose
body stops short of them:

| start | exclusive end | body |
| --- | --- | --- |
| 00432ED0 | 00433560 | ShipCaptain update, vtable 00CF5CE8 +DCh; previous function 00432E60 ends at 00432ECA |
| 0042DEF0 | 0042DF7D | mover matrix getter, vtable +120h; previous function 0042DCD0 ends at 0042DEEB |
