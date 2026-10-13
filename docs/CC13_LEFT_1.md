# CC13 left 1: bodies left open by the cc13 water, terrain and sound passes (cc13-left1)

Worker cc13-left1, branch `agent/cc13-left1`, 2026-10-13. It continues the handoffs in
docs/CC13_WATER_2B.md (segments 92-94), docs/CC13_TERRAIN_1.md "Pass 2" and docs/CC13_SOUND_1.md "Pass 3".

Status: **reconstructed, build-tested** only. `./scripts/build.ps1` passes with these sources. Nothing here is
ABI-compatible, fixture-tested or game-validated, and nothing is wired into a host. No test was added.

## Counts

| area | open at start | reconstructed and recorded | still open |
|---|---|---|---|
| water and weather, segments 92-94 | 99 | 88 | 11 |
| terrain, 00AD5590-00B1BD20 (Pass 2 list) | 17 | 11 | 6 |
| sound, segment 74 (Pass 3) | not started | 0 | all |

That is 99 reconstruction records, all `reconstructed_build_tested`. No Ghidra annotation was applied, and no
ledger name was changed.

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

### Release-sequence destructors

Ten destructors are lists of steps run in listing order: release a reference, unlink a node, free,
release a string, destroy a vector, or clear a material's parameter owner (`W3_ReleaseCalls`).

- `00BA24B0` wave field base (34 field steps and six vectors).
- `00BA2E60` beams, `00BA42A0` water particles, `00BA71C0` height generator.
- `00BAC8A0` tracer resource, `00BAE170` spray resource, `00BAE210` spray object.
- `00BB03D0` coastline, `00BB0C90` coast mesh.
- `00BBB660` MSAR provider. Its entry range destroy `00BBB020` takes ECX first, EDX last.

The listing settles the details the decompile blurs:

- `00BA2E60` and `00BA42A0` unlink a node without clearing the slot.
- `00BAE210` unlinks with no null test.
- `00BB0C90` frees two blocks without clearing them.

### Constructors and factories

- **Settings constructors.** Beams `00BA3A60` and water particles `00BA43B0`. Each builds a default
  scene-record layer in its frame, copies the settings out of it (`0078D9F0` / `0078D940`) and stores
  a 0.0f.
- **MSAR provider** `00BBB5B0`.
- **Factories.** `00BA6FB0` and `00BB3C60` are `__fastcall` with register arguments. The four
  application factories are `00BC02E0`, `00BC0350`, `00BC03C0` and `00BC0440`. `00BC03C0` re-stores
  its two float arguments through FLD/FSTP.

### D3D builders

These call the renderer device (`00F8D394`) through its native vtable. The slots are read from the
call sites:

- `+38h` declaration from a file;
- `+5Ch` vertex buffer, `+60h` index buffer;
- `+64h` texture by name;
- `+80h` back-buffer size;
- `+88h` render target, `+94h` depth surface.

The textures expose `+30h` surface, `+3Ch` width and `+40h` height. The surfaces expose `+1Ch` width
and `+20h` height. Engine routines go through `W3_EngineCalls`. Where the routine is a single load,
it is inlined: `00B74640`, `00B732C0`, `00B73260`, `00BA7080`, `00BC3100`, `00BC3080` and
`00BC3010`.

- `00B9DC20` reflection targets. The arguments of the back-buffer pairs are pushed interleaved.
  Depth format `50h` (D16) is used when the colour format is `17h`, otherwise `4Bh`.
- `00B9EA50` wave-damp object (no caller). `00BA4EB0` height-stream bind: `00B74640` takes ONE
  stack argument, so the second push of each pair is the element index.
- Height generator: `00BB3DB0` streams, `00BA94F0` constructor (its scale global `00E14454` is read
  per call), `00BB3AB0` derived constructor.
- `00BAE900` spray object and `00BB0E30` coast mesh. The coast mesh has 31 tuning DWORDs, read from
  .rdata on disk with `local/l1_consts.py`.
- `00BA6C40` wave field. Its base constructor `00BA1400` takes one argument.
- `00BC2D50` waterdrops geometry.
- `00BC54F0` cloud render model. It writes the shared block `01090904..010909AC`, so the caller
  passes a pointer to that block.
- `00BC6060` cloud plane model. Ghidra's body ends at `00BC63AB`; the index fill and releases after
  it were read from the PE.

### RNG

Nothing reconstructed here draws from a random generator. `00BAE2E0` (the spray resource, which
draws from stream 0) is still open.

## Terrain (Pass 2 leftovers): end of `terrain_particles_cc13.*`

The new image calls are in `TerrainLeftHost`.

- **Atlas group.** Unload `00AEFA30`, register `00AEF280`, load directory `00AEFF40` and registry
  destruct `00AEF090`. The split-name test `00AEF3C0` takes ECX name and EDX item. `004254B0`, the
  "Atlas file not found" log, is a bare RET in this build.
- **Floating particles** `00AF0F20`.
- **Bush type constructor** `00AE1E10`. Its two NativeStrings arrive by value, and the callee
  releases them.
- **Bush cell cull** `00AE1A10`. It returns running + n; the decompile's float return was wrong. Each
  child's result already includes the running value it was given, so the parent's sum counts those
  values again. That doubling is the image's behaviour.
- **Bush object build** `00AE1FB0`. It saves and restores the camera far plane (`+1D8h`) and builds
  a 128h-byte cull record on its stack, with the planes' d lowered by 50.0. The source throws past 16
  planes, where the image would overwrite its frame.
- **Height field.**
  - `00ADC1C0` TGA dump. `00BF0050` takes ECX file, EDX image.
  - `00ADB6F0` byte stamp (no xref).
  - `00ADB980` get-or-create tile. Operator new's push shifts the frame for the tile store at
    `00ADB9D4`, which is easy to misread. The box quotients were traced by hand.

## Handoff

Branch `agent/cc13-left1`. What is still open, in the suggested order:

1. **Water (11).**
   - Plumbing `00BD29C0`, `00BD2A50`, `00BD2760` and `00BD2B30`. These are the named string tree and
     the assertion formatter. They need the STL tree helpers `00485F50`, `0049A230` and `00480620`
     (none reconstructed), plus `00BD23E0`, which is tagged stl_probable and not in the list. Bind
     the helpers, or reconstruct the map first.
   - Builders `00BA3BD0` (beams init), `00BADC70` (tracer init), `00BAD150` (skinned mesh) and
     `00BA44F0` (particles init).
   - `00BAE2E0` (spray resource). It draws from stream 0, so route that draw through a host callback.
   - The two largest: `00BA1400` (wave field base constructor) and `00BA7530` (height generator
     passes).
   - The `W3_EngineCalls` interface, the device-slot table and the `W3_Temporary` helper in Part f
     cover most of what these need.
2. **Terrain (6).**
   - `00AFDCF0` draws debug axes: three lines through device `+B4h`, coloured `FF0000FF` and
     `FF00FF00`. Its x87 needs the listing.
   - `00AE5160` is the tree type constructor.
   - `00ADE0E0` and `00ADC930` are the visibility-group builders.
   - `00AE5870` and `00AE2450` are the group initialisers.
3. **Sound.** The docs/CC13_SOUND_1.md "Pass 3" list is untouched.

Tools are in this tree's `local/`:

- `l1_open.py` and `l1_sizes.py` list what is open in a band. Run `bsp.py index --if-stale` first.
- `l1_consts.py` reads .rdata DWORDs from the PE.
- `l1_ledger.py` adds the records from a `|`-separated file.
- `l1_core.ps1` builds only `bsp_core`.
