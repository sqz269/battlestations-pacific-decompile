# GUI routines of segments 16, 24 and 28, reconstruction pass (packet cc13-fe5)

Segments 28 (00680DB0..00696450: HUD warning screen, front-end managers, in-game interface, title and
attract screens, unit "doing" icon, award-tracker hint queue, observer helpers), 24 (00604BC0..0060CB40:
plane HUD, plane effects and plane-spawn screens) and 16 (00544E60..00554600: weapon-group HUD 2Eh,
main-menu command-bar router, idle-camera shots, control-binding screen). When the packet started the
index listed 328 functions there with a reviewed ledger name, no reconstruction record and no tag
(53 trivial bodies, 275 others).

Files: `include/bsp/frontend_seg16_24_28_cc13.hpp`, `src/frontend_seg16_24_28_cc13.cpp` (namespace
`bsp::cc13_fe5`, registered in `cmake/startup.cmake`). Records: written by `local/fe5_records.py` and
`local/fe5_rec3.py` in the worker tree (`ledger.upsert`, lease-checked; not committed). Call-site report:
`reports/cc13_fe5_calls.json`. Names are hypotheses, not recovered symbols.

## Status

| Level | What |
| --- | --- |
| Reconstruction records added | 309 addresses: 49 trivial-slot rows, 237 reconstructed routines (some as shared-shape rows), 22 backfills of existing source, 1 backfill fragment |
| Reconstructed, build-tested | every record with source `src/frontend_seg16_24_28_cc13.cpp`; `./scripts/build.ps1` passes (2/2 tests) |
| Backfilled | existing reconstructions in `src/frontend_screen_animation.cpp`, `src/hud_warning_screen.cpp`, `src/title_init.cpp`, `src/multi_menu_screens.cpp`, `src/hud_weapon_group_screen.cpp`, each checked against the live decompile (and the listing where noted) |
| Fixture-tested / ABI-compatible / game-validated | none. New C++ interfaces over raw image objects; no native adapter; nothing wired into `bsp_game`; no game run |
| Ghidra | not modified by this packet (no renames, no comments) |

`tools/verify_report_calls.py reports/cc13_fe5_calls.json`: 519 call rows checked, 0 failed. Two
sites the cached listing showed past a routine's end (00550990's unwind tail at 005509D0 and the bytes
after 006883B0's RET at 0068844F) lie in no Ghidra function and are excluded.

## Method and model

- Every routine was read from the live Ghidra decompile (`bsp.py ghidra decompile`, cached in
  `local/fe5_dec/`) and, wherever the pseudocode had register arguments, x87, tail jumps, odd argument
  counts or stack reuse, from the disk listing (`bsp.py disasm-raw`, cached in `local/fe5_asm/`).
  Constants and strings were read from the PE on disk (`local/fe5_pe.py`).
- Model as cc13-fe2: raw image objects (`ingame_gui_cc13::Raw`), engine calls through `Fe5Host`, whose
  methods name the image routine and the register that carries `this`. No invented globals: each global
  is a host accessor naming its address (`manager_global`, `ingame_interface_00e198c4`, ...).
  Function-local statics (00E1988C, 00E192F4) are host-owned blocks.
- Identical or same-shape bodies share one function or table (`kTrivialSlots`, `kSmallHudScreens`,
  `kUnitInputContexts`, `kSpectateScreens`, `kCommandRoute04/0C/18`, the ptr-array twins, the
  observer-handle twins); the record scope is `shared_shape_model` and its note names the table.
- x87 chains follow the listing's rounding points (float stores), e.g. the warning pulse, the gauge
  needle, the blinker and the control-binding marker.
- The session message C4h (00605270..00605320) is a row (`kSessionMessageC4`) for cc13-fe2b's
  session-message model; the functions are fe2's.

## Reconstructed (by area)

- Segment 28: warning screen 50h (enter, wait show/hide, reset, exit, register, layout, sound, the four
  alert bodies as `warning_screen_show_alert`, second text, warning-2 text, collect children);
  00681F40 world-edge test; manager registry tree (equal range, erase key, iterator increments);
  front-end manager base construct/destroy; logo sequence (trampoline, construct, destruct, the
  Logos.lua reader with the "(region)"/"(platform)" substitution); interface music; main, multiplayer
  and options menus (constructors, localisation tables, apply/pop/push/leave/open-pause/close-pause,
  scene exit, online error handling, request count, deactivate-and-hide); the singleton teardown
  00687C00; 006883B0 record assign; 00688780 string-list clear; the STL `_Insert_n` (00688470, also
  0054B080, and the byte version 0054AED0); attract and title screens; the in-game interface level
  helpers (0068A120..0068C0B0 except 0068BC80) and the six small HUD screen constructors/destructors;
  the HUD root constructor; the GUI blinker; the unit "doing" icon; the award-tracker hint queue and
  its deque (pop, push, grow map); the engine dword vector; the observer lock helpers; the main-scene
  render submit 0068A0D0.
- Segment 24: plane effects screen 3Fh (construct, destruct, register, enter, exit, chase camera, fov,
  view mode, select camera, bind plane, release view, visibility, object release), plane HUD screen 3Eh
  (construct, destruct, register, enter, exit, payload icon, set plane, collect children, indicator
  grid), gauge needle, owned buffers/vectors, plane-spawn camera and the screen 40h/41h helpers.
- Segment 16: weapon-group screen 2Eh (construct, destruct, register, resume, slot widgets, orbit
  camera, view centre, timed entries, observed event, collect children), the observer-handle entry
  arrays and the engine pointer arrays, the static colour 00545750, the gun-kind predicates
  005459B0/005459E0, the command-bar router and hover, 0054AC10, the idle-camera screen and shot
  helpers, the control-binding hover/release/marker/keyboard prompt, the input-device tree min/max.

## Findings and pitfalls

- The "PlaneHudScreen" routines that use +B8h as a unit (00604DF0, 00604F30, 00605E50, 006068A0,
  00608280, 00608B60, 00608D50) work on the plane effects screen 3Fh layout (its constructor 00606470
  zeroes +94h, +98h, +B8h); on the plane HUD screen 3Eh +B8h is a GUI page (00606950 removes it).
- 00608B60 clears +FCh before testing it, so it always picks the chase camera.
- 00682990's decompile loses the stack arguments; the listing gives (text, rgba, frame), RET 0Ch.
- The four warning alert bodies reuse one rounded alpha for all three widgets (the decompile shows a
  stray `1` for the second widget).
- 00683330 is RET 0Ch with three alphas; the decompile shows two parameters (the existing
  `src/frontend_screen_animation.cpp` model is right).
- 00681F40 compares with JA throughout, so a NaN coordinate is "not near the edge"; the decompile's
  `<=` form inverts that.
- 00549A80: when the session browser's +40h screen takes the event, the routine returns without
  offering it to the in-game interface (00549BA4 JNE 00549C1A); 00549C30 offers the multiplayer menu
  only +40h..+48h.
- 00553BF0 (ledger name `BSP_ControlBindingScreen_ResetSelection`) opens the Xbox virtual keyboard
  (XShowKeyboardUI through 00A4D43A with a 30h-character result buffer at +60h); the name should be
  revisited.
- 00689E80 plays the attract movie with z = -902.0 (00CEFCB8); its plate comment says 1.0f.
- cc13-fe2's session-message read model passes the cursor to `read_bool_00428d70`; the image passes
  ECX = cursor+4h (00605300 here, 006503C0 there).

## For cc9 (gameplay; not edited here)

- 00604DF0 / 00605CC0 / 00608D50 release the plane's control roles through 0077C470 (masks 1FEh, or
  1FFh then 1/1 when screen 8Ch is shown). Reconstructed here as host calls; the role transfer is
  cc9's.
- 00549200 steps the weapon group through cc9's 00548410 on a lost unit.
- `include/bsp/gun_aiming.hpp:216` calls 005459B0's answer `muzzle_blocked`; 005459B0 tests the weapon
  kind `[[gun+3F4h]+80h]` in {2, 3, 4, 6}. Both predicates are reconstructed here as pure reads.
- Existing cc9 models without records (not backfilled: not re-verified in this pass): 005484F0
  (`src/hud_weapon_group_screen.cpp`), 00546A20, 00548410, 005470C0 and 00549260
  (`src/mission_camera.cpp`), and 006082D0 (`src/game_hosts_units.cpp` player bomb release).
- Not reconstructed, gameplay reads: 0068BC80 (support availability over the unit lists),
  00605630 (direction angle deltas) and 006057E0 (response curve; possibly plane control input).

## Not done in this pass (open)

- Large binders and per-frame bodies: 00680DB0 (unit library model load), 00681BA0 (unit library
  update), 00606C60 (plane HUD layout, 629 decompiled lines), 00607F60 (cockpit camera; x87 and a
  register-reused plane pointer), 00608F30 (plane HUD widget rebuild), 00609620 (drop indicator),
  00609BD0 (instruments), 0060ABD0 (plane effects update).
- 00550DD0 and 00550FC0 (idle-camera shot uninitialised copy and `_Insert_n`).

## Handoff (cc13-fe5, 2026-10-12)

State: four commits on `agent/cc13-fe5` (batches 1-4 plus this doc), 309 records, build green after
the final `git merge origin/main`. Next steps for a successor: the open list above, starting with
00608F30 and 00681BA0 (medium bodies whose callees are already host methods here), then the binders
00606C60 and 00680DB0 (read widget names through the listing: some come from .data pointers).
Record generator and caches live in the worker tree's `local/` (fe5_records.py, fe5_rec3.py,
fe5_dec/, fe5_asm/); rebuild the caches with `local/fe5_fetch.py dec|asm` if the tree is gone.
