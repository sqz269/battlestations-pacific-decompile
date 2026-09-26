# The GUI extent 00AA1FE0 and its platform inputs

Packet `cc9_gui_extent_inputs`, cc9-platform2, 2026-09-25. Names are descriptive hypotheses, not
recovered symbols. The switch `kHudGuiExtentBound` (`include/bsp/game_hosts_hud_world.hpp`) is
**committed OFF**. Its pair waits for the session reconnect.

## 1. The getter

`BSP_Gui_GetAspectExtent` 00AA1FE0 is `__thiscall(float out[2])` with ECX = out, a plain RET, body
00AA1FE0..00AA201A (listing):

```
ECX = [0109CF04]                       ; the platform object
a   = [ECX+0Dh] ? [00D5BD9C] : [00D5BD98]   ; 16/9 (3FE38E39) : 4/3 (3FAAAAAB), MOVSS then FLD float
out[0] = a / double [00CF5750]         ; FDIVR, 1.3333333730697632 (the 4/3 float widened), FSTP float
out[1] = a / float [ECX+10h]           ; FDIV, FSTP float
```

- **No cache.** Both fields are read on every call.
- **Callers:** 0043A660 (the world-to-screen projection, mode 1, which multiplies y by `out[1]`),
  006435D0 (the markers update) and 00B156C0.
- **This process makes two per-call records of it:**

| record | USN04 4500 | E2 9000 |
| --- | ---: | ---: |
| `UnitPickScreen::gui_extent` (screen 29h's projection, per candidate) | 251,897 | 514,851 |
| `HudMarkers::gui_extent` | 82,422 | 163,422 |

The counts are from `local\tgt_on_usn04.log` and `local\grt_on_e2.log`. Both records answer (1, 1).

## 2. The producers of +0Dh and +10h

The platform object is the 184h-byte Win32 platform published at `[0109CF04]`
(docs/APP_INIT_PLATFORM.md). Two routines write these fields, and they apply the same law:

| writer | when | +10h, the active aspect | +0Dh, widescreen |
| --- | --- | --- | --- |
| `BSP_Win32Platform_CreateWindowAndDevice` 00BECEE0, 00BED0D9..00BED146 | window creation | fullscreen: the desktop aspect (`GetClientRect(GetDesktopWindow())`, width/height, also stored in +3Ch); windowed: requested width / height. x87 FIDIV, rounded to float once | +10h > [00D5BD98], strictly |
| `BSP_Win32Platform_ApplyWindowMode` 00BEBFA0 | a window-mode change | fullscreen: +3Ch; windowed: width / height | the same test |

The size comes from the settings `Resolution` (docs/SETTINGS_STARTUP_OWNER.md):
- an unsupported entry falls back to 640x480;
- without an options file, the desktop scan keeps the first matching mode.

**The host.** `src/platform_window.cpp` (lines 193..196) applies the creation law to
`Win32PlatformFields::active_aspect` and `widescreen`. It is the front end's platform code, used here
as a contract. `set_active_platform_state` publishes the object, and this packet adds a read-only
`active_platform_state()` beside it (`src/game_hosts.cpp`) so the HUD can read it. The front end
already receives `widescreen` for its GUI widescreen alignment (00AA8750).

## 3. The binding and its values

`bsp::gui_aspect_extent_00aa1fe0` (`include/bsp/gui_aspect_extent.hpp`) reproduces the arithmetic.
The x87 quotient at the default 53-bit control word is a double division rounded once to float.
With `kHudGuiExtentBound`, both call sites read the published platform object.
- The markers binding (`src/game_hosts_hud_world.cpp`) and screen 29h's projection
  (`src/game_hosts_hud.cpp`) become concrete rows.
- If no platform object is published, they keep the record and (1, 1).

| mode | +10h (hex) | +0Dh | extent (hex) |
| --- | --- | --- | --- |
| 640x480, 1024x768 (4:3) | 1.3333334 (3FAAAAAB) | 0 | (1.0, 1.0) |
| 1280x720, 1920x1080, 2560x1440 (16:9) | 1.7777778 (3FE38E39) | 1 | (1.3333333 (3FAAAAAA), 1.0) |
| 1920x1200 (16:10) | 1.6 | 1 | (1.3333333, 1.1111112) |

- **What the HUD reads.** Both host consumers read `out[1]` only, the height. The height is exactly
  1.0 at every 4:3 and 16:9 mode, so the substitution was already the image's value there. Only a
  mode between the presets (16:10, 5:4) moves the projection.
- **This machine's runs.** The options file here (`options.txt`: `resolution=640x480 fullscreen=0`)
  and the host renderer's single enumerated mode give 640x480 windowed, so the extent is (1, 1).
- **The image's default.** With no options file, the desktop scan picks the desktop mode first:
  2560x1440 on this machine, 16:9. That gives (1.3333333, 1.0), and the HUD still sees a height
  of 1.0.

## 4. Predictions, written before the pair

One tree (main 4794c365e plus this), `local\bin\ext_off` against `local\bin\ext_on`,
`BSP_GUNNERY_RNG_STREAMS=1`, USN04 4700/4500 and E2 9200/9000, this machine's 640x480 windowed
options.

| row | OFF | ON |
| --- | --- | --- |
| UnitPickScreen::gui_extent | 251,897 / 514,851 unimplemented | same counts, concrete |
| HudMarkers::gui_extent | 82,422 / 163,422 unimplemented | same counts, concrete |

- **Unimplemented total.** It falls by 334,319 in USN04 and 678,273 in E2.
- **Summary lines.** All identical: the extent is (1, 1) on both sides, so no projection moves.
- **A later confirmation.** A 16:10 or 5:4 run (not available on this host's single mode) would
  be the one that moves marker and pick positions.

## 5. The pair

`local\ext_off_usn04.log` against `local\ext_on_usn04.log`, USN04 4700/4500,
`BSP_GUNNERY_RNG_STREAMS=1`. The runs came after the session reconnect, so both sides ran at
**2560x1440 windowed**, not the 640x480 section 4 assumed.

| row | OFF | ON |
| --- | ---: | ---: |
| unimplemented total | 2,782,240 | 2,447,921 (-334,319; predicted -334,319) |
| UnitPickScreen::gui_extent | 251,897 unimplemented | 251,897 concrete |
| HudMarkers::gui_extent | 82,422 unimplemented | 82,422 concrete |

- **Summary lines.** All 182 are identical.
- **Why nothing moved at 16:9.** 2560x1440 is 16:9, so the extent is (1.3333333, 1.0). Both
  consumers read the height, which is still exactly 1.0 (section 3's table).
- **Result.** Every prediction holds. **`kHudGuiExtentBound` flips ON.**
