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

## For cc9

- `include/bsp/world_entities.hpp:378` `marker_set_scale_006dbac0(target, mode, flag, value)` (the
  BSP_MarkerManager_Update walk, overridden in `src/game_hosts_mission_frame.cpp:747`) lacks the DL
  enable argument; the image passes DL = 1 at 006DC49A. The override is empty, so nothing changes
  behaviour today.
