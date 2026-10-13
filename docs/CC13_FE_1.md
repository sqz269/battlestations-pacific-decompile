# Frontend and HUD menu screens, segments 20 and 23 (packet cc13-fe1)

Addresses: segment 20 (005805A0-005C5600) and segment 23 (005E09A0-00604A50).

Code: `include/bsp/frontend_screens_cc13.hpp`, `src/frontend_screens_cc13.cpp` (namespace
`bsp::cc13_fe1`), registered in `cmake/startup.cmake`. Records: `config/reconstruction/00580000.jsonl`
.. `00600000.jsonl` (evidence `docs/CC13_FE_1.md`, notes start with `cc13-fe1`). Call-site report:
`reports/cc13_fe1_calls.json`. Record generator: `local/fe1_records.py` in the worker tree (not committed).

This is a reconstruction pass over routines the naming passes had named but left without source:
docs/CC13_FRONTEND_MENUS.md (cc13-frontend1..3), docs/CC13_BAND_0050_0059.md, and the cc13-mopup1 /
cc13-ng2 plate comments. Names are hypotheses, not recovered symbols.

## Status

| Level | What |
| --- | --- |
| Reconstruction records added | 266 (batch 1: 204, batch 2: 48, batch 3: 14), including the one backfill |
| Reconstructed, build-tested | every record with source `src/frontend_screens_cc13.cpp`; `./scripts/build.ps1` passes (2/2 tests) |
| Backfilled record for existing source | 005BD550 `hud_minimap_screen_enter` (`src/hud_screens.cpp:718`), checked against the listing |
| Fixture-tested / ABI-compatible / game-validated | none. New C++ interfaces over raw native storage and host objects; no native adapter; nothing wired into `bsp_game`; no game run |
| Ghidra | not modified by this packet (no renames, no comments) |

At the start of the packet the index listed 419 functions in the two segments with a ledger name, no
reconstruction record and no tag.

## Method and conventions

- Every routine was read from the live Ghidra decompile (`local/fe1_dec.py`, cached per address) and,
  wherever the pseudocode had register arguments, x87, tail jumps or suspicious argument counts, from
  the disk listing (`bsp.py disasm-raw` through `local/fe1_asmf.py`). Call targets of every table entry
  were read from the listing, not from names.
- Screens and records are raw native storage (`read_field` / `write_field` at the image's offsets).
  Vtable words written by constructors and destructors keep their image addresses as values.
- Engine routines that are not reconstructed are host-interface methods named with their address.
  Already reconstructed code is called directly: `new_profile_cancel_005807d0`
  (`src/frontend_menus.cpp`), `destroy_native_string_header_0041dd20`,
  `copy_native_string_header_00be0a30_fragment`, `construct_native_string_header_0041e870`
  (`native_string.hpp`), `kOptionsPages` / `options_page_info` (`options_menu_screens.hpp`).
- Byte-identical or same-shape routines share one table-driven function (the record scope is
  `shared_shape_model` and the note names the table).
- `reports/cc13_fe1_calls.json` lists the direct CALL/JMP sites from the reconstructed routines to
  every address the header and source name; `tools/verify_report_calls.py` checks all 242 rows
  (0 failed, after the final `git merge origin/main`).

## Pitfalls found while reading

- Several "NoOp" listbox slots are `RET 8`, and the pause-cheat update is `RET 4`; Ghidra shows all of
  them as `void f(void)`. `kNoOpSlots` keeps the byte counts.
- 005830A0 calls the preview's vtable +08h with no argument (Ghidra shows `(0)`), and the counted
  release calls vtable slot 0 with ECX only.
- 00588460 is `RET 8` but pushes only one argument to 00585C50 (Ghidra shows one parameter).
- 0059AA10 passes two stack arguments to 00B189F0 (slot 0, texture) and stores the third argument at
  +140h; the decompile shows three arguments to the material call.
- 0059D2A0 calls 00803CE0 with ECX only; the unit's side index comes from (+04h)->+54h.
- 005808B0's second flag selects offset 0 and an attached listener when set (the decompile's
  `-(uint)(flag != 0) & ...` idiom), so it is an "active" flag, not "hide".
- The game page builder 005F58F0 calls 008D45C0 on screen+D8h before building the title and keeps
  `result > 1` (SETG) across the build.

## What was reconstructed (batch 1)

Screen vtable leaves (tables): screen ids (12), `MOV AL,1` slots (4), empty slots with their RET
size (15), adjustor thunks (9), child collectors (12), scroller-recapture listener thunks (005EA940,
005FFAA0, 005E10C0, 005EA700).

Native containers: tree clear (11 instances), tree destroy (12), objective-tree assign (2), empty
tree / list constructors (00583360, 00590080, 005840B0), list destroy 00583120, POD-8 uninitialized
copy 00582ED0, tree buy-node (7), uint16 vector constructor 0058A980, pointer-array push 0059FA40,
owned-object list clear 005B7840, option page tidy 005F4A20, sample vector tidy 005B9C40.

Native strings and records: string destroy at +0/+4 (3), the string-plus-tail copy constructors
(00585F20, 00585FA0, 00586070, 00586150, 005894F0, 00589480), the 3Ch record copy 00588640,
find_first_of 005B69B0, record clear 005B7FA0, observer slot set (005B7F60, 005B7B40), optional vec3
0059BD20, int wrap 005EF4D0, options row label 005F0220, small predicates (005F97A0, 005F9410,
005FC620, 006049F0), 005F9480, 005FC640, unit casts and HUD health (005F9A30, 0059AC50).

Widgets: the focus styling shared by four listeners (005FC650, 005E6B80, 005EF8D0, 005E6E20), the
show/hide fan-outs (005E6B10, 005BCA30, 005B5BD0, 005B5D40, 005F9490, 0059AD60, 0059F9B0), 005BCA80,
0059AD20.

Handles: counted release, 005830A0, 0059F780, 005BD690, 005BD6C0, 0059BEF0, map icon arrays
(0059FD80, 005A09A0), 005F97D0.

Screen lifetime: page-loading registers (005EE010 `GUI_objectives`, 005F9FF0 `GUI_order`, 00604890
`GUI_pause_cheat`), 005EA720, 005EA7C0; constructors 005E6D90, 005EA8C0, 005EE440, 005F9440,
00604780; destructors 005E70B0, 005EABC0, 005EDF30, 005F9D70, 00604810.

Main menu: 005861A0, 00590E40/50, 00588460, 0058F540, 0058F560 (exit), 00580780, 00583DE0, 005808B0,
005C3850.

Options: prompt callbacks (005EFCB0/CD0, 005F5B90, 005F6FB0, 005F6FE0, 005F01C0), 005F5AC0, 005F7020
(exit), 005EFCF0, 005EF500, the four page builders (005F58F0, 005F59F0, 005F41E0, 005F40F0).

HUD: pause resume/open/prompts/exit (005FD440, 005FF0D0, 005FD610, 005FD5B0, 005FD880), order close
005F9520, objectives expand 005ED5D0, voice stop 005B68D0, map icon texture/colour/side
(0059AA10, 0059A9B0, 0059D2A0), 005A4860, 005AAAD0.

Multiplayer: 005E4F90, 005E6B60, 005EA7F0, 005EA960, 005E6F00 (start hosting), 005E5AE0, 005E1990.

## What was reconstructed (batch 2)

Native string records: two- and three-string records (005EFE80, 005F19E0, 005FD6F0, 005FD790), option
row destruct/construct (005EFDC0, 005EFFA0, by-value string arguments destroyed by the callee),
005FDCA0, string-then-sub-object constructors (005FE370, 005FE430, 005FF330, 005FF4D0), mission score
pairs (005943D0, 00594340, 005942A0), 005F1930, 005F1750, the NativeString vector erase 00589690, the
string-int tree copy 0058BD40, the element-destroying vector tidy thunks (005E0AC0, 005F6210), 00590250,
005B8070.

HUD map arithmetic: zoom animation and scale (0059AB10, 0059ABC0), world box centre and margin test
(0059C920, 0059C9B0), the map-area test 0059D410 and the off-screen edge code 0059A910 (x87 compare
order and NaN routes read from the listing), 005A21E0, 0059ADC0, 005B5D50, the unit icon map lookups
(005A22D0, 005A31A0) and the minimap icon map find 005BE110.

Multiplayer: mode selector custom mode, back, list select (row-to-mode table 005ECA6C) and focus
listener (005EC4C0, 005ECA90, 005EC990, 005EAAC0); lobby settings change check and map stepping
(005E50D0, 005E1BF0).

Main menu: async text completion 00590A00, profile loaded 00590D80, prompt thunk 00592AD0.

Options and pause: 005F0390, 005F0730, slider listener 005F4C10, the pause Lua console command 005FD360
(log formats in the original Hungarian, read from the image), 005F60F0.

## What was reconstructed (batch 3)

HUD map and minimap pools: marker object deletion 005A3AA0, the arrow widget pool 005A4A00, the unit
icon get-or-create 005B2CB0, and the minimap screen constructor, exit and destructor (005C0810,
005C0E10, 005C0520). Two-page registers: lobby 005E1820 (`FE_lobby`, `FE_lobby_settings`) and options
005F0250 (`_Options`, `FE_options`). Options screen constructor/destructor (005F6030, 005F1BB0) and the
pause screen constructor/destructor (005FFCB0, 005FF980). Mission tree: the int-vector table reader
005C4AD0 and the 154h side-block constructor 005C4BE0.

## Uncertainty

- Field meanings in comments (focus byte +D4h, "active" list box, pending order bytes) follow the
  call sites; the names are hypotheses.
- `widget type` values 2/3/6/12h are compared, not interpreted; the type names in the WidgetHost
  comment are guesses from the widgets the screens bind.
- 005808B0 forms `+558h + 0.0` with x87 FADD; the C++ adds 0.0f in float, which is exact.
- 0059AB10 and 0059C9B0 keep x87 80-bit intermediates; the C++ uses double. 0059AB10 can differ in the
  last float bit by double rounding; 0059C9B0's bounds are exact in double unless the operands'
  exponents differ by more than 29.
- The checked-iterator calls to 00BF6713 (005A22D0, 005E1BF0, 005F4C10) are modelled as host calls; in
  the image they report an invalid parameter and do not return normally.
- 0059AA40 `BSP_Math_PiecewiseLerp4` is reconstructed in `src/ng2_cc13.cpp` by cc13-ng2 without a
  record; it is left to that packet.

## Handoff (cc13-fe1)

Commits on agent/cc13-fe1: a7f62a2ea (batch 1), 2f9acedc7 (batch 2), 2c8ffff15 (batch 3) and the
merge of origin/main 80fa541fd, plus the final doc commit. `./scripts/build.ps1` passes after the
merge (2/2 tests). Nothing was run in `bsp_game`; Ghidra was not modified.

Remaining in the two segments (ledger name, no record, no tag; regenerate the list with the query in
the packet brief): 153 functions, of which 15 are 256 bytes or less, 87 are 256-1024 bytes and 51 are
larger. The small ones left on purpose:

- 0059AA40 is reconstructed in `src/ng2_cc13.cpp` by cc13-ng2; that packet should add its record.
- 00594290 has an index extent of 16 bytes but its body (the mission score record member teardown)
  runs to about 250 decompiled lines; check the function bounds before reconstructing it.
- 00592640 (play-mission prompt: resets the 800h-pair vehicle class index maps at registry+10h and
  +2010h, then remaps the profile's checkpoint class) touches the class registry that cc9's
  game_hosts_lua.cpp models; coordinate before wiring it.
- x87-heavy layout helpers 0059D350, 0059D6D0, 005B7390: read them with `local/sc2_x87.py`-style
  tracing; constants are doubles in .rdata.
- 005EA810 (XNet QoS lookup), 005EF550 (XStringVerify), 005EFAB0 (mbstowcs of the gamer tag) are
  thin wrappers around XLive imports.
- 0058EE40, 0059FAD0, 005A07B0, 005A1220, 005A2650, 005F6EF0 are straightforward next targets.

Next by value among the larger routines: the screen Enter/Update/HandleGuiEvent bodies (HudPause
00603120/00603CA0/00604190, HudObjectives 005EEA00/005EEF60, HudOrder 005FAD70/005F9DD0, the multi
main menu 005E7160/005E73A0/005EA2D0), the options rows (005F7C40, 005F54F0, 005F5BD0), and the lobby
005E4490/005E2320. 005AB190 (11 KB HudMap register) and 005B45C0 (map update; its zoom part is in
`map_screen_update_cc13.hpp`) need range-by-range passes.

Tooling (worker tree `local/`, not committed): `fe1_dec.py` (cached live decompile), `fe1_asmf.py`
(compact listing; it hides only prologue pushes and epilogue pops, an earlier version hid argument
pushes too), `fe1_calls.py`, `fe1_records.py` (all records, lease-checked upsert), `fe1_callreport.py`.
# Second packet: cc13-fe1b (screen bodies)

Branch agent/cc13-fe1b, same files. 11 records (277 cc13-fe1 records in all), build passes,
`reports/cc13_fe1_calls.json` 302 rows, 0 failed. Nothing run in `bsp_game`; Ghidra not modified.

Reconstructed, build-tested:

| Address | C++ | What |
|---|---|---|
| 005E73A0 | `multi_main_on_listbox_selection_changed_005e73a0` | help line from 00E08980 / 00E0896C for the focused list |
| 005E7160 | `multi_main_on_listbox_activate_005e7160` | quick match: host / mode select / friends; ranked list: ranked or player title, list swap |
| 005FDB50 | `hud_pause_on_press_005fdb50` | pause press: A2h activates the list row, A3h resumes; scroller thumb and arrows |
| 005ED450 | `mode_selector_on_press_005ed450` | mode selector press: accept / back / tabs, option steps, auto-scroll 0.004 |
| 005EBC90 | `mode_selector_on_listbox_selection_changed_005ebc90` | mode texts from 00E089BC / 00E0898C, help 00E089A4, scroll range |
| 005BC920 | `hud_narrative_update_005bc920` | panel visibility, then the five per-frame steps on [game+21F0h] |
| 005FD890 | `hud_pause_show_page_005fd890` | pause page / list page switch, row list teardown, frame title |
| 005F5BD0 | `options_screen_enter_005f5bd0` | options enter: settings copy, front-end vs in-game list box, page builder |
| 005FAD70 | `hud_order_screen_enter_005fad70` | order panel: ORDPERMANENT hint, ship vs squadron widget sets |
| 00594290 | `mission_score_record_destroy_members_00594290` | bounds checked: a 5-byte JMP 00593570; Ghidra shows the target's body |
| 0059AA40 | (record only) `piecewise_lerp4_0059aa40` in `src/ng2_cc13.cpp` | backfilled after comparing the listing |

Pitfalls: the press handlers pass the integer state 2 to widget +88h/+84h (the focus styling passes
the bool), so `PressHost` carries integer-state methods. 005E7160 reads the help line for the quick
table from the *ranked* list box after the swap, as the image does. 005BC920 ignores its stack argument
and steps with the game's frame time.

## For cc9

- 00592640 `BSP_MainMenu_OnPlayMissionPromptResult` is left to cc9: on result 1 it sets screen+5Ch,
  copies the profile checkpoint for the selected mission (game+6ACh), resets both 800h-entry vehicle
  class index maps of the class registry (+10h and +2010h) to the identity, writes the one remapped
  pair, then plays the selected mission. Those maps are what `src/game_hosts_lua.cpp` models
  (registry+2010h); the 00592AD0 thunk that calls it is reconstructed here.

## Handoff (cc13-fe1b)

Remaining by value: HudPause enter/update/activate (00603120, 00603CA0, 00604190), HudObjectives
enter/update (005EEA00, 005EEF60; x87), HudOrder update 005F9DD0 (x87), multi main update 005EA2D0,
options ActivateListRow 005F7C40 and BuildMainList 005F54F0, lobby 005E4490 / 005E2320, and the large
registers (005AB190, 005BB130, 00602280).