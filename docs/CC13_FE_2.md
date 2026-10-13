# In-game GUI screens, reconstruction pass (packet cc13-fe2)

Segments 26 (0061D5C0..00654A70) and 27 (00654A90..00680D30). The earlier passes
(docs/CC13_INGAME_GUI.md, cc13_ingame_gui / cc13-frontend2 / cc13-frontend3 / cc13-rewards1) named
the screens and reconstructed the rewards update/enter, the support-manager info panel and the
GUI sounds. This pass takes the functions that had a reviewed ledger name but no reconstruction
record and no tag (402 when the packet started, `local/bsp_index.sqlite`).

Files: `include/bsp/ingame_gui_cc13.hpp`, `src/ingame_gui_cc13.cpp` (registered in
`cmake/startup.cmake`). Records: `local/fe2_records.py` writes them (`ledger.upsert`, lease-checked).

## Status vocabulary

- **Reconstructed, build-tested**: C++ written from the live decompile (`bsp.py ghidra decompile`,
  verified against `bsp.py disasm-raw` wherever ECX, a stack argument, x87 or a store order
  mattered) and compiled by `scripts/build.ps1` (MSVC Win32). Nothing is ABI-compatible or
  game-validated, and nothing was run in `bsp_game`.
- **Backfilled**: an existing reconstruction elsewhere in `src/` with no record; checked against
  the decompile, record added (`reconstructed_ledger_backfilled_from_source`).

## Model

Every routine takes the image object as raw bytes (`Raw`, Win32 layout, offsets as in the image)
and reaches the engine through host interfaces whose methods are named after the image routine
(`HudScreenServices`, `HudScreenHost`, `NativeStringHost`, `RewardsScreenHost`,
`ScreenTeardownHost`). No globals are invented: every global the image touches is a host accessor
that names its address. Vtable stores are written as the image's vtable addresses.

ABI facts used throughout (all from disasm-raw):

- The observer pair routines are register calls: 00694A60 register / 006952A0 unregister /
  00694AF0 is-registered take ECX = the observed object and EDX = the observer member (for example
  screen+8h, or +14h with the observed pointer at member+14h).
- Collect-children slots (+24h) are `__thiscall(ECX = screen, vector* out), RET 4`; each pushes the
  ADDRESS of a page slot through 004D6790 with ECX = out.
- The GUI page calls: 004C12B0 then 00AA5840(name, 1, 0) to load, 00AA31F0(page) to remove,
  00AA7E00(name, 1) with ECX = the parent widget to find a child, 00AAB4C0(0) with ECX = the child
  to clone it.
- 004F83B0 (visibility commit) takes ECX = the screen.
- The rewards screen's widget handlers (0061D770, 0062B3B0) and its scroller recapture (00626720)
  run with ECX = the widget listener at screen+0Ch, so their offsets are listener-relative.
- 00B0CFC0 / 00B0D6A0 / 00B0CF80 run with ECX = *00F8D39C and take floats by value.

## Reconstructed

### Trivial slots and children (tables)

`kTrivialSlots` (45 rows): every screen-id getter (`MOV EAX,imm; RET`), the byte predicates, the
empty bodies with their RET sizes, the this-adjusting deleting-destructor thunks (`SUB ECX,n; JMP`)
and 0067B3F0 (`JMP 004F71D0`). `kCollectChildrenSlots` (11 rows): the page offsets each
collect-children slot pushes.

Screen ids confirmed: 04h rewards, 05h unit library, 28h cursor, 43h spectator, 44h HUD root,
45h ship status, 47h periscope, 48h submarine, 49h follow, 4Dh markers, 4Eh support manager,
54h tactical sheet, 5Ah scene init, 5Ch press start.

### Screens

| Screen | Routines |
|---|---|
| 54h tactical sheet | construct 0067AF40, self-managed 0067AF80 (session mode 0), exit 0067AFA0 (closes *00E1930C), close 0067AFD0, destruct 0067B040, update 0067B0B0 (actions 4Ah/4Bh), listbox activate 0067B0E0 (`ADD ECX,-8`), register 0067B110 ("TacticalSheetGui"), enter 0067B1D0 ("Main_Listbox") |
| 28h cursor | construct 0067B300, destruct 0067B780, register 0067BCB0 ("CursorGui"), enter 0067BD70 ("Cursor_Icon") |
| 49h follow | construct 0067BE30, register 0067BE80, exit 0067BEC0 |
| 4Ah binoculars | register 0067C430 (page loaded and hidden but not kept), enter 0067C050, exit 0067C0A0, released 0067C130, view update and recentre 0067C0E0 |
| 24h bomb/torpedo view | released 0067B3A0, queue selection 0067B3D0, exit 0067B450, enter 0067BAA0 |
| 27h common overlay | observer released 0067BC60 |
| 43h spectator | construct 00643DA0, destruct 00643E00, register 00643E80 (wanted at once), enter 00643F40 |
| 5Ah scene init | exit 00636D80, construct 00636D90, destruct 00636E00, update 00636E80, register 00636F30 |
| 48h submarine | construct 00650140, register 00650170, destruct 00650910, layout 00650B90 |
| 47h periscope | released 006502D0, exit 00650810, construct 00650B00, destruct 00650970, layout 00651230 ("Periscope_Model", "Tavcso_Model") |
| 4Dh markers | set view centre 00637620, entry hide-all 006374B0, colour-icon hide-all 00637540, endpoints 00638B20, colour-icon/name/befogo destructors 006391C0 / 00639450 / 00639960, name/befogo constructors 0063E1C0 / 0063E950, screen construct 00640C80, exit 0063E360, destruct 006404A0 |
| 46h ship view | register 0064A360, enter 0064A450, exit 0064A490, released 0064A4C0 |
| 44h HUD root | register 00645F10, destruct 00647DB0 |
| 45h ship status | exit 0064D8D0, timed-entry clear 00644240 |
| 04h rewards | release sounds 0061EC00, widget release 0061D770, widget press 0062B3B0, exit 00621720, continue 0062B110, scroller recapture 00626720 |
| 4Eh support manager | mode tests 006529F0 / 00652A00, slot clones 00652A60, arrow pairs 00652AA0 / 00652AD0 / 00652B00 / 00652B30, powerup info 00652B60 |

The scene-init update (00636E80) re-colours Message_Text with alpha
`|sin(float(2 * clock))| * 0.75 + 0.25`: the doubled clock and the FSIN result are each stored to
float, the sign bit is cleared in an integer register, and the scale/offset are doubles
(00CEC9D8, 00D7A348) applied on the x87 stack.

0062B3B0 dispatches a type-3 button on the first byte of its name: A2h continues, A5h shows the
gamer card of the selected scoreboard row, A7h the player review (both on the multiplayer page,
+4Ch == 1, through XShowGamerCardUI / XShowPlayerReviewUI). The auto-scroll speed is 0.004f
(00CEC178).

### Shared shapes

- `observer_ref_set`: 0064B340, 00656DE0, 00656E20, 0064B610, 0064BB50 (the last two return the
  ref). `observer_ref_released`: 0064B590, 0065AFA0, 0065B040, 0065B090.
- Native-string records: pair destroy 0061F210 / 00645FC0, single-string release 0061F2A0 /
  00623370 (+8h) and 0065CBC0 (+4h), the 60h scoreboard row destroy/assign/copy 0061F2C0 /
  0061F540 / 00622790 (the assign copies +20h..+57h, +58h and the +5Ch byte; +1Ch is never copied),
  the 1Ch bonus record assign 0061F4A0, the 10h record copy 00622870, 00622F90, the 10h string pair
  assign/copy 00646230 / 00646E20 and its copy / copy_backward / fill 006462A0 / 00646350 /
  00646F20, fill_n 00646170, vector tidy 00625EC0 / 00647C90.

### Small helpers

0061EB80 (profile +C0h award vector non-empty), 0064A4F0 / 0064A540 (widget quad fade),
0064B6B0 ("PTBoat" when the camera's unit is kind 0Eh, else "ShipCaptain"), 0064B6D0.

## Backfilled

00638E50 (src/hud_markers_runtime.cpp), 00644A60 / 00645710 / 00648290 (src/hud_root_rows.cpp),
006488D0 / 00639480 (src/hud_screens.cpp), 0064A960 / 0064A9F0 / 0064AAD0 / 0064ABD0
(src/hud_ship_screen.cpp), 0064A400 / 0067BF00 (src/hud_warning_screen.cpp), 0067C170 / 0067DF40
(src/band_0067_cc13.cpp). Fragments: 0064DD30 (src/hud_ship_screen.cpp; two blocks are host
calls), 0064B650 / 0064DA40 (src/mission_camera.cpp; state-level camera models).

## Left to cc9 (drives gameplay state)

Not reconstructed here, listed for the gameplay lanes:

- 00644A20 (root released: clears the controlled unit through BSP_Game_SetControlledUnit),
  00645BB0 (root listener: SetControlledUnit), 00647040 (push the controlled unit's interface),
  00647300 (SetSpectatedUnit), 006485A0 (ForceSelectUnit), 00648930 (EnsureSelectableControlledUnit),
  0067BB50 (captain role take, 0077C470), 0067B7E0 (bomb view update: ForceSelectUnit after the
  timeout), 0064A270 (repair priority), 0064A2F0 (order menu close: repair channel dispatch),
  0064D610 (ship view update: issues orders on action 95h), 00649860 (HUD root update),
  0066EA60 and 006743E0.. / 00675C40 onward (cc9 air operations and the support-manager update).
- Exits that release unit roles through 0077C470 (cc9's SendRoleTransfer) are reconstructed with
  that call kept as a host method: 0067C0A0, 0064A490.

## Call sites

`reports/cc13_fe2_calls.json` lists every direct CALL/JMP in the reconstructed bodies (scanned with
disasm-raw up to the INT3 padding); `tools/verify_report_calls.py` checks it against the live
listing (325 rows, 0 failed at the first commit).

Code with no Ghidra function, found because a first scan ran past function ends: calls at
0061F604/0061F619 (after 0061F540), 006375B8/006375D6 (after 00637540), 00646EFE (after 00646E20)
and 0064A394..0064A3DF (after 0064A360). Not defined in this pass.

## Not done in this pass (open)

- Session-message classes in these segments (160/A0h, 148/94h, 161/A1h, 130/82h, 167..169/A7h..A9h:
  IsType / Write / Read / ScalarDelete / construct). They belong with the native session-message
  series (docs/NATIVE_SESSION_MESSAGE_BASE_R177.md onward, which stops at 112).
- STL instantiations over plain ints and rows (heap/sort helpers 0061D5C0..00626320, list/tree
  helpers 00659AA0..0066B2F0): library code; no record added.
