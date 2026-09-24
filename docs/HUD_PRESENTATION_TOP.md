# The eight largest HUD and presentation rows

Packet `cc9_hud_presentation_top` (worker cc9-platform, 2026-09-23).
Addresses: 004F75C0 00AA1FE0 00B70490 00B62D10 004C43C0 00B6DB70 00432650 00640620 0063FFC0

The counts are from `local\s11_on_usn04.log`, the ON side of `docs/UNIT_INSTANCE_STEP11.md`: USN04
4500 frames, 2,325,250 unimplemented calls.

## 1. The eight rows

| # | calls | record | what the image does per call | HUD state it needs | in the host | this packet |
| ---: | ---: | --- | --- | --- | --- | --- |
| 1 | 82,435 | FrontEndScreen::update (004F75C0) | the per-frame update virtual of each registered in-mission screen other than 35h (minimap) and 4Dh (markers); about 18 screens a frame | a reconstruction of each screen's update virtual and its widgets | only the two named screens are reconstructed | **record**: needs each screen's update, one packet per screen |
| 2 | 82,422 | HudMarkers::gui_extent (00AA1FE0) | returns (a / (4/3), a / [platform+10h]) with a = 16/9 when [platform+0Dh] is set, else 4/3 (00D5BD9C, 00D5BD98, double 00CF5750) | the platform's widescreen byte +0Dh and active aspect +10h | on `Win32PlatformState`, but they reach the HUD host only through `src/game_hosts.cpp`, which cc10 leases | **record**: needs the active aspect handed to the HUD host. At 640x480 windowed the law gives (1, 1), which is what the host returns now. |
| 3 | 73,264 | HudMarkers::view_projection_matrix (00B70490) | the camera's cached view-projection matrix | the in-mission camera at game+19FCh | none; the host uses a fixed top-down stand-in | **record**: needs a camera |
| 4 | 73,264 | HudMarkers::transform_vec4 (00B62D10) | row-vector float4 times 4x4 matrix with the x87 schedule | a matrix | the stand-in camera | **bound**: the recovered `transform_native_vector4_00b62d10` runs on the stand-in written as a matrix |
| 5 | 40,500 | InGameInterfaceUpdate::input_action_pressed (004C43C0) | the rising-edge test on the input singleton's 30h-byte action record | the action records | the menu host holds the one driven record (press-start 4Eh) and answers every index by 004C43C0's rule; `GameFrameHost` already routes 00737AE7 through it | **bound**: routed the same way |
| 6 | 18,320 | HudMinimap::refresh_renderer_basis (00B6DB70) | refreshes the renderer camera node's world matrix before its axes are read | the renderer camera node | none | **record**: needs a camera |
| 7 | 18,320 | HudMinimap::global_config (00432650) | returns the global config singleton, whose +6Ch/+70h the minimap reads | the two Globals["Minimap"] values 0087D7B0 writes | read from the installed `globals.lua` into the host's two cells | **bound**: done when the two values were read, record otherwise |
| 8 | 18,316 | HudMarkers::reset_marker_pool (00640620) | hides each entry past the frame's cursor in pools A, B and C, destroys pool D's tail and shrinks it through 0063FFC0, then zeroes the A, B and C cursors | the markers screen's four pools and cursors | the host places its marker widget outside any pool | **record**: needs the marker entries modelled as the four pools. An empty-pool walk would misstate the host's one placed widget. |

**None of the three bound routines writes gameplay state.**
- 004C43C0 reads the input singleton.
- 00B62D10 writes only its destination float4.
- 00432650 returns a pointer.

## 2. Substitutions

- **The camera.** The markers keep the fixed top-down orthographic camera over the mission's unit
  bounds, as before. It is now written as the matrix 00B70490 would return: `M[0] = M[9] = 1/half`,
  `M[12] = -cx/half`, `M[13] = -cz/half`, `M[14] = 0.5`, `M[15] = 1`. The recovered 00B62D10
  applies it.
- **Input.** Every action index except 4Eh is a record nothing starts. No device reports input in
  this harness.

## 3. Switch

`kHudPresentationTopBound` in `include/bsp/game_hosts_hud_world.hpp`, read by
`src/game_hosts_hud.cpp` and `src/game_hosts_hud_world.cpp`. OFF keeps the three records and the
old inline projection.

## 4. Predictions, written before the pair

The pair is one tree on main `031d447c4`, built twice differing only in the switch, with
`BSP_GUNNERY_RNG_STREAMS=1`.

* **OFF total** is 2,325,250, unless main moved a row since.
* **ON total** is OFF minus 132,084 = 73,264 + 40,500 + 18,320. `transform_vec4`,
  `input_action_pressed` and `HudMinimap::global_config` leave the table as done rows with the
  same counts.
* **Rows expected flat:** every other table row and every gameplay summary line, including damage,
  deaths, queued hits, releases and mission end. `input_action_pressed` answers false in both legs,
  since no in-mission action is driven.
* **One line may move.** The mission-markers summary can move only in rounding.
  `x*(1/half) + (-cx/half)` is not bit-identical to `(x - cx)/half`, so a marker's clip position
  may differ in its last bits. The counts on that line (`added`, `on_screen`, `collapsed`,
  `widgets`) are expected identical.

## 5. The pair

Logs `local\hp_off_usn04.log` and `local\hp_on_usn04.log`, each run from a copy of its own build
(`build\off`, `build\on`). Both use `--frames 4700 --press-start-frame 30 --menu-select USN04
--mission-frames 4500 --mission-frame-seconds 0.05` with `BSP_GUNNERY_RNG_STREAMS=1`.

| row | OFF | ON |
| --- | ---: | ---: |
| unimplemented total | 2,342,268 | **2,210,184** (-132,084) |
| HudMarkers::transform_vec4 (00B62D10) | 73,264 UNIMPLEMENTED | 73,264 done |
| InGameInterfaceUpdate::input_action_pressed (004C43C0) | 40,500 UNIMPLEMENTED | 40,500 done |
| HudMinimap::global_config (00432650) | 18,320 UNIMPLEMENTED | 18,320 done |
| every other table row | | identical |
| all 151 `summary` lines, the mission-markers line included | | identical |

**Against the predictions:**
- **ON minus OFF** is exactly the predicted 132,084. No other row moved.
- **The markers line** did not move even in rounding.
- **The OFF total** is 17,018 above 2,325,250. Main moved between `ed1b14b5d` and `031d447c4`, and
  the moved rows are AI rows such as `AiCommand::avoid_zone_offset_point`, none in this packet's
  area. My prediction said "unless main moved a row", and it did.
- **The startup callback count** `PlatformLoopCallbacks::pretranslate` differs, 13 against 8. It
  differs between identical runs, as in the two earlier pairs.

**Result.** The pair matches the predictions. **The switch lands ON.**
