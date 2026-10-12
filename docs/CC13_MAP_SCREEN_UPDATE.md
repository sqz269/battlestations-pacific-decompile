# HUD map screen zoom: `005B45C0` (cc13-warnings1)

`BSP_HudMapScreen_Update` (`005B45C0`) is `__thiscall(ECX = screen, float dt)`, `RET 4`. It is
slot `+20h` of the HudMapScreen vtable `00CF0134`. cc13-frontend2 had already defined it in Ghidra
(body `005B45C0..005B5B62`, 1465 instructions, five-way switch at `005B5B64`), so no definition was
needed. The bytes after the body are the switch table, then two small undefined functions at
`005B5B90` and `005B5BB0`.

This packet reconstructs the **zoom transition and the smooth zoom level**, which together drive
the `cSmoothMapZoomLevel` shader constant (`docs/CC13_STRATEGIC_MAP.md`). The map screen keeps its
`Simple3dScene` at screen+8h (`005AB1B9`). The scene fields, seen from the screen:
- screen+1Ch is scene `+14h` (`cSmoothMapZoomLevel` / `cZoomLerp`).
- screen+20h is `cShadowFade`.
- screen+2Ch is the level.
- screen+30h and +34h are the extents.

Status: **reconstructed** in `src/map_screen_update_cc13.cpp` (header
`include/bsp/map_screen_update_cc13.hpp`) and **build-tested**. Nothing is wired into a host or
validated against the game. The ledger records the pieces as fragments of `005B45C0`. All the
x87 work was traced from the listing; the pan-target formulas were checked with `tools/x87_sym.py`.
The C++ uses double where the x87 keeps 80-bit intermediates.

## Pieces

1. **Cursor to world** (`005B46E8..005B476C`, `map_cursor_to_world_005b46e8`). The mouse comes
   from GuiManager `+5Ch/+60h`, or is 0.5/0.5 when game+634h is set. It is mapped as
   `+554h = (mx - 0.5) * extent + pan_x` and `+558h = (0.5 - my) * extent_v + pan_y`. The 0.5
   centres are the `.data` floats `00E08890`/`00E08894`.
2. **Transition step** (`005B4772..005B496C`, `step_map_zoom_005b4772`). It runs while
   `remaining` (`+5ACh`) is positive.
   - When `dt >= remaining`, every value snaps to its target.
   - Otherwise `f = dt / remaining` is stored as a float. The step then lerps the extents, the pan
     (`+54Ch/+550h` toward `+5E8h/+5ECh`) and the three other pairs (`+5B8h`, `+5DCh`, `+5C4h`).
     It moves the cross-weights `+5D0h -= f` and `+5D4h += f`, lerps `+5D8h` toward float(level),
     and sets `remaining -= dt`.
3. **Wheel trigger and start** (`005B4E3C..005B5177`, `begin_map_zoom_005b4e3c`).
   - It runs when input `+24E8h` is set and `+24E0h` is clear. A negative wheel delta (`+24E4h`)
     zooms in while level < 3. A positive one zooms out while level > `+5FCh`.
   - The start records the old values, steps the level and sets the direction flags `+40h`/`+41h`.
     It sets `remaining` = GlobalConfig `+2B8h`, and the target extents to `00E0CF40[level]` and
     that times 0.75.
   - It computes a pan target that keeps the point under the cursor fixed. With
     `k = old_extent - new_extent` (as written: `(1 - new/old) * old` in, `(1 - old/new) * new`
     out) and the cursor in normalised units `u`, `v`:
     - zoom in: `pan + (u - 0.5) * k` and `pan_y - k * (v - 0.5) * 0.75`
     - zoom out: the signs are mirrored.

     The host then clamps the target with `0059E120`.
4. **Shader values** (`005B58DF..005B59FE`, `update_map_zoom_shader_values_005b58df`).
   - While a direction flag is set, `smooth = (level + 1) - t` when zooming in, or `t + (level + 1)`
     when zooming out, with `t = remaining / duration`. The flags are never cleared, so at rest
     smooth = level + 1, the same value `0093CCC0` starts with.
   - `+28h = |2(t - 0.5)|^3` dips to 0 mid-transition.
   - Then `cShadowFade = 1 - clamp(smooth - 2, 0, 1)` and `+24h = 1 - (1 - (smooth - 1)/2)^3`.

Inputs that are not image constants: the BSS table `00F89AE4` (per level), GlobalConfig `+2BCh`
and `+2B8h`, and `0059ABC0()`.

## Skipped (unit-command and other paths, listed only)

The rest of `005B45C0` was not reconstructed:
- spectate and controlled-unit handling: `[00E188D8]` vtable `+13Ch`, `0077C470`, `00647300`
  `BSP_InGameHudRoot_SetSpectatedUnit`, `00927880`, `00927F10`.
- navigation queries: `0082ADA0` and `004178F0`.
- awards and hints: `004E1CA0`, `0068E5B0`, `00690FD0`.
- the multiplayer timer at `+60Ch`: `009052E0`, `00772A40`.
- GUI widget placement and text: `00AA8240`, `00ABBE50`, the five-way switch, the `MAPPERMANENT`
  key.
- the map helpers `0059AB10`, `0059D350`, `005A3210`, `005A48C0`, `005B25D0`, `005B2D70` and
  `0059E120` (pan clamp).

The frame also writes the camera block at `[00E198C4]+F0h..+FCh`.

## Handoff

- `0059E120` (pan clamp) and `0059ABC0` (the `+5E0h` value) are the next self-contained reads.
- Nothing calls these pieces yet. A host would run them from the map screen's update slot.
