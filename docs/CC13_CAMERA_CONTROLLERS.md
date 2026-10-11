# Movie-camera keyframe modifiers (packet `cc13_camera_controllers`)

Segment 45 (0078D880..007A4860), worker cc13-camera1, 2026-10-11. Source:
`include/bsp/cc13_camera_controllers.hpp`, `src/cc13_camera_controllers.cpp`.

Every name here is a hypothesis, not a recovered symbol. Evidence is the listing (Ghidra export
or Capstone over the disk image, cited by address), the strings each parser pushes, and the
vtables in `.rdata`.

## 1. What was already covered, and what this packet adds

The segment's call spine is the new movie camera (`MovCamNew`): keyframe defaults 00799B00,
the Lua parse 007A0EB0, add/insert 007A42C0/007A2CD0, the step 007911E0, begin 00791020,
weight 00795C10, position 00795650, track evaluation 00798130 and the 570h-byte mover
0079D020/0079A3B0. Those are reconstructed in `src/hud_movie_camera.cpp` (cc9, packets
`cc9_movie_camera_*`, last change 2026-09-27) and documented in `docs/HUD_PICK_SEGMENT_QUERY.md`
sections 8 and 10. That work lists the keyframe `modifier` key as unsupported
(`HUD_PICK_SEGMENT_QUERY.md` 10.2: "`goaround` ... left: a packet of its own"; "`gamecamera` /
`fpscamera` ... left"). This packet reads those modifiers. The keywords the partition tool
attached to the segment (`thetalinearblend`, `rholinearblend`, `theta`, `blendtime`, `postype`)
are theirs: `blendtime` and `postype` belong to 007A0EB0 (already covered), the rest to the
modifier parsers 007A3100 and 007A3B20.

`src/camera_position_modes.cpp` (00794070, 00794130, 007954A0) and
`src/native_vehicle_camera_array.cpp` (0078E230) also predate this packet. Nothing here
duplicates them; `grep` of `src/` and `include/` for every address below found no earlier body.

## 2. The modifier class family

Each keyframe (`MovieKeyframe`, 104h bytes) owns at most one modifier at `+E4h`.
00799DE0 `BSP_MovieKeyframe_SetModifier` deletes the old one (slot `+0`, flag 1), stores the new
one, calls its slot `+4h` with the keyframe and reads its type id (slot `+30h`).

Four 13-slot vtables, read from the disk image:

| slot | base 00D04414 | type 1 00D04448 | `goaround` 00D04C8C | `gamecamera` 00D04CC0 | caller |
| --- | --- | --- | --- | --- | --- |
| +0 | 00791620 | 0078FBF0 | 0079E310 | 0079E330 | deleting dtor |
| +4 | 0078E780 | 0078E780 | 0078E780 | 0078E780 | SetOwner, 00799DE0 |
| +8 | 0078E6C0 `ret 8` | **007A0340** | 0078E810 `ret 8` | 0078E820 `ret 8` | step(elapsed, input*), 007911E0 at 00791295 |
| +C | 0078E6D0 `ret 0Ch` | 0078E6D0 | **00791C90** | **00795EE0** | evaluate(elapsed, pose*, offset), 00795C10 at 00795CB7 |
| +10 | 0078E6E0 | 00791640 | 0078E6E0 | 0078E870 | track tick(dt), 00797E40 |
| +14 | 0078E6F0 | 0078E6F0 | 0078E6F0 | 0078E8A0 | key tick(dt), 0078E630 |
| +18 | 0078E700 `ret 8` | 007916C0 | 0078E700 | 0079CF80 | begin(camera, t), 00791020 at 007910BF |
| +1C | 0078E710 | 0078E710 | **007A3100** | **007A3B20** | ParseLua(table), 007A0EB0 at 007A1BB2 |
| +20 | 0078E720 `xor eax` | 0078E720 | 0078E720 | 0079DD00 | camera getter |
| +24 | 0078E730 | 0078E730 | 007918B0 | 00792710 | write bits, 00791360 at 0079151D |
| +28 | 0078E740 | 0078E740 | 007A2690 | 007A2A80 | read bits, 007A2370 |
| +2C | 0078E750 `ret` | 0078E750 | 0078E750 | 0078E750 | (no caller read) |
| +30 | 0078E760 (0) | 0078E800 (1) | 0079DC30 (2) | 0079DCF0 (3) | type id |

The slot callers were found by sweeping `.text` for a `+E4h` load followed by a virtual call
(a Capstone sweep of the whole `.text`, 60-instruction window, which can miss a caller that keeps
the modifier in a register longer); for slot `+Ch` it found one caller, 00795CB7 in 00795C10, which passes
`(a1 - key start, &pose, a2)`. `docs/HUD_PICK_SEGMENT_QUERY.md` 10.2 says goaround runs through
slot `+8h` each evaluation; that slot is a `ret 8` in goaround's table, and its work is in slot
`+Ch`.

Creation: 007A0EB0 compares the `modifier` string (007A1ACD..007A1B78) with `goaround`
(64h bytes, 007A0650), `gamecamera` (7Ch, 007A06D0(0)) and `fpscamera` (7Ch, 007A06D0(1)); the
two camera kinds also set keyframe `+CCh = 0` and `+25h = 1`. It then calls 00799DE0 and slot
`+1Ch` with the table that holds the `modifier` key. 007A2370 (`BSP_MovieKeyframe_ReadBits`)
reads a 4-bit type and builds type 1 (0078E790, 2Ch bytes), 2 or 3, then calls slot `+28h`.

## 3. Curves and the blend

All curves are `vector<pair<float,float>>` (10h-byte vector, first pointer at `+4h`) of
(time, value), appended by 007A0A30 `BSP_ModifierCurve_PushBack` in Lua order and never sorted.

**Parse shape** (007A3100, 007A3B20): a number `v` is one key `(0, v)`; any other non-nil value is
read as a table: `(0, t[1])`, then `(t[2], t[3])`, `(t[4], t[5])` ... while `t[2k]` is non-nil.
Values go through 00B66270 (`lua_tonumber` spilled to float32). Angles (`theta`, `rho`) are then
multiplied by the double 00D046D8 = 0.01745329238474369 and stored as float (007A334F FMUL).

**Scan** (00791CE2..00791DC6, the same five times in 00791C90 and twice in 00795EE0): `prev`
starts as (0, start value). For each key in order, if `t < key.time` the scan stops (FCOMIP /
JC); otherwise `prev = key`. On a stop, `f = (t - prev.time) / (key.time - prev.time)` (FSTP
00791DA3) and `next = key.value`; when no key stops it, `f` and `next` stay 0 so the value holds
the last key.

**Smooth blend** (radius, theta, rho and both gamecamera angles):
`s = ease(f)` with `ease` = 0078FCF0 (0 below 0, 1 above 1, `2f^2` below 0.5, else
`1 - 2(1-f)^2`, each square stored to float). Then `w = s(1-L) + fL` (FSTP, 00791E66) and
`value = w*next + prev*(1-w)` (FSTP, 00791E90). `L` is the curve's `*linearblend` weight; `f`
in the `fL` term is the unclamped fraction.

**Linear blend** (zoom, patterntime): `value = (1-f)*prev + next*f` with `f` unclamped
(0079238D..007923A7), start value 1.0 (00D7A24C).

## 4. `goaround` (type 2)

Layout (007A0650 constructor, 007A3100 parse): `+4h` owner keyframe, `+8h` `radiuslinearblend`,
`+Ch` `thetalinearblend`, `+10h` `rholinearblend` (all 0.6, 00CE3D30), curves `+14h` `radius`,
`+24h` `theta`, `+34h` `rho`, `+44h` `zoom`, `+54h` `patterntime`. The three weights are set only
when the key is a number (007A3A3D IsNumber).

00791C90 `BSP_GoAroundModifier_Evaluate`, RET 0Ch: `t = max(a1 + a3, 0)` (00791C93..00791CC0);
radius starts at 100.0 (00CE3D08), theta and rho at 0. Then (007921E6..00792262, each FSIN/FCOS
result spilled to float):

```
key.work_40.x = -sin(rho) * radius * cos(theta) + key.local_34.x
key.work_40.z =  cos(rho) * radius * cos(theta) + key.local_34.z
key.work_40.y =  sin(theta) * radius             + key.local_34.y
```

`rho` is the heading about Y and `theta` the elevation; the keyframe's authored position is the
orbit centre. When the zoom curve has keys, keyframe `+C8h` (the zoom 007911E0 copies from
`+C4h` each step) is overwritten; when the patterntime curve has keys, keyframe `+D0h` is
(`smooth_d0` in `hud_movie_camera.hpp`; its meaning as the smoothing time is cc9's reading).

Bits (007918B0 / 007A2690): weights `+8h`, `+10h`, `+Ch` as numeric floats (zero flag 0, signed
0, scale 1.0, 8 bits); each curve as an 8-bit count and per key (time: 0, 1, 256.0, 18 bits)
and (value: 0, 1, scale, 18 bits), scale 10000.0 for radius, 100.0 for the rest. The reader
appends without clearing.

## 5. `gamecamera` / `fpscamera` (type 3)

Layout (007A06D0, 007A3B20, 00795EE0): `+4h` owner, `+8h` camera mover, `+Ch` previous t
(-1.0, 00D7A260), `+10h` `thetalinearblend`, `+14h` `rholinearblend` (0.8, 00CE74F8), `+18h`
theta curve (`theta`, else `Vlook`), `+28h` rho curve (`rho`, else `Hlook`), `+38h` an observer
(vtable 00CE3CF0) whose `+14h` (`+4Ch`) is `targetentity.Ptr`, `+48h` byte 1, `+50h`/`+54h`
entity counts, `+58h`/`+68h` vectors of 18h-byte observers, `+78h` the fps byte.

**Entity keys.** A curve value given as `{Ptr = entity}` becomes `count + 10000.0` (FILD +
FADD double 00CE4BD8, 007A3FF9) and the entity is appended as an observer (004BEF00, 007A2DB0).
When the value is a table without a userdata `Ptr`, 007A407F pushes `[ESP+18h]`, a slot that
path never writes: the previous key's value, or stack garbage for the first key. The source
keeps the previous value (0 for the first) and says so.
00792500 / 00792610 return a key's value, or for a value >= 10000.0 (00CE3D64) the angle to entity
`ftol(v - 9999.900390625)` (double 00D04658, CRT 00BF7420): its `+FCh` in the parent keyframe
entity's derived affine inverse (00414E10, 004142E0) gives `p`; theta is
`-atan2(p.y, sqrt(p.x^2 + p.z^2))` (sum and root spilled to float, 007925D4/007925E1), rho is
`atan2(p.x, p.z)`. A missing index or entity yields 0.

**00795EE0 `BSP_GameCameraModifier_Evaluate`**, RET 0Ch:

1. `t = a1 + a3` (00795EF4); keyframe `+CCh = 0`; `dt = t - [+Ch]`. If `dt <= 0` it returns
   without touching `+Ch` (00795F15).
2. With no camera it only stores `+Ch = t`.
3. Otherwise theta and rho are blended (start 0, the entity-aware key values). If either curve
   has keys or a target is set, the parent kind decides (vtable `+5Ch`):
   kind 0Fh or 18h: with the fps byte, a target overrides both angles (computed through the
   parent's cached inverse, `+10Ch`/`+110h` via 00B63D50) and 0042AA00(rho, theta, 1) is
   called on the camera; without it the camera's `+3E8h`/`+3C8h` get theta and `+3E4h`/`+3C4h`
   rho. Kind 6, when the camera's `+3E8h` dword is 0: `+384h` = rho, `+388h` = theta.
4. With the fps byte and a kind-0Fh parent, 007C7800(parent, camera).
5. Camera vtable `+DCh`(dt), then `+114h` into a 16-float matrix; the keyframe gets the matrix
   at `+7Ch`, `+BCh = 1`, position `+40h` = row 3, up `+64h` = row 1.
6. With a linked key at `+20h`: its `+25h = 1`, up `+58h` = row 1, position `+34h` =
   row 3 + row 2 * 200.0 (double 00CE4D70), and `+64h`/`+40h` copied from those.
7. `+Ch = t` (007965BD).

**0079CBD0** (ledger name was `BSP_FlightLeaderControlBlock_GetOrCreate_Provisional`, packet
`cc8_plane_squadron_host`; renamed `BSP_GameCameraModifier_EnsureCamera`). ECX is this modifier:
its Begin slot 0079CF80 calls it at 0079CF8B. The cc8 record read the kind-18h substitution
(`+3D0h`, the squadron's flight leader) correctly; the object it builds is the modifier's camera,
chosen by the substituted entity's kind: 8 -> 00650830 (410h bytes, then 0042F6C0 and the
`+384h..+398h` seed the cc8 record describes), 6 -> `BSP_ShipCaptainCamera_Construct` (404h) and
`BSP_CameraMover_SetTargetAndMode(e, 0)`, 0Fh -> 006065D0 + 0042E030 + vtable `+1C4h` with the
fps byte, else 00605130 + 0042AAF0, 46h -> 0078E830 (vtables 00D044B8/00D0449C/00D04494/00D0447C)
+ 0042EB20, 45h -> 005176F0 + 0042CB30. Every arm calls `BSP_CameraMover_SetCameraNode` with the
world's `+19FCh`; then camera vtable `+A0h` and camera `+380h = 0`. The source reconstructs the
choice only; the camera classes are the host's.

Bits (00792710 / 007A2A80): weights `+14h` then `+10h`, then the theta and rho curves, value
scale 100.0. Entity markers travel as plain values; the observer lists are not written.

## 6. Type 1, the input orbit (vtable 00D04448)

Only 007A2370 builds it, and its read slot is the base `ret 4`, so a replayed keyframe of type 1
starts from the constructor values: yaw `+8h` 0, yaw rate `+Ch` 2.0, pitch `+10h` pi/2, pitch
rate `+14h` 2.0, distance `+18h` 50, distance rate `+1Ch` 70, last time `+20h` -1, deck index
`+24h` -1, deck count `+28h` 0. No parser creates one, and its write slot is the base no-op.

007A0340 `BSP_OrbitInputModifier_Step` is slot `+8h`: 007911E0 passes (clock - key start, a
copy of camera `+394h`, 1Ch bytes). It reads axes at `+8h` (yaw), `+Ch` (pitch), `+18h`
(distance) and button bytes `+4h..+7h`:

- `dt = time - last` when `last >= 0` (COMISS/JC 007A034E), else 0.
- per axis `a`: `rate * dt * (1 - (1 - a)^2)` added for `a >= 0`, and
  `rate * dt * (1 - (1 + a)^2)` subtracted for `a < 0` (one x87 chain each, 007A038D..007A03C7).
  Distance has the opposite sign: a positive axis shortens it (007A0459 FSUBR).
- pitch clamp: below 0.01 (00D7A238) -> 0.01; above the double 3.1315927505493164 (00D04D20) ->
  the float 00D04D1C.
- with decks: `+5h && +4h` increments the index, `+7h && +6h` decrements it; below -1 wraps to
  count-1, at count wraps to -1. A deck index sets keyframe transform `+2Ch = 3`, position `+34h`
  and up `+58h` from row `index` (5Ch bytes, `+38h` and `+18h`) of `[[parent+538h]+BCh]`, grown by
  005CD640 (`BSP_VehicleCameraArray_Resize`) first.
- without one: `+34h = 0`, `+2Ch = 1`, yaw and pitch wrapped by 00605070 (reused from
  `src/ship_ai_bearing_rating.cpp`), `+40h = +34h`, then `+40h += (sin y sin p d, cos p d,
  cos y sin p d)` (007A0593..007A0638).
- `last = time` on both arms.

00791640 (slot `+10h`, RET 4, dt unused) sets the distance rate `+1Ch` to
`clamp(|distance| * 1.4, 5, 300)` (double 00D045F0, floats 00CE3850/00CE3AE8).

007916C0 (slot `+18h`, begin, RET 8 with (camera, t)) seeds the orbit from the camera's current
pose: camera `+528h` (16 floats), multiplied by the parent's cached inverse (`+110h`, 00413920)
when the keyframe has a parent; `d` = its row 3 minus keyframe `+34h`; distance = `|d|` (0 unless
the float sum of squares exceeds the double 1e-10 at 00CE3820); below 1e-5 (00CE3C70) yaw and
pitch are 0, else pitch = `acos(d.y/|d|)` (0 above 1, pi below -1, CRT 00BF9940) and yaw =
`atan2(d.x/|d|, d.z/|d|)`. Deck count `+28h` = `[[parent+538h]+BCh]+4h`, or 0 without the
array or the parent (00791876/00791887/00791896). So a replayed type-1 key starts where the
camera already is and orbits the key's authored position.

## 7. Function table

| address | name | status |
| --- | --- | --- |
| 0078E770 | BSP_CameraModifier_ConstructBase | named |
| 0078E780 | BSP_CameraModifier_SetOwner | named, defined |
| 0078E790 | BSP_OrbitInputModifier_Construct | reconstructed |
| 0078E800 | BSP_OrbitInputModifier_GetTypeId | named (constant 1), defined |
| 00791640 | BSP_OrbitInputModifier_TrackTick | reconstructed |
| 007916C0 | BSP_OrbitInputModifier_Begin | reconstructed over a host |
| 0078E870 | BSP_GameCameraModifier_TrackTick | named, defined |
| 007A0340 | BSP_OrbitInputModifier_Step | reconstructed |
| 007A0650 | BSP_GoAroundModifier_Construct | reconstructed |
| 007A06D0 | BSP_GameCameraModifier_Construct | reconstructed (observer vtable not modelled) |
| 007A0A30 | BSP_ModifierCurve_PushBack | reconstructed as `emplace_back` |
| 00791C90 | BSP_GoAroundModifier_Evaluate | reconstructed |
| 007A3100 | BSP_GoAroundModifier_ParseLua | reconstructed over `ModifierLuaValue` |
| 007918B0 | BSP_GoAroundModifier_WriteBits | reconstructed, defined |
| 007A2690 | BSP_GoAroundModifier_ReadBits | reconstructed |
| 0079DC30 | BSP_GoAroundModifier_GetTypeId | named (constant 2), defined |
| 00795EE0 | BSP_GameCameraModifier_Evaluate | reconstructed over a host, defined |
| 007A3B20 | BSP_GameCameraModifier_ParseLua | reconstructed |
| 00792500 | BSP_GameCameraModifier_ResolveThetaKey | reconstructed over a host |
| 00792610 | BSP_GameCameraModifier_ResolveRhoKey | reconstructed over a host |
| 00792710 | BSP_GameCameraModifier_WriteBits | reconstructed, defined |
| 007A2A80 | BSP_GameCameraModifier_ReadBits | reconstructed |
| 0079CF80 | BSP_GameCameraModifier_Begin | named, defined |
| 0079CBD0 | BSP_GameCameraModifier_EnsureCamera | class choice reconstructed; renamed from the cc8 provisional name |
| 0079DCF0 | BSP_GameCameraModifier_GetTypeId | named (constant 3), defined |
| 0079DD00 | BSP_GameCameraModifier_GetCamera | named, defined |
| 00799DE0 | BSP_MovieKeyframe_SetModifier | named |
| 00791360 | BSP_MovieKeyframe_WriteBits | named |
| 007A2370 | BSP_MovieKeyframe_ReadBits | named |

"Defined" means the function did not exist in Ghidra and was created by
`tools/ghidra_define_function.py` (record kept in the worktree's `local/c1_defined_functions.json`): 007918B0 and
00795EE0 lay inside the bodies Ghidra had given 00791360 and 00795D90, 0079CF80 inside 0079CBD0,
00792710 after 007926F0.

## 8. Status and uncertainty

- **Reconstructed, build-tested.** Not ABI-compatible: curves are `std::vector`, entities and
  cameras are host handles, the Lua table is a value tree. No fixture or game run exercises the
  code; no shipped script in this installation was checked for `modifier` use (cc9's runs of
  USN01/USN02/USN04 logged `unsupported=0`, so none of those missions reach it).
- **x87.** Chains are written in double with a float cast at each native FSTP; that matches the
  x87 at 53-bit precision when every product of two floats is exact, which holds for the blend
  and curve arithmetic. FSIN/FCOS/atan2/sqrt are `std::` calls and can differ in the last bit.
- **Not bound.** `src/hud_movie_camera.cpp` still counts `modifier` as unsupported; binding these
  functions there belongs to that lane (cc9).
- **Open:** goaround/gamecamera destructors
  0079DB90/0079DC40, gamecamera key tick 0078E8A0 (a `ret 4`) and begin's camera vtable `+124h`
  argument (`camera_arg + 3B8h`).

Call-site rows for `tools/verify_report_calls.py`: `reports/cc13_camera_controllers.json`
(15 rows, 0 failed on 2026-10-11).
