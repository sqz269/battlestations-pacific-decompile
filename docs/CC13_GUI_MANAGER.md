# GUI manager and widget base, segments 76 and 77 (packet cc13-gui6)

Addresses: 00AA0F70-00AA6720 (segment 76) and 00AA6750-00AACE40 (segment 77).

Code: `include/bsp/gui_manager_cc13.hpp`, `src/gui_manager_cc13.cpp`.
Names: `config/names/00aa0000.jsonl` (136 new or replaced records, all prefixed
"Hypothesis, not a recovered symbol. cc13-gui6"), applied to Ghidra with
`tools/ghidra_annotate.py --apply` (prior values in the annotate tool's local backup).

This packet was resumed after a power outage. The previous worker's tree held no GUI-manager
work: its 13 "uncommitted files" were an unfinished `git merge origin/main` (MERGE_HEAD
ebed5ecf8, a startup.cmake conflict). That merge was aborted and origin/main re-merged clean.

## Status

| Level | What |
| --- | --- |
| Named in ledger and Ghidra | all 132 unnamed functions of both segments, plus four bad names replaced (below) and 00AAC410, which has no Ghidra function |
| Reconstructed, build-tested | 17 routines in `gui_manager_cc13.cpp` (table below); `./scripts/build.ps1` passes |
| ABI-compatible / game-validated | none. The C++ is new interfaces over host objects; the hover pass never runs in the image |

Most of segment 76/77's spine was already reconstructed by earlier packets (manager construct,
load, update, draw, hit test, page registry; widget transform, visibility, colour, clip refresh,
property reader and describer). What was left is the factory stubs, two more std::sort
instantiations, the two Mit pools, the Button class, and a handful of small widget routines.

## Replaced names

| Address | Old | New | Why |
| --- | --- | --- | --- |
| 00AA6750 | `x` | `BSP_GuiWidget_GetResolvedPosition` | the old ledger name was a single letter (a tool artefact); the evidence text already described the resolved-position walk that `resolved_position()` in `include/bsp/gui_widget.hpp` reconstructs |
| 00AA6BE0 | `TRIV_body_00aa6be0` | `BSP_Gui_SetPointerActivationReader` | `MOV [00F8BC84],ECX; RET`; BSP_Game_OnInitOnce passes 00696430 `BSP_GuiInput_ReadPointerActivation` (004DD6DB) |
| 00AAB5E0 | `TRIV_body_00aab5e0` | `BSP_GuiButton_SetSelectionCallback` | `MOV [00F8BCFC],ECX; RET`; BSP_Game_OnInitOnce passes 00941250 `BSP_GuiSound_RequestSelectionFlags` (004DD705) |
| 00AAC260 | `CG_static_dtor_stub_00aac260` | `BSP_GuiButton_AllocatePoolBlock` | a pool stub (`MOV ECX,pool; JMP 00AABF80`) like the segment-79 stubs of docs/CC13_HUD_AND_WIDGETS.md |

## Reconstructed routines

| Address | Name | C++ | ABI | Notes |
| --- | --- | --- | --- | --- |
| 00AA2110 | BSP_GuiManager_SetSafeZoneVisible | `set_safe_zone_visible_00aa2110` | thiscall(manager, byte), RET 4 | see below |
| 00AA3F70 | BSP_GuiManager_UpdatePageList | `update_page_list_00aa3f70` | thiscall(manager, vector by value, float), RET 14h | see below |
| 00AA4EF0 | BSP_GuiManager_RelayoutAllPagesWideScreen | `relayout_all_pages_00aa4ef0` | fastcall(ECX = manager) | 00AA8710 on each page of a snapshot of the +14h vector |
| 00AA6B10 | BSP_GuiWidget_ContainsPoint | `contains_point_00aa6b10` | thiscall(widget, float[2]) -> AL, RET 4 | unreferenced |
| 00AA69E0 | BSP_GuiWidget_ScaleToExtent | `scale_for_extent_00aa69e0` | thiscall(widget, float, float), RET 8 | unreferenced |
| 00AA8EC0 | BSP_GuiWidget_SetHitClipRectRecursive | `set_rect_00aa8ec0` | thiscall(widget, float[4], byte), RET 8 | |
| 00AA8F30 | BSP_GuiWidget_SetHitClipRectFromWidget | `rect_from_widget_00aa8f30` | thiscall(widget, widget*, byte), RET 8 | |
| 00AA9C90 | BSP_GuiWidget_HoistFlaggedChildren | `hoist_flagged_children_00aa9c90` | thiscall(widget, string by value), RET 8 | only self-referenced |
| 00AA5390 | BSP_GuiManager_ShowOnlyWidgets | `show_only_widgets_00aa5390` | thiscall(manager, const vector*), RET 4 | unreferenced |
| 00AA2420, 00AA7810 | BSP_GuiWidget_NameLess (+twin) | `name_less_00aa2420` | cdecl(widget**, widget**) | |
| 00AA9AB0 | BSP_GuiWidget_GetQualifiedName | `qualified_name_00aa9ab0` | thiscall(widget, out), RET 4 | only self-referenced |
| 00AAA0F0 | BSP_GuiWidget_VisitChildPaths | `visit_child_paths_00aaa0f0` | thiscall(widget, const string&), RET 4 | widget-base virtual +6Ch |
| 00AAA2B0 | BSP_GuiWidget_FindChildByNameSuffix | `find_child_by_name_suffix_00aaa2b0` | thiscall(widget, const string*), RET 4 | 19 call sites |
| 00AAB980 | BSP_GuiButton_SetFocused | `set_focused_button_00aab980` | fastcall(ECX = button), tail JMP | |
| 00AABD70 | BSP_GuiButton_ReadProperties | `read_button_label_00aabd70` | thiscall(button, reader*), RET 4 | Button virtual +18h |
| 00AAC270 | BSP_GuiButton_DescribeProperties | `button_label_is_described_00aac270` | thiscall(button, writer*), RET 4 | Button virtual +1Ch |
| 00AAC410 | BSP_GuiButton_UpdateHover | `update_button_hover_00aac410` | cdecl(void), RET | no Ghidra function; unreferenced |

### The safe-zone frame (00AA2110)

The manager keeps `safezone_43_FrameBox` at +7Ch and `safezone_169_FrameBox` at +80h
(docs/APP_INIT_FONTS_GUI.md entries 9 and 10) and byte +84h for "a frame is shown".
Show picks one frame from the platform wide-screen byte `[0109CF04]+0Dh` and calls its virtual
+34h(1); hide calls +34h(0) on +80h, then +7Ch. Both directions are no-ops when +84h already
matches. The wide-screen byte is read only at show time, so an aspect change while a frame is
shown leaves the old frame up until a hide/show cycle. The only caller is BSP_Game_OnInitOnce,
which passes the byte at 00F88A31 (docs/APP_INIT_GAME_ENTRY.md).

### The page-list update (00AA3F70)

The vector arrives by value (16 bytes: proxy, first, last, end) with a float after it (RET 14h).
Manager +6Ch is set to `pages[0]` before `BSP_GuiManager_UpdatePointer` (00AA3FB1) and cleared
after the walk (00AA400E); while it is set, the hit test 00AA2F10 skips every other page, so
the pointer pass only sees the first page of the list. Each page gets virtual +40h(seconds) when its virtual +38h is true.
Compared with the full update 00AA4F80 (include/bsp/blocking_screen.hpp) there is no
live-entry test and no highlight reset. An empty list reaches the invalid-parameter handler
00BF6713 before anything else; the C++ returns instead and says so.

### Widget rectangle at +38h..+44h

00AA8F30 builds `{abs.x, abs.x + size.x, abs.y, abs.y + size.y}` from a source widget (the
pivot is not subtracted, unlike 00AA6B10) and 00AA8EC0 stores it at +38h..+44h, optionally on
the whole subtree. The base constructor zeroes these four dwords (00AA9390). The callers are
list-style screens copying a container's extent onto their rows (BSP_ControlBindingScreen
builder 0055D380, 00561BE0, BSP_LeaderboardsScreen_FillRows 00561EB0).

The reader is the recursive hit walk 00AA8BD0. Each edge is skipped when it is exactly 0.0
(UCOMISS against zero, LAHF, TEST AH,44h, JNP); otherwise the widget is rejected when its hit
box lies wholly outside that edge: left limit > box right (00AA8CDF..00AA8D3B), box left >
right limit (00AA8D41..00AA8D63), top limit > box bottom (00AA8D69..00AA8D87), box top > bottom
limit (00AA8D89..00AA8DAB). Which stack value is which box edge follows the pointer
containment compares that come next (00AA8DAD..00AA8DD1: pointer x below the value at x87
st(3) rejects, so that value is the box's left, and so on), tracked on the all-limits-zero path.
So the rectangle is a hit-test clip: rows scrolled fully outside a list box cannot be clicked,
while a partly visible row stays hittable over its whole box.

`include/bsp/gui_layer.hpp` (`GuiHitLimits`, a 2026-09-10 reconstruction) already modelled these
four compares correctly, but named the fields `gate_right`, `min_left`, `min_bottom` and
`min_top`. This packet renamed them `left`, `right`, `top` and `bottom` with the evidence above
and updated the four uses in `src/gui_layer.cpp`; the logic is unchanged. (An earlier version of
this section had the edge roles inverted and said a partly outside widget is rejected; the
pointer compares above corrected that.)

The sweep (`local/gui6_rect_scan.py 00A90000 00AD8000`: Capstone over every INT3-delimited
chunk, non-CALL/LEA memory operands at +38h, +3Ch, +40h and +44h on one base register) found
13 chunks: the writer 00AA8EC0 (positive control), the constructors 00AA9390 and 00AA9520, the
hit walk 00AA8BD0, and nine others. Those checked use the offsets on a different object, for
example the vertex element offsets at 00AC2BC3 in BSP_GuiLine_Flush; no drawing use of the
widget rectangle was found.

### The Button widget (type 15)

Layout type 15 (`kClasses` in src/gui_layout_loader.cpp) is a 0FCh-byte widget with vtable
00D5BED0. Earlier docs treated 00D5BED0 as a generic widget table (GUI_WIDGET_TRANSFORM.md,
GUI_WIDGET_SCENE_VISIBILITY.md); it is the Button's table, written by its constructor 00AAC6F0,
copy constructor 00AA3C60 and destructor 00AAC530. Inherited slots are the widget base's, so
the earlier slot observations stay valid.

| Offset | Field | Evidence |
| --- | --- | --- |
| +ECh | listener; virtual +4(button) on a click | 00AAC4E0..00AAC4EC |
| +F0h | label widget, the child named by `Label`; its virtual +80h takes the visual state | 00AABD70 (stored from 00AA7E00), 00AAB99F |
| +F4h/+F8h | `Label` native string (property key 00D5C23C, default 00CE3A0C) | 00AABD8A..00AABDB0 |

Globals: the registry `std::list<Button*>` at 00F8BD38 (sentinel 00F8BD3C; every default-built
button appends itself, the destructor removes itself); the focused button 00F8BCF8; the
selection callback 00F8BCFC; the pointer-activation reader 00F8BC84 (shared with the list box).
Label states: 0 normal, 2 focused, 3 disabled (byte +77h), with disabled taking precedence.
The destructor clears 00F8BCF8 whichever button is being destroyed (00AAC570..00AAC579).
The copy constructor copies +F0h verbatim, so a clone points at the source's label child until
its property reader runs, and it does not append the clone to the registry (00AA3C60 has no
list insert), while the destructor still removes it (a no-op `remove`).

The hover pass 00AAC410 has no Ghidra function and, after a full scan of the image, no rel32
call and no absolute dword reference: in this build nothing calls it, so buttons never take
focus or fire their listener through this path. 00AAB980 is reached only from it. The pass is
reconstructed anyway because the class's behaviour is otherwise unintelligible.

### Mit pools for the widget base and the Button

Two more `Mit::allocator_list_elem` instantiations, with the layout in
docs/CC13_POST_EFFECTS_AND_RESOURCES.md but 64 slots per block (that doc's `MitBlockPool` is
fixed at 32, so it is not reused):

| Payload | Stride | Block | Init | Allocate | Free | Trim (slot 0) | Ctor / dtor | Vtable | Global |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 0ECh widget base | F0h | 3C84h | 00AA6CA0 | 00AA7A80 | 00AA75F0 | 00AA7550 | 00AA79A0 / 00AA7470 | 00D5C120 | 00F8BC94 (from BSP_GuiWidget_DeleteBase_Provisional) |
| 0FCh Button | 100h | 4084h | 00AAB680 | 00AABF80 | 00AABAB0 | 00AABEE0 | 00AABE00 / 00AAB9F0 | 00D5C238 | via stub 00AAC260 |

A third instantiation serves the Movie widget: 00AACA00 `BSP_GuiMoviePool_InitBlock` (4 slots,
stride 134h, 130h payload, block 4DCh), allocated by 00AAD6D0 `BSP_GuiMoviePool_Allocate`
through the stub 00AAD990 `BSP_GuiMovie_AllocatePoolBlock` (`MOV ECX,00F8BD4C; JMP 00AAD6D0`),
which the Movie factory 00AA1770 calls.

## Named, not reconstructed

These were named from their bodies and references; they are library-shaped or trivially small,
and reconstructing them would duplicate the STL.

- **Factory stubs** 00AA1410..00AA19B0, 00AA2340, 00AA4440: one per `kClasses` row (Icon, Progbar,
  Line, Scrollbar, Grid, Model, Movie, Curve, Sound, ClipBox, Section, AnimIcon, Button). Each
  calls its pool allocator, then the default constructor for a null ECX source or the copy
  constructor otherwise, and returns 0 when the pool returns 0. Named `*_DefaultOrCopyFactory`
  after the existing 00AA1380 `BSP_GuiText_DefaultOrCopyFactory`.
- **Copy constructors**: AnimIcon 00AA1C60 (vtable 00D5BCE0), Button 00AA3C60, list box
  00AA5B40 (vtable 00D5BBF8, unreferenced: list boxes are not cloned through 00AA6560).
- **Two std::sort instantiations** ordered by widget name: one for 00AA5A00
  (`_Med3` 00AA2A30, `_Median` 00AA2D90, `_Push_heap` 00AA2A90, `_Adjust_heap` 00AA2E30,
  `_Unguarded_partition` 00AA34A0, `_Make_heap` 00AA37D0, `_Sort_heap` 00AA4550) and one for
  00AAAED0 (00AA7BF0, 00AA8080, 00AA7C50, 00AA8120, 00AA8FB0, 00AA92E0, 00AAA260); the shared
  `_Rotate` is 00AA1E60.
- **Tree, list and vector helpers**: iterator decrement 00AA1210, rightmost node 00AA1040,
  erase range 00AA49E0, the Button registry list (00AAC180, 00AAC210, 00AAC310, 00AAC3B0),
  the manager's named-widget list node 00AA4D30 / entry copy 00AA3D30, element-size allocators
  and size helpers, and the Mit `{data, count, capacity}` pointer-array helpers.
- **Small widget routines**: 00AA8610 alpha set over a subtree (+5Ch and material diffuse
  alpha), 00AA8B80 delete one timed entry, 00AAA650 move all children to a new parent,
  00AA8200 the `cGuiElement` type record, 00AAC820 movie texture binding (three planes from the
  player's virtual +38h), 00AA2B30 texture preload for vehicle-class icons, 00AA2B80 render
  queue reset.

### Unreferenced code

37 of the 132 functions have no rel32 call and no absolute dword reference anywhere in the
image (scan of every E8/E9 target in .text and every aligned and unaligned dword in the mapped
image), plus 00AAC410. They are mostly duplicate template instantiations, but include
00AA6B10, 00AA69E0, 00AAA650, 00AA5390 and 00AA5B40. Their names say so in the ledger evidence.
A scan cannot rule out a computed address; none of these is the target of a jump table.

## Hoisted children and show-only (00AA9C90, 00AA5390)

00AA9C90 walks a widget's children. A child with byte +76h set is handed to the manager
through 00AA59B0, which reparents it under the manager's +44h widget (00AAA5A0
AttachChild(child, 0)) and appends `{path, child}` to the manager list object at +34h (head
+38h). The recorded path is the one passed in: neither the child's nor the current widget's
name is appended, and the iterator steps before the adoption so the unlink is safe. Other
children are recursed into with the same path when the current widget is a page root (type 1),
else with `path + "." + name` (00AA9DF2..00AA9E70). Its only references are its own recursion
(00AA9DE8, 00AA9E70) and 00AA59B0 is reached only from it, so in this build the manager list at
+34h stays empty and nothing reads it (the `+ 0x38` hits in the segment-76 decompiles of
00AA2F10, 00AA3910, 00AA3F70 and 00AA4F80 are virtual +38h calls). docs/CC13_FRONTEND_MENUS.md
line 111 sets +76h through a virtual +4(1); whether that path was meant to feed this one is open.

00AA5390 (thiscall(manager, const vector*), RET 4, unreferenced) saves each listed widget's node
factor (+4Ch, then +ACh) as an int with CVTTSS2SI (00AA5446), sets factor 0 through 00B6DA70 on
every widget reachable from the manager pages in breadth-first order, then writes the saved
value back with CVTSI2SS (00AA5680) on each listed widget and every ancestor. The int round trip
truncates, so a saved 0.5 is restored as 0.

## Movie vtable slot +74h (00AAC7F0)

The vtable 00D5C270 follows the string `cGuiMovie` (00D5C250) and holds BSP_GuiMovie_Update
00AADF40 at +40h and BSP_GuiMovie_SetCompletion 00AAFEE0 at +7Ch, so it is the Movie widget's.
00AAC7F0 is its slot +74h (00D5C2E4). In the widget base that slot is 00A9AC00
`BSP_GuiWidget_BasePostConstructReturn`, a bare RET; the AnimIcon overrides it with 00AD2DF0.
The Movie's override calls BSP_GuiWidget_SetLocalPositionAndBounds 00AA7DC0 with (0, 0, 0), so it
is named `BSP_GuiMovie_PostConstructResetPosition`.

## Open questions

- Manager +44h, the adopting widget of 00AA59B0: not set by the constructor 00AA5D70; its
  writer was not looked for, since the only path to it is dead.

## Verification

- `./scripts/build.ps1` in the worktree: build and both existing test targets pass.
- `python tools/verify_report_calls.py reports/cc13_gui6_calls.json`: every call row checked
  against the live Ghidra bodies, 0 failed. The rows are generated by
  `local/gui6_make_report.py` from the raw image.
- Every reference count above comes from `local/gui6_absrefs.py` (full-image dword and rel32
  scan of the installed executable), not from `bsp.py callers`, which under-reports.
- 00AAC410 was created in Ghidra with `tools/ghidra_define_function.py 00aac410 00aac4fa`
  (234 bytes, under the write lock); `_free` 00BF9DC8 still reads as returning afterwards.
- No new tests: the routines are small and their risk is in the host bindings, which do not
  exist yet.

## Handoff

Segments 76 and 77 have no unnamed functions and no `_Provisional` names of their own left. Scripts in this worktree:
`local/gui6_absrefs.py` (full-image reference scan), `local/gui6_rect_scan.py` (field-offset
sweep), `local/gui6_make_report.py`, `local/gui6_names.tsv` (the applied names with evidence).
