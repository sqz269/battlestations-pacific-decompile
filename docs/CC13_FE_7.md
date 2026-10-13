# cc13-fe7: follow-ups from cc13-fe5 and cc13-fe4

Worker cc13-fe7 (cc13 lineage, Claude), 2026-10-12. Inputs: the handoffs of `docs/CC13_FE_5.md` and
`docs/CC13_FE_4.md`. Code: `src/frontend_seg16_24_28_cc13.cpp` / `include/bsp/frontend_seg16_24_28_cc13.hpp`
(namespace `bsp::cc13_fe5`, host `Fe5Host`) and `src/ingame_gui_cc13.cpp` / `include/bsp/ingame_gui_cc13.hpp`.

## Status vocabulary

Everything below is *reconstructed* from the live Ghidra decompile and the disk listing
(`bsp.py disasm-raw`) and *build-tested* (`scripts/build.ps1` passes). Nothing is ABI-compatible or
game-validated. No game runs were made.

## Fixes

### Session-message field reads take cursor + 4h

`session_message_read` and `session_message130_read_006563a0` (`src/ingame_gui_cc13.cpp`) passed the Read
argument to the field readers. The image passes it unchanged to the header read 0075B4C0 and passes
argument + 4h (a bit-cursor sub-object) to every field read:

- 006503C0: `add edi,4; mov ecx,edi` before each 00428D70.
- 00605300 and 0064A7E0: `lea ecx,[edi+4]` before 00428D70 / 00428D30.
- 00656530 and 006563A0: `add edi,4; mov ecx,edi` before 00428D70, 00428D10 and 00428E30.

The writes are unchanged: 00650390 passes the Write argument itself (`mov ecx,edi`) to 004290B0. The
native session-message files already model this (`&stream->cursor_04`). The header comment in
`SessionMessageHost` now states both rules.

### Annotations

- 00553BF0 is now `BSP_ControlBindingScreen_OpenVirtualKeyboard` (was `..._ResetSelection`). Evidence: user
  index from 00A3EAC0 when 00A3E510 says a user is selected (else 0); clears the default text +E6h and the
  1Ch-byte overlapped block +44h; memsets the 80h-byte result buffer +60h; calls XShowKeyboardUI (thunk
  00A4D43A) with (user, 0, +E6h, 0, 0, +60h, 30h, +44h); sets +E4h = 1.
- 00689E80: the plate comment said the attract movie is played with 1.0f. The float is loaded from 00CEFCB8
  (-902.0) at 00689EFF. The annotate tool appended a corrected `update` paragraph (the old paragraph is kept,
  per the comment-preservation rule).
- 00550DD0 is now `TMPL_IdleCameraShot_UninitFillN_00550dd0` (was `..._UninitCopyRet_...`): it forwards to
  00550BF0, which copy-constructs `count` copies of one value. 00550BF0 itself is still named
  `STL_UninitCopy_00550bf0`; it is an uninitialized fill (not renamed here).
- 00605B20 had a `FUN_` name; it is now `BSP_HudPlaneScreen_StepLowAltitudeBlink` (the low-altitude warning
  blinker called by 00609BD0).

## Reconstructed

All in namespace `bsp::cc13_fe5` (`src/frontend_seg16_24_28_cc13.cpp`) unless marked seg15
(`bsp::frontend_seg15_22_cc13`, `src/frontend_seg15_22_cc13.cpp`, fe4's `Seg15Host`).

| Address | Function | ABI |
|---|---|---|
| 00608F30 | `plane_screen_rebuild_widgets_00608f30` | fastcall(plane HUD screen 3Eh) |
| 00681BA0 | `unit_library_update_00681ba0` | thiscall(screen, dt unused), RET 4 |
| 00550DD0 | `idle_camera_shot_uninit_fill_00550dd0` | ECX = &value, (dest, count, proxy), RET 0Ch |
| 00550FC0 | `idle_camera_shot_vector_insert_n_00550fc0` | thiscall(vector, iterator, count, value&), RET 10h |
| 00607F60 | `plane_screen_ensure_cockpit_camera_00607f60` | fastcall(plane effects screen 3Fh) |
| 00609620 | `plane_screen_update_drop_indicator_00609620` | fastcall(screen 3Eh) |
| 00605B20 | `plane_screen_altitude_blink_00605b20` | thiscall(screen 3Eh, float altitude), RET 4 |
| 00609BD0 | `plane_screen_update_instruments_00609bd0` | thiscall(screen 3Eh, dt, bool, bool), RET 0Ch |
| 00606C60 | `plane_screen_load_layout_00606c60` | fastcall(screen 3Eh), vtable +14h |
| 00680DB0 | `unit_library_load_unit_model_00680db0` | fastcall(unit library screen) |
| 005D5340 | seg15 `multi_game_lobby_reset_slots_005d5340` | no ECX use |
| 005D67E0 | seg15 `multi_game_lobby_set_focus_005d67e0` | thiscall(screen, widget), RET 4 |
| 005D6C10 | seg15 `multi_game_lobby_refresh_map_icon_005d6c10` | fastcall(screen) |
| 005D8530 | seg15 `multi_game_lobby_confirm_kick_005d8530` | fastcall(slot record) |
| 005D8800 | seg15 `multi_game_lobby_prompt_swap_slot_005d8800` | ECX = requester, EDX = target |
| 005D8950 | seg15 `multi_game_lobby_show_swap_refused_005d8950` | EDX = slot |
| 005D98E0 | seg15 `multi_game_lobby_refresh_map_availability_005d98e0` | fastcall(screen) |
| 005D9A10 | seg15 `multi_game_lobby_exit_005d9a10` | fastcall(screen) |
| 005D9C50 | seg15 `multi_game_lobby_map_mask_005d9c50` | ECX passed through, unused stack argument, RET 4, EDX:EAX |
| 005D9D90 | seg15 `multi_game_lobby_on_listbox_activate_005d9d90` | thiscall(screen+8h, widget, flag), RET 8 |
| 005DA000 | seg15 `multi_game_lobby_on_list_select_005da000` | thiscall(screen+0Ch, widget), RET 4 |
| 005DA210 | seg15 `multi_game_lobby_confirm_leave_005da210` | fastcall(screen) |
| 005DA530 | seg15 `multi_game_lobby_apply_selected_map_005da530` | fastcall(screen) |

Notes per function:

- 00608F30 inlines the type map of 00604C80 (type +970h: 9, 0Bh, 0Dh, 0Fh, 12h, 0Ah map to 0..5);
  `plane_hud_payload_layout` is that map. Slot x positions follow the listing: odd counts keep slot 0 at 0.5 and
  place slot i at 0.51 +- 0.02 * (i / 2); even counts place odd slots at 0.5 + 0.005 * (i + 1) and even slots at
  0.5 - 0.005 * (1, 2, ...). The back widget sits at z - 1.0. The callers in this file (00609390, the 3Fh enter)
  now call the reconstruction instead of a host method.
- 00681BA0 reads listbox +70h twice (row 0 moves the digit cursor +ECh, row 1 steps the digit through
  `band_0067_cc13::digit_spinner_step_0067df40`), polls action 4Ah in mode 1 without using the result, and steps
  the class index modulo 1000 until `VehicleClass[i]` is a table with a string `Mesh`; then 00680DB0.
- 00550FC0 follows VC8 `_Insert_n` including both catch blocks (00551186, 00551240).
- 00607F60: with the plane's +1C0h answering non-zero the eye is the model point [plane+538h]+244h and the camera
  observes the plane (link camera+500h, observed +514h); otherwise the eye is (0, 1.8, [model+A0h] * 0.6) and the
  link is cleared. Near plane 0.5 only on the first path. The chase camera +94h is nulled, not deleted.
- 00609620 builds two bit vectors (`ready`, `gate`: descriptive names) and a float vector as locals.
- 00609BD0 (whole listing walked, stack offsets tracked across the EBP push 0060A042..0060A394). Corrections to the
  decompile: the stick vector has components (t194 * [BE8h] + [BF0h] * t198, t194 * [BECh] + [BF0h] * t198,
  *00F8757C) and the stick excess adds to all three; the two byte arguments are `ordnance_view` (arg 2: the
  ordnance branch 0060A42C) and `aim_view` (arg 3); the torpedo marker uses the altitude read at the top of the
  function. The reticle rows are screen+10h + row*14h, bound by 00606C60 from the groups AA, rocket, bomb, torpedo,
  parat, DC (rows 0..5). 00609BD0 picks row 4 when 007B9500 answers and places it at the 007BCCF0 release point, so
  the ledger name of 007B9500 (`BSP_WeaponController_HasLevelBombOrdnance`) may really mean paratroopers (not
  renamed; check before relying on it). The
  RNG draw at 0060AAD4 is stream 1 (ECX = 1 at 0060AAA2; `reports/cc13_rng_stream_census.json` agrees), so the
  presentation stream rule is met; the plate comment's "stream 0" is wrong.
- 00606C60 binds 33 named widgets; the parent of every find and every store come from the listing. The five cross
  icons of each row are zeroed first and skipped per row (`kPlaneReticleSlots`); "cross_F2_Icon" is placed at half
  the F icon's size and not kept. The names agree with the `kWidgets3E` inventory in `src/hud_screens.cpp`.
- 00680DB0: the Lua receiver of every call was resolved from its ECX stack slot. Each platform entry with an
  integer key, an integer `Gun[1]` and a string `DeviceClass[Gun[1]].Mesh` gets a part instance mounted on the
  hull's last point group "slot<key>" (more than two points) with RotY(-yaw) * basis, its "barrel" note node
  RotX(-pitch); yaw/pitch from `RestAngles` (vector2) or the means of `Windows[1]` Min/Max Horz/Vert angles.
- Lobby (seg15): 005D9C50 returns 1 << (atol(suffix) - 1), not 1 << atol(suffix) as fe4's note guessed, and
  ignores its stack argument (005D8650 takes none). 005D5340's loop counter is EBP. 005D9D90 / 005DA000 run on the
  +8h / +0Ch listener sub-objects; offsets are rebased to the screen. 005D8530 ignores 004BB850's AL.

## 0060ABD0 not reconstructed (deferred to the lead)

0060ABD0 (`BSP_HudPlaneEffectsScreen_Update`, 0060ABD0..0060C5AB, about 1600 instructions) was read up to
0060B9F1. It is the player plane control frame, not presentation only: 007C3CB0 countdown, 0077C470 role transfer
(cc9's), 007BB9A0 local-input gate, the free-look camera on action A0h with clamps 00415690 (ECX value, EDX low,
stack high), stick/throttle input through 004C5070 hold times into 007B8C90 (pilot command block), bomb fire
00608280 / 006082D0, two stream-1 RNG draws (0060B0CF, 0060B101: +3B4h / +3B8h of the chase camera jittered by
+-0.01 while +9F8h), the 5000-unit sweep probe (0042B570, 0098B370) and the wind / turbo effects +64h..+78h. It
overlaps cc9's plane-control hosts; I stopped rather than build a second control model. Structure notes for a
successor: effects at 0060ADF3..0060B097 (create through 00440490 + 008687C0 + 004845D0 + 00440A30, stop through
00867B10 + byte +9 + 00484620(0)); probe 0060B112..0060B455; free look 0060B47E..0060BA10; input and command
block from 0060BA10.

## Records

`python tools/bsp.py ledger add-function` for each row above (generator `local/fe7_records.py` in the worker tree,
not committed). Names changed with `ledger add-name --replace` and `tools/ghidra_annotate.py --apply`: 00553BF0,
00689E80 (comment), 00550DD0, 00605B20 (new), 005D9C50.

## Handoff (cc13-fe7, 2026-10-12)

State: five commits on `agent/cc13-fe7`, 24 reconstruction records, five name records, build green.
Open:

- 0060ABD0 (above): needs an ownership decision with cc9 before anyone models it.
- fe4's remaining list (`docs/CC13_FE_4.md`, Handoff): the x87 HUD counter layouts 00529D70, 0052A100,
  0052A470, 0052A860, 0052BB10, relayout 00529A30, tick 0052B080, and the large bodies; the lobby routines of the
  "ECX pass" list are done here.
- Host classes are not implemented anywhere (the models are build-tested only); `Fe5Host` gained about 90 methods
  and `Seg15Host` about 30 in this pass.
- 00550BF0 keeps the name `STL_UninitCopy_00550bf0`; it is an uninitialized fill.
- Tools in the worker tree's `local/` (not committed): `fe7_fetch.py` / `fe7_show.py` (caches),
  `fe7_squeeze.py` (listing with string-pool noise dropped and string literals resolved), `fe7_finds.py` (parent
  and store of each call of interest), `fe7_records.py`, `fe7_pe.py`.
