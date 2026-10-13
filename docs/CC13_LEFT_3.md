# CC13 leftovers pass 3 (packet cc13-left3)

This packet reconstructs bodies that the cc13 system passes left open, as listed in their handoffs:
`docs/CC13_SYS_6.md` ("Continuation (packet cc13-sys6b)"), `docs/CC13_SYS_4.md` (sys4b) and
`docs/CC13_SYS_3.md` (open items).

Status terms: *reconstructed* means C++ exists in `src/` with a ledger record; *build-tested* means
it compiles in the MSVC Win32 build (`scripts/build.ps1`). Nothing here is ABI compatible, wired
into a running host, or game-validated. Names are descriptive hypotheses kept from the ledger.

Evidence for every body: live Ghidra decompile and `bsp.py disasm-raw` over the whole body;
constants and strings from the PE (`tools/pe_const_read.py`); x87 operand order from the bytes
(`tools/x87_dc_form_scan.py` and the raw opcode bytes for DE-form pops).

## 1. Segments 49 and 78 (`engine_seg49_64_78_cc13.*`)

Eleven records.

| Address | Function | Notes |
|---|---|---|
| `008023B0` | `rain_emitter_build_008023b0` | The rain model build. See below. |
| `007FB520` | `collect_content_ids_007fb520` | "DL_Content_" ids from both unlock sets. |
| `007FC490` | `checkpoint_get_copy_007fc490` | The checkpoint map +94h, find or insert, then copy. |
| `007FCD90` | `evaluate_rank_award_007fcd90` | The profile's award map +A0h after a mission. |
| `00AAE770` | `gui_movie_loaded_hook_00aae770` | The movie widget's one-quad mesh. |
| `00AB4CE0` | `gui_anim_icon_visit_child_paths_00ab4ce0` | Builds per-frame path strings and drops them. |
| `00AB9240` | `gui_text_position_cursor_00ab9240` | The text cursor quad (x87). |
| `00AC0610` | `gui_frame_box_place_border_icon_00ac0610` | x87, register-free stack ABI. |
| `00AC0970` | `gui_frame_box_set_icon_extent_00ac0970` | `__stdcall`; ECX is not read. |
| `00AC0BD0` | `gui_text_icon_pair_layout_00ac0bd0` | x87. |
| `00AC10A0` | `gui_frame_box_layout_border_icons_00ac10a0` | The nine-slice layout, run twice for text. |

### The rain build draws stream 0, not stream 1

The packet brief said that the rain build's draws are on stream 1. The listing shows otherwise. All
seven `00BD2F10` calls in `008023B0` are preceded by `XOR ECX, ECX`, for example at `00802500` and
`00802984`. `reports/cc13_rng_stream_census.json` agrees: sites `00802507`, `00802550`, `008025A3`,
`00802608`, `0080266D`, `008029B0` and `008029D1` are listed as stream 0. Only the per-frame step
`00801400` uses stream 1. The reconstruction therefore routes the build's draws through
`RainBuildHost::random_uniform_00bd2f10_stream0`, and its comment records that the image shares
gameplay stream 0 here. Following the RNG rule, a host that wires the build in must give it a
separate presentation stream or keep it behind an OFF switch. The build runs once per preset, from
`BSP_WorldAtmosphere_ApplyPreset`. It draws `5n + 2n` numbers for `n` columns, so in the image a
weather preset change shifts gameplay stream 0 by `7n` draws.

### Other findings

- **Rain model.** The build fills `+1C4h..+1F7h` from the descriptor. `RainModelFields` now names
  `+194h`, `+1A0h`, `+1CCh..+1DCh` and the shader's `box_radius` at `+1F0h`, and the `+184h`
  vector holds `RainColumn` records. The draw section's vertex count is
  `trunc(([00F87474] >> 2) * [00F87470] * 4.0)`, read back after the build sets the fraction
  `[00F87470]` to 1.0. The section is appended to the mesh twice, before and after `00B73C10`.
- **`00AAE770`.** The mesh is released right after `00B75170` takes it. The final release in the
  epilogue releases the vertex format, not the allocation: `00AAE863` overwrites `[ESP+1Ch]`.
- **`00AB4CE0`.** The per-frame string is built and released without being used. The
  reconstruction returns those strings so that the work stays observable.
- **`00AB9240`.** The bearing comes from the glyph that `00AD4480` selected last. That is the glyph
  of character `index - 1`, or of the first character when the index is 0. The height term is
  `(fh + 1) - 1`, computed on the x87 stack (`DC C2` at `00AB9349`, then `DE EA` at `00AB934B`). `[00E12FD4]` is a `.data`
  float with the image value 0.75 and is read through the host.
- **`00AC0970`.** The x-update test compares `w` with vertex 1's x. The y-update test compares `h`
  itself, not `0.75 * h`, with vertex 2's y, and then writes `[00E12FF0] * h`. `00E12FF0` is a
  `.data` 0.75 with exactly two literal readers (both here) and no literal writer, so it stays a
  constant.
- **`00AC10A0`.** The second argument of `00AC0610` (`&size`) is never read. The text branch pushes
  the border flag as a dword whose upper three bytes are stack residue, because `00AB6CB0` writes
  only one byte.

### Not gameplay

`007FCD90` updates the profile's post-mission award map, which is meta-state. No item in this
section moves simulation state, so the For cc9 list is empty.

## 2. Segments 4, 6 and 9 (`engine_seg4_6_9_cc13.*`)

Twelve records.

| Address | Function | Notes |
|---|---|---|
| `0044C970` | `session_message_e1_construct_0044c970` | Builds the E1 message's `Dialogues = {...}` Lua source. |
| `004CAC30` | `game_pump_front_end_frame_004cac30` | Front-end frame: pump, GUI update, render. |
| `004CCBD0` | `game_render_foliage_and_shadows_004ccbd0` | Foliage visibility, shadow view, main scene. |
| `004CB420` | `scene_environment_block_construct_004cb420` | Default environment block and terrain globals. |
| `004CCCC0` | `game_leave_session_004cccc0` | Session teardown sequence. |
| `004CD450` | `game_save_next_screenshot_004cd450` | Unreferenced screenshot writer. |
| `004C9EC0` | `scene_ensure_default_lighting_004c9ec0` | "AllLights", ambient, "TestDirectionalLight". |
| `00471080` | `anim_entity_sentity_init_00471080` | Draws stream 0. See below. |
| `00479BD0` | `vehicle_visual_entity_sentity_init_00479bd0` | Base init, then +18Ch = 0. |
| `00466BB0` / `00466C00` | `scene_name_hash_iterator_begin/next` | The 64-bucket chained hash iterator. |
| `00467C80` | `scene_property_value_construct_vector3_00467c80` | Byte/word header, xyz, w. |

- **`0044C970`** takes its LuaObject argument by value (14h bytes, `RET 1Ch`) and reassigns it to
  `table["sequence"]` before iterating. `00B662B0` (GetString) takes no stack argument, so the
  `00CE4AFC` push before it (`",},`) is the argument of the third append.
- **`004CB420`** has no code worth transcribing by hand: it is 118 block stores and 85 global
  stores. `local/l3_envblock.py` resolves them from the live decompile, tracking the `uVar`/`local`
  temporaries and reading `.rdata` constants from the PE. `local/l3_envcheck.py` then compares the
  set of offsets with every `[ESI+disp]` and absolute store in the listing. The listing has 116
  `[ESI+disp]` stores; `+140h` and `+20Ch` are written through `EDI`/`EBP`. The 85 absolute stores
  match the table with no difference. The globals are the terrain shader globals that `004EE9E0`
  overwrites (cc13-weather1).
- **`00471080`** draws `00BD2F10` with `ECX = 0`, which is stream 0 (census site `004710BB`,
  classed PRESENTATION). The reconstruction routes the draw through
  `AnimEntityInitHost::random_uniform_00bd2f10_stream0`, and its comment records that the image
  shares the gameplay stream.
- **`00466BB0`** tests the bound before it increments, so its scan can stop at bucket index 40h and
  read one slot past the 64 heads (`table+108h`). A non-zero count rules that case out.
  `SceneNameHashTable` keeps a 65th slot so that the read stays defined.

## For cc9

- `004C3C90` (no callers): `004BFDF0` clears the local player's unit lists, then the 004C3CB0
  latch `game+193Ch` = 0. This is gameplay unit-list state, which lives in `game_hosts_world`.
  Not reconstructed here.
- `004C6BE0` `BSP_Game_GenerateObject` (caller `00944FD0`, which `cc9-lua52` holds a lease on):
  `0046DC10` creates a hidden entity, and `004C3840(0)` runs when `game+1FE4h != 0`. This creates
  gameplay entities. Not reconstructed here.

## 3. Segments 11, 35 and 66 (`engine_seg11_35_66_cc13.*`)

| Address | Function | Notes |
|---|---|---|
| `0098C9C0` | `describe_os_version_0098c9c0` | The MSDN "DisplaySystemVersion" sample with Vista strings, taken branch by branch from the listing. |
| `00995690` | `download_manager_scan_content_00995690` | Re-verifies the installed content packages and mounts them. |

- **`0098C9C0`**: the decompile scrambles the NT branch, so the control flow comes from the listing.
  For the EX path, server product types (2 and 3) other than 5.0 and 5.2 append "Server 4.0 " or
  "Server 4.0, Enterprise Edition ", and that includes Vista servers. The Q246009 probe calls
  `RegCloseKey` even when `RegOpenKeyExA` failed. A registry query that fails, or a `cbData` above
  50h, returns false with the key still open.
- **`00995690`**: the decompile is register-spilled, so the body comes from the listing. Findings:
  - The `.mpkg` test is `index == size - 5` with `index = -1` when there is no match. A
    four-character name without ".mpkg" therefore also passes.
  - When the size probe does not return `8007007Ah` with a size of 100h or less, the wide path
    buffer is never written, and `00436630` converts whatever the stack holds.
  - The folder and package-name strings live at function scope. A record skipped by the
    `+10h && !+11h` test that still has `+11h` set after the recompute pushes the previous
    record's folder and name. `ContentScanRecord` is a string view of the existing
    `ContentPackageRecord`.

## x87 precision

The rain build and the GUI layout bodies run on the main thread. From device creation onward its
precision control is 24 bits (`docs/CC13_FPU_PRECISION.md`), so every x87 operation rounds to float
precision, including values held on the stack. The reconstructions in section 1 therefore use float
arithmetic. The first build used double intermediates; the follow-up commit switches them to float.
`rain_emitter_step_00801400` (sys6b, same file) still models its x87-held threshold `dt * 0.3 * k`
in double. Under the 24-bit control word that should also be float. It has not been changed here.
