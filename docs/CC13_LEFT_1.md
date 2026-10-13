# CC13 left 1: bodies left open by the cc13 water, terrain and sound passes (cc13-left1)

Worker cc13-left1, branch `agent/cc13-left1`, 2026-10-13. It continues the handoffs in
docs/CC13_WATER_2B.md (segments 92-94), docs/CC13_TERRAIN_1.md "Pass 2" and docs/CC13_SOUND_1.md "Pass 3".

Status: **reconstructed, build-tested** only. `bsp_core` builds with these sources. Nothing here is
ABI-compatible, fixture-tested or game-validated, and nothing is wired into a host. No test was added.

## Method

Bodies were read from the live `bsp.py ghidra decompile` (reliable, per the lead correction in
CC13_WATER_2B.md). The listing (`bsp.py disasm-raw`) was checked wherever the decompile hid an ECX or
EDX argument, for x87 copies (FLD/FSTP versus MOV) and for every thunk. Names are hypotheses.

The model works on native storage, as Part e did: `{data, count, capacity}` arrays, intrusive
reference slots and eight-byte strings. A routine a body calls that has no native-storage
reconstruction is passed in by the caller as a function pointer named after its address. That is a
binding, not a stub: nothing is invented and nothing is skipped.

## Water and weather (segments 92-94): Part f of `water_weather_cc13.*`

### Coverage regions and cell grids

The coverage region is 30h bytes: `+0h` owner, `+4h` byte vector, `+10h..+2Ch` eight DWORDs. The
grid vector holds 0Ch cell grids `{width, height, data}` whose cells are 18h bytes.

- Native-storage forms of the cell-grid resize `00B9D490` and assign `00B9E0C0`. Their semantic
  models already exist (`ocean_cell_grid_*`); the vectors need them on native storage. Not new records.
- Grid vector reserve `00B9E7A0` and resize `00B9EF20`.
- Region destroy-mask `00B9E6B0`, assign `00B9E6F0` (no self-assignment test), copy construct
  `00B9ED40`.
- Region vector reserve `00B9EDA0`, resize `00B9EFA0`, push back `00B9FFF0` and swap-last erase
  `00B9ECE0`. Resize zeroes only `+0h..+0Ch` of a new region.

### Lightning segments

`00BC7FA0` reserve and `00BC8150` append. Both copy the twelve floats with FLD/FSTP (listing at
`00BC7FEC` and `00BC817E`), so they are `w2_pod_reserve`/`w2_pod_push_back` at 30h with float copies.

### Runtime plumbing (00BD15D0-00BD2B30)

- `00BD15D0` destroys the second pool owner. It is the twin of `destroy_native_string_pool_00bd14c0`:
  its section sits at `+0B89A98h`, its depth at `+0B89AB0h` and its publication at `01090AAC`. It sets
  the shared small-return gate `01090AA4`.
- `00BD1890` is byte-identical to `00BD1860`, and so is reconstructed as a call to it. `00BD18F0`
  destroys an event. Neither has a caller or a data xref.
- String builder appends: char `00BD1B10`, float `00BD1D90` and pointer `00BD1E80`. The float form
  uses the builder's `+14h` format, falling back to `01090AB4`. Both follow the temporary schedule
  of the existing `append_native_log_int_00bd1bb0`.
- `00BD2180` is the error-reporter singleton getter (`0109DBDC`). It follows
  `debug_feature_lock_singleton_00be8360` and calls `error_reporter_construct_00bf2c40`.
- `00BD2390` copy-constructs a string into its argument. `00BD26B0` appends a C string to the log
  message text at `+20h`.

### Shells

- **Deleting destructors.** Nine of them share one body: `00BC51F0`, `00BC5D60`, `00BC6F70`,
  `00BC2A00`, `00BBE600`, `00BC7BE0`, `00BC93C0`, `00BC2010` and `00BBC460`. The header lists each one's
  vtable, body and dispose routine. Four of the dispose routines are pool frees.
- **Thunks.**
  - The `SUB ECX,4` adjustors `00BA24A0` and `00BA6F70`.
  - The cloud-instance type-info thunk `00BBE7C0`.
  - The member-forwarding open `00BB8E70`.
  - The factory-slot deletes `00BBC4A0` and `00BBC4C0`, which are twins.
- **Constructors.**
  - Ref-counted base `00BC7B60`.
  - Caustics and shore-wave texture sources `00BBC670` and `00BBC7C0`, over the existing
    `construct_native_texture_source_00c30470`.
  - Pooled waterdrops `00BC2160`.
  - The GeneratedModel subclasses `00BC7DB0` ("cloudplane") and `00BC93E0` ("Lightning").
  - Cloud class `00BC4A60`.
- **Model pools.** The three `Mit::allocator_list_elem` pools have the DirectionalLightPoolStorage
  layout:
  - construct: `00BC5220`, `00BC5D90`, `00BC7190`;
  - destroy: `00BC4FB0`, `00BC5B20`, `00BC6D10`;
  - free thunks: `00BC51E0`, `00BC5D50`, `00BC6F60`.
- **Destructor bodies.** Lightning model `00BC8380`, cloud class `00BC4660` and sky box `00BBF6D0`.
- **Miscellaneous.**
  - Memory-stream cast `00BB8690`.
  - Tracer curve record destroy `00BACA80`, which has the `w2_destroy_via` shape.
  - `00BB5330`. It takes the member name in **EDX**: the body pushes the incoming EDX as the stack
    argument of `00BB4F40`. It has no caller.
