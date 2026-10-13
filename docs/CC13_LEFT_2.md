# CC13 left 2: engine bodies left open by cc13-left1 (cc13-left2)

Worker cc13-left2, branch `agent/cc13-left2`, 2026-10-13. It continues the handoff in docs/CC13_LEFT_1.md
(water and weather, terrain) and the docs/CC13_SOUND_1.md "Pass 3" list.

Status: **reconstructed, build-tested** only. Nothing here is ABI-compatible, fixture-tested or
game-validated, and nothing is wired into a host. No test was added.

## Method

Bodies were read from the live `bsp.py ghidra decompile` and, for every builder, from the full listing
(`bsp.py disasm-raw`, compacted with `local/l2_asmf.py`). Every x87 register-register operation the
code depends on was checked against its bytes (`bsp.py ghidra bytes`), because the D8/DC forms print
alike. Names are hypotheses.

The native-storage conventions of Parts e and f carry over: `{data, count, capacity}` arrays, intrusive
reference slots, eight-byte strings, and a routine a body calls that has no native-storage
reconstruction is passed in by the caller as a function pointer or interface method named after its
address.

## Water and weather: Part g of `water_weather_cc13.*`

### The string map and the assertion record

The `map<string, string>` instantiation at 0048xxxx/0049xxxx is a byte twin of the physical-index
instantiation at 00BDxxxx (`local/l2_twin.py` compares the raw bytes; every differing byte is a rel32 call
target that pairs with the twin's own callee, or an SEH handler address):

| this build | twin | role |
|---|---|---|
| `00485F50` | `00BDABA0` | allocate a 20h node |
| `00480620` | `00BD9860` | iterator increment |
| `00494EF0` | `00BDF7A0` | erase a subtree (pair destroy `00489D50`, then free) |
| `00494C30` | `00BDFD80` | erase(iterator); its tag `STL_xlen_throw` is wrong |
| `0049A230` | `00BE0C30` | erase(first, last) |

Each is reconstructed as a call to the twin's existing native-storage body.

The assertion record (2Ch) is file `+0h`, line `+8h`, expression `+0Ch`, the map `+14h`, message `+20h`
and level `+28h` (0 WARNING, 1 DEBUG, 2 ERROR, 3 FATAL). `00BD29C0` constructs it, `00BD2A50`
destroys it, `00BD23E0` formats the context (`<key=value> ` per entry, then the message), `00BD2760`
formats the full report through the log builder, and `00BD2B30` reports it. None of the three outer
routines has a caller or a data reference: the machinery is dead in this build. The handler
`01090AB8` that `00BD2B30` prefers is written only by `BSP_AutoTest_ParseTestName` (`004278C2`); the
listing passes it the text in ECX.

### Builders

- `00BAE2E0` spray resource. Four 30h vertices per sub-element. The rotation r is a draw from
  `00BD2F10` on stream 0 in the image (ECX 0 at `00BAE4BD`); the source takes it from a
  `W4_PresentationRandom` the host supplies, per the cc9 RNG rule.
- `00BA3BD0` beams and `00BA44F0` water particles. The particles' vertex colours come from the
  module-private Mersenne state `00E13A88` (docs/CC13_RNG_STREAM_CENSUS.md), not a thread stream; the
  draw is `random_next_u32_00ba2c20` inlined.
- `00BADC70` tracer initialise and `00BAD150` skinned tracer mesh. The static builder `00BACBD0` and the
  vertex writer `00BAA0F0` are bound (their reconstructions are semantic models only). `00BFA910` is
  `_CIlog` (FLDLN2/FYL2X); `00BFEEEC` is taken as `_CIexp` from its use on k*log(x).

## Terrain: end of `terrain_particles_cc13.*`

The new image calls are in `TerrainLeft2Host` (and `TerrainLeft2NodeCalls` for the node pair).

- `00AFDCF0` is not a transform update: it draws the emitter's debug axes. Its stack slots were tracked
  through every push (`local/l2_esp.py`): X in `FFFF0000h`, then Z in `FF0000FFh`, then Y in
  `FF00FF00h`, each from the origin to origin + (M * unit - M.translation), through device vtable
  `+0B4h`. It runs only when BYTE `[[emitter +0A0h] +7Ch]` is set.
- `00AE5160` tree type constructor. The texture name falls back to the empty string at `00F8C260`
  (loader zero-fill). A material with no textures (WORD `+34h` <= 0) yields a null texture whose name
  address is 8, which the image dereferences.
- `00AE5870` tree group initialiser. The instance vertex buffer is read straight from the stream;
  the visibility object `00AE5550` gets the owner's box less the owner node's world translation.
- `00AE2450` bush group initialiser. The two names arrive by value and the callee releases them;
  the source takes ownership the same way. The x87 box centre and half extents were checked against the
  bytes (`DC C9`, `DE E6`/`E1`/`E2`).
- `00ADE0E0` flat visibility group. It is the `00ADB980` tile body run for a 4 x 4 grid, except that
  only the first height block's samples are set.

## Sound: the Pass 3 list in `sound_cc13.*`

- Native-node iterator steps `00A81AD0` (increment) and `00A81B80` (decrement), and the four
  wrappers `00A82360`, `00A82810`, `00A823C0`, `00A82870`.
- `00A81D10` is a byte twin of the scene-graph node allocator `00B76450`.
- `00A7F0D0` and `00A7FAD0` (resize to 0, then free), `00A89B10` (query lock destructor) and
  `00A7FE60` (listener preset).
- Resource cache records (2Ch): default constructor `00A84280` (the snapshot's `ConstructLists` is a
  misnomer), copy constructor `00A843A0`, pop back `00A84380`, reserve `00A84450` and erase-swap-last
  `00A84D20`. The record destroy and assign (`00A842E0`, `00A84680`) and the alias-list helpers
  (`004C3020`, `004D48A0`) have projected or pool-typed reconstructions elsewhere, so they are bound.
- `00A7EA30`, the debug dump: it builds one line per sound object and throws it away.
- `00BA1400` wave field base constructor. Its many pooled temporaries were matched to their stack
  slots, which settles the `00B4C700` operands: ECX the model name, EDX the geometry file and the first
  stack word the shader. The "OceanVelAndDepth" model it creates is rooted under the world and given
  the ocean triangle but never stored in a field.
