# cc13-fe9: open frontend bodies after cc13-fe8

Packet cc13-fe9 continues the segment 12/17 frontend list left open by cc13-fe8 (docs/CC13_FE_8.md,
handoff). Code lives in `src/frontend_seg12_17_cc13.cpp` and `include/bsp/frontend_seg12_17_cc13.hpp`
(namespace `bsp::cc13_fe3`), as behaviour models over raw native storage and host interfaces.

Status words: every routine below is *reconstructed* and *build-tested*; none is ABI-compatible,
fixture-tested or game-validated. Names are hypotheses carried over from the ledger. Evidence is the
disasm-raw listing (x87 and register arguments read from it), the live decompile for structure, and
strings and constants read from the PE on disk.

## Control binding

Conventions from cc13-fe8 hold: the list listener is screen+8h, the widget listener screen+0Ch, the
page +168h, the per-page selections +16Ch + page*4. Device keys are 8-byte NativeStrings in the input
settings' vector +18h..+1Ch; `map[key]` (0055C110 on settings +8h) gives the device entry, a
case-insensitive tree (head +4h) of binding names with a name vector +34h, a settings name vector
+44h, a kind map +18h (0055A9A0) and a float value map +24h (00444BE0).

- 0055C5A0 walks the settings list's own std::list of rows (list +14Ch, head pointer +150h, row at
  node +8h), not the list box row accessor. Rows without a `main_Text` child (recurse 0) are skipped.
  Conflict markers move their `Left_Text` to `-(float)(width / 960.0)`.
- 0055D380 places the conflict marker's `Right_Text` at `(float)(width / 960.0) + 1/96.0`
  (00CEE728), the mirror of 0055C5A0. When the settings vector has no kind 3/4 (invert) setting, the
  `^FE_pc.controls_sensitivities` label lands on the invert header already in the list (ESI still holds
  it at 0055DF87), so the invert header is relabelled rather than a second header added. The slider
  position reuses `control_slider_position_0055ccc0` from `frontend_menus`.
- 0055E3A0 has three command bars (page 2, page 3, the rest), a touch-controller prompt on page 2
  (slot 3, kind 3, page set to 4), and a page-1 preset list built from the Lua global
  `ControlPresetsOrder` and the `Presets/*.preset` resources whose files start with the 15 bytes
  `CONTROL_PRESET\x01`. At most 10 rows (7 with `[00E188A8]+61Fh`) take file presets; names must be
  non-empty and alphanumeric or space after dropping the first 8 characters and the first `.preset`.
- 0055F710 reads `ControlPresets[name]` without a nil check, but `Preset` from a script only when not
  nil.
- 0055FCA0: on page 0 selection 0 is the "general" header row and leaves the screen (options page 4,
  interface request 0Ch); on page 1 selection 0 is the built-in preset (sets +38h), a positive
  selection is a file preset (`Presets/%s.preset` into +3Ch, prompt only when the VFS has it).
