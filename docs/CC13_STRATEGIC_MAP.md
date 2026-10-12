# Segment 61 presentation side: the map scene and map overlay (cc13-warnings1)

Segment 61 runs from `0092E0B0` to `00951F20`, about 595 entries including EH funclets. Most of it
is gameplay, which cc9 owns: unit physics, repair, spawning, the support manager and resource
usage. This packet took only the presentation cluster `0093CB10..0093F4B0`. It covers the
`Simple3dScene` that the HUD map, formation and library screens own, the map quad that carries
`cSmoothMapZoomLevel`, and a 2D map-overlay debug drawer.

Status words: **reconstructed** (pure rules in `src/strategic_map_cc13.cpp`, header
`include/bsp/strategic_map_cc13.hpp`), **build-tested**. Nothing is wired into a host or validated
against the game. The names are hypotheses. Every listed address was read from the listing
(`disasm-raw`), and every literal and constant from the PE.

## Triage

| Range | Content | Owner / decision |
| --- | --- | --- |
| `0092E0B0..0093CA20` | UnitController steering, hydro and network state; part detach and wreck debris (`fizika_%d`, `wreck_%d`, `hajobelso`); repair task (fire, water, failures, `EngineJam`) | cc9, already mostly named (`BSP_UnitController_*`, `BSP_RepairTask_*`). Still `FUN_`: `00935D30` (hull interior, `hajobelso`/`HATULLYA`/`ELELLYE`). |
| `0093CB10..0093DE10` | Simple3dScene and map scene materials | **this packet** |
| `0093E510..0093F4B0` | 2D map-overlay debug drawer and sound-overlay helpers | **this packet** (named; two rules reconstructed) |
| `0093F520..00941550` | Sound sample parameters and options, sound request queue, GUI sound flags; `0093FF10` is a sound debug overlay (`Lin`/`Log`, `%s(%d%%%%) %s:%d/%d/%dm:%d`) with no reference; `00940EC0` (`InPauseGUI`), `00941070` (`Normal`) | Audio lane, listed only |
| `00941D30..0094C8F0` | Spawn manager and Lua spawn bindings (`luakod`, `excludeRadiusOverride`, `groupMembers`), `ResourceUsage`/`OwnerPlayer` property bags, `[supportmanager]` | cc9. Still `FUN_`: `00945450` (`luakod`), `009467B0`/`00946870` (`ResourceUsage`, `OwnerPlayer`), `00947A30` (`[supportmanager]`), `0094B600`/`0094BFF0` (spawn property bags: Equipment, WingCount, VelocitySI, TorpedoDirector, CamoColor, AutoAttackTarget, ReloadEnabled). |
| `0094CCD0..0094EBD0` | `iostdnet` ordered-map instantiations | library, untouched |
| `0094F910..00950130` | `Effect`/`Period`/`Delay` readers, `%.4f` | cc9 (gameplay effects), listed only |
| `00951160..00951E30` | Traffic records, entity id table, vehicle-class constructors | already named; `00951820`/`00951940` (`used`) still `FUN_` |

## Reconstructed (build-tested, reconstruct-only)

### Map zoom: `0093CCC0` `BSP_Simple3dScene_Init` (`init_map_scene_zoom_0093ccc0`)

The function is `__fastcall` with ECX = scene. Its zoom part, `0093CDDF..0093CE25`, sets:
- scene `+18h` and `+20h` to 1.0f.
- `+14h` = float(level `+24h`) + 1.0 (FILD, FADD qword, FSTP dword). This is the field the
  shaders read as `cSmoothMapZoomLevel` and `cZoomLerp`.
- `+28h` = `00E0CF40[level]`. The table is `{48000, 18000, 8000, 3000}` metres.
- `+2Ch` = `+28h` * 0.75 (FMUL qword `00CEC9D8`).

The constructor `0093CB10` sets the level to 0, and the native does no bound check. The map screen
keeps its scene at screen+8h (`005AB1B9`).

The per-frame smoothing of `+14h` is **not in this segment**. The zoom table is also read by the
map-screen update `005B45C0` (`005B4FAC`), which has no Ghidra function (`docs/CC13_FRONTEND_MENUS.md`)
and keeps its own copies at screen `+5B0h`/`+5B4h`. The lerp is reconstructed in `docs/CC13_MAP_SCREEN_UPDATE.md`.

The rest of `0093CCC0` builds the scene graph: `Simple3dSceneHandler`, `SceneHandlerCamera`, the
viewport, `MapLights` and `TestDirectionalLight` normalised through `00419510`, and lighting from
game+5FCh. That part is engine plumbing and is not reconstructed.

### Map quad: `0093DE10` `BSP_MapScene_BuildQuad`

The function is `__thiscall(ECX = scene, mode, rgba[4], NativeString effect, y_offset, width,
height, alpha_ref)`, `RET 2Ch` (11 dwords). The argument layout comes from the stack offsets and
the string release at `0093E2A7`. It builds the model `MeshQuad<mode>` with vertex format
`pf43uf42.mvfm`:
- **Vertices** (`build_map_quad_vertices_0093de10`): one 4-vertex triangle strip on the XZ plane,
  traced through the x87 stack at `0093DFEF..0093E0A7`. In order, as (x, y, z, u, v):
  (-w/2, 0, h/2, 0, 0), (w/2, 0, h/2, 1, 0), (-w/2, 0, -h/2, 0, 1), (w/2, 0, -h/2, 1, 1). The draw
  section gets `+8h` = 5, `+10h` = 4 and `+18h` = 2.
- **Shader constants by mode** (`map_quad_shader_bindings_0093de10`, `0093E0BE..0093E457`):

  | mode | constants |
  | --- | --- |
  | 0Fh | `cBaseSize`, after storing (w, h) at scene `+30h` |
  | 0 | `cSmoothMapZoomLevel` -> scene `+14h` |
  | 0Dh | `cZoomLerp` -> scene `+14h`, `cAspectRatio` -> `00D19720` (0.75f) |
  | 0Ch | `cShadowFade` -> scene `+18h`, `cStaticShot_Size_OffsetX_OffsetY` -> `00E1891C` (float3, BSS) |
  | 0Eh / others | none |

- It sets the material's diffuse colour to rgba. `00B17310` sets alpha ref only when
  `alpha_ref < 100h`.
- **Placement** (`map_quad_position_0093de10`, `0093E1F7..0093E296`): the quad is placed at the
  centre of the map bounds (game `+711Ch/+7120h/+7124h` and `+7128h/+712Ch/+7130h`), with y raised
  by `y_offset`, through the node's vtable `+30h`.
- Callers are 5 sites in `BSP_HudMapScreen_Register` and 3 in `BSP_HudFormationScreen_Register`.

### Overlay ellipse: `0093E9A0` `BSP_MapOverlay_DrawEllipse` (`map_overlay_ellipse_0093e9a0`)

It draws 32 chords at angles i*pi*0.0625 for i = 1..32. Each angle is rounded to float before
FSIN/FCOS. Each endpoint is center + radius*(cos, sin), drawn through `0093E510`. The world-to-screen
step of the overlay (`map_overlay_world_to_screen`, `0093EAEA`) is
x = `+18h` + `+0h`*(wx - `+20h`) and y = `+1Ch` - `+4h`*(wy - `+24h`).

## Named only

| Address | Name | Evidence |
| --- | --- | --- |
| `0093CB10` | `BSP_Simple3dScene_Construct` | Sets vtable `00D19724`, zeroes `+4h..+10h` and `+24h`. |
| `0093CC20` | `BSP_Simple3dScene_Release` | Releases `+0Ch`, `+10h`, `+8h` and `+4h`. |
| `0093D250` | `BSP_Simple3dScene_SubmitRender` | `BSP_RenderCommandQueue_SubmitScene(+4h, +8h, ..)`. Called by the two ortho interface views. |
| `0093D2D0` | `BSP_MapScene_SetupOceanMaterials` | Recursive over child `+34h` / sibling `+3Ch`. Loads the ocean `.shfx` and `bterrain_underwater_test.mshd`, then registers `cSmoothMapZoomLevel` (scene `+14h`), `cOceanSize` (`00F89B10`, `00F89AF4`), `cStaticShot_*`, `cBasic___ShoreH_DirtH_SimpleDist_SimpleFog` (`00E18928`), `cSnoise___Div_Max_UV`, `cHnoise___Div_Max`, `cGround___slope_UV_maskUV` (`00F89B04`), `cUW___slope_UV`, `cUWColor0/1` and `cShadowFade` (scene `+18h`). |
| `0093E510` | `BSP_MapOverlay_DrawWorldLine` | `RET 10h`. Draws through `00860030`; it has five self-calls that were not traced. |
| `0093EAE0` | `BSP_MapOverlay_DrawArrowHead` | Two `00860030` strokes. |
| `0093EC80`, `0093EDC0`, `0093EEB0` | `BSP_MapOverlay_DrawArrowMidHead`, `_DrawArrow`, `_DrawWorldCircle` | No reference (`callsite_census`). |
| `0093EEE0` | `BSP_Math_RoundToStep` | `_ftol(v/step + 0.5) * step`. |
| `0093EFC0` | `BSP_Matrix44_NotEqual` | 16-dword compare. |
| `0093F280`, `0093F4B0` | `BSP_SoundOverlay_AxisStep`, `_CompareByVfunc1C` | Helpers of `0093FF10`. |

`007BAC50` (no reference) and `0093FF10` (no reference) are the only users of the overlay drawer,
so it is debug code.

## RNG

This cluster makes no game-RNG draw. `local/wn1_rng.py` over `0093CB10..0093F520` finds no
`00BD2F10`/`00BD2FC0` reference. `00BF7420` in `0093EEE0` is `_ftol`, not `rand`.

## Handoff

1. Done: the `+14h` smooth-zoom update is in `docs/CC13_MAP_SCREEN_UPDATE.md`.
2. `0093E510`'s self-recursion (clip or subdivision) was not traced.
3. The audio rows (`0093F520..00941550`) and every cc9 row above are listed only, not touched.
