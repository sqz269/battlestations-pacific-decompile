# Menu and HUD screens of segments 12, 15 and 19 (packet `cc13_menus4_segments`)

Worker cc13-menus4, 2026-10-11. Lease ranges 0056D9A0..00580530 (segment 19), 004F9D80..00519FE0
(segment 12) and 00527CB0..00544B60 (segment 15). Conventions follow docs/CC13_FRONTEND_MENUS.md:
base screen vtable 00CEAE54 slots +00h GetScreenId, +04h IsSelfManaged, +08h OccludesLowerLevels,
+0Ch scalar deleting destructor, +10h Register, +14h BindLayout, +18h Enter, +1Ch Exit, +20h Update,
+24h CollectChildren; listbox listener +04h activate (stack row, list), +08h selection changed;
widget listener +04h HandleGuiEvent. Every name is a hypothesis, not a recovered symbol.

Status words: *named* (ledger + Ghidra), *reconstructed* (C++ in `src/menu_screens_2.cpp`),
*build-tested* (`scripts/build.ps1` passes). Nothing here was run in `bsp_game`, and no
reconstruction is fixture-tested or game-validated.

## Functions Ghidra lacked

An `.rdata` scan for code pointers into the three ranges with no function start
(`local/m4_scan.py`, refs restricted to 00CE0000..00D00000 and 16-byte alignment) found 86 slot
targets. Extents come from recursive descent over the disk image (`local/m4_extent.py`); 0052BAA0 and
0053A8E0 each contain one `LEA ECX,[ECX]` alignment hole inside a loop and were checked by hand
(`local/m4_holes.py`). All 86 were defined with `tools/ghidra_define_function.py`; the record is
`reports/cc13_menus4_defined.json`. Data-looking hits outside the vtable area (refs at
00D2xxxx..00D5xxxx, unaligned starts) were not defined.

## Screen classes

| Screen | Id | Constructor | Primary vtable | Register | Enter | Exit | Update | Collect |
|---|---|---|---|---|---|---|---|---|
| MultiSessionBrowser (`INTF_MULTISESSIONBROWSER`) | 1Ch | 00574240 | 00CEF134 | 0056F320 | 0057A680 | 0056DCB0 | 0057B600 | 00573C00 |
| LoadingScreen | 59h | 0057C1C0 | 00CEF264 | | | | | 0057CF10 |
| TacticalLibrary (`INTF_ACHIEVEMENTS`) | 0Bh | 005098B0 | 00CEB9B4 | 0050FDB0 | 00511F40 | 0050B2D0 | 005162B0 | 00509DA0 |
| InGameScreen23 (manager +ACh) | 23h | 005178B0 | 00CEC194 | 005176A0 | 00517820 | 00517860 | 005178A0 (RET 4) | 00517690 (RET 4) |
| FrontEndFrame (self-managed) | 0Ch | 00517D10 | 00CEC39C | 00517ED0 | base | base | base | 00518E00 |
| HudBomber (manager +70h, inline) | 25h | inline | 00CEC590 | 00519830 | 00519120 | 00519160 | 00519BB0 | 00519D40 |
| HudCounters (self-managed) | 51h | 0052B030 | 00CED0AC | 0052CBF0 | 00528C40 | 00528C50 | 0052BAA0 | 0052D1D0 |
| Credits | | 0052FCE0 | 00CED200 | 0052FFC0 | 0052EE30 | 0052E150 | 0052E4B0 | 0052FD70 |
| MenuPrompt (self-managed) | 55h | 00533120 | 00CED3C4 | 005311C0 | 00531380 | 00530970 | 00532DC0 | 00532350 |
| HudFormation (occludes) | 2Ah | 00538E30 | 00CED6FC | 00538F80 | 0053A4D0 | 00536800 | 0053A8E0 | 00538D20 |
| FrameStats (self-managed) | 57h | 0053BA10 | 00CEDA10 | 0053C740 | 0053BA70 | 0053BA90 (RET) | base | 0053D220 |
| HudFreeCamera (`INTF_FREECAMERA`'s only screen) | 2Bh | 00540BA0 | 00CEDCA8 | 00540FE0 | 0053DD90 | 0053D280 (RET) | 00540390 | 00540380 |

Ids are the `MOV EAX,id; RET` bodies of slot +00h. Class names come from strings (credits:
`Scripts/datatables/Credits.lua`, `FE.main_credits`; frame stats: `Fps_Text`, `MinFps_Text`,
`MaxFps_Text` and the `PG_*` rows; browser: `FE.multi_browser_title`), from the existing names of
their Register slots (HudCounters, HudFormation, HudFreeCamera, HudBomber), or from
docs/IN_MISSION_INTERFACE_MANAGER.md (2Bh is `INTF_FREECAMERA`'s only level-1 screen). The id-23h
screen has no string; its enter sets the view FOV to 1.0 and observes the unit at +1Ch, so it keeps
a neutral name. The session-message vtable 00CEB138 (type 0DDh = 221, writer: byte +10h 8 bits,
+18h 6 bits, +1Ch 32 bits) and three singleton destructors were named alongside.

108 names were written (`local/m4_names1.py`): the slots above, 23 adjustor thunks
(`<dtor>_Adjustor<N>`), listener slots (browser 0057AEF0/005787E0/0057AF20/00573BD0, tactical
library 00517280/00515B40/00517300/00509D70, menu prompt 00532D70/00530640, formation 00533B30,
control layout 00527E30, credits 0052D200, three observer `OnReleased` slots), the shared
`BSP_GuiWidgetListener_Empty1C` (004FA170), the browser's `SetPage` 0057A0F0 and `PollSessionList`
0057B530 (its evidence was corrected later, see below). 0056BDA0 (browser BindLayout) lies in segment 18 and was named without a lease
conflict (unleased).

## Session browser (screen 1Ch)

Layout used by the reconstructions (2D0h bytes): +10h..+18h applied filter bytes and +19h..+21h
their edit copy (0056E360 copies applied to edit and reselects row 0 of +34h; 0057A910 copies edit to
applied, rebuilds rows with +2B0h and saves the nine bytes through 007FBA30 on game+650h); +244h..+24Ch
applied and +250h..+258h edit dwords; +265h refresh pending, +268h refresh countdown (10.0, 00CE38B8),
+26Ch selected row, +270h poll countdown (2.5, 00CEECEC), +274h, +278h search object (0056AC40),
+284h join elapsed (cap 12.0, 00CEECF4), +288h join cancel, +28Ch page (0 search, 1 results,
2 joining), +2A0h vector of 0A0h-byte session records. Widgets: +2Ch filter panel (virtual +38h is
"open", byte +85h "focused"), +30h result list, +34h filter list, +3Ch/+40h toggles driven by
+50h/+51h, +4Ch empty-list text, +74h, +88h, +D4h.

- **0057A0F0 SetPage** (`session_browser_set_page_0057a0f0`, reconstructed, build-tested).
  `__thiscall`(screen, page), RET 4, EBP frame. Stores the page, then does nothing unless the base
  bytes +04h and +05h are both set. Page 0 calls session +8Ch, builds the type mask 00E08730 from
  filter bytes (+14h 40h, +10h 1, +11h 10h, +13h 80h, +12h 20h) and starts the search through
  session +88h(00E08700); a pending refresh instead targets the selected record (00E08702 = 1, and
  00E08710/00E08714 from the record's object when its +04h byte is 0FFh), or does not search at all
  when nothing valid is selected. It then arms the poll timer and, for a fresh search, shows the
  please-wait screen (004B6E30 on *(00E19698)) with an empty command bar. Page 1 rebuilds the rows
  (005789C0(6), or 00576D20 when a refresh finds session +19Ch false), attaches the list listener
  (screen+8) and focuses the list, a toggle, or the empty text. Page 2 resets the join timer and
  shows please-wait with `globals.back` (command 0A3h, placement 2) as the fourth command.
- **0057A680 Enter** (reconstructed, build-tested): frame title `FE.multi_browser_title` (00518D60,
  ECX = 0Ah), row 8 of +34h and widget +74h follow the signed-in answer of session +198h, the
  game-type filter 00E086C0 (7, 4, 5, 6, <=3) selects one filter byte pair, SetPage(0) and, online in
  mode 6 or 7, SetPage(2). The list is cleared or focused only when its pointer is unchanged across
  the input tick (pointer read before 00A92C40, compared after).
- **0056DCB0 Exit** (reconstructed, build-tested): 00E086C0 = -1, filter list reset when +F0h.
- **0057B600 Update** (reconstructed, build-tested): `__thiscall`(screen, float), RET 4, EBX = 0.
  On results with a non-empty list the refresh countdown runs (paused while the +1E4h scroller is
  dragged) and at <0 restarts page 0 with the refresh flag. Then 0056DD80 and 0056E3E0 (per-frame
  filter and command-bar work, unread), one of three scrollers, action 4Ah (toggle the filter panel
  while +50h is set), back 4Bh (close the panel discarding edits, leave results through 00568CB0, or
  arm the join cancel). Page 0 polls (session +8Ch when the poll countdown ends, 0057B530 when
  session +90h is false). Page 1 maps 50h (refresh), 4Fh (gamer card: 00568D40 XUID, 00A3EAC0 user,
  stdcall 00A4D440), 46h, 47h (toggle +50h), 4Ch and 4Dh. Page 2 adds min(seconds, 0.1) to +284h;
  at 12 s it cancels (00688C70, request 1Bh and error kind 4 unless 004C5090(4Bh)), or finishes
  through 0057AC10 when 00E08700 is set; mode 5 does the same before the timeout; mode 7 with two or
  more occupied player slots (bytes game+750h + i*118h), game+18C8h >= 0 and 004B6BC0 pops the
  request and pushes 14h; mode 6 with 00A39750 == 217h calls 0076A7A0(+24h) and SetPage(2).
- **0057AE50 JoinSelected** and **0057AEF0 OnListboxActivate** (reconstructed, build-tested): online
  only, a selected index inside the record vector, 0057A9C0(record, 0).
- **0057B530 PollSessionList** (reconstructed, build-tested): takes the client's +94h result vector.
  When the dirty byte 00E08701 was set (it is cleared) and the vector is non-empty it joins result 0
  (00566370(0), 0057A9C0(record, 0)). Otherwise, with 00E08700 set, it resets +274h, clears the
  `std::list` at +278h (0056A090, relink, +280h = 0) and +288h and tail-jumps 0057AC10; otherwise it
  clears 00E08702 and shows the results (SetPage 1). This corrects docs/MULTI_MENU_SCREENS.md, which
  read the function as rebuilding the screen's own record list.

- **0056EE00 FilterValueDown / 0056EFB0 FilterValueUp / 0056ED20 SetFilterValueText** (reconstructed,
  build-tested): the selected row of the filter list +34h picks the edit byte +19h+row. Rows <= 4 (a
  signed compare, so no selection, -1, toggles +18h) and row 8 toggle a yes/no byte; rows 0..4 also
  clear 00E086C0. Rows 5 and 6 step +250h/+254h through 0..7 and row 7 steps +258h through 0..500 by
  100, wrapping both ways; the row byte mirrors count != 0. The row's `setting_value_Text` child then
  receives `globals.yes`, `globals.no` or the number (004260B0).
- **0056E3E0 UpdateCommandBar** (reconstructed as a selector, build-tested): filter list focused ->
  accept/navigate/change/cancel; +50h -> navigate/scroll_menu/`FE.change_filter`/refresh/back; result
  list not focused -> nothing unless +51h (navigate/scroll_menu/refresh/back); a valid selected
  session online adds `globals.join` (0A2h) when byte +264h is set and `globals.showgamercard`
  (0A5h) when signed in; otherwise navigate/refresh/back. The C++ takes the inputs pre-read; the
  original reads them in branch order and asks session +198h only on the selected-session path.

- **0057AF20 HandleGuiEvent** (reconstructed, build-tested): `__thiscall`(screen+0Ch, event), RET 4.
  Command buttons (event kind 3, command byte at *(event+F0h), default *(00E194B0)): 0A2h applies an
  open filter edit, opens the filter panel when +50h is set, or joins the selected session; 0A3h
  cancels an open filter edit or leaves the results (00568CB0); 0A7h refreshes (SetPage 0); 0A5h shows
  the gamer card when the list is focused and the user signed in. Other widgets: +40h/+3Ch select the
  toggle pair, the column headers +2B4h/+2B8h/+2BCh/+2C0h rebuild the rows with sort key 0/1/5/4,
  +BCh/+A8h/+ACh drive the +12Ch scroller (thumb, auto-scroll 2/1 at 0.002), +B8h/+B0h/+B4h the +188h
  scroller, +78h/+80h/+7Ch the result scroller +1E4h (after 0056AAD0, and they restart the refresh
  countdown), and with the filter panel open the row arrows `scroll_right_Icon`/`scroll_left_Icon`
  select their row (00A9C8F0 with event+D8h) and step it up/down. Non-returning paths end by
  highlighting the widget: kind 6 -> virtual +88h(2, 0, 1.0), kind 12h -> virtual +84h(2).

## Segment 18 browser helpers (second turn, lease `cc13_menus4_seg18`)

Containment of the listener slots 00568A30 (+0Ch) and 00568B60 (+18h) of vtable 00CEF100: 00568930
ends with `RET` at 00568A24 followed by INT3 padding (00568A25..00568A2F), and Ghidra's body of
00568930 holds exactly the 72 instructions of the descent, so both slots are separate functions.
They and the destructor 005692E0 (ref 00CEED6C) were defined. Reconstructed and build-tested:

- **00568930 LayoutFilterRows**: rows +54h..+74h at x = 00AA67F0(+F4h).x, y = 0.075 (00CEED68) plus
  the row step +25Ch per row (rounded to float each step); the content height (last visible row + 1)
  * step + 0.0111 (00CEED60) is applied to the +12Ch scroller (00683790, then range = height minus
  the +44h viewport height) only when it differs from +240h, which the enter resets to 0.
- **00568A30 OnWidgetReleased / 00568B60 OnWidgetHover**: end thumb drags (00683320) or auto-scroll
  (00683300 with 0) on the three scrollers, the result scroller restarting the refresh countdown;
  hover scales command buttons (virtual +4Ch, 0.5 hovered, else 1.0), selects toggle +3Ch from
  widget +114h while the list is focused, and redraws the rest by kind (6: +88h, 12h: +84h). The hover
  path's compare chain is reproduced literally (it can only skip the redraw for an event equal to three
  different widgets at once).
- **00568CB0 BackFromResults** (session +8Ch, mode 0, pop request), **00568D40 GetSelectedXuid**
  (record +98h/+9Ch), **00568DF0 SelectToggle**, **0056AA70 FocusList** and **0056AAD0 ReturnToList**.

## Smaller reconstructions

- **0052E150 Credits exit**: stops the credits stream (*(00E198AC)+54h, 00A86F40), restarts the menu
  stream (+50h) with the .data name 00E18E64 and fades it in, 00AA33A0(0) on the GUI manager, and
  deletes and nulls every item of the +28h pointer vector (virtual +4(1)).

- **0052E4B0 Credits update**: the float argument is overwritten with game+21ECh. Back (4Bh) pushes
  interface request (1, 0) on *(00E198AC). 46h/47h step the speed offset +20h by (double)0.1F, 4Ch
  sets it to -(+3Ch) (00D7A208 is -0.0), 4Dh to 0; clamp to [-1, 1]. The scroll +34h wraps to
  -height when positive, is written to the widget at +1Ch (00AA78F0) and moves by
  (+3Ch + +20h) * seconds; below -height it is reset to 0.
- **00517820/00517860** screen 23h enter/exit and **00519120/00519160** bomber enter/exit: observer
  registration on the unit at +1Ch (00694AF0 test, 00694A60 tail jump; ECX = unit, EDX = screen+8),
  0077C470(1FEh, 0) on exit; the bomber also binds/releases the weapon-group screen
  (*(00E198C4)+50h, 00549260/005470C0), sends (4, 0) to unit+9D4h and tail-jumps 0051E8E0 on
  *(00E198C4)+4Ch.
- **00528C40/00528C50/0052BAA0** HUD counters: show/hide *(+8h); update ticks +0Ch (no null test),
  every non-null entry of the +14h array (+18h count) and +10h with 0052B080, then 00529810.
- **00517280** tactical-library list activate: a row whose +28h text equals `99999`
  (00425850, case-insensitive) opens 00515E00; otherwise the float +438h is pulled to within
  0.0021 (00CEC168) of the index word +434h (the first dword of the +434h selector, whose
  0x6C-byte records 004FA890 indexes).

Tactical-library view mode: +90h is 0 for the 2D information panel and 1 for the 3D model view.

- **00511B40 ToggleView / 00514C20 SwitchView** (reconstructed, build-tested): 00511B40 flips +90h,
  hiding the panel widgets +508h/+50Ch/+510h/+518h for the model view and showing them again (then
  tail-jumping 00510C20) for the panel. 00514C20 toggles and refreshes the new mode: kinds 5/6 store
  the selected record's +48h field (004FA890 on +434h) in +4B0h and redraw the order-of-battle or
  unlocks page; kind 2 rebuilds the command bar with `globals.2dview` in the model view and
  `globals.3dview` in the panel; the model view then loads the vehicle model (0050AA10).
- **00515BC0 Leave / 00515E00 Back** (reconstructed, build-tested): online with both front-end
  managers present the screen acts only when 00687860(0Bh, 0) is true. In the model view (+90h == 1)
  they switch back to the panel through 00511B40 plus the order-of-battle (kind 5, 004FFF20) or
  unlocks (kind 6, 004FFD00) page or, for kind 2, the command bar (0, "", 0), (0A7h,
  `globals.3dview`, 1), (0B3h, `globals.navigate`, 1), (0B2h, `globals.scroll_menu`, 1), (0A3h,
  `globals.back`, 2). Back stops there; Leave (and Back in the panel) then pops the request interface
  online, or offline sets 00E08874 = 0Bh + (+9Ch != 0) and pushes interface request (1, 0).

## Front-end frame setters (segment 12)

The 164h-byte frame singleton (004C1AC0, constructor 00517D10, screen 0Ch) and its three handle
arrays are described in docs/GAME_TITLE_INIT.md. Reconstructed and build-tested:

- **00518D60 Select**: ECX = backdrop slot, EDX = panel slot, stack (title slot, string*), RET 8.
  Stores 00E18D84/00E18D88/00E18D8C, applies the selection, and sets the title when the string's
  length word is non-zero. This is the `set_frame_title_00518d60` that the menu packets call.
- **00518C80 ApplySelection**: each non-null handle of set +160h is shown exactly when its slot is
  the selected one (panel, then title, then backdrop; the backdrop pass and the tail jump to 00518B90
  are skipped when game+5D4h is 0Dh).
- **00518B90 SetFlag**: ECX = flag; stores 00E08654 and, when a panel is selected and loaded, calls
  `frame_flag_Icon` virtual +88h(word flag, 0, 1.0).
- **005189B0 SetTitle**: submits the string to `title_Text` and sizes `szarnyas_FrameBox`: width =
  extent(+114h)/960 + 0.2667 when that quotient exceeds 0.2667, else 0.5333; 0.7 when game+61Fh or
  game+634h is set; the height is kept (00AA6740 +4).

A second batch of 35 names (`local/m4_names2.py`) covers the browser helpers (filter edit begin /
apply 0056E360/0057A910, filter rows, command bar, row builders, JoinSession 0057A9C0), the tactical
library loaders (PowerUpLib, NoseArt, movies, TutorialLib, UnitLib), its model preview (0050AA10,
0050A950), the order-of-battle and unlocks pages, MenuPrompt Register/Exit (005311C0/00530970) and
the frame-stats formatter 0053BCC0. Most are provisional (string and caller evidence only).

Call sites: `reports/cc13_menu_screens_2.json`, 256 rows, `tools/verify_report_calls.py`: 0 failed.

## Uncertainty

- Action ids 46h..50h are not mapped to buttons; the descriptions are by effect.
- Session-client virtual slots (+88h, +8Ch, +90h, +94h, +198h, +19Ch) are named by their use here.
- The browser's row builders (00576D20, 005789C0, 00579980, 00577650), 0056DD80, 0056E3E0,
  0057A9C0, 0057AC10 and the HandleGuiEvent 0057AF20 were not read in full.
- Screen 23h is not in any interface list of docs/IN_MISSION_INTERFACE_MANAGER.md's table.

## Handoff (cc13-menus4, 2026-10-11)

Commits on agent/cc13-menus4 (see `git log`): slot definitions and 108 names, 35 helper names plus
the frame setters, the browser filter/command-bar work, HandleGuiEvent/credits exit, and the poll and
view-toggle batch, with origin/main merged once. `scripts/build.ps1` passes; 233 call rows, 0 failed.
30 functions reconstructed and build-tested in `src/menu_screens_2.cpp` (12 browser, 2 credits, 4
observed-unit screens, 3 HUD counters, 5 tactical library, 4 front-end frame); none ran in `bsp_game`.
Helper scripts are in the worktree's `local/` with the `m4_` prefix (`m4_scan.py`, `m4_batch.py`,
`m4_ctor.py` vtables per constructor, `m4_info.py` one-line function summaries, `m4_holes.py`,
`m4_rd.py` image constants, `m4_names1.py`/`m4_names2.py`).

Open, in order of value:

1. Session browser: JoinSession 0057A9C0 (lobby options copy, the `sessionname` Lua global, a
   session block copied into *(game+207Ch)+96Ch..+9A4h, mode changes) and FinishJoin 0057AC10; the row
   builders 00576D20/005789C0/00579980/00577650; filter rows 0056D9A0/0056DD80; PrepareEnter 00570300;
   selection-changed 005787E0. The browser's segment-18 helpers (00568930 layout, 00568CB0 back from
   results, 00568D40 XUID, 00568DF0, 0056AAD0, 0056AA70, 0056AC40, 0056B610, BindLayout 0056BDA0) are
   outside these leases; segment 18 (00568CB0..0056D660) is unassigned. The widget-listener slots
   00568A30 (+0Ch) and 00568B60 (+18h) of vtable 00CEF100 lie inside Ghidra's body of 00568930: check
   containment (two checks: range and padding) before defining them.
2. Tactical library: Register 0050FDB0, Enter 00511F40 (11488 bytes), Update 005162B0 (4048),
   HandleGuiEvent 00517300 (six-way switch at 00517600), selection changed 00515B40, the loaders.
3. Credits Enter 0052EE30 and Register 0052FFC0 (Credits.lua, item styles 0052D9C0).
4. HUD screens: formation Enter/Exit/Update (0053A4D0, 00536800, 0053A8E0, 4333 bytes), frame-stats
   Register 0053C740, menu-prompt Exit 00530970. The free-camera Enter/Update (0053DD90, 00540390)
   build camera matrices; coordinate with the camera lane first.
5. Left unnamed: 005347F0 (vtable 00CED548 slot +0Ch, tests 78h/49h/46h), 00519340 (unit vtables
   +130h, returns +3ACh), 0057D3E0 (00CEFC24 run, `XOR AL,AL; RET 8`).

## Handoff update (cc13-menus4, second turn, 2026-10-11)

0334b0af1 adds the segment-18 browser helpers (8 reconstructed, 3 functions defined, 9 names) and the
dated 0057B530 correction appended to docs/MULTI_MENU_SCREENS.md; 38 functions are now reconstructed and
build-tested, 256 call rows, 0 failed. Item 1 of the handoff above is done for 00568930..0056AAD0 and
the two listener slots. Still open, in order:

1. FinishJoin 0057AC10, first reading from the pseudocode (asm not read): when +288h is set it cancels
   (session +8Ch, mode 0, pop request); it then walks the client's +94h results, looks each record's
   first dword up in the tree at +278h (0056B850, header +27Ch) and, for a key not yet in the tree that
   00574100 accepts, stores 00E08760 = -1.0, inserts the key (00571FD0) and joins that record
   (0057A9C0(record, 0)). With no candidate it pops the request when 006878D0 > 0, sets 00E19610 = 1,
   pushes request 10h, copies byte *(00F8A2FC)+4Dh to game+2059h, returns to page 0 (+270h = 2.5,
   +274h = -1) and clears the tree. So +278h is the set of sessions already tried.
2. JoinSession 0057A9C0 (lobby options 00575xxx copy, sessionname Lua global from the record's wide
   name, session +8Ch, mode 2, then for record +04h == 0FFh a session block copied into
   *(game+207Ch)+96Ch..+9A4h via 00568CE0, mode 6 and SetPage(2); otherwise 007701F0 and SetPage(2) or
   the error path with request 1Bh).
3. The row builders 00576D20/005789C0/00579980/00577650, BindLayout 0056BDA0, 0056B610, 0056D660 (filter
   value refresh, ERROR!), PrepareEnter 00570300, selection changed 005787E0.
4. Credits enter 0052EE30, tactical library enter 00511F40 / update 005162B0, the free camera
   (0053DD90, 00540390; the camera conventions are in docs/CC13_CAMERA_CONTROLLERS.md, only cc9's
   hud_movie_camera.cpp is off-limits).
## Session browser join path, rows and layout (cc13-menus5, 2026-10-11)

Reconstructed and build-tested in `src/menu_screens_2.cpp` (none ran in `bsp_game`; the multiplayer
session layer is reconstructed only, not wired). Call sites are in `reports/cc13_menu_screens_2.json`.

| Address | Name (hypothesis) | ABI | Evidence |
|---|---|---|---|
| 0057A9C0 | JoinSession | `__thiscall(screen, record, arg)`, RET 8; arg never read | asm 0057A9C0..0057AC09; EDI = 2 from 0057AA92 is both the network mode and the SetPage argument at 0057ABEC |
| 0057AC10 | FinishJoin | `__thiscall(screen)`, RET | asm; the tree clear is the inlined `_Tidy` loop (0057AE10..0057AE2C: 11 bytes at 0057AE23 Ghidra left undisassembled because `_free` is marked no-return; bytes `83 C4 04 80 7E 11 00 8B EE 74 E2`) |
| 00574100 | BSP_SessionRecord_IsJoinable | `__thiscall(record)`, SEH 00C6F1B9 | decompile + asm: the two 00574050 calls take ECX = game+650h (the profile, "RANK" 00CEF15C) |
| 00568490 | BSP_SessionRecord_HasRequiredContent | `__thiscall(record)` | decompile (two mask subsets) |
| 00568CE0 | BSP_SessionSearchResult_CopyConstruct | `__thiscall(dst, src)`, RET 4 | decompile; vtable 00CEED04 |
| 00A39B80 | BSP_NetworkClient_Connect | `__thiscall(client)` | string "cNetworkClient::Connect() --> XSessionCreate"; pSessionInfo = client+8A4h |
| 007701F0 | BSP_Session_SelectListedServer | `__thiscall(game+1EF0h, index)`, RET 4 | decompile; provisional |
| 006878D0 | BSP_InterfaceManager_RequestCount | `__thiscall(mgr)` | decompile (vector +5Ch size) |
| 00576D20 | UpdateSelectedRow (was BuildRowsDefault) | `__thiscall(screen)`, SEH 00C6F43B | asm 00576D20..0057763D |
| 005789C0 | BuildRows | `__thiscall(screen, sort key)`, RET 4, SEH 00C6F5C0 | asm 005789C0..00579972 |
| 0056A870 | BSP_SessionRecord_LessBySortKey | `__thiscall(key*, a, b)`, RET 8 | decompile (jump table) |
| 00576BE0 | RecordPassesFilters | `__thiscall(screen, record)`, RET 4 | decompile + x87 asm 00576CD2..00576D17 |
| 00578320 | BSP_SessionBrowserRow_SetConnectionIcons | `__fastcall(group, value)`, SEH 00C6F4A8 | asm; JG at 0057837E hides the icon |
| 0056BDA0 | BindLayout | `__thiscall(screen)`, slot +14h of 00CEF134 | asm; 17 listener calls (screen+0Ch, 0) |

Findings:

- The 3Ch-byte block JoinSession writes at `*(game+207Ch)+96Ch` is the client's `XSESSION_INFO`
  (client = +C8h, so +8A4h, the pointer 00A39B80 hands to `XSessionCreate`): session id +96Ch, XNADDR
  +974h..+997h (REP MOVSD of 9 dwords), key exchange key +998h..+9A7h. The source object (00568CE0)
  keeps them at +04h, +1Ch and +0Ch.
- Record layout additions (0A0h bytes): +08h wide session name (`std::wstring`, buffer +0Ch, capacity
  +20h), +24h open slots (> 0), +28h players, +2Ch max players, +30h (filter byte +17h, upper bound
  screen+24Ch), +34h (sort key 4), +38h connection value (ping icons), +48h/+49h rank range, +4Ch 14
  lobby option indices (+54h = game mode, kind 2), +88h/+8Ch required content masks, +90h/+94h ratio.
- `screen+278h` is a `std::set` of record keys (record+00h) already tried; FinishJoin inserts before
  joining and clears it when no candidate is left. The poll 0057B530 clears the same set through
  0056A090 (the host hook `clear_search_list_0056a090`).
- 00576D20 is not a row builder: with exactly one client result it replaces the stored record with the
  same session id (insert before + erase, leaving the 0A0h temporary from `operator new` unfreed in
  the listing) and rewrites only row +26Ch; otherwise it greys that row (0.5, 0.5, 0.5, 1).
- Row colours: joinable (249/255, 219/255, 0.6, 1) from 00CEF16C/00CEF168/00CE3D30; locked
  (1, 0, 0, 1) static 00E19498.
- Sort: `+2B0h` key, reverse flags 00E1947C[key] toggled when the same key is chosen again, header
  icons +2B4h/+2B8h/+2BCh/+2C0h for keys 0/1/5/4 get state 3 (reversed) or 2 through widget +88h.
- BuildRows reads the list's selected-row name and `_stricmp`s it per row, but the result is discarded
  (00579817..00579835); only the +26Ch row name picks the new +26Ch.
- BindLayout widget map: +2Ch filters page, +28h browser page, +EC menu_line_Group, +34h
  Settings_Listbox, +128h servers_fix_Group, +38h servers_FrameBox, +4Ch noserver_Text, +84h
  servers_Clipbox, +30h servers_Listbox, +E8h listbox1_Group (row template), +88h slider_pos_Group,
  +118h slider_Group, +78h slider_FrameBox, +7Ch ScrollUp_Icon, +80h ScrollDown_Icon, +90h back_Icon
  height, +8Ch clip height, +120h settings_Group, +40h settings_FrameBox, +48h setting_Clipbox, +124h
  setting_template_Group, +A4h rank_Icon, +94h setting_Text, +98h setting_var_Text, +D0h
  game_mode_var_Text, +A0h map_Icon, +9Ch tilt_Icon, +B8h/+B0h/+B4h settings scroll frame/down/up,
  +114h filters_Group, +44h setting_Clipbox, +FCh textbox_Group, +100h moving_Group, +F4h/+F8h filter
  row templates, +BCh/+A8h/+ACh filter scroll frame/down/up, +3Ch New_FrameBox. Scrollers: +1E4h
  (content +30h), +188h (content +124h), +12Ch (content +100h).

Uncertainty: the record fields +30h, +34h, +90h/+94h and the registry +B8h/+BCh have no semantic
anchor; 007701F0's server list is unexamined beyond the copy; the x87 ratio in 00576BE0 is modelled
in double (the original keeps an 80-bit intermediate); `std::sort` stands in for the introsort
00574A50, which agrees except for the order of equal keys.

## Credits Enter (cc13-menus5)

`credits_enter_0052ee30` (slot +18h of vtable 00CED200, `__thiscall(screen)`, frame 89Ch, SEH
00C6BD92), reconstructed and build-tested; evidence is the listing 0052EE30..0052F90D, read whole.

- Music: creates the manager's +54h stream when it is null (operator new 54h, 00A877D0 with a copy of
  manager+48h), stops +50h, starts +54h with the string at 00E18E64 and fades it in; then
  00AA33A0(1) on the GUI manager, title `FE.main_credits`, command bar (A3h, `globals.back`, 2).
- Item rows (24h bytes, 0052D9C0): +00h Type (default 1), +04h Text, +0Ch Color (default white),
  +1Ch Offset, +20h Align (default 2). Types: 0 category clone (+10h), 1 person clone (+14h), 2 the
  `<Text>_Icon` child of +1Ch cloned, 3 blank line (height 1/36), 4 vertical gap (y += Offset, height
  -0.0 - RowSpace), 5 indent +48h = Offset, 6 same line (+48h = previous width + indent, y = row y).
- Person texts containing `%` are widened and passed as a format to vswprintf (0052D1E0) with 2122h;
  this installation's Credits.lua uses it twice, in `"WBIE LOGO, WB SHIELD: %c & (c) Warner Bros.
  Entertainment Inc."`, so `%c` becomes the TRADE MARK SIGN.
- Placement: x by Align from the +18h frame (00AA6750 origin, 00AA6740 size): left + indent, right
  (origin + width - item width - indent), centre (origin + width/2 - item width/2); other Align
  values leave x at 0.05 (00CE7638). Position (x + Offset, y, -100). Text width = widget+114h / 960.
  Row step y += RowSpace + height. Screen +40h is StartY and +44h RowSpace (the constructor's 0.5
  and 1/72 are defaults that Register overrides from Credits.lua, not re-read here).
- After the loop +38h = y (the update's scroll limit), frame +18h vtable +70h, +20h = 0.

Uncertainty: 004C5E60's narrow-to-wide rule was not traced (modelled as byte zero-extension); the
`%` formatter models only `%c` and `%%`. (Resolved by cc13-menus6: Register stores
`clipbox_Clipbox` at +18h and its child `moving_Group` at +1Ch.)

## Tactical library Enter (cc13-menus5)

`tactical_library_enter_00511f40` (slot +18h of vtable 00CEB9B4, `__thiscall(screen)`, 11488 bytes,
SEH 00C6A616), reconstructed and build-tested. Ghidra decompiles it but warns "unable to track
spacebase fully" and drops ten blocks as unreachable, and most widget calls lose their ECX; the
reconstruction follows the listing (`m5_dis.py` 00511F40..00514BD6), using the pseudocode only for the
order of the per-kind calls and the five-triple command-bar argument lists, whose string pushes were
matched in the listing.

- Kinds (+94h) from jump table 00514BD8: 0 personal achievements (template +4F4h), 1 power-ups
  (+4F8h), 2 units and weapons (+4ECh), 3 movies (+4F0h), 4 naval academy (+500h), 5 order of battle
  (+4ECh), 6 unlocks (+4ECh, help line only, no frame title), 7 none, 8 nose art (+504h). Each case sets
  `FE.taclib_<kind>_title` through 00518D60 (ECX = -1, EDX = 0, title slot 0), `FE.taclib_<kind>_help`
  through 0054A0C0, runs its list builder and rebuilds the command bar.
- A pending entry (+9Ch non-null, +A0h != 2, kind != 7) switches to kind 6 with 0050E3C0 when +A4h is
  set, else kind 5 with 0050EF80. Kind 4 first runs `luaLoadControlFunctionNames()` (0 = success; the
  two Hungarian diagnostics say "executed / could not execute ... in GIAchievements") and marks the
  academy records (vector +404h, 6Ch bytes, byte +44h): all when there is no entry, otherwise only the
  entry's ids (*(entry+8Ch) vector, index lookup 004FAAC0); +3C0h becomes the last marked record's
  +40h and is handed to 00511BF0.
- Carousel: three clones of the kind's template at +508h/+50Ch/+510h, scales (0.5, 0.5), (1, 1),
  (0.5, 0.5) through widget +48h, x = centre - 0.46, centre, centre + 0.46 (+43Ch = 0.46 from
  00CEBE7C, +440h = the centre clone's local x); per-panel lookups pic_BG_Icon (+580h stride 0Ch),
  icon_big_BG_Icon (+57Ch), icon_big_BG2_Icon (+584h, non-recursive), keret_Framebox (+5A0h..), and per
  kind lock_Icon (0), video_Movie plus a pic_BG_Icon clone (4), pic_Hide_Icon (3).
- The data list per kind for 0050D900(list, screen+444h): 0 +3D4h, 1 +3E4h, 2/5/6 +3F4h, 3 +3C4h, 4
  +404h, 8 +414h. Kind 8 then runs 004FC8C0, kind 6 004FBE90(1). +5B4h = -1 on every exit.

Uncertainty: the list builders (005012F0, 00500E90, 00500860, 00500450, 005010C0, 00506660, 005001A0,
00500640) are named from their single call sites only; 00502480 (HDR_, ReflectionCubeMap) is the 3D
preview setup shared with the front-end preview and Register; 00B33AC0 (ECX = *(00F8D394), arg 1) is
unexamined. The Update 005162B0 was reconstructed by cc13-menus6 (see below); menus5 left it at (register-dependent loop bodies,
`unaff_ESI`/`unaff_EDI`, x87 row placement); see the handoff.

## HUD free camera Enter and Update (cc13-menus5)

`free_camera_enter_0053dd90` and `free_camera_update_00540390` (vtable 00CEDCA8 slots +18h and +20h),
reconstructed and build-tested; read from the listings 0053DD90..0053DF3B and 00540390..00540AE7 (every
x87 sequence and register argument checked there). They reuse `unit_step_towards_0042ac60`
(`unit_motion.hpp`) and `wrapped_angle_add_00438aa0` (`unit_rudder.hpp`); 0042D2E0 is the existing
`extract_camera_matrix_angles_0042d2e0` (outputs X, Y, Z = EDX, then the two stack pointers).

- Enter: mover +24h created once (operator new 900, memset, 00432750; 00923870(mover, 0, game+19CCh,
  identity); 00694A60 with EDX = screen+8). Source pose: the saved matrix 00E192B0 once when byte
  00E192A9 is set (cleared), otherwise `*(game+19FCh)+F0h` after 00B6DB70 when that node's +5Ch bit 1 is
  clear. 004134F0 copies it into mover+74h; angles to +54h/+58h/+5Ch; mover +C8h/+10Ch cleared and
  0042ED50 run on its child chain (+48h, next +44h); 004BC410 installs it; +38h..+48h = 0.
- Update (RET 4): returns unless +04h and +24h. Axes from the input owner (004BEC00()+4): forward
  +1164h x 200, strafe +1194h x 100, vertical +11C4h x 100, yaw +11F4h x 1.7453 (100 deg), pitch +1224h
  x 1.7453, roll +1344h x 0.7854 (45 deg). Each of the first five drives a smoothed field (+38h, +3Ch,
  +40h, +44h, +48h): opposite sign -> 0, smaller magnitude -> taken, larger -> 0042AC60 by
  dt x scale x +34h. Velocity = +38h x row2 (+ECh) + +3Ch x row0 (+CCh) + +40h x row1 (+DCh) of the
  mover world matrix; mover+A4h += dt x velocity. Angles re-read from mover+74h; control +1260h steps
  X toward 0, control +1290h steps X, Y, Z toward 0 (pi/2 per s); otherwise X += +48h x dt (wrapped,
  clamped to +-1.5533 = 89 deg), Y += +44h x dt, Z += raw roll x dt. 00467050(X, Y, Z) on mover+74h.
  Action 61h (004C43C0) calls 0053D230 with the world translation (it forwards it to the object at
  00E188D8: vtable +5Ch(6), then +118h(position)). Then 0042ED50, 004329D0(dt), +64h = +20h and the
  burst container 0053F2D0(dt, mover+CCh) at +64h. With no player units (game+1970h == 0) it calls
  the printf-style debug sink 004C7070(game, 750.0, 10.0, "NO MORE PLAYER UNITS") every update.

Uncertainty: the axis roles are named from the motion they drive (pitch is the clamped X angle); the
vectors are summed in float as stored by the original but the x87 products carry extended precision
before each store (a double-rounding difference is possible in rare cases); 0053D230's target object
(00E188D8) and the burst container are not examined.

## Handoff (cc13-menus5, 2026-10-11)

Commits on agent/cc13-menus5: 849e54ee8 (browser join, rows, layout), fc41dd662 (credits Enter),
b9d599c39 (tactical library Enter) and the free-camera commit after them; menus4's e2779b1d4 was merged
first. 9 functions newly reconstructed and build-tested here plus 5 helper reconstructions (00568490,
00574100, 0056A870, 00576BE0, 00578320), 30 names, 613 call rows, 0 failed; none ran in `bsp_game`.
Helper scripts in the worktree's `local/` with the `m5_` prefix: `m5_dc.py` (uncapped, noise-filtered
live decompile), `m5_bind.py` (one line per find/load/vcall from an `m5_dis.py` listing),
`m5_rows.py` (adds host_steps rows for direct calls), `m5_names*.py`.

Open, in order of value:

1. Tactical library Update 005162B0 (4048 bytes): decompiles, but the order-of-battle (kind 5, vector
   +460h, 2Ch records) and unlocks (kind 6, vector +470h) row loops lose ESI/EDI and position rows with
   x87 through 00AA8240; read the listing. Kind 3 stops a playing movie on action 4Bh and restarts the
   menu stream 00E18D78; kind 7 runs 00514FB0 on 4Bh; +5B4h counts down and then launches the selected mission (BSP_MainMenu_PlaySelectedMission, game+6ACh/+6B0h = the list selection).
2. Session browser RefreshRowDetails 00577650 (3280 bytes; map name from registry +34h via
   FE.unknown_mapname, the settings rows 00569FF0, `_atol` over a 00E198AC+5Ch+24h list) and the
   uncalled 00579980 (row-builder sibling); PrepareEnter 00570300, selection changed 005787E0,
   0056D660 (filter value refresh, ERROR!), 0056B610.
3. Credits Register 0052FFC0 (Credits.lua, StartY/RowSpace/Speed, which of clipbox_Clipbox and
   moving_Group lands at +18h/+1Ch) and ReadItemStyle's `Align` read, which the pseudocode shows
   writing through a null pointer (check the listing).
4. HUD screens from menus4's list: formation Enter/Exit/Update (0053A4D0, 00536800, 0053A8E0), frame
   stats Register 0053C740, menu-prompt Exit 00530970.

## Tactical library Update, carousel step and nose-art apply (cc13-menus6, 2026-10-11)

Reconstructed and build-tested in `src/menu_screens_2.cpp` (`tactical_library_update_005162b0`,
`tactical_library_carousel_step_00516010`, `tactical_library_apply_nose_art_005161c0`); none ran in
`bsp_game`. Evidence is the listing, read whole (`m6_m4dis.py` 005162B0..00517276, 00516010..005161B6,
005161C0..005162A0); the pseudocode was used only for the block order.

| Address | Name (hypothesis) | ABI | Evidence |
|---|---|---|---|
| 005162B0 | BSP_TacticalLibraryScreen_Update | slot +20h of 00CEB9B4, `__thiscall(screen, float dt)`, RET 4, SEH 00C6A7E0 | listing; ESI = screen throughout |
| 00516010 | BSP_TacticalLibraryScreen_StepCarousel | `__thiscall(screen, bool forward)`, RET 4 | listing; called on actions 4Ch (1) / 4Dh (0) |
| 005161C0 | BSP_TacticalLibraryScreen_ApplyNoseArt | `__thiscall(screen)`, RET | listing; called on 4Ah in kind 8 |

Register and stack recovery (what the decompiler lost):

- Kind 5 (order of battle) row loop 00516481..00516849: `[esp+10h]` is the row counter (the value the
  pseudocode shows as `unaff_ESI` written to widget+D8h), EDI the record index parsed by `_atol` from
  the row's data text (row virtual +28h, then +4), EBP = screen+45Ch (the vector object; begin +460h,
  end +464h, 2Ch records). The arrow widgets +1D8h/+278h are indexed by the record, not by the row.
  004FB090 gets record+00h for every row.
- Kind 6 (unlocks) loop 0051685B..00516B10: EBP is the row and record index, EDI walks screen+1D8h
  (EDI-0A0h = +138h, EDI+0A0h = +278h) and `[esp+10h]` is the byte offset into the +470h records.
  The image does not bound the 40-entry widget arrays; the reconstruction stops at 40.
- x87: dt stays in ST0 from 005162D9 until the +5B4h test consumes it (FSUBR/FSTP, or FSTP ST0 at
  00516389). Left arrow x = origin.x (spilled as a double at `[esp+34h]`) - own width / 1.2
  (00CEC160); right arrow x = origin.x + the GUI manager's highlight widget width (00AA0F50(1), then
  00AA6740); y = origin.y - 0.018 (00CEC158, kind 5) or 0.023 (00CEC150, kind 6); z = -10 (00CE6848)
  in kind 5, the origin z in kind 6. The origin is fetched twice per row (once per arrow).
- Arrow frames through virtual +88h(frame, 0, 1.0f): 4 locked (kind 5, record without children and
  +0Ch == 1), 3 disabled (left: +138h[i] == 0; right: == child count in kind 5, == count - 1 in kind 6),
  otherwise virtual +94h is read twice and a frame outside 1..2 is reset to 0. Record +0Ch == 0 hides
  both arrows in kind 5; an empty child vector (record +18h, begin +1Ch, end +20h) hides them in kind 6.

Control flow: +8Ch counts down while positive; +5B4h (-1 = idle, 00D7A260) counts down and at <= 0
stops the loading queue, resets the GUI screens, stores the +C4h list selection in game+6B0h and
+6ACh and calls 005922F0 (ECX = *(*(00E198AC)+58h)). Kind 3 with a playing movie and action 4Bh stops
it, restarts the menu stream +50h with the empty name 00E18D78 and returns. Kind 5 runs
`profile_hints(6)` and returns through 004FF6F0 on 4Ah. Kind 7 runs 00514FB0 on 4Bh, arms +5B4h = 0.5
on 4Ah (only when idle), then `profile_hints(7)`. In the 3D view (+90h == 1) 4Bh toggles back and shows
the kind 5/6 page or, for kind 2, the bar (0,"",0)(A7h 3dview)(B3h navigate)(B2h scroll_menu)(A3h back).
Otherwise one of: console flag 00F88A30 + action 50h in kinds 2/5/6 -> 00514C20; +90h == 0 and 4Bh ->
Back when list byte +85h is set, else list virtual +60h(1) and the kind 5 page or the bar
(A2h continue,0)(0,"",1)(B3h navigate)(A3h back)(0,"",1); 50h in kind 5 -> 00511BF0(atol of the selected
row) and the bar (A2h continue,0)(AFh change)(B3h navigate)(A7h 3dview)(A3h back); 4Ah -> 004FF580
(kind 3), 005161C0 (kind 8), Back when 004FC9D0 is true (kind 6); 4Ch / 4Dh -> 00516010(1 / 0) unless
kind 7. Every non-returning branch then runs 00515050(dt) unless kind 7, and kind 8 hides the three
keret frames +5A0h..+5A8h (virtual +84h(0)) and shows the one whose panel (+508h/+50Ch/+510h,
virtual +38h) shows the profile's nose art: game+730h when the current profile's +28h is non-zero,
else +734h, compared with record +48h from 004FA8F0 / 004FA960 / 004FAA00 on the +434h selector; the
left panel also needs +438h > 1.0.

00516010: returns for kind 7 and, in kind 4, while +8Ch > 0; sets +8Ch = 0.5, steps the selector
(004FC720 forward, 004FC7A0 back), 00510C20, 00506550 (kind 5) or 00504C60 (kind 6), 00515050(0.001),
detaches the scroller content and sets its range to (text height of +A8h + (float)(1/72 in kind 4,
else 0)) - +5ACh height, the pad narrowed to float in the argument slot first.

005161C0: record +434h of the 6Ch-byte nose-art vector +448h, field +48h, goes to game+734h when the
library list row (*(*(00E198AC)+70h)+B8h) parses to 0, else game+730h; then 007F8690 on the current
profile game+18CCh[game+18ECh] with {+734h, +730h, 0} and Back.

Uncertainty: the precision of the x87 arithmetic is taken as the CRT's 53-bit default, so double
arithmetic stands in for it; 00AA0F50(1) is named "highlight widget" only by its existing ledger name;
the callees 004FB090, 004FF6F0, 00514FB0, 00515050, 004FF580, 004FC9D0, 004FC720/004FC7A0 and the
selector record readers are unexamined. The `invalid_parameter` checks return in the reconstruction
where the CRT handler aborts.

## Session browser RefreshRowDetails and the uncalled row appender (cc13-menus6)

`session_browser_refresh_row_details_00577650` and `session_browser_append_record_rows_00579980`,
reconstructed and build-tested. Evidence: listing 00577650..005782E3 and 00579980..0057A0C9, read whole
(the decompile of 00577650 fails to track the stack).

- Record: list +30h selected index + (list byte +148h ? list+164h : 0) into +2A0h. Out of range ->
  map text "-", map icon frame 0 with UV (0,0,1,1) (00568290 inlined), tilt icon +9Ch shown, rank texts
  "-" with their icons hidden, ratio text "-", icon +E4h hidden, ratio text shown by
  `*(00F8A2FC)->vtable+198h()`.
- Map name: registry 00576B10 +34h, option key 1 (`[esp+18h] = 1` before 00575E80), inner map copied
  (00573340) and searched (0056B8C0) for record+24h; found -> 005732C0's string, else
  `FE.unknown_mapname`; submitted to +D0h with arg 1. screen+264h = 00568490 on the record re-fetched
  with a second selection read.
- Map icon: registry +1Ch option 1 (00575FD0, 0056B930); missing -> frame 0, UV (0,0,1,1), tilt shown.
  Present -> frame = index*8 - 7; the 434h-byte map definitions (*(00E198AC)+5Ch, vector +24h) are
  searched for the one whose name string (+00h) `_atol`s to record+24h. Game mode record+54h (jump
  table 005782E4) picks an extent and a frame offset: modes 0..3 by record+58h (table 00578304:
  +364h/+6, +37Ch/+5, +394h/+4, +3ACh/+3), 4 +3F4h/+1, 5 +40Ch/+2, 6 +3C4h/+7, 7 +3DCh/+0. UV =
  ((x0+15000)/30000, 1-(z0+15000)/30000, (x1+15000)/30000, 1-(z1+15000)/30000) from the extent's
  corners (+0/+8 and +0Ch/+14h); aspect = |x0-x1| / |z0-z1|; size (aspect*150/960, 150/720) below 1,
  (150/960, 150/aspect/720) above 1, (0.15625, 0.2083) at 1. Then tilt hidden and frame applied.
- Ranks +48h/+49h: 1..20 -> icon +DCh/+E0h frame rank-1 shown, text +C4h/+CCh hidden; else text "-".
- Ratio: when record+90h > registry+B8h, "N %" (N = `_ftol2`(+94h/+90h*100), unsigned FILD fix-ups)
  in +D8h, red (00568220, static 00E19498) when +94h/+90h > registry+BCh, else the joinable colour;
  otherwise "-" in the joinable colour.
- 00579980 has no caller: E8/E9 rel32 and absolute-dword scans of the image find nothing (the same
  scan finds BuildRows' 8 call sites). It appends one row per +2A0h record exactly as BuildRows' row
  body, without clearing, filtering, sorting, the +30h/+34h floats or the selected-row match.

Finding: 00577650 uses record+24h as the map option value (registry key 1 and the map definitions'
names), so the field menus5 labelled "open slots (> 0)" from IsJoinable is more likely the map id;
the field keeps its name in the header until the join path is rechecked.

Uncertainty: modes outside the tables reach 00577BE3 with the extent locals uninitialised (stack
slots last holding iterators); the reconstruction skips the UV and size update there. Whether the
map-name lookup inserts key 1 (00575E80 may be `operator[]`) is not modelled.

## Credits Register and ReadItemStyle (cc13-menus6)

`credits_register_0052ffc0` and `credits_read_item_style_0052d9c0`, reconstructed and build-tested;
evidence listing 0052FFC0..005305F9 and 0052D9C0..0052DBB2.

- Register: 004F71D0, page `FE_credits` -> +0Ch, `category_template_Text` (+10h) and
  `person_template_Text` (+14h) found recursively in the page and hidden, `clipbox_Clipbox` -> +18h,
  `moving_Group` (searched under +18h) -> +1Ch. `Scripts/datatables/Credits.lua`, global `Credits`:
  `Speed` -> +3Ch, `StartY` -> +40h, `RowSpace` -> +44h (floats, type 2). `Credits_Items[i]` for
  i = 1.. while the index exists: 0052FF10 pushes a row with an empty text onto +4Ch, the reader
  enters index i (virtual +4h), 0052D9C0 fills `back()`, the reader leaves (virtual +8h).
- ReadItemStyle writes every default first (Type 1, colour from the white static 00E18E50, Align 2,
  Offset 0), then reads Type (int), Text (string), Offset (float), Color (vec4) and Align (int).
- Align correction: the pseudocode's null destination for `Align` is a decompiler artefact. The
  listing stores `lea eax,[edi+20h]` to the `push ecx` slot `[esp+10h]` at 0052DA3B and reloads it at
  0052DB84 for the read; every reader call is callee-cleaned, so the slot is intact. Align is read into
  item+20h; CreditsItem::align and the Enter's placement already assume this.

## Menu prompt Exit, HUD formation Enter/Exit, frame stats Register (cc13-menus6)

Reconstructed and build-tested (`menu_prompt_exit_00530970`, `hud_formation_enter_0053a4d0`,
`hud_formation_exit_00536800`, `frame_stats_register_0053c740`); evidence the listings, read whole.

- 00530970 (MenuPrompt Exit): hides +18h and +24h; if any of 00E198AC, 00E198B4, 00E198B8, 00E198C8
  exists with byte +3Ch set and *(00E1930C) exists, *(00E1930C)+4 = 1; +288h = 0, +284h = -1.
- 0053A4D0 (HudFormation Enter): *(00E188D8) null jumps straight to the tail (0053A8A1) with whatever
  +138h holds. Otherwise +138h = *(00E188D8)+284h (null: 0068AB80(1, 1) on *(00E198C4) and return).
  The leader (group+14h, pose refreshed by 00414DB0 when byte +C8h is clear, before each read)
  gives +17Ch = matrix x (+FCh) and +180h = matrix z (+104h); +178h and +174h get position
  (x, +18Ch, z) and (x, +188h, z) through virtual +30h. Then 0068E420, 0054E610, hint
  `FORMATIONPERMANENT` (00690FD0(hint, 0)). For each member 0..group+4F8h-1 (0070D060, nulls
  skipped): observer pair (00694A60, EDX = screen+64h), 00538D60 into map +110h and map +11Ch (scale
  1.0), a clone of +B0h with `icon_Icon` set to frame 00653200(member+538h) and shown, appended to list
  +ACh; for the leader 00A9C740 selects the icon widget (the value pushed is the icon, not the row);
  a type-0Dh widget (00AA6640, empty name) at local z -1000 (00D7A240), shown, texture
  *(00E08694), colour (0.5, 1, 0.5, 0.5), appended under +98h and pushed onto +128h (005389D0). Tail:
  +13Ch = *(leader+738h) virtual +50h, +7Ch = +7Dh = 0, 00533A30.
- 00536800 (HudFormation Exit): 0054E610, 00533A30; map +110h: each marker's +4 (read unguarded)
  unregisters the screen pair (006952A0, EDX = screen+64h), the marker is deleted through virtual +0;
  map +11Ch markers deleted; both trees freed (005356E0) and reset; list +ACh cleared; each +12Ch link
  widget deleted through virtual +4 and nulled, then the vector is emptied.
- 0053C740 (FrameStats Register): 004F71D0; page `_Fps` -> +0Ch; `PG_<n>p_Text`, `PG_<n>_Text`,
  `PG_<n>s_Text` for n = 1..6 -> +20h..+64h; `Fps_Text` +10h, `MinFps_Text` +18h, `MaxFps_Text` +1Ch,
  `Warning_Text` +14h; `Fingerprint_Text` is looked up after the payload (0053BC00 singleton, 00BE9620
  returns singleton+4) and set through 00ABBE50(payload, 1); it is not stored.

Uncertainty: 0054E610, 00533A30, 0068E420, 005A6AD0 and 00653200 are unexamined; the formation maps
are modelled as value lists in key order.
