# The minimap's camera getters

cc9-platform2, 2026-09-25, packet `cc9_minimap_camera_getters`. It follows
docs/UNIMPLEMENTED_RANKING_2.md and docs/HUD_CAMERA_TEAM_LISTS.md. Names are descriptive hypotheses,
not recovered symbols.

## 1. The camera unit

005C154E calls 004B4B00. It is bound as `HudMinimap::camera_unit` under `kHudCameraUnitBound`
(docs/HUD_CAMERA_TEAM_LISTS.md): the controlled ship.
- The update keeps it in EBP and in `[ESP+2Ch]`, read at deeper stack depths as `[ESP+34h]`.
- The slot is read back at 005C1770 (restoring EBP), at 005C1911 (skipping the camera unit's own
  icon) and at 005C1851 (the wedge below).

## 2. The direction wedge's heading, 005C1851..005C187F

```
ECX = [ESP+34h]                      ; the camera unit
ST0 = unit->vtable[C8h]()            ; 005C1862
ST0 -= icon_heading ([ESP+24h])      ; 005C1864 FSUB float
ST0 += pi/2 (double 00CF1438)        ; 005C186C
minimap_dir_Icon->vtable[44h](float) ; 005C187F SetRotation
```

docs/HUD_MINIMAP.md line 82 left "obj" unnamed. It is the camera unit. The ship vtable 00CFC3D0 holds
at +C8h (00CFC498) **0042BA40**, which has no Ghidra function: 0042BA40..0042BA66 inclusive, RET
at 0042BA66, INT3 after.

```
0042BA44  CALL 00414DB0             ; refresh the world pose
0042BA49  FLD [unit+ECh]            ; forward row x
0042BA4F  FLD [unit+F4h]            ; forward row z
0042BA55  CALL 00BF701A             ; the CRT atan2(x, z)
0042BA5A  FSTP float, FLD            ; rounded to float
0042BA63  FCHS                       ; the negated world yaw
```

This is not the hull heading at +50h (`BSP_UnitInstance_GetHullHeading`, unit+1050h), which the
icons use at 005C1CA1. It is recomputed from the forward row.

**The host** set the wedge to `kHudMinimapHeadingBias - icon`: the float pi/2 of 00CE3830, and no
unit term. It recorded `HudMinimap::camera_heading_virtual` against 004B4B00. So the wedge did not
turn with the ship.

**The binding** (`kHudMinimapDirectionWedgeBound`, `src/game_hosts_hud_world.cpp`):
`-atan2(forward.x, forward.z)`, rounded to float, minus the icon heading, plus the double pi/2,
stored as float. The forward row comes from `GameUnitsHost::unit_pose`. The row is
`HudMinimap::camera_heading_0042ba40`.

## 3. Predictions, written before the pair

One tree (main cd02f85be plus this), `local\bin\wd_off` against `local\bin\wd_on`, USN04 4700/4500,
`BSP_GUNNERY_RNG_STREAMS=1`, 1600x900.
- **Rows.** `HudMinimap::camera_heading_virtual` goes from 9,160 unimplemented to none, and
  `HudMinimap::camera_heading_0042ba40` appears with 9,160 concrete.
- **Unimplemented total.** It falls by 9,160.
- **Summary lines.** All identical.
  - The wedge's rotation feeds no summary line: the sprite bridge counts quads, not angles.
  - The minimap line's `heading=` is the map heading, which does not change.
  - No gameplay reads the wedge.

## 4. The pair

`local\wd_off_usn04.log` against `local\wd_on_usn04.log`, both at 1600x900.

| row | OFF | ON |
| --- | ---: | ---: |
| unimplemented total | 2,377,592 | 2,368,432 (-9,160; predicted -9,160) |
| HudMinimap::camera_heading_virtual | 9,160 unimplemented | none |
| HudMinimap::camera_heading_0042ba40 | none | 9,160 concrete |

- **Summary lines.** All 185 are identical.
- **Result.** Every prediction holds. **`kHudMinimapDirectionWedgeBound` is ON.**
