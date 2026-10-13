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

## Part B: segment 72, online (00A46B80-00A625D0)

At the start 146 functions in the range had a ledger name but no record and no tag. Grepping first
showed that 15 of them already had code from cc13-sweepA1 (`src/network_online_cc13.cpp`, documented
in docs/CC13_BAND_00A1_00A4.md) but no reconstruction record. Those records were added, with
00A47900 re-checked against the listing.

New code is in `include/bsp/online_cc13.hpp` and `src/online_cc13.cpp` (namespace
`bsp::online_cc13`, registered in cmake/startup.cmake). All of it is reconstructed and build-tested,
and none of it is game-validated. 69 records:

- **XLiveSystem slots (31).** Field getters (+ECh, +EDh, +124h, +128h, +8Ch, +1BD5h, +1BDCh,
  +1D28h, +1D40h), constant and no-op slots, the 00A47A60/00A47A80 twins, and 00A48E90.
- **System (2).** 00A46EE0 shutdown and 00A46F30.
- **Constructors (4).** Voice 00A48F10, session 00A499D0, matchmaking 00A49CF0, host QoS 00A4A380.
- **Voice (9).** 00A49000 GetLocalChatData, 00A49040 SubmitIncomingChatData, 00A49220,
  00A492D0, 00A49530 update, 00A48F60 mute list, 00A495E0 add talker, 00A49840 remote talking,
  00A493A0 destroy. The XHV2 vtable slots are named from the IXHV2Engine method order, which
  sweepA1's +24h/+2Ch/+48h already match: +0Ch StartLocalProcessingModes, +10h Stop..., +14h
  StartRemoteProcessingModes, +20h RegisterLocalTalker, +28h RegisterRemoteTalker, +30h
  GetRemoteTalkers, +34h IsHeadsetPresent, +38h IsLocalTalking, +3Ch IsRemoteTalking, +44h
  GetLocalChatData, +4Ch SubmitIncomingChatData. The mode arrays are 00D24A34 = {2} (voice chat)
  and 00D24A30 = {1} (loopback).
- **XSession (6).** Arbitration register, join and leave for local and remote players, remove
  remote player, and QoS listen.
- **Matchmaking and host QoS (12).** Remove-at, cancel, reset (with its JMP thunk 00A4B460),
  the byte-identical 00A4B410 cancel copy, release/finish, and the two relink-and-lookup routines.
- **Helpers (5).** Player-list construct and copy, _free-and-clear, pooled-string release, the
  00A4BDC0 scalar delete, and 00A4A850 XUserSetProperty.

**Rename.** 00A474E0 was `BSP_XLiveSystem_InsertSortedPlayer`. It is now
`STL_PlayerList_CopyConstruct_00a474e0`: it buys a sentinel, zeroes the size and inserts the whole
source range through 00A46800. Nothing in it sorts.

### Part B findings

1. **Host QoS pointer arrays overlap.** 00A4A420 writes the XNADDR, XNKID and XNKEY pointer arrays
   at +7Ch, +80h and +84h. They are one dword apart and the index steps by 4, so with two or more
   entries each iteration overwrites the previous one's XNKID and XNKEY slots. The matchmaking
   twin 00A49D20 spaces its arrays 19h entries apart. In practice the host QoS list is probably
   a single entry.
2. **00A495E0 has no capacity check** against the 16 talker records that 00A493C0 allocates.
3. **00A46EE0 returns false** when online was never started, even though BL = 1 is loaded first.
4. **00A48F60 ignores XUserMuteListQuery's result.** After a failure it applies the previous
   talker's `muted` value, or uninitialized stack for the first talker. It handles user slot 0
   only.
5. **00A49000 never reads its second argument.** It returns the engine's written byte count, or 0.

### Part B not done (in order of value)

- **System routines with player records:** 00A46D60 LeaveSession, 00A46F60 BeginSessionSearch,
  00A47130, 00A47220, 00A47350, 00A47AA0, 00A48330, 00A47B80. The player record is 3Ch+ bytes:
  an MSVC9 std::string at +0, an XUID/XNADDR union at +20h, the user slot at +30h, and flag bytes
  at +38h..+3Bh with +3Ah = 1. The layout comes from the stack frames of 00A47130 and 00A47220 and
  still needs a struct.
- **Large routines:** 00A488D0 Update (5C0h), 00A48450 RefreshHostQosUi (480h), 00A49DF0
  PollSearch (590h), 00A4A4E0 host QoS Poll, 00A4A890 SetUserProperties, 00A4AC40 Create,
  00A4B2A0 / 00A4B7F0 BeginSearch, 00A4B490 SelectQueryId, 00A4BB00 FillFromGameSetup,
  00A493C0 voice Initialize. The decompiler loses register arguments in several of them (00A48F60
  and 00A49840 needed the listing), so check each XLive call's argument count in the listing.
- **Movie decoder bases** (00A4C470..00A4D2F0, about 30 small vtordisp thunks, nops and
  constructors). Check src/movie_decoder.cpp first: it mentions 00A4C770, 00A4C8B0, 00A4C920,
  00A4D040 and 00A4D0F0.
- **The TELEMETRY_iostdnet and CreateNamedPipeW entries** (00A500E0..00A607EE) are library code.
  They should be tagged rather than reconstructed.

## Handoff

- Shas: see the final report. The branch is `agent/cc13-gui2`. origin/main was merged before the
  last build.
- Records: Part A 19; Part B 69 new plus 15 sweepA1 back-records. Names: 13 new (the defined
  functions) and 1 replaced (00A474E0).
- Ghidra: 13 functions defined with names, and their exports refreshed. During Part B the shared
  exports directory (`exports/bsp` in the main checkout) disappeared; this was reported to the
  lead. Later reads used live `bsp.py ghidra decompile`. The 00A474E0 rename is in the ledger
  only, so apply it with `tools/ghidra_annotate.py --apply --addresses 00a474e0`.
- Scripts are in this worktree's `local/` with the prefix `gui2_`: `gui2_lst.py` (compact listing
  with string annotation), `gui2_raw.py` (one-line raw disassembly), `gui2_dec.py` (compact live
  decompile), `gui2_img.py` (strings and dwords from the PE), `gui2_grepaddr.py` (find existing
  code for a target list), and `gui2_b*.txt` with `gui1_mk.py` / `gui1_rec.py` (record batches).