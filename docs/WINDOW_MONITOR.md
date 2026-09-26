# Window placement: which screen bsp_game.exe opens on

Addresses: 00becee0 (arguments 5 and 6), 00bed028..00bed07d (harness note; no reconstruction change)

Harness-only option added 2026-09-25 at the user's request. The image creates its window at 0,0
and moves it back to 0,0 after the style change (00bed028..00bed07d); arguments 5 and 6 of
00becee0 have no writer. The reconstruction keeps that behaviour unchanged. The translation
lives in the host binding only: `GameWindowHost` adds the chosen monitor's origin to the
position of `create_window` and of every `set_window_pos` without `SWP_NOMOVE`, so the
reconstructed routine still writes 0,0 and the window opens on the chosen screen.

## Choosing the screen

Precedence, highest first:

1. `--window-monitor <n|primary|smallest|largest>` or `--window-origin X,Y` on the command line.
   `n` is 1-based in `EnumDisplayMonitors` order; `smallest`/`largest` pick by area; `X,Y` is a
   raw desktop offset.
2. The `BSP_WINDOW_MONITOR` environment variable, read by the executable when no option is given.
3. `config/run_game.json` key `window_monitor`, forwarded by `tools/run_game.ps1` as
   `--window-monitor` when neither of the above is present. This machine's default is
   `smallest` (DISPLAY3, 1920x1080 at -1920,353 on 2026-09-25).
4. Otherwise no translation: the primary monitor, as the image.

A spec that cannot be honoured (an index past the last monitor, a malformed origin) is logged
as `window monitor '<spec>' not honoured: <reason>` and the run continues on the primary
monitor. Every run logs `window monitor <description>: origin X,Y` and, after `show_window`,
the actual `window rect left,top WxH` from `GetWindowRect`, so a log shows where the window
went. Fullscreen requests move with the offset too, since they use the same 0,0 placement.

Not a reconstruction: no ledger record, no Ghidra change, no reference row moves (the window
position is not read by any gameplay path in the host).

Probe 2026-09-25: `window monitor smallest = \.\DISPLAY3 1920x1080: origin -1920,353`, then
`window rect -1920,353 2576x1460`. The window keeps the size the game asks for (here the
2560x1440 desktop-sized back buffer plus frame), so on a smaller monitor it overhangs to the
right and bottom; the placement option does not resize it.

## Resolution: `--window-resolution <WxH|fit>` (2026-09-25, same request)

The options file on this machine asks for 2560x1440 windowed, so on a 1920x1080 screen the
window overhung. The override is applied to the host's read view of the settings block right
after the native loader has parsed and validated `options.txt`, so the window request, the
present size and the back buffer all follow it while the file on disk and the native block
keep their values (the file's resolution index is kept too; only the options screen shows it).

- `WxH` sets the size outright.
- `fit` keeps the file's size when its framed window (`AdjustWindowRectEx` with the windowed
  style) fits the chosen monitor, else takes the largest of 3840x2160, 2560x1440, 1920x1080,
  1600x900, 1366x768, 1280x720, 1024x576, 800x600, 640x480 whose framed window fits. On the
  1920x1080 DISPLAY3 that is 1600x900 (a 1920x1080 client plus frame is 1936x1100).
- Precedence as for the monitor: option, then `BSP_WINDOW_RESOLUTION`, then
  `config/run_game.json` key `window_resolution` (this machine: `fit`), then the file.
- The run logs `window resolution override <spec>: WxH -> WxH (monitor WxH, index kept N)`.

Reference runs: the GUI extent (docs/GUI_EXTENT_INPUTS.md) is identical at every 16:9 mode, and
no gameplay path reads the window size, so a 16:9 override should leave every gameplay row
unchanged; a same-binary check is recorded below when taken. Logs carry the effective
resolution in their `settings resolution=` line only before the override (the loader's line);
the `window request` and `back_buffer` lines show the effective size.

Same-binary check, 2026-09-25 (cc9 tree at the flips merge e69168f54, RNG streams on, USN04 4700/4500,
both runs on DISPLAY3): `local\cc9_res_fit_usn04.log` (fit = 1600x900) against
`local\cc9_res_2560_usn04.log` (`--window-resolution 2560x1440`). All 186 summary lines are
identical except the `window request`, `window rect` and options-file `resolution=` lines; the
census, gunnery, deaths and unimplemented-call rows match exactly. The fitted default therefore
changes no reference row, and `settings resolution=` in a log names the file value while
`window request` names the effective one.
