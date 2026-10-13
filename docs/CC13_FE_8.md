# cc13-fe8: open frontend bodies (fe3 leftovers, register-argument fixes)

Packet cc13-fe8 (cc13 lineage, Claude). Reconstruction pass over the bodies that cc13-fe3
(docs/CC13_FE_3.md) and cc13-fe6 (docs/CC13_FE_6.md) left open. Code:
`src/frontend_seg12_17_cc13.cpp`, `include/bsp/frontend_seg12_17_cc13.hpp`, namespace
`bsp::cc13_fe3` (section "Packet cc13-fe8 additions").

Status vocabulary: every routine below is **reconstructed and build-tested**. None is
ABI-compatible (the native `__thiscall`/`__fastcall` conventions are documented, not reproduced),
fixture-tested or game-validated. Nothing is wired into `bsp_game`, and no game was run. Names are
hypotheses; no Ghidra renames or comments were made in this pass.

Evidence: `bsp.py show` (export pseudocode) for the shape, then `bsp.py disasm-raw` over the
whole body for every call's ECX/EDX and pushes, RET sizes and x87 sequences. Constants and
strings were read from the PE on disk. Call sites are in `reports/cc13_fe8_calls.json`, checked
by `tools/verify_report_calls.py` (0 failed).

## Register-argument fixes

- **006DBAC0 `BSP_GuiElement_ApplyHighlightPulse`** takes DL as an enable flag (`MOV BL, DL;
  TEST BL, BL` at 006DBAD9). `include/bsp/marker_manager_cc13.hpp` now models it:
  `apply_highlight_pulse_006dbac0(element, bool enable, mode, flag, value)`. The removal call
  (`remove_gui_highlight_006dc5a0`) passes DL = 0 (`XOR DL, DL` at 006DC661). The other callers:
  `BSP_MarkerManager_Update` passes DL = 1 (`MOV DL, 1` at 006DC49A), 005EF442 passes
  `[screen+70h]` (already modelled in `frontend_screens_cc13.hpp`). 0051C430 and 0069659A were not
  traced.
- **00A9CDF0 `BSP_GuiListbox_ClearOtherTwo`** (EDX register argument): already modelled by
  cc13-fe6 (`list_box_clear_other_two_00a9cdf0(keep, edx_list, stack_list)` in
  `frontend_screens_cc13.hpp`); no other model of it exists in `src/` or `include/`.

## Routines

| Address | C++ | Native ABI | Notes |
|---|---|---|---|
| 00561320 | `leaderboards_listener_focus_00561320` | thiscall(listener = screen+0Ch, widget), RET 4 | focus styling and auto-scroll stop |
| 00561BE0 | `leaderboards_build_board_list_00561be0` | fastcall(ECX screen), RET | ledger name RefreshFilterLabel; it builds the 18-row boards list |
| 00561EB0 | `leaderboards_fill_rows_00561eb0` | fastcall(ECX screen), RET | result rows, pending selection, arrow alphas |
| 00562750 | `leaderboards_enter_00562750` | fastcall(ECX screen), RET | |
| 005638F0 | `leaderboards_read_table_005638f0` | no arguments, RET | Scripts/datatables/Leaderboards.lua |
| 00564220 | `leaderboards_register_00564220` | fastcall(ECX screen), RET | 26 lookups, two scroll bars |
| 00565190 | `leaderboards_listener_activate_00565190` | thiscall(listener = screen+0Ch, widget), RET 4 | |
| 00565540 | `leaderboards_update_00565540` | thiscall(screen, float dt), RET 4 | actions, paging, read polling |
| 0055C200 | `control_binding_show_conflict_text_0055c200` | fastcall(ECX screen), RET | |
| 0055C3B0 | `control_binding_refresh_selected_binding_0055c3b0` | fastcall(ECX screen), RET | |
| 0055CEC0 | `control_binding_scroll_to_selection_0055cec0` | thiscall(screen, BYTE no_scroll), RET 4, AL | x87 visibility test |
| 0055D180 | `control_binding_on_selection_changed_0055d180` | thiscall(screen, row, list box), RET 8 | |
| 0055FA80 | `control_binding_enter_0055fa80` | fastcall(ECX screen), RET | in-mission list choice |
| 00560210 | `control_binding_confirm_or_next_device_00560210` | fastcall(ECX screen), RET | accept-changes prompt |
| 005001A0 | `library_build_unlocks_list_005001a0` | fastcall(ECX screen), RET | one row |
| 00500450 | `library_build_movie_list_00500450` | fastcall(ECX screen), RET | one row |
| 00500640 | `library_build_nose_art_list_00500640` | fastcall(ECX screen), RET | two rows, unlock-gated |
| 00500860 | `library_build_units_weapons_list_00500860` | fastcall(ECX screen), RET | five rows |
| 00500E90 | `library_build_power_up_list_00500e90` | fastcall(ECX screen), RET | three rows |
| 005010C0 | `library_build_academy_list_005010c0` | fastcall(ECX screen), RET | thirteen candidates |
| 005012F0 | `library_build_achievements_list_005012f0` | fastcall(ECX screen), RET | six rows |
| 004F9D80 | `library_colour_static_grey_004f9d80` | no arguments, EAX = 00E18D64 | function-static colour |
| 00506510 | `library_reset_vehicle_class_order_00506510` | fastcall(ECX screen), tail JMP 004F8FA0 | |
| 00506550 | `library_show_current_entry_detail_00506550` | fastcall(ECX screen), RET | registry permutation swap |
| 005661C0 | `session_list_absolute_selection_005661c0` | fastcall(ECX list box), EAX | ECX is the list box itself |
| 00568290 | `gui_icon_set_uv_rect_00568290` | thiscall(icon, const float uv[4]), RET 4 | |
| 004FFD00 | `library_show_unlocks_page_004ffd00` | fastcall(ECX screen), RET | command bar, title |
| 004FFF20 | `library_show_order_of_battle_page_004fff20` | fastcall(ECX screen), RET | command bar, title |
| 00509DE0 | `library_build_map_entries_00509de0` | fastcall(ECX screen), RET | locked flags from the game map |
| 0050D900 | `library_filter_entries_0050d900` | thiscall(screen, source, out), RET 8 | ledger name ClearList |
| 00517ED0 | `front_end_frame_register_00517ed0` | fastcall(ECX screen), RET | three page tables, frame index 4 |
| 00517300 | `library_handle_gui_event_00517300` | thiscall(listener = screen+68h, widget), RET 4 | jump table 00517600 |
| 005178F0 | `in_game_screen23_bind_unit_005178f0` | thiscall(screen, unit), RET 4 | airfield camera |
| 005198F0 | `bomber_view_ensure_camera_005198f0` | fastcall(ECX screen), RET | ship camera, binoculars |
| 004FE310 | `library_entry_assign_004fe310` | thiscall(to, const from), RET 4, EAX = to | 6Ch entry copy-assign |
| 00503250 | `library_refresh_unlock_rows_00503250` | thiscall(screen, class id), RET 4 | |
| 004FD500 | `library_find_entry_for_side_004fd500` | thiscall(library, ref*, dword[4]), RET 8, AL | ledger name CountAvailableMapsForSide |
| 00511BF0 | `library_select_order_of_battle_side_00511bf0` | thiscall(screen, side), RET 4 | x87 +438h and scroll range; jump table 00511F10 |

### Leaderboards

- Listener offsets: the row and widget listener is screen+0Ch, so 00561320 and 00565190 compare
  against listener offsets 0Ch below the screen offsets (`[esi+110h]` is the entries list box
  +11Ch). The list-box listener is screen+8h.
- 00561BE0 names: the routine never touches the filter text; it fills the boards list box +94h
  with 18 clones of +98h whose "filter_Text" child shows each board row's +4h name. The ledger name
  `RefreshFilterLabel` is kept in the ledger; the C++ name describes the body.
- Scroll ranges: 00561BE0 computes `h98 * 18.0 + 1/72` on x87 and spills it as a double
  (`FSTP qword`), then subtracts h90 and rounds to float. 00561EB0 computes `h120 * 1.005` (FMUL
  qword 00CEE950) then `FIMUL` by the entries row count, spills as a double, subtracts h118. The
  C++ evaluates the product chain in double (one extra rounding against the extended-precision
  chain; results agree except in the last double ulp before the final float rounding).
- 00561EB0 arrow alphas: 00683330 receives (0.0, up, down) in mode 0, where up/down are 1.0 or
  0.25f (00CE3868) from `first - 17 > 0` and `first + 17 < total (+14Ch)`. Mode 1 passes all 1.0
  when the list has more than 17 rows, else (0, 0.25, 0.25); mode 2 always (0, 0.25, 0.25).
  Traced from the stack stores at 005623D7..0056246A.
- 00561EB0 pending selection: +0B8h is saved before the focus move and restored after it, so the
  boards selection that the move wrote is discarded. The model keeps that order.
- 005638F0: the row vector gets 18 rows whether or not `LBoards[i]` exists (a missing entry pushes
  a zeroed row). `Columns[j]` lands at row +1Ch + j*4 with no bound in the image; more than eight
  columns would overwrite the loader's other stack locals. The model stops storing at eight and
  still counts them.
- 00565190 type 3: the icon test reads the first byte of the widget label (+0F0h): A7h cycles the
  filter (as does the filter text +134h itself), A5h shows the gamer card, A3h returns to the menu.
  `XShowGamerCardUI` receives (00A3EAC0() on [00F8ABE8], XUID at result row +58h); 00A3EAC0 takes no
  stack argument (the two XUID pushes before it are XShowGamerCardUI's).
- 00565190 type 6 / 12h: the vtable +88h / +84h calls push the literal 2 as the state, unlike the
  focus listener's 0/1.
- 00565540 command bar: the 15 pushes at 00565934..00565967 give (A7h "FE.toggle_filter" 0,
  B3h "globals.navigate" 1, A5h/0 "globals.showgamercard"/"" 1, B2h/0 "globals.scroll_menu"/"" 1,
  A3h "globals.back" 2). The stack slots were traced by hand across the pushes.
- 00565540 gate: failing 00585B40 or 00585810(CL = 0) jumps to 00565A83, which sets +160h (the
  next update returns to the menu). The duplicate-sign-in prompt path reaches the same store.
- 00565540 read polling: `0x80004005` and `0x8000FFFF` hide the please-wait screen; only
  `0x80004005` raises the read error. The 30-second time-out compares `(float)(t + dt)` against the
  double 30.0 (00CE7630).

### Control binding

- 0055C200 and 0055C3B0 look the device key up in the input settings' device map (0055C110 is that
  map's `operator[]`) and the binding name in the entry's tree (00555940 `find`). "Not found" is
  `iterator.node == entry+4h` (the head), checked against the container at 0055C4B5.
- 0055C200 pushes the conflict result as a dword whose low byte is AL and whose upper bytes are
  the stale contents of `[esp+0Ch]`; the model passes 0 or 1.
- 0055CEC0 is the same visibility routine as 00561860 (leaderboards MoveSelection) over the
  binding list +20h, clip +218h and the embedded scroller +1A0h. The snap paths skip rows whose
  +77h byte is set.

- 0055FA80 erases the string +3Ch with the inlined `erase(0, min(length, 7FFFFFFFh))` (a memmove of
  the remainder, then 0041DD40(length - n, 1)); the model keeps that sequence literally.
- 00560210 raises the prompt with slot 0, kind 4 and callback 0055FA10 (the revert-prompt result
  handler already described in docs/CC13_FE_3.md) after appending three button labels to the prompt
  singleton's +268h vector and clearing its +284h.

### Tactical library category lists

- The seven builders share one row recipe (clone +0ACh, `MenuItem_Text`, row name = the decimal
  index through vtable +2Ch, row +77h = 0, label, append) and finish with 00A9CF30(1, 1). Only the
  units/weapons builder shows the text (vtable +34h(1)) before labelling it; the others label first.
- Label sources: [00E08494] (movies), [00E084F0 + i*4] for i < 2 (nose art, built when +98h >= 5Ah or
  +98h == i), [+31Ch + i*8] for i < 5 (units/weapons), [+344h + i*8] for i < 3 (power-ups),
  [+35Ch + i*8] for i < 13 skipping ids for which 004FAE70 answers true (academy), [00E084D8 + i*4]
  for i < 6 (achievements). Labels go through a 200-byte buffer (strcpy_s, or sprintf `%s`); the
  model passes them through without the CRT's over-length handler.
- 005001A0 builds one row: the vehicle-class name of record 0's id at +458h when that id equals +454h,
  else `FE.unitlib_nounlock` with +454h = -1 and +458h = +138h = 0. The image raises the checked-
  iterator report (00BF6713) when the record vector is empty; the model assumes a record.
- 005661C0 passes its own ECX to 00A9C920 unchanged, so the object is the session list box (a list
  box with a scroll offset +164h behind the +148h byte), not the browser screen. `menu_screens_2`'s
  host already calls it on list +30h.
- 004FFD00 calls atol on the current item's name and discards the result.
- 00506550 resets both registry permutation tables (+10h, +2010h, 800h dwords) to the identity and
  then swaps one pair: table1[rec] = class id and table2[class id] = rec, where rec is the first dword
  of the order-of-battle record named by the current item.
- 00509DE0's find is the inlined `lower_bound` plus `!(key < node)`: an empty key orders before
  any non-empty node key, and a non-empty key against an empty node key counts as found.
- 0050D900 (ledger `ClearList`) filters: it empties the output vector and copies the entries whose
  +40h side equals +3C0h or exceeds 59h and whose +44h byte is set, then resets the view by kind.

### Frame, library events and in-game cameras

- 00517ED0 loads pages only in the row of the frame index it has just set to 4; every other slot of
  the 5x13 (+0Ch), 5x2 (+110h) and 5x2 (+138h) tables is zeroed.
- 00517300 runs on the listener at screen+68h; its offsets were rebased to the screen. The jump
  table 00517600 maps view kinds 3..8 to PlayEntryMovie, nothing, ShowDifficultySelect,
  IsEntryUnlocked/Back, the timer rule and ApplyNoseArt. The timer +5B4h is compared for exact
  equality with -1.0f (UCOMISS, `TEST AH, 44h; JP`).
- 005178F0 and 005198F0 construct their camera movers inline (`new`, memset, base constructor, four
  vtable stores). `memset` runs before the null test in the image; the model tests first. The
  call-summary tool reported ECX = 0 for 00923870 because it followed the failed-allocation branch;
  the listing shows ECX = the new camera on the normal path.
- 005178F0 calls `BSP_UnitInstance_SendRoleTransfer` (0077C470, role 1FEh) on the previous unit;
  that is gameplay state reached from an in-game screen (see For cc9).
- 004FE310 releases the old intrusive reference through vtable +0 (ECX = the object, no stack
  argument) when InterlockedDecrement (import 00CE2220) reaches 0; the new one is retained first
  (00CE221C). Byte +47h is padding and is not copied.
- 00503250 keeps its record pointer in a stack local that is neither reset per row nor initialised
  before the first kind-5/6 row; for other view kinds a non-current row reads the previous row's
  record (or garbage for the first). The model stops at a row that would read the uninitialised
  value.
- 004FD500 (ledger `CountAvailableMapsForSide`) is a lookup returning AL; its no-match EAX carries
  the entry count in the upper bytes.
- 00511BF0: +438h is FILD + FADD qword 0.003 rounded once to float (the model adds in double, which
  holds the int exactly; the only difference is double against extended rounding of the sum). The
  scroll range adds the x87 text height (00AB6BD0 returns in ST0) to the float `extra`, spills the sum
  as a double, subtracts h(+5ACh) and rounds to float.

## For cc9

- `include/bsp/world_entities.hpp:378` `marker_set_scale_006dbac0(target, mode, flag, value)` (the
  BSP_MarkerManager_Update walk, overridden in `src/game_hosts_mission_frame.cpp:747`) lacks the DL
  enable argument; the image passes DL = 1 at 006DC49A. The override is empty, so nothing changes
  behaviour today.
- `in_game_screen23_bind_unit_005178f0` (005178F0) sends 0077C470 (role 1FEh, 0) to the unit it
  releases and makes the airfield camera the game's camera mover (004BC410). Wiring it into the host
  changes in-game unit state; that is the gameplay lane's decision.
