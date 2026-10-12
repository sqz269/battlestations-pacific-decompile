# Segment 2: the camera movers (packet cc13-net3, segment 2)

Partition segment 2 runs from `0042A920` to `0043E840`. This packet leaves out
`00435D30..0043CF9B`, the profile and Lua-text cluster that `docs/CC13_PROFILE_SAVE.md` covers.

Status vocabulary: *named* means a Ghidra rename and ledger record. *Reconstructed* means C++ in
`src/segment2_camera_cc13.cpp`, which so far is build-tested only. Nothing here is game-validated.

## Triage

`local/net3_triage.py` and `local/net3_triage2.py` work from the index and a text search of
`src/`, `include/` and `docs/`. The segment, minus the excluded cluster, holds 287 functions:
- 60 already have reconstruction records;
- 23 are EH/CRT/STL helpers;
- 29 are cited by existing C++;
- 145 had no mention anywhere.

Almost no strings point into the segment (8 references, 3 of them `vector<T> too long`). What it
holds is the **camera-mover class family**: the base class, every subclass's per-frame step, and
the vector/matrix helpers they share.

Earlier work this packet builds on:
- `docs/MISSION_CAMERA.md` and `src/mission_camera.cpp` (cc9, ShipCaptain) reconstruct the base
  publish `004329D0`, the water probe `0042F0C0`, the shake setter `0042C4C0` and the constructor
  fields of `00432750`.
- `docs/HUD_PICK_SEGMENT_QUERY.md` covers the movie camera.
- `docs/CC13_CAMERA_CONTROLLERS.md` documents `BSP_GameCameraModifier_EnsureCamera`, which
  chooses a mover class by entity kind.

This packet does not touch those files.

## Never-defined bodies created

The undefined regions were found by comparing each function's last Ghidra instruction with the
next function start (`local/net3_bodyends.py`). Each region was then swept from its first
instruction to a `RET` followed by `INT3` padding up to a 16-byte boundary (`local/net3_sweep.py`).
Seven bodies were created under the Ghidra write lock and the program was saved:

| body | references (disk scan) |
|---|---|
| 00430020..004307EF | vtable `00CE3E60` slot DCh (`00CE3F3C`); also absolute dwords in `.rdata` and at `0060E4EC` |
| 00431480..00431F13 | `CALL` at `00434F2B` (in 00434E50); absolute at `007A008C` (in `BSP_MovCam_DumpPositions`) |
| 00431F20..00431F3C | none found |
| 004335C0..004337F0 | vtable `00CEC5F0` slot DCh (`00CEC6CC`) |
| 00434140..0043480D | vtable `00CF6250` slot DCh (`00CF632C`) |
| 00434810..00434E44 | vtable `00CF4410` slot DCh (`00CF44EC`) |
| 00434E50..0043511E | vtable `00CF4238` slot DCh (`00CF4314`) |

Afterwards `_free` (`00BF9DC8`, thunks `00BF65AC`/`00BF6989`) still reads
`void __cdecl _free(void *)`, not noreturn.

**Listing defect left in place.** Ghidra's body for `00434E50` stops at `00434F4F` and resumes
at `00434FF2`. A stale code unit at `00434F52` (an `INT3` decoded mid-instruction) overlaps the
real fall-through `00434F50: CMP dword ptr [EAX+19CCh],0`. The bridge has no clear-code endpoint,
so the overlap remains. Decompiles of 00434E50 miss `00434F50..00434FF1`; read that stretch with
`bsp.py disasm-raw`.

## The class map

Each mover class has its own vtable; slot DCh is its per-frame step `(mover, float dt) RET 4`.
The class ids come from `docs/ENTITY_CLASS_IDS.md`. The kind each class is built for comes from
`BSP_GameCameraModifier_EnsureCamera` (`docs/CC13_CAMERA_CONTROLLERS.md`) and from `0067B4D0`.

| vtable | class | built by | for | slot DCh step |
|---|---|---|---|---|
| `00CE3B28` | 4Ch base | `00432750` BSP_CameraMover_ConstructBase | - | `00435410` BSP_CameraMover_StepBase (forwards to `004329D0`) |
| `00CE3E60` | 4Eh | `00435120` | INTF_BOMBVIEW, torpedo or no target | `00430020` BSP_TorpedoViewCamera_Step |
| `00CE3FD0` | 4Eh | `00435280` | INTF_BOMBVIEW, any other target | `00430DA0` BSP_BombViewCamera_Step |
| `00CEC1F8` | 53h | `005176F0` | kind 45h, MAirfield | `00433590` BSP_AirfieldCamera_Step |
| `00CEC5F0` | 56h | `00519490`, `005198F0` | not established | `004335C0` (provisional name) |
| `00CF6250` | - | `00650830`, `00651370` | kind 8, MSubmarine | `00434140` BSP_SubmarineCamera_Step (calls `00433800`/`00433CB0`, which read `ShipCamera.LengthMult`) |
| `00CF4410` | 4Ch | `006065D0` | kind 0Fh, planes | `00434810` BSP_PlaneCamera_Step |
| `00CF4238` | 51h | `00605130` | any other kind | `00434E50` BSP_DefaultUnitCamera_Step (calls `00431480`, which calls `0042F800`, which reads the `PlaneCamera/*` and `BombCamera/BombSub*` tunables) |
| `00CF5CE8` | 50h | `BSP_ShipCaptainCamera_Construct` | kind 6, ships | cc9's `docs/MISSION_CAMERA.md` |

`00CE3E60` and `00CE3FD0` both answer class 4Eh (`docs/ENTITY_CLASS_IDS.md` noted the
ambiguity). They are the two INTF_BOMBVIEW cameras, built by `0067B4D0`. That function is called
on the interface's `+8Ch` by the 26h arm (`docs/IN_MISSION_INTERFACE_MANAGER.md`):
- **Torpedo view.** With no target, or a target of kind 2Bh (MTorpedo), it allocates 3CCh bytes,
  calls `00435120` (stored at `+50h`), then `0042FEE0(target)`.
- **Bomb view.** Otherwise it allocates 3F8h bytes, calls `00435280` (stored at `+4Ch`), then
  `004307F0(target, launcher)`.
- Either way `BSP_Game_SetCameraMover` installs the camera.

The bomb view embeds a tick element at `+170h` (vtable `00CE3F94`). Its slot `+4h` is `00430970`
(BSP_BombViewCamera_TickElementSlot4), which reads the `BombCamera` follow-distance tunables.

## Reconstructed

All of these are in `src/segment2_camera_cc13.cpp`; calls into other subsystems go through
`WeaponViewHost`.

- **`00435120` / `00435280`, the constructor tails.**
  - Both run `00432750`, set their vtables (the bomb view also sets `+10h` `00CE3FB4`, `+24h`
    `00CE3FAC` and `+170h` `00CE3F94`), and embed the target observer at `+384h`. The observer
    has vtable `00CE3CF0`, `+4h..+Ch = 0`, `+10h = 1`, and its `+14h` is the target, which is
    camera `+398h`.
  - The bomb view also zeroes `+3F0h`/`+3F4h`.
- **`0042FEE0` (torpedo, RET 4).**
  - On a target change it unregisters the old pair (`006952A0`, ECX = old target, EDX =
    observer) and registers the new one (`00694A60`).
  - It copies the world camera node's matrix into the camera: node `[game+19FCh]`, `+F0h`, through
    `004134F0`, after `00B6DB70` when bit 2 of node `+5Ch` is clear.
  - It saves the start position, forward and up at `+3A8h`/`+3B4h`/`+3C0h`.
  - `+39Ch` gets the target's world position, or the camera position when there is no target.
- **`004307F0` (bomb, RET 8).** The same observer update, then `+3D8h = 1.0`. With a target it
  also:
  - copies the node matrix;
  - sets `+3CCh` to `launcher->vtable[34h]()` (the launcher is not null-checked) and `+3DCh` to
    the BSS vector `00F87574`;
  - saves forward and up at `+39Ch`/`+3A8h` and the position at `+3C0h`;
  - sets `+3E8h = 0.1` and `+3ECh = 0.22`.

  It always zeroes `+3F0h`/`+3F4h`.
- **`00430DA0`, the bomb view step.** blend = `dt * tuning+E8h` (`BombCamera/CameraPosSmooth`).
  1. The target point `+3B4h` follows the target's world position. It is kept at least 3.0 high
     (`00CE3854`), unless the target is a running torpedo (kind 2Bh with `[+314h]+F4h > 0`) that
     is already at or below -15.0 (double `00CE3D58`).
  2. The wanted point is the target point plus the offset `+3DCh`.
  3. The camera position `+3C0h` drifts by `dt * +3CCh`. With a running torpedo and a wanted
     point below 0, its height is capped at -3.0 (`00CE3D50`).
  4. The forward `+39Ch` blends towards the unit vector to the wanted point. The distance is 0
     when the square is at most 1e-10, and the division is not guarded.
  5. The up `+3A8h` blends towards (0, 1, 0) at blend / 3.0.
  6. Rows 2/1 get forward and up, then `0085DC80` orthonormalizes; row 3 gets the position.
  7. `0042D610` orbits the pose about world up by `+3F0h`, then about row 0 by `+3F4h`, both
     through the wanted point.
  8. `+C8h` and `+10Ch` are cleared, the children are invalidated (`0042ED50`), `00414DB0`
     refreshes the camera, and `[+1BCh]->vtable[34h](&camera+CCh)` publishes.

  The symbolic trace (`tools/x87_sym.py 00430DA0`) gives the float spill points the C++ keeps.
- **`0042D610` (RET 0Ch).** `len = 00419440(axis)`. The axis is divided by `-len`, and
  `0085E8C0(matrix, point, &unit, len * angle)` rotates the matrix about that axis through the
  point. With the unit axes the step passes, this is a rotation by `angle` about `-axis`.

## Named only

`00432750`, `00435410`, `00430020`, `00430970`, `00433590`, `004335C0`, `00434140`, `00434810`
and `00434E50`, with the evidence in the table above.

## Handoff

Totals: 7 bodies created, 15 Ghidra renames with evidence (14 class, constructor and step names, plus
`0042D610`), 6 reconstruction records, build-tested only. Tools: `local/net3_*.py` in
the cc13-net2 worktree (triage, body ends, sweep, rel32 scan, vtable compare).

Next, in order of value:

- `00430020`, the torpedo view step (2000 bytes). It uses segment queries
  (`BSP_SpatialIndex_QuerySegment`) and the initialized `.data` tunables at
  `00E08074..00E08084`.
- `00430970`, the bomb view tick slot.
- The other classes' steps: `00433590`, `004335C0`, `00434140`, `00434810`, `00434E50`,
  `00431480`, `0042F800`, `00433800`, `00433CB0`.
- The 4Ch base slots (`0042B8xx..0042BBxx`, mostly small accessors), and the math helpers with
  callers outside the segment: `0042B570`, `0042C1B0`, `0042C750`, `0042C940`, `0042E180`.
- The overlap at `00434F52`.


## Continuation by cc13-render5 (2026-10-12 06:20Z)

cc13-net3 retired; cc13-render5 took the handoff above (tree `cc13-render4`). New code is in the
same two files and is **build-tested only**.

### `00430020` BSP_TorpedoViewCamera_Step (reconstructed)

`__thiscall(camera, float dt)`, RET 4. Read with `tools/x87_sym.py 00430020` on both target
branches (`--out 0042B260=c:0c`). The `.data` tunables, read only here: `00E08074` 0.03 (sub-step),
`00E08078` 0.08 (x/z blend per full sub-step), `00E0807C` 0.04 (height blend), `00E08080` 30.0
(trail distance), `00E08084` 3.0 (height).

- Aim point. With a target: `+39Ch` = its world position; the target's world forward row x/z
  (`+ECh`, `+F4h`) flattened and normalized (`0042B260`) gives aim = position - 30 * forward, and
  the aim height is 3.0 when that is below 0, else 3.0 above it. Without a target: the camera's own
  flattened forward times 100.0 (`00D7A220`) behind `+39Ch`, height 30.0 (`00CE38C8`). The listing
  at `0043019A` is `DC C9` (FMUL ST1,ST0), so both products use 100.0; Ghidra's pseudocode
  hides the normalized values entirely.
- `dt` is consumed in sub-steps of at most 0.03 (`while dt > 0`, `dt -= 0.03` stored to float).
  Per sub-step, t = 0.08 * step / 0.03:
  - a segment query (`BSP_SpatialIndex_QuerySegment`, `0098ADD0`) from the eye's x/z at height 3.0
    to `+39Ch`; when it hits a part, aim height = max(aim height, part `[+24h]+4h` + hit entity y +
    4.0). The raise persists for later sub-steps;
  - smoothed eye `+3A8h` eases to the aim (x/z by t, y by 0.04), the pose position eases to it
    likewise;
  - the look direction (target position with height +3.0 when the target is at or above 0, else
    3.0, minus the new position, normalized) is eased into `+3B4h` and then into the forward row;
    `+3C0h` eases to (0,1,0) and the up row to it; every ease is three float spills; then
    `0085DC80` orthonormalizes.
- Always: `+C8h`/`+10Ch` cleared, children invalidated, published (as the bomb step).

### `00430970`, renamed BSP_BombViewCamera_UpdateVelocity (reconstructed)

ECX is the bomb view's `+170h` element, so element `+228h/+244h/+250h/+25Ch/+268h` are camera
`+398h` (target), `+3B4h` (target point), `+3C0h` (position), `+3CCh` (velocity) and `+3D8h`
(blend, set to 1.0 by `004307F0`). Ground height under the target point
(`BSP_World_GroundHeightAt`) is floored at 8.0 (`00CE3918`); the follow distance is
`BSP_Math_InterpolateClamped(0, FollowDistFar, NearAlt/1, FollowDistNear, height)` with the
Torpedo (kind 2Bh), Rocket (33h) or Bomb (anything else; the 2Ah query's result is unused) row of
the BombCamera tunables, or FinalDist (`+138h`) without a target. The aim is the target point plus
1.6 (`00CE3D48`) x the target's velocity (vtable `34h`). The wanted velocity is the unit vector to
the aim times (length - distance) / 1.6 (unguarded division when the length is 0); without a
target it is damped by 0.8 and its y is raised to at least half its horizontal speed. Velocity =
blend x old + (1 - blend) x wanted; the blend drops by CamVelBlenderAcceleration (`+F0h`) while it
is above MaxCamVelBlender (`+F4h`).

### Other per-class steps

| address | name | status |
|---|---|---|
| `00433590` | BSP_AirfieldCamera_Step | reconstructed: when byte `+380h` is set, pose = matrix `+2B8h`, then `004329D0(dt)` |
| `004335C0` | **BSP_BomberViewCamera_Step** (renamed from the class-56 placeholder) | reconstructed (host matrix ops). The override globals `00E17670` (flag), `00E17674` (pitch), `00E17678` (yaw) are written only by the Lua binding at `008A0850` ("luaMW_PlayerSetBomberView failed:"), so class 56 is the player's bomber view. Orbit with eye height +10.0 (`00CE3DC0`) |
| `00434140` | BSP_SubmarineCamera_Step | named earlier; selects `00433800` (`+40Ch == 0`) or `00433CB0(dt, flag)`, now named StepMode0/StepMode1 (structural) |
| `00434810` | BSP_PlaneCamera_Step | not read in this packet |
| `00434E50` | BSP_DefaultUnitCamera_Step | not read; its callee `00431480` is now BSP_DefaultUnitCamera_UpdateChase and `0042F800` BSP_UnitCamera_EvaluateChaseTunables (both provisional, not reconstructed) |

### Math helpers

| address | name | status |
|---|---|---|
| `0042B570` | BSP_ContactPointList_Append | reconstructed (8 x {point, normal}, count `+C0h`) |
| `0042C1B0` | BSP_RandomWander_Retarget (provisional) | named only. It draws four times from the shared gameplay RNG `00BD2F10`, so a host wiring it in must follow the cc13 RNG rule (separate presentation stream or OFF switch); callers `0042DAF0` and the plane code `007C7800` |
| `0042C750` | BSP_CameraMover_AnglesWithinTolerance (provisional) | named only |
| `0042C940` | BSP_CameraMover_OrbitTargetFromYaw (provisional) | named only; callers in segment 16 |
| `0042E180` | - | not named: gated on byte `+1F4h`, calls `0042D0D0`, `00B6DB70`, `00B6FC30`; callers are the two otherwise-unreferenced thunks `0042E3D0`/`0042E460` |

### Handoff (cc13-render5)

Remaining, in order: `00434810` (plane step, 1.6 KB, calls `0042DAF0` which uses
`0042C1B0`'s RNG), `00434E50` with `00431480`/`0042F800` (read `00434F50..00434FF1` with
`disasm-raw`; the misaligned instruction at `00434F52` is still unrepaired), `00434140` with
`00433800`/`00433CB0`, then `0042C750`, `0042C940`, `0042E180`. All five provisional names above
should be revisited when their bodies are read.


## Continuation by cc13-camera2 (2026-10-12)

cc13-render5 retired; cc13-camera2 took its handoff. All new code is in the same two files,
**reconstructed and Win32 build-tested only** (no fixture, no original-ABI check, not game-validated).
x87 was read with `tools/x87_sym.py` and checked instruction by instruction against the listing where
branches move values between the FPU stack and spilled slots. Ghidra renames applied through
`tools/ghidra_annotate.py` (prior values in the tree's `local/ghidra-annotations-20261012T065507Z.json`).

| address | name (Ghidra / ledger) | C++ | notes |
|---|---|---|---|
| `00434810` | **BSP_PlaneCockpitCamera_Step** (was BSP_PlaneCamera_Step) | `plane_cockpit_camera_step_00434810` | below |
| `0042DAF0` | BSP_RandomWander_Advance (new) | `random_wander_advance_0042daf0` | below |
| `0042C090` | BSP_RandomWander_Evaluate (new) | `random_wander_evaluate_0042c090` | below |
| `0042C1B0` | BSP_RandomWander_Retarget | `random_wander_retarget_0042c1b0` | RNG, below |
| `00434E50` | **BSP_PlaneChaseCamera_Step** (was BSP_DefaultUnitCamera_Step) | `plane_chase_camera_step_00434e50` | body read past 00434F52 with `disasm-raw` |
| `00431480` | **BSP_PlaneChaseCamera_UpdateChase** (was BSP_DefaultUnitCamera_UpdateChase) | `plane_chase_camera_update_chase_00431480` | below |
| `0042F800` | **BSP_PlaneChaseCamera_UpdateOffsets** (was BSP_UnitCamera_EvaluateChaseTunables) | `chase_camera_update_offsets_0042f800` | below |
| `0042B6E0` | BSP_Matrix_PremultiplyYawThenPitch (new) | `rotate_yaw_then_pitch_0042b6e0` | m = RotY(-0-yaw) x m, then RotX(-0-pitch) x m |
| `00434140` | BSP_SubmarineCamera_Step | `submarine_camera_step_00434140` | below |
| `00433800` | **BSP_SubmarineCamera_PlaceSurfaced** (was StepMode0) | `submarine_camera_step_mode0_00433800` | below |
| `00433CB0` | **BSP_SubmarineCamera_PlaceSubmerged** (was StepMode1) | `submarine_camera_step_mode1_00433cb0` | below |
| `0042D5E0` | BSP_Matrix_SetForwardOrthonormalize (new) | `set_forward_orthonormalize_0042d5e0` | row +20h = v, 0085DC80 |
| `0042C750` | **BSP_CameraMover_TurnTowardAngles** (was AnglesWithinTolerance) | `camera_mover_turn_toward_0042c750` | below |
| `0042C940` | BSP_CameraMover_OrbitTargetFromYaw | `orbit_target_from_yaw_0042c940` | below |
| `0042E180` | BSP_CameraMover_BuildFrustumPlanes (new) | `mover_build_frustum_planes_0042e180` | below |

Reused, not duplicated: `normalize_camera_basis_0042b260`, `force_event_vector_length_0042b2f0`,
`dive_bomb_interpolate_clamped_00419010`, `max_native_float_by_ref_00415550`,
`wrapped_angle_add_00438aa0` / `wrapped_angle_subtract_00438b10`. Matrix builds/multiplies,
orthonormalize, direction transforms, entity/node pose refreshes and the tuning/settings singletons are
host calls.

### RNG: the random wander (cc9 deterministic pairs)

`0042C1B0` is reached from the cockpit step through `0042DAF0` (three records per step) and from the
plane code `007C7800` (not read here). Every draw is `00BD2F10` BSP_Random_UniformFloatRange with
`XOR ECX,ECX`, i.e. the **shared gameplay stream 0**. Per retarget, in order:

| site | draws | condition | use |
|---|---|---|---|
| `0042C1F9` | U(+40h, +44h) | only when +40h != +44h (FUCOMIP; NaN draws) | period +28h (else +40h) |
| `0042C23D` | U(+34h, +38h) | only when +34h != +38h | radius (else +34h) |
| `0042C26C` | U(0.5 `00CE3800`, 5.7831855 `00CE3CB0`) | always | azimuth step, added to +3Ch with 00438AA0 |
| `0042C2A6` | U(0, 3.1415927 `00D7A264`) | always | elevation |

So a retarget draws 2 to 4 values, and `0042DAF0` retargets once per elapsed period while the budget
+2Ch lasts (possibly several times in one step, none once elapsed passes the life +30h). The C++ takes a
caller-supplied `WanderRandom`; it never touches stream 0. A host that wires the cockpit camera in must
give it a separate presentation stream (or keep it behind an OFF switch), per the cc13 RNG rule.

Record (48h): +0 start, +0Ch target = r (sin az sin el, cos az sin el, cos el), +18h output, +24h
elapsed, +28h period, +2Ch budget, +30h life, +34h/+38h radius range, +3Ch azimuth, +40h/+44h period
range. Evaluate (`0042C090`): output = target + (1 - t^2)(start - target), t = min(elapsed / period, 1),
then times 1 - u^2, u = min(elapsed / life, 1) (both clamps take 1.0 for NaN).

### `00434810` the plane cockpit camera

The tuning copy at entry (`0042E740()+84h`, 13 dwords) is read only at `+ACh`,
PlaneCamera/CockpitMaxHeadMoveDist, which is why the class is taken to be the cockpit view (provisional;
the tuning doc's consumer column did not list this function). With the counted reference +3C0h set
(released through `0042D9A0(&+3C0h, 0)` when the plane's byte +44h is set): advance the three wanders;
head move +3DCh = their sum scaled by r / InterpolateClamped(0.7, 0.7, 2.0, 1.2, r), r = |sum| /
CockpitMaxHeadMoveDist; when dt != 0 (NaN counts) ease the three 14h-byte angles (current += dt * rate /
period * (target - current)); pose = the plane's world matrix (+F0h), eye moved by the 0042D0D0
direction transform of +3C4h + head move; rotate about the eye by Y(+38Ch), X(+3A0h), Z(-+3B4h), each only
when |angle| > 1e-4 (double compare), then the +514h modifier (vtable 8Ch gate, 94h matrix); the look
frame +3E8h = plane matrix with position + transform(+3D0h + head move). Always: +2B8h = pose, 004329D0.

### `00434E50` / `00431480` / `0042F800`: the chase camera

`00434E50` (vtable 00CF4238): optional `[mover+170h]->vtable[4]([game+21ECh])` when `[game+635h]`;
timer +3ECh -= 0.08 dt (floor 0); target state +900h == 6 clears +3F0h; target position into +38Ch when
+911h is clear; `00431480`; ground clamp **inside the stretch Ghidra misses** (00434F50..00434FF1:
y >= ground + 5.0, written back to +2E8h); pose = chase; when +380h: below height 10, floor
max(water + 2, 2) (0078CF20), publish, and the sink +1B8h gets the target's vtable[34h] vector;
+3F8h counts down.

`00431480` runs sub-steps of min(rest, 0.01) and keeps going while rest - 0.01 >= 0 (so dt = 0.025
runs three passes). Live target (non-null, +5Dh and +911h clear): `0042F800(step)` first, chase = target
pose +CCh, `0042B6E0(+3D0h, +3CCh)`, RotZ(-0 - +3B0h), position += right x +3B4h + up x (+3B8h +
[class+16Ch]) - forward x (+3BCh + [class+168h] / +384h) (right/up from the target, forward from the
rotated chase), `0042B6E0(+3E0h, +3DCh)`. Otherwise (+3E4h/+3E8h zeroed): within 20 of +38Ch the eye is
put at 20; forward eases to the unit direction at 4.25/s, up to world up at 1.5/s; with a non-null
target whose +911h is clear (one with +5Dh set lands here) the eye moves by dir x step x speed
(vtable 38h) x +3F4h (+3F4h eases to 0.25 at 0.1/s) and is pushed out by (d - 60) x step when
horizontally closer than 60; with no target it drifts sideways 5/s along the right row and the push-out
radius is 100; height pulled toward 45 at (45 - y) x step when below. 0085DC80 each pass.

`0042F800` reads the target's rotation rates +B50h (roll) / +B54h (pitch) / +B58h (yaw), +B28h, +900h,
+970h, +9B4h and `[class+200h]`, so its target is a plane: 00CF4238 is the plane chase camera (settled in the follow-up below). Wanted offsets: +39Ch = ZRotMul x (-roll), +3A0h =
XDistMul x yaw - YDistRollMul x (-roll), +3A4h = -YDistMul x (-pitch) + +3C0h, +3A8h = ZDistMul x
[+B28h], each eased at its smooth rate x dt, then +3B0h..+3BCh eased toward them at the same rates.
States 4/5 below height 3.0 raise +3ECh to InterpolateClamped(1.0, 0.6, 4.0, 0, height) x
[class+200h]. Without flag +3F0h: +3C0h = 0, +3D4h/+3D8h decay at 3/s, +3C4h eases to -+3E4h along the
short arc (LookAroundSmoothRate) and is fmod-wrapped into [-pi, pi], +3C8h eases to -m (m = +3E8h biased
by +3ECh). With the flag and +970h != 12h, when +3E8h in degrees is at most BombSubDistLimit: mode 10
picks TorpedoSubDist/Angle (or the "2" pair when vtable[5Ch](16h)) for +3C0h and +3D8h, any other mode
sets +3C0h = -min(BombSubDistMin, (limit - degrees) x BombSubDistMul). Tail: +3DCh/+3E0h ease to
+3D4h/+3D8h at 3/s; +3CCh becomes +3C4h - kl x w (w = wrapped +3C4h - +3CCh, kl = dt x
LookAroundSmoothRate), fmod-wrapped: the listing computes w + (0 - w) x kl + +3CCh, so the yaw lands kl x w
**short** of the wanted value rather than easing by kl x w (literal, flagged); +3D0h eases to +3C8h.

### `00434140` the submarine camera

`0042DCD0(dt, 1)`; latch +405h (and +408h = chase y) once the target is null or its +5Dh set;
+3E8h counts down. Mode +40Ch 2 (periscope): pitch clamped to +-0.2094 rad (12 deg), chase = (RotX x
RotY) x target pose, eye = node +3F4h's world point +3F8h with y + 2 (x 1.1 when above 0). Modes 0/1:
h = [target+1204h] + 2 (at least -8); surfaced -> submerged when h - 0.5 > target y, back when
target y > h + 0.5. Submerged: `00433CB0(dt, 1)`. Surfaced: `00433800`, and below h + 2.5 the surfaced
and `00433CB0(dt, 0)` poses are blended by t = InterpolateClamped(h + 0.5, 0, h + 2.5, 1, y): eye and the
point 150 ahead are mixed, the forward is the normalized difference (0042D5E0, then 0085DC80). Then
0042C610. After a latch the chase y is floored at -125 (latched height <= 0) or max(latched, 3). When
+380h: pose = chase, 004329D0.

Both placement helpers split the chase forward F into its part along the target's flattened heading,
its world-up part (`00F8758C`) and the rest, scaled by `[class+53Ch]`, `[class+544h]`, `[class+540h]`
times ShipCamera.LengthMult (`[00424C40()]+454h`), and subtract the sum from the anchor. Surfaced
anchor y = target y - 0.3 x forward.y x [class+A0h] + [class+538h]; negative pitch is applied before the
offset (the eye follows it), non-negative after (look only). Submerged: pitch >= target y / 50 (stored
only when the flag is set), both rotations first, y + min([class+538h], 10) then capped at -3.0.

### Mover helpers

- `0042C750` turns +384h/+388h toward (yaw, pitch) by at most the two steps, scaling the axis with the
  smaller error so both arrive together; an axis within its step snaps; returns 1 only when both snap;
  does nothing when both steps are below 0.001. Ghidra had split the pitch-step tail 0042C8F3..0042C931
  into a bogus `FUN_0042C8F3` (no callers, reached by the JBE at 0042C8D8); repaired, see the follow-up.
- `0042C940`: pitch = clamp(pitch, +3ECh + (+3F0h - +3ECh) / 4, 0); chase = RotX(-0.075) x
  RotY(-(+384h + offset)); eye = target - 2 x +38Ch x forward; RotX(-0 - pitch); 0042C610; +400h = 1.
- `0042E180` (once, byte +1F4h): local planes at +258h: near (0, 0, -1, node+1D4h), far (0, 0, 1,
  300 / [[game+19FCh]+1C4h]), top/bottom (0, +-cos v, -sin v, 0) with v = node+1C4h / 2, sides
  (+-cos h, 0, -sin h, 0) with h = 00B6FC30 / 2; world planes at +1F8h: 0042D0D0 normal, d = n . node
  position + local d.

### Follow-up (cc13-camera2, 2026-10-12)

- **0042C750 repaired.** `tools/ghidra_define_function.py 0042c750 0042c934 --recreate --delete-inside`
  under the write lock deleted the bogus `FUN_0042C8F3` and re-created `BSP_CameraMover_TurnTowardAngles`
  over 0042C750..0042C933 (name, plate comment and the default signature kept; event log in the tree's
  `local/cm2_define_0042c750.json`). The decompile now shows the pitch step (Add/SubtractWrappedAngle
  on +388h). `_free` (00BF9DC8) still reads `void __cdecl _free(void * _Memory)`, body ..00BF9E55.
- **Class settled.** `BSP_GameCameraModifier_EnsureCamera` (0079CBD0; src/cc13_camera_controllers.cpp,
  docs/CC13_CAMERA_CONTROLLERS.md) builds kind 0Fh (planes) with 006065D0 when the fps byte is set and
  00605130 otherwise; the only other caller of 00605130 is 00605E50 in the segment-24 plane code. So
  vtable 00CF4238 is the plane chase (third-person) camera, not a generic default, and 00CF4410 the
  cockpit. Renamed: 00605130 BSP_PlaneChaseCamera_Construct, 006065D0 BSP_PlaneCockpitCamera_Construct,
  00434E50 BSP_PlaneChaseCamera_Step, 00431480 BSP_PlaneChaseCamera_UpdateChase; the C++ types and
  functions follow (`PlaneChaseCamera`, `plane_chase_camera_step_00434e50`, ...). The class-map row
  "any other kind" above is superseded.
- **Call-site report** `reports/cc13_camera2_calls.json`: 152 direct CALL rows over the 15 functions,
  `tools/verify_report_calls.py` 0 failed. The one call in 00434F50..00434FF1 (00434F8D ->
  00903860) is listed separately: live Ghidra puts no function over that stretch because of the stale
  unit at 00434F52, which stays for the user's CodeBrowser pass.

Remaining open: 00434F52; `007C7800` (plane code) also calls `0042C1B0` on stream 0; the literal
`+3CCh` "short by kl x w" yaw update in `0042F800` wants an in-game check.
