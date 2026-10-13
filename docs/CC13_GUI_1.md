# GUI widget library reconstruction, segments 76-79 (packet cc13-gui1)

Addresses: 00AA0F70-00AD5360 (partition segments 76 to 79).

Code: `include/bsp/gui_widgets_cc13.hpp`, `src/gui_widgets_cc13.cpp`,
`include/bsp/native_gui_widget_pools_cc13.hpp`, `src/native_gui_widget_pools_cc13.cpp`, and an
appended block at the end of `src/native_material_pools.cpp` (the shared native slab-pool
engine). Records: `config/reconstruction/00aa0000.jsonl` .. `00ad0000.jsonl` (evidence field
starts with `docs/CC13_GUI_1.md`). Call-site report: `reports/cc13_gui1_calls.json`.

This is a reconstruction pass over functions the naming passes (cc13-gui6
`docs/CC13_GUI_MANAGER.md`, cc13_gui_render `docs/CC13_GUI_RENDER.md`,
`docs/CC13_HUD_AND_WIDGETS.md`) had named but left without source. Names are hypotheses, not
recovered symbols; class names quoted below (cGuiElement, cGuiCurve, cGuiProgressbar) are the
strings the type-descriptor initializers store.

## Status

| Level | What |
| --- | --- |
| Reconstruction records added | 329 (grep `config/reconstruction/00a*.jsonl` for `CC13_GUI_1`) |
| Reconstructed, build-tested | all 329; `./scripts/build.ps1` passes (2/2 tests) |
| Of which backfilled records for existing source | 00AA6750 (`src/gui_widget.cpp`), 00AB1680 (`src/gui_icon.cpp`), 00AA1FE0 (`include/bsp/gui_aspect_extent.hpp`), 00AA3E00 (`src/gui_render_order.cpp`), each re-checked against the listing |
| Fixture-tested / ABI-compatible / game-validated | none. The C++ is new interfaces over raw native storage and host objects; no native adapter; no game run |
| Renamed in the ledger | 21 replacements: 00AC8320 -> BSP_GuiScrollbarPool_Trim; nine GetTypeChain -> GetOwnTypeId and nine GetTypeId -> GetTypeName (what they return was read); 00AB0220/00AB0350 PushSubtitleRecord/PushExtratitleRecord -> ResizeSubtitles/ResizeExtratitles. 64 new ledger names for pool, stub and type routines that only had Ghidra-side names |

Ghidra was not modified by this packet (no renames applied, no functions created); the ledger
names above still need `tools/ghidra_annotate.py --apply` by whoever next holds the write lock.

## Method and conventions

- Every routine was read from the export or live decompile, and from the listing (compact
  `disasm-raw`) wherever pseudocode had register arguments, lost stack arguments (the describers),
  x87 or suspicious loops. Strings were read from the image bytes.
- Engine routines that are not reconstructed are host-interface methods named with their
  address (`construct_widget_base_00aa9390`, `rebuild_00ab3cb0`). Routines already reconstructed
  elsewhere are called directly: `recompose_local_transform` / `refresh_local_bounds`
  (gui_widget.hpp), `reserve_native_float_array_008153e0` / `resize_native_float_array_00818030`,
  `name_less_00aa2420`, `GuiIconDelayedLoads`.
- Native widget objects are raw storage: the reconstructions write the fixed offsets the image
  writes (vtable words keep their image addresses as values).
- `reports/cc13_gui1_calls.json` lists 421 direct CALL/JMP sites from the reconstructed routines
  to the addresses the headers name; `tools/verify_report_calls.py` checks all 421 (0 failures).

## What was reconstructed

### Fourteen widget slab pools (110 records)

Widget, Button, Movie, Icon, Curve, Section, Line, Scrollbar, Grid, Model, Sound, ClipBox,
FrameBox and AnimIcon pools are instances of the same Mit slab-pool template already
reconstructed for the GUI Group and layer pools. `NativeGuiWidgetPool<Traits>` reuses the engine
in `src/native_material_pools.cpp`; the trait table in `native_gui_widget_pools_cc13.hpp` carries
per pool: global, vtable, slot size, slots per slab, and the addresses of Construct, Allocate,
Free, Trim (vtable +0), Destroy, InitBlock, FreeThunk and the allocate stub.

Evidence: constants from each Allocate (slab size, free stack, free count; all fourteen satisfy
`slot*n == free`, `free + 2n == count`, `count + 4 == slab`), each Construct (vtable) and each
FreeThunk or stub (global). Every member was compared with the Widget pool member after
replacing immediates: Allocate, Construct, Destroy and FreeThunk are identical; Free differs only
in the division-by-stride sequence; Trim and InitBlock only in strength reduction (Widget and
Sound Trim pseudocode compared line by line). The out-of-line slab-table vector members
(00AA6FA0 family, nine routines, no callers) are reconstructed as `native_gui_pool_table_*`.

| Pool | Global | Slot | Slots |
| --- | --- | --- | --- |
| Widget | 00F8BC94 | F0h | 64 |
| Button | 00F8BD00 | 100h | 64 |
| Movie | 00F8BD4C | 134h | 4 |
| Icon (also Progbar) | 00F8BDA0 | 13Ch | 64 |
| Curve | 00F8BE5C | 144h | 64 |
| Section | 00F8BEAC | 130h | 8 |
| Line | 00F8BEFC | 108h | 32 |
| Scrollbar | 00F8C004 | 100h | 16 |
| Grid | 00F8C050 | 128h | 16 |
| Model | 00F8C09C | 120h | 8 |
| Sound | 00F8C0EC | 10Ch | 16 |
| ClipBox | 00F8C13C | 110h | 16 |
| FrameBox | 00F8C188 | 120h | 16 |
| AnimIcon | 00F8C1D4 | 148h | 4 |

### Type descriptors (35 records)

`GuiTypeDescriptor<Depth>`: own id, ancestor ids nearest first, then the class-name address, the
same family as the light/node descriptors in `light_type_bootstrap.hpp`. The ledger's
"GetTypeChain" returns the own-id word and "GetTypeId" returns the NAME word that follows the ids
(for Progbar 00D5CCDC "cGuiProgressbar"); both were renamed. IsKindOf scans the ids. Four
initializers were read (00AA8200 cGuiElement under the root 0109DB84, 00ABCBD0/00ABCC20
cGuiCurve, 00AC7B80/00AC7BD0 cGuiProgressbar under Icon).

### Factories, deleting destructors, destructors (32 records)

The fourteen type-table stubs are `source ? new T(*source) : new T()` with class pool operator
new/delete (the unwind funclet returns the slot through the pool free thunk). Thirteen scalar
deleting destructors, the Progbar/Scrollbar destroy thunks and the Line destructor.

### Property describers and readers

- 00AAAED0 the widget base describer: Pos, Size, Pivot, Scale, Rotate, Color, LowColor,
  HighColor, BlendFactor against their defaults; WideScreenAlign as "Left"/"Right"; Visible only
  for non-root widgets; MouseBlock true always written and then MouseHit skipped; then children
  in case-insensitive name order, each in a named sub-table.
- Progbar 00AC7A90/00AC7B00 (DefaultValue), Scrollbar 00AC8770 (Alignment = "Vertical" only
  when +F8h is 0, MinValue, MaxValue), Model 00ACC4E0/00ACCE20, Icon 00AB2B70 (see below).
- 00AB2B70 Icon: a single default-size state suppresses Size by zeroing the widget size around
  the base describer; PartialDisplayType is written with variant tag 1 (an integer, unnamed in
  `GuiValueTag`); ShaderName is written only when non-empty, against "GuiDefault.mshd"; states go
  into "States" with integer keys i+1 (loop bound read from the listing; the decompiler lost it)
  and Size/Pivot default to the WIDGET's size and pivot.

### Constructors, setters, members

Progbar, Scrollbar, Sound, Model, Line, Curve, Grid, Button, AnimIcon and Icon constructors,
copy constructors and destructors; Layer name get/set; the widget transform setters
00AA7910/7930/7950/7D40/7D80 over `GuiWidgetTransform`; Curve point list clear/add.

### The two std::sort instantiation sets (19 records)

00AA5A00 and 00AAAED0 sort child pointers with two byte-identical instantiations of VS2008
std::sort (pairs compared instruction by instruction). `msvc9_sort` reproduces the library
algorithms in the instantiations' operation order (med3, median with the 40-element ninther,
unguarded partition, heap fallback, introsort ideal halving, and the rotate-based insertion
sort the instantiations use), so equal names permute exactly as in the image; 00AA7810 is the
byte-identical twin of the already reconstructed predicate 00AA2420.

## Findings worth knowing

1. **Progbar overruns its Icon-pool slot.** The Progbar factory 00AA14A0 loads 140h into ECX
   and calls the Icon pool stub 00AB2B60, which discards ECX: Progbar objects live in 13Ch Icon
   slots (138h payload; 00AB1340 writes the slab index at +138h and 00AB2150 reads it back).
   The Progbar constructor writes its value at +138h and the reader writes DefaultValue at
   +13Ch. So a live Progbar's value overwrites its own slot's slab-index word, and DefaultValue
   overwrites the first dword of the next slot (the next widget's vtable pointer, or the free
   stack for slot 63). Freeing a Progbar (scalar deleting destructor 00AC7900 -> free thunk
   00AB24A0) would index the slab table with the float's bit pattern. Consequences are inferred
   from the code, not observed; whether pages holding progress bars are ever freed was not
   checked.
2. Copy constructors that do not copy: Sound 00ACDBA0 runs the default base constructor (no base
   fields, no strings); Curve 00ABE100 yields an empty curve; Icon 00AB5D30 drops ShaderName;
   Progbar 00AC7840 drops DefaultValue; copied Buttons 00AA3C60 are not put in the 00F8BD38
   button list.
3. The Button destructor 00AAC530 clears the global 00F8BCF8 whenever it is non-zero, without
   comparing it with the button being destroyed.
4. The Curve reader 00ABE1D0 (not reconstructed) builds the padded control array only when the
   authored point count is BELOW 2 (`cmp eax,2; jae 00ABE59C`), and that path then indexes
   points [0] and [1] through checked iterators. Recorded as an anomaly to resolve; the existing
   `gui_curve_control_points_00abca10` covers the normal path.
5. `src/gui_layout_loader.cpp`'s base-property table gives `name_literal` 00D5C1F4 for LowColor
   and 00D5C1EC for HighColor. The image strings there are "...enAlign" and "WideScreenAlign";
   the describer pushes LowColor 00D5C214 and HighColor 00D5C208. The reader's literals were not
   re-read; the table's owner should check them.
6. Ghidra boundaries: the Widget pool free thunk 00AA7880 and allocate stub 00AA7CF0 are code in
   padding with no Ghidra function; several type-descriptor initializers sit inside other
   functions' bodies (calls to 006FAC20 at 00AC23C5/00AC2415 in CG_static_dtor_stub_00AC2380,
   00AC8535/00AC8585, 00ACEA55/00ACEAA5, 00AD07B5/00AD0805, 00AA937C in STL_MakeHeap 00AA92E0,
   00AB31B5/00AB3205 in 00AB3180). These need function creation by a flow-repair packet.

### Later batches (71 records)

- Icon: describer, copy constructor, destructor, the 00F8BD98 delayed-load singleton accessor,
  destructor and file-loaded callback (over the existing `GuiIconDelayedLoads`).
- Movie: position reset, video-texture binding (three slots), player forwarder, subtitle clear,
  and the two record-vector resizes (renamed: they are `std::vector::resize`, not push).
- Readers with sub-tables: Line "Points", Group (a five-byte JMP), Curve "PosArray" plus Texture.
- FrameBox: stream accessor, state count (2Ch records), destructor, state-record insert.
- AnimIcon: loaded hook and the update (x87 read from the bytes; see the header comment).
- Named and timed entries, the hoisted-widget adopt, owned-object deletes, Layer copy, Scrollbar
  loaded hook, manager cursor position, render-queue reset, Curve SetTexture and destructor,
  Model copy.
- 35 VS2008 container internals (tree head/clear/destroy/_Max/_Dec, list sentinel/buy node/
  clear/destroy/construct/copy, `_Allocate` with its overflow check, deallocate, free helpers,
  vector size).

More findings from these batches:

7. The Scrollbar loaded hook 00AC8890 looks up its four arrow/background/slider children and
   discards every result (EAX is never read); it only matters for the lookups' side effects.
8. The Layer copy constructor 00AC6040 resets the store descriptor (+108h..+11Ch) to its defaults
   (near 0.1, far 1000, authored Priority 0, RenderOrder 0.0) instead of copying it, and does not
   copy +F4h; the Model copy does not copy the scene (+ECh/+F0h) but reloads the model.
9. `_Dec` 00AA1210: the out-of-range test after the climb tests the iterator's node, which the
   climb only moves to non-nil parents, so decrementing begin() silently yields the head.
10. 00AA2B30 and 00ABDD80 pass an uninitialized stack pair as the resolver's size argument.

## Remaining in the range (not reconstructed here)

Of the 276 functions listed at the start (ledger name, no record, no tag), 19 remain:
00AA2020 (GUI ortho projection), 00AA4040 (render-mode worker start), 00AA5A00 (existing
`collect_sorted_children_00aa5a00` uses std::sort, so equal names do not permute as the image
does; it should call `msvc9_sort::sort` before it gets a record), 00AA5B40 (Listbox copy),
00AAD9A0 (Movie describer), 00AB10D0, 00AB3840, 00AB3B30, 00AB44B0, 00AB64C0 (Icon state and
texture routines), 00AB7B60 (Text describer), 00ABC7A0 (Curve describer), 00ABCC70 and
00AC2430 (Curve/Line geometry), 00AC0C90, 00AC6BE0 (unreferenced), 00ACC8A0, 00ACCB90,
00ACCCF0 (Model material and load).

## Handoff

- Ghidra: apply the 85 ledger name changes (`tools/ghidra_annotate.py --apply` over
  `config/names/00aa0000.jsonl`..`00ad0000.jsonl` for the cc13-gui1 records), and create
  functions for 00AA7880, 00AA7CF0 and the hidden type-descriptor initializers listed in
  finding 6 (a flow-repair packet; check the INT3 padding and run the shape comparison first).
- Next reconstruction targets, in order of value: the describers 00AAD9A0, 00AB7B60, 00ABC7A0
  (pattern: `gui_widget_describe_properties_00aaaed0` plus `write_unless_default`; read names
  from the listing because the decompiler drops the describer's stack arguments), then the
  Icon state routines (layout in `gui_icon.hpp`), then the geometry writers (x87; read DC-form
  operands from the bytes).
- Scripts used (in the worktree's `local/`, prefix `gui1_`): `gui1_poolscan.py` (pool constants),
  `gui1_shape.py` / `gui1_pair.py` (normalized instruction comparison), `gui1_dis.py` (compact
  disassembly), `gui1_rec.py` + `gui1_mk.py` (record batches), `gui1_callrows.py` (call report).