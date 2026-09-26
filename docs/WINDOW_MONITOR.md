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
