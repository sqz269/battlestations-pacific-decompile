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

## Tactical library

- 004FE870 binds the layout (three list boxes with listener +64h and input filter 004F90D0, the
  category groups, the info box scroller +0CCh with its scroll bar).
- The five loaders fill 6Ch entries (layout in docs/CC13_FE_8.md / 004FE310). The grouped ones
  (power-ups 3 groups, tutorials 13, units 5) open their own Lua owner (`Open(4)`, script run with
  flag 0) and append a fresh entry before filling the vector's last element in place; the nose-art
  and movie loaders reuse one local entry. The nose-art loop runs `NoseArt[0..999]` and skips nil or
  non-table items rather than stopping. Texture paths: `models/Textures/noseart/` + Texture,
  `fe/movies/` + the image table at 00E08498, `fe/units/` + GUITextureBackGround and `common/icons/`
  + GUITextureForeGround; power-ups use the two names unprefixed. A fresh entry leaves +40h..+48h
  uninitialised in the image; the model zeroes them.
- 0050FDB0 also builds the "no unlock" entry +5B0h (dereferenced even when `operator new` fails) and
  the achievements list from the award table: category by name prefix (`RANK_` 0, `CA_` 1, `MA_` and
  `AO_` 2, `RUA_` 3, `GA_` 4, `SA_` 5, `B` 7 with +44h cleared, otherwise 6), then calls the loaders
  in the order unit library, 00504130, power-ups, movies, tutorials, nose art, 00502480.
- The five view accessors 004FA890..004FAA00 read the filtered vector through the view at +434h
  (index +0h, float position +4h); 004FAA00 clamps the next card to the last entry.
- 00510C20 drives the three-card carousel and the detail text. Tutorials run their `LuaCommand`
  through 006B8AD0 and log the result through the empty 004254B0 (Hungarian format strings at
  00CEBD60 / 00CEBD34); their description takes the `_pc` key when `[00F88A30]` is clear and the key
  localises to something other than itself.- 00502480 fills a 288h-byte stack environment block (004CB420 / 004C6520) with constants and hands it
  to the ocean environment (0078C9B0). The two constant tables in the source were produced by a
  symbolic pass over the stack stores (`local/fe9_stackfill.py` with callee stack purges read from
  each callee's `RET n`, then `local/fe9_fillgen.py`); a write trace confirmed that none of the six
  triples passed to 004E5C00 is stored again after its call, so the model applies the tables and then
  the six linearisations. The fog scalars are passed while +0D4h is still 300.0 and +0E0h still 20.0;
  the block later holds 500.0 and 0.0 there. The sky is `HDR_sky_069` when `[00F88A0C]` is set.

## Session browser

- 005684D0 clones the create-session option rows (rank min, rank max, drop rate, each followed by a
  hidden value widget and a "-" label) one row step (+260h) apart from the label template's position;
  the drop-rate rows' visibility comes from `[00F8A2FC]` vtable +198h. The scroll range is
  `(float)(y - 0.501)` with y the row after the last.
## fe1 registers (src/frontend_screens_cc13.cpp)

- 005BB130 (HUD narrative register) binds the blackout, subtitle and narrative pages, looks
  `textbox_main_Text` up twice (+34h and +38h hold the same child), copies the subtitle text's colour
  (vtable +54h) into +44h and into the single message-box element it leaves in the vector +90h, and
  reads DialogFadeTime, TempCharWait and TempWaitBase from `Scripts/datatables/DialogGlobals.lua` in a
  private Lua owner opened with 1.- 00601200 (pause-screen hint list) walks the award tracker's hint history, which is a map keyed by
  hint name (head at tracker +98h, iterated with 004B7DE0), not a list. Each hint gets a row; a key
  containing `AAAAAA` anywhere is shown from position 6 on. The exclamation mark
  (`felkialtojel_Text`) marks hints present with a non-zero count in the tracker's new-hint map +7Ch,
  and, for expanded hints (screen list +128h), the sub-hints found in that hint's new-hint list. Sub-hint
  rows take the language text (language 0 / 1 from `[[00E188A8]+18CCh+[+18ECh]*4]+28h`), else the
  record's default, else the sub-hint key. Only top-level rows get an index-map entry; sub rows still
  advance the index.
## For cc9

Nothing in this pass changes gameplay state. 00510C20 runs a tutorial entry's `LuaCommand` through the
game's Lua machine (006B8AD0) when the library shows it; a host that wires the tutorial page in will
execute authored Lua, which is the lead's call to enable.

## Handoff (cc13-fe9, 2026-10-12)

Twenty-three routines reconstructed and build-tested, with ledger records (all
`reconstructed_build_tested`, behaviour models, not ABI-compatible, not game-validated) and 1515
verified call rows in `reports/cc13_fe9_calls.json`. No Ghidra renames or comments.

- Control binding: 0055C5A0, 0055D380, 0055E3A0, 0055F710, 0055FCA0.
- Tactical library: 004FE870, 0050B8E0, 0050BF00, 0050C300, 0050C760, 0050CE70, 0050FDB0, 00510C20,
  00502480, and the view accessors 004FA890, 004FA8F0, 004FA960, 004FA9C0, 004FAA00.
- Session browser: 005684D0.
- fe1: 005BB130 (HUD narrative register), 00601200 (pause hint list).

Still open:

- **005072C0 Preview_OnModelLoaded (7.9 KB).** Read but not modelled. Structure from the decompile:
  hides `fizika_00..19` descendants except `ep_fizika` nodes and every `hajobelso` child (visibility 0,
  scale vtable +2Ch with [00CE4BC8]); collects `rotor`, `rotor0..3` notes into the preview object's dword
  vector (+424h, storage +428h..+430h; 00505D50 on growth, "+Rotor %d=0x%x" log) and hides the `rotor_still*` /
  `rotor_still_dam*` notes; ship classification by VehicleClass/DeviceClass type strings (Cargo,
  Destroyer, TorpedoBoat, LandingShip, Submarine, ...) into byte +4CCh, view distances from
  `UnitlibViewDistance`, `water` nodes hidden for submarines, `szarny letorik` nodes hidden, then the
  `Platforms` / `RestAngles` / `Windows` turret setup with point-group bases (BSP_Vector3f_Cross,
  BSP_Matrix_OrthonormalizeBasisRows). Needs the listing for ECX at nearly every call.
- **005AB190 HudMap register (11 KB)**, fe1: not started.
- From cc13-fe8's list, still open in segments 12/17: 004FF6F0 ShowDifficultySelect, 00500A90
  FillOrderOfBattleRow, 005021E0 LoadDownloadableClasses, 00503510 FrontEndPreview_Draw, 00504130
  LoadClassTables, 00506660 BuildOrderOfBattleList, 0050A400 ReadVehicleClassRow, 0050AA10
  Preview_LoadVehicleModel, 0050E3C0 / 0050EF80 OpenEntryAs*, 00515050 UpdateTutorialMovie.

Tools (this worktree's `local/`, not committed): the fe8 tools re-pointed at this tree, plus
`fe9_asm.py` (folds the inlined NativeString construction / memcpy / pool-free sequences and names
call targets), `fe9_stackfill.py` (symbolic pass over stack stores, callee stack purges from each
callee's `RET n` via capstone; `--trace` lists every store into the block found at the
`--base-call` constructor), `fe9_fillgen.py` (turns a trace into C++ write tables) and
`fe9_ledger.py` / `fe9_callrows.py`. `fe9_asm.py` drops conditional-jump context around the folded
sequences; read flag-dependent branches from the raw dump.