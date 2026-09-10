# World-content phase of cSkeletonAppMidway::Init

Addresses: 00af0060, 00740840, 00ad9ac0, 00ad71c0, 00af1450, 00bbcb40, 00af0b10

Packet `app_init_world_effects`, packet 9 of `docs/APP_INITIALIZE_MAP.md`. Worked read-only against
project `bsp`, program `/battlestationspacific.exe`, base 00400000. No Ghidra rename, comment,
function creation, prototype or save was made from this packet; reviewed names were added to the
ledger only. Every descriptive name below is a hypothesis, not a recovered symbol. Raw pseudocode
and listings live in ignored `exports/bsp/functions/<address>/`.

## Order in Init

Two of the seven addresses do not sit inside the phase-9 window. They are in this packet because
they own world-effect state that the phase-9 content depends on.

| Call site | Address | Phase |
|---|---|---|
| 0073d8cc | 00bbcb40 | 3, platform; runs before the window object is constructed |
| 0073de42 | 00af0060 | 9 |
| 0073de8c | 00740840 | 9 |
| 0073deba | 00ad9ac0 | 9 |
| 0073deda | 00ad71c0 | 9 |
| 0073df82 | 00af1450 | 9 |
| 0073e02b | 00af0b10 | 10, after the Bink and network objects |

Between 0073deda and 0073df82 Init runs 00b14a10, 00b3c4c0 and 00ad9f90, which belong to the
renderer-device packet and were treated as external. The indirect `00f8c218` vtable +0Ch call at
0073df87 was also left alone: 00f8c218 is written by 00ad9d00 and 00ad9da0, neither of which this
packet touches.

## Calling conventions and RET sizes

Read from the listing, not from the pseudocode; Ghidra loses register arguments in five of these.

| Address | Original interface | Return |
|---|---|---|
| 00af0060 | `__thiscall`, ECX = atlas manager `00f8c26c`, one stack argument (native string), RET 4 | void |
| 00740840 | `__thiscall`, ECX = freshly allocated 1Ch object, no stack arguments, RET | EAX = this |
| 00ad9ac0 | `__thiscall`, ECX = freshly allocated A8h object, no stack arguments, RET | EAX = this |
| 00ad71c0 | `__thiscall`, ECX = foliage system `00f8c210`, one stack byte, RET 4 | void |
| 00af1450 | `__thiscall`, ECX = freshly allocated 30h object, no stack arguments, RET | EAX = this |
| 00bbcb40 | `__cdecl`, no arguments, RET | void |
| 00af0b10 | `__thiscall`, ECX = freshly allocated 34h object, no stack arguments, RET | EAX = this |

`00ad71c0` reads its boolean with `MOV AL, byte ptr [ESP+4]` as its very first instruction. The
Ghidra listing that `bsp.py ghidra disasm 00ad71c0` prints starts at 00ad71c4 and omits it, which
makes the pseudocode look as though the flag arrives already in AL. The disk decode
(`bsp.py disasm-raw 00ad71c0`) shows the load; bytes at 00ad71c0 are `8a 44 24 04`.

Four of the seven are constructors whose base constructor publishes the instance into a fixed
global and registers a lifetime with `BSP_SingletonLifetime_Register` (00bd0c30). Init never stores
the returned pointer, so the global is the only handle.

| Constructor | Base ctor | Global it publishes | Derived vtable |
|---|---|---|---|
| 00740840 | 0073fa70 | 00e1aea0 | PTR_CG_scalar_deleting_dtor_00741060_00cff2a0 |
| 00ad9ac0 | 00ad6950 | 00f8c210 | PTR_CG_scalar_deleting_dtor_00ad97b0_00d5d270 |
| 00ad9ac0 nested 14h object | 00ad6810 | 00f8c264 | PTR_CG_scalar_deleting_dtor_00ad9bd0_00d5d274 |
| 00af1450 | 00af1140 | 00f8c280 | PTR_CG_scalar_deleting_dtor_00af1430_00d5d844 |
| 00af0b10 | 00af06a0 | 00f8c274 | PTR_CG_scalar_deleting_dtor_00af1080_00d5d7f8 |

## 00af0060, the atlas loader

Init builds the native string `interface/textures/common.ats` from 00ce81c4 and calls
`00af0060(ECX = [00f8c26c], &path)`.

The body logs `Loading atlas: %s` through 004254b0, lowercases a copy of the path, and rejects
anything whose last four bytes are not `.ats` under `__stricmp` against 00d5d23c. It then computes
the directory prefix as `max(rfind('\\'), rfind('/'))` over the two one-character constants 00ce7894
and 00ce7898, takes `substring(0, index + 1)`, and falls back to `./` (00d5d7d0) when neither
separator occurs. With the extension string `ats` (00d5d7cc) it calls 00886280 on the file-system
singleton `0109ceec`, which returns a heap array of names plus a count; the loop reads one name per
iteration through 00557a90 and the array is freed with `free` at 00af0396.

For each enumerated name it calls `00aef3c0(ECX = requested path, EDX = candidate)`, an
`__fastcall` predicate returning AL. That predicate is the interesting part:

1. lowercase both sides and replace every `\\` with `/` (00ce7894 to 00ce7898, through 004cad40);
2. exact case-insensitive equality returns true immediately;
3. a candidate shorter than the request returns false;
4. `rfind('/')` must return the *same index* on both sides, and when that index is non-negative the
   text before it must match case-insensitively;
5. both names must end in `.ats`;
6. the result is `equals_ci(stem(request) + "_", candidate.substr(0, len(request) - 3))`, where
   `stem` is the request without its four-byte extension. Both spans are measured from the request.

So one request pulls in every `<stem>_*.ats` sibling. This is not a curiosity: the installation has
**no** `interface/textures/common.ats`. It has exactly four matching files, `common_dxt1.ats`,
`common_dxt5_1.ats`, `common_dxt5_2.ats` and `common_dxt5_3.ats`, so the request names a logical
atlas that is stored as four DXT-format-specific parts.

When the predicate accepts, the code walks the manager's texture array (base +10h, count +14h,
element name string at +8 through the accessor 00b33e40, which is `LEA EAX,[ECX+8]; RET`) and calls
`00aef280(ECX = manager, &candidate)` only when no entry matched. **The comparison is against the
requested path, not against the candidate** (`MOV ECX,[EDI]` at 00af0305, EDI being the incoming
argument). Loaded textures carry `.dds` paths taken from the atlas header, so this guard cannot fire
for a `.ats` request. It is reproduced in the reconstruction because it is in the binary, not
because it can do anything.

`00aef280` allocates a 1Ch text-buffer object (00af5600), copies the name, resolves it through
`BSP_VFS_ResolveExistingName` (00bdf4c0), logs `Atlas file not found: %s` on failure, and otherwise
reads the file with 00af5850 and parses it with `BSP_TextureAtlas_ParseBuffer` (00aeeaf0). That
parser and the `.ats` grammar are already covered by `docs/ATLAS_PARSER.md` and implemented in
`src/texture_atlas.cpp`; this packet does not duplicate them.

`00af0060` has seven callers, so it is a general entry point, not a startup-only helper:
004c9a70 `BSP_Game_OnInitTitle`, 004e3aa0 `BSP_Game_OnInit`, 0057cb60, 0068cc70, 0073d410,
00873eb0 and 00ad60d0.

## 00740840, the decal definition table

Constructs the 1Ch decal system (allocated at 0073de6d), zeroes +04h..+18h, builds a Lua state
(00b66bd0 then 00b6a020 with 1) and loads `scripts/datatables/decals.lua` through 00b69d40. The
string length pushed to the resize is 1Dh, which is exactly the 29 bytes of that path.

It pushes the global table `Decals` (00b67800) and iterates it (00b67080, 00b66420, 00b67190). For
each element whose value is a table (00b660a0) it allocates 28h bytes, constructs the record with
00740410, and reads seven fields in this order. Value accessors: 00b662b0 string, 00b66270 float
(x87, stored down to float32), 00b66290 integer.

| Record offset | Lua key | Type | How it is resolved |
|---|---|---|---|
| +00, +04 | the table key | native string | copied from the iterator key |
| +08 | `Maxnum` | int32 | 00b66290 |
| +0C | `Size` | float | 00b66270; key constant is 00cff278 |
| +10 | `Radius` | float | 00b66270 |
| +14 | (none) | pointer | `decal.mvfm` through renderer vtable +34h, set by 00740410 |
| +18 | `Texture` | pointer | renderer `00f8d394` vtable +64h with (name, 0) |
| +1C | `Shader` | pointer | renderer `00f8d394` vtable +48h with (name) |
| +20 | `LifeTime` | float | 00b66270 |
| +24 | `FadeOutTime` | float | 00b66270 |

Records are appended to a doubling vector on the system object: base +10h, count +14h, capacity
+18h, growth through 0073e9a0 with `max(1, capacity * 2)`. That is the same three-field container
shape the atlas manager uses.

Installed-file check: `scripts/datatables/decals.lua` defines `Decals` with exactly three entries,
`machine_gun_decal`, `explosion_hole` and `landscape_hole`, and every entry carries all seven keys
in the order `Size, Radius, Maxnum, Texture, Shader, LifeTime, FadeOutTime`. All three use
`"white.tga"` and `"decal.mshd"`. `shaderfx/common/decal.shfx` exists; there is no loose
`decal.mvfm` anywhere in the installation.

## 00ad9ac0 and 00ae84e0, the foliage type table

`00ad9ac0` is only the constructor of the A8h foliage system (allocated at 0073de91). It zeroes most
of the object, sets the two bytes at +05h and +06h to 0 and 1, allocates a nested 14h object
constructed by 00ad6810 (published at 00f8c264), and then calls `00ae84e0`, which does the parsing.
It finally clears +9Ch..+A4h and the byte at +04h.

`00ae84e0` builds its own Lua state and loads `Effects\foliage\FoliageTypes.lua` (the literal is
stored with a trailing space in the image; the resize length is taken from the string, so the
trailing byte is not part of the request). It pushes the global `Foliages` and iterates it. Each
element has a `name` field and then a set of *indexed* sub-entries: the parser holds a running
one-based counter (initialised to 1 at 00ae87f8) and, through 004caca0, appends it to eight fixed
stems built at 00ae8899..00ae8be4 in this order: `model`, `modeltype`, `percent`, `uptexture`,
`sidetexture`, `scale`, `height`, `topheight`.

Per sub-index the parser probes with 00b65fb0 (nil test) and takes one of three branches:

* `model<N>` present: read `model<N>` (string), `modeltype<N>` (int) and `percent<N>` (int).
* `model<N>` absent, `uptexture<N>` present: read `uptexture<N>`, `sidetexture<N>` (strings),
  `scale<N>`, `height<N>`, `topheight<N>` (floats) and `percent<N>` (int).
* neither: emit `Foliage '%s' se modell, se uptexture nincs megadva!` and stop the sub-index walk.
  The message is Hungarian for "neither model nor uptexture was given".

Installed-file check: `effects/foliage/foliagetypes.lua` is 532 lines and defines 24 entries. Its
header comment records the `modeltype` domain as `0 - TREE, 1 - CORAL, 2 - SEAWEED`. The file uses
both branches: `3D Coconut` is a single-model entry, `3D Korall` has four model variants at 25 per
cent each, `Mix - Lombos` is an impostor entry with `uptexture<N>`/`sidetexture<N>` pairs pointing
at `effects/foliage/Baobab*.tga`. The impostor branch is what feeds the camera-facing quad batch
`docs/GAME_RENDER_TAIL.md` describes at 00af0c50.

## 00ad71c0, publishing the Foliage setting

`00ad71c0(ECX = [00f8c210], AL from [ESP+4])` writes the byte to `00f8c20c` and then forwards it to
two pointer vectors on the foliage system: `this+54h` iterated over `[+58h, +5Ch)` calling
`00ae0710(ECX = element, flag)`, and `this+64h` iterated over `[+68h, +6Ch)` calling
`00ae2c80(ECX = element, flag)`. Both loops reload `00f8c20c` per element rather than keeping the
argument. The interleaved calls to 00bf6713 are STL checked-iterator bound assertions.

The gate at 0073debf is `CMP byte ptr [00f88a06], 0`, pushing 1 when non-zero and 0 otherwise.
`00f88a06` is settings +86h on the settings object at 00f88980, which `docs/APP_INIT_BOOTSTRAP.md`
line 195 identifies as the **`Foliage`** key, constant 00d15ecc. There is no writing cross-reference
to 00f88a06 in the image, matching the pattern that doc already records for the other settings
bytes; the only other reader is 004de610.

`00f8c20c` has exactly four cross-references: the write and two reads inside 00ad71c0, and one read
in 00ad92c0. So the global is the per-process foliage-enable latch consumed by the foliage draw
path, not a general graphics flag.

## 00af1450, the particle shader and atlas set

Constructs the 30h object allocated at 0073df5c, in this order:

1. renderer `00f8d394` vtable +60h returns the index-buffer factory; its virtual +0Ch creates a
   buffer of **60000** indices, stored at +04h.
2. Fill loop 00af14fe..00af1520: a base counter runs `0 <= v < 40000` in steps of four, writing six
   16-bit indices per quad in the order `v, v+1, v+2, v+2, v+1, v+3`. That is 10000 quads and
   exactly 60000 indices. The counter is 32-bit and the store is 16-bit, so the base wraps through
   the signed-short boundary at v = 32768 and the stored values continue as unsigned 16-bit. The
   buffer is unlocked through virtual +10h.
3. `Particles/Textures/atl_all.dds` (length 1Eh = 30) through renderer vtable +64h. One reference is
   held across the whole construction and released at 00af1957 with `InterlockedDecrement` followed
   by the object's own virtual +0 when the count reaches zero.
4. `particleaxialsprite.mvfm` (length 18h = 24) through renderer vtable +38h into +1Ch, then vtable
   +40h builds the vertex declaration into +20h.
5. Five materials through `BSP_Material_CreateForEffectName` (00535320), each immediately given the
   atlas in texture slot 0 via `BSP_Material_SetTextureSlot` (00b189f0):
   `particleaxialsprite_n0s0.mshd` to +08h, `_n0s1` to +0Ch, `_n1s0` to +10h, `_n1s1` to +14h,
   `particleaxialsprite_dist.mshd` to +18h.
6. `ParticleFloating.mvfm` (length 15h = 21) into +28h, its declaration into +2Ch, then
   `ParticleFloating.mshd` into +24h, again with the atlas in slot 0.

Installed-file check: `particles/textures/atl_all.dds` exists and is 2,785,408 bytes.
`shaderfx/common/` contains `particleaxialsprite_n0s0.shfx`, `_n0s1`, `_n1s0`, `_n1s1`,
`particleaxialsprite_dist.shfx` and `particlefloating.shfx`, so the five permutations plus the
floating shader are real assets under a different extension. There is no `.mvfm` file anywhere in
the installation and no loose `.mpkg`; how the resource manager resolves a `.mvfm` request is
unresolved by this packet.

The set is published at `00f8c280` by the base constructor 00af1140.

## 00bbcb40, the water caustics and shore-wave texture sources

`00bbcb40` allocates 10h bytes, constructs them with `00bbc900`, and stores the result in
`01090900`; on allocation failure it stores 0. `00bbc900` fills four slots, each a separately
allocated four-byte object that is nothing but a vtable pointer:

| Slot | Name passed to the base constructor | Constructor | Vtable |
|---|---|---|---|
| +00 | `CausticsTextureSource` | 00bbc5f0 | PTR_LAB_00d644e8 |
| +04 | `ShoreWaveTextureSource0` | 00bbc740 | PTR_LAB_00d644f0 |
| +08 | `ShoreWaveTextureSource1` | 00bbc740 | PTR_LAB_00d644f0 |
| +0C | `ShoreWaveTextureSource2` | 00bbc740 | PTR_LAB_00d644f0 |

The caustics source and the shore-wave sources are therefore different classes sharing the base
registration path 00b1b730, which is itself a lazy singleton over `00f8d41c`. The name is the
registry key: `docs/GAME_WORLD_OCEAN.md` line 123 records `shorewavetexturesource0..2` alongside
`oceanheightmap` as the segment keywords of the ocean renderer, so these are the string keys the
shore-wave layers look up. `01090900` is otherwise touched only by
`BSP_GlobalObject_Destroy_01090900` (00bbc5d0) and one data reference at 00bbe2fe.

Because this runs at 0073d8cc, before the D3D9 device exists, the four objects can only be name
registrations; no texture is created here.

## 00af0b10, the foliage group manager

Constructs the 34h object allocated at 0073e005. The base constructor 00af06a0 writes the instance
to `00f8c274` at 00af06f6 and registers its lifetime. The derived body sets the vtable, zeroes
+04h..+18h and +24h..+28h, copies `00d7a24c` into +2Ch and calls `BSP_CriticalSection_Create`
(00bd1860) into +30h. `00d7a24c` holds the bytes `00 00 80 3f`, that is **1.0f**.

`00f8c274` is the object `docs/GAME_RENDER_TAIL.md` already identifies as the foliage group manager:
its `00af0450` setter is `MOV [ECX+2Ch], arg; RET 4` and the matching getter 00af0460 feeds
component z of shader constant c33 `Time` at 00b46cdb. So this packet establishes what that scalar
starts at, 1.0f, and that the manager owns a critical section from construction.

## Corrections and additions to existing docs

Listed here, not edited in place.

* `docs/APP_INITIALIZE_MAP.md` line 143 says 00ad71c0 "iterates the range `this+0x58 .. this+0x5c`
  and publishes `DAT_00f8c20c`". There are **two** ranges, `[+58h, +5Ch)` through 00ae0710 and
  `[+68h, +6Ch)` through 00ae2c80, and the flag is written before either loop rather than published
  after them. The same line does not record the gate; the argument is the `Foliage` setting byte
  00f88a06.
* `docs/APP_INITIALIZE_MAP.md` item 56 describes 00af0b10 as "object with a critical section, seeded
  from `DAT_00d7a24c`". It is specifically the foliage group manager singleton `00f8c274` of
  `docs/GAME_RENDER_TAIL.md`, and the seed is 1.0f into the +2Ch shader-time scalar.
* `docs/APP_INITIALIZE_MAP.md` item 47 records the atlas call as loading
  `interface/textures/common.ats`. That file does not exist; the call loads the four
  `common_*.ats` split parts through the 00aef3c0 prefix rule.
* `docs/GAME_RENDER_TAIL.md` line 258 lists "whatever registers records into +8h/+0Ch" of the
  particle clock as open. This packet does not close it: 00af1450 builds the shader and atlas set at
  `00f8c280` and never touches the clock singleton `00f8d420`. The two are separate objects.

## Uncertainties

* The dedupe guard in 00af0060 compares registered *texture* paths against the requested *.ats*
  path. On the evidence it can never match. Whether this is a defect or whether some caller passes a
  `.dds` path is not settled; all seven callers were not traced.
* `00ae84e0` was read for its key set, branch structure and file name, not line by line. The exact
  container the foliage records land in, and the meaning of the A8h object's remaining fields, are
  not recovered. The 815-line pseudocode has heavy stack-slot aliasing.
* The nested 14h object at `00f8c264` built inside 00ad9ac0 is unidentified beyond its vtable.
* Renderer vtable slot +34h (used for `decal.mvfm`) and +38h (used for the two particle formats)
  both return vertex formats but are different entries; why the decal record uses the earlier slot
  is not established.
* No `.mvfm` file exists in the installation, so the vertex-format request path is unverified
  against real data.
* `00f88a06` has no writing cross-reference, so what sets the `Foliage` setting is unresolved. This
  matches what `docs/APP_INIT_AUDIO_ONLINE.md` records for `00f889a4`.

## What remains

* Trace 00ae84e0 to the foliage record layout and the container it fills.
* Identify the 00f8c264 object and the `00ad9f90` / `00b3c4c0` constructors that Init runs between
  the foliage and particle steps (renderer-device packet).
* Resolve the `.mvfm` request path and the renderer vtable +34h / +38h split.
* Confirm the consumer side: 00ad92c0 reads 00f8c20c, 00ae0710 and 00ae2c80 take the flag.

## State reached

| Address | State |
|---|---|
| 00af0060 | reconstructed, build-tested, installed-file-checked (four `common_*.ats` parts) |
| 00aef3c0 | reconstructed, build-tested, installed-file-checked; one focused case in tests/math_tests.cpp |
| 00740840 | analyzed, record layout reconstructed, installed-file-checked (3 entries, 7 keys) |
| 00ad9ac0 | analyzed |
| 00ae84e0 | analyzed, key set and branch structure recovered, installed-file-checked (24 entries) |
| 00ad71c0 | reconstructed, build-tested |
| 00af1450 | reconstructed, build-tested, installed-file-checked (atlas texture and six `.shfx`) |
| 00bbcb40 | reconstructed, build-tested |
| 00af0b10 | reconstructed, build-tested |

Nothing here is a binary-compatible replacement. `include/bsp/world_effects_startup.hpp` and
`src/world_effects_startup.cpp` expose new C++ interfaces over an injected host, one method per
native call site.

## Ghidra function coverage

All seven leased addresses have Ghidra functions with complete bodies. The indirect helpers read
here (00aef3c0, 00aef280, 00ae84e0, 00bbc900, 00740410, 00af5850) are functions as well. No new
function definition is needed before the orchestrator applies the ledger names.

One tool problem: `python tools/bsp.py ghidra disasm <addr>` omits the function's first instruction.
For 00ad71c0 the listing begins at 00ad71c4 and drops `MOV AL, byte ptr [ESP+4]` at 00ad71c0, which
changes how the parameter appears to arrive. `bsp.py disasm-raw` shows it correctly.
