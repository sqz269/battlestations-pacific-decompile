# Descriptor samplers to material texture bindings

Read-only analysis on 2026-09-09. Each live export/read batch verified project
`bsp`, program `/battlestationspacific.exe` with `Client.verify()`. Ignored
evidence is in `exports/bsp/sampler_binding/`. Descriptive names below are
hypotheses; this document does not claim the material pass is reconstructed.

## Construction

The descriptor loader `00b45ee0` builds each selected combiner through
`00b3c3a0` -> `00b3b3c0`. After VS/PS creation, `00b3b3c0` clears builder
+8Ch/+90h/+94h and calls `00b3b280` first for the base descriptor, then for
the effect descriptor. These counters are shared across both calls.

`00b3b280` ABI: ECX builder, stack pass then descriptor, RET8 at00b3b3af.
It visits descriptor+C4h pointers in stored order for descriptor+C8h count.
There is no reflection-name lookup in this helper. Each sampler is handled
by its TextureSource (+14h):

| Source | Action before sampler-state processing |
|---|---|
|0|Append Index (+20h), VertexSampler (+0Ch) to the pass through00b5f100; increment builder+8Ch.|
|1|Lookup animator by source name (+18h) through004de4b0/00b1b4d0. On success, append a dynamic record using00b44cf0(builder+8Ch, animator, name), set pass+80h byte1, release temporary animator reference and increment builder+8Ch. On failure, no dynamic record or +8Ch increment.|
|3|Register Index/name with builder+78h owner using00b18fb0; append signed `-1 - Index` and stage through00b5f100; increment builder+8Ch.|
|2 or other|No texture record and no +8Ch increment in this helper. State processing still happens.|

Every descriptor sampler then contributes its +24h SamplerStates list to
`00b5ed60(pass, slot, state, value)` in stored order. Pixel slots use current
builder+94h; vertex slots use builder+90h +16. After state processing, that
stage counter advances once, including source2/unknown or failed animator
entries. Type/dimension and +28h TextureStageStates are not read in this
helper. Do not infer that a parsed texture-stage list is applied here.

`00b5f100`: ECX pass, stack signed texture index then stage byte, RET8.
It appends an8-byte record to pass+24h pointer/+28h count/+2Ch capacity.
Record+0 is index; record+4 is the stage byte. The other three bytes in the
copied stage DWORD come from uninitialized stack padding and are not flags.
Capacity doubles (minimum1); no sort, deduplication or bounds validation.

`00b5ed60`: ECX pass, stack slot/state/value, RET0Ch at00b5edf4. It accesses
the sampler block at pass+20h: vector pointer+8h, count+Ch, capacity+10h;
each12-byte record is slot/state/value. Search proceeds from the beginning.
The first matching `(slot,state)` is retained in place, with value replaced
if different; otherwise append. This is **last supplied value wins** for
the first matching record, unlike the Lua state-table parser's append helper.

After both descriptors, `00b3b3c0` removes sampler-state records for unused
pixel slots0..15 according to compiled PS metadata+84h. `00b5eff0` removes
every matching slot by swapping in the last record, so resulting order may
change. Vertex slots16 onward are outside that cleanup loop. Metadata+84h
is now produced by the recovered reflection mapping in shader_reflection.cpp;
see SHADER_COMPILED_CONSTANT_BINDING.md for the native x86 shift rule.

## Draw-time source0 path

`00b43410` ABI: ECX pass, six stack arguments, RET18h. Its texture fragment
is00b43470..00b4352f, after render/sampler blocks and shaders are bound.
The first stack argument is a render entry; entry+4 points to geometry,
geometry+20h points to the material used for source0.

The fragment visits pass+24h records with pass+28h count. It maintains an
independent pixel slot counter initially0 and vertex slot counter initially16.
For each record at **overall record ordinal i**, it tests
`PS_metadata[+84h] & (1u << (i & 31))`. If that bit is zero and the stage
byte is zero, it issues no texture call but still increments pixel slot.
This is not a test using the pixel-only counter; preserve the assembly's
flattened ordinal when mixed-stage records eventually become supported.
Vertex records bypass this optimization test.

For a selected nonnegative record index, native code loads the signed16-bit
texture count at material+34h. Index >= count gives null. Otherwise it reads
the logical texture pointer at material+10h +4*index. This is direct material
indexing, not a sampler-name lookup or descriptor append-position lookup.
For negative record indices, owner=pass+14h resolves `-1-index` through
00b17d90 instead. In particular, a malformed negative source0 Index would
enter the owner route; a host must not silently treat it as a normal material
index. Source3 overflow/malformed indices remain outside a safe adapter.

The selected texture, including null on an out-of-range source0 index, goes
to renderer virtual+130h with the current stage slot. Assembly00b43500
pushes EAX for **both** stage branches; the decompiler omits this texture
argument in its vertex call. Pixel slot advances once for every pixel record,
vertex slot once for every vertex record. The existing reconstructed
`bind_texture_00b24710` maps logical slots16+ to D3D slots257+ and preserves
logical identity ownership.

Dynamic records are separate from this static record vector:00b42350 visits
pass+5Ch pointer entries for +6Ch count, calls animator virtual+2Ch for a
texture, binds record+0 slot, then invokes animator virtual+30h to update
constants. ShadowMap/ShadowTexture special routes are handled elsewhere in
00b3b3c0 and late00b42350. Mixed static/dynamic slot interaction is not
established by the source0-only route and must not be invented.

## Smallest useful next implementation

Implement the complete bounded append/update helpers00b5f100 and00b5ed60 in
a typed material-pass representation, then the source0-only construction
slice of00b3b280 plus static binding slice00b43470..00b4352f. Retain source
index and stage separately; use the already implemented logical texture and
sampler-state cache bindings. Supply material textures from a typed owner
and compiled usage metadata from its real producer. A source0 pixel sampler
with Index0 (installed alphablend.shfx/MyTexture) is the smallest grounded
integration target for an actual textured material draw.

Do not count this as the complete00b3b280/00b43410 routines. Unimplemented
animator, owner-file, shadow and lifetime routes require explicit boundaries.
Reject unsupported sources in a typed partial builder rather than accepting
them with stubbed texture resolution. Native allocation failures, invalid
graphs and malformed index behavior are not ordinary host defaults.

## Byte evidence

Live bytes matched the original disk PE for these ranges. The first three
are full function bodies; the last is only the audited draw-time fragment.

|Start|Bytes|SHA-256|
|---|---:|---|
|00b3b280|306|cdf241047d8ec93b39fa9058ac6b15b71c386832dfb4170b2dc7c0fcce6e28d9|
|00b5f100|81|43f1eca0b14a4a2a70304c466e895e6c9c63794db89fa31beadd817cd955bd70|
|00b5ed60|151|950a8b3a6e0d214b81bfa62909c66cb6c00dc05243c92fa565c52c65a4ba8119|
|00b43470|192|e9e359c10c123a6e733b00054e1c1b442985b653189dc7090e66a8237369235b|

No native calls, new tests, C++ changes, Ghidra annotation changes, builds or
game execution were performed for this analysis.

## Implemented source0 material binding

`material_samplers.hpp/.cpp` supplies a typed pass with signed index/stage
records and a sampler-state block. Complete semantic append/update/removal
helpers00b5f100,00b5ed60,00b5eff0 are ported; vector growth, native padding
and intrusive allocation are not ABI-compatible. The source0 construction
projection carries shared counters across descriptor calls and uses parsed
SamplerStates. It intentionally does not apply TextureStageStates, which the
original helper never reads. Unused pixel-state pruning uses swap-last removal.

The binding projection retains the original overall record ordinal mask test,
separate stage counters and null for an out-of-range material index. A typed
material vector replaces the native signed16 count/array owner. It rejects
negative indices, owners above32767 textures and stage counts beyond D3D9
limits before issuing calls. Source1/3/other records fail construction before
mutation; those native routes are still pending. HRESULT failure stops later
binds, which is explicit host error handling rather than recovered rollback.
Finalize the pass before copying its state block into the identity-cached
renderer; mutating an already-bound identity would skip required device calls.

The existing shader probe evaluates installed alphablend.shfx, compiles a
diagnostic shader referencing its generated MyTexture declaration, and uses
the real reflected sampler mask1. A1x1 host texture supplied as material
index0 reaches device slot0 with wrap U/V states; GetTexture/GetSamplerState
verify identity/values. An empty material then binds null through the same
record. Device state is captured/restored and borrowed COM texture lifetime
outlives cache references. This is a binding check, not a full alphablend
textured draw. Build, existing CTests and all prior D3D9 pixel checks pass.
The update/removal edge cases and arbitrary mixed-stage layouts are grounded
in assembly but not exercised by that one installed sampler fixture.
