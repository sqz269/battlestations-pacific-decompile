# Front-end segments 15 and 22, reconstruction pass (packet cc13-fe4)

Segment 15 (00527CB0..00544B60: control layout, HUD counters, credits, menu prompt, HUD
formation, frame stats, free camera, debug text lists) and segment 22 (005CD070..005DF510:
movie screens, multiplayer chat, error, pause and game-lobby screens). This pass takes the
functions that had a reviewed ledger name but no reconstruction record and no tag (334 when the
packet started, `local/bsp_index.sqlite`).

Files: `include/bsp/frontend_seg15_22_cc13.hpp`, `src/frontend_seg15_22_cc13.cpp` (registered in
`cmake/startup.cmake`). Records: `local/fe4_records.py` writes them (`ledger.upsert`, lease-checked).

## Status vocabulary

- **Reconstructed, build-tested**: C++ written from the live decompile (`bsp.py ghidra decompile`),
  checked against `bsp.py disasm-raw` wherever ECX, a stack argument, x87 or a store order
  mattered, and compiled by `scripts/build.ps1` (MSVC Win32). Nothing is ABI-compatible or
  game-validated, and nothing was run in `bsp_game`.
- **Backfilled**: an existing reconstruction elsewhere in `src/` with no record; checked against
  the decompile, record added (`reconstructed_ledger_backfilled_from_source`).

## Model

The routines take the image object as raw bytes (`Raw` from `bsp/ingame_gui_cc13.hpp`) and reach
the engine through `Seg15Host`, which extends `HudScreenServices` from the cc13-fe2 pass. Every
host method names the image routine it stands for; a global the image touches is a host accessor
that names its address (`global_object`, `global_block`). Routines of these segments that this pass
did not reconstruct are host calls too (for example the counter layouts 00529D70, 0052A100,
0052A860, 0052BB10 and the entry constructors 0052C1B0 / 0052C640).

ABI facts used throughout (all from disasm-raw):

- 004CC460 (push interface request) runs on different managers: the control layout uses
  *00E198B8 (options), the credits listener *00E198AC (main front end).
- 004D92B0 (action edge or repeat) takes ECX = *00E188A8.
- Collect-children slots (+24h) are `__thiscall(screen, out), RET 4`, pushing page-slot addresses
  through 004D6790.
- Native strings passed by value (count, data) are freed by the callee; the model takes a
  `std::string_view`.
- Several containers are MSVC 8 vectors `{allocator, first, last, end}`; `vector_insert_n` models
  `_Insert_n` once (1.5x growth, the xlen limit per element size) for 0052F9E0 and the later
  instances.

## Reconstructed

### Trivial slots

`kTrivialSlots` (35 rows, generated from disasm-raw by `local/fe4_triv.py`): screen ids 0Eh control
layout, 51h HUD counters, 08h credits, 55h menu prompt, 2Ah HUD formation, 57h frame stats, 2Bh free
camera, 36h movie camera debug, 15h chat, 1Dh error, 19h pause (multiplayer); the byte predicates,
empty bodies with their RET sizes, the adjustor thunks and the tail jumps (005361B0 -> 00535C30,
00536250 -> 00535DD0, 0053FF00 -> 0053F250).

### Containers

Pointer arrays `{data, size, capacity}`: reserve 00529700 / 00529760, resize 005297C0 / 005299D0,
push back 00529900 / 00529940, word push back 005298C0, erase 00529620, index 00529650, destroy
00529C50 / 0052AE40. The 18h-byte observer-handle records (vtable 00CE75CC, observed +14h, set byte
+10h): reserve 00529C70, push back 0052AD50, swap-remove 0052ADD0 (re-registers the observer pair),
resize 0052AE60, destroy 0052C0A0.

### Control layout (0Eh)

Construct 00527D00, destruct 00527DA0, register 00527E60 (FE_controls_X360_listbox +10h,
FE_controls_X360 +14h), bind 00527F90, exit 00528920, update 005289B0 (back = action 4Bh ->
interface 0Ch), collect 00528A60, button-flag take 00527CB0, listeners 00527D50 / 005289E0 /
00527E30.

00527F90 copies the stick-label texts: for each of the two name triples at 00E08674
(LeftStick*/RightStick* Vert/Horz/Click) and each group widget (Plane_Group, Ship_Group,
Submarine_Group at +1Ch/+20h/+24h), the child's text (+F4h) is copied into the next of 18 native
strings at 00E18DC0.

### HUD counters (51h)

Screen layout: +8h page GUI_counters, +0Ch HP entry, +10h countdown entry, +14h score entries
(pointer array), +20h visible list (pointer array). Entry (0ACh bytes): +0h root widget, +4h/+8h
position widgets, +28h text, +70h record array, +7Ch/+88h/+90h strings, +98h seconds, +A0h time
format, +A4h owns widget, +A5h..+A9h occlusion flags.

Routines: format time 00528B00, set value 00528BE0, bar 00528E90 (frac = max(value, 0.1) / max;
scales {frac, 1}, {frac * a, 1}, {clamp(a - b, 0, 1) * frac, 1}), occlusion 00528F70 / 00529200 /
005293B0 / 00529810 (with the body at 00529120, which has no Ghidra function), remove 0052AB40 /
0052AB90, construct 0052B030, show 0052B9B0 / 0052BFA0 / 0052C990 / 0052CAC0, ensure entry
0052C850, entry destruct 0052C0C0 / 0052CDC0, register 0052CBF0, destruct 0052CDF0, remove entry
0052CF00, collect 0052D1D0.

Session senders (message 23h, `bsp::NativeSessionMessage35` layout, sent through 00772330 with
ECX = *00E188A8 + 1EF0h): unit HP display 0052B400 (unit ids from each unit's +174h) and hide
0052B570; score display 0052B620, score pair 0052B780, hide 0052B900. These carry Lua
`DisplayUnitHP` / `DisplayScores` / `HideScoreDisplay` presentation to the other players.

Edges and constants from the image: 0.76 / 0.877 (00CED048 / 00CED050), 0.936 / 1.052
(00CED038 / 00CED040), 0.71 / 0.89 (00CED058 / 00CED05C), 0.355 / 0.2 (00CED030 / 00CE54A0), the
-900 wrap with +1000 (00CED028 / 00CE47A0); the wide-screen choice is *(*0109CF04 + 0Dh).

### Credits

Listeners 0052D200 / 0052D210 / 0052D6B0 / 0052E5F0, vswprintf wrapper 0052D1E0, string helpers
0052DFB0 / 0052E6D0 / 0052E0F0, item copy / assign / placement / copy-backward 0052E2C0 / 0052E340 /
0052E3B0 / 0052E410, item vector insert 0052F9E0, construct 0052FCE0 (scroll speeds 0.0444f, 0.5f,
0.01389f), collect 0052FD70, destruct 0052FD80.

### Menu prompt (55h)

Toggle listener 005309F0 (in session mode 2 the widget's bound record +D8h takes the flag), the
00E18E6C singleton destructor bodies 00530CC0 / 00530E00, register 005311C0 (_DialogueBox,
dialog_BG_FrameBox, _DialogueFarBlackPlane), record copy 00531C90 (48h bytes), record list clear
00531F80, collect 00532350 / 005329C0, button activate 00532D70 (results 1, 0, 2, 3).

### HUD formation (2Ah)

Action readers 005337C0 / 005353A0, widget sets 00533830 / 00533920 / 00533A30, listener tail call
00533B20, vec2 compare 00533D30, formation message destructor 005347C0, refresh 00534BC0 (Exit then
Enter when the formation UI flag and the group are set), input-context restore 00534C00, action
release tests 00534E90 / 00535EE0, member selection 00536100 (cursor = (-x / w + 0.5,
z / h + 0.5)), collect 00538D20, member marker 00538D60, construct 00538E30.

Correction: the menu_screens_2 note on 00538D60 calls the visibility call "decompile only"; the
listing makes it (00B6DA70 on marker +128h with 0.0f, 0).

## Backfilled

- 00530970 menu prompt exit (`src/menu_screens_2.cpp`).

## For cc9

(none yet)
