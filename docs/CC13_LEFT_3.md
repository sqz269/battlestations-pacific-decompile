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
