# GUI widget library, finishing pass (packet cc13-gui2)

Follows the handoff of `docs/CC13_GUI_1.md`. Part A closes the 19 functions that packet left in
segments 76-79 (00AA0000-00AD558F), defines the Ghidra functions its finding 6 listed, and fixes
its finding 5. Part B (segment 72, online) is described in its own section below.

## Status vocabulary

- **Reconstructed, build-tested**: C++ written from the listing and compiled by
  `scripts/build.ps1` (MSVC Win32). None of it is ABI-compatible or game-validated; every
  routine reaches the engine through a host interface.
- **Defined**: a Ghidra function created over existing bytes with
  `tools/ghidra_define_function.py` (record `reports/cc13_gui2_defined.json`), exports refreshed.

## Part A: what changed

### 00AA5A00 now uses the image's sort

`collect_sorted_children_00aa5a00` (src/gui_render_order.cpp) called `std::sort`. It now calls
`gui_widgets_cc13::msvc9_sort::sort(first, last, count, pred)`, the VS2008 introsort that
cc13-gui1 transcribed from instantiation set A, with `_Ideal` = the element count exactly as
00AA5AF7..00AA5AFD pushes it. Equal names therefore permute as in the image. The header comment
that described the order as unspecified was corrected. The record was added only after the switch.

### Reconstructed (19 records)

| Address | C++ | Notes |
|---|---|---|
| 00AAD9A0 | `gui_movie_describe_properties_00aad9a0` | base describer, then MovieName unless "" |
| 00AB7B60 | `gui_text_describe_properties_00ab7b60` | 14 keys; MISColors sub-table; shadow keys only when DefaultShadow is -1 |
| 00ABC7A0 | `gui_curve_describe_properties_00abc7a0` | PosArray integer-keyed Vec3 (x, y, width), Texture always |
| 00AB10D0 | `gui_icon_rebuild_current_state_00ab10d0` | vtable +8Ch: vtable +80h(MOVZX current state) |
| 00AB3840 | `gui_icon_apply_delayed_texture_00ab3840` | delayed-load delivery per matching state |
| 00AB3B30 | `gui_icon_state_set_delayed_placeholder_00ab3b30` | transparent.tga placeholder |
| 00AB44B0 | `gui_icon_set_state_texture_00ab44b0` | rename + authored UV + resolve |
| 00AB64C0 | `gui_icon_add_state_00ab64c0` | append a state from a live texture |
| 00ABCC70 | `gui_curve_create_geometry_00abcc70` | vtable +74h |
| 00AC2430 | `gui_line_create_geometry_00ac2430` | vtable +74h |
| 00ACC8A0 | `gui_model_setup_material_constants_00acc8a0` | recursive over the node tree |
| 00ACCB90 | `gui_model_load_resource_00accb90` | |
| 00ACCCF0 | `gui_model_load_model_00acccf0` | |
| 00AC0C90 | `gui_nine_slice_construct_00ac0c90` | unreferenced |
| 00AC6BE0 | `gui_preload_lua_assets_00ac6be0` | unreferenced, register ABI |
| 00AA5B40 | `gui_listbox_copy_construct_00aa5b40` | unreferenced copy constructor |
| 00AA2020 | `set_gui_ortho_projection_00aa2020` (gui_render_order) | x87; see uncertainty below |
| 00AA4040 | `prepare_and_start_render_worker_00aa4040` (gui_render_order) | |
| 00AA5A00 | `collect_sorted_children_00aa5a00` (gui_render_order) | record added after the sort switch |

The Icon routines use the typed `GuiIconWidget` / `GuiIconState` layout of
`include/bsp/gui_icon.hpp` (now included by `gui_widgets_cc13.hpp`); the others use raw native
offsets like the rest of cc13-gui1's code. Each declaration in the header carries address, ABI and
listing evidence.

### Ghidra functions defined (13) and names added

`tools/ghidra_define_function.py` under the write lock, after checking with `list_analyzers` that
'Non-Returning Functions - Discovered' is disabled; no analysis was run. Ledger names were added
first so each function took its name on creation; exports were refreshed with
`bsp.py ghidra export ... --force`.

- 00AA7880 `BSP_GuiWidgetPool_FreeThunk`, 00AA7CF0 `BSP_GuiWidgetPool_AllocateStub`.
- 00AA9320 `STL_PopHeap_WidgetByName_00aa9320` (VS2008 `_Pop_heap` of sort set B, found between
  00AA92E0 and 00AA9350; not in finding 6).
- Type-descriptor initializers, each a pair: a variant that returns nothing and a variant that
  returns the descriptor in EAX. 00AA9350 `BSP_GuiElement_TypeIdInitReturn`; 00AB31D0
  `BSP_GuiIcon_TypeIdInitReturn` (the plain variant 00AB3180 already existed); 00AC2390 /
  00AC23E0 Line; 00AC8500 / 00AC8550 Scrollbar; 00ACEA20 / 00ACEA70 ClipBox; 00AD0780 / 00AD07D0
  FrameBox (`BSP_Gui<Class>_TypeIdInit` / `..._TypeIdInitReturn`). Class names are from the
  string each writes at +0Ch.
- 00AB10D0 already had a Ghidra function (its ledger note said otherwise).

A byte scan (E8/E9 rel32 and absolute dwords; the same scan finds the known callers of the
sibling stubs 00AAC260, 00AABD60, 00AB24A0, 00AB3180) finds no reference to any of the 13 new
functions. They are dead code in INT3 padding: the two Widget pool stubs included, so the Widget
pool is reached only through its allocate/free routines directly.

### Finding 5 fixed

`src/gui_layout_loader.cpp` (last edited 2026-09-11 by a Codex lane, unleased) had wrong literal
addresses in `kBaseProperties`. Re-read from the 00AAA710 listing (00AAA783..00AAACB9):
LowColor 00D5C214 (was 00D5C1F4, which is inside "WideScreenAlign"), HighColor 00D5C208 (was
00D5C1EC, "WideScreenAlign"), and WideScreenAlign itself 00D5C1EC (was 00CE92E4, which is the
value literal "Left"). The other rows match the image.

## Part A findings

1. **Text ShadowPos is constant.** 00AB8028 compares +160h with 1, but no branch reads the flags
   and both outcomes store 00D5C604 "Behind". A page re-serialised by the describer always says
   ShadowPos = "Behind".
2. **00AB44B0 passes an uninitialized size** to 00AA2660 (the pair at [ESP+10h] is never
   written). 00AA2660 fills a size only when both components are zero, and the result is
   discarded, so the only effect is on whether that fill runs. It also releases the resolved
   texture without a null test.
3. **00ACCCF0 dereferences +ECh without a null test** when a texture override is present; a
   ModelName that fails to load combined with ModelTextureOverride faults.
4. **00ACCB90 runs 00ACC8A0 over the widget's scene (+4Ch), not the loaded root**, so the
   constants are registered over the whole scene subtree, including any other node already
   parented there (the walk itself was read; what else hangs under +4Ch was not checked).
5. **00AC2430 releases its mesh object right after handing it to the scene** and keeps using
   it; the same for the material after the draw section takes it. Correct if the scene and the
   section hold references, which the code implies but this packet did not verify.
6. **00AC6BE0 resolves textures with scale 0.0** (FLDZ), where 00AB44B0 and 00AB66C0 (gui_icon.hpp)
   use 1.0; it is unreferenced, so this does not matter at run time.
7. **00AB10D0 zero-extends the state index** (MOVZX), so the constructor's -1 reaches vtable
   +80h as 0000FFFFh; 00AB3CB0's unsigned range check rejects it either way.

## Uncertainty

- 00AA2020: the two divisions run on the x87 at the process precision-control setting; the C++
  rounds a double quotient to float. On 4:3 the quotient is exactly 1; elsewhere a last-bit
  difference is possible only through double rounding.
- The texture-reference model (retain/release on the host) follows the inlined
  InterlockedIncrement/Decrement pairs and the record copy 00AB3A70 / release 00AB3280; the
  texture's vtable slot 0 at count zero was not read.
- `tools/verify_report_calls.py` checks JSON call-row reports; this packet has none, so it was
  not applicable.
