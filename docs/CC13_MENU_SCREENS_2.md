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

Call sites: `reports/cc13_menu_screens_2.json`, 233 rows, `tools/verify_report_calls.py`: 0 failed.

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