# The in-mission camera: what produces the matrix the HUD projects with

Packet `cc9_mission_camera` (worker cc9-platform, 2026-09-23). **Part 1: the read.** The binding
(`kMissionCameraBound`) is not written yet; section 5 says what it needs.
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

## 4. The update, `00432ED0`

This is the body the binding has to reconstruct. `__thiscall(mover, float dt)`, RET 4, body
00432ED0..00433560, x87 throughout. Read so far, in order:

1. **Countdown.** `+3E8h` counts down to zero.
2. **Sway.** With a target at +3ACh, `t = min(2*dt, 1)`. `+3FCh` moves toward
   `unit+984h * unit+980h * 3.0` (rudder times throttle times the double at 00D7A2B0) by `t`, and
   `+3F8h` moves toward the new `+3FCh` by `t`.
3. **Hook.** `0042DCD0(this, dt, +3F4h == 0)`, `__thiscall`, RET 8. The target pushed at 00432F93 is overwritten by the `FSTP [ESP]` at 00432F96, so the two arguments are the delta and the flag.
4. **Death mode.** With a target and `+400h` clear: when `+3F4h` is 0 and the target's +5Dh byte
   is set, `+3F4h` becomes 1. The yaw +384h is then re-based on the target's basis angles through
   0042D2E0 and 00438AA0.
5. **Pivot.** The pivot is the target's world X and Z (+FCh, +104h) and `class+548h` for height.
6. **Orientation matrix at +2B8h.** In mode 0, 0042D5A0 builds it from the target's forward
   (unit+ECh, +F4h) and the global vector at 00F8758C (meaning not read). Rotation Y by `-yaw` (00B646E0) is
   multiplied in, and the pivot is offset by the sway `+3F8h` along the matrix's first row. In
   mode 1 it is rotation Y by `-yaw` alone.
7. **Pitch.** When `+388h` is below 0, rotation X by `-pitch` (00B64640) is multiplied in.
8. **Offset (read to 00433411; the rest is not read yet).** The target's forward is normalised
   with its Y dropped (0042B260). It is dotted with the orientation row taken at +2D8h and combined
   with the global vector at 00F8758C..00F87594. Each term is then scaled:

   | term | field | source |
   | --- | --- | --- |
   | the whole offset | settings+454h | `ShipCamera.LengthMult` |
   | the front component | class+53Ch | `CameraDistanceFront` |
   | the vertical component | class+544h | `CameraDistanceVertical` |
   | the side component | class+540h | `CameraDistanceSide` |

   The ship's `CameraMinHeight` (class+548h) is the pivot height. The final matrix at +2F8h is
   built after 00433411 through 00B64640, 00413920, 004134F0 and 0042C610.
   **Host state:** the host does not load these fields today. Neither
   `load_gameplay_tuning_settings` nor the class loader's camera keys are called by the unit
   host, so the binding must add both reads.
9. **Tail.** `004329D0` runs the shake (+1C0h, decayed by +1CCh; GlobalConfig through 00432650),
   the velocity integration (+1DCh..+1F0h), 0042F0C0 and 0042ED50. It then pushes the pose into
   the node.

**Input.** No input read was found in the part read so far. The orbit keys would reach the mover
through its other virtuals, which is where the binding must confirm that nothing reads the input
host.

## 5. What the binding still needs

1. **The rest of `00432ED0`** (00433236..0043355D) and the unread helpers `0042DCD0`, `0042D5A0`,
   `0042C610`, `0042F0C0`, `0042ED50` and `00415510`, read instruction by instruction. The
   already-recovered helpers are reused: 0042D2E0, 00438AA0, 0042B260, 0042B2F0, 00B646E0,
   00B64640, 00413920, 004134F0 and 00414DB0.
2. **The pieces of `004329D0`** that shape the pose: the shake and velocity terms. The global
   config shake fields are read by 0087D7B0 (`CameraShake`), which the host does not load.
3. **The node's projection in mission.** The constructor `00B71A80` defaults are:

   | field | value | source |
   | --- | --- | --- |
   | fov +1C4h | 0.6981317 (40 degrees) | 00CE7D20 |
   | aspect +1C8h | 4/3 | 00D5BD98 |
   | near +1D4h | 1.0 | 00D7A24C |
   | far +1D8h | 50000.0 | 00D0C5F8 |

   The mission-time writers of +1C4h/+1C8h still have to be found. The aspect ultimately comes from
   the platform's +10h, which only `src/game_hosts.cpp` (cc10's lease) carries; that is a labelled
   substitution of 4/3 at 640x480.
4. **Host plumbing.** A `CameraState` owned by the HUD world host whose transform takes the mover's
   pose each frame. The markers' `project_clip_space` and the minimap's `renderer_basis` then read
   it through 00B6DB70 and 00B70490 in place of the top-down stand-in.

## 6. No Ghidra function

`ghidra proto` answers `?`. The bodies sit after INT3 padding that ends the previous function, whose
body stops short of them:

| start | exclusive end | body |
| --- | --- | --- |
| 00432ED0 | 00433560 | ShipCaptain update, vtable 00CF5CE8 +DCh; previous function 00432E60 ends at 00432ECA |
| 0042DEF0 | 0042DF7D | mover matrix getter, vtable +120h; previous function 0042DCD0 ends at 0042DEEB |
