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

## Reconstructed

| Address | Function | ABI | Notes |
|---|---|---|---|
| 00608F30 | `plane_screen_rebuild_widgets_00608f30` | fastcall(screen 3Eh) | delete and rebuild the per-slot widget pairs |
| 00681BA0 | `unit_library_update_00681ba0` | thiscall(screen, dt unused), RET 4 | input and class stepping |
| 00550DD0 | `idle_camera_shot_uninit_fill_00550dd0` | ECX = &value, (dest, count, proxy), RET 0Ch | 00550BF0 folded in |
| 00550FC0 | `idle_camera_shot_vector_insert_n_00550fc0` | thiscall(vector, iterator, count, value&), RET 10h | VC8 `_Insert_n` |
| 00607F60 | `plane_screen_ensure_cockpit_camera_00607f60` | fastcall(screen 3Fh) | cockpit camera |
| 00609620 | `plane_screen_update_drop_indicator_00609620` | fastcall(screen 3Eh) | payload slot widgets |

Notes per function:

- 00608F30 inlines the type map of 00604C80 (type +970h: 9, 0Bh, 0Dh, 0Fh, 12h, 0Ah map to 0..5).
  `plane_hud_payload_layout` is that map. The slot x positions follow the listing: odd counts keep slot 0 at
  0.5 and place slot i at 0.51 +- 0.02 * (i / 2); even counts place odd slots at 0.5 + 0.005 * (i + 1) and even
  slots at 0.5 - 0.005 * (1, 2, ...). The back widget is at z - 1.0 (00D7A210). The widget vector inserts are
  host calls naming 0043DB60 and 0052EB90. The callers in this file (00609390, the 3Fh enter) now call the
  reconstruction instead of a host method.
- 00681BA0 reads listbox +70h twice (row 0 is the digit cursor, row 1 the digit value), polls 4Ah in mode 1
  without using the result, and steps the class index modulo 1000 until `VehicleClass[i]` is a table with a
  string `Mesh`. The model load 00680DB0 is a host call (not reconstructed in this pass).
- 00550FC0 follows VC8 `_Insert_n` exactly, including both catch blocks (00551186: destroy the new prefix,
  free, rethrow; 00551240: destroy the moved tail, rethrow).
- 00607F60: with the plane's virtual +1C0h answering non-zero the eye is the model point [plane+538h]+244h
  and the camera observes the plane; otherwise the eye is (0, 1.8, [model+A0h] * 0.6) and the observer link
  is cleared. Near plane 0.5 only on the first path. The chase camera +94h is nulled, not deleted.
- 00609620 builds two bit vectors and a float vector as locals (modelled with `std::vector`), then drives the
  front widgets +94h and back widgets +A4h. `ready` and `gate` are descriptive names for the two bit
  vectors.

## Records

`python tools/bsp.py ledger add-function` for each row above (generator `local/fe7_records.py` in the worker
tree, not committed).
