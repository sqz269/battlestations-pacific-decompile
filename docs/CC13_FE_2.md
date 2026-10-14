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
and 0067B3F0 (`JMP 004F71D0`). The markers thunk 006405F0 also has code (cc13-fix3):
the deleting destructor `hud_markers_scalar_delete_00640d50` runs the markers destructor
006404A0, frees the object when flags bit 0 is set and returns it; the thunk
`hud_markers_scalar_delete_adjustor8_006405f0` subtracts 8 and jumps there.
`kCollectChildrenSlots` (11 rows): the page offsets each
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

### Second batch

| Area | Routines |
|---|---|
| 04h rewards | selected-row flag 0061D8B0 (signed-in Xbox user, +230h[row]), play-and-remove 00622E00, construct 00626630, destruct 00626770 |
| 4Dh markers | colour-icon set construct 0063DEC0 (CLOSE_Group: Yellow, White, Coffe (sic), Blue, Red), unit-marker widgets destruct 00638F50, pooled marker hand-out 00640860 / 00640940 (`kObjectiveMarkerPool`, `kCommandUnitMarkerPool`) |
| unit icons | 00644AE0: ordnance kind for planes (through their leader +3D0h) and kind 0Fh, record class for kind 1Bh (45h when it is 1Bh with +50h clear), 4/7 for kinds 0Ah/0Ch with record byte +808h, else the class at +C4h; classes map through 006440C0 (src/band_0060_cc13.cpp, called directly) |
| unit tests | 00645160 (forwards ECX, local slot in EDX, flag on the stack to 00645060), 00645180 (session: owner id +54h against the local player record +28h) |
| 44h HUD root | set interface 00646040, text helper 00646D30 (ellipsized C string on the text at +34h, width -1.0f) |
| 45h ship screen | unit ping 0064A8E0, set unit 0064D590 (observer member +170h) |
| 47h periscope | attach 00650210, detach 006506E0 (role release, fov 1.0), enter 00650750 (periscope state +122Ch / +1234h selects camera mode 1 or 0), bind-and-reset 00651760 (writes the unit's +122Ch back to 0 unless it is 2) |
| 5Ch press start | storage completion 0067CA40 |

Corrections from the listing: 00644AE0's decompile hides that every class-icon call is a tail
jump with ECX = a constant or field (45h, +C4h, 10h, 11h, 13h, 14h); 0067C0E0 takes a float.

### Third batch (observer objects, support-manager records)

- Observer-reference objects (callback-owner base, observed pointer at +14h, +10h "set" byte):
  destructors 00653400 (vtable 00CF64AC) / 006569A0 (00CF6494), copy constructors 00658710 /
  006587C0, and 0065AFF0 joins the released handlers.
- 0065BB00 unit-slot entry destructor (target observer +34h, MSVC string tidy at +14h).
- 006626E0 observer/string record assign (legacy string +10h, observer member +34h).
- 006683D0 string-vector holder destructor; 00669350 int + map record assign; 00660540
  int/string/int record copy.
- Support-manager slot record (58h, vtable 00CF6564): assign 0065CA50, copy 0065CAF0 (members
  re-vtabled to 00CF6514 / 00CF652C after their copy constructors), "out" test 00658530 (+28h set,
  state +2Ch 3 or 4).
- 0065BE80 tracked unit target (member +208h, observed +21Ch); 0066FFA0 available-entries test
  (effective game mode 8 only; cached in +479h against game+648h).

### Fourth batch (unit library, press start, text fitting)

- Unit library (05h, vtable 00CF6E50, 3D scene sub-object at +8h built by 0093CB10 and torn down
  through 0093CC80): construct 00680C20, destruct 0067EF50, exit 0067DED0 (preview delete, movie
  stop, title music, pending profile save), reset selection 0067E580, model-part release 0067ED30
  (two parallel vectors: parts deleted, handles released, both cleared).
- Press start: 0067C880 (device button 0 or 0Eh), 0067C8F0 (pending invite for the active user).
- Text fitting: 0067D870 is `__fastcall(ECX text, EDX prefix, src, cursor_base, first, last)`,
  RET 10h; 0067D940 slides a window over the source until the text (prefix + window) fits on one
  line (+110h line count), starting from min(cursor_base + 8, count). The decompile's argument
  list for both is right only once EDX is read as the prefix.

### Fifth batch (layout binders)

The page loads and child finds were extracted with `local/fe2_bind.py` (capstone over the disk
image: string pushes, 00AA5840 / 00AA7E00 calls, the ECX source of each find and the store that
follows), and every store the script could not pair was read from disasm-raw.

| Routine | Pages and notable extras |
|---|---|
| 00650E00 periscope register | GUI_periscope (+6Ch); effect handles "Warning Dive DIVE Dive" (+30h), "Lvl3Water" (+38h), "PeriscopeEmerge" (+3Ch), "PeriscopeDrops" (+4Ch) through 00871BA0, stored refcounted |
| 006463E0 HUD root layout | GUI_powerups (+F8h, templates hidden), GUI_unit (+2Ch; unit_name_Text nudged by (+0.006, -0.005)), GUI_selector (+28h, hidden; ClosedUnitHUD_Group +40h shown) |
| 0064C0F0 ship-status layout | GUI_ship (+34h; stick y kept in +138h), GUI_repair (+38h), GUI_ship_effects (+3Ch; four "Villanas" flash icons, finds first and stores together at 0064D52B..0064D53D, then hidden), GUI_ship_damage (+40h, shown; Hl_1..4 alpha 0) |
| 0065DAA0 support-manager layout | radial menu of the page at +78h (pum_info_Text inside sm_close_Group: both names are built before the two calls), circle_large_Group scale (1.0001, 1.0001), slot rebind 0065D220, GUI_support_infopanel (+16Ch, hidden), map_Group position into +1B4h..+1BCh |

Left for a later pass: 0061FBE0 (rewards bind, 58 finds plus scroller set-ups; several group
names are not pushed as immediates, so the script mispairs them), 0064BB90 (ship-status register:
four samples through 00870CD0 and more), 0067F080 (unit library register: 3D scene set-up),
00629D10 (rewards register: page names through 00450540).

### Sixth batch (support-manager fades, icon frames)

- 006723F0 close fade: +28Ch counts down by dt (float-stored, clamped at 0); above 0.5 the four
  widgets +7Ch..+88h take alpha 2t - 1 through 00AA8610, otherwise 00670020 runs, +30Ch clears and
  the alpha is 1 - 2t; exactly 0 sets +2B1h and clears +288h.
- 0065BD90 name-label fade-in at 4 per second (00D7A328) in +1E8h; the frame +18Ch is sized to
  (text width / 960 * fade + 0.21, +198h) (00CEC380, 00CF6558, x87 product kept in extended
  precision before the add).
- 00652BA0 slot icon frame (unit kinds 16h, 0Eh, 15h, 7, 8, 0Dh, 0Ah through vtable +18h; record
  byte +808h picks the alternate frame), 00658620 support record icon frame (states 5 / 3-4 / 1 while
  +1DCh is clear; out-of-play unit type 00E08FA0; +298h counts the calls that fall through).

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

## Seventh batch (cc13-fe2b): the four large binders

- 0061FBE0 rewards bind layout. The six page groups of FE_scoring are found by the names held in
  the char* cells 00E08C70..00E08C84 (single_page1, multi_page1, objectives_page, detail_page,
  multiscore_page, awards_page; read through the host because the cells live in .data); this is
  why the first extraction mispaired them. Every arrow, scroll icon and thumb gets the widget
  listener at +0Ch (00AA6BC0(+0Ch, 0)). Each of the three scroll panes (+68h, +128h, +19Ch) binds
  its move group (00683790) and calls 00683380(thumb, back_y - thumb_y, up, down). The pop-up's
  A_Text gets the key glyph of action A2h (00531030, 00531130, 00AB80C0, 00ABAED0); the listener
  argument of 00531130 is the +0Ch value saved at 0061FC8F (uncertain).
- 00629D10 rewards register: FE_scoring page, bind, and the global title-key vector 00E196BC
  refilled with the six FE.scoring_* keys.
- 0064BB90 ship-status register: effect handles 1..4 (00870CD0 with EDX = id), state reset, the
  unit observation dropped, environment_unavailable.fsb into +1Ch.
- 0067F080 unit library register: UnitLibGui page and the preview scene (light (1, 2, 0)/sqrt 5,
  diffuse 0.6, ambient 0.2, motion blur and HDR blocks, shadow owner, VehicleClasses.lua and
  DeviceClasses.lua). The HDR block's field order follows the decompile's stack layout and the
  motion-blur block the listing; both are host data, not checked against the renderer.

## Eighth batch (cc13-fe2b): session messages

kSessionMessageClasses describes the seven classes: type, accepted base types of IsType (46h,
plus 49h for all but A0h), vtable, constructor shape and the payload fields Write/Read serialise
after the extended header (0075B480 / 0075B4C0):

| Type | Payload | Routines |
|---|---|---|
| A0h (160) | signed 4 bits +1Ch | 0064A770, 0064A7A0, 0064A7C0, 0064A7E0, 0064ADE0 |
| 94h (148) | none here (+1Ch byte cleared) | 0064A820, 0064A850, 0064AE00 |
| A1h (161) | bool +1Ch, bool +20h | 00650310, 00650360, 00650390, 006503C0, 00650430 |
| 82h (130) | bool +1Ch, 6 bits +20h, 3 bits +24h, optional 12-bit word +28h, 4 bits +2Ch (read only) | 006562F0, 006563A0, 00656420 |
| A7h..A9h (167..169) | bool +1Ch, 2 bits +20h, bool +24h | 006564C0..006567B0, constructors 00656570 (A8h) / 00656670 (A9h) |

All seven scalar deleting destructors restore the root vtable 00CE4974 and free on flag bit 0.
They sit beside the native session-message series (docs/NATIVE_SESSION_MESSAGE_BASE_R177.md) but
do not use its types; a later integration could fold them into that series.

## Handoff (cc13-fe2, 2026-10-12)

237 records (220 reconstructed in `src/ingame_gui_cc13.cpp`, 17 backfills), build-tested, call
report 632 rows / 0 failed. Ghidra was not written (the functions already carry reviewed names;
no new names were introduced, so no annotation batch was needed).

Update (cc13-fe2b): 0061FBE0, 00629D10, 0064BB90, 0067F080 and the session-message classes are now done (271 records in all); strike them from the lists below.

Remaining reconstruction candidates in the two segments (ledger name, no record, no tag), by area:

- Rewards: 0061F9D0 (debrief speech), 0061FBE0 (bind layout, see the fifth batch note),
  00621870 (listbox selection), 00621C60 (refresh page), 00622360 (vehicle unlocks), 00623710
  (date tokens), 00629D10 (register), 0062A970 (rank page).
- Scene init enter 00637080 (decompile confused by the stack; read the listing).
- Markers: 00638C60 (off-screen arrow, x87), 00639230, 006394B0, 006395E0, 00639810 (display
  range: the distance comes back on the stack), 0063C110 (unit widget set, 1296 bytes of finds:
  fe2_bind.py applies), 0063C620, 0063D1E0, 00640620, 00640A20, 00640D70, 00641910, 00641E30,
  00642040, 00642960, 00642C20, 00643360, 006434E0. Several are cited by cc9's
  game_hosts_hud_world.cpp as host calls; check that lane's leases first.
- HUD root award ticker 00648060 / 00648AB0; ship screen 0064AE20 / 0064B170 (x87) / 0064B370 /
  0064BB90; submarine 00650450, 00651370, 006515B0; unit icon frame 00652C80.
- Support manager: 006567D0, 0065A2B0, 0065BBC0, 0065D220, 0065FBF0, 00660B10, 00661240,
  006619E0, 00662470, 006735D0 (construct), 00674B80 (enter; outside this lease, next to cc9's
  air-operations range), record helpers 00668540 / 006697A0.
- Bomb and torpedo-boat view cameras 0067B4D0 / 0067C1A0; invite 0067D6E0, 0067D6B0; unit
  library 0067E9A0, 0067F080, 0067F4D0, 00680520.
- Session messages and STL instantiations: see "Not done in this pass".

Tools: `local/fe2_records.py` (records), `local/fe2_calls.py` (call report), `local/fe2_bind.py`
(page/find extraction; run with `--str` and check for two string pushes before one find),
`local/fe2_dec.py` / `local/fe2_asmf.py` (batch decompile and listing). They live in the worktree's
`local/` and are not committed.

## Update (cc13-fe10, 2026-10-13)

cc13-fe10 (`docs/CC13_FE_10.md`) tagged the 63 STL instantiations (`stl_shape`, evidence
`cc13fe10_stl`), defined the code listed under "Call sites" (six bodies: 0061F5E0, 00637590,
006375A0, 00646ED0, 0064A380, 0064A3C0; plus 00662690 found after the support-manager exit) and
reconstructed the rewards speech / selection / unlocks / rank page, several marker, HUD-root
ticker and support-manager routines, the invite acceptance, two unit-library listeners and the
torpedo-boat camera bind. Its Handoff section lists what is still open.
